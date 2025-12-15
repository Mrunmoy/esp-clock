#include "WebServer.hpp"
#include "WifiManager.hpp"
#include "ConfigManager.hpp"
#include "esp_log.h"
#include "esp_system.h"
#include <string>
#include <cJSON.h>

namespace
{
	const char* TAG = "WebServer";
	httpd_handle_t server = nullptr;

	// Declare symbols created by `EMBED_FILES`
	extern const uint8_t index_html_start[] asm("_binary_index_html_start");
	extern const uint8_t index_html_end[]   asm("_binary_index_html_end");

	// Handler to serve the embedded index.html at "/"
	esp_err_t rootHandler(httpd_req_t* req)
	{
		size_t htmlLen = index_html_end - index_html_start;
		httpd_resp_set_type(req, "text/html");
		return httpd_resp_send(req, reinterpret_cast<const char*>(index_html_start), htmlLen);
	}

	// Handler to get current configuration
	esp_err_t configGetHandler(httpd_req_t* req)
	{
		DisplayConfig config;
		ConfigManager::loadConfig(config);

		cJSON* root = cJSON_CreateObject();
		cJSON_AddBoolToObject(root, "showClock", config.showClock);
		cJSON_AddBoolToObject(root, "showWeather", config.showWeather);
		cJSON_AddBoolToObject(root, "showStarWars", config.showStarWarsQuotes);
		cJSON_AddBoolToObject(root, "showLOTR", config.showLOTRQuotes);
		cJSON_AddBoolToObject(root, "displayFlipped", config.displayFlipped);
		cJSON_AddNumberToObject(root, "brightness", config.brightness);
		cJSON_AddStringToObject(root, "customText", config.customText);
		cJSON_AddStringToObject(root, "weatherApiKey", config.weatherApiKey);

		char* jsonStr = cJSON_Print(root);
		httpd_resp_set_type(req, "application/json");
		httpd_resp_send(req, jsonStr, strlen(jsonStr));

		free(jsonStr);
		cJSON_Delete(root);
		return ESP_OK;
	}

	// Handler to save configuration
	esp_err_t configPostHandler(httpd_req_t* req)
	{
		char buf[512];
		int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
		if (ret <= 0)
		{
			if (ret == HTTPD_SOCK_ERR_TIMEOUT)
			{
				httpd_resp_send_408(req);
			}
			return ESP_FAIL;
		}
		buf[ret] = '\0';

		cJSON* root = cJSON_Parse(buf);
		if (root == NULL)
		{
			httpd_resp_send_500(req);
			return ESP_FAIL;
		}

		DisplayConfig config;
		ConfigManager::loadConfig(config);

		cJSON* item = cJSON_GetObjectItem(root, "showClock");
		if (item) config.showClock = cJSON_IsTrue(item);

		item = cJSON_GetObjectItem(root, "showWeather");
		if (item) config.showWeather = cJSON_IsTrue(item);

		item = cJSON_GetObjectItem(root, "showStarWars");
		if (item) config.showStarWarsQuotes = cJSON_IsTrue(item);

		item = cJSON_GetObjectItem(root, "showLOTR");
		if (item) config.showLOTRQuotes = cJSON_IsTrue(item);

		item = cJSON_GetObjectItem(root, "displayFlipped");
		if (item) config.displayFlipped = cJSON_IsTrue(item);

		item = cJSON_GetObjectItem(root, "brightness");
		if (item && cJSON_IsNumber(item))
		{
			int brightness = item->valueint;
			if (brightness < 0) brightness = 0;
			if (brightness > 15) brightness = 15;
			config.brightness = brightness;
		}

		item = cJSON_GetObjectItem(root, "customText");
		if (item && cJSON_IsString(item))
		{
			strncpy(config.customText, item->valuestring, sizeof(config.customText) - 1);
		}

		item = cJSON_GetObjectItem(root, "weatherApiKey");
		if (item && cJSON_IsString(item))
		{
			strncpy(config.weatherApiKey, item->valuestring, sizeof(config.weatherApiKey) - 1);
			config.weatherApiKey[sizeof(config.weatherApiKey) - 1] = '\0';
		}

		ConfigManager::saveConfig(config);
		cJSON_Delete(root);

		httpd_resp_send(req, "OK", 2);
		return ESP_OK;
	}

	// Handler to save WiFi configuration
	esp_err_t wifiConfigHandler(httpd_req_t* req)
	{
		char buf[256];
		int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
		if (ret <= 0)
		{
			if (ret == HTTPD_SOCK_ERR_TIMEOUT)
			{
				httpd_resp_send_408(req);
			}
			return ESP_FAIL;
		}
		buf[ret] = '\0';

		cJSON* root = cJSON_Parse(buf);
		if (root == NULL)
		{
			httpd_resp_send_500(req);
			return ESP_FAIL;
		}

		cJSON* ssidItem = cJSON_GetObjectItem(root, "ssid");
		cJSON* passItem = cJSON_GetObjectItem(root, "password");

		if (ssidItem && cJSON_IsString(ssidItem) && passItem && cJSON_IsString(passItem))
		{
			WifiManager::saveWiFiConfig(ssidItem->valuestring, passItem->valuestring);
			httpd_resp_send(req, "OK - Rebooting...", 18);
			cJSON_Delete(root);

			// Reboot after 2 seconds
			vTaskDelay(2000 / portTICK_PERIOD_MS);
			esp_restart();
			return ESP_OK;
		}

		cJSON_Delete(root);
		httpd_resp_send_500(req);
		return ESP_FAIL;
	}
}

void WebServer::start()
{
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.uri_match_fn = httpd_uri_match_wildcard;

	if (httpd_start(&server, &config) == ESP_OK)
	{
		httpd_uri_t rootUri = {
			.uri      = "/",
			.method   = HTTP_GET,
			.handler  = rootHandler,
			.user_ctx = nullptr,
		};

		httpd_uri_t configGetUri = {
			.uri      = "/api/config",
			.method   = HTTP_GET,
			.handler  = configGetHandler,
			.user_ctx = nullptr,
		};

		httpd_uri_t configPostUri = {
			.uri      = "/api/config",
			.method   = HTTP_POST,
			.handler  = configPostHandler,
			.user_ctx = nullptr,
		};

		httpd_uri_t wifiConfigUri = {
			.uri      = "/api/wifi",
			.method   = HTTP_POST,
			.handler  = wifiConfigHandler,
			.user_ctx = nullptr,
		};

		httpd_register_uri_handler(server, &rootUri);
		httpd_register_uri_handler(server, &configGetUri);
		httpd_register_uri_handler(server, &configPostUri);
		httpd_register_uri_handler(server, &wifiConfigUri);

		ESP_LOGI(TAG, "Web server started");
	}
	else
	{
		ESP_LOGE(TAG, "Failed to start web server");
	}
}

void WebServer::stop()
{
	if (server)
	{
		httpd_stop(server);
		server = nullptr;
	}
}
