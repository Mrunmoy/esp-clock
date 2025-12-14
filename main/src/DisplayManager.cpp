#include "DisplayManager.hpp"
#include "Font5x7.hpp"
#include <cstring>

DisplayManager::DisplayManager(MAX7219* display)
	: display_(display)
	, scroll_offset_(0)
{
}

void DisplayManager::clear()
{
	display_->clear();
}

void DisplayManager::drawChar(char c, int x_offset)
{
	const uint8_t* bitmap = getCharBitmap(c);
	if (!bitmap)
		return;

	// 5x7 font, each character is 5 pixels wide
	for (int col = 0; col < 5; col++)
	{
		int global_x = x_offset + col;
		if (global_x < 0 || global_x >= 40)  // 5 devices * 8 pixels
			continue;

		int device = global_x / 8;
		int device_col = global_x % 8;

		uint8_t col_data = bitmap[col];

		// Set each bit in the column
		for (int row = 0; row < 8; row++)
		{
			bool pixel_on = (col_data & (1 << row)) != 0;
			display_->setPixel(device, row, 7 - device_col, pixel_on);
		}
	}
}

void DisplayManager::displayText(const char* text, int start_x)
{
	clear();

	int x = start_x;
	for (size_t i = 0; i < strlen(text); i++)
	{
		drawChar(text[i], x);
		x += 6;  // 5 pixels + 1 pixel spacing
	}

	display_->displayBuffer();
}

void DisplayManager::scrollText(const char* text, int scroll_speed_ms)
{
	if (!text || strlen(text) == 0)
		return;

	int text_width = strlen(text) * 6;  // 6 pixels per character (5 + 1 spacing)
	int display_width = 40;  // 5 devices * 8 pixels

	for (int offset = display_width; offset > -text_width; offset--)
	{
		clear();

		int x = offset;
		for (size_t i = 0; i < strlen(text); i++)
		{
			if (x + 5 >= 0 && x < display_width)
			{
				drawChar(text[i], x);
			}
			x += 6;
		}

		display_->displayBuffer();
		vTaskDelay(pdMS_TO_TICKS(scroll_speed_ms));
	}
}

void DisplayManager::displayClock(int hour, int minute, bool show_seconds)
{
	char time_str[16];
	snprintf(time_str, sizeof(time_str), "%02d:%02d", hour, minute);
	displayText(time_str, 4);  // Center on 5 displays
}

void DisplayManager::update()
{
	display_->displayBuffer();
}

const uint8_t* DisplayManager::getCharBitmap(char c)
{
	return Font5x7::getChar(c);
}
