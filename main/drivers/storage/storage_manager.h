/**
 * @file storage_manager.h
 * @brief Storage Subsystem Manager (MicroSD Card SPI, NVS Flash)
 */

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <esp_err.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize configured storage systems (SD Card, NVS)
 */
esp_err_t storage_manager_init(void);

/**
 * @brief Check if MicroSD card is mounted and available
 */
bool storage_manager_is_sd_available(void);

#ifdef __cplusplus
}
#endif

#endif // STORAGE_MANAGER_H
