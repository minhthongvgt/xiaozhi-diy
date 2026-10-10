#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H
#include <esp_err.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
esp_err_t storage_manager_init(void);
bool storage_manager_is_sd_available(void);
#ifdef __cplusplus
}
#endif
#endif 
