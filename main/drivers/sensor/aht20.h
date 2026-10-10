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
esp_err_t aht20_init(i2c_master_bus_handle_t i2c_bus, aht20_handle_t *out_handle);
esp_err_t aht20_read(aht20_handle_t handle, float *out_temp_c, float *out_hum_pct);
void aht20_deinit(aht20_handle_t handle);
#ifdef __cplusplus
}
#endif
#endif 
