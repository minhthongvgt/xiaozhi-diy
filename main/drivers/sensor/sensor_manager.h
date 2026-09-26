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
 *
 * @note ADC1 is initialized for analog sensors (Gas MQ-2, LDR).
 *       Digital sensors (DHT, PIR, Vibration, Flame) are GPIO-configured here.
 *
 * @note Precondition for VL6180X (ToF/ALS): The active board class MUST call
 *       bus_manager_init_i2c() or bus_manager_set_i2c_bus() BEFORE any call to
 *       sensor_read_environment() or sensor_read_distance().
 *       VL6180X uses lazy initialization via get_vl6180x_dev().
 */
esp_err_t sensor_manager_init(void);

/**
 * @brief Read snapshot of environmental sensors (AHT20, SHT3x, BMP280, BH1750, SCD40)
 */
esp_err_t sensor_read_environment(sensor_environment_t *out_env);

/**
 * @brief Read snapshot of distance sensors (VL6180X Laser ToF, HC-SR04)
 *
 * @note VL6180X is auto-initialized on first call if I2C bus is available.
 *       If I2C bus is not set, returns fallback baseline values.
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
