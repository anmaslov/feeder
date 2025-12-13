/*
  web_server.h - Веб-сервер и HTTP обработчики
*/

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include "config.h"

// Глобальный объект веб-сервера
extern WebServer server;

// Инициализация веб-сервера
void webServerSetup();

// Обработка клиентов (вызывать в loop)
void webServerLoop();

// HTTP обработчики
void handleRoot();
void handleTime();
void handleGetSchedules();
void handleSaveSchedules();
void handleFeed();
void handleToggle();
void handleSetBase();
void handleCapture();

#endif
