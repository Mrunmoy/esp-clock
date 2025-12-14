#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

#include "WifiManager.hpp"
#include "WebServer.hpp"
#include "ConfigManager.hpp"
#include "TimeSync.hpp"
#include "MAX7219.hpp"
#include "DisplayManager.hpp"
#include "DisplayController.hpp"

static const char* TAG = "main";

// GPIO pin definitions for ESP32-S3 and MAX7219
#define MAX7219_CLK_PIN   GPIO_NUM_12
#define MAX7219_MOSI_PIN  GPIO_NUM_11
#define MAX7219_CS_PIN    GPIO_NUM_10

extern "C" void app_main(void)
{
	ESP_LOGI(TAG, "ESP Clock starting...");

	// Initialize managers
	WifiManager::init();
	ConfigManager::init();

	// Check if WiFi is configured
	if (WifiManager::hasConfiguredWiFi())
	{
		ESP_LOGI(TAG, "Connecting to configured WiFi...");
		WifiManager::connectToConfiguredWiFi();
		WifiManager::waitForConnection();

		if (WifiManager::isConnected())
		{
			ESP_LOGI(TAG, "WiFi connected successfully");

			// Initialize time sync
			TimeSync::init();
			TimeSync::syncTime();
		}
		else
		{
			ESP_LOGW(TAG, "Failed to connect to WiFi, starting AP mode");
			WifiManager::startConfigAP();
		}
	}
	else
	{
		ESP_LOGI(TAG, "No WiFi configured, starting configuration AP");
		WifiManager::startConfigAP();
	}

	// Start web server
	WebServer::start();

	// Initialize MAX7219 display
	MAX7219 display(5);  // 5 devices in series
	if (!display.init(MAX7219_CLK_PIN, MAX7219_MOSI_PIN, MAX7219_CS_PIN))
	{
		ESP_LOGE(TAG, "Failed to initialize MAX7219 display");
		return;
	}

	display.setBrightness(8);  // Medium brightness
	ESP_LOGI(TAG, "MAX7219 display initialized");

	// Create display manager and controller
	DisplayManager displayManager(&display);
	DisplayController displayController(&displayManager);

	// Show startup message
	displayManager.scrollText("ESP-Clock v1.0", 50);

	// Start display controller
	displayController.start();

	ESP_LOGI(TAG, "ESP Clock initialization complete");

	// Main loop
	while (true)
	{
		displayController.updateDisplay();
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
