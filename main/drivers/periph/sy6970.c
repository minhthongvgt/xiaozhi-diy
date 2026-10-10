#include "drivers/periph/sy6970.h"
#include <esp_log.h>
#include <stdlib.h>
#define TAG "SY6970"
#define I2C_TIMEOUT_MS 100
#define REG_CHARGE_STATUS 0x0B
#define REG_CHARGE_TARGET 0x06
#define REG_BATTERY_V     0x0E
#define BATTERY_MIN_MV    3200
typedef struct sy6970_dev_s {
    i2c_master_dev_handle_t i2c_dev;
} sy6970_dev_s;
static esp_err_t read_reg(sy6970_dev_s *dev, uint8_t reg, uint8_t *val)
{
    return i2c_master_transmit_receive(dev->i2c_dev, &reg, 1, val, 1, I2C_TIMEOUT_MS);
}
esp_err_t sy6970_init(i2c_master_bus_handle_t i2c_bus, sy6970_handle_t *out_handle)
{
    if (!i2c_bus || !out_handle) return ESP_ERR_INVALID_ARG;
    *out_handle = NULL;
    if (i2c_master_probe(i2c_bus, SY6970_I2C_ADDR, 50) != ESP_OK) return ESP_ERR_NOT_FOUND;
    sy6970_dev_s *dev = calloc(1, sizeof(sy6970_dev_s));
    if (!dev) return ESP_ERR_NO_MEM;
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SY6970_I2C_ADDR,
        .scl_speed_hz = 100000,
    };
    esp_err_t ret = i2c_master_bus_add_device(i2c_bus, &cfg, &dev->i2c_dev);
    if (ret != ESP_OK) {
        free(dev);
        return ret;
    }
    uint8_t status = 0;
    if (read_reg(dev, REG_CHARGE_STATUS, &status) != ESP_OK) {
        sy6970_deinit(dev);
        return ESP_ERR_NOT_FOUND;
    }
    ESP_LOGI(TAG, "SY6970 charger detected at 0x%02X", SY6970_I2C_ADDR);
    *out_handle = dev;
    return ESP_OK;
}
void sy6970_deinit(sy6970_handle_t handle)
{
    if (!handle) return;
    if (handle->i2c_dev) i2c_master_bus_rm_device(handle->i2c_dev);
    free(handle);
}
esp_err_t sy6970_read_status(sy6970_handle_t handle, sy6970_status_t *out_status)
{
    if (!handle || !out_status) return ESP_ERR_INVALID_ARG;
    uint8_t chg = 0, vbat = 0, target = 0;
    esp_err_t ret = read_reg(handle, REG_CHARGE_STATUS, &chg);
    if (ret != ESP_OK) return ret;
    ret = read_reg(handle, REG_BATTERY_V, &vbat);
    if (ret != ESP_OK) return ret;
    ret = read_reg(handle, REG_CHARGE_TARGET, &target);
    if (ret != ESP_OK) return ret;
    int changing = (chg >> 3) & 0x03;
    out_status->charging = changing != 0;
    out_status->charge_done = changing == 3;
    out_status->power_good = (chg & 0x04) != 0;
    int v = vbat & 0x7F;
    out_status->battery_mv = v == 0 ? 0 : v * 20 + 2304;
    int limit_code = (target & 0xFC) >> 2;
    int limit_mv = limit_code > 0x30 ? 4608 : limit_code * 16 + 3840;
    int pct = 0;
    if (out_status->battery_mv > BATTERY_MIN_MV && limit_mv > BATTERY_MIN_MV) {
        pct = (out_status->battery_mv - BATTERY_MIN_MV) * 100 / (limit_mv - BATTERY_MIN_MV);
    }
    out_status->battery_pct = pct > 100 ? 100 : (pct < 0 ? 0 : pct);
    return ESP_OK;
}
