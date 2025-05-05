#ifndef LOG_H
#define LOG_H

#include <Arduino.h>
#include <Print.h>
#include <stdarg.h>
#include <time.h>
#ifdef ESP32
#include <NTPSync.h>
#include <WiFi.h>
#include <freertos/task.h>
#include <sys/time.h>
#endif

// Macro helpers para verificação em tempo de compilação
#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4

#ifndef CURRENT_LOG_LEVEL
#define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG
#endif

//----------- Definição dos pinos para a serial alternativa (RX, TX) -------------------
#ifndef UART_RX_PIN
#define UART_RX_PIN 44
#endif

#ifndef UART_TX_PIN
#define UART_TX_PIN 43
#endif
//--------------------------------------------------------------------------------------
//---------------------- Velocidades de comunicação serial -----------------------------
#ifndef MAIN_SERIAL_BAUDRATE
#define MAIN_SERIAL_BAUDRATE 115200
#endif
//--------------------------------------------------------------------------------------

// Macros condicionais
#if CURRENT_LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG(format, ...) Log::log(LogLevel::DEBUG, F("DEBUG"), F(__func__), __FILE__, __LINE__, format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(format, ...) Log::log(LogLevel::INFO, F("INFO"), F(__func__), __FILE__, __LINE__, format, ##__VA_ARGS__)
#else
#define LOG_INFO(format, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_WARNING
#define LOG_WARN(format, ...) Log::log(LogLevel::WARNING, F("WARN"), F(__func__), __FILE__, __LINE__, format, ##__VA_ARGS__)
#else
#define LOG_WARN(format, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR(format, ...) Log::log(LogLevel::ERROR, F("ERROR"), F(__func__), __FILE__, __LINE__, format, ##__VA_ARGS__)
#else
#define LOG_ERROR(format, ...)
#endif

enum class LogLevel : uint8_t
{
    NONE = 0,
    ERROR,
    WARNING,
    INFO,
    DEBUG
};

enum class LogFormat : uint8_t
{
    TEXT = 0,
    JSON
};

class Log
{
public:
    // static Preferences _prefs;

    static void begin(Print *output = &Serial, uint16_t bufferSize = 256);

    static void setLogLevel(LogLevel level);
    static void setFormat(LogFormat format);
    static void enableColors(bool enable);
    static void enableTimestamp(bool enable);
    static void enableThreadId(bool enable);
    static void enableNewline(bool enable);
    static void showDetails(bool show);        // Controla file/line/function
    static void enableJsonEscape(bool enable); // Esconde detalhes extras
    static bool isTimeSynced();
    static bool isUsingInternalClock();

    static void log(LogLevel level,
                    const __FlashStringHelper *tag,
                    const __FlashStringHelper *funcName,
                    const char *file,
                    int line,
                    const char *format, ...);

private:
    static Print *_output;
    static LogLevel _currentLevel;
    static LogFormat _format;
    static bool _colorsEnabled;
    static bool _timestampEnabled;
    static bool _threadIdEnabled;
    static bool _newlineEnabled;
    static uint16_t _bufferSize;
    static char *_buffer;
    static bool _showDetails;
    static bool _jsonEscapeEnabled;
    static bool _timeSynced;
    static uint32_t _bootTime; // Tempo em segundos desde o boot
    static bool _usingInternalClock;

    static const char *getColorCode(LogLevel level);
    static const char *getResetCode();
    static void printThreadId();
    static void printTimestamp();
    static void updateInternalClock();
    static time_t getCurrentTime();
};
#endif