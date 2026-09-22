/**
 * @file storage_init.h
 * @brief Storage Subsystem Initialization Interface
 */

#ifndef DRIVERS_STORAGE_INIT_H
#define DRIVERS_STORAGE_INIT_H

#include <esp_err.h>
#include "storage_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize configured storage systems (SD Card, NVS)
 */
esp_err_t storage_init(void);

#ifdef __cplusplus
}
#endif

#endif // DRIVERS_STORAGE_INIT_H
