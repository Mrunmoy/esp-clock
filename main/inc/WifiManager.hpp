/**
 * @file WifiManager.hpp
 * @brief WiFi connection and access point management
 *
 * This class handles all WiFi-related functionality including:
 * - Initializing WiFi and NVS (Non-Volatile Storage)
 * - Managing WiFi credentials
 * - Creating a configuration Access Point (AP)
 * - Connecting to saved WiFi networks
 */

#pragma once

#include <string>

/**
 * @class WifiManager
 * @brief Manages WiFi connectivity and configuration
 *
 * Provides static methods for WiFi initialization, credential storage,
 * and connection management. Supports both AP mode (for configuration)
 * and STA mode (for connecting to existing networks).
 */
class WifiManager
{
public:
	/**
	 * @brief Initialize WiFi subsystem and NVS
	 *
	 * Must be called before any other WiFi operations.
	 * Initializes NVS flash storage and ESP network interface.
	 */
	static void init();

	/**
	 * @brief Start the configuration Access Point
	 *
	 * Creates a WiFi AP with credentials from sdkconfig:
	 * - SSID: CONFIG_ESP_WIFI_SSID
	 * - Password: CONFIG_ESP_WIFI_PASSWORD
	 * - IP: 192.168.4.1
	 */
	static void startConfigAP();

	/**
	 * @brief Connect to previously saved WiFi network
	 *
	 * Attempts to connect using credentials stored in NVS.
	 * Does nothing if no credentials are saved.
	 */
	static void connectToConfiguredWiFi();

	/**
	 * @brief Save WiFi credentials to NVS
	 *
	 * @param ssid WiFi network name
	 * @param password WiFi password
	 * @return true if saved successfully, false otherwise
	 */
	static bool saveWiFiConfig(const char* ssid, const char* password);

	/**
	 * @brief Check if WiFi credentials are stored
	 *
	 * @return true if credentials exist in NVS, false otherwise
	 */
	static bool hasConfiguredWiFi();

	/**
	 * @brief Retrieve stored WiFi credentials
	 *
	 * @param ssid Buffer to store SSID
	 * @param password Buffer to store password
	 * @param max_len Maximum buffer size
	 */
	static void getConfiguredWiFi(char* ssid, char* password, size_t max_len);

	/**
	 * @brief Check WiFi connection status
	 *
	 * @return true if connected to WiFi, false otherwise
	 */
	static bool isConnected();

	/**
	 * @brief Block until WiFi connection is established
	 *
	 * Waits up to a maximum retry count for connection.
	 */
	static void waitForConnection();
};
