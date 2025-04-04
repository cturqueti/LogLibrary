#include "LogLibrary.h"

Print *Log::_output = &Serial;
LogLevel Log::_currentLevel = LogLevel::DEBUG;
bool Log::_colorsEnabled = true;
bool Log::_newlineEnabled = true;
uint16_t Log::_bufferSize = 256;
char *Log::_buffer = nullptr;

void Log::begin(Print *output, uint16_t bufferSize)
{
    _output = output ? output : &Serial;
    _bufferSize = bufferSize;

    if (_buffer)
    {
        delete[] _buffer;
    }
    _buffer = new char[_bufferSize];
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

void Log::setLogLevel(LogLevel level)
{
    _currentLevel = level;
}

void Log::enableColors(bool enable)
{
    _colorsEnabled = enable;
}

void Log::enableNewline(bool enable)
{
    _newlineEnabled = enable;
}

void Log::log(LogLevel level, const __FlashStringHelper *tag, const char *format, ...)
{
    if (level > _currentLevel || !_output || !_buffer)
        return;

    // Formata a mensagem
    va_list args;
    va_start(args, format);
    vsnprintf(_buffer, _bufferSize, format, args);
    va_end(args);

    // Imprime o log formatado
    _output->print(getColorCode(level));
    _output->print('[');
    _output->print(tag);
    _output->print("][");
    _output->print(millis());
    _output->print("] ");
    _output->print(_buffer);
    _output->print(getResetCode());

    if (_newlineEnabled)
    {
        _output->println();
    }
}