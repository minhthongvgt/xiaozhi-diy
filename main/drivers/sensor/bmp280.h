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
esp_err_t bmp280_init(i2c_master_bus_handle_t i2c_bus, bmp280_handle_t *out_handle);
esp_err_t bmp280_read(bmp280_handle_t handle, float *out_pressure_hpa, float *out_temperature_c);
void bmp280_deinit(bmp280_handle_t handle);
#ifdef __cplusplus
}
#endif
#endif 
