#pragma once
#include <stdint.h>
#include <driver/gpio.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    gpio_num_t pin1;
    gpio_num_t pin2;
    char label[32];
} extra_gpio_config_t;
int parse_extra_gpios(const char* config_str, extra_gpio_config_t* out_configs, int max_configs);
#ifdef __cplusplus
}
#endif
