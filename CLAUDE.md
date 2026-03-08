# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP-Clock is an ESP32-S3 IoT clock that drives 5 MAX7219 8x8 LED matrix modules via SPI. It displays time (NTP-synced), weather (OpenWeatherMap), quotes, and custom scrolling text. Configuration is done through a web UI served from embedded HTML.

## Build Commands

```bash
# Set up ESP-IDF environment (required before any idf.py command)
source ./env.sh

# Build (sets target to esp32s3 automatically)
./build.sh

# Build for a different target
./build.sh esp32

# Build with idf.py directly
idf.py build

# Clean build
rm -rf build && idf.py build

# Configure project settings (WiFi, API keys, timezone)
idf.py menuconfig  # Navigate to "ESP Clock Configuration"

# Flash and monitor
idf.py -p /dev/ttyACM0 flash monitor

# Docker build (no local ESP-IDF needed)
./docker-build.sh
```

There are no unit tests in this project.

## Architecture

The app runs on FreeRTOS with a 100ms main loop in `app_main()` (`main/main.cpp`). All components use static class methods (no instantiation except MAX7219, DisplayManager, DisplayController).

**Startup flow:** Init WiFi → connect or start AP → init TimeSync → init MAX7219 display → start WebServer → enter main loop calling `displayController.updateDisplay()`.

**Key component relationships:**
- `DisplayController` orchestrates mode switching (10s interval) and delegates rendering to `DisplayManager`
- `DisplayManager` handles font rendering and scrolling on the 40-column (5×8) display surface via `MAX7219`
- `ConfigManager` reads/writes NVS storage; config is hot-reloaded every loop iteration
- `WebServer` serves embedded `index.html` and exposes REST endpoints (`GET/POST /api/config`, `POST /api/wifi`)
- `WeatherFetcher` calls OpenWeatherMap API hourly

**Hardware:** SPI to MAX7219 chain on GPIO 12 (CLK), 11 (MOSI), 10 (CS). Display buffer is `display_buffer_[device][row]` where each byte is 8 LED columns.

## Code Style

- C++17, Allman brace style, tabs for indentation (width 4)
- `.clang-format` is present — run `clang-format` to auto-format
- Headers in `main/inc/`, implementations in `main/src/`
- ESP-IDF logging macros (`ESP_LOGI`, `ESP_LOGW`, `ESP_LOGE`) with a static `TAG` per file

## Configuration

Build-time settings are in `main/Kconfig.projbuild` (OpenWeather API key, city, timezone, WiFi AP credentials). Runtime settings (display modes, custom text, flip, brightness) are stored in NVS and managed by `ConfigManager`.

## Dependencies

ESP-IDF v5.5 is pinned as a git submodule at `third_party/esp-idf`. The `env.sh` script sets `IDF_PATH` and `IDF_TOOLS_PATH` and sources ESP-IDF's `export.sh`. First run installs ~400MB of toolchain files to `third_party/esp-idf-tools`.
