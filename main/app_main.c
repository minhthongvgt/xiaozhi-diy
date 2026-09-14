/**
 * @file app_main.c
 * @brief Hardware Safety Gate and Subsystem Initialization Coordinator (C)
 */

#include "gpio_validator.h"
#include "boards/common/bus_manager.h"
#include "drivers/audio/audio_init.h"
#include "drivers/display/display_init.h"
#include "drivers/input/input_manager.h"
#include "drivers/sensor/sensor_manager.h"
#include "drivers/actuator/actuator_manager.h"
#include "drivers/storage/storage_manager.h"
#include "protocols/mcp_device.h"
#include <esp_log.h>

#define TAG "AppMainC"

esp_err_t app_main_hardware_init(void)
{
    ESP_LOGI(TAG, "===============================================");
    ESP_LOGI(TAG, "  Xiaozhi Custom Hardware Subsystem Init (C)   ");
    ESP_LOGI(TAG, "===============================================");

    // Step 1: Mandatory Hardware Safety Validation (Blocks GPIO 26-37)
    esp_err_t ret = gpio_safety_validate();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CRITICAL: GPIO Safety Validation failed! Hardware boot halted.");
        return ret;
    }

    // Step 2: Bus Manager (I2C/SPI Arbitration)
    bus_manager_init();

    // Step 3: Storage (NVS & SD)
    storage_manager_init();

    // Step 4: User Inputs (Buttons, Encoder, Touch)
    input_manager_init();

    // Step 5: Sensors
    sensor_manager_init();

    // Step 6: Actuators (Relay, Servo, Motors)
    actuator_manager_init();

    // Step 7: MCP Device-side Execution
    mcp_device_init();

    ESP_LOGI(TAG, "All hardware subsystems initialized successfully.");
    return ESP_OK;
}
