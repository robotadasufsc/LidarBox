#ifdef GPS_ADHTECH_GT_735T

#include "./common.h"

#include "../debug.h"

TinyGPSPlus gps;

bool setup_gps() {
	// GPS is on the ProMicro's UART (Serial1)
	// RX: pin 0; TX: pin 1

	// The default baud rate is 9600
	Serial1.begin(9600);

	{  // Wait for GPS
		size_t first_verification = millis();
#ifdef DEBUG_TO_SERIAL
		size_t last_verification = first_verification;
#endif

		while(!Serial1) {
			size_t current_time = millis();

			if(current_time - first_verification >= 60000)
				return false;

#ifdef DEBUG_TO_SERIAL
			if((current_time - last_verification) >= 5000) {
				DEBUGLN(F("Could not lock GPS, wait..."));
				last_verification = current_time;
			}
#endif
		}
	}

	// TinyGPS++ mainly works on GGA and RMC, so we turn off the other NMEA sentences

	// Deactivate the messages we do not want

	// "\xB5\x62\x06\x01\x08\x00\xF0\x01\x01\x00\x01\x01\x01\x01\x05\x3A"
	Serial1.write("\xB5\x62\x06\x01\x08\x00\xF0\x01\x01\x00\x01\x01\x01\x01\x05\x3A", 16);  // GLL

	delay(250);
	while(Serial1.available()) Serial1.read();

	Serial1.write("\xB5\x62\x06\x01\x08\x00\xF0\x02\x01\x00\x01\x01\x01\x01\x06\x41", 16);  // GSA

	delay(250);
	while(Serial1.available()) Serial1.read();

	Serial1.write("\xB5\x62\x06\x01\x08\x00\xF0\x03\x01\x00\x01\x01\x01\x01\x07\x48", 16);  // GSV

	delay(250);
	while(Serial1.available()) Serial1.read();

	Serial1.write("\xB5\x62\x06\x01\x08\x00\xF0\x05\x01\x00\x01\x01\x01\x01\x09\x56", 16);  // VTG

	delay(250);
	while(Serial1.available()) Serial1.read();

	Serial1.write("\xB5\x62\x06\x01\x08\x00\xF0\x08\x00\x00\x00\x00\x00\x00\x07\x5B", 16);  // ZDA

	delay(250);
	while(Serial1.available()) Serial1.read();

	// Keep the messages we want
	Serial1.write("\xB5\x62\x06\x01\x08\x00\xF0\x00\x01\x01\x01\x01\x01\x01\x05\x38", 16);  // GGA

	delay(250);
	while(Serial1.available()) Serial1.read();

	Serial1.write("\xB5\x62\x06\x01\x08\x00\xF0\x04\x01\x01\x01\x01\x01\x01\x09\x54", 16);  // RMC

	delay(250);
	while(Serial1.available()) Serial1.read();

	delay(250);

	// Save the configuration
	Serial1.write("\xB5\x62\x06\x09\x0C\x00\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\x00\x00\x19\x80", 20);

	// FIXME wait for ACKs, do not discard
	delay(250);
	while(Serial1.available()) Serial1.read();

    return true;
}

void wakeful_delay(unsigned long ms) {
	unsigned long limit = millis() + ms;

	do {
		consume_gps();
	} while(millis() < limit);
}

void consume_gps() {
	while(Serial1.available() > 0) {
#ifdef DEBUG_NMEA
		// Echo GPS to USB-serial port for debugging
		char c = Serial1.read();
		if(c == '\r') {
			continue;
		} else if(c == '\n') {
			DEBUGLN();
		} else {
			if(DEBUG_STREAM) DEBUG_STREAM.write(c);
		}
		gps.encode(c);
#else
		gps.encode(Serial1.read());
#endif
	}
}

#endif
