/*
On the Arduino Micro, ProMicro, Leonardo, Yùn Rev 2, SDA and SCL are the pins
D2 and D3.
*/

#ifdef LIDAR_BENEWAKE_TF02

#include "common.h"

#include <Arduino.h>
#include <Wire.h>

#include "../debug.h"

// The I2C address of the lidar is preset to 0x10
#define I2C_ADDR 0x10

void read_response(size_t size) {
    Wire.requestFrom(I2C_ADDR, size);

    DEBUG(F("Response: "));
    while(Wire.available()) {
#ifdef DEBUG_TO_SERIAL
        DEBUG(' ');
        DEBUG(Wire.read());
#else
        Wire.read();
#endif
    }
    DEBUGLN();
}

bool setup_lidar() {
	// startup I2C bus for the LiDAR
	Wire.begin();
    Wire.setTimeout(250);

    // Get firmware version major:u8 minor:u8 micro:u48
    Wire.beginTransmission(I2C_ADDR);
    Wire.write("\x5A\x04\x01\x5F");
    Wire.endTransmission();
    delay(100);

    {
        size_t size = Wire.requestFrom(I2C_ADDR, 7);
        if(size == 0) {
            DEBUG(F("The LiDAR is probably in serial mode"));
            return false;
        }

        Wire.read(); Wire.read(); Wire.read();

        uint8_t micro_version = Wire.read();
        uint8_t minor_version = Wire.read();
        uint8_t major_version = Wire.read();
        
        Wire.read();

        DEBUG("LiDAR FW: ");
        DEBUG(major_version);
        DEBUG('.');
        DEBUG(minor_version);
        DEBUG('.');
        DEBUGLN(micro_version);
    }

    // Set framerate to 1000
    Wire.beginTransmission(I2C_ADDR);
    Wire.write("\x5A\x06\x03\xE8\x03\x4E");
    Wire.endTransmission();
    delay(100);
    read_response(6);

    // Set output format
    Wire.beginTransmission(I2C_ADDR);
    Wire.write("\x5A\x05\x05\x01\x65");
    Wire.endTransmission();
    delay(100);
    read_response(5);

    // Save configuration
    Wire.beginTransmission(I2C_ADDR);
    Wire.write("\x5A\x04\x11\x6F");
    Wire.endTransmission();
    delay(100);
    read_response(5); // MUST BE hex { 5A 05 11 01 71 }
	
	return true;  // We assume it has successfully set communication
}

/**
 * 0x5959: u16 - Start of each message
 * distance: u16 - The distance in centimetres
 * strength: u16 - The reflection strength
 * temp: u16 - The temperature in °C
 * checksum: u8 - The message checksum
 */
int16_t get_lidar_distance_cm() {
    uint8_t msg[10];

    Wire.beginTransmission(I2C_ADDR);
    Wire.write("\x5A\x05\x00\x01\x60", 5);
    Wire.endTransmission();

	Wire.requestFrom(I2C_ADDR, 9);

    size_t size = Wire.available();
    size_t limit = size > 9 ? 9 : size;
    for(size_t i = 0; i < limit; i++) msg[i] = Wire.read();
    while(Wire.available()) Wire.read();
    msg[limit] = 0;
    
    if(size == 9 && msg[0] == 0x59 && msg[1] == 0x59) {
        uint16_t distance = (msg[3] << 8) | msg[2];

        // calculate checksum
        uint16_t sum = 0;
        for(int i = 0; i < 8; i++) sum += msg[i];

        // return reading if the sum checks out
        if(msg[8] == (sum & 0xFF)) return distance;
    }

    DEBUG(F("Bad LiDAR message: '"));
    DEBUGHEX(msg, limit);
    DEBUGLN(F("'"));

    return -1;
}

#endif
