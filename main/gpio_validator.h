#ifndef GPIO_VALIDATOR_H
#define GPIO_VALIDATOR_H
#include <stdbool.h>
#include <esp_err.h>
#include <driver/gpio.h>
#ifdef __cplusplus
extern "C" {
#endif
#define ESP32S3_N16R8_IS_RESERVED_PIN(p) ((p) >= 26 && (p) <= 37)
esp_err_t gpio_safety_validate(void);
esp_err_t gpio_validator_run(void);
const char* gpio_validator_get_error_log(void);
bool gpio_is_pin_safe(gpio_num_t pin, const char* periph_name);
#ifdef __cplusplus
}
#endif
#endif 
