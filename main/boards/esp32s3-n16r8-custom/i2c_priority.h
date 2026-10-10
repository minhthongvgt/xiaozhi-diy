#ifndef BOARDS_ESP32S3_N16R8_CUSTOM_I2C_PRIORITY_H
#define BOARDS_ESP32S3_N16R8_CUSTOM_I2C_PRIORITY_H
#include <driver/gpio.h>
#ifdef __cplusplus
extern "C" {
#endif
void board_resolve_i2c_pins(gpio_num_t *sda_pin, gpio_num_t *scl_pin);
#ifdef __cplusplus
}
#endif
#endif 
