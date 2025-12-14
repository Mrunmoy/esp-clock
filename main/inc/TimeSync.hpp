#pragma once

#include <ctime>

class TimeSync
{
public:
	static void init();
	static void syncTime();
	static bool isTimeSynced();
	static void getCurrentTime(struct tm& timeinfo);
	static void getTimeString(char* buffer, size_t size, const char* format = "%H:%M:%S");
};
