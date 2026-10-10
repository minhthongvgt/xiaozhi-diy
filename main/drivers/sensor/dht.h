#ifndef DHT_H
#define DHT_H
#include <esp_err.h>
#include <driver/gpio.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    DHT_TYPE_DHT11 = 0,
    DHT_TYPE_DHT22 = 1,
    DHT_TYPE_AUTO  = 2,
} dht_type_t;
esp_err_t dht_init(gpio_num_t pin);
esp_err_t dht_read_data(gpio_num_t pin, float *out_temp, float *out_humidity);
esp_err_t dht_read_raw(gpio_num_t pin, dht_type_t type, float *out_temp, float *out_humidity);
#ifdef __cplusplus
}
#endif
#endif 
