#pragma once

#include <TinyGPS++.h>

/*
GPS data is always coming in, we can't just delay() or go to sleep between outputs, we constantly have to read and
digest the NMEA sentences coming in. The TinyGPS++ examples have a smartDelay() function to aid with this.
*/

// The TinyGPS++ object
static TinyGPSPlus gps;

/**
 * Sets up the GPS module and wait for a fix.
 * 
 * @return Whether the GPS was successfully set up.
 */
bool setup_gps();

/**
 * Sleeps while still accepting the GPS data.
 * 
 * @param ms The sleep duration, in milliseconds.
 */
void wakeful_delay(unsigned long ms);

/**
 * Consume the GPS data present in the UART buffer.
 */
void consume_gps();
