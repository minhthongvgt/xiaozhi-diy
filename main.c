/**
 * @file main.c
 * @brief ESP-IDF Firmware Boot Entry Point & Hardware Safety Gate
 */

#include <stdio.h>
#include <esp_log.h>
#include <esp_err.h>
#include <nvs_flash.h>

#define TAG "BootEntry"

// Forward declaration of the safety validator and application startup
extern esp_err_t gpio_safety_validate(void);
extern void app_main_init(void);

/**
 * @brief Main application entry for ESP-IDF
 */
void app_main(void)
{
    ESP_LOGI(TAG, "===============================================");
    ESP_LOGI(TAG, "   Xiaozhi AI Chatbot Firmware - Starting...   ");
    ESP_LOGI(TAG, "===============================================");

    // Step 1: Execute hardware GPIO safety validation before any peripheral power-on
#if defined(CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM)
    esp_err_t val_err = gpio_safety_validate();
    if (val_err != ESP_OK) {
        ESP_LOGE(TAG, "CRITICAL: GPIO Safety Validation Failed! Halting boot to prevent hardware damage.");
        return;
    }
    ESP_LOGI(TAG, "GPIO Safety Validation passed successfully.");
#endif

    // Step 2: Initialize Non-Volatile Storage (NVS)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing corrupted NVS partition...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Step 3: Call main application coordinator
    app_main_init();
}
