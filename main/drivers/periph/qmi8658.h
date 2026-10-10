#ifndef QMI8658_H
#define QMI8658_H
#include <esp_err.h>
#include <driver/i2c_master.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define QMI8658_I2C_ADDR_PRIMARY   0x6B
#define QMI8658_I2C_ADDR_SECONDARY 0x6A
typedef struct qmi8658_sample_t {
    float accel_g[3];
    float gyro_dps[3];
    float temperature_c;
    float pitch_deg;
    float roll_deg;
} qmi8658_sample_t;
typedef struct qmi8658_dev_s *qmi8658_handle_t;
esp_err_t qmi8658_init(i2c_master_bus_handle_t i2c_bus, qmi8658_handle_t *out_handle);
void qmi8658_deinit(qmi8658_handle_t handle);
esp_err_t qmi8658_read(qmi8658_handle_t handle, qmi8658_sample_t *out_sample);
#ifdef __cplusplus
}
#endif
#endif
