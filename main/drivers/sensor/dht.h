/**
 * @file dht.h
 * @brief DHT11 / DHT22 Single-Wire Temperature & Humidity Sensor Driver (ESP-IDF 6.1)
 */

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

/**
 * @brief Initialize GPIO pin for DHT sensor with internal pull-up
 * @param pin User-selected GPIO pin number
 * @return esp_err_t ESP_OK on success
 */
esp_err_t dht_init(gpio_num_t pin);

/**
 * @brief Read temperature and humidity with automatic 1.5s caching
 * @param pin User-selected GPIO pin number
 * @param[out] out_temp Temperature in degrees Celsius
 * @param[out] out_humidity Relative humidity percentage (0-100%)
 * @return esp_err_t ESP_OK on success, ESP_ERR_TIMEOUT or ESP_ERR_INVALID_CRC on failure
 */
esp_err_t dht_read_data(gpio_num_t pin, float *out_temp, float *out_humidity);

/**
 * @brief Low-level un-cached raw read from DHT sensor
 * @param pin User-selected GPIO pin number
 * @param type Sensor type (DHT11, DHT22 or AUTO)
 * @param[out] out_temp Temperature in degrees Celsius
 * @param[out] out_humidity Relative humidity percentage
 * @return esp_err_t ESP_OK on success
 */
esp_err_t dht_read_raw(gpio_num_t pin, dht_type_t type, float *out_temp, float *out_humidity);

#ifdef __cplusplus
}
#endif

#endif // DHT_H
