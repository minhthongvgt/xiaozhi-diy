#include "scd4x.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdlib.h>
#include <string.h>
#define TAG "Scd4x"
#define SCD4X_CMD_START_PERIODIC_MEASUREMENT  0x21B1
#define SCD4X_CMD_READ_MEASUREMENT            0xEC05
#define SCD4X_CMD_STOP_PERIODIC_MEASUREMENT   0x3F86
#define SCD4X_CMD_GET_DATA_READY_STATUS       0xE4B8
typedef struct scd4x_dev_t {
    i2c_master_dev_handle_t i2c_dev;
    bool is_measuring;
} scd4x_dev_t;
static uint8_t sensirion_crc8(const uint8_t *data, uint16_t count)
{
    uint8_t crc = 0xFF;
    for (uint16_t i = 0; i < count; i++) {
        crc ^= data[i];
        for (uint8_t bit = 8; bit > 0; --bit) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}
static esp_err_t scd4x_send_cmd(scd4x_handle_t dev, uint16_t cmd)
{
    if (!dev || !dev->i2c_dev) return ESP_ERR_INVALID_ARG;
    uint8_t buf[2] = {
        (uint8_t)((cmd >> 8) & 0xFF),
        (uint8_t)(cmd & 0xFF)
    };
    return i2c_master_transmit(dev->i2c_dev, buf, 2, 100);
}
esp_err_t scd4x_init(i2c_master_bus_handle_t i2c_bus, scd4x_handle_t *out_handle)
{
    if (!i2c_bus || !out_handle) {
        return ESP_ERR_INVALID_ARG;
    }
    *out_handle = NULL;
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SCD4X_I2C_ADDR,
        .scl_speed_hz = 100000, 
    };
    i2c_master_dev_handle_t i2c_dev = NULL;
    esp_err_t err = i2c_master_bus_add_device(i2c_bus, &dev_cfg, &i2c_dev);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "Failed to register SCD4x device on I2C bus: %s", esp_err_to_name(err));
        return err;
    }
    struct scd4x_dev_t *dev = (struct scd4x_dev_t *)calloc(1, sizeof(struct scd4x_dev_t));
    if (!dev) {
        i2c_master_bus_rm_device(i2c_dev);
        return ESP_ERR_NO_MEM;
    }
    dev->i2c_dev = i2c_dev;
    err = scd4x_send_cmd(dev, SCD4X_CMD_STOP_PERIODIC_MEASUREMENT);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "SCD4x not detected at address 0x%02X: %s", SCD4X_I2C_ADDR, esp_err_to_name(err));
        i2c_master_bus_rm_device(i2c_dev);
        free(dev);
        return ESP_ERR_NOT_FOUND;
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    err = scd4x_send_cmd(dev, SCD4X_CMD_START_PERIODIC_MEASUREMENT);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to start SCD4x periodic measurement: %s", esp_err_to_name(err));
        i2c_master_bus_rm_device(i2c_dev);
        free(dev);
        return err;
    }
    dev->is_measuring = true;
    *out_handle = dev;
    ESP_LOGI(TAG, "Sensirion SCD40/41 CO2 Sensor detected & initialized at address 0x%02X", SCD4X_I2C_ADDR);
    return ESP_OK;
}
esp_err_t scd4x_read_measurement(scd4x_handle_t handle, float *out_co2_ppm, float *out_temp_c, float *out_hum_pct)
{
    if (!handle || !out_co2_ppm || !out_temp_c || !out_hum_pct) {
        return ESP_ERR_INVALID_ARG;
    }
    *out_co2_ppm = 0.0f;
    *out_temp_c = 0.0f;
    *out_hum_pct = 0.0f;
    uint8_t cmd_buf[2] = {
        (uint8_t)((SCD4X_CMD_READ_MEASUREMENT >> 8) & 0xFF),
        (uint8_t)(SCD4X_CMD_READ_MEASUREMENT & 0xFF)
    };
    uint8_t rx_data[9] = {0};
    esp_err_t err = i2c_master_transmit_receive(handle->i2c_dev, cmd_buf, 2, rx_data, 9, 200);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "SCD4x read measurement transmit/receive failed: %s", esp_err_to_name(err));
        return err;
    }
    if (sensirion_crc8(&rx_data[0], 2) != rx_data[2]) {
        ESP_LOGW(TAG, "SCD4x CO2 CRC mismatch");
        return ESP_FAIL;
    }
    if (sensirion_crc8(&rx_data[3], 2) != rx_data[5]) {
        ESP_LOGW(TAG, "SCD4x Temp CRC mismatch");
        return ESP_FAIL;
    }
    if (sensirion_crc8(&rx_data[6], 2) != rx_data[8]) {
        ESP_LOGW(TAG, "SCD4x Hum CRC mismatch");
        return ESP_FAIL;
    }
    uint16_t co2_raw = ((uint16_t)rx_data[0] << 8) | rx_data[1];
    uint16_t temp_raw = ((uint16_t)rx_data[3] << 8) | rx_data[4];
    uint16_t hum_raw = ((uint16_t)rx_data[6] << 8) | rx_data[7];
    if (co2_raw == 0) {
        return ESP_ERR_INVALID_STATE;
    }
    *out_co2_ppm = (float)co2_raw;
    *out_temp_c = -45.0f + 175.0f * ((float)temp_raw / 65536.0f);
    *out_hum_pct = 100.0f * ((float)hum_raw / 65536.0f);
    return ESP_OK;
}
void scd4x_deinit(scd4x_handle_t handle)
{
    if (!handle) return;
    if (handle->is_measuring) {
        scd4x_send_cmd(handle, SCD4X_CMD_STOP_PERIODIC_MEASUREMENT);
    }
    if (handle->i2c_dev) {
        i2c_master_bus_rm_device(handle->i2c_dev);
    }
    free(handle);
}
