/**
 * @file ConfigManager.hpp
 * @brief Display configuration management and persistence
 *
 * Manages user preferences for what content to display on the LED matrix,
 * including clock, weather, quotes, and custom text. Configuration is
 * stored in NVS for persistence across reboots.
 */

#pragma once

#include <cstdint>

/**
 * @struct DisplayConfig
 * @brief Configuration for display content modes
 *
 * Defines which content modes are enabled. The display cycles through
 * all enabled modes every 10 seconds.
 */
struct DisplayConfig
{
	bool showClock;            ///< Display current time
	bool showWeather;          ///< Display weather information
	bool showStarWarsQuotes;   ///< Display Star Wars quotes
	bool showLOTRQuotes;       ///< Display Lord of the Rings quotes
	char customText[256];      ///< Custom user-defined text to scroll
};

/**
 * @class ConfigManager
 * @brief Manages display configuration storage and retrieval
 *
 * Provides methods to save and load display preferences to/from NVS.
 * Configuration persists across device reboots.
 */
class ConfigManager
{
public:
	/**
	 * @brief Initialize configuration manager
	 *
	 * Currently a placeholder as NVS is initialized by WifiManager.
	 */
	static void init();

	/**
	 * @brief Save display configuration to NVS
	 *
	 * @param config Configuration to save
	 * @return true if saved successfully, false on error
	 */
	static bool saveConfig(const DisplayConfig& config);

	/**
	 * @brief Load display configuration from NVS
	 *
	 * If no configuration exists in NVS, loads default configuration.
	 *
	 * @param config Output parameter for loaded configuration
	 * @return true if loaded from NVS, false if using defaults
	 */
	static bool loadConfig(DisplayConfig& config);

	/**
	 * @brief Get default display configuration
	 *
	 * Default: Clock enabled, all other modes disabled.
	 *
	 * @param config Output parameter for default configuration
	 */
	static void getDefaultConfig(DisplayConfig& config);
};
