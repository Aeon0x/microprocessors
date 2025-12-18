#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi
const char* ssid = "samgtu_emp";
const char* password = "SNye6Gsyc6";

// Пины
const int LDR_PIN = A0;
const int LED1_PIN = D7;
const int LED2_PIN = D6;
const int LED3_PIN = D5; 

// Состояния светодиодов
bool ledStates[3] = {LOW, LOW, LOW};

// Веб-сервер на порту 80
ESP8266WebServer server(80);

// Функция перевода ADC → люксы
float calculateLuxFromADC(int adcValue) {
  return (adcValue + 101.0f) / 3.0f;
}

// Чтение датчика и расчёт люксов
float readLux() {
  int adc = analogRead(LDR_PIN);  // 0–1023
  return calculateLuxFromADC(adc);
}

// Настройка пинов
void setupPins() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  digitalWrite(LED1_PIN, ledStates[0]);
  digitalWrite(LED2_PIN, ledStates[1]);
  digitalWrite(LED3_PIN, ledStates[2]);
}

// Генерация HTML-страницы
String createHtmlPage() {
  float lux = readLux();

  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP8266: Люксметр</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin: 15px; }
    .block { margin: 15px auto; padding: 12px; border: 1px solid #ccc; max-width: 360px; }
    .led { display: inline-block; margin: 8px; padding: 10px; border-radius: 5px; }
    .led-on { background-color: #d4edda; border: 2px solid #28a745; }
    .led-off { background-color: #f8d7da; border: 2px solid #dc3545; }
    button { padding: 6px 14px; font-size: 14px; margin: 4px; cursor: pointer; }
    .value { font-size: 1.3em; color: #007bff; transition: all 0.3s; }
    .loading { color: #6c757d; }
  </style>
</head>
<body>
  <h1>Люксметр на ESP8266</h1>
  <div class="block">
    <h2>Данные с датчика</h2>
    <p>Освещённость: <span id="lux-value" class="value">)rawliteral" + String(lux, 1) + R"rawliteral(</span> люкс</p>
    <p><small>(ADC: )rawliteral" 
    + String((int)((lux * 3.0f) - 101.0f)) + R"rawliteral()</small></p>
  </div>

  <div class="block">
    <h2>Управление светодиодами</h2>
)rawliteral";

  const char* names[] = {"Зелёный", "Жёлтый", "Красный"};
  for (int i = 0; i < 3; i++) {
    String cls = (ledStates[i] == HIGH) ? "led-on" : "led-off";
    String txt = (ledStates[i] == HIGH) ? "ВКЛ" : "ВЫКЛ";
    String col = (ledStates[i] == HIGH) ? "green" : "red";

    html += "<div class='" + cls + "'>";
    html += "<h3>" + String(names[i]) + "</h3>";
    html += "<p>Статус: <b style='color:" + col + "'>" + txt + "</b></p>";
    html += "<a href='/L" + String(i+1) + "_ON'><button>ВКЛ</button></a>";
    html += "<a href='/L" + String(i+1) + "_OFF'><button>ВЫКЛ</button></a>";
    html += "</div>";
  }

  html += R"rawliteral(
  </div>
  <p><small>Обновите страницу для актуальных данных</small></p>
  <script>
  function updateLux() {
    const luxElement = document.getElementById('lux-value');
    luxElement.className = 'value loading';
    luxElement.textContent = '…';

    fetch('/api/lux')
      .then(response => response.json())
      .then(data => {
        luxElement.textContent = data.lux;
        luxElement.className = 'value';
      })
      .catch(() => {
        luxElement.textContent = 'ошибка';
        luxElement.style.color = 'red';
      });
  }

  // Обновляем при загрузке и каждые 1000 мс
  document.addEventListener('DOMContentLoaded', () => {
    updateLux();
    setInterval(updateLux, 1000);
  });
</script>
</body>
</html>
)rawliteral";

  return html;
}

// Обработчики
void handleRoot() {
  server.send(200, "text/html", createHtmlPage());
}

void handleLed1On()  { ledStates[0] = HIGH;  digitalWrite(LED1_PIN, HIGH);  server.sendHeader("Location", "/"); server.send(303); }
void handleLed1Off() { ledStates[0] = LOW;   digitalWrite(LED1_PIN, LOW);   server.sendHeader("Location", "/"); server.send(303); }
void handleLed2On()  { ledStates[1] = HIGH;  digitalWrite(LED2_PIN, HIGH);  server.sendHeader("Location", "/"); server.send(303); }
void handleLed2Off() { ledStates[1] = LOW;   digitalWrite(LED2_PIN, LOW);   server.sendHeader("Location", "/"); server.send(303); }
void handleLed3On()  { ledStates[2] = HIGH;  digitalWrite(LED3_PIN, HIGH);  server.sendHeader("Location", "/"); server.send(303); }
void handleLed3Off() { ledStates[2] = LOW;   digitalWrite(LED3_PIN, LOW);   server.sendHeader("Location", "/"); server.send(303); }

// Подключение к Wi-Fi
void connectToWiFi() {
  Serial.print("Подключение к ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts++ < 20) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ Wi-Fi подключён");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n✗ Не удалось подключиться. Перезагрузка...");
    delay(1000);
    ESP.restart();
  }
}

// Инициализация
void setup() {
  Serial.begin(115200);
  setupPins();
  connectToWiFi();

  server.on("/", handleRoot);
  server.on("/L1_ON",  handleLed1On);
  server.on("/L1_OFF", handleLed1Off);
  server.on("/L2_ON",  handleLed2On);
  server.on("/L2_OFF", handleLed2Off);
  server.on("/L3_ON",  handleLed3On);
  server.on("/L3_OFF", handleLed3Off);

  server.on("/api/lux", []() {
    float lux = readLux();
    String json = "{\"lux\":" + String(lux, 1) + "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("→ HTTP-сервер запущен");
}

void loop() {
  server.handleClient();
}