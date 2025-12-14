#include "ConfigManager.hpp"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <cstring>

namespace
{
	const char* TAG = "ConfigManager";
}

void ConfigManager::init()
{
	// NVS should already be initialized by WifiManager
}

bool ConfigManager::saveConfig(const DisplayConfig& config)
{
	nvs_handle_t nvsHandle;
	esp_err_t err;

	err = nvs_open("storage", NVS_READWRITE, &nvsHandle);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
		return false;
	}

	err = nvs_set_u8(nvsHandle, "show_clock", config.showClock ? 1 : 0);
	if (err != ESP_OK) goto error;

	err = nvs_set_u8(nvsHandle, "show_weather", config.showWeather ? 1 : 0);
	if (err != ESP_OK) goto error;

	err = nvs_set_u8(nvsHandle, "show_sw", config.showStarWarsQuotes ? 1 : 0);
	if (err != ESP_OK) goto error;

	err = nvs_set_u8(nvsHandle, "show_lotr", config.showLOTRQuotes ? 1 : 0);
	if (err != ESP_OK) goto error;

	err = nvs_set_str(nvsHandle, "custom_text", config.customText);
	if (err != ESP_OK) goto error;

	err = nvs_commit(nvsHandle);
	if (err != ESP_OK) goto error;

	nvs_close(nvsHandle);
	ESP_LOGI(TAG, "Display config saved successfully");
	return true;

error:
	ESP_LOGE(TAG, "Error saving config: %s", esp_err_to_name(err));
	nvs_close(nvsHandle);
	return false;
}

bool ConfigManager::loadConfig(DisplayConfig& config)
{
	nvs_handle_t nvsHandle;
	esp_err_t err;

	err = nvs_open("storage", NVS_READONLY, &nvsHandle);
	if (err != ESP_OK)
	{
		ESP_LOGW(TAG, "Error opening NVS handle, using defaults: %s", esp_err_to_name(err));
		getDefaultConfig(config);
		return false;
	}

	uint8_t val;
	err = nvs_get_u8(nvsHandle, "show_clock", &val);
	config.showClock = (err == ESP_OK) ? (val != 0) : true;

	err = nvs_get_u8(nvsHandle, "show_weather", &val);
	config.showWeather = (err == ESP_OK) ? (val != 0) : false;

	err = nvs_get_u8(nvsHandle, "show_sw", &val);
	config.showStarWarsQuotes = (err == ESP_OK) ? (val != 0) : false;

	err = nvs_get_u8(nvsHandle, "show_lotr", &val);
	config.showLOTRQuotes = (err == ESP_OK) ? (val != 0) : false;

	size_t textLen = sizeof(config.customText);
	err = nvs_get_str(nvsHandle, "custom_text", config.customText, &textLen);
	if (err != ESP_OK)
	{
		config.customText[0] = '\0';
	}

	nvs_close(nvsHandle);
	return true;
}

void ConfigManager::getDefaultConfig(DisplayConfig& config)
{
	config.showClock = true;
	config.showWeather = false;
	config.showStarWarsQuotes = false;
	config.showLOTRQuotes = false;
	config.customText[0] = '\0';
}
