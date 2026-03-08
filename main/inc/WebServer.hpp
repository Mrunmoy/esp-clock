#pragma once

#include "esp_http_server.h"
#include "MoistureSensor.hpp"

class WebServer
{
public:
	static void start();
	static void stop();
	static void updateMoistureReading(const MoistureReading& reading);
};
