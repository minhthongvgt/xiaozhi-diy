/**
 * @file board_init.c
 * @brief Generic Board Initialization Implementation
 */

#include "boards/board_init.h"
#include <esp_err.h>

#if defined(CONFIG_BOARD_TYPE_ESP32_S3_WROOM)
#include "boards/esp32s3_wroom/board_init.h"
#endif

#include "boards/common/bus_manager.h"
#include <sdkconfig.h>

esp_err_t board_init(void)
{
#if defined(CONFIG_BOARD_TYPE_ESP32_S3_WROOM)
    return board_esp32s3_wroom_init();
#elif defined(CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM) || defined(CONFIG_BOARD_TYPE_CUSTOM_S3_N16R8)
    // Initialize shared I2C bus early so sensor_init and input_init have active I2C hardware bus
    gpio_num_t sda = GPIO_NUM_8;
    gpio_num_t scl = GPIO_NUM_9;
#if defined(CONFIG_CUSTOM_TOUCH_PIN_SDA) && (CONFIG_CUSTOM_TOUCH_PIN_SDA >= 0)
    sda = (gpio_num_t)CONFIG_CUSTOM_TOUCH_PIN_SDA;
#elif defined(CONFIG_CUSTOM_DISPLAY_PIN_I2C_SDA) && (CONFIG_CUSTOM_DISPLAY_PIN_I2C_SDA >= 0)
    sda = (gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_I2C_SDA;
#endif

#if defined(CONFIG_CUSTOM_TOUCH_PIN_SCL) && (CONFIG_CUSTOM_TOUCH_PIN_SCL >= 0)
    scl = (gpio_num_t)CONFIG_CUSTOM_TOUCH_PIN_SCL;
#elif defined(CONFIG_CUSTOM_DISPLAY_PIN_I2C_SCL) && (CONFIG_CUSTOM_DISPLAY_PIN_I2C_SCL >= 0)
    scl = (gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_I2C_SCL;
#endif

    bus_manager_init_i2c(sda, scl, 400000);
    return ESP_OK;
#else
    return ESP_OK;
#endif
}
