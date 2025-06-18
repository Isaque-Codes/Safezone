#include "mqttPublisher.h"
#include <Arduino.h>
#include <ezTime.h>

// Variável global para a instância do MqttPublisher
// Isso é necessário para que a função de callback do PubSubClient possa acessar os métodos da classe.
// É uma solução comum em ambientes embarcados onde callbacks C++ não podem ser métodos de classe diretamente.
MqttPublisher *globalMqttPublisher = nullptr;

// Função de callback do PubSubClient
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    if (globalMqttPublisher)
    {
        String message = "";
        for (unsigned int i = 0; i < length; i++)
        {
            message += (char)payload[i];
        }
        globalMqttPublisher->handleIncomingMessage(message);
    }
}

MqttPublisher::MqttPublisher(const char *mqttServer, int mqttPort, const char *mqttId, const char *pubTopic, const char *subTopic)
    : _client(_espClient), _mqttServer(mqttServer), _mqttPort(mqttPort), _mqttId(mqttId), _mqttTopicPub(pubTopic), _mqttTopicSub(subTopic), _remoteAccessCallback(nullptr)
{
    _client.setServer(_mqttServer, _mqttPort);
    _client.setCallback(mqttCallback);
    globalMqttPublisher = this; // Atribui a instância atual à variável global
}

void MqttPublisher::setup()
{
    // A conexão WiFi e ezTime.waitForSync() devem ser chamados no setup() principal do Arduino
    // Serial.begin(9600); // Já deve ser chamado no setup() principal
    // conectaWiFi(); // Já deve ser chamado no setup() principal
    // waitForSync(); // Já deve ser chamado no setup() principal

    reconnect();
}

void MqttPublisher::loop()
{
    if (!_client.connected())
    {
        reconnect();
    }
    _client.loop();
}

void MqttPublisher::reconnect()
{
    while (!_client.connected())
    {
        Serial.println("Conectando ao MQTT...");
        if (_client.connect(_mqttId))
        {
            Serial.println("Conectado com sucesso");
            _client.subscribe(_mqttTopicSub);
        }
        else
        {
            Serial.print("falha, rc=");
            Serial.print(_client.state());
            Serial.println(" tentando novamente em 5 segundos");
            delay(5000);
        }
    }
}

bool MqttPublisher::publishSensorReadings(float luz, float movimento, float pressao)
{
    // Verifica se passaram 3 segundos ou se houve mudança significativa
    unsigned long agora = millis();
    bool mudou = false;

    // Verifica se os valores atuais são diferentes dos últimos publicados
    if (_lastSensorReadings.containsKey("luz") && _lastSensorReadings["luz"] != luz)
        mudou = true;
    if (_lastSensorReadings.containsKey("movimento") && _lastSensorReadings["movimento"] != movimento)
        mudou = true;
    if (_lastSensorReadings.containsKey("pressao") && _lastSensorReadings["pressao"] != pressao)
        mudou = true;

    if ((agora - _lastSensorPublishTime > 3000) || mudou)
    {
        StaticJsonDocument<512> mensagem;
        mensagem["timestamp"] = now(); // ezTime now()
        mensagem["tipo"] = "leituraSensores";
        mensagem["leituras"]["luz"] = luz;
        mensagem["leituras"]["movimento"] = movimento;
        mensagem["leituras"]["pressao"] = pressao;

        String payload;
        serializeJson(mensagem, payload);
        bool published = _client.publish(_mqttTopicPub, payload.c_str());

        if (published)
        {
            _lastSensorReadings["luz"] = luz;
            _lastSensorReadings["movimento"] = movimento;
            _lastSensorReadings["pressao"] = pressao;
            _lastSensorPublishTime = agora;
        }
        return published;
    }
    return false;
}

bool MqttPublisher::publishAccessAttempt(bool success, int userId)
{
    StaticJsonDocument<256> mensagem;
    mensagem["timestamp"] = now(); // ezTime now()
    mensagem["tipo"] = "tentativaAcesso";
    mensagem["resultado"] = success ? "sucesso" : "falha";
    mensagem["idUsuario"] = userId;

    String payload;
    serializeJson(mensagem, payload);
    return _client.publish(_mqttTopicPub, payload.c_str());
}

bool MqttPublisher::publishAlarmAlert(bool pressureAlarm, bool motionAlarm, bool lightAlarm, bool generalAlarm)
{
    StaticJsonDocument<256> mensagem;
    mensagem["timestamp"] = now(); // ezTime now()
    mensagem["tipo"] = "alertaInvasao";

    JsonArray sensores = mensagem.createNestedArray("sensoresAtivados");
    if (pressureAlarm)
        sensores.add("pressao");
    if (motionAlarm)
        sensores.add("movimento");
    if (lightAlarm)
        sensores.add("luz");

    mensagem["dispararAlarmeGeral"] = generalAlarm;

    String payload;
    serializeJson(mensagem, payload);
    return _client.publish(_mqttTopicPub, payload.c_str());
}

void MqttPublisher::setRemoteAccessCallback(void (*callback)(bool, int))
{
    _remoteAccessCallback = callback;
}

void MqttPublisher::handleIncomingMessage(String &message)
{
    Serial.print("Mensagem MQTT recebida: ");
    Serial.println(message);

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    String commandType = doc["comando"].as<String>();

    if (commandType == "liberar")
    {
        handleRemoteAccessCommand(doc);
    }
    else
    {
        Serial.println("Comando MQTT desconhecido.");
    }
}

void MqttPublisher::handleRemoteAccessCommand(JsonDocument &doc)
{
    bool liberar = doc["comando"].as<String>() == "liberar";
    int duracao = doc["duracao_segundos"] | 0; // Default to 0 if not present

    if (_remoteAccessCallback)
    {
        _remoteAccessCallback(liberar, duracao);
    }
}
