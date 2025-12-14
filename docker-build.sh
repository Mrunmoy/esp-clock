#!/bin/bash

# Build script using Docker for esp-clock project

set -e

echo "Building esp-clock using Docker..."

# Check if running with sudo or if user is in docker group
if ! docker ps > /dev/null 2>&1; then
    echo "Docker permission issue detected. Trying with sudo..."
    DOCKER_CMD="sudo docker"
else
    DOCKER_CMD="docker"
fi

# Build the Docker image
$DOCKER_CMD build -t esp-clock-builder .

# Run the build
$DOCKER_CMD run --rm -v "$(pwd)":/project esp-clock-builder

echo "Build complete! Output is in the build/ directory"
