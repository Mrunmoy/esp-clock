#pragma once

#include <string>

class WifiManager
{
public:
	static void init();
	static void startConfigAP();
	static void connectToConfiguredWiFi();
	static bool saveWiFiConfig(const char* ssid, const char* password);
	static bool hasConfiguredWiFi();
	static void getConfiguredWiFi(char* ssid, char* password, size_t max_len);
	static bool isConnected();
	static void waitForConnection();
};
