# 🐾 ESP32-CAM Pet Feeder

Smart automatic pet feeder based on ESP32-CAM with web interface, scheduling, MQTT integration, and OTA updates.

![ESP32-CAM](https://img.shields.io/badge/ESP32-CAM-blue)
![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange)
![License](https://img.shields.io/badge/license-MIT-green)
![MQTT](https://img.shields.io/badge/MQTT-Integrated-green)

🇷🇺 [Русская версия](README_RU.md)

## 📋 Features

### Core Functions:
- ✅ **Button Control**: short press to feed, long press for calibration
- ✅ **RGB LED Indication**: WS2812B strip (2 LEDs) with animations
- ✅ **Settings Storage**: all parameters saved in non-volatile memory
- ✅ **WiFi Connection**: automatic connection to home network
- ✅ **Web Interface**: browser-based control
- ✅ **Scheduling**: up to 5 automatic feedings per day
- ✅ **NTP Sync**: accurate time from the internet
- ✅ **OTA Updates**: over-the-air firmware updates
- ✅ **MQTT Integration**: logs, control, monitoring

### MQTT Features:
- 📡 **Remote Logs**: all debugging via MQTT
- 📊 **Smart Monitoring**: uptime + date/time in one message (every minute)
- 🎮 **Remote Feeding**: commands via MQTT
- 📱 **Home Assistant**: full integration with binary sensor
- 🔔 **Real-time Status**: JSON with feeding information

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
Copy the example file and fill in your data:
```bash
cp .env.example .env
```

Edit the `.env` file:
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
```

### 3. USB Upload
```bash
# Uncomment USB upload in platformio.ini if using cable
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
| 3 blinks at startup | Board loaded |
| 5 fast blinks | WiFi connected |
| LED constantly on | Feeding or calibration in progress |
| 2 fast blinks | Settings saved |
| Blinking during OTA | Firmware update in progress |

### Button Control

#### Short Press (< 0.5 sec)
- Starts feeding process
- Motor dispenses saved portion
- LED on during operation

#### Long Press (> 0.5 sec)
- Starts portion calibration
- LED turns on
- Motor runs while button held
- Release when desired amount dispensed
- New portion size automatically saved
- 2 fast blinks confirm save

## 🔧 Parameter Settings

In `src/main.cpp` you can modify:

```cpp
#define FEED_SPEED 3000     // Motor speed (µs between steps)
#define STEPS_FRW 19        // Steps forward
#define STEPS_BKW 12        // Steps backward (prevents jamming)
```

## 📁 Project Structure

```
feeder/
├── src/
│   ├── main.cpp           # Main code
│   ├── feeder.cpp         # Motor and LED control
│   ├── schedule.cpp       # Schedule logic
│   ├── mqtt_handler.cpp   # MQTT client
│   ├── web_server.cpp     # HTTP API
│   └── SimpleButton.h     # Button library
├── include/
│   ├── config.h           # Configuration (pins, timers)
│   ├── feeder.h           # Feeder header
│   ├── schedule.h         # Schedule header
│   ├── mqtt_handler.h     # MQTT header
│   └── web_server.h       # Web server header
├── data/
│   ├── config.json        # Settings (schedule, portions)
│   └── index.html         # Web interface
├── .env.example           # Environment variables example
├── load_env.py            # .env loading script
├── platformio.ini         # PlatformIO configuration
├── LOVELACE_CARD.yaml     # Home Assistant card
├── .gitignore
├── LICENSE
└── README.md
```

## 🌐 OTA Update

### Via PlatformIO
Add to `platformio.ini`:
```ini
upload_protocol = espota
upload_port = feeder-cam.local
```

### Via Arduino IDE
1. Menu: Tools → Port
2. Select `feeder-cam at 192.168.x.x`
3. Upload sketch

### With Password (optional)
Uncomment in `main.cpp`:
```cpp
ArduinoOTA.setPassword("admin");
```

## 🔍 Debugging

Connect to Serial Monitor (115200 baud) to view logs:
```
ESP32-CAM Pet Feeder ready!
Loaded portion: 100
Connecting to WiFi.....
WiFi connected!
IP address: 192.168.x.x
OTA ready
MQTT connected
Hostname: feeder-cam
```

## 📡 MQTT Integration

### Quick Start:

Feeder automatically connects to MQTT broker (settings from `.env`):
- **Server:** `your_mqtt_server:1883`
- **Login/password:** from `.env` file

### Topics:

| Topic | Type | Description |
|-------|------|-------------|
| `feeder/logs` | Publish | All logs and debugging |
| `feeder/uptime` | Publish | Uptime + date/time (every minute) |
| `feeder/feed` | Subscribe | Feed command |
| `feeder/status` | Publish | Feeding status (JSON) |
| `feeder/availability` | Publish | online/offline (deprecated) |

> **New in v3.1.1:** Topic `feeder/uptime` now includes date and time of last send!  
> Format: `Uptime: 0 days 1:23:45 | Last: 07.12.2025 14:30:15`

### Command Examples:

```bash
# View logs
mosquitto_sub -h YOUR_MQTT_SERVER -u YOUR_USER -P YOUR_PASSWORD -t "feeder/logs"

# View uptime with date/time
mosquitto_sub -h YOUR_MQTT_SERVER -u YOUR_USER -P YOUR_PASSWORD -t "feeder/uptime"

# Feed
mosquitto_pub -h YOUR_MQTT_SERVER -u YOUR_USER -P YOUR_PASSWORD -t "feeder/feed" -m "100"

# Check online (by uptime)
mosquitto_sub -h YOUR_MQTT_SERVER -u YOUR_USER -P YOUR_PASSWORD -t "feeder/uptime" -C 1 -W 150
```

### Home Assistant (v3.1.1):

```yaml
binary_sensor:
  - platform: mqtt
    name: "Feeder Online"
    state_topic: "feeder/uptime"
    payload_on: "Uptime"
    device_class: connectivity
    expire_after: 150  # Offline if no message for 2.5 minutes

mqtt:
  button:
    - name: "Feed Cat"
      command_topic: "feeder/feed"
      payload_press: "15"
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

### LED doesn't blink
- GPIO 4 may conflict with SD card
- Try changing `LED_PIN` to 33 (built-in LED)

### OTA doesn't work
- Ensure ESP32 and computer are on same network
- Check that port 3232 is not blocked by firewall
- Try using IP address instead of hostname

### MQTT won't connect
- Check broker is running: `systemctl status mosquitto`
- Verify IP address and credentials
- Check Serial Monitor for error codes

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
