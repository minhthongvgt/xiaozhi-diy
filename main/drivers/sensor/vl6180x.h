/**
 * @file vl6180x.h
 * @brief STMicroelectronics VL6180 / VL6180X Time-of-Flight (ToF) & Ambient Light Sensor (ALS) Driver
 * @version 1.0
 * @date 2026-09-16
 * 
 * Standardized driver for ESP-IDF 6.1 using modern driver/i2c_master.h architecture.
 * Implements ST AN4545 initialization sequence, sub-millimeter ranging, and calibrated ALS lux calculation.
 */

#ifndef VL6180X_H
#define VL6180X_H

#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VL6180X_DEFAULT_I2C_ADDR       0x29
#define VL6180X_EXPECTED_MODEL_ID      0xB4

/**
 * @brief VL6180X 16-bit Register Addresses (ST DT0037 & AN4545)
 */
#define VL6180X_REG_IDENTIFICATION_MODEL_ID           0x0000
#define VL6180X_REG_IDENTIFICATION_MODEL_REV_MAJOR    0x0001
#define VL6180X_REG_IDENTIFICATION_MODEL_REV_MINOR    0x0002
#define VL6180X_REG_IDENTIFICATION_MODULE_REV_MAJOR   0x0003
#define VL6180X_REG_IDENTIFICATION_MODULE_REV_MINOR   0x0004
#define VL6180X_REG_IDENTIFICATION_DATE_HI            0x0006
#define VL6180X_REG_IDENTIFICATION_DATE_LO            0x0007
#define VL6180X_REG_IDENTIFICATION_TIME               0x0008

#define VL6180X_REG_SYSTEM_MODE_GPIO0                 0x0010
#define VL6180X_REG_SYSTEM_MODE_GPIO1                 0x0011
#define VL6180X_REG_SYSTEM_HISTORY_CTRL               0x0012
#define VL6180X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO      0x0014
#define VL6180X_REG_SYSTEM_INTERRUPT_CLEAR            0x0015
#define VL6180X_REG_SYSTEM_FRESH_OUT_OF_RESET         0x0016
#define VL6180X_REG_SYSTEM_GROUPED_PARAMETER_HOLD     0x0017

#define VL6180X_REG_SYSRANGE_START                    0x0018
#define VL6180X_REG_SYSRANGE_THRESH_HIGH              0x0019
#define VL6180X_REG_SYSRANGE_THRESH_LOW               0x001A
#define VL6180X_REG_SYSRANGE_INTERMEASUREMENT_PERIOD  0x001B
#define VL6180X_REG_SYSRANGE_MAX_CONVERGENCE_TIME     0x001C
#define VL6180X_REG_SYSRANGE_CROSSTALK_COMPENSATION_RATE 0x001E
#define VL6180X_REG_SYSRANGE_CROSSTALK_VALID_HEIGHT   0x0021
#define VL6180X_REG_SYSRANGE_EARLY_CONVERGENCE_ESTIMATE 0x0022
#define VL6180X_REG_SYSRANGE_PART_TO_PART_RANGE_OFFSET 0x0024
#define VL6180X_REG_SYSRANGE_RANGE_IGNORE_VALID_HEIGHT 0x0025
#define VL6180X_REG_SYSRANGE_RANGE_IGNORE_THRESHOLD   0x0026
#define VL6180X_REG_SYSRANGE_MAX_AMBIENT_LEVEL_MULT   0x002C
#define VL6180X_REG_SYSRANGE_RANGE_CHECK_ENABLES      0x002D
#define VL6180X_REG_SYSRANGE_VHV_RECALIBRATE          0x002E
#define VL6180X_REG_SYSRANGE_VHV_REPEAT_RATE          0x0031

#define VL6180X_REG_SYSALS_START                      0x0038
#define VL6180X_REG_SYSALS_THRESH_HIGH                0x003A
#define VL6180X_REG_SYSALS_THRESH_LOW                 0x003C
#define VL6180X_REG_SYSALS_INTERMEASUREMENT_PERIOD    0x003E
#define VL6180X_REG_SYSALS_ANALOGUE_GAIN              0x003F
#define VL6180X_REG_SYSALS_INTEGRATION_PERIOD         0x0040

#define VL6180X_REG_RESULT_RANGE_STATUS               0x004D
#define VL6180X_REG_RESULT_ALS_STATUS                 0x004E
#define VL6180X_REG_RESULT_INTERRUPT_STATUS_GPIO      0x004F
#define VL6180X_REG_RESULT_ALS_VAL                    0x0050
#define VL6180X_REG_RESULT_RANGE_VAL                  0x0062
#define VL6180X_REG_RESULT_RANGE_RAW                  0x0064
#define VL6180X_REG_RESULT_RANGE_RETURN_RATE          0x0066
#define VL6180X_REG_RESULT_RANGE_REFERENCE_RATE       0x0068
#define VL6180X_REG_RESULT_RANGE_RETURN_SIGNAL_COUNT  0x006C
#define VL6180X_REG_RESULT_RANGE_REFERENCE_SIGNAL_COUNT 0x0070
#define VL6180X_REG_RESULT_RANGE_RETURN_AMB_COUNT     0x0074
#define VL6180X_REG_RESULT_RANGE_REFERENCE_AMB_COUNT  0x0078
#define VL6180X_REG_RESULT_RANGE_RETURN_CONV_TIME     0x007C
#define VL6180X_REG_RESULT_RANGE_REFERENCE_CONV_TIME  0x0080

#define VL6180X_REG_READOUT_AVERAGING_SAMPLE_PERIOD   0x010A
#define VL6180X_REG_FIRMWARE_BOOTUP                   0x0119
#define VL6180X_REG_I2C_SLAVE_DEVICE_ADDRESS          0x0212

/**
 * @brief Range measurement error status codes (bits [7:4] of RESULT_RANGE_STATUS)
 */
typedef enum {
    VL6180X_ERROR_NONE               = 0,  /*!< Valid measurement */
    VL6180X_ERROR_SYSERR_1           = 1,  /*!< System error VCSEL continuity test 1 */
    VL6180X_ERROR_SYSERR_2           = 2,  /*!< System error VCSEL continuity test 2 */
    VL6180X_ERROR_SYSERR_3           = 3,  /*!< System error VHV value out of range */
    VL6180X_ERROR_SYSERR_4           = 4,  /*!< System error VHV recalibration failed */
    VL6180X_ERROR_SYSERR_5           = 5,  /*!< System error SNR out of range */
    VL6180X_ERROR_ECEFAIL            = 6,  /*!< Early convergence estimate check failed */
    VL6180X_ERROR_NOCONVERGENCE      = 7,  /*!< Max convergence time reached without lock */
    VL6180X_ERROR_RANGEIGNORE        = 8,  /*!< Range ignore check failed */
    VL6180X_ERROR_SNR                = 11, /*!< Ambient light condition too high / low SNR */
    VL6180X_ERROR_RAWUFL             = 12, /*!< Raw range underflow */
    VL6180X_ERROR_RAWOFL             = 13, /*!< Raw range overflow */
    VL6180X_ERROR_RANGEUFL           = 14, /*!< Filtered range underflow */
    VL6180X_ERROR_RANGEOFL           = 15  /*!< Filtered range overflow */
} vl6180x_range_error_t;

/**
 * @brief ALS Analog Gain Settings
 */
typedef enum {
    VL6180X_ALS_GAIN_20   = 0, /*!< Gain 20 */
    VL6180X_ALS_GAIN_10   = 1, /*!< Gain 10 */
    VL6180X_ALS_GAIN_5    = 2, /*!< Gain 5 */
    VL6180X_ALS_GAIN_2_5  = 3, /*!< Gain 2.5 */
    VL6180X_ALS_GAIN_1_67 = 4, /*!< Gain 1.67 */
    VL6180X_ALS_GAIN_1_25 = 5, /*!< Gain 1.25 */
    VL6180X_ALS_GAIN_1    = 6, /*!< Gain 1.0 (Standard default) */
    VL6180X_ALS_GAIN_40   = 7  /*!< Gain 40 */
} vl6180x_als_gain_t;

/**
 * @brief Configuration structure for VL6180X device
 */
typedef struct {
    i2c_master_bus_handle_t i2c_bus;    /*!< ESP-IDF 6.1 I2C master bus handle */
    uint8_t                 i2c_addr;   /*!< I2C device address (default: 0x29) */
    gpio_num_t              xshut_pin;  /*!< Power/shutdown control pin (GPIO_NUM_NC if unused) */
    gpio_num_t              gpio1_pin;  /*!< Interrupt/data-ready pin (GPIO_NUM_NC if unused) */
    uint8_t                 scaling;    /*!< Range scaling factor: 1 (0-100mm), 2 (0-200mm), 3 (0-300mm) */
} vl6180x_config_t;

typedef struct vl6180x_dev_s *vl6180x_handle_t;

/**
 * @brief Initialize VL6180X sensor with modern ESP-IDF 6.1 I2C master driver
 * 
 * Configures XSHUT (if provided), verifies Model ID (0xB4), applies ST AN4545
 * mandatory tuning registers, performs VHV calibration, and prepares sensor.
 * 
 * @param config Pointer to configuration struct
 * @param[out] out_handle Pointer to allocated device handle
 * @return esp_err_t ESP_OK on success, or appropriate error code
 */
esp_err_t vl6180x_init(const vl6180x_config_t *config, vl6180x_handle_t *out_handle);

/**
 * @brief De-initialize VL6180X and release resources
 * 
 * @param handle Device handle
 * @return esp_err_t ESP_OK on success
 */
esp_err_t vl6180x_deinit(vl6180x_handle_t handle);

/**
 * @brief Perform single-shot range measurement
 * 
 * @param handle Device handle
 * @param[out] out_distance_mm Measured distance in millimeters (0 to ~400mm depending on scaling)
 * @return esp_err_t ESP_OK on success, ESP_ERR_TIMEOUT if sensor hung, ESP_FAIL if range invalid
 */
esp_err_t vl6180x_read_distance_mm(vl6180x_handle_t handle, float *out_distance_mm);

/**
 * @brief Perform single-shot Ambient Light (ALS) measurement in Lux
 * 
 * Reads raw ALS count and converts to calibrated Lux according to current Gain & Integration time.
 * 
 * @param handle Device handle
 * @param[out] out_lux Measured ambient illuminance in Lux (0.1 to 100,000 Lux)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t vl6180x_read_ambient_lux(vl6180x_handle_t handle, float *out_lux);

/**
 * @brief Configure ALS Gain
 * 
 * @param handle Device handle
 * @param gain Gain setting (default VL6180X_ALS_GAIN_1)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t vl6180x_set_als_gain(vl6180x_handle_t handle, vl6180x_als_gain_t gain);

/**
 * @brief Change I2C address dynamically (stored until power-cycle or reset)
 * 
 * @param handle Device handle
 * @param new_addr 7-bit new address (e.g. 0x2A)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t vl6180x_set_i2c_address(vl6180x_handle_t handle, uint8_t new_addr);

/**
 * @brief Set distance range scaling factor (1x, 2x, or 3x)
 * 
 * @param handle Device handle
 * @param scaling 1, 2, or 3
 * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG if unsupported
 */
esp_err_t vl6180x_set_scaling(vl6180x_handle_t handle, uint8_t scaling);

#ifdef __cplusplus
}
#endif

#endif // VL6180X_H
