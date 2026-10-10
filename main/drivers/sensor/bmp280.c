#include "bmp280.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdlib.h>
#include <string.h>
#define TAG "Bmp280"
#define BMP280_REG_ID          0xD0
#define BMP280_REG_RESET       0xE0
#define BMP280_REG_STATUS      0xF3
#define BMP280_REG_CTRL_MEAS   0xF4
#define BMP280_REG_CONFIG      0xF5
#define BMP280_REG_PRESS_MSB   0xF7
#define BMP280_REG_CALIB_START 0x88
#define BMP280_CHIP_ID_BMP280  0x58
#define BMP280_CHIP_ID_BME280  0x60
typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} bmp280_calib_t;
typedef struct bmp280_dev_t {
    i2c_master_dev_handle_t i2c_dev;
    bmp280_calib_t          calib;
    int32_t                 t_fine;
} bmp280_dev_t;
static esp_err_t bmp280_read_regs(bmp280_handle_t dev, uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(dev->i2c_dev, &reg, 1, data, len, 100);
}
static esp_err_t bmp280_write_reg(bmp280_handle_t dev, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return i2c_master_transmit(dev->i2c_dev, buf, 2, 100);
}
esp_err_t bmp280_init(i2c_master_bus_handle_t i2c_bus, bmp280_handle_t *out_handle)
{
    if (!i2c_bus || !out_handle) return ESP_ERR_INVALID_ARG;
    *out_handle = NULL;
    const uint8_t addrs[2] = { BMP280_I2C_ADDR_PRIMARY, BMP280_I2C_ADDR_SECONDARY };
    i2c_master_dev_handle_t i2c_dev = NULL;
    uint8_t chip_id = 0;
    uint8_t found_addr = 0;
    for (int i = 0; i < 2; i++) {
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = addrs[i],
            .scl_speed_hz = 400000,
        };
        if (i2c_master_bus_add_device(i2c_bus, &dev_cfg, &i2c_dev) == ESP_OK) {
            uint8_t reg = BMP280_REG_ID;
            if (i2c_master_transmit_receive(i2c_dev, &reg, 1, &chip_id, 1, 50) == ESP_OK) {
                if (chip_id == BMP280_CHIP_ID_BMP280 || chip_id == BMP280_CHIP_ID_BME280) {
                    found_addr = addrs[i];
                    break;
                }
            }
            i2c_master_bus_rm_device(i2c_dev);
            i2c_dev = NULL;
        }
    }
    if (!i2c_dev) {
        ESP_LOGD(TAG, "BMP280/BME280 not detected on I2C bus (checked 0x76, 0x77)");
        return ESP_ERR_NOT_FOUND;
    }
    struct bmp280_dev_t *dev = (struct bmp280_dev_t *)calloc(1, sizeof(struct bmp280_dev_t));
    if (!dev) {
        i2c_master_bus_rm_device(i2c_dev);
        return ESP_ERR_NO_MEM;
    }
    dev->i2c_dev = i2c_dev;
    uint8_t calib_buf[24] = {0};
    esp_err_t err = bmp280_read_regs(dev, BMP280_REG_CALIB_START, calib_buf, 24);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read BMP280 calibration registers: %s", esp_err_to_name(err));
        i2c_master_bus_rm_device(i2c_dev);
        free(dev);
        return err;
    }
    dev->calib.dig_T1 = (uint16_t)(calib_buf[0] | (calib_buf[1] << 8));
    dev->calib.dig_T2 = (int16_t)(calib_buf[2] | (calib_buf[3] << 8));
    dev->calib.dig_T3 = (int16_t)(calib_buf[4] | (calib_buf[5] << 8));
    dev->calib.dig_P1 = (uint16_t)(calib_buf[6] | (calib_buf[7] << 8));
    dev->calib.dig_P2 = (int16_t)(calib_buf[8] | (calib_buf[9] << 8));
    dev->calib.dig_P3 = (int16_t)(calib_buf[10] | (calib_buf[11] << 8));
    dev->calib.dig_P4 = (int16_t)(calib_buf[12] | (calib_buf[13] << 8));
    dev->calib.dig_P5 = (int16_t)(calib_buf[14] | (calib_buf[15] << 8));
    dev->calib.dig_P6 = (int16_t)(calib_buf[16] | (calib_buf[17] << 8));
    dev->calib.dig_P7 = (int16_t)(calib_buf[18] | (calib_buf[19] << 8));
    dev->calib.dig_P8 = (int16_t)(calib_buf[20] | (calib_buf[21] << 8));
    dev->calib.dig_P9 = (int16_t)(calib_buf[22] | (calib_buf[23] << 8));
    bmp280_write_reg(dev, BMP280_REG_CONFIG, (0x00 << 5) | (0x04 << 2));
    bmp280_write_reg(dev, BMP280_REG_CTRL_MEAS, (0x02 << 5) | (0x05 << 2) | 0x03);
    *out_handle = dev;
    ESP_LOGI(TAG, "BMP280/BME280 (ID 0x%02X) detected & configured on I2C address 0x%02X", chip_id, found_addr);
    return ESP_OK;
}
esp_err_t bmp280_read(bmp280_handle_t dev, float *out_pressure_hpa, float *out_temperature_c)
{
    if (!dev || !out_pressure_hpa || !out_temperature_c) return ESP_ERR_INVALID_ARG;
    uint8_t data[6] = {0};
    esp_err_t err = bmp280_read_regs(dev, BMP280_REG_PRESS_MSB, data, 6);
    if (err != ESP_OK) {
        return err;
    }
    int32_t adc_p = ((int32_t)data[0] << 12) | ((int32_t)data[1] << 4) | ((int32_t)data[2] >> 4);
    int32_t adc_t = ((int32_t)data[3] << 12) | ((int32_t)data[4] << 4) | ((int32_t)data[5] >> 4);
    int32_t var1_t = ((((adc_t >> 3) - ((int32_t)dev->calib.dig_T1 << 1))) * ((int32_t)dev->calib.dig_T2)) >> 11;
    int32_t var2_t = (((((adc_t >> 4) - ((int32_t)dev->calib.dig_T1)) *
                        ((adc_t >> 4) - ((int32_t)dev->calib.dig_T1))) >> 12) *
                      ((int32_t)dev->calib.dig_T3)) >> 14;
    dev->t_fine = var1_t + var2_t;
    int32_t t = (dev->t_fine * 5 + 128) >> 8;
    *out_temperature_c = (float)t / 100.0f;
    int64_t var1_p = ((int64_t)dev->t_fine) - 128000;
    int64_t var2_p = var1_p * var1_p * (int64_t)dev->calib.dig_P6;
    var2_p = var2_p + ((var1_p * (int64_t)dev->calib.dig_P5) << 17);
    var2_p = var2_p + (((int64_t)dev->calib.dig_P4) << 35);
    var1_p = ((var1_p * var1_p * (int64_t)dev->calib.dig_P3) >> 8) +
             ((var1_p * (int64_t)dev->calib.dig_P2) << 12);
    var1_p = (((((int64_t)1) << 47) + var1_p)) * ((int64_t)dev->calib.dig_P1) >> 33;
    if (var1_p == 0) {
        *out_pressure_hpa = 0.0f;
        return ESP_FAIL;
    }
    int64_t p = 1048576 - adc_p;
    p = (((p << 31) - var2_p) * 3125) / var1_p;
    var1_p = (((int64_t)dev->calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2_p = (((int64_t)dev->calib.dig_P8) * p) >> 19;
    p = ((p + var1_p + var2_p) >> 8) + (((int64_t)dev->calib.dig_P7) << 4);
    float pressure_pa = (float)p / 256.0f;
    *out_pressure_hpa = pressure_pa / 100.0f; 
    return ESP_OK;
}
void bmp280_deinit(bmp280_handle_t handle)
{
    if (!handle) return;
    if (handle->i2c_dev) {
        i2c_master_bus_rm_device(handle->i2c_dev);
    }
    free(handle);
}
