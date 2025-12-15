#pragma once

#include "MAX7219.hpp"
#include <string>

class DisplayManager
{
public:
	DisplayManager(MAX7219* display);

	void clear();
	void displayText(const char* text, int startX = 0);
	void scrollText(const char* text, int scrollSpeedMs = 50);
	void displayClock(int hour, int minute, bool showSeconds = false);
	void update();

private:
	void drawChar(char c, int xOffset);
	const uint8_t* getCharBitmap(char c);

	MAX7219* m_display = nullptr;
	int m_scrollOffset = 0;
	std::string m_scrollText = "";
};
