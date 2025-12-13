"""
load_env.py - Загрузка переменных окружения из .env файла
Используется PlatformIO для подстановки секретных данных при сборке
"""

import os
from pathlib import Path

Import("env")

# Путь к .env файлу
env_file = Path(env.subst("$PROJECT_DIR")) / ".env"

def load_env_file(filepath):
    """Загрузка переменных из .env файла"""
    env_vars = {}
    if filepath.exists():
        with open(filepath, encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith('#') and '=' in line:
                    key, value = line.split('=', 1)
                    env_vars[key.strip()] = value.strip()
    return env_vars

# Загружаем переменные
env_vars = load_env_file(env_file)

if not env_vars:
    print("\n" + "="*50)
    print("⚠️  ВНИМАНИЕ: Файл .env не найден или пуст!")
    print("   Скопируйте .env.example в .env и заполните данные:")
    print("   cp .env.example .env")
    print("="*50 + "\n")
    # Значения по умолчанию для компиляции
    env_vars = {
        'WIFI_SSID': 'NOT_SET',
        'WIFI_PASSWORD': 'NOT_SET',
        'MQTT_SERVER': '192.168.1.1',
        'MQTT_PORT': '1883',
        'MQTT_USER': '',
        'MQTT_PASSWORD': '',
        'MQTT_CLIENT_ID': 'ESP32-Feeder',
        'OTA_HOSTNAME': 'ESP32-Feeder',
        'OTA_PASSWORD': '',
        'ESP_IP': '192.168.1.1'
    }

# Формируем build flags
build_flags = [
    f'-DWIFI_SSID=\\"{env_vars.get("WIFI_SSID", "NOT_SET")}\\"',
    f'-DWIFI_PASSWORD=\\"{env_vars.get("WIFI_PASSWORD", "NOT_SET")}\\"',
    f'-DMQTT_SERVER=\\"{env_vars.get("MQTT_SERVER", "192.168.1.1")}\\"',
    f'-DMQTT_PORT={env_vars.get("MQTT_PORT", "1883")}',
    f'-DMQTT_USER=\\"{env_vars.get("MQTT_USER", "")}\\"',
    f'-DMQTT_PASSWORD=\\"{env_vars.get("MQTT_PASSWORD", "")}\\"',
    f'-DMQTT_CLIENT_ID=\\"{env_vars.get("MQTT_CLIENT_ID", "ESP32-Feeder")}\\"',
    f'-DOTA_HOSTNAME=\\"{env_vars.get("OTA_HOSTNAME", "ESP32-Feeder")}\\"',
    f'-DOTA_PASSWORD=\\"{env_vars.get("OTA_PASSWORD", "")}\\"',
]

# Добавляем флаги
for flag in build_flags:
    env.Append(BUILD_FLAGS=[flag])

# Обновляем upload_port из .env если есть
esp_ip = env_vars.get("ESP_IP", "")
if esp_ip:
    env.Replace(UPLOAD_PORT=esp_ip)

print(f"✓ Загружены настройки из .env")
print(f"  WiFi SSID: {env_vars.get('WIFI_SSID', 'NOT_SET')}")
print(f"  MQTT Server: {env_vars.get('MQTT_SERVER', 'NOT_SET')}")
print(f"  ESP IP (OTA): {esp_ip if esp_ip else 'не задан'}")
