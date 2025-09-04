#include "sensorDeDigitais.h"
#include <Arduino.h>

// --- Construtor: Inicializa os objetos e variaveis da classe ---
FingerprintSensor::FingerprintSensor(HardwareSerial *serial, uint32_t password, int rxPin, int txPin)
    : _finger(serial, password), _mySerial(serial), _rxPin(rxPin), _txPin(txPin), _liberacaoAcesso(false)
{
    // O construtor usa uma lista de inicializacao para configurar os membros da classe.
}

// --- Inicializa o sensor e verifica a comunicacao ---
bool FingerprintSensor::begin(long baudRate)
{
    _mySerial->begin(baudRate, SERIAL_8N1, _rxPin, _txPin);
    Serial.begin(9600);
    while (!Serial)
    {
        delay(100); // Espera o monitor serial ficar pronto.
    }
    Serial.println("Iniciando o sistema de impressao digital...");

    // Tenta se comunicar com o sensor usando a senha.
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

    // Pega informacoes basicas do sensor, como capacidade de armazenamento.
    _finger.getParameters();
    Serial.print("Capacidade de armazenamento: ");
    Serial.println(_finger.capacity);
    Serial.print("Nivel de seguranca: ");
    Serial.println(_finger.security_level);

    Serial.println("Sistema pronto. Escolha uma opcao:");
    printMenu();
    return true;
}

// --- Apenas imprime o menu de opcoes no monitor serial ---
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

// --- Fluxo principal para cadastrar uma nova digital ---
void FingerprintSensor::enrollFingerprint()
{
    int id;
    // Pede ao usuario um ID para salvar a digital.
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

    // Chama a funcao que faz o processo de leitura em duas etapas.
    uint8_t p = getFingerprintEnroll();
    if (p == FINGERPRINT_OK)
    {
        // Se a leitura foi bem sucedida, salva o modelo na memoria do sensor.
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

// --- Verifica se a digital apresentada existe no banco de dados ---
void FingerprintSensor::verifyFingerprint()
{
    Serial.println("Coloque o dedo no sensor para verificar...");
    Serial.println("Aguardando o dedo...");

    _liberacaoAcesso = false; // Reseta a permissao antes de cada nova verificacao.

    // Chama a funcao de busca rapida.
    uint8_t p = getFingerprintIDez();
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Impressao digital encontrada!");
        Serial.print("ID: ");
        Serial.print(_finger.fingerID);
        Serial.print(" | Confianca: ");
        Serial.println(_finger.confidence);
        _liberacaoAcesso = true; // Permite o acesso se a digital for encontrada.
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

// --- Apaga uma digital especifica da memoria do sensor ---
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

    // Manda o comando de exclusao para o sensor.
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

// --- Pede ao sensor para contar quantas digitais estao salvas ---
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

// --- Funcao publica para o main.cpp saber se o acesso foi liberado ---
bool FingerprintSensor::isAccessGranted()
{
    return _liberacaoAcesso;
}

// --- Funcao interna que faz o processo de leitura em duas etapas para o cadastro ---
uint8_t FingerprintSensor::getFingerprintEnroll()
{
    // --- ETAPA 1: Captura da primeira imagem ---
    uint8_t p = -1;
    Serial.println("Aguardando o dedo...");
    while (p != FINGERPRINT_OK)
    {
        p = _finger.getImage(); // Pede uma imagem ao sensor.
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Imagem capturada.");
            break;
        case FINGERPRINT_NOFINGER:
            break; // Continua tentando se nao houver dedo.
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

    // Converte a imagem em um "template", um formato de dados que o sensor entende.
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

    // Pede ao usuario para remover o dedo, para garantir duas leituras diferentes.
    Serial.println("Retire o dedo do sensor.");
    delay(2000);
    p = -1;
    while (p != FINGERPRINT_NOFINGER)
    {
        p = _finger.getImage();
        delay(100);
    }
    Serial.println("Dedo removido.");

    // --- ETAPA 2: Captura da segunda imagem (do mesmo dedo) ---
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

    // Converte a segunda imagem em um segundo template.
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

    // Pede ao sensor para combinar os dois templates em um modelo final mais preciso.
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
    return FINGERPRINT_OK; // Retorna sucesso se tudo deu certo.
}

// --- Funcao de busca mais simples, nao usada no fluxo principal, mas util para testes ---
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

    // Converte a imagem em template.
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

    // Procura o template no banco de dados.
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

// --- Funcao otimizada para busca: captura a imagem e procura no banco de dados ---
uint8_t FingerprintSensor::getFingerprintIDez()
{
    uint8_t p = -1;
    // Fica em loop ate conseguir uma imagem valida do dedo.
    while (p != FINGERPRINT_OK)
    {
        p = _finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            // Serial.println("Imagem capturada."); // Comentado para nao poluir a serial na operacao normal.
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

    // Converte a imagem em template.
    p = _finger.image2Tz(1);
    if (p != FINGERPRINT_OK)
        return p;

    // Executa a busca rapida do template no banco de dados do sensor.
    p = _finger.fingerFastSearch();
    return p;
}