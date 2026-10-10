#include "drivers/sensor/bq27220.h"
#include <esp_log.h>
#include <stdlib.h>
#define TAG "BQ27220"
#define I2C_TIMEOUT_MS 100
#define BQ27220_CMD_TEMPERATURE        0x06
#define BQ27220_CMD_VOLTAGE            0x08
#define BQ27220_CMD_BATTERY_STATUS     0x0A
#define BQ27220_CMD_CURRENT            0x0C
#define BQ27220_CMD_REMAINING_CAPACITY 0x10
#define BQ27220_CMD_FULL_CHARGE_CAP    0x12
#define BQ27220_CMD_AVERAGE_CURRENT    0x14
#define BQ27220_CMD_TIME_TO_EMPTY      0x16
#define BQ27220_CMD_TIME_TO_FULL       0x18
#define BQ27220_CMD_STATE_OF_CHARGE    0x2C
#define BQ27220_CMD_STATE_OF_HEALTH    0x2E
#define BQ27220_STATUS_DSG             0x0001
typedef struct bq27220_dev_s {
    i2c_master_dev_handle_t i2c_dev;
} bq27220_dev_s;
static esp_err_t read_word(bq27220_dev_s *dev, uint8_t cmd, uint16_t *out)
{
    uint8_t rx[2] = {0};
    esp_err_t ret = i2c_master_transmit_receive(dev->i2c_dev, &cmd, 1, rx, sizeof(rx), I2C_TIMEOUT_MS);
    if (ret == ESP_OK) {
        *out = (uint16_t)(rx[1] << 8 | rx[0]);
    }
    return ret;
}
esp_err_t bq27220_init(i2c_master_bus_handle_t i2c_bus, bq27220_handle_t *out_handle)
{
    if (!i2c_bus || !out_handle) return ESP_ERR_INVALID_ARG;
    *out_handle = NULL;
    if (i2c_master_probe(i2c_bus, BQ27220_I2C_ADDR, 50) != ESP_OK) {
        return ESP_ERR_NOT_FOUND;
    }
    bq27220_dev_s *dev = calloc(1, sizeof(bq27220_dev_s));
    if (!dev) return ESP_ERR_NO_MEM;
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BQ27220_I2C_ADDR,
        .scl_speed_hz = 100000,
    };
    esp_err_t ret = i2c_master_bus_add_device(i2c_bus, &dev_cfg, &dev->i2c_dev);
    if (ret != ESP_OK) {
        free(dev);
        return ret;
    }
    uint16_t mv = 0;
    ret = read_word(dev, BQ27220_CMD_VOLTAGE, &mv);
    if (ret != ESP_OK || mv < 2000 || mv > 5500) {
        ESP_LOGW(TAG, "Implausible battery voltage %u mV at 0x%02X (err: %s)", mv, BQ27220_I2C_ADDR,
                 esp_err_to_name(ret));
        bq27220_deinit(dev);
        return ESP_ERR_NOT_FOUND;
    }
    ESP_LOGI(TAG, "BQ27220 fuel gauge initialized (battery %u mV)", mv);
    *out_handle = dev;
    return ESP_OK;
}
void bq27220_deinit(bq27220_handle_t handle)
{
    if (!handle) return;
    if (handle->i2c_dev) i2c_master_bus_rm_device(handle->i2c_dev);
    free(handle);
}
esp_err_t bq27220_read(bq27220_handle_t handle, bq27220_data_t *out_data)
{
    if (!handle || !out_data) return ESP_ERR_INVALID_ARG;
    uint16_t voltage = 0, current = 0, avg_current = 0, temperature = 0, soc = 0, soh = 0;
    uint16_t remaining = 0, full = 0, tte = 0, ttf = 0, status = 0;
    esp_err_t ret = read_word(handle, BQ27220_CMD_VOLTAGE, &voltage);
    if (ret != ESP_OK) return ret;
    read_word(handle, BQ27220_CMD_CURRENT, &current);
    read_word(handle, BQ27220_CMD_AVERAGE_CURRENT, &avg_current);
    read_word(handle, BQ27220_CMD_TEMPERATURE, &temperature);
    read_word(handle, BQ27220_CMD_STATE_OF_CHARGE, &soc);
    read_word(handle, BQ27220_CMD_STATE_OF_HEALTH, &soh);
    read_word(handle, BQ27220_CMD_REMAINING_CAPACITY, &remaining);
    read_word(handle, BQ27220_CMD_FULL_CHARGE_CAP, &full);
    read_word(handle, BQ27220_CMD_TIME_TO_EMPTY, &tte);
    read_word(handle, BQ27220_CMD_TIME_TO_FULL, &ttf);
    read_word(handle, BQ27220_CMD_BATTERY_STATUS, &status);
    out_data->voltage_v = voltage / 1000.0f;
    out_data->current_ma = (float)(int16_t)current;
    out_data->average_current_ma = (float)(int16_t)avg_current;
    out_data->temperature_c = temperature / 10.0f - 273.15f;
    out_data->state_of_charge_pct = soc > 100 ? 100 : soc;
    out_data->state_of_health_pct = soh > 100 ? 100 : soh;
    out_data->remaining_capacity_mah = remaining;
    out_data->full_charge_capacity_mah = full;
    out_data->time_to_empty_min = tte;
    out_data->time_to_full_min = ttf;
    out_data->is_charging = !(status & BQ27220_STATUS_DSG) && (int16_t)current > 0;
    return ESP_OK;
}
