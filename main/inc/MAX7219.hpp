#pragma once

#include "driver/spi_master.h"
#include <cstdint>

class MAX7219
{
public:
	MAX7219(int num_devices = 5);
	~MAX7219();

	bool init(int clk_pin, int mosi_pin, int cs_pin);
	void clear();
	void setBrightness(uint8_t intensity);
	void setPixel(int device, int row, int col, bool on);
	void setRow(int device, int row, uint8_t data);
	void displayBuffer();
	void scrollText(const char* text, int delay_ms = 100);

	// Get raw buffer for direct manipulation
	uint8_t* getBuffer(int device);

private:
	void writeRegister(int device, uint8_t reg, uint8_t data);
	void writeAll(uint8_t reg, uint8_t data);

	spi_device_handle_t spi_;
	int num_devices_;
	uint8_t** display_buffer_;
	int cs_pin_;
};
