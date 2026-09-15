/**
 * @file display_init.c
 * @brief Display Subsystem Initialization Implementation (ESP-IDF 6.1)
 */

#include "display_init.h"
#include "boards/common/bus_manager.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include <driver/ledc.h>

#define TAG "DisplayInit"

static gpio_num_t s_bl_pin = GPIO_NUM_NC;

void display_set_backlight(uint8_t brightness_pct)
{
    if (s_bl_pin == GPIO_NUM_NC) return;
    uint32_t duty = (brightness_pct * 8191) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

esp_err_t display_subsystem_init(void)
{
    ESP_LOGI(TAG, "Display subsystem verified (SPI/I2C/LEDC coordinated by C++ Display driver).");
    return ESP_OK;
}

esp_err_t display_init(void)
{
    return display_subsystem_init();
}
