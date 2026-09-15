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
    static bool s_board_initialized = false;
    if (s_board_initialized) {
        ESP_LOGD(TAG, "ESP32-S3 WROOM Board hardware already initialized.");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing ESP32-S3 WROOM-1 N16R8 Hardware Subsystems...");

    // Bus manager and GPIO validator are safely coordinated by app_main_hardware_init
    s_board_initialized = true;
    ESP_LOGI(TAG, "ESP32-S3 WROOM Board hardware base initialized successfully.");
    return ESP_OK;
}
