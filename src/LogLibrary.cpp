#include "LogLibrary.h"

Print *Log::_output = &Serial;
LogLevel Log::_currentLevel = LogLevel::DEBUG;
LogFormat Log::_format = LogFormat::TEXT;
bool Log::_colorsEnabled = true;
bool Log::_timestampEnabled = true;
bool Log::_threadIdEnabled = true;
bool Log::_newlineEnabled = true;
uint16_t Log::_bufferSize = 256;
char *Log::_buffer = nullptr;
bool Log::_showDetails = false;
bool Log::_jsonEscapeEnabled = false;
bool Log::_timeSynced = false;
Preferences Log::_prefs;
Timeval Log::_timeval = {
    .time_zone = "America/Sao_Paulo",
    .ntp_server1 = "pool.ntp.org",
    .ntp_server2 = "a.st1.ntp.br",
    .ntp_server3 = "ntp.cais.rnp.br"};

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
    restoreTimeFromPrefs();
    startNTPAsinc();
#endif
}

void Log::setTimeval(const char *timezone,
                     const char *ntpServer1,
                     const char *ntpServer2,
                     const char *ntpServer3)
{
    _timeval.time_zone = timezone;
    _timeval.ntp_server1 = ntpServer1;
    _timeval.ntp_server2 = ntpServer2;
    _timeval.ntp_server3 = ntpServer3;
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
    if (!_timestampEnabled || !_timeSynced)
        return;

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        _output->print("[Time Not Synced] ");
        return;
    }

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

bool Log::syncTime()
{
    if (!_timeval.time_zone || !_timeval.ntp_server1)
    {
        LOG_ERROR("Timezone ou NTP server não configurado");
        return false;
    }

    configTzTime(_timeval.time_zone,
                 _timeval.ntp_server1,
                 _timeval.ntp_server2,
                 _timeval.ntp_server3);
    // Espera pela sincronização (máximo 10 tentativas)
    int retry = 0;
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo) && retry < 20)
    {
        delay(500);
        retry++;
    }

    _timeSynced = (retry < 20);
    if (_timeSynced)
    {
        saveTimeToPrefs(&timeinfo);
        LOG_INFO("Hora sincronizada: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }
    return _timeSynced;
}

void Log::saveTimeToPrefs(struct tm *timeinfo)
{
    time_t t = mktime(timeinfo); // converte struct tm para time_t
    _prefs.begin("time", false);
    _prefs.putULong64("lastSync", t);
    _prefs.end();
}

void Log::restoreTimeFromPrefs()
{
    _prefs.begin("time", true);
    time_t savedTime = _prefs.getULong64("lastSync", 0);
    _prefs.end();

    if (savedTime > 0)
    {
        struct timeval tv = {.tv_sec = savedTime};
        settimeofday(&tv, nullptr);
        LOG_INFO("Hora restaurada da memória: %lu", savedTime);
    }
}

void timeSyncTask(void *param)
{
    const TickType_t delayTime = pdMS_TO_TICKS(5 * 60 * 1000); // 5 minutos
    while (true)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Log::syncTime();
        }
        vTaskDelay(delayTime);
    }
}

void Log::startNTPAsinc()
{
    // Outras inicializações
    Log::begin(&Serial);

    // Cria a tarefa de sincronização de horário (com prioridade baixa)
    xTaskCreatePinnedToCore(
        timeSyncTask,   // Função da tarefa
        "TimeSyncTask", // Nome da tarefa
        4096,           // Stack size
        nullptr,        // Parâmetro
        1,              // Prioridade
        nullptr,        // Handle
        APP_CPU_NUM     // Core (0 ou 1)
    );
}