/**
 * @file input_manager.h
 * @brief Input Peripherals Driver Manager (Buttons, Touch, Rotary Encoder, Touch Slider)
 */

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <esp_err.h>
#include <driver/gpio.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all configured input devices
 */
esp_err_t input_manager_init(void);

/**
 * @brief Get current rotary encoder position (ticks)
 */
int32_t input_manager_get_encoder_value(void);

/**
 * @brief Get delta rotation ticks since last call and reset delta
 */
int32_t input_manager_get_encoder_diff(void);

/**
 * @brief Check if rotary encoder key button is currently pressed
 */
bool input_manager_is_encoder_key_pressed(void);

/**
 * @brief Read capacitive touch slider position (0..100%) and touch contact state
 * @param out_pos_pct Output position percentage (0..100)
 * @param out_touched Output touch status
 * @return ESP_OK if slider is active, ESP_ERR_NOT_FOUND if slider is disabled
 */
esp_err_t input_manager_get_touch_slider(uint8_t *out_pos_pct, bool *out_touched);

#ifdef __cplusplus
}
#endif

#endif // INPUT_MANAGER_H
