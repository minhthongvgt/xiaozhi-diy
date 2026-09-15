/**
 * @file bus_manager.h
 * @brief I2C and SPI Bus Arbitration and Centralized Resource Manager (ESP-IDF 6.1)
 */

#ifndef BUS_MANAGER_H
#define BUS_MANAGER_H

#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <driver/spi_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the primary I2C master bus using modern driver/i2c_master.h
 *
 * @param sda_pin GPIO pin for SDA
 * @param scl_pin GPIO pin for SCL
 * @param clk_speed_hz I2C clock frequency in Hz (e.g. 100000 or 400000)
 * @return ESP_OK on success
 */
esp_err_t bus_manager_init_i2c(gpio_num_t sda_pin, gpio_num_t scl_pin, uint32_t clk_speed_hz);

/**
 * @brief Get the centralized I2C master bus handle
 *
 * @return i2c_master_bus_handle_t or NULL if not initialized
 */
i2c_master_bus_handle_t bus_manager_get_i2c_bus(void);

/**
 * @brief Set the centralized I2C master bus handle (if initialized externally)
 *
 * @param bus Handle to active I2C master bus
 */
void bus_manager_set_i2c_bus(i2c_master_bus_handle_t bus);

/**
 * @brief Thread-safe registration of an I2C device on the centralized master bus
 *
 * @param dev_cfg Pointer to device configuration
 * @param[out] dev_handle Pointer to output handle
 * @return ESP_OK on success
 */
esp_err_t bus_manager_add_i2c_device(const i2c_device_config_t *dev_cfg, i2c_master_dev_handle_t *dev_handle);

/**
 * @brief Acquire exclusive lock on the I2C bus
 */
bool bus_manager_i2c_lock(TickType_t wait_ticks);

/**
 * @brief Release exclusive lock on the I2C bus
 */
void bus_manager_i2c_unlock(void);

/**
 * @brief Initialize the primary SPI host for shared peripherals (Display, SD Card, RFID)
 *
 * @param host_id SPI host (e.g. SPI2_HOST)
 * @param mosi_pin MOSI GPIO
 * @param miso_pin MISO GPIO (or GPIO_NUM_NC)
 * @param sclk_pin SCLK GPIO
 * @param max_transfer_sz Maximum transfer size in bytes
 * @return ESP_OK on success
 */
esp_err_t bus_manager_init_spi(spi_host_device_t host_id,
                               gpio_num_t mosi_pin,
                               gpio_num_t miso_pin,
                               gpio_num_t sclk_pin,
                               int max_transfer_sz);

/**
 * @brief Acquire SPI bus arbitration lock
 */
bool bus_manager_spi_lock(spi_host_device_t host_id, TickType_t wait_ticks);

/**
 * @brief Release SPI bus arbitration lock
 */
void bus_manager_spi_unlock(spi_host_device_t host_id);

/**
 * @brief Global bus manager initialization
 */
esp_err_t bus_manager_init(void);

#ifdef __cplusplus
}
#endif

#endif // BUS_MANAGER_H
