#pragma once

#include <cstdint>

class Font5x7
{
public:
	static const uint8_t* getChar(char c);

private:
	static const uint8_t font_data[][5];
};
