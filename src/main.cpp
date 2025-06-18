#include "monitoramento.h"
#include "sensorDeDigitais.h"
#include <Arduino.h>
#include "internet.h"
#include "mqtt.h"

#define RX_FINGERPRINT 16
#define TX_FINGERPRINT 17
#define PASSWORD 0x00000000

FingerprintSensor sensorDigital(&Serial2, PASSWORD, RX_FINGERPRINT, TX_FINGERPRINT);

unsigned long tempoUltimaVerificacao = 0;
const unsigned long intervaloVerificacao = 100; // tempo em ms entre cada verificação

void setup()
{
  Serial.begin(9600);
  Serial2.begin(57600, SERIAL_8N1, RX_FINGERPRINT, TX_FINGERPRINT);

  iniciarMonitoramento();

  if (!sensorDigital.begin(57600))
  {
    Serial.println("Falha ao inicializar o sensor de digitais.");
  }

  Serial.println("Sistema de segurança iniciado.");
  mqtt();        // Inicializa conexão Wi-Fi e MQTT
  mqttConnect(); // Garante conexão com o broker

  // Teste de tratamento de mensagem JSON recebida
  String msg = "{\"teste2\": true}";
  tratamentoMsg(msg);
}

void loop()
{
  atualizarMonitoramento();

  bool dispararAlarme = alarmeSensorPressao || alarmeSensorMovimento || alarmeSensorLuz;

  // Leitura não bloqueante do menu via Serial
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
      // Aqui você pode acionar a trava elétrica ou registrar o acesso
    }

    if (dispararAlarme)
    {
      // Serial.println("ALARME ATIVADO!");
      // Aqui você pode acionar buzina, luz ou enviar alerta
    }
  }

  // Nenhum delay necessário — o loop roda livre e rápido
  testeconect(); // Mantém conexão e publica mensagens periodicamente
}
