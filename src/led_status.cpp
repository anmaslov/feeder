/*
  led_status.cpp - Управление адресной LED лентой
*/

#include "led_status.h"

// LED массив
CRGB leds[NUM_LEDS];

// Инициализация LED ленты
void ledSetup() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS);
  Serial.println("[OK] LED лента инициализирована");
}

// Установить оба светодиода в один цвет
void ledSetColor(CRGB color) {
  leds[0] = color;
  leds[1] = color;
  FastLED.show();
}

// Установить цвет для каждого светодиода отдельно
void ledSetColors(CRGB color0, CRGB color1) {
  leds[0] = color0;
  leds[1] = color1;
  FastLED.show();
}

// Выключить все светодиоды
void ledClear() {
  FastLED.clear();
  FastLED.show();
}

// Мигалка синим-красным при старте
void ledFlashStartup() {
  for (byte i = 0; i < 3; i++) {
    ledSetColors(CRGB::Blue, CRGB::Red);
    delay(200);
    ledSetColors(CRGB::Red, CRGB::Blue);
    delay(200);
  }
  ledClear();
}

// Анимация во время кормления
void ledFeedAnimation(int revCount) {
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

// Индикация состояния системы (мигание как маяк - короткая вспышка)
void ledUpdateStatus(SystemStatus status) {
  static unsigned long lastBlink = 0;
  static bool isBlinking = false;
  static unsigned long blinkStart = 0;
  
  const uint8_t LOW_BRIGHTNESS = 15;
  const unsigned long FLASH_DURATION = 50;
  
  unsigned long blinkInterval;
  CRGB color;
  
  switch (status) {
    case STATUS_OK:
      blinkInterval = 10000;
      color = CRGB::Green;
      break;
    case STATUS_WIFI_ISSUE:
      blinkInterval = 10000;
      color = CRGB::Blue;
      break;
    case STATUS_ERROR:
      blinkInterval = 3000;
      color = CRGB::Red;
      break;
    default:
      blinkInterval = 30000;
      color = CRGB::Green;
  }
  
  unsigned long now = millis();
  
  if (!isBlinking && (now - lastBlink >= blinkInterval)) {
    isBlinking = true;
    blinkStart = now;
    lastBlink = now;
    
    FastLED.setBrightness(LOW_BRIGHTNESS);
    leds[0] = color;
    FastLED.show();
  }
  
  if (isBlinking && (now - blinkStart >= FLASH_DURATION)) {
    isBlinking = false;
    
    leds[0] = CRGB::Black;
    FastLED.show();
    FastLED.setBrightness(LED_BRIGHTNESS);
  }
}
