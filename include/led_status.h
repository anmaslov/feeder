/*
  led_status.h - Управление адресной LED лентой
*/

#ifndef LED_STATUS_H
#define LED_STATUS_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"

// LED массив
extern CRGB leds[NUM_LEDS];

// Статусы системы для индикации
enum SystemStatus {
  STATUS_OK,           // Все хорошо - зеленый раз в 30 сек
  STATUS_WIFI_ISSUE,   // Проблемы с WiFi - синий раз в 10 сек
  STATUS_ERROR         // Ошибка - красный
};

// Инициализация LED ленты
void ledSetup();

// Установить оба светодиода в один цвет
void ledSetColor(CRGB color);

// Установить цвет для каждого светодиода отдельно
void ledSetColors(CRGB color0, CRGB color1);

// Выключить все светодиоды
void ledClear();

// Мигалка при старте
void ledFlashStartup();

// Анимация кормления
void ledFeedAnimation(int revCount);

// Индикация состояния системы (маяк)
void ledUpdateStatus(SystemStatus status);

#endif
