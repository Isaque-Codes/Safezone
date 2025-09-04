#ifndef MONITORAMENTO_H
#define MONITORAMENTO_H

#include <Arduino.h>

extern bool alarmeSensorPressao;
extern bool alarmeSensorMovimento;
extern bool alarmeSensorLuz;
extern float medida;
extern int distanciaCM;
extern int leituraLDR;

void iniciarMonitoramento();
void atualizarMonitoramento();

#endif