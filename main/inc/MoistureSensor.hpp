/**
 * @file MoistureSensor.hpp
 * @brief Capacitive soil moisture sensor driver using ESP32-S3 ADC
 *
 * Reads the DFRobot SEN0193 Capacitive Soil Moisture Sensor via ADC1.
 * The sensor outputs an analog voltage inversely proportional to moisture:
 * higher ADC readings indicate drier soil.
 */

#pragma once

#include <cstdint>
#include "esp_adc/adc_oneshot.h"

/**
 * @struct MoistureReading
 * @brief A single moisture sensor reading with raw and computed values
 */
struct MoistureReading
{
	uint16_t rawAdc;       ///< Raw 12-bit ADC value (0-4095)
	int millivolts;        ///< Calibrated voltage in mV
	uint8_t percentage;    ///< Moisture percentage (0-100)
};

/**
 * @brief Compute moisture percentage from raw ADC value
 *
 * Pure function for testability. Maps ADC reading to 0-100%
 * using calibration values. Higher ADC = drier = lower percentage.
 *
 * @param rawAdc Raw ADC reading
 * @param airValue ADC reading in dry air (0% moisture)
 * @param waterValue ADC reading submerged in water (100% moisture)
 * @return Moisture percentage clamped to 0-100
 */
uint8_t computeMoisturePercent(uint16_t rawAdc, uint16_t airValue, uint16_t waterValue);

/**
 * @brief Get plant status string based on moisture percentage
 *
 * @param percentage Moisture percentage (0-100)
 * @return Status string describing plant watering needs
 */
const char* getMoistureStatus(uint8_t percentage);

/**
 * @class MoistureSensor
 * @brief ADC-based soil moisture sensor driver
 *
 * Uses ESP-IDF adc_oneshot API on ADC1 channel 3 (GPIO4) with
 * 12-bit resolution and 11dB attenuation for 0-3.1V input range.
 */
class MoistureSensor
{
public:
	/**
	 * @brief Initialize the ADC for moisture sensor reading
	 *
	 * Configures ADC1 with oneshot mode, 12-bit resolution,
	 * and 11dB attenuation on the configured GPIO pin.
	 *
	 * @return true if ADC initialization succeeded
	 */
	static bool init();

	/**
	 * @brief Read the sensor and return a MoistureReading
	 *
	 * Takes multiple samples and averages them to reduce noise.
	 *
	 * @param airValue Calibration value for dry air
	 * @param waterValue Calibration value for water
	 * @return MoistureReading with raw, voltage, and percentage values
	 */
	static MoistureReading read(uint16_t airValue, uint16_t waterValue);

	/**
	 * @brief Read raw ADC value (average of multiple samples)
	 *
	 * @return Averaged raw ADC value
	 */
	static uint16_t readRaw();
};
