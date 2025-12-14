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

	DisplayManager* m_display;
	DisplayConfig m_config;
	WeatherData m_weatherData;
	uint32_t m_lastWeatherUpdate;
	int m_currentMode;
	uint32_t m_lastModeSwitch;
};
