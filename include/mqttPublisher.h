#ifndef MQTTPUBLISHER_H
#define MQTTPUBLISHER_H

#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// Forward declaration para a função de callback
void mqttCallback(char *topic, byte *payload, unsigned int length);

class MqttPublisher
{
public:
    MqttPublisher(const char *mqttServer, int mqttPort, const char *mqttId, const char *pubTopic, const char *subTopic);
    void setup();
    void loop();
    bool publishSensorReadings(float luz, float movimento, float pressao); // Alterado para float para flexibilidade
    bool publishAccessAttempt(bool success, int userId);
    bool publishAlarmAlert(bool pressureAlarm, bool motionAlarm, bool lightAlarm, bool generalAlarm);
    void setRemoteAccessCallback(void (*callback)(bool, int)); // Callback para liberação remota

    // Funções auxiliares para tratamento de mensagens recebidas
    void handleIncomingMessage(String &message);

private:
    WiFiClient _espClient;
    PubSubClient _client;
    const char *_mqttServer;
    int _mqttPort;
    const char *_mqttId;
    const char *_mqttTopicPub;
    const char *_mqttTopicSub;
    unsigned long _lastSensorPublishTime = 0;
    StaticJsonDocument<256> _lastSensorReadings;
    void reconnect();
    void (*_remoteAccessCallback)(bool, int);

    void handleRemoteAccessCommand(JsonDocument &doc);
};

#endif // MQTTPUBLISHER_H
