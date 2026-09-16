/**
 * @file board_init.h
 * @brief ESP32-S3 WROOM Board Initialization Interface
 */

#ifndef BOARD_INIT_H
#define BOARD_INIT_H

#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all hardware components configured for the ESP32-S3 WROOM board
 */
esp_err_t board_esp32s3_wroom_init(void);

#ifdef __cplusplus
}
#endif

#endif // BOARD_INIT_H
