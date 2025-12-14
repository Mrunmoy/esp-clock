#pragma once

#include <cstdint>

struct DisplayConfig
{
	bool showClock;
	bool showWeather;
	bool showStarWarsQuotes;
	bool showLOTRQuotes;
	char customText[256];
};

class ConfigManager
{
public:
	static void init();
	static bool saveConfig(const DisplayConfig& config);
	static bool loadConfig(DisplayConfig& config);
	static void getDefaultConfig(DisplayConfig& config);
};
