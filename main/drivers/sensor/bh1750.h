/**
 * @file bh1750.h
 * @brief Rohm BH1750 Ambient Light Sensor Driver (ESP-IDF 6.1)
 */

#ifndef BH1750_H
#define BH1750_H

#include <esp_err.h>
#include <driver/i2c_master.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BH1750_I2C_ADDR_PRIMARY   0x23
#define BH1750_I2C_ADDR_SECONDARY 0x5C

typedef struct bh1750_dev_t* bh1750_handle_t;

/**
 * @brief Initialize BH1750 Ambient Light Sensor on I2C bus
 * Probes address 0x23 (fallback 0x5C), powers on and enters continuous H-resolution mode.
 *
 * @param i2c_bus Shared I2C master bus handle
 * @param[out] out_handle Returned device handle
 * @return ESP_OK if sensor is physically detected and ready,
 *         ESP_ERR_NOT_FOUND if device is absent, or error code
 */
esp_err_t bh1750_init(i2c_master_bus_handle_t i2c_bus, bh1750_handle_t *out_handle);

/**
 * @brief Read real ambient illuminance in Lux (0 - 65535 lux)
 *
 * @param handle Device handle
 * @param[out] out_lux Measured illuminance in lux
 * @return ESP_OK on valid real measurement, ESP_FAIL on communication error
 */
esp_err_t bh1750_read_lux(bh1750_handle_t handle, float *out_lux);

/**
 * @brief Deinitialize BH1750 sensor
 * @param handle Device handle
 */
void bh1750_deinit(bh1750_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // BH1750_H
