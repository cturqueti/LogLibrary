# LogLibrary  
## A Custom Log Library for (AVR, ESP32, ESP8266, ARM) based microcontrollers

![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange?style=plastic)  
![Licença](https://img.shields.io/badge/Licen%C3%A7a-Apache%202.0-blue.svg?style=plastic)  
![Versão](https://img.shields.io/badge/Vers%C3%A3o-1.0.0-green.svg?style=plastic)  

## 📋 Recursos:  
✅ Múltiplos níveis de log (DEBUG, INFO, WARN, ERROR)

🌈 Cores ANSI opcionais para melhor legibilidade

⏱ Timestamp automático com millis()

📡 Suporte a múltiplas saídas (Serial, Serial1, etc.)

🧩 Compatível com diversas plataformas (AVR, ESP32, ESP8266, ARM)

📚 Buffer configurável para mensagens

## 📦 Instalação:  
### Via PlatformIO (recomendado)
Adicione no seu platformio.ini:

```ini
lib_deps = 
    https://github.com/cturqueti/LogLibrary.git
```
### Via Arduino IDE:  
Baixe o último release

Extraia para ~/Arduino/libraries/LogLibrary

Reinicie a Arduino IDE

## 🚀 Uso Básico:
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
## ⚙️ Configuração:
### Níveis de Log
```cpp
Log.setLogLevel(LogLevel::DEBUG);  // Mostra todos os logs
// LogLevel::INFO, LogLevel::WARN, LogLevel::ERROR, LogLevel::NONE
```
### Saída Customizada
```cpp
Serial2.begin(115200);
Log.begin(&Serial2);  // Usa Serial2 como saída
```
### Cores ANSI
```cpp
Log.enableColors(true);  // Ativa cores (padrão)
// Log.enableColors(false);  // Desativa cores
```
### Tamanho do Buffer
```cpp
Log.begin(&Serial, 512);  // Buffer de 512 bytes
```
## 📝 Exemplo Completo:
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

## 📊 Saída Exemplo:  
[DEBUG][1250] Este é um debug  
[INFO][1251] Informação importante  
[WARN][1252] Atenção: temperatura alta  
[ERROR][1253] ERRO: Sensor não respondendo  

## 🌍 Compatibilidade:  
Plataforma	Testado em
ATmega328	Arduino Uno, Nano
ATmega2560	Arduino Mega
ESP32	NodeMCU-32S
ESP8266	NodeMCU 1.0
STM32	Blue Pill
SAM	Arduino Due


## 🤝 Contribuição:  
Contribuições são bem-vindas! Por favor:

Faça um fork do projeto

Crie uma branch (git checkout -b feature/nova-funcionalidade)

Commit suas mudanças (git commit -m 'Adiciona nova funcionalidade')

Push para a branch (git push origin feature/nova-funcionalidade)

Abra um Pull Request

## 📜 Licença
Copyright 2025 cturqueti

Licenciado sob a Apache License, Versão 2.0 (a "Licença");
você não pode usar este arquivo exceto em conformidade com a Licença.
Você pode obter uma cópia da Licença em:

http://www.apache.org/licenses/LICENSE-2.0

A menos que exigido por lei aplicável ou acordado por escrito, o software
distribuído sob a Licença é distribuído "COMO ESTÁ",
SEM GARANTIAS OU CONDIÇÕES DE QUALQUER TIPO, expressas ou implícitas.
Consulte a Licença para o idioma específico que rege as permissões e
limitações sob a Licença.

Consulte o arquivo [LICENSE](LICENSE) para o texto completo da licença e
[NOTICE](NOTICE) para informações sobre atribuições e histórico de modificações.


## 📝 Dicas:
🔧 Dica profissional: Use LOG_DEBUG apenas durante desenvolvimento e mude para LogLevel::INFO em produção para melhor performance!