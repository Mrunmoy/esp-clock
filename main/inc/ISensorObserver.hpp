#pragma once

/// Represents a single moisture sensor reading.
struct MoistureReading
{
	float moisture;  ///< Soil moisture level in percent (0.0 = dry, 100.0 = saturated)
	bool  valid;     ///< True when the reading was successfully acquired
};

/// Observer interface for sensor data consumers.
/// Implement this interface to receive pushed readings from a sensor source.
class ISensorObserver
{
public:
	virtual ~ISensorObserver() = default;

	/// Called by the sensor thread whenever a new reading is available.
	/// Implementations must be thread-safe.
	virtual void onMoistureReading(const MoistureReading& reading) = 0;
};
