#include <Arduino.h>
#include "monitoramento.h"
#include "sensorDeDigitais.h"
#include "internet.h"
#include "senhas.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ezTime.h>
#include <LiquidCrystal_I2C.h>

#define RX_FINGERPRINT 16
#define TX_FINGERPRINT 17
#define PASSWORD 0x00000000

FingerprintSensor sensorDigital(&Serial2, PASSWORD, RX_FINGERPRINT, TX_FINGERPRINT);

WiFiClient espClient;
PubSubClient client(espClient);
Timezone tempoLocal;
LiquidCrystal_I2C lcd(0x27, 20, 4);

const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqtt_id = "esp32-senai134-publisher";
const char *mqtt_topic_sub = "senai134/teste1/publicando";
const char *mqtt_topic_pub = "senai134/teste1/publicando";

unsigned long tempoUltimaVerificacao = 0;
const unsigned long intervaloVerificacao = 100;

void enviarLeituraSensores(PubSubClient &client, Timezone &tempoLocal, const char *topico);
void callback(char *, byte *, unsigned int);
void mqttConnect(void);
void tratamentoMsg(String);
void mostraDisplay(float temp, float umid, time_t timestamp);
void templateDisplay(void);

void setup()
{
  Serial.begin(9600);
  Serial2.begin(57600, SERIAL_8N1, RX_FINGERPRINT, TX_FINGERPRINT);

  lcd.init();
  lcd.backlight();

  conectaWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  tempoLocal.setLocation("America/Sao_Paulo");

  iniciarMonitoramento();

  if (!sensorDigital.begin(57600))
  {
    Serial.println("Falha ao inicializar o sensor de digitais.");
  }

  Serial.println("Sistema de segurança iniciado.");
}

void loop()
{
  checkWiFi();
  if (!client.connected())
    mqttConnect();

  client.loop();

  atualizarMonitoramento();

  bool dispararAlarme = alarmeSensorPressao || alarmeSensorMovimento || alarmeSensorLuz;

  enviarLeituraSensores(client, tempoLocal, mqtt_topic_pub);

  if (Serial.available())
  {
    char opcao = Serial.read();

    switch (opcao)
    {
    case '1':
      sensorDigital.enrollFingerprint();
      break;
    case '2':
      sensorDigital.verifyFingerprint();
      break;
    case '3':
      sensorDigital.deleteFingerprint();
      break;
    case '4':
      sensorDigital.getFingerprintCount();
      break;
    case 'm':
    case 'M':
      sensorDigital.printMenu();
      break;
    default:
      Serial.println("Opcao invalida.");
      break;
    }
  }

  // Checagem periódica não bloqueante
  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoUltimaVerificacao >= intervaloVerificacao)
  {
    tempoUltimaVerificacao = tempoAtual;

    if (sensorDigital.isAccessGranted())
    {
      // Serial.println("Acesso autorizado via impressão digital!");
    }

    if (dispararAlarme)
    {
      // Serial.println("ALARME ATIVADO!");
    }
  }
}

void enviarLeituraSensores(PubSubClient &client, Timezone &tempo, const char *topico)
{
  static unsigned long ultimaLeitura = 0;
  const unsigned long intervaloLeitura = 3000;

  unsigned long agora = millis();
  if (agora - ultimaLeitura >= intervaloLeitura)
  {
    ultimaLeitura = agora;

    JsonDocument doc;
    String mensagem;

    doc["sensor_luz"] = alarmeSensorLuz;
    doc["sensor_movimento"] = alarmeSensorMovimento;
    doc["sensor_pressao"] = alarmeSensorPressao;
    doc["timestamp"] = tempo.now();

    serializeJson(doc, mensagem);
    Serial.println("[MQTT] Enviando leitura sensores:");
    Serial.println(mensagem);
    client.publish(topico, mensagem.c_str());
  }
}

void liberarAcesso(PubSubClient &client, Timezone &tempo, const char *topico)
{
  unsigned long agora = millis();
  {
    JsonDocument doc;
    String mensagem;

    doc["sensor_luz"] = "acessoLiberado";
    doc["timestamp"] = tempo.now();

    serializeJson(doc, mensagem);
    Serial.println("[MQTT] Enviando leitura sensores:");
    Serial.println(mensagem);
    client.publish(topico, mensagem.c_str());
  }
}

void tratamentoMsg(String msg)
{
  String mensagem = msg;
  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, mensagem);

  if (erro)
  {
    Serial.println("Mensagem Recebida não esta no formato Json");
    return;
  }

  else
  {
    float temperatura, umidade;
    time_t timestamp;

    if (!doc["temp"].isNull())
      temperatura = doc["temp"];

    if (!doc["umid"].isNull())
      umidade = doc["umid"];

    if (!doc["timestamp"].isNull())
      timestamp = doc["timestamp"];

    mostraDisplay(temperatura, umidade, timestamp);
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.printf("mensagem recebida em %s: ", topic);

  String mensagem = "";
  for (unsigned int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    mensagem += c;
  }
  Serial.println(mensagem);

  tratamentoMsg(mensagem);
}

void mostraDisplay(float temp, float umid, time_t time)
{
  float temperatura = temp;
  float umidade = umid;
  time_t timestamp = time;

  lcd.setCursor(6, 1);
  temperatura = constrain(temperatura, 0, 99);
  lcd.print(temperatura, 1);

  umidade = constrain(umidade, 0, 100);
  lcd.setCursor(6, 2);
  lcd.print(umidade, 1);

  lcd.setCursor(0, 3);
  tempoLocal.setTime(timestamp);
  lcd.print(tempoLocal.dateTime("d/m/Y H:i"));
}

void mqttConnect()
{
  while (!client.connected())
  {
    Serial.println("Conectando ao MQTT...");

    if (client.connect(mqtt_id))
    {
      Serial.println("Conectado com sucesso");
      client.subscribe(mqtt_topic_sub);
    }

    else
    {
      Serial.print("falha, rc=");
      Serial.println(client.state());
      Serial.println("tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void templateDisplay()
{
  lcd.home();
  lcd.print("Ambiente Monitorado");

  lcd.setCursor(0, 1);
  lcd.print("Temp:     C"); // (6,1)

  lcd.setCursor(0, 2);
  lcd.print("Umid:     %"); // (6,1)
}