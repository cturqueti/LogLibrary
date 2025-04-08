#include <Arduino.h>
#include <LogLibrary.h>

/**
 * @brief Inicializa o sistema
 *
 * Realiza a inicialização básica do sistema, como a configuração da
 * serial, e imprime logs de exemplo com cores habilitadas.
 */
void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    };

    Log::begin(&Serial);
    Log::enableColors(true);
    Log::setLogLevel(LogLevel::DEBUG); // Mostrar todos os níveis

    LOG_INFO("Log com cores habilitadas");
    LOG_WARN("Aviso colorido!");
    LOG_ERROR("Erro colorido!");
}

/**
 * @brief La o principal do sistema
 *
 * Exemplo simples de um la o principal que espera 3 segundos
 * entre cada itera o.
 */
void loop()
{
    // Rotina do programa
    delay(3000);
}
