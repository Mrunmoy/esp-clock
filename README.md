# Soil Moisture Monitor

A plant soil moisture monitoring system using an ESP32-S3-Zero and a DFRobot Capacitive Soil Moisture Sensor (SEN0193). Reads soil moisture via ADC and displays live readings through a web dashboard.

## Features

- **Real-time soil moisture monitoring** via capacitive sensor (no corrosion)
- **Web-based dashboard** with live gauge, percentage, and plant status
- **Sensor calibration** through the web UI (dry air / water values)
- **Configurable read interval** (default 2 seconds)
- **WiFi configuration** via built-in access point on first boot
- **Persistent settings** stored in NVS flash

## Hardware

### Board: [Waveshare ESP32-S3-Zero](https://www.waveshare.com/wiki/ESP32-S3-Zero)
- ESP32-S3 dual-core, 240 MHz
- 4MB Flash, 2MB PSRAM
- USB-C, castellated pads

### Sensor: [DFRobot SEN0193 Capacitive Soil Moisture Sensor](https://www.dfrobot.com/product-1385.html)
- Analog output (0-3.0V)
- 3.3V-5.5V operating voltage
- Capacitive sensing (corrosion-resistant)

### Wiring

| ESP32-S3-Zero | Sensor       |
|---------------|-------------|
| 3V3           | VCC (red)   |
| GND           | GND (black) |
| GPIO4         | Signal (blue)|

GPIO4 is on ADC1 (works while WiFi is active) and is not a strapping pin.

## Quick Start

### Prerequisites

- Python 3.9+
- Docker (for Docker builds) OR build dependencies listed below

### Building with Docker

```bash
git clone --recursive <repo-url> soil-monitor
cd soil-monitor
./docker-build.sh
```

### Building Locally

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install git wget flex bison gperf python3 python3-pip \
    python3-venv cmake ninja-build ccache libffi-dev libssl-dev \
    dfu-util libusb-1.0-0

# Clone with submodules
git clone --recursive <repo-url> soil-monitor
cd soil-monitor

# Build using build.py (recommended)
python3 build.py

# Or build manually
source ./env.sh
idf.py set-target esp32s3
idf.py build
```

### Flash and Monitor

```bash
python3 build.py --flash --monitor --port /dev/ttyACM0

# Or manually
source ./env.sh
idf.py -p /dev/ttyACM0 flash monitor
```

### Run Unit Tests Only

```bash
python3 build.py --test
```

## Configuration

### Initial WiFi Setup

1. On first boot, the ESP32 creates a WiFi access point:
   - SSID: `SOIL_MONITOR_AP`
   - Password: `soilmonitor2024`

2. Connect to this network and open `http://192.168.4.1`

3. Enter your home WiFi credentials — the device will reboot and connect

### Sensor Calibration

1. Open the web dashboard at the device's IP address
2. Note the **Raw ADC** value with the sensor in dry air — enter as "Dry Air Value"
3. Note the **Raw ADC** value with the sensor in water — enter as "Water Value"
4. Click "Save Calibration"

Typical values:
- Dry air: ~2800-3200
- Submerged in water: ~1400-1700

### Moisture Status Thresholds

| Percentage | Status |
|-----------|--------|
| 0-20%     | Very Dry - Water immediately! |
| 21-40%    | Dry - Needs watering |
| 41-60%    | Moist - Good |
| 61-80%    | Wet - Well watered |
| 81-100%   | Very Wet - Reduce watering |

## Project Structure

```
soil-monitor/
├── main/
│   ├── inc/                    # Header files
│   │   ├── MoistureSensor.hpp  # ADC sensor driver
│   │   ├── ConfigManager.hpp   # Calibration persistence
│   │   ├── WebServer.hpp       # HTTP server + REST API
│   │   └── WifiManager.hpp     # WiFi AP/STA management
│   ├── src/                    # Implementations
│   ├── index.html              # Web dashboard UI
│   ├── main.cpp                # Entry point and main loop
│   ├── CMakeLists.txt          # Component build config
│   └── Kconfig.projbuild       # Build-time configuration
├── test/
│   └── test_moisture.cpp       # Host-side unit tests
├── build.py                    # Python build script
├── build.sh                    # Shell build script
├── docker-build.sh             # Docker build script
├── Dockerfile                  # Docker build environment
├── env.sh                      # ESP-IDF environment setup
├── DESIGN.md                   # Design document
└── ARCHITECTURE.md             # Architecture details
```

## API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/` | Web dashboard |
| GET | `/api/moisture` | Current reading (rawAdc, percentage, status) |
| GET | `/api/config` | Calibration config |
| POST | `/api/config` | Update calibration |
| POST | `/api/wifi` | Update WiFi credentials (triggers reboot) |

## License

This project is open source and available under the MIT License.
