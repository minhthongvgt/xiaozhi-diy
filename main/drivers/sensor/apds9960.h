#ifndef APDS9960_H
#define APDS9960_H
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define APDS9960_I2C_ADDR 0x39
typedef enum apds9960_gesture_t {
    APDS9960_GESTURE_NONE = 0,
    APDS9960_GESTURE_UP,
    APDS9960_GESTURE_DOWN,
    APDS9960_GESTURE_LEFT,
    APDS9960_GESTURE_RIGHT,
} apds9960_gesture_t;
typedef struct apds9960_color_t {
    uint16_t clear;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
} apds9960_color_t;
typedef struct apds9960_dev_s *apds9960_handle_t;
esp_err_t apds9960_init(i2c_master_bus_handle_t i2c_bus, gpio_num_t int_pin, apds9960_handle_t *out_handle);
void apds9960_deinit(apds9960_handle_t handle);
esp_err_t apds9960_read_proximity(apds9960_handle_t handle, uint8_t *out_proximity);
esp_err_t apds9960_read_color(apds9960_handle_t handle, apds9960_color_t *out_color);
float apds9960_color_to_lux(const apds9960_color_t *color);
esp_err_t apds9960_read_gesture(apds9960_handle_t handle, uint32_t timeout_ms, apds9960_gesture_t *out_gesture);
const char *apds9960_gesture_name(apds9960_gesture_t gesture);
#ifdef __cplusplus
}
#endif
#endif
