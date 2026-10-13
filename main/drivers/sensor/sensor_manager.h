#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H
#include <esp_err.h>
#include <stdbool.h>
#include <stdint.h>
#include <driver/gpio.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct sensor_environment_t {
    float temperature_c;
    float humidity_pct;
    float pressure_hpa;
    float light_lux;
    float co2_ppm;
    bool has_temperature;
    bool has_humidity;
    bool has_pressure;
    bool has_light;
    bool has_co2;
    bool valid;
} sensor_environment_t;
typedef struct sensor_distance_t {
    float distance_laser_mm;
    float distance_ultrasonic_cm;
    bool has_laser;
    bool has_ultrasonic;
    bool valid;
} sensor_distance_t;
typedef struct sensor_security_t {
    bool motion_detected;
    bool vibration_detected;
    bool flame_detected;
    float gas_level_ppm;
    bool gas_leak_alert;
    bool has_motion;
    bool has_vibration;
    bool has_flame;
    bool has_gas;
    bool valid;
} sensor_security_t;
typedef struct sensor_power_t {
    float battery_voltage;
    int battery_percentage;
    bool is_charging;
    float bus_current_ma;
    float bus_power_mw;
    bool has_voltage;
    bool has_percentage;
    bool has_charging;
    bool has_current;
    bool valid;
} sensor_power_t;
typedef struct sensor_color_t {
    uint8_t proximity;
    uint16_t clear;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    float light_lux;
    bool valid;
} sensor_color_t;
esp_err_t sensor_manager_init(void);
esp_err_t sensor_read_environment(sensor_environment_t *out_env);
esp_err_t sensor_read_distance(sensor_distance_t *out_dist);
esp_err_t sensor_read_security(sensor_security_t *out_sec);
esp_err_t sensor_read_power(sensor_power_t *out_pwr);
esp_err_t sensor_read_proximity_color(sensor_color_t *out_color);
esp_err_t sensor_read_gesture(uint32_t timeout_ms, const char **out_gesture);
bool sensor_environment_available(void);
bool sensor_distance_available(void);
bool sensor_security_available(void);
bool sensor_power_available(void);
bool sensor_color_available(void);
void sensor_set_dht_pin(gpio_num_t pin);
gpio_num_t sensor_get_dht_pin(void);
void sensor_set_mq_pin(gpio_num_t pin);
gpio_num_t sensor_get_mq_pin(void);
void sensor_set_hcsr04_pins(gpio_num_t trig_pin, gpio_num_t echo_pin);
#ifdef __cplusplus
}
#endif
#endif
