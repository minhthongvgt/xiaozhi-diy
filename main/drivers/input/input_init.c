/**
 * @file input_init.c
 * @brief Input Devices Initialization Implementation
 */

#include <esp_err.h>
#include "drivers/input/input_init.h"

esp_err_t input_init(void)
{
    return input_manager_init();
}
