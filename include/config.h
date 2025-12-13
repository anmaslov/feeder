/*
  config.h - Конфигурация кормушки ESP32
  
  Секретные данные (WiFi, MQTT) загружаются из .env файла
  через build flags в platformio.ini
  
  Скопируйте .env.example в .env и заполните своими данными
*/

#ifndef CONFIG_H
#define CONFIG_H

// ==================== ВЕРСИЯ ====================
#define FIRMWARE_VERSION "3.2.0"
#define FIRMWARE_MODEL "ESP32-CAM Feeder"

// ==================== ПИНЫ ====================
#define BTN_PIN 2           // Кнопка
#define LED_PIN 16          // Пин адресной RGB ленты
#define NUM_LEDS 2          // Количество светодиодов
#define LED_BRIGHTNESS 50   // Яркость LED (0-255)

// Пины драйвера мотора (фазаА1, фазаА2, фазаВ1, фазаВ2)
const byte MOTOR_PINS[] = {12, 13, 15, 14};

// ==================== МОТОР ====================
#define FEED_SPEED 3000     // Задержка между шагами мотора (мкс)
#define STEPS_FRW 19        // Шаги вперёд
#define STEPS_BKW 12        // Шаги назад
#define DEFAULT_FEED_AMOUNT 15  // Порция по умолчанию (оборотов)

// ==================== РАСПИСАНИЕ ====================
#define MAX_SCHEDULES 5     // Максимальное количество расписаний

// ==================== WIFI (из .env) ====================
#ifndef WIFI_SSID
  #define WIFI_SSID "NOT_SET"
#endif

#ifndef WIFI_PASSWORD
  #define WIFI_PASSWORD "NOT_SET"
#endif

#define WIFI_CONNECT_TIMEOUT 20000  // Таймаут подключения (мс)

// ==================== OTA (из .env) ====================
#ifndef OTA_HOSTNAME
  #define OTA_HOSTNAME "ESP32-Feeder"
#endif

#ifndef OTA_PASSWORD
  #define OTA_PASSWORD ""
#endif

// ==================== NTP ====================
#define NTP_SERVER "ru.pool.ntp.org"
#define GMT_OFFSET_SEC 10800    // GMT+3 (Москва)
#define DAYLIGHT_OFFSET_SEC 0

// ==================== MQTT (из .env) ====================
#ifndef MQTT_SERVER
  #define MQTT_SERVER "192.168.1.1"
#endif

#ifndef MQTT_PORT
  #define MQTT_PORT 1883
#endif

#ifndef MQTT_USER
  #define MQTT_USER ""
#endif

#ifndef MQTT_PASSWORD
  #define MQTT_PASSWORD ""
#endif

#ifndef MQTT_CLIENT_ID
  #define MQTT_CLIENT_ID "ESP32-Feeder"
#endif

#define MQTT_RECONNECT_INTERVAL 5000  // Интервал переподключения (мс)

// MQTT топики
#define MQTT_TOPIC_BOOT_TIME "homeassistant/sensor/feeder/boot_time/state"
#define MQTT_TOPIC_FEED_CMD "homeassistant/button/feeder/feed/set"
#define MQTT_TOPIC_LAST_FEEDING "homeassistant/sensor/feeder/last_feeding/state"
#define MQTT_TOPIC_AVAILABILITY "homeassistant/binary_sensor/feeder/availability/state"

// ==================== ТАЙМЕРЫ ====================
#define HEARTBEAT_INTERVAL 30000    // Интервал heartbeat в Serial (мс)
#define SCHEDULE_CHECK_INTERVAL 1000  // Интервал проверки расписания (мс)

#endif // CONFIG_H
