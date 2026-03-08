#include "MoistureSensor.hpp"

#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#include <cstring>

namespace
{
	const char* TAG = "MoistureSensor";

	/// ADC channel wired to the moisture sensor output.
	/// Configurable via CONFIG_MOISTURE_SENSOR_ADC_CHANNEL (Kconfig).
	constexpr adc_channel_t kAdcChannel =
	    static_cast<adc_channel_t>(CONFIG_MOISTURE_SENSOR_ADC_CHANNEL);

	/// 12-bit ADC full-scale value.
	constexpr float kAdcMax = 4095.0f;

	/// Convert a raw ADC count to a moisture percentage.
	/// Most resistive/capacitive probes produce a high voltage when dry and a
	/// low voltage when wet, so the conversion is inverted.
	float adcToMoisture(int raw)
	{
		float clamped = static_cast<float>(raw < 0 ? 0 : (raw > 4095 ? 4095 : raw));
		return (1.0f - clamped / kAdcMax) * 100.0f;
	}
}

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

MoistureSensor::MoistureSensor(uint32_t intervalMs)
    : m_intervalMs(intervalMs)
{
	m_mutex = xSemaphoreCreateMutex();
}

MoistureSensor::~MoistureSensor()
{
	stop();
	if (m_mutex)
	{
		vSemaphoreDelete(m_mutex);
		m_mutex = nullptr;
	}
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

void MoistureSensor::addObserver(ISensorObserver* observer)
{
	if (observer)
	{
		m_observers.push_back(observer);
	}
}

void MoistureSensor::start()
{
	if (m_taskHandle)
	{
		ESP_LOGW(TAG, "Sensor task already running");
		return;
	}
	xTaskCreate(taskFunction, "moisture_task", 2048, this, 5, &m_taskHandle);
	ESP_LOGI(TAG, "Moisture sensor task started (interval %lu ms)", m_intervalMs);
}

void MoistureSensor::stop()
{
	if (m_taskHandle)
	{
		vTaskDelete(m_taskHandle);
		m_taskHandle = nullptr;
		ESP_LOGI(TAG, "Moisture sensor task stopped");
	}
}

MoistureReading MoistureSensor::getLastReading() const
{
	MoistureReading reading{};
	if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
	{
		reading = m_lastReading;
		xSemaphoreGive(m_mutex);
	}
	return reading;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void MoistureSensor::taskFunction(void* arg)
{
	MoistureSensor* self = static_cast<MoistureSensor*>(arg);
	while (true)
	{
		self->readAndPublish();
		vTaskDelay(pdMS_TO_TICKS(self->m_intervalMs));
	}
}

void MoistureSensor::readAndPublish()
{
	MoistureReading reading;
	reading.moisture = readRaw();
	reading.valid    = true;

	// Store thread-safely
	if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
	{
		m_lastReading = reading;
		xSemaphoreGive(m_mutex);
	}

	ESP_LOGI(TAG, "Moisture: %.1f%%", reading.moisture);

	// Notify all registered observers (push pattern)
	for (ISensorObserver* observer : m_observers)
	{
		observer->onMoistureReading(reading);
	}
}

float MoistureSensor::readRaw() const
{
	// Initialize ADC in oneshot mode for each reading.
	// A production implementation may keep the unit handle open; here we
	// favor simplicity to keep the driver changes minimal.
	adc_oneshot_unit_handle_t adcHandle = nullptr;
	adc_oneshot_unit_init_cfg_t initCfg = {};
	initCfg.unit_id                     = ADC_UNIT_1;

	if (adc_oneshot_new_unit(&initCfg, &adcHandle) != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to initialize ADC unit");
		return 0.0f;
	}

	adc_oneshot_chan_cfg_t chanCfg = {};
	chanCfg.atten                  = ADC_ATTEN_DB_12;
	chanCfg.bitwidth               = ADC_BITWIDTH_DEFAULT;

	if (adc_oneshot_config_channel(adcHandle, kAdcChannel, &chanCfg) != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to configure ADC channel %d", kAdcChannel);
		adc_oneshot_del_unit(adcHandle);
		return 0.0f;
	}

	int raw = 0;
	esp_err_t err = adc_oneshot_read(adcHandle, kAdcChannel, &raw);
	adc_oneshot_del_unit(adcHandle);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "ADC read failed: %s", esp_err_to_name(err));
		return 0.0f;
	}

	return adcToMoisture(raw);
}
