#include "WebServer.hpp"
#include "WifiManager.hpp"
#include "ConfigManager.hpp"
#include "MoistureSensor.hpp"
#include "esp_log.h"
#include "esp_system.h"
#include <string>
#include <cJSON.h>

namespace
{
	const char* TAG = "WebServer";
	httpd_handle_t server = nullptr;

	// Latest moisture reading, updated by main loop
	MoistureReading latestReading = {};

	extern const uint8_t index_html_start[] asm("_binary_index_html_start");
	extern const uint8_t index_html_end[]   asm("_binary_index_html_end");

	esp_err_t rootHandler(httpd_req_t* req)
	{
		size_t htmlLen = index_html_end - index_html_start;
		httpd_resp_set_type(req, "text/html");
		return httpd_resp_send(req, reinterpret_cast<const char*>(index_html_start), htmlLen);
	}

	esp_err_t moistureGetHandler(httpd_req_t* req)
	{
		cJSON* root = cJSON_CreateObject();
		cJSON_AddNumberToObject(root, "rawAdc", latestReading.rawAdc);
		cJSON_AddNumberToObject(root, "millivolts", latestReading.millivolts);
		cJSON_AddNumberToObject(root, "percentage", latestReading.percentage);
		cJSON_AddStringToObject(root, "status", getMoistureStatus(latestReading.percentage));

		char* jsonStr = cJSON_Print(root);
		httpd_resp_set_type(req, "application/json");
		httpd_resp_send(req, jsonStr, strlen(jsonStr));

		free(jsonStr);
		cJSON_Delete(root);
		return ESP_OK;
	}

	esp_err_t configGetHandler(httpd_req_t* req)
	{
		MoistureConfig config;
		ConfigManager::loadConfig(config);

		cJSON* root = cJSON_CreateObject();
		cJSON_AddNumberToObject(root, "airValue", config.airValue);
		cJSON_AddNumberToObject(root, "waterValue", config.waterValue);
		cJSON_AddNumberToObject(root, "readIntervalMs", config.readIntervalMs);

		char* jsonStr = cJSON_Print(root);
		httpd_resp_set_type(req, "application/json");
		httpd_resp_send(req, jsonStr, strlen(jsonStr));

		free(jsonStr);
		cJSON_Delete(root);
		return ESP_OK;
	}

	esp_err_t configPostHandler(httpd_req_t* req)
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

		MoistureConfig config;
		ConfigManager::loadConfig(config);

		cJSON* item = cJSON_GetObjectItem(root, "airValue");
		if (item && cJSON_IsNumber(item))
		{
			config.airValue = static_cast<uint16_t>(item->valueint);
		}

		item = cJSON_GetObjectItem(root, "waterValue");
		if (item && cJSON_IsNumber(item))
		{
			config.waterValue = static_cast<uint16_t>(item->valueint);
		}

		item = cJSON_GetObjectItem(root, "readIntervalMs");
		if (item && cJSON_IsNumber(item))
		{
			config.readIntervalMs = static_cast<uint32_t>(item->valueint);
		}

		ConfigManager::saveConfig(config);
		cJSON_Delete(root);

		httpd_resp_send(req, "OK", 2);
		return ESP_OK;
	}

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

			vTaskDelay(2000 / portTICK_PERIOD_MS);
			esp_restart();
			return ESP_OK;
		}

		cJSON_Delete(root);
		httpd_resp_send_500(req);
		return ESP_FAIL;
	}
}

void WebServer::updateMoistureReading(const MoistureReading& reading)
{
	latestReading = reading;
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

		httpd_uri_t moistureUri = {
			.uri      = "/api/moisture",
			.method   = HTTP_GET,
			.handler  = moistureGetHandler,
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
		httpd_register_uri_handler(server, &moistureUri);
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
