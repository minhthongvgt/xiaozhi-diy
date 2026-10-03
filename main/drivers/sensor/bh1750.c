#include "bh1750.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdlib.h>
#define TAG "Bh1750"
#define BH1750_CMD_POWER_ON           0x01
#define BH1750_CMD_RESET              0x07
#define BH1750_CMD_CONTINUOUS_H_RES   0x10
typedef struct bh1750_dev_t {
    i2c_master_dev_handle_t i2c_dev;
} bh1750_dev_t;
esp_err_t bh1750_init(i2c_master_bus_handle_t i2c_bus, bh1750_handle_t *out_handle)
{
    if (!i2c_bus || !out_handle) return ESP_ERR_INVALID_ARG;
    *out_handle = NULL;
    const uint8_t addrs[2] = { BH1750_I2C_ADDR_PRIMARY, BH1750_I2C_ADDR_SECONDARY };
    i2c_master_dev_handle_t i2c_dev = NULL;
    uint8_t found_addr = 0;
    for (int i = 0; i < 2; i++) {
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = addrs[i],
            .scl_speed_hz = 400000,
        };
        if (i2c_master_bus_add_device(i2c_bus, &dev_cfg, &i2c_dev) == ESP_OK) {
            uint8_t pwr_on = BH1750_CMD_POWER_ON;
            if (i2c_master_transmit(i2c_dev, &pwr_on, 1, 50) == ESP_OK) {
                found_addr = addrs[i];
                break;
            }
            i2c_master_bus_rm_device(i2c_dev);
            i2c_dev = NULL;
        }
    }
    if (!i2c_dev) {
        ESP_LOGD(TAG, "BH1750 not detected on I2C bus (checked 0x23, 0x5C)");
        return ESP_ERR_NOT_FOUND;
    }
    uint8_t mode_cmd = BH1750_CMD_CONTINUOUS_H_RES;
    esp_err_t err = i2c_master_transmit(i2c_dev, &mode_cmd, 1, 100);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set BH1750 measurement mode: %s", esp_err_to_name(err));
        i2c_master_bus_rm_device(i2c_dev);
        return err;
    }
    struct bh1750_dev_t *dev = (struct bh1750_dev_t *)calloc(1, sizeof(struct bh1750_dev_t));
    if (!dev) {
        i2c_master_bus_rm_device(i2c_dev);
        return ESP_ERR_NO_MEM;
    }
    dev->i2c_dev = i2c_dev;
    *out_handle = dev;
    ESP_LOGI(TAG, "BH1750 Ambient Light Sensor initialized on I2C address 0x%02X", found_addr);
    return ESP_OK;
}
esp_err_t bh1750_read_lux(bh1750_handle_t dev, float *out_lux)
{
    if (!dev || !out_lux) return ESP_ERR_INVALID_ARG;
    *out_lux = 0.0f;
    uint8_t data[2] = {0};
    esp_err_t err = i2c_master_receive(dev->i2c_dev, data, 2, 100);
    if (err != ESP_OK) {
        return err;
    }
    uint16_t raw = ((uint16_t)data[0] << 8) | data[1];
    *out_lux = (float)raw / 1.2f;
    return ESP_OK;
}
void bh1750_deinit(bh1750_handle_t handle)
{
    if (!handle) return;
    if (handle->i2c_dev) {
        i2c_master_bus_rm_device(handle->i2c_dev);
    }
    free(handle);
}
