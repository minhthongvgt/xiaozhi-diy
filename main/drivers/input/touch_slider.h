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
esp_err_t touch_slider_init(gpio_num_t pad1_pin, gpio_num_t pad2_pin, gpio_num_t pad3_pin, touch_slider_handle_t *out_handle);
esp_err_t touch_slider_read(touch_slider_handle_t handle, uint8_t *out_position_pct, bool *out_is_touched);
esp_err_t touch_slider_deinit(touch_slider_handle_t handle);
#ifdef __cplusplus
}
#endif
#endif 
