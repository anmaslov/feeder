/*
  ESP32-CAM Автокормушка v3.2
  Основано на проекте GyverFeed2 от AlexGyver
  
  Модульная структура:
  - config.h       : Настройки (WiFi, MQTT, пины)
  - feeder.h/cpp   : Мотор, LED, кормление
  - schedule.h/cpp : Расписание
  - mqtt_handler.h/cpp : MQTT
  - web_server.h/cpp   : HTTP API
*/

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <time.h>

#include "SimpleButton.h"
#include "config.h"
#include "led_status.h"
#include "feeder.h"
#include "schedule.h"
#include "mqtt_handler.h"
#include "web_server.h"

// ==================== ПЕРЕМЕННЫЕ ====================
SimpleButton btn(BTN_PIN);
bool wifiConnected = false;
bool wifiServicesInitialized = false;  // Флаг: сервисы (NTP, OTA, MQTT, WebServer) инициализированы
unsigned long lastWifiReconnect = 0;
#define WIFI_RECONNECT_INTERVAL 10000  // Интервал между попытками переподключения (мс)

// Forward-объявления
void ntpSetup();
void otaSetup();

// ==================== WiFi ====================
void wifiSetup() {
  Serial.println("\n--- Подключение к WiFi ---");
  Serial.printf("SSID: %s\n", WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Индикация подключения
  ledSetColor(CRGB::Yellow);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    attempts++;
    Serial.print(".");
    ledSetColors(
      (attempts % 2) ? CRGB::Yellow : CRGB::Black,
      (attempts % 2) ? CRGB::Black : CRGB::Yellow
    );
  }
  
  Serial.println();
  wifiConnected = (WiFi.status() == WL_CONNECTED);
  
  if (wifiConnected) {
    Serial.println("[OK] WiFi подключен!");
    Serial.printf("     IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("     MAC: %s\n", WiFi.macAddress().c_str());
    
    ledSetColor(CRGB::Green);
    delay(1000);
  } else {
    Serial.println("[ОШИБКА] WiFi не подключен!");
    Serial.println("  Проверьте SSID и пароль в config.h");
    
    ledSetColor(CRGB::Red);
    delay(2000);
  }
  
  ledClear();
}

// ==================== WiFi Reconnect ====================
void wifiReconnect() {
  if (millis() - lastWifiReconnect < WIFI_RECONNECT_INTERVAL) return;
  lastWifiReconnect = millis();

  Serial.println("[WIFI] Соединение потеряно, переподключение...");
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.printf("[OK] WiFi переподключен! IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("[WARN] WiFi не удалось переподключить");
  }
}

// Инициализация WiFi-зависимых сервисов (вызывается один раз при первом подключении)
void initWifiServices() {
  if (wifiServicesInitialized) return;

  Serial.println("[INIT] Инициализация Wi-Fi сервисов...");
  ntpSetup();
  otaSetup();
  mqttSetup();
  webServerSetup();
  wifiServicesInitialized = true;

  Serial.println("\n===========================================");
  Serial.println("  СИСТЕМА ГОТОВА!");
  Serial.printf("  http://%s\n", WiFi.localIP().toString().c_str());
  Serial.println("===========================================\n");
}

// ==================== NTP ====================
void ntpSetup() {
  Serial.println("--- Синхронизация времени ---");
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  
  delay(2000);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.printf("[OK] Время: %02d.%02d.%04d %02d:%02d:%02d\n",
      timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  } else {
    Serial.println("[WARN] Время еще не синхронизировано");
  }
}

// ==================== OTA ====================
void otaSetup() {
  Serial.println("--- Настройка OTA ---");
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  
  ArduinoOTA.onStart([]() {
    Serial.println("[OTA] Начало обновления...");
    ledSetColor(CRGB::Purple);
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[OTA] %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\n[OTA] Готово!");
    ledSetColor(CRGB::Green);
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Ошибка[%u]\n", error);
  });
  
  ArduinoOTA.begin();
  Serial.println("[OK] OTA готов");
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n===========================================");
  Serial.println("  ESP32-CAM Автокормушка v3.2");
  Serial.println("===========================================\n");
  
  // 1. Инициализация оборудования
  feederSetup();
  ledFlashStartup();
  
  // 2. Загрузка настроек
  scheduleSetup();
  
  // 3. Подключение к WiFi
  wifiSetup();
  
  if (wifiConnected) {
    initWifiServices();
  }
}

// ==================== LOOP ====================
void loop() {
  delay(10);
  btn.tick();
  
  // Актуализируем состояние Wi-Fi
  wifiConnected = (WiFi.status() == WL_CONNECTED);

  // Переподключение при потере связи
  if (!wifiConnected) {
    wifiReconnect();
  }

  // Индикация состояния
  ledUpdateStatus(wifiConnected ? STATUS_OK : STATUS_WIFI_ISSUE);
  
  // Инициализация сервисов при первом успешном подключении
  if (wifiConnected) {
    initWifiServices();
    // MQTT
    mqttLoop();
    
    // Веб-сервер
    webServerLoop();
    
    // OTA
    ArduinoOTA.handle();
    
    // Проверка расписания (раз в секунду)
    static unsigned long lastScheduleCheck = 0;
    if (millis() - lastScheduleCheck >= 1000) {
      checkSchedule();
      lastScheduleCheck = millis();
    }
  }
  
  // Heartbeat (раз в 30 секунд)
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > 30000) {
    Serial.printf("[INFO] Uptime: %lu сек, WiFi: %s, MQTT: %s\n", 
                  millis() / 1000, 
                  wifiConnected ? "OK" : "FAIL",
                  mqttConnected ? "OK" : "FAIL");
    lastHeartbeat = millis();
  }
  
  // Кнопка: клик - кормление
  if (btn.click()) {
    Serial.println("[BTN] Клик - кормление");
    feed();
    publishLastFeeding(feedAmount, "button");
  }
  
  // Кнопка: удержание - калибровка
  if (btn.hold()) {
    Serial.println("[BTN] Калибровка");
    int newAmount = 0;
    
    ledSetColor(CRGB::Green);
    
    while (btn.isHold()) {
      btn.tick();
      oneRev();
      newAmount++;
      if (newAmount % 20 == 0) {
        Serial.printf("[CAL] %d оборотов\n", newAmount);
      }
    }
    
    disableMotor();
    ledClear();
    
    feedAmount = newAmount;
    saveSettings();
    Serial.printf("[BTN] Новая порция: %d\n", feedAmount);
  }
}