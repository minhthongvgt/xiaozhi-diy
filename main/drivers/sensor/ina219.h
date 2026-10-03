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
esp_err_t ina219_init(i2c_master_bus_handle_t i2c_bus, ina219_handle_t *out_handle);
esp_err_t ina219_read(ina219_handle_t handle, float *out_bus_voltage_v, float *out_current_ma, float *out_power_mw);
void ina219_deinit(ina219_handle_t handle);
#ifdef __cplusplus
}
#endif
#endif 
