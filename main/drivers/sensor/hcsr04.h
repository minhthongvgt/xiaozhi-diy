/**
 * @file hcsr04.h
 * @brief HC-SR04 / US-100 Ultrasonic Distance Sensor Driver (ESP-IDF 6.1)
 */

#ifndef HCSR04_H
#define HCSR04_H

#include <esp_err.h>
#include <hal/gpio_types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hcsr04_dev_t* hcsr04_handle_t;

/**
 * @brief Initialize HC-SR04 Ultrasonic Sensor
 *
 * @param trig_pin GPIO pin connected to Trigger
 * @param echo_pin GPIO pin connected to Echo
 * @param[out] out_handle Returned device handle
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if pins are invalid
 */
esp_err_t hcsr04_init(gpio_num_t trig_pin, gpio_num_t echo_pin, hcsr04_handle_t *out_handle);

/**
 * @brief Measure genuine distance in centimeters (2.0 to 400.0 cm)
 *
 * @param handle HC-SR04 handle
 * @param[out] out_distance_cm Measured distance in centimeters
 * @return ESP_OK on valid echo received,
 *         ESP_ERR_TIMEOUT if no obstacle detected or sensor disconnected
 */
esp_err_t hcsr04_read_distance(hcsr04_handle_t handle, float *out_distance_cm);

/**
 * @brief Deinitialize HC-SR04 sensor
 * @param handle Device handle
 */
void hcsr04_deinit(hcsr04_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // HCSR04_H
