#ifndef PCA9685_H
#define PCA9685_H
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define PCA9685_I2C_ADDR_DEFAULT 0x40
#define PCA9685_CHANNEL_COUNT    16
typedef struct pca9685_config_t {
    i2c_master_bus_handle_t i2c_bus;
    uint8_t                 i2c_addr;
    gpio_num_t              oe_pin;
    uint16_t                pwm_freq_hz;
} pca9685_config_t;
typedef struct pca9685_dev_s *pca9685_handle_t;
esp_err_t pca9685_init(const pca9685_config_t *config, pca9685_handle_t *out_handle);
void pca9685_deinit(pca9685_handle_t handle);
esp_err_t pca9685_set_frequency(pca9685_handle_t handle, uint16_t freq_hz);
uint16_t pca9685_get_frequency(pca9685_handle_t handle);
esp_err_t pca9685_set_pwm(pca9685_handle_t handle, uint8_t channel, uint16_t on_tick, uint16_t off_tick);
esp_err_t pca9685_set_duty(pca9685_handle_t handle, uint8_t channel, float duty_pct);
esp_err_t pca9685_set_pulse_us(pca9685_handle_t handle, uint8_t channel, uint32_t pulse_us);
esp_err_t pca9685_set_servo_angle(pca9685_handle_t handle, uint8_t channel, float angle_deg,
                                  uint32_t min_us, uint32_t max_us);
esp_err_t pca9685_set_output_enabled(pca9685_handle_t handle, bool enabled);
esp_err_t pca9685_all_off(pca9685_handle_t handle);
#ifdef __cplusplus
}
#endif
#endif
