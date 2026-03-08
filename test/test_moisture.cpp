/**
 * @file test_moisture.cpp
 * @brief Unit tests for moisture sensor computation logic
 *
 * Tests the pure functions computeMoisturePercent() and getMoistureStatus()
 * which contain the core business logic and can be tested without hardware.
 *
 * Build and run with:
 *   g++ -std=c++17 -o test_moisture test/test_moisture.cpp -DUNIT_TEST
 *   ./test_moisture
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

// ---- Inline the pure functions under test (no ESP-IDF dependencies) ----

uint8_t computeMoisturePercent(uint16_t rawAdc, uint16_t airValue, uint16_t waterValue)
{
	if (airValue == waterValue)
	{
		return 0;
	}

	if (rawAdc >= airValue)
	{
		return 0;
	}
	if (rawAdc <= waterValue)
	{
		return 100;
	}

	int percent = 100 - ((int)(rawAdc - waterValue) * 100 / (int)(airValue - waterValue));

	if (percent < 0) percent = 0;
	if (percent > 100) percent = 100;

	return static_cast<uint8_t>(percent);
}

const char* getMoistureStatus(uint8_t percentage)
{
	if (percentage <= 20)
	{
		return "Very Dry - Water immediately!";
	}
	if (percentage <= 40)
	{
		return "Dry - Needs watering";
	}
	if (percentage <= 60)
	{
		return "Moist - Good";
	}
	if (percentage <= 80)
	{
		return "Wet - Well watered";
	}
	return "Very Wet - Reduce watering";
}

// ---- Test framework (minimal) ----

static int testsPassed = 0;
static int testsFailed = 0;

#define ASSERT_EQ(expected, actual, msg) \
	do { \
		if ((expected) == (actual)) { \
			testsPassed++; \
		} else { \
			testsFailed++; \
			printf("FAIL: %s - expected %d, got %d\n", msg, (int)(expected), (int)(actual)); \
		} \
	} while(0)

#define ASSERT_STREQ(expected, actual, msg) \
	do { \
		if (strcmp((expected), (actual)) == 0) { \
			testsPassed++; \
		} else { \
			testsFailed++; \
			printf("FAIL: %s - expected \"%s\", got \"%s\"\n", msg, (expected), (actual)); \
		} \
	} while(0)

// ---- Tests for computeMoisturePercent ----

void test_dry_air_returns_zero()
{
	// Raw ADC at or above air value should be 0% moisture
	ASSERT_EQ(0, computeMoisturePercent(3000, 3000, 1500), "raw == airValue");
	ASSERT_EQ(0, computeMoisturePercent(3500, 3000, 1500), "raw > airValue");
}

void test_submerged_returns_hundred()
{
	// Raw ADC at or below water value should be 100% moisture
	ASSERT_EQ(100, computeMoisturePercent(1500, 3000, 1500), "raw == waterValue");
	ASSERT_EQ(100, computeMoisturePercent(1000, 3000, 1500), "raw < waterValue");
}

void test_midpoint_returns_fifty()
{
	// Midpoint between air(3000) and water(1500) is 2250
	uint8_t result = computeMoisturePercent(2250, 3000, 1500);
	ASSERT_EQ(50, result, "midpoint raw value");
}

void test_quarter_points()
{
	// 25% moisture: raw = 3000 - (3000-1500)*0.25 = 2625
	uint8_t q25 = computeMoisturePercent(2625, 3000, 1500);
	ASSERT_EQ(25, q25, "25% moisture");

	// 75% moisture: raw = 3000 - (3000-1500)*0.75 = 1875
	uint8_t q75 = computeMoisturePercent(1875, 3000, 1500);
	ASSERT_EQ(75, q75, "75% moisture");
}

void test_equal_calibration_returns_zero()
{
	// Edge case: air == water should not divide by zero
	ASSERT_EQ(0, computeMoisturePercent(2000, 2000, 2000), "equal calibration values");
}

void test_custom_calibration_range()
{
	// Different calibration: air=2800, water=1200, range=1600
	// midpoint=2000 -> 50%
	uint8_t result = computeMoisturePercent(2000, 2800, 1200);
	ASSERT_EQ(50, result, "custom calibration midpoint");
}

void test_boundary_values()
{
	// Test with extreme ADC values
	ASSERT_EQ(0, computeMoisturePercent(4095, 4095, 0), "max ADC = air");
	ASSERT_EQ(100, computeMoisturePercent(0, 4095, 0), "zero ADC = water");
	ASSERT_EQ(50, computeMoisturePercent(2048, 4095, 0), "mid full range");
}

// ---- Tests for getMoistureStatus ----

void test_status_very_dry()
{
	ASSERT_STREQ("Very Dry - Water immediately!", getMoistureStatus(0), "0% status");
	ASSERT_STREQ("Very Dry - Water immediately!", getMoistureStatus(20), "20% status");
}

void test_status_dry()
{
	ASSERT_STREQ("Dry - Needs watering", getMoistureStatus(21), "21% status");
	ASSERT_STREQ("Dry - Needs watering", getMoistureStatus(40), "40% status");
}

void test_status_moist()
{
	ASSERT_STREQ("Moist - Good", getMoistureStatus(41), "41% status");
	ASSERT_STREQ("Moist - Good", getMoistureStatus(60), "60% status");
}

void test_status_wet()
{
	ASSERT_STREQ("Wet - Well watered", getMoistureStatus(61), "61% status");
	ASSERT_STREQ("Wet - Well watered", getMoistureStatus(80), "80% status");
}

void test_status_very_wet()
{
	ASSERT_STREQ("Very Wet - Reduce watering", getMoistureStatus(81), "81% status");
	ASSERT_STREQ("Very Wet - Reduce watering", getMoistureStatus(100), "100% status");
}

// ---- Main ----

int main()
{
	printf("=== Soil Moisture Sensor Unit Tests ===\n\n");

	// computeMoisturePercent tests
	printf("-- computeMoisturePercent --\n");
	test_dry_air_returns_zero();
	test_submerged_returns_hundred();
	test_midpoint_returns_fifty();
	test_quarter_points();
	test_equal_calibration_returns_zero();
	test_custom_calibration_range();
	test_boundary_values();

	// getMoistureStatus tests
	printf("-- getMoistureStatus --\n");
	test_status_very_dry();
	test_status_dry();
	test_status_moist();
	test_status_wet();
	test_status_very_wet();

	printf("\n=== Results: %d passed, %d failed ===\n", testsPassed, testsFailed);

	return testsFailed > 0 ? 1 : 0;
}
