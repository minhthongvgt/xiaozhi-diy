/**
 * @file board_init.c
 * @brief Generic Board Initialization Implementation
 */

#include "boards/board_init.h"
#include <esp_err.h>

#if defined(CONFIG_BOARD_TYPE_ESP32_S3_WROOM)
#include "boards/esp32s3_wroom/board_init.h"
#endif

esp_err_t board_init(void)
{
#if defined(CONFIG_BOARD_TYPE_ESP32_S3_WROOM)
    return board_esp32s3_wroom_init();
#elif defined(CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM) || defined(CONFIG_BOARD_TYPE_CUSTOM_S3_N16R8)
    // The Custom N16R8 board initialization is fully handled in C++ via WifiBoard/Application
    return ESP_OK;
#else
    return ESP_OK;
#endif
}
