/**
 * @file gpio_validator.h
 * @brief ESP32-S3 N16R8 Hardware GPIO Safety Validator
 */

#ifndef GPIO_VALIDATOR_H
#define GPIO_VALIDATOR_H

#include <stdbool.h>
#include <esp_err.h>
#include <driver/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Macro to determine if a GPIO pin is in the forbidden Octal PSRAM/Flash range [26..37]
 */
#define ESP32S3_N16R8_IS_RESERVED_PIN(p) ((p) >= 26 && (p) <= 37)

/**
 * @brief Validates all active GPIO configurations at boot time.
 *
 * Checks:
 * 1. Prohibits any active peripheral from using pins in the range 26..37 on ESP32-S3 N16R8.
 * 2. Checks for conflicting pin allocations among independent peripherals.
 *    (Permits legitimate I2C bus sharing and I2S Duplex clock sharing).
 *
 * @return ESP_OK if all configurations are safe and valid.
 *         ESP_ERR_INVALID_STATE if a critical pin conflict or forbidden pin is detected.
 */
esp_err_t gpio_safety_validate(void);

/**
 * @brief Alias for gpio_safety_validate() as specified in review report
 */
esp_err_t gpio_validator_run(void);

/**
 * @brief Get human-readable error log if validation fails
 */
const char* gpio_validator_get_error_log(void);

/**
 * @brief Checks if a single pin is valid and safe for peripheral use.
 *
 * @param pin The GPIO pin number.
 * @param periph_name Descriptive name of the peripheral for logging.
 * @return true if safe, false if forbidden.
 */
bool gpio_is_pin_safe(gpio_num_t pin, const char* periph_name);

#ifdef __cplusplus
}
#endif

#endif // GPIO_VALIDATOR_H
