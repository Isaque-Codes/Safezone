#ifndef FINGERPRINT_SENSOR_H
#define FINGERPRINT_SENSOR_H

#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

class FingerprintSensor
{
public:
    FingerprintSensor(HardwareSerial *serial, uint32_t password, int rxPin, int txPin);
    bool begin(long baudRate);
    void printMenu();
    void enrollFingerprint();
    void verifyFingerprint();
    void deleteFingerprint();
    void getFingerprintCount();
    bool isAccessGranted();

private:
    Adafruit_Fingerprint _finger;
    HardwareSerial *_mySerial;
    bool _liberacaoAcesso;
    int _rxPin;
    int _txPin;

    uint8_t getFingerprintEnroll();
    uint8_t getFingerprintID();
    uint8_t getFingerprintIDez();
};

#endif // FINGERPRINT_SENSOR_H
