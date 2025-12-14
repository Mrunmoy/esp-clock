#include "MAX7219.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <cstring>

static const char* TAG = "MAX7219";

// MAX7219 Register addresses
#define REG_NOOP        0x00
#define REG_DIGIT0      0x01
#define REG_DIGIT1      0x02
#define REG_DIGIT2      0x03
#define REG_DIGIT3      0x04
#define REG_DIGIT4      0x05
#define REG_DIGIT5      0x06
#define REG_DIGIT6      0x07
#define REG_DIGIT7      0x08
#define REG_DECODE_MODE 0x09
#define REG_INTENSITY   0x0A
#define REG_SCAN_LIMIT  0x0B
#define REG_SHUTDOWN    0x0C
#define REG_DISPLAY_TEST 0x0F

MAX7219::MAX7219(int num_devices)
	: spi_(nullptr)
	, num_devices_(num_devices)
	, display_buffer_(nullptr)
	, cs_pin_(-1)
{
	// Allocate display buffer
	display_buffer_ = new uint8_t*[num_devices_];
	for (int i = 0; i < num_devices_; i++)
	{
		display_buffer_[i] = new uint8_t[8];
		memset(display_buffer_[i], 0, 8);
	}
}

MAX7219::~MAX7219()
{
	if (display_buffer_)
	{
		for (int i = 0; i < num_devices_; i++)
		{
			delete[] display_buffer_[i];
		}
		delete[] display_buffer_;
	}

	if (spi_)
	{
		spi_bus_remove_device(spi_);
	}
}

bool MAX7219::init(int clk_pin, int mosi_pin, int cs_pin)
{
	cs_pin_ = cs_pin;

	// Configure SPI bus
	spi_bus_config_t bus_config = {};
	bus_config.mosi_io_num = mosi_pin;
	bus_config.miso_io_num = -1;  // Not used
	bus_config.sclk_io_num = clk_pin;
	bus_config.quadwp_io_num = -1;
	bus_config.quadhd_io_num = -1;
	bus_config.max_transfer_sz = 0;

	esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
		return false;
	}

	// Configure SPI device
	spi_device_interface_config_t dev_config = {};
	dev_config.clock_speed_hz = 10 * 1000 * 1000;  // 10 MHz
	dev_config.mode = 0;
	dev_config.spics_io_num = cs_pin;
	dev_config.queue_size = 1;

	ret = spi_bus_add_device(SPI2_HOST, &dev_config, &spi_);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
		return false;
	}

	// Initialize MAX7219 devices
	writeAll(REG_SHUTDOWN, 0x00);      // Shutdown mode
	writeAll(REG_DECODE_MODE, 0x00);   // No decode (raw mode)
	writeAll(REG_SCAN_LIMIT, 0x07);    // Scan all 8 digits
	writeAll(REG_INTENSITY, 0x08);     // Medium intensity
	writeAll(REG_DISPLAY_TEST, 0x00);  // Normal operation
	writeAll(REG_SHUTDOWN, 0x01);      // Normal operation

	clear();

	ESP_LOGI(TAG, "Initialized %d MAX7219 device(s)", num_devices_);
	return true;
}

void MAX7219::clear()
{
	for (int i = 0; i < num_devices_; i++)
	{
		memset(display_buffer_[i], 0, 8);
	}
	displayBuffer();
}

void MAX7219::setBrightness(uint8_t intensity)
{
	if (intensity > 15)
		intensity = 15;
	writeAll(REG_INTENSITY, intensity);
}

void MAX7219::setPixel(int device, int row, int col, bool on)
{
	if (device < 0 || device >= num_devices_)
		return;
	if (row < 0 || row >= 8 || col < 0 || col >= 8)
		return;

	if (on)
	{
		display_buffer_[device][row] |= (1 << col);
	}
	else
	{
		display_buffer_[device][row] &= ~(1 << col);
	}
}

void MAX7219::setRow(int device, int row, uint8_t data)
{
	if (device < 0 || device >= num_devices_)
		return;
	if (row < 0 || row >= 8)
		return;

	display_buffer_[device][row] = data;
}

void MAX7219::displayBuffer()
{
	for (int row = 0; row < 8; row++)
	{
		for (int dev = 0; dev < num_devices_; dev++)
		{
			writeRegister(dev, REG_DIGIT0 + row, display_buffer_[dev][row]);
		}
	}
}

uint8_t* MAX7219::getBuffer(int device)
{
	if (device < 0 || device >= num_devices_)
		return nullptr;
	return display_buffer_[device];
}

void MAX7219::writeRegister(int device, uint8_t reg, uint8_t data)
{
	if (device < 0 || device >= num_devices_)
		return;

	uint8_t tx_data[2 * num_devices_];
	memset(tx_data, 0, sizeof(tx_data));

	// Fill with NOOP for all devices except the target
	for (int i = 0; i < num_devices_; i++)
	{
		if (i == device)
		{
			tx_data[i * 2] = reg;
			tx_data[i * 2 + 1] = data;
		}
		else
		{
			tx_data[i * 2] = REG_NOOP;
			tx_data[i * 2 + 1] = 0x00;
		}
	}

	spi_transaction_t trans = {};
	trans.length = 16 * num_devices_;
	trans.tx_buffer = tx_data;

	spi_device_transmit(spi_, &trans);
}

void MAX7219::writeAll(uint8_t reg, uint8_t data)
{
	uint8_t tx_data[2 * num_devices_];

	for (int i = 0; i < num_devices_; i++)
	{
		tx_data[i * 2] = reg;
		tx_data[i * 2 + 1] = data;
	}

	spi_transaction_t trans = {};
	trans.length = 16 * num_devices_;
	trans.tx_buffer = tx_data;

	spi_device_transmit(spi_, &trans);
}

void MAX7219::scrollText(const char* text, int delay_ms)
{
	// This is a placeholder - will be implemented in DisplayManager
	ESP_LOGW(TAG, "scrollText not implemented in MAX7219 - use DisplayManager");
}
