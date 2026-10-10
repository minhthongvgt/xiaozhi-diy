#ifndef HCSR04_H
#define HCSR04_H
#include <esp_err.h>
#include <hal/gpio_types.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct hcsr04_dev_t* hcsr04_handle_t;
esp_err_t hcsr04_init(gpio_num_t trig_pin, gpio_num_t echo_pin, hcsr04_handle_t *out_handle);
esp_err_t hcsr04_read_distance(hcsr04_handle_t handle, float *out_distance_cm);
void hcsr04_deinit(hcsr04_handle_t handle);
#ifdef __cplusplus
}
#endif
#endif 
