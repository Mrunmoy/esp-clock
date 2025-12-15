#include "DisplayManager.hpp"
#include "Font5x7.hpp"
#include <cstring>

DisplayManager::DisplayManager(MAX7219* display)
	: m_display(display)
	, m_scrollOffset(0)
{
}

void DisplayManager::clear()
{
	m_display->clear();
}

void DisplayManager::drawChar(char c, int xOffset)
{
	const uint8_t* bitmap = getCharBitmap(c);
	if (!bitmap)
		return;

	// 5x7 font, each character is 5 pixels wide
	for (int col = 0; col < 5; col++)
	{
		int globalX = xOffset + col;
		if (globalX < 0 || globalX >= 40)  // 5 devices * 8 pixels
			continue;

		int device = globalX / 8;
		int deviceCol = globalX % 8;

		// Mirror character horizontally when flipped
		int bitmapCol = m_flipped ? (4 - col) : col;
		uint8_t colData = bitmap[bitmapCol];

		// Set each bit in the column
		for (int row = 0; row < 8; row++)
		{
			bool pixelOn = (colData & (1 << row)) != 0;

			if (m_flipped)
			{
				// Flip Y (vertical) for upside-down display
				int flippedRow = 7 - row;
				m_display->setPixel(device, flippedRow, 7 - deviceCol, pixelOn);
			}
			else
			{
				m_display->setPixel(device, row, 7 - deviceCol, pixelOn);
			}
		}
	}
}

void DisplayManager::displayText(const char* text, int startX)
{
	clear();

	size_t textLen = strlen(text);

	if (m_flipped)
	{
		// Draw characters in reverse order when flipped
		int x = startX;
		for (int i = textLen - 1; i >= 0; i--)
		{
			drawChar(text[i], x);
			x += 6;  // 5 pixels + 1 pixel spacing
		}
	}
	else
	{
		// Draw characters in normal order
		int x = startX;
		for (size_t i = 0; i < textLen; i++)
		{
			drawChar(text[i], x);
			x += 6;  // 5 pixels + 1 pixel spacing
		}
	}

	m_display->displayBuffer();
}

void DisplayManager::scrollText(const char* text, int scrollSpeedMs)
{
	if (!text || strlen(text) == 0)
		return;

	size_t textLen = strlen(text);
	int textWidth = textLen * 6;  // 6 pixels per character (5 + 1 spacing)
	int displayWidth = 40;  // 5 devices * 8 pixels

	// Scroll from right to left (start offscreen right, move left)
	for (int offset = -textWidth; offset < displayWidth; offset++)
	{
		clear();

		if (m_flipped)
		{
			// Draw characters in reverse order when flipped
			int x = offset;
			for (int i = textLen - 1; i >= 0; i--)
			{
				if (x + 5 >= 0 && x < displayWidth)
				{
					drawChar(text[i], x);
				}
				x += 6;
			}
		}
		else
		{
			// Draw characters in normal order
			int x = offset;
			for (size_t i = 0; i < textLen; i++)
			{
				if (x + 5 >= 0 && x < displayWidth)
				{
					drawChar(text[i], x);
				}
				x += 6;
			}
		}

		m_display->displayBuffer();
		vTaskDelay(pdMS_TO_TICKS(scrollSpeedMs));
	}
}

void DisplayManager::displayClock(int hour, int minute, bool showSeconds)
{
	char timeStr[16];
	snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour, minute);
	displayText(timeStr, 4);  // Center on 5 displays
}

void DisplayManager::update()
{
	m_display->displayBuffer();
}

const uint8_t* DisplayManager::getCharBitmap(char c)
{
	return Font5x7::getChar(c);
}

void DisplayManager::setFlipped(bool flipped)
{
	m_flipped = flipped;
}

void DisplayManager::setBrightness(uint8_t intensity)
{
	// MAX7219 supports brightness values 0-15
	if (intensity > 15)
		intensity = 15;
	m_display->setBrightness(intensity);
}
