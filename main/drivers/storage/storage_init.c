/**
 * @file storage_init.c
 * @brief Storage Subsystem Initialization Implementation
 */

#include "drivers/storage/storage_init.h"

esp_err_t storage_init(void)
{
    return storage_manager_init();
}
