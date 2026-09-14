/**
 * @file board_init.c
 * @brief ESP32-S3 WROOM Board Initialization Implementation
 */

#include "board_init.h"
#include "boards/common/bus_manager.h"
#include "gpio_validator.h"
#include <esp_log.h>
#include <sdkconfig.h>

#define TAG "BoardWroomInit"

esp_err_t board_esp32s3_wroom_init(void)
{
    ESP_LOGI(TAG, "Initializing ESP32-S3 WROOM-1 N16R8 Hardware Subsystems...");

    // 1. Validate GPIO safety at boot
    esp_err_t err = gpio_safety_validate();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Board initialization aborted: GPIO safety violation detected!");
        return err;
    }

    // 2. Initialize Bus Manager (I2C Master and SPI Host)
    err = bus_manager_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Bus Manager: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "ESP32-S3 WROOM Board hardware base initialized successfully.");
    return ESP_OK;
}
