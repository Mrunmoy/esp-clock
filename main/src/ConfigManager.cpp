#include "ConfigManager.hpp"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

namespace
{
	const char* TAG = "ConfigManager";
}

void ConfigManager::init()
{
	// NVS is initialized by WifiManager
}

bool ConfigManager::saveConfig(const MoistureConfig& config)
{
	nvs_handle_t nvsHandle;
	esp_err_t err;

	err = nvs_open("storage", NVS_READWRITE, &nvsHandle);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
		return false;
	}

	err = nvs_set_u16(nvsHandle, "air_value", config.airValue);
	if (err != ESP_OK) goto error;

	err = nvs_set_u16(nvsHandle, "water_value", config.waterValue);
	if (err != ESP_OK) goto error;

	err = nvs_set_u32(nvsHandle, "read_interval", config.readIntervalMs);
	if (err != ESP_OK) goto error;

	err = nvs_commit(nvsHandle);
	if (err != ESP_OK) goto error;

	nvs_close(nvsHandle);
	ESP_LOGI(TAG, "Config saved (air=%u, water=%u, interval=%lu ms)",
	         config.airValue, config.waterValue, (unsigned long)config.readIntervalMs);
	return true;

error:
	ESP_LOGE(TAG, "Error saving config: %s", esp_err_to_name(err));
	nvs_close(nvsHandle);
	return false;
}

bool ConfigManager::loadConfig(MoistureConfig& config)
{
	nvs_handle_t nvsHandle;
	esp_err_t err;

	err = nvs_open("storage", NVS_READONLY, &nvsHandle);
	if (err != ESP_OK)
	{
		ESP_LOGW(TAG, "Error opening NVS, using defaults: %s", esp_err_to_name(err));
		getDefaultConfig(config);
		return false;
	}

	uint16_t u16val;
	uint32_t u32val;

	err = nvs_get_u16(nvsHandle, "air_value", &u16val);
	config.airValue = (err == ESP_OK) ? u16val : 3000;

	err = nvs_get_u16(nvsHandle, "water_value", &u16val);
	config.waterValue = (err == ESP_OK) ? u16val : 1500;

	err = nvs_get_u32(nvsHandle, "read_interval", &u32val);
	config.readIntervalMs = (err == ESP_OK) ? u32val : 2000;

	nvs_close(nvsHandle);
	return true;
}

void ConfigManager::getDefaultConfig(MoistureConfig& config)
{
	config.airValue = 3000;
	config.waterValue = 1500;
	config.readIntervalMs = 2000;
}
