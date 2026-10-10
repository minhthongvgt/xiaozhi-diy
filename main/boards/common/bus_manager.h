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
esp_err_t bus_manager_init_i2c(gpio_num_t sda_pin, gpio_num_t scl_pin, uint32_t clk_speed_hz);
i2c_master_bus_handle_t bus_manager_get_i2c_bus(void);
void bus_manager_set_i2c_bus(i2c_master_bus_handle_t bus);
esp_err_t bus_manager_add_i2c_device(const i2c_device_config_t *dev_cfg, i2c_master_dev_handle_t *dev_handle);
bool bus_manager_i2c_lock(TickType_t wait_ticks);
void bus_manager_i2c_unlock(void);
esp_err_t bus_manager_probe_i2c_address(uint8_t address);
esp_err_t bus_manager_init_spi(spi_host_device_t host_id,
                               gpio_num_t mosi_pin,
                               gpio_num_t miso_pin,
                               gpio_num_t sclk_pin,
                               int max_transfer_sz);
bool bus_manager_spi_lock(spi_host_device_t host_id, TickType_t wait_ticks);
void bus_manager_spi_unlock(spi_host_device_t host_id);
esp_err_t bus_manager_init(void);
#ifdef __cplusplus
}
#endif
#endif 
