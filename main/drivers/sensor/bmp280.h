/**
 * @file bmp280.h
 * @brief Bosch BMP280 / BME280 Barometric Pressure & Temperature Sensor Driver (ESP-IDF 6.1)
 * Standardized I2C driver reading real atmospheric pressure and temperature.
 */

#ifndef BMP280_H
#define BMP280_H

#include <esp_err.h>
#include <driver/i2c_master.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BMP280_I2C_ADDR_PRIMARY   0x76
#define BMP280_I2C_ADDR_SECONDARY 0x77

typedef struct bmp280_dev_t* bmp280_handle_t;

/**
 * @brief Initialize BMP280 / BME280 Sensor on I2C bus
 * Probes address 0x76 (fallback 0x77), reads chip ID (0x58 or 0x60) and calibration constants.
 *
 * @param i2c_bus Shared I2C master bus handle
 * @param[out] out_handle Returned device handle
 * @return ESP_OK if sensor is physically detected and initialized,
 *         ESP_ERR_NOT_FOUND if device is absent, or error code
 */
esp_err_t bmp280_init(i2c_master_bus_handle_t i2c_bus, bmp280_handle_t *out_handle);

/**
 * @brief Read real barometric pressure in hPa and temperature in Celsius
 * Uses official Bosch Sensortec integer/floating-point compensation equations.
 *
 * @param handle BMP280 handle
 * @param[out] out_pressure_hpa Measured atmospheric pressure in hPa (e.g. 1013.25 hPa)
 * @param[out] out_temperature_c Measured ambient temperature in degrees Celsius
 * @return ESP_OK on valid real measurement, ESP_FAIL on communication error
 */
esp_err_t bmp280_read(bmp280_handle_t handle, float *out_pressure_hpa, float *out_temperature_c);

/**
 * @brief Deinitialize BMP280 sensor and free resources
 * @param handle Device handle
 */
void bmp280_deinit(bmp280_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // BMP280_H
