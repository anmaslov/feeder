/*
  schedule.cpp - Управление расписанием кормления
*/

#include "schedule.h"
#include "feeder.h"
#include "mqtt_handler.h"
#include <time.h>

// Массив расписаний
Schedule schedules[MAX_SCHEDULES];

// Глобальный объект Preferences
Preferences preferences;

// Инициализация расписания
void scheduleSetup() {
  loadSettings();
  Serial.println("[OK] Расписание инициализировано");
}

// Сохранение настроек
void saveSettings() {
  preferences.begin("feeder", false);
  preferences.putInt("feedAmount", feedAmount);
  
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    char key_h[16], key_m[16], key_a[16], key_e[16];
    sprintf(key_h, "sched%d_h", i);
    sprintf(key_m, "sched%d_m", i);
    sprintf(key_a, "sched%d_a", i);
    sprintf(key_e, "sched%d_e", i);
    
    preferences.putUChar(key_h, schedules[i].hour);
    preferences.putUChar(key_m, schedules[i].minute);
    preferences.putInt(key_a, schedules[i].amount);
    preferences.putBool(key_e, schedules[i].enabled);
  }
  
  preferences.end();
  Serial.println("[PREF] Настройки сохранены");
}

// Загрузка настроек
void loadSettings() {
  preferences.begin("feeder", true);
  feedAmount = preferences.getInt("feedAmount", DEFAULT_FEED_AMOUNT);
  
  // Инициализация расписаний значениями по умолчанию
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    schedules[i].hour = (i * 4) % 24;  // 0, 4, 8, 12, 16
    schedules[i].minute = 0;
    schedules[i].amount = DEFAULT_FEED_AMOUNT;
    schedules[i].enabled = (i < 3);  // Первые 3 включены
    schedules[i].fed = false;
  }
  
  // Пытаемся загрузить из памяти (если есть)
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    char key_h[16], key_m[16], key_a[16], key_e[16];
    sprintf(key_h, "sched%d_h", i);
    sprintf(key_m, "sched%d_m", i);
    sprintf(key_a, "sched%d_a", i);
    sprintf(key_e, "sched%d_e", i);
    
    if (preferences.isKey(key_h)) {
      schedules[i].hour = preferences.getUChar(key_h, schedules[i].hour);
      schedules[i].minute = preferences.getUChar(key_m, schedules[i].minute);
      schedules[i].amount = preferences.getInt(key_a, schedules[i].amount);
      schedules[i].enabled = preferences.getBool(key_e, schedules[i].enabled);
    }
  }
  
  preferences.end();
  
  // Показываем загруженные расписания
  Serial.println("[PREF] Расписания загружены:");
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    if (schedules[i].enabled) {
      Serial.printf("  #%d: %02d:%02d - %d оборотов\n", 
                    i + 1, schedules[i].hour, schedules[i].minute, schedules[i].amount);
    }
  }
  Serial.printf("[PREF] Базовая порция: %d оборотов\n", feedAmount);
}

// Проверка расписания
void checkSchedule() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;
  
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    if (!schedules[i].enabled) continue;
    
    // Совпадает ли час и минута?
    bool timeMatch = (timeinfo.tm_hour == schedules[i].hour && 
                      timeinfo.tm_min == schedules[i].minute);
    
    if (timeMatch) {
      // Время совпало! Проверяем первые 10 секунд для надежности
      if (timeinfo.tm_sec < 10 && !schedules[i].fed) {
        Serial.printf("[SCHEDULE] Расписание #%d (%02d:%02d): %d оборотов\n", 
                i + 1, schedules[i].hour, schedules[i].minute, schedules[i].amount);
        
        feed(schedules[i].amount);
        publishLastFeeding(schedules[i].amount, "schedule");
        schedules[i].fed = true;
      }
    } else {
      // Время НЕ совпадает - сбрасываем флаг
      schedules[i].fed = false;
    }
  }
}
