#pragma once

#include "DisplayManager.hpp"
#include "ConfigManager.hpp"
#include "WeatherFetcher.hpp"

class DisplayController
{
public:
	DisplayController(DisplayManager* display);

	void start();
	void updateDisplay();

private:
	void displayClock();
	void displayWeather();
	void displayQuote(bool star_wars);
	void displayCustomText();

	DisplayManager* display_;
	DisplayConfig config_;
	WeatherData weather_data_;
	uint32_t last_weather_update_;
	int current_mode_;
	uint32_t last_mode_switch_;
};
