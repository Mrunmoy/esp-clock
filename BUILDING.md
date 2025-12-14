# Building ESP-Clock Locally

This guide provides detailed instructions for building ESP-Clock from source using ESP-IDF 5.5.

## System Requirements

### Operating System
- Linux (Ubuntu 20.04+ recommended)
- macOS (10.15+)
- Windows (WSL2 or native)

### Software Requirements
- **Python**: 3.9 or newer
- **Git**: For cloning repositories
- **Build tools**: cmake, ninja, gcc

## Installation Steps

### 1. Install System Dependencies

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    git wget flex bison gperf python3 python3-pip python3-venv \
    cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
```

#### macOS
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake ninja dfu-util python3
```

#### Windows (WSL2)
```bash
# Enable WSL2 and install Ubuntu from Microsoft Store
# Then follow Ubuntu instructions above
```

### 2. Clone Repository

```bash
# Clone with submodules (ESP-IDF is included)
git clone --recursive <your-repo-url> esp-clock
cd esp-clock

# If you already cloned without --recursive:
git submodule update --init --recursive
```

### 3. Install ESP-IDF Tools

The project includes ESP-IDF 5.5 as a git submodule in `third_party/esp-idf`.

```bash
# Source the environment setup script
# This will:
# - Set IDF_PATH to third_party/esp-idf
# - Create Python virtual environment in third_party/esp-idf-tools
# - Install ESP-IDF Python dependencies
# - Download and install ESP32-S3 toolchain
source ./env.sh
```

**First Run**: The first time you run `env.sh`, it will download approximately 400MB of tools. This may take 5-10 minutes depending on your internet connection.

**Subsequent Runs**: After initial setup, `env.sh` completes in seconds.

### 4. Set Target Platform

ESP-Clock supports multiple ESP32 variants, but is primarily designed for ESP32-S3.

```bash
# Set target to ESP32-S3 (recommended)
idf.py set-target esp32s3

# Or for other targets:
idf.py set-target esp32    # Original ESP32
idf.py set-target esp32c3  # ESP32-C3 (RISC-V)
```

**Note**: Setting the target will clean the build directory and regenerate configuration.

### 5. Configure Project

Configure project-specific settings using menuconfig:

```bash
idf.py menuconfig
```

Navigate to **"ESP Clock Configuration"** and configure:

#### Required Settings
- **OpenWeather API Key**: Get free key from https://openweathermap.org/api
- **Weather City**: Your city name (e.g., "Sydney", "New York")
- **Weather Country Code**: Two-letter ISO code (e.g., "AU", "US", "GB")

#### Optional Settings
- **Timezone**: POSIX timezone string for your location
  - Sydney: `AEST-10AEDT,M10.1.0,M4.1.0/3`
  - New York: `EST5EDT,M3.2.0,M11.1.0`
  - London: `GMT0BST,M3.5.0/1,M10.5.0`
  - Los Angeles: `PST8PDT,M3.2.0,M11.1.0`
- **NTP Server**: Default is `pool.ntp.org`
- **WiFi AP Credentials**: Default SSID and password for configuration mode

### 6. Build

Build the project using the provided build script or idf.py directly:

```bash
# Option 1: Use build.sh (recommended)
./build.sh

# Option 2: Use idf.py directly
idf.py build

# Build for different target
./build.sh esp32
```

**Build Output**:
- Binary: `build/esp-clock.bin`
- Bootloader: `build/bootloader/bootloader.bin`
- Partition table: `build/partition_table/partition-table.bin`

## Flashing to Device

### 1. Connect ESP32-S3

Connect your ESP32-S3 board via USB. The device will appear as a serial port:
- Linux: `/dev/ttyUSB0` or `/dev/ttyACM0`
- macOS: `/dev/cu.usbserial-*` or `/dev/cu.usbmodem-*`
- Windows: `COM3`, `COM4`, etc.

### 2. Flash Firmware

```bash
# Flash and start serial monitor
idf.py -p /dev/ttyACM0 flash monitor

# Or flash only (no monitor)
idf.py -p /dev/ttyACM0 flash

# Press Ctrl+] to exit monitor
```

### 3. Monitor Serial Output

```bash
# Start monitor without flashing
idf.py -p /dev/ttyACM0 monitor
```

## Build Troubleshooting

### "Command 'idf.py' not found"

**Solution**: Source the environment script first
```bash
source ./env.sh
idf.py build
```

### "Submodule 'third_party/esp-idf' not initialized"

**Solution**: Initialize git submodules
```bash
git submodule update --init --recursive
```

### "Python version too old"

**Solution**: Ensure Python 3.9+ is installed
```bash
python3 --version  # Should show 3.9 or higher

# Ubuntu: install python3.9
sudo apt-get install python3.9 python3.9-venv

# macOS: use Homebrew
brew install python@3.9
```

### "Target does not match"

**Solution**: Clean and rebuild with correct target
```bash
rm -rf build
idf.py set-target esp32s3
idf.py build
```

### Build succeeds but flash fails

**Solution**: Check serial port and permissions
```bash
# List available ports
ls -l /dev/tty*

# Add user to dialout group (Linux)
sudo usermod -a -G dialout $USER
# Log out and log back in

# Try different baud rate
idf.py -p /dev/ttyACM0 -b 115200 flash
```

### "No space left on device" during build

**Solution**: Clean build directory
```bash
rm -rf build
idf.py build
```

## Build Configuration Files

### Important Files

- `CMakeLists.txt` - Root build configuration
- `main/CMakeLists.txt` - Main component configuration
- `main/Kconfig.projbuild` - Custom configuration menu
- `sdkconfig` - Build configuration (generated)
- `env.sh` - Environment setup script
- `build.sh` - Build wrapper script

### Cleaning Build Artifacts

```bash
# Clean build directory
rm -rf build

# Clean and rebuild
idf.py fullclean
idf.py build

# Erase flash (removes NVS data)
idf.py erase-flash
```

## Advanced Build Options

### Optimization Levels

```bash
# Debug build (default)
idf.py build

# Release build with optimizations
idf.py menuconfig
# Navigate to: Compiler options → Optimization Level → Optimize for performance (-O2)
idf.py build
```

### Partition Table

Default partition layout:
```
# Name,   Type, SubType, Offset,  Size
nvs,      data, nvs,     0x9000,  24K
phy_init, data, phy,     0xf000,  4K
factory,  app,  factory, 0x10000, 1M
```

To customize, edit `partitions.csv` and set in menuconfig:
- Partition Table → Custom partition table CSV

### Build Verbosity

```bash
# Verbose build output
idf.py -v build

# Very verbose (shows all commands)
idf.py build -v
```

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build ESP-Clock

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v3
      with:
        submodules: 'recursive'

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y git wget flex bison gperf python3 \
          python3-pip python3-venv cmake ninja-build ccache \
          libffi-dev libssl-dev dfu-util libusb-1.0-0

    - name: Build firmware
      run: |
        source ./env.sh
        idf.py set-target esp32s3
        idf.py build

    - name: Upload artifacts
      uses: actions/upload-artifact@v3
      with:
        name: firmware
        path: build/esp-clock.bin
```

## Docker Build (Alternative)

A Dockerfile is included for containerized builds:

```bash
# Build Docker image
docker build -t esp-clock-builder .

# Build firmware in container
./docker-build.sh

# Output will be in build/ directory
```

## Next Steps

After successful build and flash:
1. Device will create WiFi AP: `ESP_CLOCK_AP`
2. Connect and navigate to http://192.168.4.1
3. Configure your WiFi credentials
4. Device reboots and connects to your network
5. Access web UI at device's IP to configure display modes

For more information, see:
- [README.md](README.md) - General project information
- [ARCHITECTURE.md](ARCHITECTURE.md) - Software architecture details
