/*
  mqtt_handler.h - MQTT функции для кормушки ESP32
*/

#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "config.h"

// Внешние переменные
extern PubSubClient mqttClient;
extern WiFiClient espClient;
extern bool mqttConnected;
extern bool bootTimePublished;

// Прототип функции кормления (определена в feeder.cpp)
extern void feed(int amount);

// Функции
void mqttSetup();
void mqttConnect();
void mqttLoop();
void publishBootTime();
void publishLastFeeding(int amount, const char* source);
void publishHomeAssistantDiscovery();

#endif // MQTT_HANDLER_H
