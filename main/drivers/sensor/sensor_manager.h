/**
 * @file sensor_manager.h
 * @brief Environmental, Gas, Flow, Space, and Identity Sensor Subsystem Manager
 */

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

// Legacy structure for backward compatibility
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

/**
 * @brief Initialize all configured sensor peripherals
 */
esp_err_t sensor_manager_init(void);

/**
 * @brief Read snapshot of environmental sensors (AHT20, SHT3x, BMP280, BH1750, SCD40)
 */
esp_err_t sensor_read_environment(sensor_environment_t *out_env);

/**
 * @brief Read snapshot of distance sensors (VL53L0X, HC-SR04)
 */
esp_err_t sensor_read_distance(sensor_distance_t *out_dist);

/**
 * @brief Read snapshot of security & safety sensors (PIR, SW-420, Flame, MQ-2 Gas)
 */
esp_err_t sensor_read_security(sensor_security_t *out_sec);

/**
 * @brief Read snapshot of power & battery status (ADC / INA219 / TP4056)
 */
esp_err_t sensor_read_power(sensor_power_t *out_pwr);

/**
 * @brief Read current snapshot from active sensors (legacy API)
 */
esp_err_t sensor_manager_read_all(sensor_data_t *out_data);

/**
 * @brief Configure or dynamically update the DHT11/22 GPIO pin
 * @param pin User-selected GPIO pin number (or GPIO_NUM_NC to disable)
 */
void sensor_set_dht_pin(gpio_num_t pin);

/**
 * @brief Get current DHT11/22 GPIO pin
 */
gpio_num_t sensor_get_dht_pin(void);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_MANAGER_H
