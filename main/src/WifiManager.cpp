#include "WifiManager.hpp"

#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_system.h"
#include "esp_netif.h"

static const char *TAG = "WifiManager";

#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static const int MAX_RETRY = 5;

extern "C" void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                   int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED)
	{
		auto* event = static_cast<wifi_event_ap_staconnected_t*>(event_data);
		ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED)
	{
		auto* event = static_cast<wifi_event_ap_stadisconnected_t*>(event_data);
		ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d, reason=%d", MAC2STR(event->mac), event->aid, event->reason);
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		esp_wifi_connect();
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		if (s_retry_num < MAX_RETRY)
		{
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP");
		}
		else
		{
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG, "connect to the AP fail");
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
		auto* event = static_cast<ip_event_got_ip_t*>(event_data);
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

void WifiManager::init()
{
	// Init NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	s_wifi_event_group = xEventGroupCreate();
}

void WifiManager::startConfigAP()
{
	esp_netif_create_default_wifi_ap();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
	                                                     ESP_EVENT_ANY_ID,
	                                                     &wifi_event_handler,
	                                                     nullptr,
	                                                     nullptr));

	wifi_config_t wifi_config{};
	strncpy(reinterpret_cast<char*>(wifi_config.ap.ssid), EXAMPLE_ESP_WIFI_SSID, sizeof(wifi_config.ap.ssid));
	wifi_config.ap.ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID);
	wifi_config.ap.channel = EXAMPLE_ESP_WIFI_CHANNEL;
	strncpy(reinterpret_cast<char*>(wifi_config.ap.password), EXAMPLE_ESP_WIFI_PASS, sizeof(wifi_config.ap.password));
	wifi_config.ap.max_connection = EXAMPLE_MAX_STA_CONN;

#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
	wifi_config.ap.authmode = WIFI_AUTH_WPA3_PSK;
	wifi_config.ap.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
#else
	wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
#endif

	wifi_config.ap.pmf_cfg.required = true;

	if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
	{
		wifi_config.ap.authmode = WIFI_AUTH_OPEN;
	}

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "SoftAP started. SSID:%s password:%s channel:%d",
	         EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

void WifiManager::connectToConfiguredWiFi()
{
	char ssid[32] = {0};
	char password[64] = {0};

	getConfiguredWiFi(ssid, password, sizeof(ssid));

	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
	                                                     ESP_EVENT_ANY_ID,
	                                                     &wifi_event_handler,
	                                                     nullptr,
	                                                     nullptr));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
	                                                     IP_EVENT_STA_GOT_IP,
	                                                     &wifi_event_handler,
	                                                     nullptr,
	                                                     nullptr));

	wifi_config_t wifi_config = {};
	strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
	strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "Connecting to SSID:%s...", ssid);
}

bool WifiManager::saveWiFiConfig(const char* ssid, const char* password)
{
	nvs_handle_t nvs_handle;
	esp_err_t err;

	err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
		return false;
	}

	err = nvs_set_str(nvs_handle, "wifi_ssid", ssid);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Error saving SSID: %s", esp_err_to_name(err));
		nvs_close(nvs_handle);
		return false;
	}

	err = nvs_set_str(nvs_handle, "wifi_pass", password);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Error saving password: %s", esp_err_to_name(err));
		nvs_close(nvs_handle);
		return false;
	}

	err = nvs_commit(nvs_handle);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
		nvs_close(nvs_handle);
		return false;
	}

	nvs_close(nvs_handle);
	ESP_LOGI(TAG, "WiFi config saved successfully");
	return true;
}

bool WifiManager::hasConfiguredWiFi()
{
	nvs_handle_t nvs_handle;
	esp_err_t err;

	err = nvs_open("storage", NVS_READONLY, &nvs_handle);
	if (err != ESP_OK)
	{
		return false;
	}

	size_t required_size = 0;
	err = nvs_get_str(nvs_handle, "wifi_ssid", nullptr, &required_size);
	nvs_close(nvs_handle);

	return (err == ESP_OK && required_size > 0);
}

void WifiManager::getConfiguredWiFi(char* ssid, char* password, size_t max_len)
{
	nvs_handle_t nvs_handle;
	esp_err_t err;

	err = nvs_open("storage", NVS_READONLY, &nvs_handle);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
		return;
	}

	size_t ssid_len = max_len;
	err = nvs_get_str(nvs_handle, "wifi_ssid", ssid, &ssid_len);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Error reading SSID: %s", esp_err_to_name(err));
	}

	size_t pass_len = max_len * 2;
	err = nvs_get_str(nvs_handle, "wifi_pass", password, &pass_len);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Error reading password: %s", esp_err_to_name(err));
	}

	nvs_close(nvs_handle);
}

bool WifiManager::isConnected()
{
	if (s_wifi_event_group == nullptr)
		return false;

	EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);
	return (bits & WIFI_CONNECTED_BIT) != 0;
}

void WifiManager::waitForConnection()
{
	if (s_wifi_event_group == nullptr)
		return;

	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
	                                        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
	                                        pdFALSE,
	                                        pdFALSE,
	                                        portMAX_DELAY);

	if (bits & WIFI_CONNECTED_BIT)
	{
		ESP_LOGI(TAG, "connected to WiFi");
	}
	else if (bits & WIFI_FAIL_BIT)
	{
		ESP_LOGI(TAG, "Failed to connect to WiFi");
	}
	else
	{
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
	}
}
