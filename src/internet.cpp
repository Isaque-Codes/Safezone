#include <Arduino.h>
#include <WiFi.h>
#include "internet.h"
#include "senhas.h"


//* -------CONFIGURAÇÃO DO WIFI--------

const unsigned long tempoEsperaConexao = 20000;
const unsigned long tempoEsperaReconexao = 10000;

void conectaWiFi()
{
    Serial.printf("Conectando ao WiFi: %s", SSID);
    WiFi.begin(SSID, SENHA);

    unsigned long tempoInicialWiFi = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - tempoInicialWiFi < tempoEsperaConexao)
    {
        Serial.print(".");
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi Conectado com sucesso! ");
        Serial.print("Endereço IP: ");
        Serial.println(WiFi.localIP());
    }

    else
    {
        Serial.println("\nFalha ao conectar no WiFi. Verifique o nome da rede e a senha");
    }
}

void checkWiFi()
{
    unsigned long tempoAtual = millis();
    static unsigned long tempoUltimaConexao = 0;

    if (tempoAtual - tempoUltimaConexao > tempoEsperaReconexao)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("\n Conexão Perdida! Tentando reconectar...");
            conectaWiFi();
        }
        tempoUltimaConexao = tempoAtual;
    }
}