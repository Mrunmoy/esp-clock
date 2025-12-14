#!/bin/bash

# Build script for esp-clock project

set -e

# Source environment
source ./env.sh

# Build the project
idf.py build
