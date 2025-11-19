#include "LogLibrary.h"
#include "Terminal.h"
#include <WebServer.h>
#include <WiFi.h>

// DEFINA AS VARIÁVEIS ESTÁTICAS NO INÍCIO DO ARQUIVO
Print *LogLibrary::_output = &Serial;
LogLevel LogLibrary::_currentLevel = LogLevel::DEBUG;
LogFormat LogLibrary::_format = LogFormat::TEXT;
uint16_t LogLibrary::_bufferSize = 256;
char *LogLibrary::_buffer = nullptr;

// Inicializar variáveis do WebSocket
WebSocketsServer *LogLibrary::_webSocket = nullptr;
bool LogLibrary::_webSocketEnabled = false;

// Variáveis FreeRTOS
TaskHandle_t LogLibrary::_webSocketTaskHandle = nullptr;
bool LogLibrary::_webSocketTaskRunning = false;
QueueHandle_t LogLibrary::_logQueue = nullptr;

// Buffer estático para timestamp
static char timestampBuffer[64];

static WebServer server(80);
static WebSocketsServer webSocket(81);

void LogLibrary::begin(Print *output, uint16_t bufferSize)
{
    _output = output ? output : &Serial;
    _bufferSize = bufferSize;

    if (_buffer)
    {
        delete[] _buffer;
    }
    _buffer = new char[_bufferSize];

    // Inicializar o buffer com zeros
    if (_buffer)
    {
        memset(_buffer, 0, _bufferSize);
    }

    // CORREÇÃO: Verificar se a queue foi criada com sucesso
    _logQueue = xQueueCreate(QUEUE_SIZE, sizeof(LogMessage));
    if (_logQueue == NULL)
    {
        Serial.println("ERRO: Não foi possível criar a queue!");
        return;
    }

    startWebSocketTask();
}

void LogLibrary::startWebSocketTask()
{
    if (_webSocketTaskRunning)
        return;

    xTaskCreatePinnedToCore(
        webSocketTask,         // Função da task
        "WebSocketTask",       // Nome da task
        8192,                  // Stack size
        NULL,                  // Parâmetros
        1,                     // Prioridade
        &_webSocketTaskHandle, // Handle da task
        1                      // Core (0 ou 1)
    );

    _webSocketTaskRunning = true;
    LOG_INFO("Task WebSocket iniciada no core %d", xPortGetCoreID());
}

void LogLibrary::stopWebSocketTask()
{
    if (_webSocketTaskHandle)
    {
        _webSocketTaskRunning = false;
        vTaskDelete(_webSocketTaskHandle);
        _webSocketTaskHandle = nullptr;
    }
}

bool LogLibrary::isWebSocketTaskRunning()
{
    return _webSocketTaskRunning;
}

void LogLibrary::webSocketTask(void *parameter)
{
    LOG_INFO("Task WebSocket iniciada - Aguardando WiFi...");

    // Aguardar conexão WiFi
    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        LOG_DEBUG("Aguardando WiFi... Status: %d", WiFi.status());
    }

    LOG_INFO("WiFi conectado! Iniciando servidores...");

    // Inicializar servidores
    initializeWebServer();

    // Loop principal da task
    while (_webSocketTaskRunning)
    {
        handleWebSocketClients();

        // CORREÇÃO: Processar mensagens da queue
        if (_logQueue)
        {
            LogMessage logMsg;
            while (xQueueReceive(_logQueue, &logMsg, 0) == pdTRUE)
            {
                // CORREÇÃO: Chamar sendToWebSocket apenas se WebSocket estiver habilitado
                if (_webSocketEnabled && _webSocket)
                {
                    sendToWebSocket(logMsg.level, logMsg.tag, logMsg.funcName,
                                    logMsg.file, logMsg.line, logMsg.message);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Limpeza ao finalizar
    webSocket.close();
    server.close();
    LOG_INFO("Task WebSocket finalizada");
    vTaskDelete(NULL);
}

void LogLibrary::initializeWebServer()
{
    // Configurar rotas do servidor
    server.on("/", []()
              { server.send(200, "text/html", "<html><body><h1>ESP32 Terminal</h1><a href='/terminal'>Ir para Terminal</a></body></html>"); });

    server.on("/terminal", []()
              { server.send(200, "text/html", htmlPage); });

    server.on("/status", []()
              {
        String status = "{\"wifi_connected\":";
        status += (WiFi.status() == WL_CONNECTED) ? "true" : "false";
        status += ",\"clients_count\":";
        status += webSocket.connectedClients();
        status += "}";
        server.send(200, "application/json", status); });

    // Configurar WebSocket
    webSocket.begin();
    webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
                      {
        switch (type) {
            case WStype_DISCONNECTED:
                LOG_DEBUG("Cliente %u desconectado", num);
                break;
            case WStype_CONNECTED: {
                IPAddress ip = webSocket.remoteIP(num);
                LOG_DEBUG("Cliente %u conectado de %s", num, ip.toString().c_str());
                webSocket.sendTXT(num, "Bem-vindo ao terminal de logs!");
                break;
            }
            case WStype_TEXT:
                LOG_DEBUG("Comando recebido: %s", payload);
                break;
        } });

    // Iniciar servidores
    server.begin();
    _webSocket = &webSocket;
    _webSocketEnabled = true;

    LOG_INFO("Servidores iniciados - Web:80, WS:81");
    LOG_INFO("Acesse: http://%s/terminal", WiFi.localIP().toString().c_str());
}

void LogLibrary::setWebSocket(WebSocketsServer *webSocket)
{
    _webSocket = webSocket;
}

void LogLibrary::enableWebSocket(bool enable)
{
    _webSocketEnabled = enable;
}

void LogLibrary::handleWebSocketClients()
{
    webSocket.loop();
    server.handleClient();
}

void LogLibrary::setLogLevel(LogLevel level) { _currentLevel = level; }

void LogLibrary::setFormat(LogFormat format) { _format = format; }

void LogLibrary::printTimestamp()
{
    _output->printf("[%s] ", getTimestampString());
}

const char *LogLibrary::getTimestampString()
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(timestampBuffer, sizeof(timestampBuffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return timestampBuffer;
}

void LogLibrary::sendToWebSocket(LogLevel level, const char *tag,
                                 const char *funcName, const char *file,
                                 int line, const char *message)
{
    if (!_webSocketEnabled || !_webSocket || !_webSocket->connectedClients())
    {
        return;
    }

    char wsBuffer[512];

    if (_format == LogFormat::TEXT)
    {
        Serial.printf("[WEB SOCKET DEBUG] Mensagem ORIGINAL: %s\n", message);

        // Substituir emojis por códigos Unicode
        String processedMessage = String(message);

        snprintf(wsBuffer, sizeof(wsBuffer),
                 "[%s][%s][%s:%d][%s]: %s",
                 getTimestampString(), tag, file, line, funcName,
                 processedMessage.c_str());
    }
    else
    {
        char escapedMsg[384] = {0};
        escapeJsonString(message, escapedMsg);

        snprintf(wsBuffer, sizeof(wsBuffer),
                 "{\"timestamp\":\"%s\",\"level\":\"%s\",\"file\":\"%s\","
                 "\"line\":%d,\"function\":\"%s\",\"message\":\"%s\"}",
                 getTimestampString(), tag, file, line, funcName, escapedMsg);
    }

    _webSocket->broadcastTXT(wsBuffer);
}

void LogLibrary::escapeJsonString(const char *input, char *output)
{
    output[0] = '\0';

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
                char buffer[7];
                sprintf(buffer, "\\u%04x", (uint8_t)*input);
                strcat(output, buffer);
            }
            else
            {
                size_t len = strlen(output);
                if (len < 383)
                {
                    output[len] = *input;
                    output[len + 1] = '\0';
                }
            }
            break;
        }
        input++;
    }
}

// CORREÇÃO PRINCIPAL: Função log usando queue corretamente
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

    // Log para Serial (comportamento original)
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
        _output->printf("\"timestamp\":\"%s\",", getTimestampString());
        _output->printf("\"level\":\"%s\",", tag);
        _output->printf("\"file\":\"%s\",", file);
        _output->printf("\"line\":%d,", line);
        _output->printf("\"function\":\"%s\",", funcName);

        char jsonMsg[_bufferSize] = {0};
        escapeJsonString(_buffer, jsonMsg);
        _output->printf("\"message\":\"%s\"", jsonMsg);
        _output->print("}");
    }
    _output->println();

    // CORREÇÃO: Enviar para queue apenas se WebSocket estiver habilitado
    if (_webSocketEnabled && _logQueue)
    {
        LogMessage logMsg;
        logMsg.level = level;

        // Converter FlashStringHelper para char arrays
        String tagStr = String(tag);
        String funcStr = String(funcName);

        strncpy(logMsg.tag, tagStr.c_str(), sizeof(logMsg.tag) - 1);
        logMsg.tag[sizeof(logMsg.tag) - 1] = '\0';

        strncpy(logMsg.funcName, funcStr.c_str(), sizeof(logMsg.funcName) - 1);
        logMsg.funcName[sizeof(logMsg.funcName) - 1] = '\0';

        // Extrair nome do arquivo
        const char *filename = strrchr(file, '/');
        if (!filename)
            filename = strrchr(file, '\\');
        if (filename)
            filename++;
        else
            filename = file;

        strncpy(logMsg.file, filename, sizeof(logMsg.file) - 1);
        logMsg.file[sizeof(logMsg.file) - 1] = '\0';

        logMsg.line = line;
        strncpy(logMsg.message, _buffer, sizeof(logMsg.message) - 1);
        logMsg.message[sizeof(logMsg.message) - 1] = '\0';

        // CORREÇÃO: Enviar para queue (não bloqueante, timeout 0)
        BaseType_t result = xQueueSend(_logQueue, &logMsg, 0);
        if (result != pdTRUE)
        {
            // Queue cheia - você pode adicionar um log de warning se quiser
            // LOG_WARN("Queue de logs cheia - mensagem perdida: %s", _buffer);
        }
    }

    // CORREÇÃO: REMOVER esta chamada - agora só usa queue
    // sendToWebSocket(level, tagStr.c_str(), funcStr.c_str(), filename, line, _buffer);
}
