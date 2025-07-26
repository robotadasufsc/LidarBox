// Program to talk to a GPS module (GlobalSat EM-506 GPS or lightware SF-11 laser rangefinder), an IMU
// (Pololu MinIMU-9 v5), and log to SparkFun SDcard board.

// Adapted from https://doi.org/10.3389/fmars.2017.00366

#include <SD.h>

#include "lidar/common.h"
#include "gps/common.h"
#include "debug.h"
#include "imu.h"

// SDcard SPI pins
#define SPI_CS  10

// Status LED pins
#define STATUS_R 5
#define STATUS_G 6
#define STATUS_B 7

#define STATUS_COLOR_BLACK    LOW,  LOW,  LOW
#define STATUS_COLOR_RED     HIGH,  LOW,  LOW
#define STATUS_COLOR_GREEN    LOW, HIGH,  LOW
#define STATUS_COLOR_YELLOW  HIGH, HIGH,  LOW
#define STATUS_COLOR_BLUE     LOW,  LOW, HIGH
#define STATUS_COLOR_MAGENTA HIGH,  LOW, HIGH
#define STATUS_COLOR_CYAN     LOW, HIGH, HIGH
#define STATUS_COLOR_WHITE   HIGH, HIGH, HIGH

enum ErrorType {
	ERR_NO_LIDAR = 1,
	ERR_NO_GPS_LOCK,
	ERR_IMU_FAIL,
	ERR_SD_FAIL,
	ERR_SD_CREATE_FAIL
};

// the logging file
static File logfile;

void set_status_color(uint8_t r, uint8_t g, uint8_t b) {
	digitalWrite(STATUS_R, r);
	digitalWrite(STATUS_G, g);
	digitalWrite(STATUS_B, b);
}

/**
 * Captures the execution of the program and report the status. The report is done through the status by blinking.
 * 
 * The number of flashes in each cycle encodes the error.
 * 
 * \param code The number of times the LED will flash each cycle.
 */
__ATTR_NORETURN__ void lock_and_report_error(size_t error_code) {
	while(1) {  // lock it up, blinking forever
		for(size_t i = 0; i < error_code; i++) {
			set_status_color(STATUS_COLOR_RED);
			delay(250);
			set_status_color(STATUS_COLOR_BLACK);
			delay(250);
		}
		delay(2000);
	}
}

#ifdef DEBUG_TO_SERIAL
size_t get_free_ram_size() {
	extern size_t __heap_start, *__brkval;
	size_t v;
	return &v - (__brkval == 0 ? &__heap_start : __brkval);
}
#endif

/**
 * Set the current date and time based on the last GPS result.
 * 
 * \param[out] date The date.
 * \param[out] time The time.
 */
void fat_datetime_callback(uint16_t *date, uint16_t *time) {
  if(gps.date.isValid())
	  *date = FAT_DATE(gps.date.year(), gps.date.month(), gps.date.day());

  if(gps.time.isValid())
  	*time = FAT_TIME(gps.time.hour(), gps.time.minute(), gps.time.second());
}

void setup() {
	// Set status LED yellow, to indicate initialisation
	pinMode(STATUS_R, OUTPUT);
	pinMode(STATUS_G, OUTPUT);
	pinMode(STATUS_B, OUTPUT);
	set_status_color(STATUS_COLOR_YELLOW);

#ifdef DEBUG_TO_SERIAL
	Serial.begin(9600);

	// Careful with this next line, if computer isn't attatched it will hang
	while(!Serial)  // loop while ProMicro takes a moment to get itself together
		if(millis() > 6000)  // give up waiting for USB cable plugin after 6 sec
			break;

	DEBUG(F("Free RAM: "));
	DEBUGLN(get_free_ram_size());
#endif

	if(!setup_lidar()) {
		DEBUGLN(F("LiDAR error."));
		lock_and_report_error(ERR_NO_LIDAR);
	}

	if(!setup_gps()) {
		DEBUGLN(F("GPS error."));
		lock_and_report_error(ERR_NO_GPS_LOCK);
	}

	if(!setup_imu()) {
		DEBUGLN(F("IMU error."));
		lock_and_report_error(ERR_IMU_FAIL);
	}
	imu.enableDefault();

	// see if the card is present and can be initialised
	if(!SD.begin(SPI_CS)) {
		DEBUGLN(F("SD card error."));
		lock_and_report_error(ERR_SD_FAIL);
	}

	consume_gps();

	// set SD file date time callback function
	SdFile::dateTimeCallback(fat_datetime_callback);

	// create a new file
	char filename[] = "LOG_0000.CSV";
	for(u16 i = 0; i < 10000; i++) {
		filename[4] = i / 1000 + '0';
		filename[5] = (i % 1000) / 100 + '0';
		filename[6] = (i % 100) / 10 + '0';
		filename[7] = i % 10 + '0';

		DEBUG(filename);
		DEBUG(F(" "))

		// Only open a new file if it doesn't exist
		if(!SD.exists(filename)) {
			// We consume the GPS data so we can set the creation date of the file
			logfile = SD.open(filename, FILE_WRITE);
			DEBUGLN(F("is available"));

			break;
		}
		DEBUGLN(F("already exists."));
	}

	consume_gps();

	delay(500);  // give it a chance to catch up before testing if it's ok.
	if(!logfile) {
		DEBUGLN(F("ERROR: couldn't create log file."));
		lock_and_report_error(ERR_SD_CREATE_FAIL);
	}

	logfile.println(F("# GPS and Laser Rangefinder logging with Pro Micro Arduino 3.3v"));
#ifndef DEBUG_TO_SERIAL
	logfile.println(F("# units:  accel=1g  gyro=deg/sec"));
#endif

	logfile.print(F("#gmt_date\tgmt_time\tnum_sats\tlongitude\tlatitude\t"));
	logfile.print(F("gps_altitude_m\tSOG_kt\tCOG\tHDOP\tlaser_altitude_cm\t"));
	logfile.println(F("tilt_deg\taccel_x\taccel_y\taccel_z\tgyro_x\tgyro_y\tgyro_z"));
	logfile.flush();

	// Should we wait a while for GPS to get a fix?
	wakeful_delay(2000);

	// Signal we are ready by blinking in cyan five times
	for(int i = 0; i < 5; i++) {
		set_status_color(STATUS_COLOR_BLACK);
		wakeful_delay(100);
		set_status_color(STATUS_COLOR_CYAN);
		wakeful_delay(100);
	}
}

#define __WRITE_GPS_MEASURE__(gps, stream, property, accessor) { \
	if(gps.property.isValid()) stream.print(gps.property.accessor()); \
	else stream.print(F("NaN")); \
	stream.print(F("\t")); \
}

/**
 * \param stream         The `Stream` to write to.
 * \param lidar_distance The distance read by the lidar, in centimetres.
 * \param imu_results    The results returned by the innertial mesurement unit.
 */
void write_data_line(Stream &stream, int16_t lidar_distance, const struct IMUData &imu_results) {
	if(gps.date.isValid()) {
		u16 year = gps.date.year();
		u8 month = gps.date.month();
		u8 day = gps.date.day();
		stream.print(year);
		stream.print(F("/"));
		if(month < 10) stream.print(F("0"));
		stream.print(month);
		stream.print(F("/"));
		if(day < 10) stream.print(F("0"));
		stream.print(day);
	} else {
		stream.print(F("INVALID"));
	}
	stream.print(F("\t"));
	if(gps.time.isValid()) {
		u8 hour = gps.time.hour();
		u8 minute = gps.time.minute();
		u8 second = gps.time.second();
		if(hour < 10) stream.print(F("0"));
		stream.print(hour);
		stream.print(F(":"));
		if(minute < 10) stream.print(F("0"));
		stream.print(minute);
		stream.print(F(":"));
		if(second < 10) stream.print(F("0"));
		stream.print(second);
	} else {
		stream.print(F("INVALID"));
	}
	stream.print(F("\t"));

	stream.print(gps.satellites.value());
	stream.print(F("\t"));
	if(gps.location.isValid()) {
		// The latitude and longitude floating point values are restricted to 32bit precision; so a total of 7 or 8
		// significant digits including those before the decimal point (1e-5 * 1852 * 60 = 1.11 meters)
		stream.print(gps.location.lng(), 6);
		stream.print(F("\t"));
		stream.print(gps.location.lat(), 6);
	} else {
		stream.print(F("NaN\tNaN"));
	}
	stream.print(F("\t"));
	
	__WRITE_GPS_MEASURE__(gps, stream, altitude, meters);
	__WRITE_GPS_MEASURE__(gps, stream, speed, knots);
	__WRITE_GPS_MEASURE__(gps, stream, course, deg);
	__WRITE_GPS_MEASURE__(gps, stream, hdop, value);
	
	if(lidar_distance == -1) stream.print(F("NaN"));
	else stream.print(lidar_distance);
	stream.print(F("\t"));
	
	stream.print(imu_results.tilt_deg, 2);
	stream.print(F("\t"));
	stream.print(imu_results.accel_x, 4);
	stream.print(F("\t"));
	stream.print(imu_results.accel_y, 4);
	stream.print(F("\t"));
	stream.print(imu_results.accel_z, 4);
	stream.print(F("\t"));
	stream.print(imu_results.gyro_x, 3);
	stream.print(F("\t"));
	stream.print(imu_results.gyro_y, 3);
	stream.print(F("\t"));
	stream.print(imu_results.gyro_z, 3);
	stream.println();
}

#undef __WRITE_GPS_MEASURE__

void loop(void) {
	// IMU
	IMUData imu_results;

	set_status_color(STATUS_COLOR_BLACK);

	// get GPS string
	consume_gps();

	// Output row without GPS data every 5 sec if no fix
	if(!gps.date.isUpdated() || gps.location.age() > 1750) {
		unsigned long first_detected = millis();
		unsigned long next_signal = 5000;

		// Make status LED blue when there is no fix
		set_status_color(STATUS_COLOR_BLUE);

		while(!gps.date.isUpdated()) {
			consume_gps();
			unsigned long delta_t = millis() - first_detected;

			if(delta_t > next_signal) {
				int16_t lidar_distance = get_lidar_distance_cm();

				get_imu_readings(imu_results);

				write_data_line(DEBUG_STREAM, lidar_distance, imu_results);

				// Make status LED magenta when writing...
				set_status_color(STATUS_COLOR_MAGENTA);
				write_data_line(logfile, lidar_distance, imu_results);
				set_status_color(STATUS_COLOR_BLUE);

				logfile.flush();

				next_signal += 5000;
			}
		}
	}

	// Make status LED green when everything is good
	set_status_color(STATUS_COLOR_GREEN);

	get_imu_readings(imu_results);

	// update gps data available scan again to clear the decks
	consume_gps();

	int16_t lidar_distance = get_lidar_distance_cm();

#ifdef DEBUG_TO_SERIAL
	// Printout to USB-serial
	write_data_line(DEBUG_STREAM, lidar_distance, imu_results);
#endif

	// write to SD card, making the status LED white
	set_status_color(STATUS_COLOR_WHITE);
	write_data_line(logfile, lidar_distance, imu_results);
	logfile.flush();
}
