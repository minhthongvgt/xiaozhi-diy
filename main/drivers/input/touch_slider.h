/**
 * @file touch_slider.h
 * @brief ESP32-S3 Onboard Capacitive Touch Slider Driver
 * Reads physical capacitance from 3 touch pads to calculate real slider position (0-100%).
 * Zero simulated data.
 */

#ifndef TOUCH_SLIDER_H
#define TOUCH_SLIDER_H

#include <esp_err.h>
#include <driver/gpio.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct touch_slider_dev_t* touch_slider_handle_t;

/**
 * @brief Initialize 3-pad capacitive touch slider
 * @param pad1_pin GPIO for Pad 1 (0% end)
 * @param pad2_pin GPIO for Pad 2 (Center 50%)
 * @param pad3_pin GPIO for Pad 3 (100% end)
 * @param out_handle Device handle returned
 * @return ESP_OK on success, or ESP_ERR_*
 */
esp_err_t touch_slider_init(gpio_num_t pad1_pin, gpio_num_t pad2_pin, gpio_num_t pad3_pin, touch_slider_handle_t *out_handle);

/**
 * @brief Read real capacitive touch slider status and position
 * @param handle Device handle
 * @param out_position_pct Calculated finger position percentage (0..100)
 * @param out_is_touched true if finger contact detected, false if untouched
 * @return ESP_OK on success
 */
esp_err_t touch_slider_read(touch_slider_handle_t handle, uint8_t *out_position_pct, bool *out_is_touched);

/**
 * @brief Deinitialize touch slider
 */
esp_err_t touch_slider_deinit(touch_slider_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // TOUCH_SLIDER_H
