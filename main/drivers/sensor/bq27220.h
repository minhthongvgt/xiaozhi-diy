#ifndef BQ27220_H
#define BQ27220_H
#include <esp_err.h>
#include <driver/i2c_master.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define BQ27220_I2C_ADDR 0x55
typedef struct bq27220_data_t {
    float    voltage_v;
    float    current_ma;
    float    average_current_ma;
    float    temperature_c;
    int      state_of_charge_pct;
    int      state_of_health_pct;
    uint16_t remaining_capacity_mah;
    uint16_t full_charge_capacity_mah;
    uint16_t time_to_empty_min;
    uint16_t time_to_full_min;
    bool     is_charging;
} bq27220_data_t;
typedef struct bq27220_dev_s *bq27220_handle_t;
esp_err_t bq27220_init(i2c_master_bus_handle_t i2c_bus, bq27220_handle_t *out_handle);
void bq27220_deinit(bq27220_handle_t handle);
esp_err_t bq27220_read(bq27220_handle_t handle, bq27220_data_t *out_data);
#ifdef __cplusplus
}
#endif
#endif
