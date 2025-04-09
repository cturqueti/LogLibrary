/**
 * @file main.cpp
 * @brief Exemplo de uso da biblioteca de logs com SoftwareSerial
 *
 * Este programa demonstra como redirecionar a saída de logs para uma porta serial
 * alternativa usando a biblioteca LogLibrary em conjunto com SoftwareSerial.
 *
 * Hardware necessário:
 * - Dispositivo ESP com Arduino (ex: ESP32 S3)
 * - Conexão serial alternativa nos pinos 44 (RX) e 43 (TX)
 */

// Biblioteca somente para processadores AVR
#include <Arduino.h>
#include <LogLibrary.h>

// Definição dos pinos para a serial alternativa (RX, TX)
#define UART_RX_PIN 44
#define UART_TX_PIN 43

// Velocidades de comunicação serial
#define MAIN_SERIAL_BAUDRATE 115200

// Tempo de espera entre iterações do loop principal (ms)
#define LOOP_DELAY_MS 2000

// Instância da serial alternativa
HardwareSerial HardwareSerialPort(0); // UART0

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

    // Inicializa a comunicação serial
    HardwareSerialPort.begin(MAIN_SERIAL_BAUDRATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

    // Configura a saída de logs para a serial alternativa
    Log::begin(&HardwareSerialPort);

    // Mensagens de teste
    LOG_INFO("Inicialização completa - Logs sendo enviados pela HardwareSerial");
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