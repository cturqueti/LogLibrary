const char *htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Terminal ESP32 - Logs</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: 'Courier New', monospace;
            background-color: #1e1e1e;
            color: #ffffff;
            margin: 0;
            padding: 20px;
        }
        #terminal {
            background-color: #000;
            border: 1px solid #333;
            border-radius: 5px;
            padding: 15px;
            height: 500px;
            overflow-y: auto;
            font-size: 12px;
            line-height: 1.3;
        }
        .log-error { color: #ff4444; font-weight: bold; }
        .log-warn { color: #ffaa00; font-weight: bold; }
        .log-info { color: #44ff44; }
        .log-debug { color: #4488ff; }
        .log-verbose { color: #8888ff; }
        .timestamp { color: #888; margin-right: 10px; }
        
        /* Estilos para diferentes níveis no formato texto */
        .level-error { color: #ff4444; font-weight: bold; }
        .level-warn { color: #ffaa00; font-weight: bold; }
        .level-info { color: #44ff44; }
        .level-debug { color: #4488ff; }
        .level-verbose { color: #8888ff; }
    </style>
</head>
<body>
    <h1>Terminal de Logs - ESP32</h1>
    <div>Conectado: <span id="status">Desconectado</span> | 
         <button onclick="clearTerminal()">Limpar Terminal</button>
    </div>
    <div id="terminal"></div>

    <script>
        var websocket;
        var terminal = document.getElementById('terminal');
        var statusSpan = document.getElementById('status');
        
        function initWebSocket() {
            websocket = new WebSocket('ws://' + window.location.hostname + ':81/');
            
            websocket.onopen = function(event) {
                statusSpan.textContent = 'Conectado';
                statusSpan.style.color = '#44ff44';
                addToTerminal('Conectado ao ESP32', 'log-info');
            };
            
            websocket.onclose = function(event) {
                statusSpan.textContent = 'Desconectado';
                statusSpan.style.color = '#ff4444';
                addToTerminal('Conexão fechada', 'log-error');
                setTimeout(initWebSocket, 2000);
            };
            
            websocket.onmessage = function(event) {
                try {
                    // Tentar parsear como JSON
                    var data = JSON.parse(event.data);
                    processJsonLog(data);
                } catch (e) {
                    // Se não for JSON, tratar como texto simples
                    processTextLog(event.data);
                }
            };
        }
        
        function processJsonLog(data) {
            var levelClass = 'log-' + data.level.toLowerCase();
            var message = '[' + data.timestamp + '][' + data.level + '][' + 
                          data.function + ']: ' + data.message;
            addToTerminal(message, levelClass);
        }
        
        function processTextLog(text) {
            // CORREÇÃO: Detecção mais precisa dos níveis de log
            var levelClass = 'log-info'; // padrão
            
            // Verificar a posição do nível no formato: [TIMESTAMP][LEVEL]...
            var levelMatch = text.match(/\]\[(ERROR|WARN|INFO|DEBUG|VERBOSE)\]/);
            if (levelMatch) {
                var level = levelMatch[1].toLowerCase();
                levelClass = 'log-' + level;
            } else {
                // Fallback: procurar em qualquer lugar do texto
                if (text.includes('[ERROR]')) {
                    levelClass = 'log-error';
                } else if (text.includes('[WARN]')) {
                    levelClass = 'log-warn';
                } else if (text.includes('[DEBUG]')) {
                    levelClass = 'log-debug';
                } else if (text.includes('[VERBOSE]')) {
                    levelClass = 'log-verbose';
                } else if (text.includes('[INFO]')) {
                    levelClass = 'log-info';
                }
            }
            
            text = convertCodepointToEmoji(text);

            addToTerminal(text, levelClass);
        }

        function convertCodepointToEmoji(text) {
            console.log("[JS DEBUG] Texto antes da conversão:", text);
            
            // Converter formatos U+XXXX
            text = text.replace(/U\+([0-9A-Fa-f]{4,6})/g, function(match, p1) {
                console.log("[JS DEBUG] Encontrado codepoint:", match, "valor:", p1);
                
                const codePoint = parseInt(p1, 16);
                console.log("[JS DEBUG] Codepoint numérico:", codePoint);
                
                // Para codepoints acima de 0xFFFF (emojis com surrogate pairs)
                if (codePoint > 0xFFFF) {
                    const high = Math.floor((codePoint - 0x10000) / 0x400) + 0xD800;
                    const low = ((codePoint - 0x10000) % 0x400) + 0xDC00;
                    const result = String.fromCharCode(high, low);
                    console.log("[JS DEBUG] Surrogate pair:", high.toString(16), low.toString(16), "resultado:", result);
                    return result;
                } else {
                    // Para codepoints simples
                    const result = String.fromCharCode(codePoint);
                    console.log("[JS DEBUG] Codepoint simples, resultado:", result);
                    return result;
                }
            });
            
            console.log("[JS DEBUG] Texto após conversão:", text);
            return text;
        }
        
        function highlightLogLevels(text) {
            // Aplicar cores inline para níveis de log em mensagens de texto
            return text
                .replace(/\[(ERROR)\]/g, '<span class="level-error">[$1]</span>')
                .replace(/\[(WARN)\]/g, '<span class="level-warn">[$1]</span>')
                .replace(/\[(INFO)\]/g, '<span class="level-info">[$1]</span>')
                .replace(/\[(DEBUG)\]/g, '<span class="level-debug">[$1]</span>')
                .replace(/\[(VERBOSE)\]/g, '<span class="level-verbose">[$1]</span>');
        }
        
        function addToTerminal(text, className) {
            var line = document.createElement('div');
            line.className = className;
            
            text = highlightLogLevels(text);
            line.innerHTML = text;
            
            terminal.appendChild(line);
            terminal.scrollTop = terminal.scrollHeight;
        }
        
        function clearTerminal() {
            terminal.innerHTML = '';
            addToTerminal('Terminal limpo', 'log-info');
        }
        
        // Inicializar quando a página carregar
        window.onload = initWebSocket;
    </script>
</body>
</html>
)rawliteral";