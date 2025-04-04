#ifndef LOG_H
#define LOG_H

#include <Arduino.h>
#include <Print.h>
#include <stdarg.h>

// Macro helpers para verificação em tempo de compilação
#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4

#ifndef CURRENT_LOG_LEVEL
#define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG
#endif

// Macros condicionais
#if CURRENT_LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG(format, ...) Log::log(LogLevel::DEBUG, F("DEBUG"), format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(format, ...) Log::log(LogLevel::INFO, F("INFO"), format, ##__VA_ARGS__)
#else
#define LOG_INFO(format, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_WARNING
#define LOG_WARN(format, ...) Log::log(LogLevel::WARNING, F("WARN"), format, ##__VA_ARGS__)
#else
#define LOG_WARN(format, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR(format, ...) Log::log(LogLevel::ERROR, F("ERROR"), format, ##__VA_ARGS__)
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

class Log
{
public:
    static void begin(Print *output = &Serial, uint16_t bufferSize = 256);
    static void setLogLevel(LogLevel level);
    static void enableColors(bool enable);
    static void enableNewline(bool enable);
    static void log(LogLevel level, const __FlashStringHelper *tag, const char *format, ...);

private:
    static Print *_output;
    static LogLevel _currentLevel;
    static bool _colorsEnabled;
    static bool _newlineEnabled;
    static uint16_t _bufferSize;
    static char *_buffer;

    static const char *getColorCode(LogLevel level);
    static const char *getResetCode();
};

#endif