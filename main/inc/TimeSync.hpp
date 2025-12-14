/**
 * @file TimeSync.hpp
 * @brief Network time synchronization via SNTP
 *
 * Manages time synchronization with NTP servers and provides
 * access to the current time. Uses SNTP (Simple Network Time Protocol)
 * to keep the device's clock accurate.
 */

#pragma once

#include <ctime>

/**
 * @class TimeSync
 * @brief SNTP time synchronization manager
 *
 * Handles initialization and synchronization with NTP servers.
 * Provides methods to access the current time in various formats.
 */
class TimeSync
{
public:
	/**
	 * @brief Initialize SNTP time sync
	 *
	 * Sets up SNTP with configured server (CONFIG_NTP_SERVER)
	 * and timezone (CONFIG_TIMEZONE).
	 */
	static void init();

	/**
	 * @brief Trigger time synchronization
	 *
	 * Blocks until time is synchronized with NTP server
	 * or maximum retry count is reached.
	 */
	static void syncTime();

	/**
	 * @brief Check if time has been synchronized
	 *
	 * @return true if time sync is complete, false otherwise
	 */
	static bool isTimeSynced();

	/**
	 * @brief Get current local time
	 *
	 * @param timeinfo Output parameter for current time
	 */
	static void getCurrentTime(struct tm& timeinfo);

	/**
	 * @brief Format current time as string
	 *
	 * @param buffer Output buffer for formatted string
	 * @param size Buffer size
	 * @param format strftime format string (default: "HH:MM:SS")
	 */
	static void getTimeString(char* buffer, size_t size, const char* format = "%H:%M:%S");
};
