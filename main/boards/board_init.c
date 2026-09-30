/**
 * @file board_init.c
 * @brief Generic Board Initialization Implementation
 */

#include "boards/board_init.h"
#include <esp_err.h>
#include <sdkconfig.h>
#include "boards/common/bus_manager.h"
#include "boards/esp32s3-n16r8-custom/i2c_priority.h"

esp_err_t board_init(void)
{
    // Initialize shared I2C bus early so sensor_init and input_init have active I2C hardware bus.
    // Pin priority is resolved by the single shared board_resolve_i2c_pins() helper so this
    // logic is not duplicated with CustomN16R8Board::InitializeI2c().
    gpio_num_t sda = (gpio_num_t)-1;
    gpio_num_t scl = (gpio_num_t)-1;
    board_resolve_i2c_pins(&sda, &scl);

    if (!GPIO_IS_VALID_GPIO(sda) || !GPIO_IS_VALID_GPIO(scl)) {
        return ESP_OK;
    }

    bus_manager_init_i2c(sda, scl, 400000);
    return ESP_OK;
}
