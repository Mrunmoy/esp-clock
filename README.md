# ESP-Clock

A smart LED matrix clock display project for ESP32-S3 using MAX7219 dot matrix displays.

## Features

- **Multiple Display Modes:**
  - Digital clock with NTP time synchronization
  - Local weather display (via OpenWeatherMap API)
  - Star Wars quotes
  - Lord of the Rings quotes
  - Custom scrolling text

- **Web-Based Configuration:**
  - Simple web UI for configuring display modes
  - WiFi configuration with automatic reboot
  - Persistent configuration storage in NVS

- **Hardware:**
  - ESP32-S3 Mini Development Board
  - 5x MAX7219 8x8 Dot Matrix LED modules (connected in series)

## Hardware Connections

### ESP32-S3 to MAX7219

| ESP32-S3 | MAX7219 |
|----------|---------|
| GPIO 12  | CLK     |
| GPIO 11  | DIN     |
| GPIO 10  | CS      |
| 5V       | VCC     |
| GND      | GND     |

## Quick Start

### Prerequisites

- Docker (for building with Docker)
- OR ESP-IDF v5.5 installed locally

### Building with Docker (Recommended)

1. Clone the repository with submodules:
```bash
git clone <your-repo-url> esp-clock
cd esp-clock
git submodule update --init --recursive
```

2. Build using Docker:
```bash
./docker-build.sh
```

3. Flash to your ESP32-S3:
```bash
# You'll need ESP-IDF tools installed locally for flashing
source ./env.sh
idf.py -p /dev/ttyUSB0 flash monitor
```

### Building Locally

#### Prerequisites

```bash
# ESP-IDF is included as a submodule, but you need Python 3.9+
python3 --version  # Should be 3.9 or newer

# Install required system packages (Ubuntu/Debian)
sudo apt-get install git wget flex bison gperf python3 python3-pip \
    python3-venv cmake ninja-build ccache libffi-dev libssl-dev \
    dfu-util libusb-1.0-0
```

#### Build Steps

1. Clone the repository with submodules:
```bash
git clone <your-repo-url> esp-clock
cd esp-clock
git submodule update --init --recursive
```

2. Set up ESP-IDF environment:
```bash
# This script:
# - Sets IDF_PATH to third_party/esp-idf
# - Installs ESP-IDF tools to third_party/esp-idf-tools (first time)
# - Exports ESP-IDF environment variables
source ./env.sh
```

3. Set target to ESP32-S3 (first time only):
```bash
idf.py set-target esp32s3
```

4. Configure the project (optional but recommended):
```bash
idf.py menuconfig
```

Navigate to "ESP Clock Configuration" to set:
- OpenWeather API key (required for weather display)
- City and country code
- Timezone
- NTP server
- Default WiFi AP credentials

5. Build the project:
```bash
# Using the build script (recommended - automatically sets target)
./build.sh

# Or use idf.py directly
idf.py build

# For other targets
./build.sh esp32    # ESP32
./build.sh esp32c3  # ESP32-C3
```

6. Flash to your ESP32-S3:
```bash
# Replace /dev/ttyACM0 with your device's serial port
# Common ports: /dev/ttyUSB0, /dev/ttyACM0, /dev/cu.usbserial-*

# Flash and monitor
idf.py -p /dev/ttyACM0 flash monitor

# Press Ctrl+] to exit monitor
```

## Configuration

### Initial Setup

1. On first boot, the ESP32 creates a WiFi access point:
   - SSID: `ESP_CLOCK_AP` (configurable in menuconfig)
   - Password: `esp-clock-2024` (configurable in menuconfig)

2. Connect to this WiFi network from your phone or computer

3. Open a web browser and navigate to:
   - `http://192.168.4.1` (default AP IP)

4. Configure your home WiFi credentials in the web UI

5. The ESP32 will reboot and connect to your WiFi network

### Display Configuration

Access the web UI at the ESP32's IP address (check your router or serial monitor for the IP).

<img width="407" height="863" alt="image" src="https://github.com/user-attachments/assets/10057735-72e4-4d01-a638-a6b91a3c0872" />

Configure:
- Which display modes to show (clock, weather, quotes, custom text)
- Custom scrolling text
- WiFi credentials (will trigger reboot)

### Weather Configuration

To enable weather display:

1. Get a free API key from [OpenWeatherMap](https://openweathermap.org/api)

2. Configure via menuconfig:
```bash
idf.py menuconfig
```

Navigate to "ESP Clock Configuration" and set:
- `OPENWEATHER_API_KEY`: Your API key
- `WEATHER_CITY`: Your city name
- `WEATHER_COUNTRY_CODE`: Two-letter country code (e.g., AU, US, UK)

3. Rebuild and flash

### Timezone Configuration

Configure your timezone via menuconfig:
```bash
idf.py menuconfig
```

Navigate to "ESP Clock Configuration" and set:
- `TIMEZONE`: POSIX timezone string

Examples:
- Sydney: `AEST-10AEDT,M10.1.0,M4.1.0/3`
- New York: `EST5EDT,M3.2.0,M11.1.0`
- London: `GMT0BST,M3.5.0/1,M10.5.0`
- Los Angeles: `PST8PDT,M3.2.0,M11.1.0`

## Project Structure

```
esp-clock/
├── main/
│   ├── inc/                    # Header files
│   │   ├── WifiManager.hpp
│   │   ├── WebServer.hpp
│   │   ├── ConfigManager.hpp
│   │   ├── TimeSync.hpp
│   │   ├── WeatherFetcher.hpp
│   │   ├── Quotes.hpp
│   │   ├── MAX7219.hpp
│   │   ├── Font5x7.hpp
│   │   ├── DisplayManager.hpp
│   │   └── DisplayController.hpp
│   ├── src/                    # Implementation files
│   ├── index.html              # Web UI
│   ├── main.cpp                # Application entry point
│   ├── CMakeLists.txt          # Component build config
│   └── Kconfig.projbuild       # Configuration menu
├── third_party/
│   └── esp-idf/                # ESP-IDF framework (submodule)
├── CMakeLists.txt              # Root build config
├── env.sh                      # Environment setup script
├── build.sh                    # Build script
├── docker-build.sh             # Docker build script
├── Dockerfile                  # Docker build environment
├── .gitignore
├── .gitmodules
└── README.md
```

## Architecture

The project follows a modular architecture inspired by the SensorFusion project template:

- **WifiManager**: Handles WiFi connection (AP and Station modes)
- **WebServer**: HTTP server with REST API for configuration
- **ConfigManager**: Persistent configuration storage using NVS
- **TimeSync**: NTP time synchronization
- **WeatherFetcher**: OpenWeatherMap API integration
- **Quotes**: Database of Star Wars and LOTR quotes
- **MAX7219**: SPI driver for MAX7219 LED matrix controller
- **Font5x7**: 5x7 bitmap font for text rendering
- **DisplayManager**: Text rendering and scrolling on LED matrix
- **DisplayController**: Orchestrates display modes and switching

## Display Modes

The display cycles through enabled modes every 10 seconds:

1. **Clock Mode**: Shows current time in HH:MM format
2. **Weather Mode**: Scrolls weather information (temp, humidity, description)
3. **Star Wars Quotes**: Random quotes from Star Wars
4. **LOTR Quotes**: Random quotes from Lord of the Rings
5. **Custom Text**: User-defined scrolling text

## Troubleshooting

### Build Issues

If you encounter build errors:

1. Ensure submodules are initialized:
```bash
git submodule update --init --recursive
```

2. Clean and rebuild:
```bash
rm -rf build
idf.py build
```

3. Check ESP-IDF version:
```bash
cd third_party/esp-idf
git describe --tags
```
Should show `v5.5` or similar.

### Display Not Working

1. Check wiring connections
2. Verify GPIO pin numbers in `main/main.cpp` match your hardware
3. Test with a simple pattern:
   - Modify `main.cpp` to call `display.setBrightness(15)` for maximum brightness
   - Comment out the display controller and add test patterns

### WiFi Issues

1. Check credentials in the web UI
2. Monitor serial output for connection status
3. Reset WiFi configuration by erasing NVS:
```bash
idf.py erase-flash
idf.py flash
```

### Time Not Syncing

1. Ensure WiFi is connected
2. Check NTP server configuration in menuconfig
3. Monitor serial output for time sync messages

## License

This project is open source and available under the MIT License.

## Credits

- Project template based on SensorFusion architecture
- MAX7219 SPI driver
- ESP-IDF framework by Espressif

## Support

For issues, questions, or contributions, please open an issue on GitHub.
