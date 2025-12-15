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
	void displayQuote(bool starWars);
	void displayCustomText();

	DisplayManager* m_display = nullptr;
	DisplayConfig m_config = {};
	WeatherData m_weatherData = {};
	uint32_t m_lastWeatherUpdate = 0;
	int m_currentMode = 0;
	uint32_t m_lastModeSwitch = 0;
};
