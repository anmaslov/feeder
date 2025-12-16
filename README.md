# 🐾 ESP32-CAM Pet Feeder

Smart automatic pet feeder based on ESP32-CAM with web interface, scheduling, MQTT integration with Home Assistant Auto Discovery, and OTA updates.

![ESP32-CAM](https://img.shields.io/badge/ESP32-CAM-blue)
![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange)
![License](https://img.shields.io/badge/license-MIT-green)
![MQTT](https://img.shields.io/badge/MQTT-Home_Assistant-green)
![Version](https://img.shields.io/badge/version-3.2.0-blue)

🇷🇺 [Русская версия](README_RU.md)

## 📋 Features

### Core Functions:
- ✅ **Button Control**: short press to feed, long press for calibration
- ✅ **RGB LED Indication**: WS2812B strip (2 LEDs) with status animations
- ✅ **Settings Storage**: all parameters saved in non-volatile memory (Preferences)
- ✅ **WiFi Connection**: automatic connection to home network
- ✅ **Web Interface**: browser-based control with schedule management
- ✅ **Scheduling**: up to 5 automatic feedings per day
- ✅ **NTP Sync**: accurate time synchronization from the internet
- ✅ **OTA Updates**: over-the-air firmware updates
- ✅ **MQTT Integration**: Home Assistant Auto Discovery support

### MQTT Features:
- 📡 **Auto Discovery**: automatic device registration in Home Assistant
- 📊 **Boot Time Sensor**: timestamp of last device boot
- 🎮 **Remote Feeding**: feed command via MQTT button
- 📱 **Home Assistant**: full integration with sensors and buttons
- 🔔 **Last Feeding Sensor**: JSON with timestamp, amount, and source
- ✅ **Availability**: online/offline binary sensor with Last Will

## 🛠 Components

| Component | Description |
|-----------|-------------|
| ESP32-CAM | Main board |
| 28BYJ-48 Stepper Motor | Motor for auger |
| ULN2003 Driver | Motor driver |
| WS2812B LED Strip | 2 addressable LEDs |
| Button | Tactile button for control |
| 5V/2A Power Supply | System power |

## 📌 Wiring Diagram

### ESP32-CAM → Motor Driver (ULN2003)
```
GPIO 12 (IN1) → Phase 1
GPIO 13 (IN2) → Phase 2
GPIO 15 (IN3) → Phase 3
GPIO 14 (IN4) → Phase 4
```

### Button
```
GPIO 2 → One button contact
GND    → Other button contact
```

### WS2812B LED Strip
```
GPIO 16 → DIN (Data In)
5V      → VCC
GND     → GND
```

### Power
```
5V  → VCC (driver and ESP32-CAM)
GND → GND (common for all components)
```

## 🚀 Installation and Setup

### 1. Clone Repository
```bash
git clone https://github.com/anmaslov/feeder.git
cd feeder
```

### 2. Configuration
Create `.env` file with your settings:
```bash
# WiFi
WIFI_SSID=your_wifi_ssid
WIFI_PASSWORD=your_wifi_password

# MQTT
MQTT_SERVER=192.168.1.100
MQTT_PORT=1883
MQTT_USER=mqtt_user
MQTT_PASSWORD=mqtt_password

# OTA
ESP_IP=192.168.1.100
OTA_HOSTNAME=ESP32-Feeder
```

### 3. USB Upload
```bash
# Comment out OTA upload in platformio.ini for USB upload
platformio run --target upload
```

### 4. Filesystem Upload (web interface)
```bash
platformio run --target uploadfs
```

### 5. OTA Update (after first upload)
```bash
platformio run --target upload --upload-port ESP_IP_FROM_ENV
```

## 📖 Usage

### LED Indication

| Indication | Meaning |
|------------|---------|
| 3 blue/red blinks at startup | Board loaded |
| Yellow blinking | WiFi connecting |
| Green LEDs (1 sec) | WiFi connected |
| Red LEDs (2 sec) | WiFi connection failed |
| Rainbow animation | Feeding in progress |
| Purple LEDs | OTA update in progress |
| Short green flash (every 30s) | System OK |
| Short blue flash (every 10s) | WiFi issue |
| Short red flash (every 3s) | Error |

### Button Control

#### Short Press (< 0.5 sec)
- Starts feeding process
- Motor dispenses saved portion
- Rainbow LED animation during operation

#### Long Press (> 0.5 sec)
- Starts portion calibration
- Green LEDs turn on
- Motor runs while button held
- Release when desired amount dispensed
- New portion size automatically saved

## 🔧 Parameter Settings

In `include/config.h` you can modify:

```cpp
#define FEED_SPEED 3000         // Motor speed (µs between steps)
#define STEPS_FRW 19            // Steps forward
#define STEPS_BKW 12            // Steps backward (prevents jamming)
#define DEFAULT_FEED_AMOUNT 15  // Default portion (revolutions)
#define MAX_SCHEDULES 5         // Maximum number of schedules
#define LED_BRIGHTNESS 50       // LED brightness (0-255)
```

## 📁 Project Structure

```
feeder/
├── src/
│   ├── main.cpp           # Main code, setup and loop
│   ├── feeder.cpp         # Motor control, LED effects, feeding
│   ├── schedule.cpp       # Schedule logic and settings storage
│   ├── mqtt_handler.cpp   # MQTT client with Auto Discovery
│   ├── web_server.cpp     # HTTP API and web interface
│   └── SimpleButton.h     # Button library
├── include/
│   ├── config.h           # Configuration (pins, timers, MQTT topics)
│   ├── feeder.h           # Feeder header
│   ├── schedule.h         # Schedule header
│   ├── mqtt_handler.h     # MQTT header
│   └── web_server.h       # Web server header
├── data/
│   ├── config.json        # Settings (schedule, portions)
│   └── index.html         # Web interface
├── load_env.py            # .env loading script for PlatformIO
├── platformio.ini         # PlatformIO configuration
├── LOVELACE_CARD.yaml     # Home Assistant Lovelace card example
├── LICENSE
├── README.md              # English documentation
└── README_RU.md           # Russian documentation
```

## 🌐 Web Interface

Access the web interface at `http://<ESP_IP>/` to:
- View current time (NTP synchronized)
- Manually trigger feeding with custom portion
- Configure up to 5 scheduled feedings
- Enable/disable individual schedules

### API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Web interface |
| `/api/time` | GET | Current time |
| `/api/schedules` | GET | Get all schedules |
| `/api/schedules` | POST | Save schedules |
| `/api/feed?amount=N` | GET | Trigger feeding |
| `/api/toggle?id=N` | GET | Toggle schedule on/off |
| `/api/setbase?amount=N` | GET | Set base portion |

## 🌐 OTA Update

### Via PlatformIO
Settings in `platformio.ini`:
```ini
upload_protocol = espota
; upload_port is set automatically from .env via load_env.py
```

### Via Arduino IDE
1. Menu: Tools → Port
2. Select `ESP32-Feeder at 192.168.x.x`
3. Upload sketch

## 🔍 Debugging

Connect to Serial Monitor (115200 baud) to view logs:
```
===========================================
  ESP32-CAM Автокормушка v3.2
===========================================

[OK] LED лента инициализирована
[OK] Пины драйвера настроены
[OK] Расписание инициализировано
[OK] WiFi подключен!
     IP: 192.168.x.x
[OK] Время: 16.12.2025 14:30:00
[OK] OTA готов
[MQTT] Настроен
[MQTT] Подключение... OK!
[DISCOVERY] Отправка конфигурации...
[DISCOVERY] Готово!
[OK] Web-сервер запущен на порту 80

===========================================
  СИСТЕМА ГОТОВА!
  http://192.168.x.x
===========================================
```

## 📡 MQTT Integration

### Auto Discovery

The feeder automatically registers itself in Home Assistant via MQTT Auto Discovery. No manual configuration needed!

### MQTT Topics

| Topic | Type | Description |
|-------|------|-------------|
| `homeassistant/binary_sensor/feeder/availability/state` | Publish | online/offline status |
| `homeassistant/sensor/feeder/boot_time/state` | Publish | ISO timestamp of last boot |
| `homeassistant/sensor/feeder/last_feeding/state` | Publish | Last feeding JSON |
| `homeassistant/button/feeder/feed/set` | Subscribe | Feed command |

### Last Feeding JSON Format
```json
{
  "timestamp": "2025-12-16T14:30:00+03:00",
  "amount": 15,
  "source": "button"  // or "mqtt", "web", "schedule"
}
```

### Entities Created in Home Assistant

| Entity | Type | Description |
|--------|------|-------------|
| `binary_sensor.kormushka_dlia_kota_kormushka_onlain` | Binary Sensor | Online/offline status |
| `sensor.kormushka_dlia_kota_vremia_zagruzki` | Sensor | Boot timestamp |
| `sensor.kormushka_dlia_kota_poslednee_kormlenie` | Sensor | Last feeding with attributes |
| `button.kormushka_dlia_kota_pokormit_kota` | Button | Feed command |

### Manual MQTT Commands

```bash
# Feed with custom amount
mosquitto_pub -h YOUR_MQTT_SERVER -u YOUR_USER -P YOUR_PASSWORD \
  -t "homeassistant/button/feeder/feed/set" -m "20"

# Subscribe to last feeding
mosquitto_sub -h YOUR_MQTT_SERVER -u YOUR_USER -P YOUR_PASSWORD \
  -t "homeassistant/sensor/feeder/last_feeding/state"
```

## 🏠 Home Assistant Lovelace Card

Example card configuration in `LOVELACE_CARD.yaml`:

```yaml
type: vertical-stack
cards:
  - type: glance
    title: 🐱 Cat Feeder
    entities:
      - entity: binary_sensor.kormushka_dlia_kota_kormushka_onlain
        name: Status
      - entity: sensor.kormushka_dlia_kota_vremia_zagruzki
        name: Boot
        format: relative
  - type: entities
    entities:
      - entity: sensor.kormushka_dlia_kota_poslednee_kormlenie
        name: Last Feeding
        format: relative
      - type: attribute
        entity: sensor.kormushka_dlia_kota_poslednee_kormlenie
        attribute: amount
        name: Portion
        suffix: " revolutions"
  - type: horizontal-stack
    cards:
      - type: button
        entity: button.kormushka_dlia_kota_pokormit_kota
        name: Feed (10)
        tap_action:
          action: call-service
          service: mqtt.publish
          data:
            topic: homeassistant/button/feeder/feed/set
            payload: "10"
```

## 🐛 Troubleshooting

### Motor doesn't spin or hums
- Check phase connection correctness
- Try swapping wires on driver
- Increase `FEED_SPEED` (slow down motor)

### WiFi won't connect
- Check SSID and password correctness
- Ensure router supports 2.4 GHz (ESP32 doesn't work with 5GHz)
- Check WiFi signal strength

### LED doesn't work
- Check GPIO 16 connection
- Verify 5V power to LED strip
- Check `NUM_LEDS` setting in config.h

### OTA doesn't work
- Ensure ESP32 and computer are on same network
- Check that port 3232 is not blocked by firewall
- Try using IP address instead of hostname

### MQTT won't connect
- Check broker is running: `systemctl status mosquitto`
- Verify IP address and credentials
- Check Serial Monitor for error codes
- Ensure MQTT user has permissions for homeassistant/# topics

### Home Assistant doesn't show entities
- Check MQTT integration is configured
- Look for entities with "kormushka" in the name
- Check MQTT broker logs for discovery messages

## 📚 Based On

Project based on [GyverFeed2](https://github.com/AlexGyver/GyverFeed2) by [AlexGyver](https://alexgyver.ru/gyverfeed2/)

## 📝 License

This project is distributed under the MIT license. See [LICENSE](LICENSE) file for details.

## 🤝 Contributing

Contributions are welcome! Feel free to open issues or pull requests.

## 📧 Contact

If you have questions or suggestions, create an issue in the repository.

---

**⭐ If you like this project, give it a star on GitHub!**
