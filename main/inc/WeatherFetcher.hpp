#pragma once

#include <string>

struct WeatherData
{
	float temperature;
	int humidity;
	char description[64];
	char icon[8];
	bool valid;
};

class WeatherFetcher
{
public:
	static bool fetchWeather(WeatherData& data);
	static void formatWeatherString(const WeatherData& data, char* buffer, size_t size);

private:
	static bool parseWeatherJSON(const char* json, WeatherData& data);
};
