#ifdef GPS_ADHTECH_GT_735T

#include "./common.h"

#include "../debug.h"

bool setup_gps()
{
	// GPS is on the ProMicro's UART (Serial1)
	// RX: pin 0; TX: pin 1

	// The default baud rate is 9600
	Serial1.begin(9600);

	{ // Wait for GPS
		size_t first_verification = millis();
#ifdef DEBUG_TO_SERIAL
		size_t last_verification = first_verification;
#endif

		while (!Serial1)
		{
			size_t current_time = millis();

			if (current_time - first_verification >= 60000)
				return false;

#ifdef DEBUG_TO_SERIAL
			if ((current_time - last_verification) >= 5000)
			{
				DEBUGLN(F("Could not lock GPS, wait..."));
				last_verification = current_time;
			}
#endif
		}
	}

	// Clear any existing data
	while (Serial1.available())
		Serial1.read();
	delay(100);

	// TinyGPS++ mainly works on GGA and RMC, so we turn off the other NMEA sentences
	// Using corrected UBX commands with proper timing

	// Deactivate the messages we do not want (with increased delays)
	Serial1.write("\xB5\x62\x06\x01\x08\x00\xF0\x01\x00\x00\x00\x00\x00\x00\x00\x35", 16); // GLL OFF
	delay(250);
	while (Serial1.available())
		Serial1.read(); // Clear response

	Serial1.write("\xB5\x62\x06\x01\x08\x00\xF0\x02\x00\x00\x00\x00\x00\x00\x01\x3C", 16); // GSA OFF
	delay(250);
	while (Serial1.available())
		Serial1.read(); // Clear response

	Serial1.write("\xB5\x62\x06\x01\x08\x00\xF0\x03\x00\x00\x00\x00\x00\x00\x02\x43", 16); // GSV OFF
	delay(250);
	while (Serial1.available())
		Serial1.read(); // Clear response

	Serial1.write("\xB5\x62\x06\x01\x08\x00\xF0\x05\x00\x00\x00\x00\x00\x00\x04\x51", 16); // VTG OFF
	delay(250);
	while (Serial1.available())
		Serial1.read(); // Clear response

	Serial1.write("\xB5\x62\x06\x01\x08\x00\xF0\x08\x00\x00\x00\x00\x00\x00\x07\x60", 16); // ZDA OFF
	delay(250);
	while (Serial1.available())
		Serial1.read(); // Clear response

	// Keep the messages we want - GGA and RMC ON
	Serial1.write("\xB5\x62\x06\x01\x08\x00\xF0\x00\x01\x00\x00\x00\x00\x00\x00\x36", 16); // GGA ON
	delay(250);
	while (Serial1.available())
		Serial1.read(); // Clear response

	Serial1.write("\xB5\x62\x06\x01\x08\x00\xF0\x04\x01\x00\x00\x00\x00\x00\x04\x4A", 16); // RMC ON
	delay(250);
	while (Serial1.available())
		Serial1.read(); // Clear response

	// Save the configuration to flash/EEPROM
	Serial1.write("\xB5\x62\x06\x09\x0D\x00\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\x00\x00\x03\x1D\x96", 21);
	delay(500);
	while (Serial1.available())
		Serial1.read(); // Clear response

#ifdef DEBUG_TO_SERIAL
	DEBUGLN(F("GPS GP-735T configured"));
#endif

	return true;
}

void wakeful_delay(unsigned long ms)
{
	unsigned long limit = millis() + ms;

	do
	{
		consume_gps();
	} while (millis() < limit);
}

void consume_gps()
{
	while (Serial1.available() > 0)
	{
#ifdef DEBUG_NMEA
		// Echo GPS to USB-serial port for debugging
		char c = Serial1.read();
		if (c == '\r')
		{
			continue;
		}
		else if (c == '\n')
		{
			DEBUGLN();
		}
		else
		{
			DEBUG(c);
		}
		gps.encode(c);
#else
		gps.encode(Serial1.read());
#endif
	}
}

#endif
