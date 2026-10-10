#include <esp_err.h>
#include "drivers/actuator/actuator_init.h"
esp_err_t actuator_init(void)
{
    return actuator_manager_init();
}
