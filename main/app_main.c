#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "gpio_validator.h"
#include "bus_manager.h"
#include "boards/board_init.h"
#include "drivers/input/input_init.h"
#include "drivers/display/display_init.h"
#include "drivers/audio/audio_init.h"
#include "drivers/sensor/sensor_init.h"
#include "drivers/actuator/actuator_init.h"
#include "drivers/storage/storage_init.h"
#include "protocols/mcp_device.h"
static const char *TAG = "XIAOZHI_MAIN";
static esp_err_t init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    return ESP_OK;
}
static esp_err_t init_event_loop(void)
{
    esp_err_t ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(ret);
    }
    return ESP_OK;
}
esp_err_t app_main_hardware_init(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔════════════════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║     🤖 XIAOZHI AI CHATBOT - CUSTOM DIY BUILDER 1.0.0    ║");
    ESP_LOGI(TAG, "║        ESP32-S3-WROOM-1 N16R8 (16MB+8MB PSRAM)          ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "[1/7] 📦 Initializing NVS (Non-Volatile Storage)...");
    if (init_nvs() != ESP_OK) {
        ESP_LOGE(TAG, "❌ NVS initialization FAILED!");
        esp_restart();
    }
    ESP_LOGI(TAG, "     ✅ NVS initialized successfully");
    ESP_LOGI(TAG, "[2/7] 🔔 Initializing Event Loop...");
    if (init_event_loop() != ESP_OK) {
        ESP_LOGE(TAG, "❌ Event Loop initialization FAILED!");
        esp_restart();
    }
    ESP_LOGI(TAG, "     ✅ Event Loop initialized");
    ESP_LOGI(TAG, "[3/7] 🛡️  Running GPIO Safety Validator...");
    if (gpio_validator_run() != ESP_OK) {
        ESP_LOGE(TAG, "🔴 ========== GPIO VALIDATION FAILED ==========");
        ESP_LOGE(TAG, "❌ GPIO configuration is INVALID or CONFLICTING!");
        ESP_LOGE(TAG, "   Check your menuconfig or board pinout.");
        ESP_LOGE(TAG, "%s", gpio_validator_get_error_log());
        ESP_LOGE(TAG, "🔴 ========== SYSTEM HALTED FOR SAFETY =========");
        return ESP_ERR_INVALID_STATE;
    }
    ESP_LOGI(TAG, "     ✅ GPIO Safety Validation PASSED ✓");
    ESP_LOGI(TAG, "[4/7] 🚌 Initializing Bus Manager (I2C/SPI)...");
    if (bus_manager_init() != ESP_OK) {
        ESP_LOGE(TAG, "❌ Bus Manager initialization FAILED!");
        esp_restart();
    }
    ESP_LOGI(TAG, "     ✅ Bus Manager initialized");
    ESP_LOGI(TAG, "[5/7] 🎛️  Initializing Board-Specific Configuration...");
    if (board_init() != ESP_OK) {
        ESP_LOGE(TAG, "❌ Board initialization FAILED!");
        esp_restart();
    }
    ESP_LOGI(TAG, "     ✅ Board initialized");
    ESP_LOGI(TAG, "[6/7] ⚙️  Initializing Peripherals...");
    if (input_init() != ESP_OK) {
        ESP_LOGW(TAG, "     ⚠️  Input devices init warning (non-critical)");
    } else {
        ESP_LOGI(TAG, "     ✅ Input devices initialized");
    }
    if (display_init() != ESP_OK) {
        ESP_LOGW(TAG, "     ⚠️  Display init warning (non-critical)");
    } else {
        ESP_LOGI(TAG, "     ✅ Display initialized");
    }
    if (audio_init() != ESP_OK) {
        ESP_LOGW(TAG, "     ⚠️  Audio init warning (non-critical)");
    } else {
        ESP_LOGI(TAG, "     ✅ Audio system initialized");
    }
    if (sensor_init() != ESP_OK) {
        ESP_LOGW(TAG, "     ⚠️  Sensor init warning (non-critical)");
    } else {
        ESP_LOGI(TAG, "     ✅ Sensors initialized");
    }
    if (actuator_init() != ESP_OK) {
        ESP_LOGW(TAG, "     ⚠️  Actuator init warning (non-critical)");
    } else {
        ESP_LOGI(TAG, "     ✅ Actuators initialized");
    }
    if (storage_init() != ESP_OK) {
        ESP_LOGW(TAG, "     ⚠️  Storage init warning (non-critical)");
    } else {
        ESP_LOGI(TAG, "     ✅ Storage initialized");
    }
    mcp_device_init();
    ESP_LOGI(TAG, "[7/7] 🎉 Hardware Subsystem Ready!");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔════════════════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║                🎯 XIAOZHI IS READY                      ║");
    ESP_LOGI(TAG, "║          Say: 'Hello XiaoZhi' to wake up!              ║");
    ESP_LOGI(TAG, "║   Supported languages: Chinese, English, Vietnamese    ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    return ESP_OK;
}
