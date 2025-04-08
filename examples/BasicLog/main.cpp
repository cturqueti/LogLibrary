#include <Arduino.h>
#include <LogLibrary.h>

/**
 * @brief Inicializa o sistema
 *
 * Realiza a inicialização básica do sistema, como a configuração da
 * serial, e imprime logs de exemplo.
 */
void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    };

    Log::begin(); // Usa Serial como saída padrão

    LOG_INFO("Sistema iniciado");
    LOG_DEBUG("Valor de teste: %d", 42);
    LOG_WARN("Algo pode estar errado...");
    LOG_ERROR("Erro crítico!");
}

/**
 * @brief Laço principal do sistema
 *
 * Exemplo simples de um laço principal que espera 5 segundos
 * entre cada iteração.
 */
void loop()
{
    // Exemplo simples
    delay(5000);
}
