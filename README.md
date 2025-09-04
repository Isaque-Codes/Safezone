# Safezone 🛡️ - Sistema de Segurança e Controle de Acesso

<p align="center">
  <img src="https://img.shields.io/badge/status-concluído-green" alt="Status: Concluído">
  <img src="https://img.shields.io/badge/licença-MIT-blue" alt="Licença MIT">
  <img src="https://img.shields.io/badge/plataforma-ESP32-orange" alt="Plataforma: ESP32">
</p>

> **Nota:** Para uma visão geral do projeto, incluindo sua aplicação comercial e PDF de apresentação, confira as imagens e os slides na pasta [`Apresentação`](/Apresentação).

## 📄 Sobre o Projeto

O **Safezone** é um sistema embarcado de segurança e controle de acesso desenvolvido em equipe como **projeto de conclusão do primeiro semestre** do SENAI. A solução utiliza um microcontrolador ESP32 para criar um ambiente monitorado de forma inteligente, combinando autenticação biométrica com múltiplos sensores ambientais e **comunicação autônoma** em tempo real via **IoT**.

O desenvolvimento seguiu metodologias ágeis (**Scrum** e **Kanban**). Nossa equipe introduziu conceitos de **Programação Orientada a Objetos (POO)** em **C/C++**, visando maior modularidade e organização do código, antes mesmo destes conceitos serem formalmente abordados no curso.

O projeto é dividido em duas unidades de hardware que se comunicam de forma automática via MQTT:

1.  **Esp Publisher (Local):** Instalado no ambiente a ser protegido, responsável por ler os sensores, gerenciar o acesso biométrico, publicar todos os eventos e controlar o acesso.
2.  **Esp Subscriber (Remoto):** Atua como uma central de notificações, recebendo os eventos e alertando o usuário através de um display LCD e alarme sonoro.

---

## ✨ Funcionalidades Principais

*   **Controle de Acesso Biométrico:** Liberação de acesso através de um sensor de impressões digitais.
*   **Monitoramento Multi-Sensor:**
    *   **Sensor de Pressão (HX711 + Célula de Carga):** Detecta peso sobre uma superfície.
    *   **Sensor de Movimento (VL53L0X):** Detecta presença ou proximidade.
    *   **Sensor de Luminosidade (LDR):** Detecta alterações súbitas na iluminação.
*   **Comunicação em Tempo Real (IoT):** Utiliza o protocolo MQTT para enviar status e alertas de forma instantânea.
*   **Sistema de Alarme Remoto:** A unidade Subscriber dispara um alarme sonoro (buzzer) ao receber um alerta de invasão.
*   **Feedback Visual:** Um display LCD na unidade Subscriber exibe o status do sistema e os resultados das tentativas de acesso.
*   **Atuador de Trava:** Controle de uma trava solenoide para bloquear/desbloquear fisicamente o acesso.

---

## 🛠️ Arquitetura e Tecnologias

O sistema é construído sobre o **microcontrolador ESP32**, programado em C/C++ utilizando o framework Arduino. A comunicação segue o padrão Publish/Subscribe.

| Componente | Hardware | Bibliotecas Principais | Responsabilidade |
| :--- | :--- | :--- | :--- |
| **Esp Publisher** | `ESP32`, `Sensor Digital`, `HX711`, `VL53L0X`, `LDR`, `Trava Solenoide` | `Adafruit_Fingerprint`, `HX711`, `Adafruit_VL53L0X`, `PubSubClient` | Coletar dados, autenticar, atuar na trava e publicar eventos. |
| **Esp Subscriber** | `ESP32`, `Display LCD I2C`, `Buzzer` | `LiquidCrystal_I2C`, `PubSubClient`, `ArduinoJson` | Receber eventos, exibir status no LCD e acionar o alarme sonoro. |
| **Comunicação** | `Wi-Fi` | `PubSubClient`, `ArduinoJson` | Troca de mensagens JSON automatizadas via broker MQTT. |

---

# 🚀 Como Replicar o Projeto

## 1. Esp Publisher (Monitoramento Local)

#### Montagem do Hardware

Conecte os componentes ao ESP32 da unidade Publisher conforme a tabela abaixo.

| Componente | Pino no Componente | Pino no ESP32 |
| :--- | :--- | :--- |
| **Sensor de Digital** | `TX (Transmissor)` | `GPIO 16 (RX2)` |
| | `RX (Receptor)` | `GPIO 17 (TX2)` |
| **Sensor de Pressão (HX711)** | `DOUT` | `GPIO 5` |
| | `SCK` | `GPIO 18` |
| **Sensor de Movimento (VL53L0X)** | `SCL` | `GPIO 22 (SCL)` |
| | `SDA` | `GPIO 21 (SDA)` |
| **Sensor de Luz (LDR)** | `Pino de Sinal` | `GPIO 33` |
| **Botão de Verificação** | `-` | `GPIO 12` |
| **Trava Solenoide** | `Pino de Sinal` | `GPIO 25` |

*Lembre-se de alimentar os sensores e a trava com a tensão correta (VCC e GND), utilizando um módulo relé para a trava, se necessário.*

#### Instalação do Software

1.  Clone o repositório:
    ```bash
    git clone https://github.com/Isaque-Codes/Safezone.git
    ```
2.  Abra a pasta do projeto no **Visual Studio Code** com a extensão **PlatformIO IDE** instalada.
3.  O PlatformIO instalará as dependências listadas em `platformio.ini` automaticamente.
4.  No arquivo `src/senhas.cpp`, altere as credenciais do Wi-Fi.
5.  No arquivo `src/main.cpp`, certifique-se de que o tópico MQTT esteja definido como:
    ```cpp
    const char *mqtt_topic_pub = "safezone/events";
    ```
6.  Conecte o ESP32, selecione a porta COM correta para o seu dispositivo e clique em Upload no PlatformIO.

#### Gerenciamento de Impressões Digitais

Para cadastrar ou excluir digitais, é necessário usar o Monitor Serial:
1.  No arquivo `src/main.cpp`, encontre e descomente o bloco de código abaixo do comentário `// --- Sessao para gerenciamento de impressoes digitais ---`.
2.  Faça o upload do código modificado.
3.  Abra o **Serial Monitor** (baud rate: 9600).
4.  Siga as instruções no menu para gerenciar as impressões digitais.
5.  Após o gerenciamento, comente o bloco de código novamente e faça o upload da versão final para operação autônoma.

---

## 2. Esp Subscriber (Central de Notificação)

#### Montagem do Hardware

Conecte os componentes ao ESP32 da unidade Subscriber.

| Componente | Pino no Componente | Pino no ESP32 |
| :--- | :--- | :--- |
| **Display LCD I2C (20x4)** | `SCL` | `GPIO 22 (SCL)` |
| | `SDA` | `GPIO 21 (SDA)` |
| **Buzzer Ativo** | `Pino de Sinal` | `GPIO 26` |

#### Instalação do Software

O código do Subscriber é simples e pode ser criado em um novo projeto PlatformIO. Ele não está no repositório porque ambos os códigos não podem coexistir no mesmo projeto. Siga os passos abaixo para replicá-lo:

1.  Crie um novo projeto no PlatformIO para o ESP32.
2.  Crie os arquivos `senhas.h`, `senhas.cpp`, `internet.h` e `internet.cpp` e copie o conteúdo correspondente do projeto Publisher para eles.
3.  Crie o arquivo `main.cpp` e copie o código abaixo para ele:
    *   Conectar-se ao Wi-Fi e ao broker MQTT.
    *   Inscrever-se no tópico de eventos (`safezone/events`).
    *   Definir uma função `callback` para processar as mensagens JSON recebidas.
    *   Dentro do `callback`, verificar o conteúdo da mensagem e controlar o display LCD e o buzzer.

<details>
<summary><strong>Clique aqui para copiar o código completo do main.cpp para o ESP Subscriber</strong></summary>

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <ezTime.h>
#include "internet.h"

// --- Hardware e Rede (Buzzer e MQTT) ---
#define pinoBuzzer 26
const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqtt_id = "senai134-safezone-subscriber";
const char *mqtt_topic_sub = "safezone-events";

// --- Instanciacao de Objetos ---
LiquidCrystal_I2C lcd(0x27, 20, 4);
WiFiClient espClient;
PubSubClient client(espClient);
Timezone tempoLocal;

// --- Prototipacao das Funcoes ---
void mqttConnect(void);
void callback(char *topic, byte *payload, unsigned int length);
void tratamentoMsg(String msg);
void mostraDisplay(JsonDocument &doc);
void templateDisplay(void);

// ====================================================================================
// SETUP
// ====================================================================================

void setup()
{
    Serial.begin(9600);

    pinMode(pinoBuzzer, OUTPUT);
    digitalWrite(pinoBuzzer, LOW);

    lcd.init();
    lcd.backlight();
    templateDisplay();

    conectaWiFi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    tempoLocal.setLocation("America/Sao_Paulo");
    Serial.println("Sistema de notificação Safezone (Subscriber) iniciado.");
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
}

// ====================================================================================
// FUNCOES
// ====================================================================================

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
            Serial.print(client.state());
            Serial.println(" tentando novamente em 5 segundos");
            delay(5000);
        }
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.printf("mensagem recebida em %s: ", topic);

    String mensagem = "";
    for (unsigned int i = 0; i < length; i++)
    {
        mensagem += (char)payload[i];
    }
    Serial.println(mensagem);

    tratamentoMsg(mensagem);
}

void tratamentoMsg(String msg)
{
    JsonDocument doc;
    DeserializationError erro = deserializeJson(doc, msg);

    if (erro)
    {
        Serial.println("Mensagem Recebida nao esta no formato Json");
        return;
    }

    mostraDisplay(doc);
}

void mostraDisplay(JsonDocument &doc)
{
    // Processa mensagens de monitoramento e alarme dos sensores
    if (doc.containsKey("sensor_luz"))
    {
        bool invasao = doc["sensor_luz"] || doc["sensor_movimento"] || doc["sensor_pressao"];
        if (invasao)
        {
            digitalWrite(pinoBuzzer, HIGH);
            lcd.setCursor(0, 1);
            lcd.print("!!! INVASAO !!!   ");
        }
        else
        {
            digitalWrite(pinoBuzzer, LOW);
            lcd.setCursor(0, 1);
            lcd.print("Status: Seguro      ");
        }
    }

    // Processa mensagens de tentativas de acesso
    if (doc.containsKey("liberar_Acesso"))
    {
        lcd.setCursor(0, 2);
        if (doc["liberar_Acesso"] == true)
        {
            lcd.print("Acesso: Liberado    ");
        }
        else
        {
            lcd.print("Acesso: Negado      ");
        }
    }

    // Atualiza o timestamp
    if (doc.containsKey("timestamp"))
    {
        time_t timestamp = doc["timestamp"];
        tempoLocal.setTime(timestamp);
        lcd.setCursor(0, 3);
        lcd.print(tempoLocal.dateTime("d/m/Y H:i:s"));
    }
}

void templateDisplay()
{
    lcd.clear();
    lcd.home();
    lcd.print("Safezone Notifier");
    lcd.setCursor(0, 1);
    lcd.print("Status: Aguardando..");
    lcd.setCursor(0, 2);
    lcd.print("Acesso: -----------");
}
```
</details>

---

## 3. Testando a Comunicação (Opcional)

Você pode visualizar as mensagens MQTT em tempo real usando um cliente de desktop como o **MQTT.fx**.

1.  Baixe e instale o [MQTT.fx](https://mqttfx.jensd.de/).
2.  Conecte-se ao mesmo broker usado no projeto:
    *   **Broker Address:** `broker.hivemq.com`
    *   **Broker Port:** `1883`
3.  Vá para a aba **Subscribe**.
4.  Digite o tópico `safezone/events` no campo de tópico e clique em **Subscribe**.
5.  Agora, qualquer evento gerado pelo seu **Esp Publisher** (uma tentativa de acesso, um alarme de sensor) aparecerá em tempo real no log do MQTT.fx. Isso confirma que seu sistema está publicando os dados corretamente na nuvem.

---

## 📜 Licença

Este projeto está sob a licença MIT.