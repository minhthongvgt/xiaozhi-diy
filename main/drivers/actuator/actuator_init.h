#ifndef DRIVERS_ACTUATOR_INIT_H
#define DRIVERS_ACTUATOR_INIT_H
#include <esp_err.h>
#include "drivers/actuator/actuator_manager.h"
#ifdef __cplusplus
extern "C" {
#endif
esp_err_t actuator_init(void);
#ifdef __cplusplus
}
#endif
#endif 
