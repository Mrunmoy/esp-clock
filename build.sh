#!/bin/bash

#
# Build script for ESP-Clock project
# Supports ESP32-S3 target with automatic environment setup
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Target chip (default: esp32s3)
TARGET="${1:-esp32s3}"

echo -e "${GREEN}ESP-Clock Build Script${NC}"
echo "Target: $TARGET"
echo ""

# Source environment
echo -e "${YELLOW}Setting up ESP-IDF environment...${NC}"
source ./env.sh

# Check if target is set correctly
CURRENT_TARGET=$(idf.py --version 2>/dev/null | grep "IDF_TARGET" | awk '{print $2}' || echo "")
if [ -z "$CURRENT_TARGET" ] || [ "$CURRENT_TARGET" != "$TARGET" ]; then
    echo -e "${YELLOW}Setting target to $TARGET...${NC}"
    idf.py set-target $TARGET
fi

# Clean build (optional, uncomment if needed)
# echo -e "${YELLOW}Cleaning build directory...${NC}"
# rm -rf build

# Build the project
echo -e "${YELLOW}Building project...${NC}"
idf.py build

# Print success message
echo ""
echo -e "${GREEN}✓ Build completed successfully!${NC}"
echo ""
echo "To flash to your device:"
echo "  idf.py -p /dev/ttyUSB0 flash monitor"
echo ""
echo "Or use the flash helper:"
echo "  idf.py -p /dev/ttyUSB0 flash && idf.py -p /dev/ttyUSB0 monitor"
