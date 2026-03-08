/**
 * @file ConfigManager.hpp
 * @brief Moisture sensor configuration management and persistence
 *
 * Manages calibration values and sensor settings. Configuration is
 * stored in NVS for persistence across reboots.
 */

#pragma once

#include <cstdint>

/**
 * @struct MoistureConfig
 * @brief Configuration for the soil moisture sensor
 */
struct MoistureConfig
{
	uint16_t airValue;         ///< ADC reading in dry air (0% moisture calibration)
	uint16_t waterValue;       ///< ADC reading in water (100% moisture calibration)
	uint32_t readIntervalMs;   ///< Sensor read interval in milliseconds
};

/**
 * @class ConfigManager
 * @brief Manages sensor configuration storage and retrieval
 */
class ConfigManager
{
public:
	static void init();
	static bool saveConfig(const MoistureConfig& config);
	static bool loadConfig(MoistureConfig& config);
	static void getDefaultConfig(MoistureConfig& config);
};
