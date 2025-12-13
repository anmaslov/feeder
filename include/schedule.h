/*
  schedule.h - Управление расписанием кормления
*/

#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

// Структура расписания
struct Schedule {
  uint8_t hour;
  uint8_t minute;
  int amount;
  bool enabled;
  bool fed;  // Флаг, что уже покормили в это время
};

// Массив расписаний
extern Schedule schedules[MAX_SCHEDULES];

// Глобальный объект Preferences
extern Preferences preferences;

// Инициализация расписания
void scheduleSetup();

// Сохранение/загрузка настроек
void saveSettings();
void loadSettings();

// Проверка расписания (вызывать каждую секунду)
void checkSchedule();

#endif
