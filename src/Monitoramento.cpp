#include "monitoramento.h"
#include <HX711.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

//* --- PRESSÃO (HX711) ---
const int LOADCELL_DOUT_PIN = 5;
const int LOADCELL_SCK_PIN = 19;
HX711 scale;
bool alarmeSensorPressao = false;
const float LIMIAR_PRESSAO = 5.0;
const unsigned long INTERVALO_PRESSAO = 500;
unsigned long ultimoMillisPressao = 0;

//* --- MOVIMENTO (VL53L0X) ---
Adafruit_VL53L0X lox;
bool alarmeSensorMovimento = false;
const unsigned long INTERVALO_MOVIMENTO = 500;
unsigned long ultimoMillisMovimento = 0;

//* --- LUZ (LDR) ---
const int pinSensorLuz = 4;
bool alarmeSensorLuz = false;
const int LIMIAR_LUZ = 300;
const unsigned long INTERVALO_LUZ = 500;
unsigned long ultimoMillisLuz = 0;

// Variáveis globais para mostrar no resumo
float medida = 0.0;
int distanciaCM = -1;
int leituraLDR = 0;

void iniciarMonitoramento()
{
    //* PRESSÃO
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale(41795);
    delay(2000);
    scale.tare();

    //* MOVIMENTO
    Wire.begin();
    if (!lox.begin())
    {
        Serial.println("Falha ao iniciar VL53L0X. Verifique conexão!");
        while (1)
            ;
    }

    //* LUZ
    pinMode(pinSensorLuz, INPUT);
}

void atualizarMonitoramento()
{
    unsigned long agora = millis();

    //* --- Sensor de Pressão ---
    if (agora - ultimoMillisPressao >= INTERVALO_PRESSAO)
    {
        ultimoMillisPressao = agora;

        medida = scale.get_units(5);
        alarmeSensorPressao = (medida >= LIMIAR_PRESSAO);

        scale.power_down();
        scale.power_up();
    }

    //* --- Sensor de Movimento ---
    if (agora - ultimoMillisMovimento >= INTERVALO_MOVIMENTO)
    {
        ultimoMillisMovimento = agora;

        VL53L0X_RangingMeasurementData_t measure;
        lox.rangingTest(&measure, false);

        if (measure.RangeStatus != 4)
        {
            distanciaCM = measure.RangeMilliMeter / 10;
            alarmeSensorMovimento = (distanciaCM >= 5 && distanciaCM <= 200);
        }
        else
        {
            distanciaCM = -1;
            alarmeSensorMovimento = false;
        }
    }

    //* --- Sensor de Luz (LDR) ---
    if (agora - ultimoMillisLuz >= INTERVALO_LUZ)
    {
        ultimoMillisLuz = agora;

        leituraLDR = analogRead(pinSensorLuz);
        alarmeSensorLuz = (leituraLDR > LIMIAR_LUZ);
    }

    //* --- Resumo das leituras e estados ---
    Serial.println("===== RESUMO MONITORAMENTO =====");

    Serial.print("Alarme Pressão: ");
    Serial.print(alarmeSensorPressao ? "ATIVO" : "inativo");
    Serial.print(" | Peso: ");
    Serial.print(medida, 2);
    Serial.println(" kg");

    Serial.print("Alarme Movimento: ");
    Serial.print(alarmeSensorMovimento ? "ATIVO" : "inativo");
    Serial.print(" | Distância: ");
    if (distanciaCM != -1)
        Serial.print(distanciaCM);
    else
        Serial.print("Fora de alcance");
    Serial.println(" cm");

    Serial.print("Alarme Luz: ");
    Serial.print(alarmeSensorLuz ? "ATIVO" : "inativo");
    Serial.print(" | Leitura LDR: ");
    Serial.println(leituraLDR);

    Serial.println("================================");
}
