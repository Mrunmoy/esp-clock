#include "DisplayController.hpp"
#include "TimeSync.hpp"
#include "Quotes.hpp"
#include "esp_log.h"
#include "esp_timer.h"
#include <cstring>

static const char* TAG = "DisplayController";

#define WEATHER_UPDATE_INTERVAL_MS (10 * 60 * 1000)  // 10 minutes
#define MODE_SWITCH_INTERVAL_MS (10 * 1000)  // 10 seconds per mode

DisplayController::DisplayController(DisplayManager* display)
	: display_(display)
	, last_weather_update_(0)
	, current_mode_(0)
	, last_mode_switch_(0)
{
	ConfigManager::loadConfig(config_);
	memset(&weather_data_, 0, sizeof(weather_data_));
}

void DisplayController::start()
{
	ESP_LOGI(TAG, "Display controller started");

	// Initial weather fetch
	if (config_.showWeather)
	{
		WeatherFetcher::fetchWeather(weather_data_);
		last_weather_update_ = esp_timer_get_time() / 1000;
	}
}

void DisplayController::updateDisplay()
{
	// Reload config in case it changed via web UI
	ConfigManager::loadConfig(config_);

	uint32_t now = esp_timer_get_time() / 1000;

	// Update weather if needed
	if (config_.showWeather && (now - last_weather_update_ > WEATHER_UPDATE_INTERVAL_MS))
	{
		WeatherFetcher::fetchWeather(weather_data_);
		last_weather_update_ = now;
	}

	// Count enabled modes
	int mode_count = 0;
	if (config_.showClock) mode_count++;
	if (config_.showWeather) mode_count++;
	if (config_.showStarWarsQuotes) mode_count++;
	if (config_.showLOTRQuotes) mode_count++;
	if (strlen(config_.customText) > 0) mode_count++;

	if (mode_count == 0)
	{
		// No modes enabled, show default message
		display_->scrollText("ESP-Clock - Configure via web UI", 50);
		vTaskDelay(pdMS_TO_TICKS(5000));
		return;
	}

	// Switch mode if interval elapsed
	if (now - last_mode_switch_ > MODE_SWITCH_INTERVAL_MS)
	{
		current_mode_ = (current_mode_ + 1) % mode_count;
		last_mode_switch_ = now;
	}

	// Display current mode
	int mode_index = 0;

	if (config_.showClock)
	{
		if (mode_index == current_mode_)
		{
			displayClock();
			return;
		}
		mode_index++;
	}

	if (config_.showWeather)
	{
		if (mode_index == current_mode_)
		{
			displayWeather();
			return;
		}
		mode_index++;
	}

	if (config_.showStarWarsQuotes)
	{
		if (mode_index == current_mode_)
		{
			displayQuote(true);
			return;
		}
		mode_index++;
	}

	if (config_.showLOTRQuotes)
	{
		if (mode_index == current_mode_)
		{
			displayQuote(false);
			return;
		}
		mode_index++;
	}

	if (strlen(config_.customText) > 0)
	{
		if (mode_index == current_mode_)
		{
			displayCustomText();
			return;
		}
		mode_index++;
	}
}

void DisplayController::displayClock()
{
	if (!TimeSync::isTimeSynced())
	{
		display_->displayText("--:--", 8);
		display_->update();
		vTaskDelay(pdMS_TO_TICKS(1000));
		return;
	}

	struct tm timeinfo;
	TimeSync::getCurrentTime(timeinfo);
	display_->displayClock(timeinfo.tm_hour, timeinfo.tm_min, false);
	vTaskDelay(pdMS_TO_TICKS(1000));
}

void DisplayController::displayWeather()
{
	char weather_str[128];
	WeatherFetcher::formatWeatherString(weather_data_, weather_str, sizeof(weather_str));
	display_->scrollText(weather_str, 50);
}

void DisplayController::displayQuote(bool star_wars)
{
	const char* quote = star_wars ? Quotes::getStarWarsQuote() : Quotes::getLOTRQuote();
	display_->scrollText(quote, 50);
}

void DisplayController::displayCustomText()
{
	display_->scrollText(config_.customText, 50);
}
