/**
 * @file aht20.h
 * @brief Aosong AHT10 / AHT20 / AHT21 Digital I2C Temperature & Humidity Sensor Driver (ESP-IDF 6.1)
 */

#ifndef AHT20_H
#define AHT20_H

#include <esp_err.h>
#include <driver/i2c_master.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AHT20_I2C_ADDR 0x38

typedef struct aht20_dev_t* aht20_handle_t;

/**
 * @brief Initialize AHT20 / AHT21 Sensor on I2C bus
 * Probes address 0x38, checks calibration status, and sends calibration init if required.
 *
 * @param i2c_bus Shared I2C master bus handle
 * @param[out] out_handle Returned device handle
 * @return ESP_OK if sensor is physically detected and ready,
 *         ESP_ERR_NOT_FOUND if device is absent, or error code
 */
esp_err_t aht20_init(i2c_master_bus_handle_t i2c_bus, aht20_handle_t *out_handle);

/**
 * @brief Read real temperature and relative humidity from AHT20
 *
 * @param handle AHT20 handle
 * @param[out] out_temp_c Temperature in degrees Celsius (-40 to +85 C)
 * @param[out] out_hum_pct Relative humidity in percent (0 to 100 %)
 * @return ESP_OK on valid real measurement, ESP_FAIL on CRC/timeout/busy
 */
esp_err_t aht20_read(aht20_handle_t handle, float *out_temp_c, float *out_hum_pct);

/**
 * @brief Deinitialize AHT20 sensor
 * @param handle Device handle
 */
void aht20_deinit(aht20_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // AHT20_H
