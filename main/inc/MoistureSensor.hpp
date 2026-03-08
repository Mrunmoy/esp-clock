#pragma once

#include "ISensorObserver.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include <vector>

/// Periodically reads a capacitive/resistive soil-moisture sensor connected to
/// an ADC channel and pushes readings to all registered observers.
///
/// Usage:
///   MoistureSensor sensor;
///   sensor.addObserver(&myObserver);
///   sensor.start();   // spawns a FreeRTOS task
class MoistureSensor
{
public:
	/// @param intervalMs  How often (milliseconds) to sample the sensor.
	explicit MoistureSensor(uint32_t intervalMs = 5000);
	~MoistureSensor();

	/// Register an observer that will be notified after every sample.
	/// Must be called before start().
	void addObserver(ISensorObserver* observer);

	/// Spawn the sensor task.  Safe to call only once.
	void start();

	/// Stop the sensor task and release resources.
	void stop();

	/// Return the most recently acquired reading (may be invalid if never sampled).
	MoistureReading getLastReading() const;

private:
	static void taskFunction(void* arg);
	void        readAndPublish();
	float       readRaw() const;

	std::vector<ISensorObserver*> m_observers;
	mutable SemaphoreHandle_t     m_mutex{nullptr};
	MoistureReading               m_lastReading{0.0f, false};
	uint32_t                      m_intervalMs;
	TaskHandle_t                  m_taskHandle{nullptr};
};
