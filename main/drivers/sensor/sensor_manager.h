#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H
#include <esp_err.h>
#include <stdbool.h>
#include <stdint.h>
#include <driver/gpio.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    float temperature_c;
    float humidity_pct;
    float pressure_hpa;
    float light_lux;
    float co2_ppm;
    float tvoc_ppb;
    bool valid;
} sensor_environment_t;
typedef struct {
    float distance_laser_mm;
    float distance_ultrasonic_cm;
    bool valid;
} sensor_distance_t;
typedef struct {
    bool motion_detected;
    bool vibration_detected;
    bool flame_detected;
    float gas_level_ppm;
    bool gas_leak_alert;
    bool valid;
} sensor_security_t;
typedef struct {
    float battery_voltage;
    int battery_percentage;
    bool is_charging;
    float bus_current_ma;
    float bus_power_mw;
    bool valid;
} sensor_power_t;
typedef struct {
    float temperature_c;
    float humidity_pct;
    float light_lux;
    float co2_ppm;
    float battery_voltage;
    bool motion_detected;
    bool vibration_detected;
    bool flame_detected;
} sensor_data_t;
esp_err_t sensor_manager_init(void);
esp_err_t sensor_read_environment(sensor_environment_t *out_env);
esp_err_t sensor_read_distance(sensor_distance_t *out_dist);
esp_err_t sensor_read_security(sensor_security_t *out_sec);
esp_err_t sensor_read_power(sensor_power_t *out_pwr);
esp_err_t sensor_manager_read_all(sensor_data_t *out_data);
void sensor_set_dht_pin(gpio_num_t pin);
gpio_num_t sensor_get_dht_pin(void);
void sensor_set_mq_pin(gpio_num_t pin);
gpio_num_t sensor_get_mq_pin(void);
void sensor_set_hcsr04_pins(gpio_num_t trig_pin, gpio_num_t echo_pin);
#ifdef __cplusplus
}
#endif
#endif 
