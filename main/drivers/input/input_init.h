/**
 * @file input_init.h
 * @brief Input Devices Initialization Interface
 */

#ifndef DRIVERS_INPUT_INIT_H
#define DRIVERS_INPUT_INIT_H

#include <esp_err.h>
#include "input_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all configured input devices
 */
esp_err_t input_init(void);

#ifdef __cplusplus
}
#endif

#endif // DRIVERS_INPUT_INIT_H
