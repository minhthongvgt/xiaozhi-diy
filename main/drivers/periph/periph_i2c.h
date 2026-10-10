#ifndef PERIPH_I2C_H
#define PERIPH_I2C_H
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#ifdef __cplusplus
extern "C" {
#endif
i2c_master_bus_handle_t periph_i2c_get_bus(gpio_num_t sda_pin, gpio_num_t scl_pin);
#ifdef __cplusplus
}
#endif
#endif
