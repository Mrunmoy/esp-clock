#include "TimeSync.hpp"
#include "esp_log.h"
#include "esp_sntp.h"
#include <sys/time.h>
#include <cstring>

static const char* TAG = "TimeSync";
static bool time_synced = false;

void time_sync_notification_cb(struct timeval *tv)
{
	ESP_LOGI(TAG, "Time synchronized");
	time_synced = true;
}

void TimeSync::init()
{
	ESP_LOGI(TAG, "Initializing SNTP");

	// Set timezone from Kconfig
	setenv("TZ", CONFIG_TIMEZONE, 1);
	tzset();

	esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
	esp_sntp_setservername(0, CONFIG_NTP_SERVER);
	esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
	esp_sntp_init();
}

void TimeSync::syncTime()
{
	ESP_LOGI(TAG, "Waiting for time synchronization...");

	int retry = 0;
	const int retry_count = 10;

	while (esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
	{
		ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}

	if (retry >= retry_count)
	{
		ESP_LOGW(TAG, "Time sync timeout");
		time_synced = false;
	}
	else
	{
		time_synced = true;
		ESP_LOGI(TAG, "Time synchronized successfully");
	}
}

bool TimeSync::isTimeSynced()
{
	return time_synced;
}

void TimeSync::getCurrentTime(struct tm& timeinfo)
{
	time_t now;
	time(&now);
	localtime_r(&now, &timeinfo);
}

void TimeSync::getTimeString(char* buffer, size_t size, const char* format)
{
	struct tm timeinfo;
	getCurrentTime(timeinfo);
	strftime(buffer, size, format, &timeinfo);
}
