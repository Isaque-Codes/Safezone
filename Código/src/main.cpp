#include <Arduino.h>
#include "monitoramento.h"
#include "sensorDeDigitais.h"
#include "internet.h"
#include "senhas.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ezTime.h>

// --- Configuracoes de Hardware e Rede ---

#define RX_FINGERPRINT 16
#define TX_FINGERPRINT 17
#define PASSWORD 0x00000000
#define pinButton 12
#define pinoTrava 25

// --- Instanciacao de Objetos ---

FingerprintSensor sensorDigital(&Serial2, PASSWORD, RX_FINGERPRINT, TX_FINGERPRINT);
WiFiClient espClient;
PubSubClient client(espClient);
Timezone tempoLocal;

// --- Configuracoes de Rede (MQTT)

const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqtt_id = "senai134-safezone-publisher";
const char *mqtt_topic_pub = "safezone-events";

// --- Variaveis de Estado ---

bool portaDestravada = false;
unsigned long tempoInicioDestravamento = 0;
const unsigned long duracaoDestravamento = 3000; // Tempo que a porta fica aberta

// --- Prototipacao das Funcoes ---

void liberarAcesso(PubSubClient &client, Timezone &tempo, const char *topico);
void enviarLeituraSensores(PubSubClient &client, Timezone &tempoLocal, const char *topico);
void mqttConnect(void);

// ====================================================================================
// SETUP
// ====================================================================================

void setup()
{
  Serial.begin(9600);
  Serial2.begin(57600, SERIAL_8N1, RX_FINGERPRINT, TX_FINGERPRINT);

  pinMode(pinoTrava, OUTPUT);
  digitalWrite(pinoTrava, LOW);
  pinMode(pinButton, INPUT_PULLUP);

  conectaWiFi();
  client.setServer(mqtt_server, mqtt_port);
  tempoLocal.setLocation("America/Sao_Paulo");

  iniciarMonitoramento();

  if (!sensorDigital.begin(57600))
  {
    Serial.println("Falha ao inicializar o sensor de digitais.");
  }

  Serial.println("Sistema de seguranca iniciado.");
}

// ====================================================================================
// LOOP
// ====================================================================================

void loop()
{
  checkWiFi();
  if (!client.connected())
    mqttConnect();

  client.loop();

  atualizarMonitoramento();

  enviarLeituraSensores(client, tempoLocal, mqtt_topic_pub);

  // --- Sessao para gerenciamento de impressoes digitais ---
  // --- Desativado durante o funcionamento, que apenas verifica ao apertar o botao ---
  /*if (Serial.available())
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
  }*/

  // --- Verificacao de impressoes digitais pelo botao (com tecnica de debounce) ---
  static bool novaTentativaDeAcesso = false;

  unsigned long currentTime = millis();

  bool stateButton = digitalRead(pinButton);
  static bool previousStateButton = 1;
  static bool lastAction = 1;

  const unsigned long debounceTime = 50;
  static unsigned long previousTime = 0;

  if (stateButton != previousStateButton)
  {
    previousTime = currentTime;
  }

  if ((currentTime - previousTime) > debounceTime)
  {
    if (stateButton != lastAction)
    {
      lastAction = stateButton;
      if (!stateButton) // O botão foi pressionado
      {
        sensorDigital.verifyFingerprint();
        novaTentativaDeAcesso = true;
      }
      else
      {
        // O botao foi solto
      }
    }
  }

  if (novaTentativaDeAcesso)
  {
    liberarAcesso(client, tempoLocal, mqtt_topic_pub);
    novaTentativaDeAcesso = false;
  }

  if (portaDestravada && (millis() - tempoInicioDestravamento >= duracaoDestravamento))
  {
    portaDestravada = false;
    digitalWrite(pinoTrava, LOW); // Trava a porta
  }
}

// ====================================================================================
// FUNCOES
// ====================================================================================

void liberarAcesso(PubSubClient &client, Timezone &tempo, const char *topico)
{
  unsigned long agora = millis();
  {
    JsonDocument doc;
    String mensagem;

    doc["liberar_Acesso"] = sensorDigital.isAccessGranted(); // Envia as tentativas de acesso (bem ou nao sucedidas)
    doc["timestamp"] = tempo.now();

    serializeJson(doc, mensagem);
    Serial.println("[MQTT] Enviando leitura sensores:");
    Serial.println(mensagem);
    client.publish(topico, mensagem.c_str());
  }

  if (sensorDigital.isAccessGranted())
  {
    portaDestravada = true;
    tempoInicioDestravamento = millis();
    digitalWrite(pinoTrava, HIGH); // Destrava a porta
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

    // --- Envia as leituras do sistema de alarme a cada 3 segundos ---
    doc["sensor_luz"] = alarmeSensorLuz;
    doc["sensor_movimento"] = alarmeSensorMovimento;
    doc["sensor_pressao"] = alarmeSensorPressao;
    doc["timestamp"] = tempo.now();

    serializeJson(doc, mensagem);
    client.publish(topico, mensagem.c_str());
  }
}

void mqttConnect()
{
  while (!client.connected())
  {
    Serial.println("Conectando ao MQTT...");

    if (client.connect(mqtt_id))
    {
      Serial.println("Conectado com sucesso");
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