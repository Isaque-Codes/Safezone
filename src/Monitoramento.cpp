#include "monitoramento.h"
#include <HX711.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

// ------------------- SENSOR DE PRESSÃO -------------------
const int LOADCELL_DOUT_PIN = 5;
const int LOADCELL_SCK_PIN = 18;
HX711 scale;
float medida = 0.0;
bool alarmeSensorPressao = false;
const float LIMIAR_PESO = 5.0;
unsigned long tempoAnteriorPressao = 0;
const unsigned long INTERVALO_PRESSAO = 5000;

// ------------------- SENSOR DE MOVIMENTO -------------------
Adafruit_VL53L0X lox;
bool alarmeSensorMovimento = false;
unsigned long ultimoMillisMovimento = 0;
const unsigned long INTERVALO_MOVIMENTO = 500;
int distanciaCM = 0;

// ------------------- SENSOR DE LUZ -------------------
const int pinSensorLuz = 33;
bool alarmeSensorLuz = false;
int leituraLDR = 0;
const int LIMIAR_LUZ = 100;
unsigned long ultimoMillisLuz = 0;
const unsigned long INTERVALO_LUZ = 500;

// ------------------- INICIALIZAÇÃO -------------------
void iniciarMonitoramento()
{
    // PRESSÃO
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale(41795);

    // Aguarda 2 segundos sem bloquear o sistema
    unsigned long start = millis();
    while (millis() - start < 2000)
    {
        // Executa outras tarefas aqui se necessário
        yield(); // para evitar watchdog reset em alguns ambientes
    }

    scale.tare();
    scale.power_up();
    Serial.println("Sensor de pressão iniciado");

    // MOVIMENTO
    Wire.begin();
    if (!lox.begin())
    {
        Serial.println("Erro ao iniciar sensor VL53L0X");
        while (1)
            ;
    }

    // LUZ
    pinMode(pinSensorLuz, INPUT);
}

//* ------------------- LOOP DE MONITORAMENTO -------------------
void atualizarMonitoramento()
{
    unsigned long agora = millis();

    // --- SENSOR DE PRESSÃO ---
    if (agora - tempoAnteriorPressao >= INTERVALO_PRESSAO)
    {
        tempoAnteriorPressao = agora;
        medida = scale.get_units(5);
        if (medida < 0)
            medida = 0;

        alarmeSensorPressao = (medida >= LIMIAR_PESO);
    }

    // --- SENSOR DE MOVIMENTO ---
    if (agora - ultimoMillisMovimento >= INTERVALO_MOVIMENTO)
    {
        ultimoMillisMovimento = agora;

        VL53L0X_RangingMeasurementData_t measure;
        lox.rangingTest(&measure, false);
        distanciaCM = measure.RangeMilliMeter / 10;
        if (distanciaCM < 40)
        {
            alarmeSensorMovimento = 1;
        }
        else
        {
            alarmeSensorMovimento = 0;
        }
    }

    // --- SENSOR DE LUZ ---
    if (agora - ultimoMillisLuz >= INTERVALO_LUZ)
    {
        ultimoMillisLuz = agora;

        leituraLDR = analogRead(pinSensorLuz);
        alarmeSensorLuz = (leituraLDR > LIMIAR_LUZ);
    }

    // -- -RESUMO SERIAL-- -
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
