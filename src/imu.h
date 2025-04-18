#pragma once

#include <LSM6.h>

struct IMUData {
	float accel_x, accel_y, accel_z;
	float gyro_x, gyro_y, gyro_z;
	float tilt_deg;
};

// IMU sensor
static LSM6 imu;

bool setup_imu();

/**
 * Reads the IMU's data. This function reads 100 samples and returns the arithmetic mean.
 */
void get_imu_readings(IMUData &results);
