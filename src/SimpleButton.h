// Простая библиотека для работы с кнопкой (в стиле EncButton от AlexGyver)
#pragma once
#include <Arduino.h>

class SimpleButton {
public:
  SimpleButton(uint8_t pin) : _pin(pin) {
    pinMode(_pin, INPUT_PULLUP);
  }
  
  void tick() {
    bool currentState = (digitalRead(_pin) == LOW);
    
    // Определяем события
    _clicked = false;
    _held = false;
    
    if (currentState && !_lastState) {
      // Кнопка только что нажата
      _pressTime = millis();
      _holding = false;
    }
    
    if (currentState && !_holding && (millis() - _pressTime > _holdTime)) {
      // Удержание
      _holding = true;
      _held = true;
    }
    
    if (!currentState && _lastState && !_holding) {
      // Кнопка отпущена после короткого нажатия
      _clicked = true;
    }
    
    _lastState = currentState;
    _isPressed = currentState;
  }
  
  bool click() { return _clicked; }
  bool hold() { return _held; }
  bool isHold() { return _holding; }
  bool isPressed() { return _isPressed; }
  
  void setHoldTimeout(uint16_t timeout) { _holdTime = timeout; }
  
private:
  uint8_t _pin;
  bool _lastState = false;
  bool _clicked = false;
  bool _held = false;
  bool _holding = false;
  bool _isPressed = false;
  uint32_t _pressTime = 0;
  uint16_t _holdTime = 500;
};
