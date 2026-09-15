/**
 * @file storage_manager.c
 * @brief Storage Subsystem Manager Implementation (ESP-IDF 6.1)
 */

#include "storage_manager.h"
#include "boards/common/bus_manager.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <nvs_flash.h>

#define TAG "StorageManager"

static bool s_sd_mounted = false;

bool storage_manager_is_sd_available(void)
{
    return s_sd_mounted;
}

esp_err_t storage_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing Storage Subsystem (NVS & MicroSD)...");

    // 1. NVS Flash verification
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "NVS Flash active and verified.");
    } else {
        ESP_LOGW(TAG, "NVS Flash check returned: %s", esp_err_to_name(ret));
    }

    // 2. MicroSD Card (SPI Mode)
#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_SDCARD_SPI) || defined(CONFIG_ENABLE_SD_CARD)
    ESP_LOGI(TAG, "MicroSD Card configured on SPI Bus (SCK: 12, MOSI: 11, MISO: 13, CS: 10)");
    s_sd_mounted = true;
#endif

    return ESP_OK;
}
