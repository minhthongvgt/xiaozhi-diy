#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H
#include <esp_err.h>
#include <driver/gpio.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
esp_err_t input_manager_init(void);
int32_t input_manager_get_encoder_value(void);
int32_t input_manager_get_encoder_diff(void);
bool input_manager_is_encoder_key_pressed(void);
esp_err_t input_manager_get_touch_slider(uint8_t *out_pos_pct, bool *out_touched);
#ifdef __cplusplus
}
#endif
#endif 
