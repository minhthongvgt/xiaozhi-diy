#ifndef SY6970_H
#define SY6970_H
#include <esp_err.h>
#include <driver/i2c_master.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define SY6970_I2C_ADDR 0x6A
typedef struct sy6970_status_t {
    bool  charging;
    bool  power_good;
    bool  charge_done;
    int   battery_mv;
    int   battery_pct;
} sy6970_status_t;
typedef struct sy6970_dev_s *sy6970_handle_t;
esp_err_t sy6970_init(i2c_master_bus_handle_t i2c_bus, sy6970_handle_t *out_handle);
void sy6970_deinit(sy6970_handle_t handle);
esp_err_t sy6970_read_status(sy6970_handle_t handle, sy6970_status_t *out_status);
#ifdef __cplusplus
}
#endif
#endif
