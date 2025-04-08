/**
 * @file main.cpp
 * @brief Exemplo de uso da biblioteca de logs com SoftwareSerial
 *
 * Este programa demonstra como redirecionar a saída de logs para uma porta serial
 * alternativa usando a biblioteca LogLibrary em conjunto com SoftwareSerial.
 *
 * Hardware necessário:
 * - Dispositivo AVR com Arduino (ex: ATmega328P)
 * - Conexão serial alternativa nos pinos 16 (RX) e 17 (TX)
 */

// Biblioteca somente para processadores AVR
#include <Arduino.h>
#include <LogLibrary.h>
#include <SoftwareSerial.h> // Adicionar "featherfly/SoftwareSerial@^1.0" ao platformio.ini

// Definição dos pinos para a serial alternativa (RX, TX)
#define ALT_SERIAL_RX 16
#define ALT_SERIAL_TX 17

// Velocidades de comunicação serial
#define MAIN_SERIAL_BAUDRATE 115200
#define ALT_SERIAL_BAUDRATE 9600

// Tempo de espera entre iterações do loop principal (ms)
#define LOOP_DELAY_MS 2000

// Instância da serial alternativa
SoftwareSerial altSerial(ALT_SERIAL_RX, ALT_SERIAL_TX);

/**
 * @brief Configuração inicial do sistema
 *
 * Esta função é executada uma vez no início do programa e realiza:
 * 1. Inicialização da comunicação serial principal (USB)
 * 2. Inicialização da comunicação serial alternativa
 * 3. Configuração da saída de logs para a serial alternativa
 * 4. Envio de mensagens de teste pelos canais de log
 */
void setup()
{
    // Inicializa a comunicação serial principal (USB)
    Serial.begin(MAIN_SERIAL_BAUDRATE);

    // Inicializa a comunicação serial alternativa
    altSerial.begin(ALT_SERIAL_BAUDRATE);

    // Configura a saída de logs para a serial alternativa
    Log::begin(&altSerial);

    // Mensagens de teste
    LOG_INFO("Inicialização completa - Logs sendo enviados pela SoftwareSerial");
    LOG_DEBUG("Mensagem de debug - Nível de detalhe aumentado");

    // Opcional: Confirmação pela serial principal
    Serial.println("Sistema inicializado. Verifique a serial alternativa para logs.");
}

/**
 * @brief Loop principal do programa
 *
 * Esta função é executada repetidamente após a setup().
 * No exemplo atual, apenas aguarda o tempo definido entre iterações.
 *
 * Em uma aplicação real, aqui seriam colocadas as rotinas principais
 * do sistema, com os devivos logs para monitoramento.
 */
void loop()
{
    // Exemplo simples - apenas aguarda o tempo definido
    delay(LOOP_DELAY_MS);

    // Em uma aplicação real, poderiam ser adicionados logs periódicos:
    // LOG_INFO("Loop principal em execução");
    // LOG_DEBUG("Tempo desde início: %lu ms", millis());
}