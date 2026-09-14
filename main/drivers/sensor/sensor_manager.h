/**
 * @file sensor_manager.h
 * @brief Environmental, Gas, Flow, Space, and Identity Sensor Subsystem Manager
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <esp_err.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

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
 * @brief Read current snapshot from active sensors
 */
esp_err_t sensor_manager_read_all(sensor_data_t *out_data);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_MANAGER_H
