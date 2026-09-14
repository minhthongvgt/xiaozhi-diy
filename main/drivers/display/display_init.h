/**
 * @file display_init.h
 * @brief Display Subsystem Driver Initializer (TFT LCD, OLED, AMOLED)
 */

#ifndef DISPLAY_INIT_H
#define DISPLAY_INIT_H

#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize configured display panel and backlight
 */
esp_err_t display_subsystem_init(void);

/**
 * @brief Turn display backlight on/off
 */
void display_set_backlight(uint8_t brightness_pct);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_INIT_H
