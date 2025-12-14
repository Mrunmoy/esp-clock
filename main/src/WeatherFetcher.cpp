#include "WeatherFetcher.hpp"
#include "esp_log.h"
#include "esp_http_client.h"
#include <cJSON.h>
#include <cstring>

namespace
{
	const char* TAG = "WeatherFetcher";
	const int MAX_HTTP_OUTPUT_BUFFER = 2048;
	char httpOutputBuffer[MAX_HTTP_OUTPUT_BUFFER];
	int httpOutputLen = 0;
}

esp_err_t httpEventHandler(esp_http_client_event_t *evt)
{
	switch(evt->event_id)
	{
	case HTTP_EVENT_ON_DATA:
		if (!esp_http_client_is_chunked_response(evt->client))
		{
			if (httpOutputLen + evt->data_len < MAX_HTTP_OUTPUT_BUFFER)
			{
				memcpy(httpOutputBuffer + httpOutputLen, evt->data, evt->data_len);
				httpOutputLen += evt->data_len;
			}
		}
		break;
	default:
		break;
	}
	return ESP_OK;
}

bool WeatherFetcher::fetchWeather(WeatherData& data)
{
	data.valid = false;

	const char* apiKey = CONFIG_OPENWEATHER_API_KEY;
	if (strlen(apiKey) == 0)
	{
		ESP_LOGW(TAG, "OpenWeather API key not configured");
		return false;
	}

	char url[256];
	snprintf(url, sizeof(url),
	         "http://api.openweathermap.org/data/2.5/weather?q=%s,%s&appid=%s&units=metric",
	         CONFIG_WEATHER_CITY, CONFIG_WEATHER_COUNTRY_CODE, apiKey);

	httpOutputLen = 0;
	memset(httpOutputBuffer, 0, sizeof(httpOutputBuffer));

	esp_http_client_config_t config = {};
	config.url = url;
	config.event_handler = httpEventHandler;
	config.timeout_ms = 5000;

	esp_http_client_handle_t client = esp_http_client_init(&config);
	esp_err_t err = esp_http_client_perform(client);

	if (err == ESP_OK)
	{
		int statusCode = esp_http_client_get_status_code(client);
		if (statusCode == 200)
		{
			httpOutputBuffer[httpOutputLen] = '\0';
			bool parsed = parseWeatherJSON(httpOutputBuffer, data);
			esp_http_client_cleanup(client);
			return parsed;
		}
		else
		{
			ESP_LOGW(TAG, "HTTP GET failed, status code: %d", statusCode);
		}
	}
	else
	{
		ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
	}

	esp_http_client_cleanup(client);
	return false;
}

bool WeatherFetcher::parseWeatherJSON(const char* json, WeatherData& data)
{
	cJSON* root = cJSON_Parse(json);
	if (root == NULL)
	{
		ESP_LOGE(TAG, "Failed to parse JSON");
		return false;
	}

	// Parse temperature from main.temp
	cJSON* main = cJSON_GetObjectItem(root, "main");
	if (main)
	{
		cJSON* temp = cJSON_GetObjectItem(main, "temp");
		cJSON* humidity = cJSON_GetObjectItem(main, "humidity");

		if (temp) data.temperature = temp->valuedouble;
		if (humidity) data.humidity = humidity->valueint;
	}

	// Parse description from weather[0].description
	cJSON* weather = cJSON_GetObjectItem(root, "weather");
	if (weather && cJSON_IsArray(weather))
	{
		cJSON* weatherItem = cJSON_GetArrayItem(weather, 0);
		if (weatherItem)
		{
			cJSON* desc = cJSON_GetObjectItem(weatherItem, "description");
			cJSON* icon = cJSON_GetObjectItem(weatherItem, "icon");

			if (desc && cJSON_IsString(desc))
			{
				strncpy(data.description, desc->valuestring, sizeof(data.description) - 1);
			}

			if (icon && cJSON_IsString(icon))
			{
				strncpy(data.icon, icon->valuestring, sizeof(data.icon) - 1);
			}
		}
	}

	data.valid = true;
	cJSON_Delete(root);

	ESP_LOGI(TAG, "Weather: %.1f°C, %d%%, %s", data.temperature, data.humidity, data.description);
	return true;
}

void WeatherFetcher::formatWeatherString(const WeatherData& data, char* buffer, size_t size)
{
	if (!data.valid)
	{
		snprintf(buffer, size, "Weather: N/A");
		return;
	}

	snprintf(buffer, size, "%.1fC %d%% %s", data.temperature, data.humidity, data.description);
}
