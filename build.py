#!/usr/bin/env python3
"""
Build script for the Soil Moisture Monitor project.

Usage:
    python3 build.py                 # Build for esp32s3 (default)
    python3 build.py --target esp32  # Build for a different target
    python3 build.py --clean         # Clean build directory first
    python3 build.py --flash         # Build and flash
    python3 build.py --flash --port /dev/ttyACM0  # Flash to specific port
    python3 build.py --monitor       # Build, flash, and monitor
    python3 build.py --test          # Run host unit tests only
"""

import argparse
import os
import subprocess
import sys


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
IDF_PATH = os.path.join(SCRIPT_DIR, "third_party", "esp-idf")
IDF_TOOLS_PATH = os.path.join(SCRIPT_DIR, "third_party", "esp-idf-tools")
DEFAULT_TARGET = "esp32s3"
DEFAULT_PORT = "/dev/ttyACM0"


def run(cmd, check=True, env=None):
    """Run a shell command and stream output."""
    print(f"\n>>> {cmd}")
    result = subprocess.run(cmd, shell=True, env=env or os.environ, cwd=SCRIPT_DIR)
    if check and result.returncode != 0:
        print(f"Command failed with exit code {result.returncode}")
        sys.exit(result.returncode)
    return result


def setup_env():
    """Source env.sh and return the resulting environment."""
    cmd = f'bash -c "source {SCRIPT_DIR}/env.sh && env"'
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True, cwd=SCRIPT_DIR)
    if result.returncode != 0:
        print("Failed to source env.sh")
        print(result.stderr)
        sys.exit(1)

    env = os.environ.copy()
    for line in result.stdout.splitlines():
        if "=" in line:
            key, _, value = line.partition("=")
            env[key] = value
    return env


def run_tests():
    """Compile and run host-side unit tests."""
    print("=== Running unit tests ===")
    test_src = os.path.join(SCRIPT_DIR, "test", "test_moisture.cpp")
    test_bin = os.path.join(SCRIPT_DIR, "test_moisture")

    result = run(f"g++ -std=c++17 -o {test_bin} {test_src} -DUNIT_TEST", check=False)
    if result.returncode != 0:
        print("Test compilation failed")
        sys.exit(1)

    result = run(test_bin, check=False)
    if os.path.exists(test_bin):
        os.remove(test_bin)
    if result.returncode != 0:
        print("Tests FAILED")
        sys.exit(1)
    print("All tests passed!")


def main():
    parser = argparse.ArgumentParser(description="Soil Moisture Monitor build script")
    parser.add_argument("--target", default=DEFAULT_TARGET,
                        help=f"Target chip (default: {DEFAULT_TARGET})")
    parser.add_argument("--clean", action="store_true",
                        help="Clean build directory before building")
    parser.add_argument("--flash", action="store_true",
                        help="Flash firmware after building")
    parser.add_argument("--monitor", action="store_true",
                        help="Flash and start serial monitor")
    parser.add_argument("--port", default=DEFAULT_PORT,
                        help=f"Serial port for flashing (default: {DEFAULT_PORT})")
    parser.add_argument("--test", action="store_true",
                        help="Run host unit tests only (no firmware build)")
    args = parser.parse_args()

    if args.test:
        run_tests()
        return

    # Run tests first
    run_tests()

    print(f"\n=== Building for {args.target} ===")

    env = setup_env()

    if args.clean:
        print("Cleaning build directory...")
        run("rm -rf build", env=env)

    run(f"idf.py set-target {args.target}", env=env)
    run("idf.py build", env=env)

    print("\nBuild completed successfully!")

    if args.flash or args.monitor:
        flash_cmd = f"idf.py -p {args.port} flash"
        if args.monitor:
            flash_cmd += " monitor"
        run(flash_cmd, env=env)


if __name__ == "__main__":
    main()
