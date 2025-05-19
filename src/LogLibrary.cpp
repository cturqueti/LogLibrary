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
uint32_t Log::_bootTime = 0;
bool Log::_usingInternalClock = true;

void Log::begin(Print *output, uint16_t bufferSize)
{
    _output = output ? output : &Serial;
    _bufferSize = bufferSize;

    if (_buffer)
    {
        delete[] _buffer;
    }
    _buffer = new char[_bufferSize];

    // Inicializa o clock interno
    _bootTime = 0;
    _usingInternalClock = true;

#ifdef ESP32
    // Configura NTP apenas no ESP32
    NTPSync::setTimeval(
        "America/Sao_Paulo", {"time.cloudflare.com", // Alternativa 1
                              "a.st1.ntp.br",        // Alternativa 2
                              "time.windows.com"}    // Alternativa 3
    );
    NTPSync::logControl(false);
    NTPSync::begin(1, 1);

    // Tenta sincronizar imediatamente
    if (NTPSync::syncTime())
    {
        _usingInternalClock = false;
    }

#endif
}

void Log::updateInternalClock()
{
    // Atualiza o tempo interno baseado em millis()
    static uint32_t lastUpdate = 0;
    uint32_t now = millis();

    if (now - lastUpdate >= 1000)
    {
        _bootTime += (now - lastUpdate) / 1000;
        lastUpdate = now;
    }
}

time_t Log::getCurrentTime()
{
#ifdef ESP32
    if (NTPSync::isTimeSynced())
    {
        _usingInternalClock = false;
    }

    if (!_usingInternalClock && NTPSync::isTimeSynced())
    {
        return NTPSync::getLastTimeSync();
    }

#endif
    updateInternalClock();
    return _bootTime;
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
    time_t now = getCurrentTime();
    struct tm timeinfo;

#ifdef ESP32
    if (!_usingInternalClock)
    {
        localtime_r(&now, &timeinfo);
    }
    else
    {
        // Para clock interno, começa em 00:00:00
        gmtime_r(&now, &timeinfo);
    }
#else
    // Para outras plataformas, usa clock interno
    gmtime_r(&now, &timeinfo);
#endif
    char buf[20];

    if (_usingInternalClock)
    {
        strftime(buf, sizeof(buf), " %m-%d %H:%M:%S", &timeinfo);
        _output->printf("[INT:%s] ", buf); // Indica que é clock interno
    }
    else
    {
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
        _output->printf("[%s] ", buf);
    }
}

void Log::printThreadId()
{
    if (!_threadIdEnabled)
        return;

#ifdef ESP32
    _output->printf("[T:%p] ", xTaskGetCurrentTaskHandle());
#endif
    return;
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
    if (_currentLevel == LogLevel::VERBOSE)
    {
        _showDetails = true;
        _threadIdEnabled = true;
        _timestampEnabled = true;
    }
    else
    {
        _showDetails = false;
        _threadIdEnabled = false;
        _timestampEnabled = false;
    }

    return;
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

bool Log::isUsingInternalClock()
{
    return _usingInternalClock;
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
