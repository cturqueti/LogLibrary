#include "LogLibrary.h"

Print *Log::_output = &Serial;
LogLevel Log::_currentLevel = LogLevel::DEBUG;
LogFormat Log::_format = LogFormat::TEXT;
bool Log::_colorsEnabled = false;
bool Log::_timestampEnabled = true;
bool Log::_threadIdEnabled = true;
bool Log::_newlineEnabled = true;
uint16_t Log::_bufferSize = 256;
char *Log::_buffer = nullptr;
bool Log::_showDetails = false;
bool Log::_jsonEscapeEnabled = false;
bool Log::_timeSynced = false;
Preferences Log::_prefs;

void Log::begin(Print *output, uint16_t bufferSize)
{
    _output = output ? output : &Serial;
    _bufferSize = bufferSize;

    if (_buffer)
    {
        delete[] _buffer;
    }
    _buffer = new char[_bufferSize];

// Configura o timestamp (ESP32)
#ifdef ESP32

    // udp.begin(123);
    // restoreTimeFromPrefs();
    NTPSync::setTimeval(
        "America/Sao_Paulo", {"time.cloudflare.com", // Alternativa 1
                              "a.st1.ntp.br",        // Alternativa 2
                              "time.windows.com"}    // Alternativa 3
    );
    NTPSync::logControl(false);
    NTPSync::begin(1, 1);

#endif
}

const char *Log::getColorCode(LogLevel level)
{
    if (!_colorsEnabled)
        return "";

    switch (level)
    {
    case LogLevel::DEBUG:
        return "\033[0;32m"; // Verde
    case LogLevel::INFO:
        return "\033[0;36m"; // Ciano
    case LogLevel::WARNING:
        return "\033[0;33m"; // Amarelo
    case LogLevel::ERROR:
        return "\033[0;31m"; // Vermelho
    default:
        return "";
    }
}

const char *Log::getResetCode()
{
    return _colorsEnabled ? "\033[0m" : "";
}

void Log::printTimestamp()
{
    if (!NTPSync::hasTimeval)
    {
        // LOG_WARN("Time not synced");
        Serial0.println("Dont have time");
        return;
    }

    time_t now = time(nullptr);

    // Converter para estrutura tm (formato de tempo local)
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    _output->printf("[%s] ", buf);
}

void Log::printThreadId()
{
    if (!_threadIdEnabled)
        return;

#ifdef ESP32
    _output->printf("[T:%p] ", xTaskGetCurrentTaskHandle());
#endif
}

void escapeJsonString(const char *input, char *output)
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

void Log::setLogLevel(LogLevel level)
{
    _currentLevel = level;
}

void Log::setFormat(LogFormat format)
{
    _format = format;
}

void Log::enableColors(bool enable)
{
    _colorsEnabled = enable;
}

void Log::enableTimestamp(bool enable)
{
    _timestampEnabled = enable;
}

void Log::enableThreadId(bool enable)
{
    _threadIdEnabled = enable;
}

void Log::enableNewline(bool enable)
{
    _newlineEnabled = enable;
}

void Log::showDetails(bool show)
{
    _showDetails = show;
}

void Log::enableJsonEscape(bool enable)
{
    _jsonEscapeEnabled = enable;
}

bool Log::isTimeSynced()
{
    return _timeSynced;
}

void Log::log(LogLevel level,
              const __FlashStringHelper *tag,
              const __FlashStringHelper *funcName,
              const char *file,
              int line,
              const char *format, ...)
{

    if (level > _currentLevel || !_output || !_buffer)
        return;

    va_list args;
    va_start(args, format);
    vsnprintf(_buffer, _bufferSize, format, args);
    va_end(args);

    if (_format == LogFormat::TEXT)
    {
        _output->print(getColorCode(level));
        printTimestamp();
        printThreadId();
        _output->printf("[%s]", tag);

        if (_showDetails)
        { // Nova flag independente
            _output->printf("[%s:%d][%s]", file, line, funcName);
        }
        _output->print(": ");
        _output->print(_buffer);
        _output->print(getResetCode());
    }
    else
    {
        _output->print("{");
        _output->printf("\"timestamp\":%lu,", millis());
        _output->printf("\"level\":\"%s\",", tag);

        if (_showDetails)
        {
            _output->printf("\"file\":\"%s\",", file);
            _output->printf("\"line\":%d,", line);
            _output->printf("\"function\":\"%s\",", funcName);
        }

        // Buffer com escape para JSON
        char jsonMsg[_bufferSize];
        escapeJsonString(_buffer, jsonMsg);
        _output->printf("\"message\":\"%s\"", jsonMsg);
        _output->print("}");
    }

    if (_newlineEnabled)
    {
        _output->println();
    }
}
