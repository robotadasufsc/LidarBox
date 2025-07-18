#ifdef GPS_ADHTECH_GT_735T_NMEA

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

    // Alternative approach using NMEA commands (similar to EM506)
    // Some GPS modules respond better to NMEA than UBX

    // Clear any existing data
    while (Serial1.available())
        Serial1.read();
    delay(100);

    // Try NMEA commands for GP-735T (similar to SiRF protocol)
    Serial1.print(F("$PSRF103,01,00,00,01*24\r\n")); // GLL off
    delay(100);
    Serial1.print(F("$PSRF103,02,00,00,01*26\r\n")); // GSA off
    delay(100);
    Serial1.print(F("$PSRF103,03,00,00,01*27\r\n")); // GSV off
    delay(100);
    Serial1.print(F("$PSRF103,05,00,00,01*21\r\n")); // VTG off
    delay(100);
    Serial1.print(F("$PSRF103,08,00,00,01*2C\r\n")); // ZDA off
    delay(100);

    // Keep GGA and RMC
    Serial1.print(F("$PSRF103,00,01,00,01*25\r\n")); // GGA on
    delay(100);
    Serial1.print(F("$PSRF103,04,01,00,01*21\r\n")); // RMC on
    delay(100);

#ifdef DEBUG_TO_SERIAL
    DEBUGLN(F("GPS GP-735T configured using NMEA commands"));
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
