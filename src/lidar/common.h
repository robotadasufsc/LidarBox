#pragma once

#include <inttypes.h>

/**
 * Starts the lidar hardware and setup communication.
 * 
 * @return wether it was started correctly
 * @retval true successfully started
 * @retval false error during lidar initialisation
 */
bool setup_lidar();

/**
 * Reads the distance from the lidar. The reading is returned in centimetres.
 * 
 * @return the distance in cenimetres
 * @retval -1 if it was not able to read the distance
 */
int16_t get_lidar_distance_cm();
