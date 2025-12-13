/*
  mqtt_handler.cpp - MQTT функции для кормушки ESP32
*/

#include "mqtt_handler.h"
#include <time.h>

// Глобальные переменные
WiFiClient espClient;
PubSubClient mqttClient(espClient);
bool mqttConnected = false;
bool bootTimePublished = false;
unsigned long lastMqttReconnect = 0;

// Callback для входящих MQTT сообщений
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("[MQTT] Получено: ");
  Serial.print(topic);
  Serial.print(" -> ");
  Serial.println(message);
  
  // Команда кормления
  if (strcmp(topic, MQTT_TOPIC_FEED_CMD) == 0) {
    int amount = message.toInt();
    if (amount <= 0) amount = DEFAULT_FEED_AMOUNT;
    
    Serial.printf("[MQTT] Команда кормления: %d оборотов\n", amount);
    feed(amount);
    publishLastFeeding(amount, "mqtt");
  }
}

// Инициализация MQTT
void mqttSetup() {
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(1024);
  
  Serial.println("[MQTT] Настроен");
  Serial.printf("  Сервер: %s:%d\n", MQTT_SERVER, MQTT_PORT);
  Serial.printf("  Client ID: %s\n", MQTT_CLIENT_ID);
}

// Подключение к MQTT брокеру
void mqttConnect() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastMqttReconnect < MQTT_RECONNECT_INTERVAL) return;
  
  lastMqttReconnect = millis();
  
  Serial.print("[MQTT] Подключение...");
  
  // Last Will: при отключении брокер опубликует "offline"
  if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, 
                          MQTT_TOPIC_AVAILABILITY, 0, true, "offline")) {
    mqttConnected = true;
    Serial.println(" OK!");
    
    // Публикуем "online"
    mqttClient.publish(MQTT_TOPIC_AVAILABILITY, "online", true);
    
    // Подписываемся на команды
    mqttClient.subscribe(MQTT_TOPIC_FEED_CMD);
    
    // Отправляем Discovery для Home Assistant
    publishHomeAssistantDiscovery();
    
  } else {
    mqttConnected = false;
    Serial.printf(" ОШИБКА (код: %d)\n", mqttClient.state());
  }
}

// Обработка MQTT в loop
void mqttLoop() {
  if (!mqttClient.connected()) {
    mqttConnected = false;
    mqttConnect();
  } else {
    mqttConnected = true;
    mqttClient.loop();
  }
  
  // Публикация времени загрузки (один раз)
  if (mqttConnected && !bootTimePublished) {
    publishBootTime();
  }
}

// Публикация времени загрузки
void publishBootTime() {
  if (!mqttConnected || bootTimePublished) return;
  
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char isoTime[40];
    strftime(isoTime, sizeof(isoTime), "%Y-%m-%dT%H:%M:%S+03:00", &timeinfo);
    mqttClient.publish(MQTT_TOPIC_BOOT_TIME, isoTime, true);
    bootTimePublished = true;
    Serial.printf("[MQTT] Boot time: %s\n", isoTime);
  }
}

// Публикация последнего кормления
void publishLastFeeding(int amount, const char* source) {
  if (!mqttConnected) return;
  
  struct tm timeinfo;
  char isoTime[40] = "1970-01-01T00:00:00+00:00";
  
  if (getLocalTime(&timeinfo)) {
    strftime(isoTime, sizeof(isoTime), "%Y-%m-%dT%H:%M:%S+03:00", &timeinfo);
  }
  
  String json = "{";
  json += "\"timestamp\":\"" + String(isoTime) + "\",";
  json += "\"amount\":" + String(amount) + ",";
  json += "\"source\":\"" + String(source) + "\"";
  json += "}";
  
  mqttClient.publish(MQTT_TOPIC_LAST_FEEDING, json.c_str(), true);
  Serial.printf("[MQTT] Кормление: %s\n", json.c_str());
}

// MQTT Auto Discovery для Home Assistant
void publishHomeAssistantDiscovery() {
  if (!mqttConnected) return;
  
  Serial.println("[DISCOVERY] Отправка конфигурации...");
  
  // Информация об устройстве
  const char* device_config = 
    "\"device\":{"
      "\"identifiers\":[\"esp32_feeder\"],"
      "\"name\":\"Кормушка для кота\","
      "\"model\":\"" FIRMWARE_MODEL "\","
      "\"manufacturer\":\"DIY\","
      "\"sw_version\":\"" FIRMWARE_VERSION "\""
    "}";
  
  char topic[150];
  char payload[600];
  
  // 1. Binary Sensor: Availability
  strcpy(topic, "homeassistant/binary_sensor/feeder/availability/config");
  sprintf(payload,
    "{"
      "\"name\":\"Кормушка Онлайн\","
      "\"unique_id\":\"feeder_availability\","
      "\"state_topic\":\"%s\","
      "\"payload_on\":\"online\","
      "\"payload_off\":\"offline\","
      "\"device_class\":\"connectivity\","
      "%s"
    "}", MQTT_TOPIC_AVAILABILITY, device_config);
  mqttClient.publish(topic, payload, true);
  delay(100);
  
  // 2. Sensor: Boot Time
  strcpy(topic, "homeassistant/sensor/feeder/boot_time/config");
  sprintf(payload,
    "{"
      "\"name\":\"Время загрузки\","
      "\"unique_id\":\"feeder_boot_time\","
      "\"state_topic\":\"%s\","
      "\"device_class\":\"timestamp\","
      "\"icon\":\"mdi:clock-start\","
      "%s"
    "}", MQTT_TOPIC_BOOT_TIME, device_config);
  mqttClient.publish(topic, payload, true);
  delay(100);
  
  // 3. Sensor: Last Feeding
  strcpy(topic, "homeassistant/sensor/feeder/last_feeding/config");
  sprintf(payload,
    "{"
      "\"name\":\"Последнее кормление\","
      "\"unique_id\":\"feeder_last_feeding\","
      "\"state_topic\":\"%s\","
      "\"device_class\":\"timestamp\","
      "\"icon\":\"mdi:food-drumstick\","
      "\"value_template\":\"{{ value_json.timestamp }}\","
      "\"json_attributes_topic\":\"%s\","
      "%s"
    "}", MQTT_TOPIC_LAST_FEEDING, MQTT_TOPIC_LAST_FEEDING, device_config);
  mqttClient.publish(topic, payload, true);
  delay(100);
  
  // 4. Button: Feed
  strcpy(topic, "homeassistant/button/feeder/feed/config");
  sprintf(payload,
    "{"
      "\"name\":\"Покормить кота\","
      "\"unique_id\":\"feeder_feed_button\","
      "\"command_topic\":\"%s\","
      "\"icon\":\"mdi:cat\","
      "\"payload_press\":\"\","
      "%s"
    "}", MQTT_TOPIC_FEED_CMD, device_config);
  mqttClient.publish(topic, payload, true);
  
  Serial.println("[DISCOVERY] Готово!");
}
