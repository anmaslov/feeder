/*
  web_server.cpp - Веб-сервер и HTTP обработчики
*/

#include "web_server.h"
#include "feeder.h"
#include "schedule.h"
#include "mqtt_handler.h"
#include <time.h>

// Глобальный объект веб-сервера
WebServer server(80);

// Инициализация веб-сервера
void webServerSetup() {
  // Инициализация SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("[WEB] Ошибка монтирования SPIFFS!");
    return;
  }
  Serial.println("[OK] SPIFFS смонтирован");
  
  // Проверка наличия index.html
  File file = SPIFFS.open("/index.html", "r");
  if (file) {
    Serial.printf("[OK] index.html найден (%d байт)\n", file.size());
    file.close();
  } else {
    Serial.println("[WARN] index.html не найден!");
  }
  
  // Регистрация обработчиков
  server.on("/", handleRoot);
  server.on("/api/time", handleTime);
  server.on("/api/schedules", HTTP_GET, handleGetSchedules);
  server.on("/api/schedules", HTTP_POST, handleSaveSchedules);
  server.on("/api/feed", handleFeed);
  server.on("/api/toggle", handleToggle);
  server.on("/api/setbase", handleSetBase);
  
  server.begin();
  Serial.println("[OK] Web-сервер запущен на порту 80");
}

// Обработка клиентов
void webServerLoop() {
  server.handleClient();
}

// Главная страница
void handleRoot() {
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "Error: index.html not found");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

// Текущее время
void handleTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    server.send(200, "application/json", "{\"time\":\"Не синхронизировано\"}");
    return;
  }
  
  char timeStr[64];
  strftime(timeStr, sizeof(timeStr), "%d.%m.%Y %H:%M:%S", &timeinfo);
  String json = "{\"time\":\"" + String(timeStr) + "\"}";
  server.send(200, "application/json", json);
}

// Получение расписаний
void handleGetSchedules() {
  String json = "{\"feedAmount\":" + String(feedAmount) + ",\"schedules\":[";
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    if (i > 0) json += ",";
    json += "{\"hour\":" + String(schedules[i].hour) + 
            ",\"minute\":" + String(schedules[i].minute) +
            ",\"amount\":" + String(schedules[i].amount) +
            ",\"enabled\":" + String(schedules[i].enabled ? "true" : "false") + "}";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

// Сохранение расписаний
void handleSaveSchedules() {
  Serial.println("[WEB] Сохранение расписания");
  
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "No data");
    return;
  }
  
  String body = server.arg("plain");
  
  // Парсинг JSON
  int pos = body.indexOf("\"schedules\"");
  if (pos < 0) {
    server.send(400, "text/plain", "Bad format");
    return;
  }
  
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    int hourPos = body.indexOf("\"hour\":", pos);
    if (hourPos < 0) break;
    
    int minPos = body.indexOf("\"minute\":", hourPos);
    int amtPos = body.indexOf("\"amount\":", minPos);
    int enPos = body.indexOf("\"enabled\":", amtPos);
    
    if (minPos > 0 && amtPos > 0) {
      int commaH = body.indexOf(',', hourPos);
      int commaM = body.indexOf(',', minPos);
      int commaA = body.indexOf(',', amtPos);
      
      if (commaH > 0 && commaM > 0) {
        schedules[i].hour = body.substring(hourPos + 7, commaH).toInt();
        schedules[i].minute = body.substring(minPos + 9, commaM).toInt();
        schedules[i].amount = body.substring(amtPos + 9, commaA > 0 ? commaA : body.indexOf('}', amtPos)).toInt();
        
        if (enPos > 0) {
          int boolEnd = body.indexOf('}', enPos);
          String enabledStr = body.substring(enPos + 10, boolEnd);
          schedules[i].enabled = (enabledStr.indexOf("true") >= 0);
        }
        
        Serial.printf("  #%d: %02d:%02d - %d об. %s\n", 
          i+1, schedules[i].hour, schedules[i].minute, schedules[i].amount,
          schedules[i].enabled ? "ВКЛ" : "ВЫКЛ");
      }
      
      pos = amtPos;
    }
  }
  
  saveSettings();
  server.send(200, "text/plain", "OK");
}

// Кормление
void handleFeed() {
  int amount = feedAmount;
  if (server.hasArg("amount")) {
    amount = server.arg("amount").toInt();
  }
  feed(amount);
  publishLastFeeding(amount, "web");
  server.send(200, "text/plain", "OK");
}

// Переключение расписания
void handleToggle() {
  if (server.hasArg("id")) {
    int id = server.arg("id").toInt() - 1;
    if (id >= 0 && id < MAX_SCHEDULES) {
      schedules[id].enabled = !schedules[id].enabled;
      Serial.printf("[WEB] Расписание %d -> %s\n", id + 1, schedules[id].enabled ? "ВКЛ" : "ВЫКЛ");
      saveSettings();
    }
  }
  server.send(200, "text/plain", "OK");
}

// Установка базовой порции
void handleSetBase() {
  if (server.hasArg("amount")) {
    feedAmount = server.arg("amount").toInt();
    Serial.printf("[WEB] Базовая порция: %d\n", feedAmount);
    saveSettings();
  }
  server.send(200, "text/plain", "OK");
}
