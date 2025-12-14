# ESP-Clock Software Architecture

## Overview

ESP-Clock is a modular IoT clock built on ESP32-S3, featuring a clean separation of concerns across hardware interfacing, network connectivity, data management, and display control.

## System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         ESP-Clock System                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌────────────┐  ┌──────────────┐  ┌─────────────────────────┐  │
│  │   main.cpp │──│ Display      │──│ MAX7219 LED Matrix      │  │
│  │   (entry)  │  │ Controller   │  │ Hardware (5 devices)    │  │
│  └────────────┘  └──────────────┘  └─────────────────────────┘  │
│         │                │                                      │
│         │                ├── DisplayManager (rendering)         │
│         │                ├── WeatherFetcher (API calls)         │
│         │                ├── Quotes (content database)          │
│         │                └── ConfigManager (user prefs)         │
│         │                                                       │
│         ├── WiFiManager ────┬── NVS Storage                     │
│         │                   └── ESP Network Interface           │
│         │                                                       │
│         ├── WebServer ──────── HTTP Server + REST API           │
│         │                                                       │
│         └── TimeSync ────────── SNTP Client                     │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

External Dependencies:
  - OpenWeatherMap API (weather data)
  - NTP Server (time synchronization)
  - User WiFi Network (internet connectivity)
```

## Component Design

### 1. Main Application (`main.cpp`)

**Responsibility**: Application initialization and main event loop

**Key Functions**:
- Initialize all subsystems in correct order
- Create and start display controller
- Run main update loop

**Data Flow**:
```
Boot → Init WiFi → Check WiFi Config → Connect/Start AP
  → Init Time Sync → Init Display → Start Web Server
  → Enter Main Loop (100ms cycle)
```

### 2. WiFi Manager

**Responsibility**: Network connectivity and credential management

**Modes**:
1. **AP Mode** (Configuration):
   - SSID: `ESP_CLOCK_AP`
   - IP: `192.168.4.1`
   - Used for initial setup

2. **STA Mode** (Station):
   - Connects to saved WiFi network
   - Enables internet access for NTP and weather

**Storage**: WiFi credentials in NVS namespace "wifi"

### 3. Web Server

**Responsibility**: HTTP server for configuration UI

**Endpoints**:
- `GET /` - Serve configuration web page
- `GET /api/config` - Get current display configuration
- `POST /api/config` - Save display configuration
- `POST /api/wifi` - Update WiFi credentials and reboot

**Architecture**:
- Embedded HTML/CSS/JS (compiled into firmware)
- RESTful JSON API
- Asynchronous request handling

### 4. Configuration Manager

**Responsibility**: Persistent storage of user preferences

**Storage Structure** (NVS namespace "storage"):
```
show_clock: uint8_t (0/1)
show_weather: uint8_t (0/1)
show_sw: uint8_t (0/1)  // Star Wars quotes
show_lotr: uint8_t (0/1)  // LOTR quotes
custom_text: string (max 256 bytes)
```

**Default Configuration**:
- Clock: Enabled
- Weather: Disabled
- Quotes: Disabled
- Custom Text: Empty

### 5. Time Synchronization

**Responsibility**: Network time protocol synchronization

**Configuration**:
- NTP Server: Configurable via menuconfig (default: `pool.ntp.org`)
- Timezone: POSIX timezone string
- Update Interval: Automatic (SNTP handles)

**Implementation**:
- Uses ESP-IDF SNTP component (lwip)
- Callbacks for sync notification
- Timezone-aware local time conversion

### 6. Weather Fetcher

**Responsibility**: Retrieve and parse weather data

**API**: OpenWeatherMap Current Weather API
```
GET http://api.openweathermap.org/data/2.5/weather
  ?q={city},{country_code}
  &appid={api_key}
  &units=metric
```

**Update Frequency**: Once per hour (3600000 ms)

**Data Structure**:
```cpp
struct WeatherData {
    float temperature;    // Celsius
    int humidity;         // Percentage
    char description[64]; // Weather description
    char icon[8];         // Icon code
    bool valid;           // Data validity flag
};
```

**Error Handling**:
- HTTP timeout: 5 seconds
- Invalid JSON: Logs error, returns invalid data
- No API key: Warns and skips fetch

### 7. Display Controller

**Responsibility**: Orchestrate display modes and content switching

**Mode Switching**:
- Interval: 10 seconds (MODE_SWITCH_INTERVAL_MS)
- Cycles through enabled modes only
- No switching if only one mode enabled

**Update Loop**:
```
Every 100ms:
  1. Reload configuration (hot reload from web changes)
  2. Check if weather needs update (hourly)
  3. Determine active mode count
  4. Check if mode switch needed (10s elapsed)
  5. Render current mode to display
```

**Mode Implementations**:
- **Clock**: Static display, updates every 1 second
- **Weather**: Scrolling text with temp/humidity/description
- **Quotes**: Scrolling random quote
- **Custom**: Scrolling user text

### 8. Display Manager

**Responsibility**: Text rendering and scrolling control

**Features**:
- 5x7 bitmap font rendering
- Horizontal text scrolling
- Multi-device coordinate mapping
- Time display formatting (HH:MM with colon separator)

**Coordinate System**:
```
Device 0   Device 1   Device 2   Device 3   Device 4
[0-7]      [8-15]     [16-23]    [24-31]    [32-39]
   ↑          ↑          ↑          ↑          ↑
Column positions (40 columns total, 8 rows)
```

### 9. MAX7219 Driver

**Responsibility**: Low-level SPI communication with LED matrix

**SPI Configuration**:
- Clock: 1 MHz
- Mode: 0 (CPOL=0, CPHA=0)
- Bit Order: MSB first

**Commands**:
- Initialize: Set scan limit, decode mode, shutdown mode
- Set brightness: 0-15 intensity levels
- Write data: Per-device row updates
- Clear: All LEDs off

**Memory Layout**:
```
display_buffer_[device][row] = 8-bit column data
  device: 0-4 (5 devices)
  row: 0-7 (8 rows)
  each bit = one LED pixel
```

## Threading Model

ESP-Clock uses FreeRTOS tasks:

### Main Task
- Priority: Default
- Stack: 4KB
- Runs main event loop
- Updates display every 100ms

### HTTP Server Task
- Priority: Default
- Stack: 4KB (configurable)
- Handles incoming HTTP requests
- Created by ESP-IDF HTTP server

### WiFi Task
- Priority: High
- Stack: 3KB
- ESP-IDF system task
- Handles WiFi events and connections

### SNTP Task
- Priority: Default
- Stack: 2KB
- ESP-IDF system task
- Periodic NTP synchronization

## Memory Management

### Flash Memory (Partitions)
```
Bootloader:  64KB  (0x1000)
Partition:   32KB  (0x8000)
App:         1MB   (0x10000)
NVS:         24KB  (factory layout)
```

### RAM Usage
- Static: ~50KB (buffers, globals)
- Heap: ~200KB available
- Stack: ~20KB (all tasks)
- DMA: Minimal (SPI driver)

### NVS Storage
- WiFi credentials: ~100 bytes
- Display config: ~300 bytes
- Total used: <1KB

## Data Flow Diagrams

### Startup Sequence
```
Power On
  ↓
Initialize NVS & WiFi
  ↓
WiFi Configured? ──No──> Start AP Mode (192.168.4.1)
  ↓ Yes                         ↓
Connect to WiFi               Start Web Server
  ↓                                ↓
Connected? ──No──────────────────┘
  ↓ Yes
Initialize Time Sync
  ↓
Sync Time with NTP
  ↓
Initialize Display Hardware
  ↓
Create Display Controller
  ↓
Start Main Loop
```

### Configuration Update Flow
```
User opens Web UI (http://IP)
  ↓
Web Server serves index.html
  ↓
User changes settings
  ↓
JavaScript sends POST /api/config
  ↓
Web Server receives JSON
  ↓
ConfigManager saves to NVS
  ↓
Response: "OK"
  ↓
Display Controller reloads config (next loop)
  ↓
Display updates with new modes
```

### Weather Update Flow
```
Main Loop checks timer
  ↓
1 hour elapsed?
  ↓ Yes
WeatherFetcher::fetchWeather()
  ↓
HTTP GET to OpenWeatherMap API
  ↓
Parse JSON response
  ↓
Extract temp, humidity, description
  ↓
Store in WeatherData struct
  ↓
DisplayController uses data
  ↓
Scroll weather text on display
```

## Configuration Management

### Build-Time Configuration (sdkconfig)
- WiFi AP credentials
- OpenWeather API key
- City and country
- Timezone string
- NTP server

### Runtime Configuration (NVS)
- Display modes enabled/disabled
- Custom text content
- WiFi STA credentials

### Hot Reload
ConfigManager is called every display loop iteration, allowing real-time configuration changes without reboot (except WiFi changes).

## Error Handling Strategy

1. **WiFi Connection Failures**:
   - Retry 5 times with backoff
   - Fall back to AP mode
   - User can reconfigure via web UI

2. **Weather API Errors**:
   - Log error
   - Keep previous weather data
   - Display "N/A" if never fetched
   - Retry on next cycle (1 hour)

3. **Time Sync Failures**:
   - Display "--:--" placeholder
   - Keep retrying in background
   - Works offline once synced initially

4. **NVS Errors**:
   - Use default configuration
   - Log warning
   - Continue operation

5. **Display Hardware Failures**:
   - Log error
   - Exit app_main (ESP will reboot)
   - Watchdog will recover if needed

## Performance Characteristics

- **Boot Time**: ~5 seconds (WiFi connection)
- **API Response**: <1 second (OpenWeather)
- **Display Update**: 10 FPS (100ms loop)
- **Scroll Speed**: Configurable (default 50ms/column)
- **Memory Footprint**: ~70KB RAM, ~900KB Flash
- **Power Consumption**: ~200mA @ 5V (LEDs on)

## Security Considerations

1. **WiFi Credentials**: Stored encrypted in NVS
2. **API Keys**: Compiled into firmware (consider hardware encryption)
3. **Web UI**: No authentication (local network only)
4. **HTTPS**: Not implemented (consider for production)

## Future Enhancements

Potential improvements:
- OTA (Over-The-Air) firmware updates
- HTTPS support for web UI
- Weather forecast (not just current)
- Multiple timezone support
- Brightness auto-adjustment (light sensor)
- Sound/alarm features
- MQTT integration
