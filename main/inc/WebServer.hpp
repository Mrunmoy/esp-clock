#pragma once

#include "esp_http_server.h"

class WebServer
{
public:
	static void start();
	static void stop();
};
