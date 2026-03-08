# Soil Moisture Sensor — Design Document

## Overview

This branch converts the ESP-Clock project into a **Plant Soil Moisture Monitor**
using a DFRobot Capacitive Soil Moisture Sensor (SEN0193) connected to a
Waveshare ESP32-S3-Zero. The LED matrix display and all clock/weather/quotes
functionality are removed. The device reads soil moisture via ADC and exposes
readings through a web dashboard.

## Hardware

### Board: Waveshare ESP32-S3-Zero
- ESP32-S3 dual-core LX7, 240 MHz
- 4MB Flash, 2MB PSRAM
- USB-C, castellated pads
- Onboard WS2812 RGB LED on GPIO21

### Sensor: DFRobot SEN0193 Capacitive Soil Moisture Sensor
- **Interface: Analog** (not I2C — outputs 0-3.0V proportional to moisture)
- Operating voltage: 3.3V–5.5V
- Output: ~3000 ADC (dry air) → ~1500 ADC (submerged in water) on 12-bit ADC
- 3-pin connector: VCC (red), GND (black), Signal (blue)

### Wiring
| ESP32-S3-Zero | Sensor   |
|---------------|----------|
| 3V3           | VCC (red)|
| GND           | GND (black)|
| GPIO4         | Signal (blue)|

GPIO4 is chosen because:
- It is on ADC1 (channels 0-9 = GPIO1-10), which works while WiFi is active
- It is not a strapping pin (GPIO0, GPIO3, GPIO45, GPIO46 are)
- It is physically accessible on the board edge

## Architecture

```
┌──────────────────────────────────────────────────┐
│              Soil Moisture Monitor                │
├──────────────────────────────────────────────────┤
│                                                  │
│  main.cpp (entry point)                          │
│    ├── WifiManager (AP/STA modes, NVS creds)     │
│    ├── WebServer (dashboard + REST API)           │
│    ├── ConfigManager (calibration, intervals)     │
│    └── MoistureSensor (ADC driver)                │
│          ├── ADC oneshot read on GPIO4            │
│          ├── Calibration (air/water values)       │
│          └── Percentage conversion                │
│                                                  │
│  Web Dashboard (index.html)                      │
│    ├── Live moisture % gauge                     │
│    ├── Raw ADC value                             │
│    ├── Plant status indicator                    │
│    ├── Calibration controls                      │
│    └── WiFi configuration                        │
│                                                  │
└──────────────────────────────────────────────────┘
```

## Components

### MoistureSensor (NEW)
- Configures ESP32-S3 ADC1 channel on GPIO4
- 12-bit resolution (0–4095), 11dB attenuation (0–3.1V range)
- Uses ESP-IDF `adc_oneshot` API (not legacy `adc1_get_raw`)
- Reads raw value, applies calibration curve for voltage, maps to 0–100%
- Averaging: takes N samples and returns mean to reduce noise
- Calibration values stored in NVS via ConfigManager

### ConfigManager (MODIFIED)
- New config struct:
  - `airValue` (uint16_t): ADC reading in dry air (default: 3000)
  - `waterValue` (uint16_t): ADC reading in water (default: 1500)
  - `readIntervalMs` (uint32_t): sensor read interval (default: 2000ms)
  - `wifiSSID`, `wifiPassword`: carried over from original

### WebServer (MODIFIED)
- Serves new dashboard HTML
- API endpoints:
  - `GET /api/moisture` — current reading (raw ADC, percentage, status)
  - `GET /api/config` — calibration values
  - `POST /api/config` — update calibration
  - `POST /api/wifi` — WiFi credentials (carried over)

### WifiManager (KEPT AS-IS)
- AP mode for initial config, STA mode for normal operation
- No changes needed

### Removed Components
- MAX7219, DisplayManager, DisplayController, Font5x7 (LED matrix)
- TimeSync (NTP not needed)
- WeatherFetcher (no weather API)
- Quotes (no quotes)

## Moisture Calculation

```
percentage = 100 - ((rawADC - waterValue) / (airValue - waterValue) * 100)
```
Clamped to 0–100%. Higher ADC = drier soil = lower percentage.

Plant status thresholds:
- 0–20%: "Very Dry — Water immediately!"
- 21–40%: "Dry — Needs watering"
- 41–60%: "Moist — Good"
- 61–80%: "Wet — Well watered"
- 81–100%: "Very Wet — Reduce watering"

## Task Breakdown

1. Create MoistureSensor driver (header + source)
2. Create unit test for moisture calculation logic
3. Modify ConfigManager for moisture calibration config
4. Modify WebServer with new API endpoints
5. Create new index.html dashboard
6. Modify main.cpp entry point
7. Update CMakeLists.txt and Kconfig.projbuild
8. Remove unused files (MAX7219, Display*, Font*, Quotes, TimeSync, Weather)
9. Create build.py script
10. Update README.md
11. Verify build succeeds
