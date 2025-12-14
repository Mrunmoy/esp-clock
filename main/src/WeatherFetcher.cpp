#include "WeatherFetcher.hpp"
#include "esp_log.h"
#include "esp_http_client.h"
#include <cJSON.h>
#include <cstring>

static const char* TAG = "WeatherFetcher";

#define MAX_HTTP_OUTPUT_BUFFER 2048

static char http_output_buffer[MAX_HTTP_OUTPUT_BUFFER];
static int http_output_len = 0;

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
	switch(evt->event_id)
	{
	case HTTP_EVENT_ON_DATA:
		if (!esp_http_client_is_chunked_response(evt->client))
		{
			if (http_output_len + evt->data_len < MAX_HTTP_OUTPUT_BUFFER)
			{
				memcpy(http_output_buffer + http_output_len, evt->data, evt->data_len);
				http_output_len += evt->data_len;
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

	const char* api_key = CONFIG_OPENWEATHER_API_KEY;
	if (strlen(api_key) == 0)
	{
		ESP_LOGW(TAG, "OpenWeather API key not configured");
		return false;
	}

	char url[256];
	snprintf(url, sizeof(url),
	         "http://api.openweathermap.org/data/2.5/weather?q=%s,%s&appid=%s&units=metric",
	         CONFIG_WEATHER_CITY, CONFIG_WEATHER_COUNTRY_CODE, api_key);

	http_output_len = 0;
	memset(http_output_buffer, 0, sizeof(http_output_buffer));

	esp_http_client_config_t config = {};
	config.url = url;
	config.event_handler = http_event_handler;
	config.timeout_ms = 5000;

	esp_http_client_handle_t client = esp_http_client_init(&config);
	esp_err_t err = esp_http_client_perform(client);

	if (err == ESP_OK)
	{
		int status_code = esp_http_client_get_status_code(client);
		if (status_code == 200)
		{
			http_output_buffer[http_output_len] = '\0';
			bool parsed = parseWeatherJSON(http_output_buffer, data);
			esp_http_client_cleanup(client);
			return parsed;
		}
		else
		{
			ESP_LOGW(TAG, "HTTP GET failed, status code: %d", status_code);
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
		cJSON* weather_item = cJSON_GetArrayItem(weather, 0);
		if (weather_item)
		{
			cJSON* desc = cJSON_GetObjectItem(weather_item, "description");
			cJSON* icon = cJSON_GetObjectItem(weather_item, "icon");

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
