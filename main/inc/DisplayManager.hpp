#pragma once

#include "MAX7219.hpp"
#include <string>

class DisplayManager
{
public:
	DisplayManager(MAX7219* display);

	void clear();
	void displayText(const char* text, int start_x = 0);
	void scrollText(const char* text, int scroll_speed_ms = 50);
	void displayClock(int hour, int minute, bool show_seconds = false);
	void update();

private:
	void drawChar(char c, int x_offset);
	const uint8_t* getCharBitmap(char c);

	MAX7219* display_;
	int scroll_offset_;
	std::string scroll_text_;
};
