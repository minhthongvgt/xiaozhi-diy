#ifndef VL53LX_H
#define VL53LX_H
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define VL53LX_DEFAULT_I2C_ADDR 0x29
typedef enum vl53lx_type_t {
    VL53LX_TYPE_UNKNOWN = 0,
    VL53LX_TYPE_VL53L0X = 1,
    VL53LX_TYPE_VL53L1X = 2,
} vl53lx_type_t;
typedef struct vl53lx_config_t {
    i2c_master_bus_handle_t i2c_bus;
    uint8_t                 i2c_addr;
    gpio_num_t              xshut_pin;
} vl53lx_config_t;
typedef struct vl53lx_dev_s *vl53lx_handle_t;
esp_err_t vl53lx_init(const vl53lx_config_t *config, vl53lx_handle_t *out_handle);
esp_err_t vl53lx_deinit(vl53lx_handle_t handle);
esp_err_t vl53lx_read_distance_mm(vl53lx_handle_t handle, float *out_distance_mm);
vl53lx_type_t vl53lx_get_type(vl53lx_handle_t handle);
const char *vl53lx_type_name(vl53lx_type_t type);
#ifdef __cplusplus
}
#endif
#endif
