/*
  feeder.cpp - Управление мотором и кормлением
*/

#include "feeder.h"

// LED массив
CRGB leds[NUM_LEDS];

// Текущая порция кормления (по умолчанию)
int feedAmount = DEFAULT_FEED_AMOUNT;

// Последовательность шагов для двигателя
static const byte steps[] = {0b1010, 0b0110, 0b0101, 0b1001};

// Инициализация LED и моторных пинов
void feederSetup() {
  // Настройка адресной ленты
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS);
  Serial.println("[OK] LED лента инициализирована");
  
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

// Мигалка синим-красным при старте
void flashLights() {
  for (byte i = 0; i < 3; i++) {
    leds[0] = CRGB::Blue;
    leds[1] = CRGB::Red;
    FastLED.show();
    delay(200);
    leds[0] = CRGB::Red;
    leds[1] = CRGB::Blue;
    FastLED.show();
    delay(200);
  }
  FastLED.clear();
  FastLED.show();
}

// Анимация во время кормления
void feedAnimation(int revCount) {
  static unsigned long lastUpdate = 0;
  static byte hue = 0;
  
  if (millis() - lastUpdate > 50) {
    leds[0] = CHSV(hue, 255, 255);
    leds[1] = CHSV(hue + 128, 255, 255);
    FastLED.show();
    hue += 5;
    lastUpdate = millis();
  }
}

// Процесс кормления
void feed(int amount) {
  if (amount == 0) amount = feedAmount;
  
  Serial.printf("[FEED] Начало кормления: %d оборотов\n", amount);
  
  for (int i = 0; i < amount; i++) {
    oneRev();
    feedAnimation(i);
    if (i % 25 == 0 && i > 0) {
      Serial.printf("[FEED] Прогресс: %d/%d\n", i, amount);
    }
  }
  
  disableMotor();
  FastLED.clear();
  FastLED.show();
  
  Serial.println("[FEED] Кормление завершено");
}

// Индикация состояния системы (мигание как маяк - короткая вспышка)
void updateStatusLed(SystemStatus status) {
  static unsigned long lastBlink = 0;
  static bool isBlinking = false;
  static unsigned long blinkStart = 0;
  
  const uint8_t LOW_BRIGHTNESS = 15;  // Низкая яркость для индикации
  const unsigned long FLASH_DURATION = 50;  // Длительность вспышки (мс)
  
  unsigned long blinkInterval;
  CRGB color;
  
  // Определяем цвет и интервал в зависимости от статуса
  switch (status) {
    case STATUS_OK:
      blinkInterval = 30000;  // 30 секунд
      color = CRGB::Green;
      break;
    case STATUS_WIFI_ISSUE:
      blinkInterval = 10000;  // 10 секунд
      color = CRGB::Blue;
      break;
    case STATUS_ERROR:
      blinkInterval = 3000;   // 3 секунды
      color = CRGB::Red;
      break;
    default:
      blinkInterval = 30000;
      color = CRGB::Green;
  }
  
  unsigned long now = millis();
  
  // Проверяем, нужно ли начать вспышку
  if (!isBlinking && (now - lastBlink >= blinkInterval)) {
    isBlinking = true;
    blinkStart = now;
    lastBlink = now;
    
    // Включаем светодиод с низкой яркостью
    FastLED.setBrightness(LOW_BRIGHTNESS);
    leds[0] = color;
    FastLED.show();
  }
  
  // Проверяем, нужно ли выключить вспышку
  if (isBlinking && (now - blinkStart >= FLASH_DURATION)) {
    isBlinking = false;
    
    // Выключаем светодиод и восстанавливаем яркость
    leds[0] = CRGB::Black;
    FastLED.show();
    FastLED.setBrightness(LED_BRIGHTNESS);
  }
}
