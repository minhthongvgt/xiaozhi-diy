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
typedef enum {
    VL6180X_ERROR_NONE               = 0,  
    VL6180X_ERROR_SYSERR_1           = 1,  
    VL6180X_ERROR_SYSERR_2           = 2,  
    VL6180X_ERROR_SYSERR_3           = 3,  
    VL6180X_ERROR_SYSERR_4           = 4,  
    VL6180X_ERROR_SYSERR_5           = 5,  
    VL6180X_ERROR_ECEFAIL            = 6,  
    VL6180X_ERROR_NOCONVERGENCE      = 7,  
    VL6180X_ERROR_RANGEIGNORE        = 8,  
    VL6180X_ERROR_SNR                = 11, 
    VL6180X_ERROR_RAWUFL             = 12, 
    VL6180X_ERROR_RAWOFL             = 13, 
    VL6180X_ERROR_RANGEUFL           = 14, 
    VL6180X_ERROR_RANGEOFL           = 15  
} vl6180x_range_error_t;
typedef enum {
    VL6180X_ALS_GAIN_20   = 0, 
    VL6180X_ALS_GAIN_10   = 1, 
    VL6180X_ALS_GAIN_5    = 2, 
    VL6180X_ALS_GAIN_2_5  = 3, 
    VL6180X_ALS_GAIN_1_67 = 4, 
    VL6180X_ALS_GAIN_1_25 = 5, 
    VL6180X_ALS_GAIN_1    = 6, 
    VL6180X_ALS_GAIN_40   = 7  
} vl6180x_als_gain_t;
typedef struct {
    i2c_master_bus_handle_t i2c_bus;    
    uint8_t                 i2c_addr;   
    gpio_num_t              xshut_pin;  
    gpio_num_t              gpio1_pin;  
    uint8_t                 scaling;    
} vl6180x_config_t;
typedef struct vl6180x_dev_s *vl6180x_handle_t;
esp_err_t vl6180x_init(const vl6180x_config_t *config, vl6180x_handle_t *out_handle);
esp_err_t vl6180x_deinit(vl6180x_handle_t handle);
esp_err_t vl6180x_read_distance_mm(vl6180x_handle_t handle, float *out_distance_mm);
esp_err_t vl6180x_read_ambient_lux(vl6180x_handle_t handle, float *out_lux);
esp_err_t vl6180x_set_als_gain(vl6180x_handle_t handle, vl6180x_als_gain_t gain);
esp_err_t vl6180x_set_i2c_address(vl6180x_handle_t handle, uint8_t new_addr);
esp_err_t vl6180x_set_scaling(vl6180x_handle_t handle, uint8_t scaling);
#ifdef __cplusplus
}
#endif
#endif 
