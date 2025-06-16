#include "sensorDeDigitais.h"
#include <Arduino.h>

FingerprintSensor::FingerprintSensor(HardwareSerial *serial, uint32_t password, int rxPin, int txPin)
    : _finger(serial, password), _mySerial(serial), _rxPin(rxPin), _txPin(txPin), _liberacaoAcesso(false)
{
}

bool FingerprintSensor::begin(long baudRate)
{
    _mySerial->begin(baudRate, SERIAL_8N1, _rxPin, _txPin);
    Serial.begin(9600);
    while (!Serial)
    {
        delay(100);
    }
    Serial.println("Iniciando o sistema de impressao digital...");

    if (_finger.verifyPassword())
    {
        Serial.println("Sensor de impressao digital encontrado!");
    }
    else
    {
        Serial.println("Sensor de impressao digital nao encontrado :(");
        Serial.println("Verifique as conexoes e a baud rate.");
        return false;
    }

    _finger.getParameters();
    Serial.print("Capacidade de armazenamento: ");
    Serial.println(_finger.capacity);
    Serial.print("Nivel de seguranca: ");
    Serial.println(_finger.security_level);

    Serial.println("Sistema pronto. Escolha uma opcao:");
    printMenu();
    return true;
}

void FingerprintSensor::printMenu()
{
    Serial.println("----------------------------------------");
    Serial.println("1. Cadastrar nova impressao digital");
    Serial.println("2. Verificar impressao digital");
    Serial.println("3. Excluir impressao digital");
    Serial.println("4. Contar impressoes digitais cadastradas");
    Serial.println("m. Mostrar menu novamente");
    Serial.println("----------------------------------------");
    Serial.print("Sua escolha: ");
}

void FingerprintSensor::enrollFingerprint()
{
    int id;
    Serial.print("Digite o ID para a nova impressao digital (1 a ");
    Serial.print(_finger.capacity);
    Serial.print("): ");
    while (!Serial.available())
        delay(100);
    id = Serial.parseInt();
    Serial.readStringUntil('\n');

    if (id < 1 || id > _finger.capacity)
    {
        Serial.println("ID invalido. Tente novamente.");
        return;
    }

    Serial.println("Coloque o dedo no sensor para cadastrar...");
    Serial.println("Aguardando o dedo...");

    uint8_t p = getFingerprintEnroll();
    if (p == FINGERPRINT_OK)
    {
        p = _finger.storeModel(id);
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.print("Impressao digital cadastrada com sucesso no ID: ");
            Serial.println(id);
            break;
        case FINGERPRINT_BADLOCATION:
            Serial.println("ID de localizacao invalido.");
            break;
        case FINGERPRINT_FLASHERR:
            Serial.println("Erro de escrita na flash.");
            break;
        default:
            Serial.print("Erro storeModel(): ");
            Serial.println(p);
            break;
        }
    }
    else
    {
        Serial.print("Erro no cadastro: ");
        Serial.println(p);
    }
    Serial.println("Sistema pronto. Escolha uma opcao:");
    printMenu();
}

void FingerprintSensor::verifyFingerprint()
{
    Serial.println("Coloque o dedo no sensor para verificar...");
    Serial.println("Aguardando o dedo...");

    _liberacaoAcesso = false;
    uint8_t p = getFingerprintIDez();
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Impressao digital encontrada!");
        Serial.print("ID: ");
        Serial.print(_finger.fingerID);
        Serial.print(" | Confianca: ");
        Serial.println(_finger.confidence);
        _liberacaoAcesso = true;
    }
    else if (p == FINGERPRINT_NOFINGER)
    {
        Serial.println("Nenhum dedo detectado.");
    }
    else if (p == FINGERPRINT_NOTFOUND)
    {
        Serial.println("Impressao digital nao encontrada no banco de dados.");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Erro de comunicacao.");
    }
    else
    {
        Serial.print("Erro desconhecido: ");
        Serial.println(p);
    }
    Serial.println("Sistema pronto. Escolha uma opcao:");
    printMenu();
}

void FingerprintSensor::deleteFingerprint()
{
    int id;
    Serial.print("Digite o ID da impressao digital a ser excluida: ");
    while (!Serial.available())
        delay(100);
    id = Serial.parseInt();
    Serial.readStringUntil('\n');

    if (id < 1 || id > _finger.capacity)
    {
        Serial.println("ID invalido. Tente novamente.");
        return;
    }

    Serial.print("Excluindo impressao digital com ID: ");
    Serial.println(id);

    uint8_t p = _finger.deleteModel(id);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Impressao digital excluida com sucesso!");
        break;
    case FINGERPRINT_BADLOCATION:
        Serial.println("ID de localizacao invalido.");
        break;
    case FINGERPRINT_FLASHERR:
        Serial.println("Erro de escrita na flash.");
        break;
    default:
        Serial.print("Erro deleteModel(): ");
        Serial.println(p);
        break;
    }
    Serial.println("Sistema pronto. Escolha uma opcao:");
    printMenu();
}

void FingerprintSensor::getFingerprintCount()
{
    uint8_t p = _finger.getTemplateCount();
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.print("Numero de impressoes digitais cadastradas: ");
        Serial.println(_finger.templateCount);
        break;
    default:
        Serial.print("Erro getTemplateCount(): ");
        Serial.println(p);
        break;
    }
    Serial.println("Sistema pronto. Escolha uma opcao:");
    printMenu();
}

bool FingerprintSensor::isAccessGranted()
{
    return _liberacaoAcesso;
}

uint8_t FingerprintSensor::getFingerprintEnroll()
{
    uint8_t p = -1;
    Serial.println("Aguardando o dedo...");
    while (p != FINGERPRINT_OK)
    {
        p = _finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Imagem capturada.");
            break;
        case FINGERPRINT_NOFINGER:
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Erro de comunicacao.");
            return p;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Erro na captura da imagem.");
            return p;
        default:
            Serial.print("Erro desconhecido: ");
            Serial.println(p);
            return p;
        }
    }

    p = _finger.image2Tz(1);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Imagem convertida para template 1.");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Imagem muito suja. Tente novamente.");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Nao foi possivel extrair caracteristicas. Tente novamente.");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Imagem invalida. Tente novamente.");
        return p;
    default:
        Serial.print("Erro image2Tz(1): ");
        Serial.println(p);
        return p;
    }

    Serial.println("Retire o dedo do sensor.");
    delay(2000);
    p = -1;
    while (p != FINGERPRINT_NOFINGER)
    {
        p = _finger.getImage();
        delay(100);
    }
    Serial.println("Dedo removido.");

    Serial.println("Coloque o MESMO dedo novamente no sensor...");
    Serial.println("Aguardando o dedo...");

    p = -1;
    while (p != FINGERPRINT_OK)
    {
        p = _finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Segunda imagem capturada.");
            break;
        case FINGERPRINT_NOFINGER:
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Erro de comunicacao.");
            return p;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Erro na captura da segunda imagem.");
            return p;
        default:
            Serial.print("Erro desconhecido: ");
            Serial.println(p);
            return p;
        }
    }

    p = _finger.image2Tz(2);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Segunda imagem convertida para template 2.");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Segunda imagem muito suja. Tente novamente.");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Nao foi possivel extrair caracteristicas da segunda imagem. Tente novamente.");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Segunda imagem invalida. Tente novamente.");
        return p;
    default:
        Serial.print("Erro image2Tz(2): ");
        Serial.println(p);
        return p;
    }

    p = _finger.createModel();
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Modelos combinados.");
        break;
    case FINGERPRINT_ENROLLMISMATCH:
        Serial.println("As impressoes digitais nao correspondem. Tente novamente.");
        return p;
    default:
        Serial.print("Erro createModel(): ");
        Serial.println(p);
        return p;
    }
    return FINGERPRINT_OK;
}

uint8_t FingerprintSensor::getFingerprintID()
{
    uint8_t p = _finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Imagem capturada.");
        break;
    case FINGERPRINT_NOFINGER:
        Serial.println("Nenhum dedo detectado.");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Erro de comunicacao.");
        return p;
    case FINGERPRINT_IMAGEFAIL:
        Serial.println("Erro na captura da imagem.");
        return p;
    default:
        Serial.print("Erro desconhecido: ");
        Serial.println(p);
        return p;
    }

    p = _finger.image2Tz(1);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Imagem convertida para template.");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Imagem muito suja. Tente novamente.");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Nao foi possivel extrair caracteristicas. Tente novamente.");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Imagem invalida. Tente novamente.");
        return p;
    default:
        Serial.print("Erro image2Tz(1): ");
        Serial.println(p);
        return p;
    }

    p = _finger.fingerFastSearch();
    if (p == FINGERPRINT_OK)
    {
        return FINGERPRINT_OK;
    }
    else if (p == FINGERPRINT_NOTFOUND)
    {
        return p;
    }
    else
    {
        return p;
    }
}

uint8_t FingerprintSensor::getFingerprintIDez()
{
    uint8_t p = -1;
    while (p != FINGERPRINT_OK)
    {
        p = _finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Imagem capturada.");
            break;
        case FINGERPRINT_NOFINGER:
            // Serial.println("Nenhum dedo detectado."); // Descomente para depuracao
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Erro de comunicacao.");
            return p;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Erro na captura da imagem.");
            return p;
        default:
            Serial.print("Erro desconhecido: ");
            Serial.println(p);
            return p;
        }
    }

    p = _finger.image2Tz(1);
    if (p != FINGERPRINT_OK)
        return p;

    p = _finger.fingerFastSearch();
    return p;
}
