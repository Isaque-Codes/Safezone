#ifndef MONITORAMENTO_H
#define MONITORAMENTO_H

#include <Arduino.h>

extern bool alarmeSensorPressao;
extern bool alarmeSensorMovimento;
extern bool alarmeSensorLuz;

void iniciarMonitoramento();
void atualizarMonitoramento();

#endif
