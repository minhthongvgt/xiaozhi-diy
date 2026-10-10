#include "aht20.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdlib.h>
#include <string.h>
#define TAG "Aht20"
#define AHT20_CMD_STATUS_CHECK 0x71
#define AHT20_CMD_INIT         0xBE
#define AHT20_CMD_TRIGGER_MEAS 0xAC
typedef struct aht20_dev_t {
    i2c_master_dev_handle_t i2c_dev;
} aht20_dev_t;
esp_err_t aht20_init(i2c_master_bus_handle_t i2c_bus, aht20_handle_t *out_handle)
{
    if (!i2c_bus || !out_handle) return ESP_ERR_INVALID_ARG;
    *out_handle = NULL;
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = AHT20_I2C_ADDR,
        .scl_speed_hz = 400000,
    };
    i2c_master_dev_handle_t i2c_dev = NULL;
    esp_err_t err = i2c_master_bus_add_device(i2c_bus, &dev_cfg, &i2c_dev);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "Failed to register AHT20 device on I2C bus: %s", esp_err_to_name(err));
        return err;
    }
    uint8_t status = 0;
    uint8_t cmd_check = AHT20_CMD_STATUS_CHECK;
    err = i2c_master_transmit_receive(i2c_dev, &cmd_check, 1, &status, 1, 100);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "AHT20/21 not responding at address 0x%02X: %s", AHT20_I2C_ADDR, esp_err_to_name(err));
        i2c_master_bus_rm_device(i2c_dev);
        return ESP_ERR_NOT_FOUND;
    }
    if ((status & 0x08) == 0) {
        uint8_t init_cmd[3] = { AHT20_CMD_INIT, 0x08, 0x00 };
        i2c_master_transmit(i2c_dev, init_cmd, 3, 100);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    struct aht20_dev_t *dev = (struct aht20_dev_t *)calloc(1, sizeof(struct aht20_dev_t));
    if (!dev) {
        i2c_master_bus_rm_device(i2c_dev);
        return ESP_ERR_NO_MEM;
    }
    dev->i2c_dev = i2c_dev;
    *out_handle = dev;
    ESP_LOGI(TAG, "AHT20/21 Temperature & Humidity Sensor initialized at address 0x%02X (Status: 0x%02X)",
             AHT20_I2C_ADDR, status);
    return ESP_OK;
}
esp_err_t aht20_read(aht20_handle_t dev, float *out_temp_c, float *out_hum_pct)
{
    if (!dev || !out_temp_c || !out_hum_pct) return ESP_ERR_INVALID_ARG;
    *out_temp_c = 0.0f;
    *out_hum_pct = 0.0f;
    uint8_t trigger_cmd[3] = { AHT20_CMD_TRIGGER_MEAS, 0x33, 0x00 };
    esp_err_t err = i2c_master_transmit(dev->i2c_dev, trigger_cmd, 3, 100);
    if (err != ESP_OK) {
        return err;
    }
    vTaskDelay(pdMS_TO_TICKS(80));
    uint8_t rx[7] = {0};
    err = i2c_master_receive(dev->i2c_dev, rx, 7, 100);
    if (err != ESP_OK) {
        return err;
    }
    if (rx[0] & 0x80) {
        ESP_LOGD(TAG, "AHT20 still busy reading");
        return ESP_ERR_INVALID_STATE;
    }
    uint32_t raw_hum = ((uint32_t)rx[1] << 12) | ((uint32_t)rx[2] << 4) | ((uint32_t)rx[3] >> 4);
    uint32_t raw_temp = (((uint32_t)rx[3] & 0x0F) << 16) | ((uint32_t)rx[4] << 8) | (uint32_t)rx[5];
    *out_hum_pct = ((float)raw_hum * 100.0f) / 1048576.0f;
    *out_temp_c = (((float)raw_temp * 200.0f) / 1048576.0f) - 50.0f;
    if (*out_hum_pct < 0.0f) *out_hum_pct = 0.0f;
    if (*out_hum_pct > 100.0f) *out_hum_pct = 100.0f;
    return ESP_OK;
}
void aht20_deinit(aht20_handle_t handle)
{
    if (!handle) return;
    if (handle->i2c_dev) {
        i2c_master_bus_rm_device(handle->i2c_dev);
    }
    free(handle);
}
