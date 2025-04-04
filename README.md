# log
📋 Recursos:  
✅ Múltiplos níveis de log (DEBUG, INFO, WARN, ERROR)

🌈 Cores ANSI opcionais para melhor legibilidade

⏱ Timestamp automático com millis()

📡 Suporte a múltiplas saídas (Serial, Serial1, etc.)

🧩 Compatível com diversas plataformas (AVR, ESP32, ESP8266, ARM)

📚 Buffer configurável para mensagens

📦 Instalação:  
Via PlatformIO (recomendado)
Adicione no seu platformio.ini:

```ini
lib_deps = 
    https://github.com/cturqueti/LogLibrary.git
```
Via Arduino IDE:  
Baixe o último release

Extraia para ~/Arduino/libraries/LogLibrary

Reinicie a Arduino IDE

🚀 Uso Básico:
```cpp
#include <LogLibrary.h>

void setup() {
    Log.begin();  // Inicializa com Serial padrão
    Log.setLogLevel(LogLevel::DEBUG);
    
    LOG_DEBUG("Iniciando sistema...");
    LOG_INFO("Free RAM: %d bytes", freeMemory());
}

void loop() {
    static int counter = 0;
    LOG_DEBUG("Contador: %d", counter++);
    delay(1000);
}
```
⚙️ Configuração:
Níveis de Log
```cpp
Log.setLogLevel(LogLevel::DEBUG);  // Mostra todos os logs
// LogLevel::INFO, LogLevel::WARN, LogLevel::ERROR, LogLevel::NONE
```
Saída Customizada
```cpp
Serial2.begin(115200);
Log.begin(&Serial2);  // Usa Serial2 como saída
```
Cores ANSI
```cpp
Log.enableColors(true);  // Ativa cores (padrão)
// Log.enableColors(false);  // Desativa cores
```
Tamanho do Buffer
```cpp
Log.begin(&Serial, 512);  // Buffer de 512 bytes
```
📝 Exemplo Completo:
```cpp
#include <LogLibrary.h>

void setup() {
    Serial.begin(115200);
    Log.begin(&Serial);
    Log.setLogLevel(LogLevel::DEBUG);
    Log.enableColors(true);

    LOG_DEBUG("Este é um debug");
    LOG_INFO("Informação importante");
    LOG_WARN("Atenção: temperatura alta");
    LOG_ERROR("ERRO: Sensor não respondendo");
}

void loop() {
    float temp = readTemperature();
    if(temp > 30.0) {
        LOG_WARN("Temperatura crítica: %.2fC", temp);
    }
    delay(1000);
}
```

📊 Saída Exemplo:  
[DEBUG][1250] Este é um debug  
[INFO][1251] Informação importante  
[WARN][1252] Atenção: temperatura alta  
[ERROR][1253] ERRO: Sensor não respondendo  

🌍 Compatibilidade:  
Plataforma	Testado em
ATmega328	Arduino Uno, Nano
ATmega2560	Arduino Mega
ESP32	NodeMCU-32S
ESP8266	NodeMCU 1.0
STM32	Blue Pill
SAM	Arduino Due


🤝 Contribuição:  
Contribuições são bem-vindas! Por favor:

Faça um fork do projeto

Crie uma branch (git checkout -b feature/nova-funcionalidade)

Commit suas mudanças (git commit -m 'Adiciona nova funcionalidade')

Push para a branch (git push origin feature/nova-funcionalidade)

Abra um Pull Request

📄 Licença:  
MIT License - Veja LICENSE para detalhes

Feito com ❤️ por Carlos A. D. Turqueti

🔧 Dica profissional: Use LOG_DEBUG apenas durante desenvolvimento e mude para LogLevel::INFO em produção para melhor performance!