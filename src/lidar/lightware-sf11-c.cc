/*
On the Arduino Micro, ProMicro, Leonardo, Yùn Rev 2, SDA and SCL are the pins
D2 and D3.
*/

#ifdef LIDAR_LIGHTWARE_SF11

#include "common.h"

#include <Wire.h>

//#define I2C_SDA 2
//#define I2C_SCL 3

// The I2C address of the lidar is preset to 0x55
#define I2C_ADR 0x55

bool setup_lidar() {
	// startup I2C bus for the LiDAR
	Wire.begin();

	// Instruct the lidar to activate the distance location
	Wire.beginTransmission(I2C_ADR);
	Wire.write(0);
	Wire.endTransmission();
	
	return true;  // We assume it has successfully set communication
}

int16_t get_lidar_distance_cm() {
	Wire.requestFrom(I2C_ADR, 2);

	if(Wire.available() < 2) return -1;

	uint16_t high = Wire.read();
	uint16_t low = Wire.read();

	return (high << 8) | low;  // combine in big endian order
}

#endif
