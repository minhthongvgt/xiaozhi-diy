/**
 * @file rotary_encoder.h
 * @brief EC11 Rotary Encoder Driver (Quadrature Decoding & Key Button)
 * Provides real hardware rotation tracking and switch event handling. Zero simulated data.
 */

#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <esp_err.h>
#include <driver/gpio.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rotary_encoder_dev_t* rotary_encoder_handle_t;

/**
 * @brief Callback for rotation events
 * @param position Current absolute position
 * @param direction +1 for Clockwise (CW), -1 for Counter-Clockwise (CCW)
 * @param user_data User context
 */
typedef void (*rotary_encoder_rotate_cb_t)(int32_t position, int direction, void *user_data);

/**
 * @brief Callback for key button events
 * @param pressed true if pressed down, false if released
 * @param user_data User context
 */
typedef void (*rotary_encoder_key_cb_t)(bool pressed, void *user_data);

typedef struct {
    gpio_num_t pin_a;                       ///< Phase A (CLK) GPIO pin
    gpio_num_t pin_b;                       ///< Phase B (DT) GPIO pin
    gpio_num_t pin_key;                     ///< Switch / Key (SW) GPIO pin (-1 if unused)
    uint8_t steps_per_detent;               ///< State steps per physical click (normally 4 or 2, default 4)
    rotary_encoder_rotate_cb_t on_rotate;  ///< Optional rotation callback
    rotary_encoder_key_cb_t on_key;        ///< Optional key callback
    void *user_data;                        ///< User context for callbacks
} rotary_encoder_config_t;

/**
 * @brief Initialize EC11 Rotary Encoder hardware driver
 * @param config Configuration struct
 * @param out_handle Returned device handle
 * @return ESP_OK on success, or ESP_ERR_* on failure
 */
esp_err_t rotary_encoder_init(const rotary_encoder_config_t *config, rotary_encoder_handle_t *out_handle);

/**
 * @brief Read current absolute position count from encoder
 * @param handle Encoder device handle
 * @return Current position (ticks)
 */
int32_t rotary_encoder_get_position(rotary_encoder_handle_t handle);

/**
 * @brief Read delta position since last call and reset delta counter
 * @param handle Encoder device handle
 * @return Delta ticks since last call
 */
int32_t rotary_encoder_get_diff(rotary_encoder_handle_t handle);

/**
 * @brief Reset absolute position counter to a specific value
 * @param handle Encoder device handle
 * @param position New position value
 */
void rotary_encoder_set_position(rotary_encoder_handle_t handle, int32_t position);

/**
 * @brief Check if rotary encoder key (push button) is currently pressed
 * @param handle Encoder device handle
 * @return true if pressed (active low), false if released or key pin is NC
 */
bool rotary_encoder_is_key_pressed(rotary_encoder_handle_t handle);

/**
 * @brief Deinitialize and release resources for rotary encoder
 * @param handle Encoder device handle
 * @return ESP_OK on success
 */
esp_err_t rotary_encoder_deinit(rotary_encoder_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // ROTARY_ENCODER_H
