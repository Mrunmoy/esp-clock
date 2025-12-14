#!/bin/bash

# Set IDF_PATH to local ESP-IDF submodule
export IDF_PATH=$(pwd)/third_party/esp-idf

# Set IDF_TOOLS_PATH for tools installation
export IDF_TOOLS_PATH=$(pwd)/third_party/esp-idf-tools

# Source ESP-IDF environment
. ${IDF_PATH}/export.sh
