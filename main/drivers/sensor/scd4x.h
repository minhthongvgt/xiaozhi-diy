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
esp_err_t scd4x_init(i2c_master_bus_handle_t i2c_bus, scd4x_handle_t *out_handle);
esp_err_t scd4x_read_measurement(scd4x_handle_t handle, float *out_co2_ppm, float *out_temp_c, float *out_hum_pct);
void scd4x_deinit(scd4x_handle_t handle);
#ifdef __cplusplus
}
#endif
#endif 
