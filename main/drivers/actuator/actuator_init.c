/**
 * @file actuator_init.c
 * @brief Actuators Subsystem Initialization Implementation
 */

#include "drivers/actuator/actuator_init.h"

esp_err_t actuator_init(void)
{
    return actuator_manager_init();
}
