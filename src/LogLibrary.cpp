#include "LogLibrary.h"

// DEFINA AS VARIÁVEIS ESTÁTICAS NO INÍCIO DO ARQUIVO
Print *LogLibrary::_output = &Serial;
LogLevel LogLibrary::_currentLevel = LogLevel::DEBUG;
LogFormat LogLibrary::_format = LogFormat::TEXT;
uint16_t LogLibrary::_bufferSize = 256;
char *LogLibrary::_buffer = nullptr;

void LogLibrary::begin(Print *output, uint16_t bufferSize)
{
    _output = &Serial; // Ou use o parâmetro output se preferir
    _bufferSize = bufferSize;

    if (_buffer)
    {
        delete[] _buffer;
    }
    _buffer = new char[_bufferSize];
}

void LogLibrary::setLogLevel(LogLevel level) { _currentLevel = level; }

void LogLibrary::setFormat(LogFormat format) { _format = format; }

void LogLibrary::printTimestamp()
{
    time_t _now;
    struct tm timeinfo;

    time(&_now);
    localtime_r(&_now, &timeinfo);

    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    _output->printf("[%s] ", buf);
}

static void escapeJsonString(const char *input, char *output)
{
    while (*input)
    {
        switch (*input)
        {
        case '\"':
            strcat(output, "\\\"");
            break;
        case '\\':
            strcat(output, "\\\\");
            break;
        case '\b':
            strcat(output, "\\b");
            break;
        case '\f':
            strcat(output, "\\f");
            break;
        case '\n':
            strcat(output, "\\n");
            break;
        case '\r':
            strcat(output, "\\r");
            break;
        case '\t':
            strcat(output, "\\t");
            break;
        default:
            if ((uint8_t)*input < 0x20)
            {
                // Caracteres de controle devem ser escapados como unicode
                char buffer[7];
                sprintf(buffer, "\\u%04x", (uint8_t)*input);
                strcat(output, buffer);
            }
            else
            {
                size_t len = strlen(output);
                output[len] = *input;
                output[len + 1] = '\0';
            }
            break;
        }
        input++;
    }
}

void LogLibrary::log(LogLevel level, const __FlashStringHelper *tag,
                     const __FlashStringHelper *funcName, const char *file,
                     int line, const char *format, ...)
{

    if (level > _currentLevel || !_output || !_buffer)
        return;

    va_list args;
    va_start(args, format);
    vsnprintf(_buffer, _bufferSize, format, args);
    va_end(args);

    if (_format == LogFormat::TEXT)
    {
        printTimestamp();
        _output->printf("[%s]", tag);
        _output->printf("[%s:%d][%s]", file, line, funcName);
        _output->print(": ");
        _output->print(_buffer);
    }
    else
    {
        _output->print("{");
        _output->printf("\"timestamp\":%lu,", millis());
        _output->printf("\"level\":\"%s\",", tag);
        _output->printf("\"file\":\"%s\",", file);
        _output->printf("\"line\":%d,", line);
        _output->printf("\"function\":\"%s\",", funcName);

        char jsonMsg[_bufferSize];
        escapeJsonString(_buffer, jsonMsg);
        _output->printf("\"message\":\"%s\"", jsonMsg);
        _output->print("}");
    }

    _output->println();
}