/**
 * @file scd4x.h
 * @brief Sensirion SCD40 / SCD41 Photoacoustic NDIR CO2 Sensor Driver (ESP-IDF 6.1)
 * Reads genuine CO2, temperature, and humidity over I2C (address 0x62).
 */

#ifndef SCD4X_H
#define SCD4X_H

#include <esp_err.h>
#include <driver/i2c_master.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCD4X_I2C_ADDR 0x62

typedef struct scd4x_dev_t* scd4x_handle_t;

/**
 * @brief Initialize Sensirion SCD40 / SCD41 NDIR CO2 Sensor
 * @param i2c_bus Shared I2C master bus handle
 * @param[out] out_handle Returned device handle
 * @return ESP_OK if SCD40 is physically present and responsive on I2C bus,
 *         ESP_ERR_NOT_FOUND if device is absent, or error code
 */
esp_err_t scd4x_init(i2c_master_bus_handle_t i2c_bus, scd4x_handle_t *out_handle);

/**
 * @brief Read genuine CO2 concentration, temperature, and relative humidity
 * @param handle SCD4x handle
 * @param[out] out_co2_ppm Measured CO2 in PPM (400 - 5000 ppm)
 * @param[out] out_temp_c Measured temperature in degrees Celsius
 * @param[out] out_hum_pct Measured relative humidity in %
 * @return ESP_OK on valid real measurement,
 *         ESP_ERR_INVALID_STATE if measurement is still in progress / not ready,
 *         ESP_FAIL if CRC check fails or communication errors out
 */
esp_err_t scd4x_read_measurement(scd4x_handle_t handle, float *out_co2_ppm, float *out_temp_c, float *out_hum_pct);

/**
 * @brief Deinitialize SCD4x sensor and free allocated memory
 * @param handle Device handle
 */
void scd4x_deinit(scd4x_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // SCD4X_H
