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
typedef void (*rotary_encoder_rotate_cb_t)(int32_t position, int direction, void *user_data);
typedef void (*rotary_encoder_key_cb_t)(bool pressed, void *user_data);
typedef struct {
    gpio_num_t pin_a;                       
    gpio_num_t pin_b;                       
    gpio_num_t pin_key;                     
    uint8_t steps_per_detent;               
    rotary_encoder_rotate_cb_t on_rotate;  
    rotary_encoder_key_cb_t on_key;        
    void *user_data;                        
} rotary_encoder_config_t;
esp_err_t rotary_encoder_init(const rotary_encoder_config_t *config, rotary_encoder_handle_t *out_handle);
int32_t rotary_encoder_get_position(rotary_encoder_handle_t handle);
int32_t rotary_encoder_get_diff(rotary_encoder_handle_t handle);
void rotary_encoder_set_position(rotary_encoder_handle_t handle, int32_t position);
bool rotary_encoder_is_key_pressed(rotary_encoder_handle_t handle);
esp_err_t rotary_encoder_deinit(rotary_encoder_handle_t handle);
#ifdef __cplusplus
}
#endif
#endif 
