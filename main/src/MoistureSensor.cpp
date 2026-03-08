#include "MoistureSensor.hpp"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

namespace
{
	const char* TAG = "MoistureSensor";

	// GPIO4 = ADC1 channel 3 on ESP32-S3
	constexpr adc_channel_t ADC_CHANNEL = ADC_CHANNEL_3;
	constexpr int NUM_SAMPLES = 16;

	adc_oneshot_unit_handle_t adcHandle = nullptr;
}

uint8_t computeMoisturePercent(uint16_t rawAdc, uint16_t airValue, uint16_t waterValue)
{
	if (airValue == waterValue)
	{
		return 0;
	}

	// Higher ADC = drier soil = lower moisture percentage
	if (rawAdc >= airValue)
	{
		return 0;
	}
	if (rawAdc <= waterValue)
	{
		return 100;
	}

	int percent = 100 - ((int)(rawAdc - waterValue) * 100 / (int)(airValue - waterValue));

	if (percent < 0) percent = 0;
	if (percent > 100) percent = 100;

	return static_cast<uint8_t>(percent);
}

const char* getMoistureStatus(uint8_t percentage)
{
	if (percentage <= 20)
	{
		return "Very Dry - Water immediately!";
	}
	if (percentage <= 40)
	{
		return "Dry - Needs watering";
	}
	if (percentage <= 60)
	{
		return "Moist - Good";
	}
	if (percentage <= 80)
	{
		return "Wet - Well watered";
	}
	return "Very Wet - Reduce watering";
}

bool MoistureSensor::init()
{
	adc_oneshot_unit_init_cfg_t unitCfg = {};
	unitCfg.unit_id = ADC_UNIT_1;

	esp_err_t err = adc_oneshot_new_unit(&unitCfg, &adcHandle);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to init ADC unit: %s", esp_err_to_name(err));
		return false;
	}

	adc_oneshot_chan_cfg_t chanCfg = {};
	chanCfg.atten = ADC_ATTEN_DB_12;
	chanCfg.bitwidth = ADC_BITWIDTH_12;

	err = adc_oneshot_config_channel(adcHandle, ADC_CHANNEL, &chanCfg);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to configure ADC channel: %s", esp_err_to_name(err));
		return false;
	}

	ESP_LOGI(TAG, "ADC initialized on channel %d (GPIO4)", ADC_CHANNEL);
	return true;
}

uint16_t MoistureSensor::readRaw()
{
	if (adcHandle == nullptr)
	{
		ESP_LOGE(TAG, "ADC not initialized");
		return 0;
	}

	uint32_t sum = 0;
	int validSamples = 0;

	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		int raw = 0;
		esp_err_t err = adc_oneshot_read(adcHandle, ADC_CHANNEL, &raw);
		if (err == ESP_OK)
		{
			sum += raw;
			validSamples++;
		}
	}

	if (validSamples == 0)
	{
		ESP_LOGE(TAG, "All ADC reads failed");
		return 0;
	}

	return static_cast<uint16_t>(sum / validSamples);
}

MoistureReading MoistureSensor::read(uint16_t airValue, uint16_t waterValue)
{
	MoistureReading reading = {};
	reading.rawAdc = readRaw();
	reading.millivolts = 0;  // Simplified — raw ADC is sufficient for percentage
	reading.percentage = computeMoisturePercent(reading.rawAdc, airValue, waterValue);
	return reading;
}
