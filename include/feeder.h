/*
  feeder.h - Управление мотором и кормлением
*/

#ifndef FEEDER_H
#define FEEDER_H

#include <Arduino.h>
#include "config.h"
#include "led_status.h"

// Текущая порция кормления
extern int feedAmount;

// Инициализация моторных пинов
void feederSetup();

// Управление мотором
void runMotor(int8_t dir);
void disableMotor();
void oneRev();

// Процесс кормления
void feed(int amount = 0);

#endif
