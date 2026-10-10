#ifndef LED_IC_H
#define LED_IC_H
#include <esp_err.h>
#include <driver/i2c_master.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum led_ic_type_t {
    LED_IC_TYPE_UNKNOWN = 0,
    LED_IC_TYPE_AW9523B,
    LED_IC_TYPE_IS31FL3236,
} led_ic_type_t;
typedef struct led_ic_config_t {
    i2c_master_bus_handle_t i2c_bus;
    bool                    allow_oled_addresses;
} led_ic_config_t;
typedef struct led_ic_dev_s *led_ic_handle_t;
esp_err_t led_ic_init(const led_ic_config_t *config, led_ic_handle_t *out_handle);
void led_ic_deinit(led_ic_handle_t handle);
led_ic_type_t led_ic_get_type(led_ic_handle_t handle);
const char *led_ic_type_name(led_ic_type_t type);
uint8_t led_ic_get_channel_count(led_ic_handle_t handle);
uint8_t led_ic_get_address(led_ic_handle_t handle);
esp_err_t led_ic_set_channel(led_ic_handle_t handle, uint8_t channel, uint8_t brightness);
esp_err_t led_ic_set_all(led_ic_handle_t handle, uint8_t brightness);
#ifdef __cplusplus
}
#endif
#endif
