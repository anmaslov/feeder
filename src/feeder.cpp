/*
  feeder.cpp - Управление мотором и кормлением
*/

#include "feeder.h"

// Текущая порция кормления (по умолчанию)
int feedAmount = DEFAULT_FEED_AMOUNT;

// Последовательность шагов для двигателя
static const byte steps[] = {0b1010, 0b0110, 0b0101, 0b1001};

// Инициализация моторных пинов
void feederSetup() {
  // LED лента
  ledSetup();

  // Настройка пинов драйвера мотора
  for (byte i = 0; i < 4; i++) {
    pinMode(MOTOR_PINS[i], OUTPUT);
  }
  Serial.println("[OK] Пины драйвера настроены");
}

// Один шаг мотора
void runMotor(int8_t dir) {
  static byte step = 0;
  for (byte i = 0; i < 4; i++) {
    digitalWrite(MOTOR_PINS[i], bitRead(steps[step & 0b11], i));
  }
  delayMicroseconds(FEED_SPEED);
  step += dir;
}

// Выключаем ток на мотор
void disableMotor() {
  for (byte i = 0; i < 4; i++) {
    digitalWrite(MOTOR_PINS[i], 0);
  }
}

// Один оборот шнека
void oneRev() {
  for (int i = 0; i < STEPS_BKW; i++) runMotor(-1);
  for (int i = 0; i < STEPS_FRW; i++) runMotor(1);
}

// Процесс кормления
void feed(int amount) {
  if (amount == 0) amount = feedAmount;
  
  Serial.printf("[FEED] Начало кормления: %d оборотов\n", amount);
  
  for (int i = 0; i < amount; i++) {
    oneRev();
    ledFeedAnimation(i);
    if (i % 25 == 0 && i > 0) {
      Serial.printf("[FEED] Прогресс: %d/%d\n", i, amount);
    }
  }
  
  disableMotor();
  ledClear();
  
  Serial.println("[FEED] Кормление завершено");
}
