/**
 * @file sensor_init.c
 * @brief Sensor Subsystem Initialization Implementation
 */

#include <esp_err.h>
#include "drivers/sensor/sensor_init.h"

esp_err_t sensor_init(void)
{
    return sensor_manager_init();
}
