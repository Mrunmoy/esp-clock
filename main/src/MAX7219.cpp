#include "MAX7219.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <cstring>

namespace
{
	const char* TAG = "MAX7219";
}

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

MAX7219::MAX7219(int numDevices)
	: m_spi(nullptr)
	, m_numDevices(numDevices)
	, m_displayBuffer(nullptr)
	, m_csPin(-1)
{
	// Allocate display buffer
	m_displayBuffer = new uint8_t*[m_numDevices];
	for (int i = 0; i < m_numDevices; i++)
	{
		m_displayBuffer[i] = new uint8_t[8];
		memset(m_displayBuffer[i], 0, 8);
	}
}

MAX7219::~MAX7219()
{
	if (m_displayBuffer)
	{
		for (int i = 0; i < m_numDevices; i++)
		{
			delete[] m_displayBuffer[i];
		}
		delete[] m_displayBuffer;
	}

	if (m_spi)
	{
		spi_bus_remove_device(m_spi);
	}
}

bool MAX7219::init(int clkPin, int mosiPin, int csPin)
{
	m_csPin = csPin;

	// Configure SPI bus
	spi_bus_config_t busConfig = {};
	busConfig.mosi_io_num = mosiPin;
	busConfig.miso_io_num = -1;  // Not used
	busConfig.sclk_io_num = clkPin;
	busConfig.quadwp_io_num = -1;
	busConfig.quadhd_io_num = -1;
	busConfig.max_transfer_sz = 0;

	esp_err_t ret = spi_bus_initialize(SPI2_HOST, &busConfig, SPI_DMA_CH_AUTO);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
		return false;
	}

	// Configure SPI device
	spi_device_interface_config_t devConfig = {};
	devConfig.clock_speed_hz = 10 * 1000 * 1000;  // 10 MHz
	devConfig.mode = 0;
	devConfig.spics_io_num = csPin;
	devConfig.queue_size = 1;

	ret = spi_bus_add_device(SPI2_HOST, &devConfig, &m_spi);
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

	ESP_LOGI(TAG, "Initialized %d MAX7219 device(s)", m_numDevices);
	return true;
}

void MAX7219::clear()
{
	for (int i = 0; i < m_numDevices; i++)
	{
		memset(m_displayBuffer[i], 0, 8);
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
	if (device < 0 || device >= m_numDevices)
		return;
	if (row < 0 || row >= 8 || col < 0 || col >= 8)
		return;

	if (on)
	{
		m_displayBuffer[device][row] |= (1 << col);
	}
	else
	{
		m_displayBuffer[device][row] &= ~(1 << col);
	}
}

void MAX7219::setRow(int device, int row, uint8_t data)
{
	if (device < 0 || device >= m_numDevices)
		return;
	if (row < 0 || row >= 8)
		return;

	m_displayBuffer[device][row] = data;
}

void MAX7219::displayBuffer()
{
	for (int row = 0; row < 8; row++)
	{
		for (int dev = 0; dev < m_numDevices; dev++)
		{
			writeRegister(dev, REG_DIGIT0 + row, m_displayBuffer[dev][row]);
		}
	}
}

uint8_t* MAX7219::getBuffer(int device)
{
	if (device < 0 || device >= m_numDevices)
		return nullptr;
	return m_displayBuffer[device];
}

void MAX7219::writeRegister(int device, uint8_t reg, uint8_t data)
{
	if (device < 0 || device >= m_numDevices)
		return;

	uint8_t txData[2 * m_numDevices];
	memset(txData, 0, sizeof(txData));

	// Fill with NOOP for all devices except the target
	for (int i = 0; i < m_numDevices; i++)
	{
		if (i == device)
		{
			txData[i * 2] = reg;
			txData[i * 2 + 1] = data;
		}
		else
		{
			txData[i * 2] = REG_NOOP;
			txData[i * 2 + 1] = 0x00;
		}
	}

	spi_transaction_t trans = {};
	trans.length = 16 * m_numDevices;
	trans.tx_buffer = txData;

	spi_device_transmit(m_spi, &trans);
}

void MAX7219::writeAll(uint8_t reg, uint8_t data)
{
	uint8_t txData[2 * m_numDevices];

	for (int i = 0; i < m_numDevices; i++)
	{
		txData[i * 2] = reg;
		txData[i * 2 + 1] = data;
	}

	spi_transaction_t trans = {};
	trans.length = 16 * m_numDevices;
	trans.tx_buffer = txData;

	spi_device_transmit(m_spi, &trans);
}

void MAX7219::scrollText(const char* text, int delayMs)
{
	// This is a placeholder - will be implemented in DisplayManager
	ESP_LOGW(TAG, "scrollText not implemented in MAX7219 - use DisplayManager");
}
