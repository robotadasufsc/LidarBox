#include "gps.h"

#include "debug.h"

bool setup_gps() {
	// GPS is on the ProMicro's UART (Serial1)
	// RX: pin 0; TX: pin 1

	// The default baud rate is 4800
	Serial1.begin(4800);

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
	Serial1.print(F("$PSRF103,02,00,00,01*26\r\n"));  // GSA off
	delay(20);
	Serial1.print(F("$PSRF103,03,00,00,01*27\r\n"));  // GSV off
	delay(20);

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
		if(c == '\r') continue;
		else if(c == '\n') DEBUGLN();
		else DEBUG(c);
		gps.encode(c);
#else
		gps.encode(Serial1.read());
#endif
	}
}
