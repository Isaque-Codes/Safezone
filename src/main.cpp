#include <Arduino.h>
#include "monitoramento.h"

void setup()
{
  Serial.begin(9600);
  iniciarMonitoramento();
  Serial.println("Sistema de monitoramento iniciado.");
}

void loop()
{
  atualizarMonitoramento();

  bool dispararAlarme = alarmeSensorPressao || alarmeSensorMovimento || alarmeSensorLuz;

  if (dispararAlarme)
  {
    Serial.println("🚨 ALARME DISPARADO! 🚨");
    // Adicione ação aqui, como acionar buzzer ou LED
  }

  delay(100); // Pequeno atraso para estabilidade
}
