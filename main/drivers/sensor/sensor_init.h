/**
 * @file sensor_init.h
 * @brief Sensor Subsystem Initialization Interface
 */

#ifndef DRIVERS_SENSOR_INIT_H
#define DRIVERS_SENSOR_INIT_H

#include <esp_err.h>
#include "sensor_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all configured sensor devices
 */
esp_err_t sensor_init(void);

#ifdef __cplusplus
}
#endif

#endif // DRIVERS_SENSOR_INIT_H
