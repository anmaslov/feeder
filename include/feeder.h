/*
  feeder.h - Управление мотором и кормлением
*/

#ifndef FEEDER_H
#define FEEDER_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"

// LED массив
extern CRGB leds[NUM_LEDS];

// Текущая порция кормления
extern int feedAmount;

// Инициализация LED и моторных пинов
void feederSetup();

// Управление мотором
void runMotor(int8_t dir);
void disableMotor();
void oneRev();

// LED эффекты
void flashLights();
void feedAnimation(int revCount);

// Индикация состояния системы (мигание как маяк)
enum SystemStatus {
  STATUS_OK,           // Все хорошо - зеленый раз в 30 сек
  STATUS_WIFI_ISSUE,   // Проблемы с WiFi - синий раз в 10 сек
  STATUS_ERROR         // Ошибка - красный
};
void updateStatusLed(SystemStatus status);

// Процесс кормления
void feed(int amount = 0);

#endif
