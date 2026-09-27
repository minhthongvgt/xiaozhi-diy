/**
 * @file ina219.h
 * @brief Texas Instruments INA219 / INA226 Bidirectional Current & Power Monitor Driver (ESP-IDF 6.1)
 */

#ifndef INA219_H
#define INA219_H

#include <esp_err.h>
#include <driver/i2c_master.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INA219_I2C_ADDR_DEFAULT 0x40

typedef struct ina219_dev_t* ina219_handle_t;

/**
 * @brief Initialize INA219 Power Monitor on I2C bus
 *
 * @param i2c_bus Shared I2C master bus handle
 * @param[out] out_handle Returned device handle
 * @return ESP_OK if sensor is physically detected on I2C (address 0x40),
 *         ESP_ERR_NOT_FOUND if device is absent, or error code
 */
esp_err_t ina219_init(i2c_master_bus_handle_t i2c_bus, ina219_handle_t *out_handle);

/**
 * @brief Read real bus voltage, current, and power
 *
 * @param handle Device handle
 * @param[out] out_bus_voltage_v Measured bus voltage in Volts (e.g. 3.7V - 12.6V)
 * @param[out] out_current_ma Measured load/charge current in mA
 * @param[out] out_power_mw Measured power in mW
 * @return ESP_OK on valid real measurement, ESP_FAIL on communication error
 */
esp_err_t ina219_read(ina219_handle_t handle, float *out_bus_voltage_v, float *out_current_ma, float *out_power_mw);

/**
 * @brief Deinitialize INA219 sensor
 * @param handle Device handle
 */
void ina219_deinit(ina219_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // INA219_H
