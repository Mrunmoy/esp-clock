#include "DisplayController.hpp"
#include "TimeSync.hpp"
#include "Quotes.hpp"
#include "esp_log.h"
#include "esp_timer.h"
#include <cstring>

namespace
{
	const char* TAG = "DisplayController";
}

#define WEATHER_UPDATE_INTERVAL_MS (60 * 60 * 1000)  // 1 hour
#define MODE_SWITCH_INTERVAL_MS (10 * 1000)  // 10 seconds per mode

DisplayController::DisplayController(DisplayManager* display)
	: m_display(display)
	, m_lastWeatherUpdate(0)
	, m_currentMode(0)
	, m_lastModeSwitch(0)
{
	ConfigManager::loadConfig(m_config);
	memset(&m_weatherData, 0, sizeof(m_weatherData));
}

void DisplayController::start()
{
	ESP_LOGI(TAG, "Display controller started");

	// Initial weather fetch
	if (m_config.showWeather)
	{
		WeatherFetcher::fetchWeather(m_weatherData);
		m_lastWeatherUpdate = esp_timer_get_time() / 1000;
	}
}

void DisplayController::updateDisplay()
{
	// Reload config in case it changed via web UI
	ConfigManager::loadConfig(m_config);

	uint32_t now = esp_timer_get_time() / 1000;

	// Update weather if needed
	if (m_config.showWeather && (now - m_lastWeatherUpdate > WEATHER_UPDATE_INTERVAL_MS))
	{
		WeatherFetcher::fetchWeather(m_weatherData);
		m_lastWeatherUpdate = now;
	}

	// Count enabled modes
	int modeCount = 0;
	if (m_config.showClock) modeCount++;
	if (m_config.showWeather) modeCount++;
	if (m_config.showStarWarsQuotes) modeCount++;
	if (m_config.showLOTRQuotes) modeCount++;
	if (strlen(m_config.customText) > 0) modeCount++;

	if (modeCount == 0)
	{
		// No modes enabled, show default message
		m_display->scrollText("ESP-Clock - Configure via web UI", 50);
		vTaskDelay(pdMS_TO_TICKS(5000));
		return;
	}

	// Switch mode if interval elapsed
	if (now - m_lastModeSwitch > MODE_SWITCH_INTERVAL_MS)
	{
		m_currentMode = (m_currentMode + 1) % modeCount;
		m_lastModeSwitch = now;
	}

	// Display current mode
	int modeIndex = 0;

	if (m_config.showClock)
	{
		if (modeIndex == m_currentMode)
		{
			displayClock();
			return;
		}
		modeIndex++;
	}

	if (m_config.showWeather)
	{
		if (modeIndex == m_currentMode)
		{
			displayWeather();
			return;
		}
		modeIndex++;
	}

	if (m_config.showStarWarsQuotes)
	{
		if (modeIndex == m_currentMode)
		{
			displayQuote(true);
			return;
		}
		modeIndex++;
	}

	if (m_config.showLOTRQuotes)
	{
		if (modeIndex == m_currentMode)
		{
			displayQuote(false);
			return;
		}
		modeIndex++;
	}

	if (strlen(m_config.customText) > 0)
	{
		if (modeIndex == m_currentMode)
		{
			displayCustomText();
			return;
		}
		modeIndex++;
	}
}

void DisplayController::displayClock()
{
	if (!TimeSync::isTimeSynced())
	{
		m_display->displayText("--:--", 8);
		m_display->update();
		vTaskDelay(pdMS_TO_TICKS(1000));
		return;
	}

	struct tm timeinfo;
	TimeSync::getCurrentTime(timeinfo);
	m_display->displayClock(timeinfo.tm_hour, timeinfo.tm_min, false);
	vTaskDelay(pdMS_TO_TICKS(1000));
}

void DisplayController::displayWeather()
{
	char weatherStr[128];
	WeatherFetcher::formatWeatherString(m_weatherData, weatherStr, sizeof(weatherStr));
	m_display->scrollText(weatherStr, 50);
}

void DisplayController::displayQuote(bool starWars)
{
	const char* quote = starWars ? Quotes::getStarWarsQuote() : Quotes::getLOTRQuote();
	m_display->scrollText(quote, 50);
}

void DisplayController::displayCustomText()
{
	m_display->scrollText(m_config.customText, 50);
}
