#include "monitoramento.h"

void setup()
{
  Serial.begin(9600);
  iniciarMonitoramento();
  Serial.println ("dvujdvujiuuief");
}

void loop()
{
  atualizarMonitoramento();

  bool dispararAlarme = alarmeSensorPressao || alarmeSensorMovimento || alarmeSensorLuz;

  if (dispararAlarme)
  {
    // Aciona o alarme
  }
}
