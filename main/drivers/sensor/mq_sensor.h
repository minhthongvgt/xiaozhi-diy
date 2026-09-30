/**
 * @file mq_sensor.h
 * @brief High-precision Driver for Analog Gas Sensors (MQ-2 / MQ-135)
 * Standardized for ESP-IDF 6.1 with Strict Physical Modeling (No Simulated Values)
 */

#ifndef MQ_SENSOR_H
#define MQ_SENSOR_H

#include <esp_err.h>
#include <esp_adc/adc_oneshot.h>
#include <hal/gpio_types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Supported MQ Gas Sensor Models
 */
typedef enum {
    MQ_TYPE_MQ2 = 0,    /*!< MQ-2: Combustible Gas, LPG, Methane, Smoke */
    MQ_TYPE_MQ135 = 1,  /*!< MQ-135: Air Quality, Ammonia, Benzene, Alcohol, Smoke */
} mq_sensor_type_t;

/**
 * @brief MQ Sensor Configuration Struct
 */
typedef struct {
    gpio_num_t       pin;        /*!< Analog pin (GPIO 1 to 10 for ADC1 on ESP32-S3) */
    mq_sensor_type_t type;       /*!< Sensor model (MQ-2 or MQ-135) */
    float            rl_kohm;    /*!< Onboard load resistor RL (typically 10.0 kOhm) */
    float            r0_kohm;    /*!< Sensor resistance R0 in clean air (typically 10.0 kOhm) */
} mq_sensor_config_t;

/**
 * @brief Opaque handle to MQ sensor instance
 */
typedef struct mq_sensor_dev_t* mq_sensor_handle_t;

/**
 * @brief Initialize MQ Gas Sensor
 * @param adc1_handle Handle to existing ADC1 oneshot unit (shared across sensors)
 * @param config Pointer to sensor configuration
 * @param[out] out_handle Returned sensor handle
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if pin is invalid, or error code
 */
esp_err_t mq_sensor_init(adc_oneshot_unit_handle_t adc1_handle,
                         const mq_sensor_config_t *config,
                         mq_sensor_handle_t *out_handle);

/**
 * @brief Read REAL physical gas concentration and voltage
 * Checks physical presence (voltage window) to eliminate floating noise / simulated data.
 *
 * @param handle Sensor handle
 * @param[out] out_ppm Calculated gas concentration in PPM (power-law curve from datasheet)
 * @param[out] out_voltage_mv Measured analog voltage in millivolts (0 - 3300 mV)
 * @return ESP_OK on valid real reading,
 *         ESP_ERR_NOT_FOUND if sensor is disconnected or out of physical voltage range
 */
esp_err_t mq_sensor_read(mq_sensor_handle_t handle, float *out_ppm, float *out_voltage_mv);

/**
 * @brief Check if gas concentration exceeds safety alert threshold
 * @param handle Sensor handle
 * @param current_ppm Real PPM reading obtained from mq_sensor_read()
 * @return true if dangerous gas level detected, false otherwise
 */
bool mq_sensor_is_leak_alert(mq_sensor_handle_t handle, float current_ppm);

/**
 * @brief Calibrate clean air baseline resistance (R0)
 * Reads multiple samples in clean air environment to compute actual R0.
 *
 * @param handle Sensor handle
 * @param[out] out_r0 Calculated R0 value in kOhms
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if sensor is disconnected
 */
esp_err_t mq_sensor_calibrate(mq_sensor_handle_t handle, float *out_r0);

/**
 * @brief Deinitialize MQ Gas Sensor and release resources
 * @param handle Sensor handle
 */
void mq_sensor_deinit(mq_sensor_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // MQ_SENSOR_H
