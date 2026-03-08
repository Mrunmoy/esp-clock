#pragma once

#include "esp_http_server.h"
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

private:
	/// Internal observer implementation — receives pushed readings and stores
	/// them so that the /api/moisture handler can serve them without polling.
	class MoistureObserver : public ISensorObserver
	{
	public:
		void onMoistureReading(const MoistureReading& reading) override;
	};

	static MoistureObserver s_moistureObserver;
};
