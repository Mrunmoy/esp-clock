#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

#include "WifiManager.hpp"
#include "WebServer.hpp"
#include "ConfigManager.hpp"
#include "MoistureSensor.hpp"

static const char* TAG = "main";

extern "C" void app_main(void)
{
	ESP_LOGI(TAG, "Soil Moisture Monitor starting...");

	// Initialize managers
	WifiManager::init();
	ConfigManager::init();

	// Check if WiFi is configured
	if (WifiManager::hasConfiguredWiFi())
	{
		ESP_LOGI(TAG, "Connecting to configured WiFi...");
		WifiManager::connectToConfiguredWiFi();
		WifiManager::waitForConnection();

		if (!WifiManager::isConnected())
		{
			ESP_LOGW(TAG, "Failed to connect to WiFi, starting AP mode");
			WifiManager::startConfigAP();
		}
		else
		{
			ESP_LOGI(TAG, "WiFi connected successfully");
		}
	}
	else
	{
		ESP_LOGI(TAG, "No WiFi configured, starting configuration AP");
		WifiManager::startConfigAP();
	}

	// Start web server
	WebServer::start();

	// Initialize moisture sensor
	if (!MoistureSensor::init())
	{
		ESP_LOGE(TAG, "Failed to initialize moisture sensor");
		return;
	}

	ESP_LOGI(TAG, "Soil Moisture Monitor initialization complete");

	// Load config
	MoistureConfig config;
	ConfigManager::loadConfig(config);

	// Main loop — read sensor and update web server
	while (true)
	{
		// Reload config each iteration for hot-reload from web UI
		ConfigManager::loadConfig(config);

		// Read sensor
		MoistureReading reading = MoistureSensor::read(config.airValue, config.waterValue);

		// Update web server with latest reading
		WebServer::updateMoistureReading(reading);

		ESP_LOGI(TAG, "Moisture: %u%% (raw=%u, status=%s)",
		         reading.percentage, reading.rawAdc,
		         getMoistureStatus(reading.percentage));

		vTaskDelay(pdMS_TO_TICKS(config.readIntervalMs));
	}
}
