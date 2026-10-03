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
esp_err_t bh1750_init(i2c_master_bus_handle_t i2c_bus, bh1750_handle_t *out_handle);
esp_err_t bh1750_read_lux(bh1750_handle_t handle, float *out_lux);
void bh1750_deinit(bh1750_handle_t handle);
#ifdef __cplusplus
}
#endif
#endif 
