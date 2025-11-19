#pragma once
#include <Arduino.h>
#include <WebSocketsServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

// Macro helpers para verificação em tempo de compilação
#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_VERBOSE 5

#ifndef CURRENT_LOG_LEVEL
#define CURRENT_LOG_LEVEL LOG_LEVEL_VERBOSE
#endif

//----------- Definição dos pinos para a serial alternativa (RX, TX)
//--------------------
#ifndef UART_RX_PIN
#define UART_RX_PIN 44
#endif

#ifndef UART_TX_PIN
#define UART_TX_PIN 43
#endif
//--------------------------------------------------------------------------------------
//---------------------- Velocidades de comunicação serial
//-----------------------------
#ifndef MAIN_SERIAL_BAUDRATE
#define MAIN_SERIAL_BAUDRATE 115200
#endif
//--------------------------------------------------------------------------------------

// Macros condicionais
#if CURRENT_LOG_LEVEL >= LOG_LEVEL_VERBOSE
#define LOG_VERBOSE(format, ...)                                          \
    LogLibrary::log(LogLevel::DEBUG, F("VERBOSE"), F(__func__), __FILE__, \
                    __LINE__, format, ##__VA_ARGS__)
#else
#define LOG_VERBOSE(format, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG(format, ...)                                          \
    LogLibrary::log(LogLevel::DEBUG, F("DEBUG"), F(__func__), __FILE__, \
                    __LINE__, format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(format, ...)                                         \
    LogLibrary::log(LogLevel::INFO, F("INFO"), F(__func__), __FILE__, \
                    __LINE__, format, ##__VA_ARGS__)
#else
#define LOG_INFO(format, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_WARNING
#define LOG_WARN(format, ...)                                            \
    LogLibrary::log(LogLevel::WARNING, F("WARN"), F(__func__), __FILE__, \
                    __LINE__, format, ##__VA_ARGS__)
#else
#define LOG_WARN(format, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR(format, ...)                                          \
    LogLibrary::log(LogLevel::ERROR, F("ERROR"), F(__func__), __FILE__, \
                    __LINE__, format, ##__VA_ARGS__)
#else
#define LOG_ERROR(format, ...)
#endif

enum class LogLevel : uint8_t
{
    NONE = 0,
    ERROR,
    WARNING,
    INFO,
    DEBUG,
    VERBOSE
};

enum class LogFormat : uint8_t
{
    TEXT = 0,
    JSON
};

class LogLibrary
{
public:
    static LogLibrary &getInstance()
    {
        static LogLibrary _instance;
        return _instance;
    }

    // ADICIONE 'static' AQUI ↓
    static void begin(Print *output = &Serial, uint16_t bufferSize = 256);
    static void setLogLevel(LogLevel level);
    static void setFormat(LogFormat format);

    static void setWebSocket(WebSocketsServer *webSocket);
    static void enableWebSocket(bool enable);

    static void startWebSocketTask();
    static void stopWebSocketTask();
    static bool isWebSocketTaskRunning();

    // ADICIONE 'static' AQUI ↓
    static void log(LogLevel level, const __FlashStringHelper *tag,
                    const __FlashStringHelper *funcName, const char *file,
                    int line, const char *format, ...);

private:
    // TORNE ESTES MEMBROS ESTÁTICOS TAMBÉM ↓
    static Print *_output;
    static LogLevel _currentLevel;
    static LogFormat _format;
    static uint16_t _bufferSize;
    static char *_buffer;

    static WebSocketsServer *_webSocket;
    static bool _webSocketEnabled;

    static TaskHandle_t _webSocketTaskHandle;
    static bool _webSocketTaskRunning;
    static QueueHandle_t _logQueue;
    static const int QUEUE_SIZE = 20;

    struct LogMessage
    {
        LogLevel level;
        char tag[16];
        char funcName[32];
        char file[32];
        int line;
        char message[256];
    };

    LogLibrary() {}; // Construtor privado para singleton

    static void printTimestamp();

    static void sendToWebSocket(LogLevel level, const char *tag,
                                const char *funcName, const char *file,
                                int line, const char *message);

    static void escapeJsonString(const char *input, char *output);
    static const char *getTimestampString();

    static void webSocketTask(void *parameter);
    static void initializeWebServer();
    static void handleWebSocketClients();
};