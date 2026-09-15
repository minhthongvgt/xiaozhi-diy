/**
 * @file input_manager.h
 * @brief Input Peripherals Driver Manager (Buttons, Touch, Rotary Encoder, Gesture)
 */

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <esp_err.h>
#include <driver/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*button_event_cb_t)(int button_id, bool pressed);

/**
 * @brief Initialize all configured input devices
 */
esp_err_t input_manager_init(void);

/**
 * @brief Register callback for physical button events
 */
void input_manager_register_button_callback(button_event_cb_t cb);

/**
 * @brief Get current rotary encoder position
 */
int32_t input_manager_get_encoder_value(void);

#ifdef __cplusplus
}
#endif

#endif // INPUT_MANAGER_H
