#include "imu.h"

#include <stddef.h>
#include <Arduino.h>

bool setup_imu() {
    if(!imu.init()) return false;

    imu.enableDefault();

    return true;
}

void get_imu_readings(IMUData &results) {
	// The Pololu MinIMU-9 v5 has a LSM6DS33 gyro and accel sensor. The values returned by the library are the raw
	// 16-bit values the sensor outputs. They can be converted to units of g (acceleration of gravity) and °/s using
	// the conversion factors specified in the  datasheet for your particular device and full scale setting (gain).
	//
	// Example: An LSM6DS33 gives an accelerometer Z axis reading of 16276 with its default full scale setting of ±2 g.
	// The LA_So (linear acceleration sensitivity) specification in the LSM6DS33 datasheet (page 15) states a
	// conversion factor of 0.061 mg at this full scale setting, so the raw reading of 16276 corresponds to
	// 16276 * 0.061 mg = 992.84 mg = 0.99284 g.

	// TODO investigate the use of linear or polynimal regression

	// for 400 samples it takes ~852ms; 100 samples take 212ms.
	constexpr size_t imu_samples = 100;

	long sum_accel_x = 0, sum_accel_y = 0, sum_accel_z = 0;
	long sum_gyro_x = 0, sum_gyro_y = 0, sum_gyro_z = 0;

	digitalWrite(LED_BUILTIN_RX, LOW);
	// alteração na horientação dos sensores, minusculo para aceleração, maiusculo para giroscópio
	// x = az ; y = -ax ; z = -ay
	// X = gZ ; Y = -gX; Z = -gY
	for(size_t i = 0; i < imu_samples; i++) {
		imu.read();
		/*
		sum_accel_x += imu.a.x;
		sum_accel_y += imu.a.y;
		sum_accel_z += imu.a.z;
		sum_gyro_x += imu.g.x;
		sum_gyro_y += imu.g.y;
		sum_gyro_z += imu.g.z;
		*/
		sum_accel_x += imu.a.z;
		sum_accel_y += -imu.a.x;
		sum_accel_z += -imu.a.y;

		sum_gyro_x += imu.g.z;
		sum_gyro_y += -imu.g.x;
		sum_gyro_z += -imu.g.y;
		}
	digitalWrite(LED_BUILTIN_RX, HIGH);

	// Reading is at 0.000061 * g (acceleration of gravity). See https://www.pololu.com/file/0J1087/LSM6DS33.pdf, page 15
	// For m/s² value they used for g would need to be known
	results.accel_x = ((float) sum_accel_x) / imu_samples * 0.000061;
	results.accel_y = ((float) sum_accel_y) / imu_samples * 0.000061;
	results.accel_z = ((float) sum_accel_z) / imu_samples * 0.000061;

	float horiz_mag = sqrt(sq(results.accel_x) + sq(results.accel_y));
	results.tilt_deg = abs(atan(horiz_mag / results.accel_z) * 180.0 / M_PI);

	// at gain level of ±245°/s, the reading is at 0.00875º/s
	// see same datasheet as gravity above page 15
	results.gyro_x = sum_gyro_x / imu_samples * 0.00875;
	results.gyro_y = sum_gyro_y / imu_samples * 0.00875;
	results.gyro_z = sum_gyro_z / imu_samples * 0.00875;
}
