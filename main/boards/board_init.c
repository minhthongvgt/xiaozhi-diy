#include "boards/board_init.h"
#include <esp_err.h>
#include <sdkconfig.h>
#if CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM
#include "boards/common/bus_manager.h"
#include "boards/esp32s3-n16r8-custom/i2c_priority.h"
#endif
esp_err_t board_init(void)
{
#if CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM
    gpio_num_t sda = (gpio_num_t)-1;
    gpio_num_t scl = (gpio_num_t)-1;
    board_resolve_i2c_pins(&sda, &scl);
    if (!GPIO_IS_VALID_GPIO(sda) || !GPIO_IS_VALID_GPIO(scl)) {
        return ESP_OK;
    }
    bus_manager_init_i2c(sda, scl, 400000);
#endif
    return ESP_OK;
}
