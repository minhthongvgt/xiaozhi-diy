/**
 * @file actuator_init.h
 * @brief Actuators Subsystem Initialization Interface
 */

#ifndef DRIVERS_ACTUATOR_INIT_H
#define DRIVERS_ACTUATOR_INIT_H

#include <esp_err.h>
#include "actuator_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all configured actuator devices
 */
esp_err_t actuator_init(void);

#ifdef __cplusplus
}
#endif

#endif // DRIVERS_ACTUATOR_INIT_H
