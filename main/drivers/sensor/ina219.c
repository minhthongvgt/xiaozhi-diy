#include "ina219.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdlib.h>
#define TAG "Ina219"
#define INA219_REG_CONFIG       0x00
#define INA219_REG_SHUNTVOLTAGE 0x01
#define INA219_REG_BUSVOLTAGE   0x02
#define INA219_REG_POWER        0x03
#define INA219_REG_CURRENT      0x04
#define INA219_REG_CALIBRATION  0x05
#define INA219_CONFIG_DEFAULT   0x399F
#define INA219_CALIB_DEFAULT    4096 
typedef struct ina219_dev_t {
    i2c_master_dev_handle_t i2c_dev;
} ina219_dev_t;
static esp_err_t ina219_write_reg16(ina219_handle_t dev, uint8_t reg, uint16_t val)
{
    uint8_t buf[3] = {
        reg,
        (uint8_t)((val >> 8) & 0xFF),
        (uint8_t)(val & 0xFF)
    };
    return i2c_master_transmit(dev->i2c_dev, buf, 3, 100);
}
static esp_err_t ina219_read_reg16(ina219_handle_t dev, uint8_t reg, uint16_t *val)
{
    uint8_t rx[2] = {0};
    esp_err_t err = i2c_master_transmit_receive(dev->i2c_dev, &reg, 1, rx, 2, 100);
    if (err == ESP_OK) {
        *val = ((uint16_t)rx[0] << 8) | rx[1];
    }
    return err;
}
esp_err_t ina219_init(i2c_master_bus_handle_t i2c_bus, ina219_handle_t *out_handle)
{
    if (!i2c_bus || !out_handle) return ESP_ERR_INVALID_ARG;
    *out_handle = NULL;
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = INA219_I2C_ADDR_DEFAULT,
        .scl_speed_hz = 400000,
    };
    i2c_master_dev_handle_t i2c_dev = NULL;
    esp_err_t err = i2c_master_bus_add_device(i2c_bus, &dev_cfg, &i2c_dev);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "Failed to register INA219 device on I2C bus: %s", esp_err_to_name(err));
        return err;
    }
    struct ina219_dev_t *dev = (struct ina219_dev_t *)calloc(1, sizeof(struct ina219_dev_t));
    if (!dev) {
        i2c_master_bus_rm_device(i2c_dev);
        return ESP_ERR_NO_MEM;
    }
    dev->i2c_dev = i2c_dev;
    err = ina219_write_reg16(dev, INA219_REG_CONFIG, INA219_CONFIG_DEFAULT);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "INA219 not detected at address 0x%02X: %s", INA219_I2C_ADDR_DEFAULT, esp_err_to_name(err));
        i2c_master_bus_rm_device(i2c_dev);
        free(dev);
        return ESP_ERR_NOT_FOUND;
    }
    ina219_write_reg16(dev, INA219_REG_CALIBRATION, INA219_CALIB_DEFAULT);
    *out_handle = dev;
    ESP_LOGI(TAG, "INA219 Current & Power Monitor initialized at address 0x%02X", INA219_I2C_ADDR_DEFAULT);
    return ESP_OK;
}
esp_err_t ina219_read(ina219_handle_t dev, float *out_bus_voltage_v, float *out_current_ma, float *out_power_mw)
{
    if (!dev || !out_bus_voltage_v || !out_current_ma || !out_power_mw) {
        return ESP_ERR_INVALID_ARG;
    }
    *out_bus_voltage_v = 0.0f;
    *out_current_ma = 0.0f;
    *out_power_mw = 0.0f;
    uint16_t raw_bus = 0;
    esp_err_t err = ina219_read_reg16(dev, INA219_REG_BUSVOLTAGE, &raw_bus);
    if (err != ESP_OK) {
        return err;
    }
    int16_t bus_mv = (int16_t)((raw_bus >> 3) * 4);
    *out_bus_voltage_v = (float)bus_mv / 1000.0f;
    uint16_t raw_current = 0;
    if (ina219_read_reg16(dev, INA219_REG_CURRENT, &raw_current) == ESP_OK) {
        int16_t cur = (int16_t)raw_current;
        *out_current_ma = (float)cur * 0.1f;
    }
    uint16_t raw_power = 0;
    if (ina219_read_reg16(dev, INA219_REG_POWER, &raw_power) == ESP_OK) {
        *out_power_mw = (float)raw_power * 2.0f;
    }
    return ESP_OK;
}
void ina219_deinit(ina219_handle_t handle)
{
    if (!handle) return;
    if (handle->i2c_dev) {
        i2c_master_bus_rm_device(handle->i2c_dev);
    }
    free(handle);
}
