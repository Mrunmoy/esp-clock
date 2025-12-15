/**
 * @file WeatherFetcher.hpp
 * @brief OpenWeatherMap API integration for weather data
 *
 * Fetches current weather information from OpenWeatherMap API
 * and parses the JSON response into a structured format.
 */

#pragma once

#include <string>

/**
 * @struct WeatherData
 * @brief Structured weather information
 *
 * Contains parsed weather data from OpenWeatherMap API response.
 */
struct WeatherData
{
	float temperature;      ///< Temperature in Celsius
	int humidity;           ///< Relative humidity percentage (0-100)
	char description[64];   ///< Weather description (e.g., "partly cloudy")
	char icon[8];           ///< Weather icon code (e.g., "01d")
	bool valid;             ///< True if data was successfully fetched
};

/**
 * @class WeatherFetcher
 * @brief Manages weather API requests and response parsing
 *
 * Uses OpenWeatherMap Current Weather API (v2.5) to fetch real-time
 * weather data. Requires internet connectivity and valid API key.
 */
class WeatherFetcher
{
public:
	/**
	 * @brief Fetch current weather from OpenWeatherMap
	 *
	 * Makes HTTP GET request to OpenWeatherMap API using API key from
	 * configuration and city/country from sdkconfig. Parses JSON response
	 * and populates WeatherData structure.
	 *
	 * @param data Output parameter for weather information
	 * @param apiKey OpenWeather API key (if empty, uses CONFIG_OPENWEATHER_API_KEY)
	 * @return true if fetch and parse successful, false on error
	 *
	 * @note Requires:
	 *  - WiFi connection
	 *  - Valid API key
	 *  - CONFIG_WEATHER_CITY set
	 *  - CONFIG_WEATHER_COUNTRY_CODE set
	 */
	static bool fetchWeather(WeatherData& data, const char* apiKey = nullptr);

	/**
	 * @brief Format weather data as display string
	 *
	 * Creates human-readable string from weather data in format:
	 * "TEMP°C HUMIDITY% DESCRIPTION"
	 *
	 * @param data Weather data to format
	 * @param buffer Output buffer for formatted string
	 * @param size Buffer size
	 */
	static void formatWeatherString(const WeatherData& data, char* buffer, size_t size);

private:
	/**
	 * @brief Parse OpenWeatherMap JSON response
	 *
	 * Extracts temperature, humidity, and description from JSON.
	 *
	 * @param json Raw JSON response string
	 * @param data Output parameter for parsed data
	 * @return true if parsing successful, false on error
	 */
	static bool parseWeatherJSON(const char* json, WeatherData& data);
};
