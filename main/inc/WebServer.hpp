#pragma once

#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "ISensorObserver.hpp"

/// HTTP server that serves the configuration UI and REST API.
///
/// The server acts as an observer for sensor data: call getMoistureObserver()
/// to obtain an ISensorObserver* that can be registered with a MoistureSensor.
/// Pushed readings are stored internally and exposed via GET /api/moisture.
class WebServer
{
public:
	static void start();
	static void stop();

	/// Returns the static observer that feeds pushed moisture data into the
	/// server.  Register this with MoistureSensor::addObserver() before
	/// calling start().
	static ISensorObserver* getMoistureObserver();

	/// Returns the most recently pushed moisture reading (thread-safe).
	static MoistureReading getLastMoistureReading();

private:
	/// Internal observer implementation — receives pushed readings and stores
	/// them so that the /api/moisture handler can serve them without polling.
	/// All shared state lives here as instance members; no file-level globals.
	class MoistureObserver : public ISensorObserver
	{
	public:
		/// Create the mutex.  Must be called once the FreeRTOS scheduler is
		/// running (i.e., from WebServer::start()).
		void init();

		void onMoistureReading(const MoistureReading& reading) override;

		/// Thread-safe snapshot of the most recently pushed reading.
		MoistureReading getLastReading();

	private:
		SemaphoreHandle_t m_mutex{nullptr};
		MoistureReading   m_lastReading{0.0f, false};
	};

	static MoistureObserver s_moistureObserver;
};
