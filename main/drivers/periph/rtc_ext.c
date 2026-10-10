#include "drivers/periph/rtc_ext.h"
#include <esp_log.h>
#include <stdlib.h>
#include <string.h>
#define TAG "RtcExt"
#define I2C_TIMEOUT_MS 100
#define DS3231_REG_SECONDS 0x00
#define DS3231_REG_CONTROL 0x0E
#define DS3231_REG_STATUS  0x0F
#define DS3231_REG_TEMP_MSB 0x11
#define DS3231_STATUS_OSF  0x80
#define PCF8563_REG_CONTROL1 0x00
#define PCF8563_REG_CONTROL2 0x01
#define PCF8563_REG_SECONDS  0x02
#define PCF8563_VL_BIT       0x80
typedef struct rtc_ext_dev_s {
    i2c_master_dev_handle_t i2c_dev;
    rtc_ext_type_t          type;
} rtc_ext_dev_s;
static uint8_t bcd_to_bin(uint8_t v)
{
    return (uint8_t)((v >> 4) * 10 + (v & 0x0F));
}
static uint8_t bin_to_bcd(uint8_t v)
{
    return (uint8_t)(((v / 10) << 4) | (v % 10));
}
static esp_err_t read_regs(rtc_ext_dev_s *dev, uint8_t reg, uint8_t *buf, size_t len)
{
    return i2c_master_transmit_receive(dev->i2c_dev, &reg, 1, buf, len, I2C_TIMEOUT_MS);
}
static esp_err_t write_regs(rtc_ext_dev_s *dev, uint8_t reg, const uint8_t *buf, size_t len)
{
    uint8_t tx[9];
    if (len > sizeof(tx) - 1) return ESP_ERR_INVALID_SIZE;
    tx[0] = reg;
    memcpy(&tx[1], buf, len);
    return i2c_master_transmit(dev->i2c_dev, tx, len + 1, I2C_TIMEOUT_MS);
}
static esp_err_t attach(i2c_master_bus_handle_t bus, uint8_t addr, rtc_ext_type_t type, rtc_ext_dev_s **out)
{
    if (i2c_master_probe(bus, addr, 50) != ESP_OK) return ESP_ERR_NOT_FOUND;
    rtc_ext_dev_s *dev = calloc(1, sizeof(rtc_ext_dev_s));
    if (!dev) return ESP_ERR_NO_MEM;
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = 100000,
    };
    esp_err_t ret = i2c_master_bus_add_device(bus, &cfg, &dev->i2c_dev);
    if (ret != ESP_OK) {
        free(dev);
        return ret;
    }
    dev->type = type;
    *out = dev;
    return ESP_OK;
}
esp_err_t rtc_ext_init(i2c_master_bus_handle_t i2c_bus, rtc_ext_handle_t *out_handle)
{
    if (!i2c_bus || !out_handle) return ESP_ERR_INVALID_ARG;
    *out_handle = NULL;
    rtc_ext_dev_s *dev = NULL;
    if (attach(i2c_bus, RTC_EXT_DS3231_ADDR, RTC_EXT_TYPE_DS3231, &dev) == ESP_OK) {
        uint8_t control = 0;
        if (read_regs(dev, DS3231_REG_CONTROL, &control, 1) != ESP_OK) {
            rtc_ext_deinit(dev);
            dev = NULL;
        } else {
            control &= (uint8_t)~0x80;
            write_regs(dev, DS3231_REG_CONTROL, &control, 1);
        }
    }
    if (!dev && attach(i2c_bus, RTC_EXT_PCF8563_ADDR, RTC_EXT_TYPE_PCF8563, &dev) == ESP_OK) {
        uint8_t ctrl[2] = {0x00, 0x00};
        if (write_regs(dev, PCF8563_REG_CONTROL1, ctrl, sizeof(ctrl)) != ESP_OK) {
            rtc_ext_deinit(dev);
            dev = NULL;
        }
    }
    if (!dev) return ESP_ERR_NOT_FOUND;
    ESP_LOGI(TAG, "External RTC %s detected", rtc_ext_type_name(dev->type));
    *out_handle = dev;
    return ESP_OK;
}
void rtc_ext_deinit(rtc_ext_handle_t handle)
{
    if (!handle) return;
    if (handle->i2c_dev) i2c_master_bus_rm_device(handle->i2c_dev);
    free(handle);
}
rtc_ext_type_t rtc_ext_get_type(rtc_ext_handle_t handle)
{
    return handle ? handle->type : RTC_EXT_TYPE_UNKNOWN;
}
const char *rtc_ext_type_name(rtc_ext_type_t type)
{
    switch (type) {
        case RTC_EXT_TYPE_DS3231:  return "DS3231";
        case RTC_EXT_TYPE_PCF8563: return "PCF8563";
        default:                   return "unknown";
    }
}
esp_err_t rtc_ext_get_time(rtc_ext_handle_t handle, struct tm *out_time, bool *out_valid)
{
    if (!handle || !out_time) return ESP_ERR_INVALID_ARG;
    uint8_t r[7] = {0};
    bool valid = true;
    memset(out_time, 0, sizeof(*out_time));
    if (handle->type == RTC_EXT_TYPE_DS3231) {
        esp_err_t ret = read_regs(handle, DS3231_REG_SECONDS, r, 7);
        if (ret != ESP_OK) return ret;
        uint8_t status = 0;
        if (read_regs(handle, DS3231_REG_STATUS, &status, 1) == ESP_OK && (status & DS3231_STATUS_OSF)) {
            valid = false;
        }
        out_time->tm_sec = bcd_to_bin(r[0] & 0x7F);
        out_time->tm_min = bcd_to_bin(r[1] & 0x7F);
        if (r[2] & 0x40) {
            int hour = bcd_to_bin(r[2] & 0x1F) % 12;
            out_time->tm_hour = (r[2] & 0x20) ? hour + 12 : hour;
        } else {
            out_time->tm_hour = bcd_to_bin(r[2] & 0x3F);
        }
        out_time->tm_wday = (r[3] & 0x07) % 7;
        out_time->tm_mday = bcd_to_bin(r[4] & 0x3F);
        out_time->tm_mon = bcd_to_bin(r[5] & 0x1F) - 1;
        out_time->tm_year = bcd_to_bin(r[6]) + ((r[5] & 0x80) ? 200 : 100);
    } else if (handle->type == RTC_EXT_TYPE_PCF8563) {
        esp_err_t ret = read_regs(handle, PCF8563_REG_SECONDS, r, 7);
        if (ret != ESP_OK) return ret;
        if (r[0] & PCF8563_VL_BIT) valid = false;
        out_time->tm_sec = bcd_to_bin(r[0] & 0x7F);
        out_time->tm_min = bcd_to_bin(r[1] & 0x7F);
        out_time->tm_hour = bcd_to_bin(r[2] & 0x3F);
        out_time->tm_mday = bcd_to_bin(r[3] & 0x3F);
        out_time->tm_wday = r[4] & 0x07;
        out_time->tm_mon = bcd_to_bin(r[5] & 0x1F) - 1;
        out_time->tm_year = bcd_to_bin(r[6]) + ((r[5] & 0x80) ? 200 : 100);
    } else {
        return ESP_ERR_INVALID_STATE;
    }
    if (out_time->tm_mon < 0 || out_time->tm_mon > 11 || out_time->tm_mday < 1 || out_time->tm_mday > 31 ||
        out_time->tm_hour > 23 || out_time->tm_min > 59 || out_time->tm_sec > 59) {
        valid = false;
    }
    out_time->tm_isdst = -1;
    if (out_valid) *out_valid = valid;
    return ESP_OK;
}
esp_err_t rtc_ext_set_time(rtc_ext_handle_t handle, const struct tm *time)
{
    if (!handle || !time) return ESP_ERR_INVALID_ARG;
    if (time->tm_year < 100 || time->tm_year > 299) return ESP_ERR_INVALID_ARG;
    int year = time->tm_year - 100;
    uint8_t century = 0;
    if (year >= 100) {
        year -= 100;
        century = 0x80;
    }
    uint8_t w[7];
    w[0] = bin_to_bcd((uint8_t)time->tm_sec);
    w[1] = bin_to_bcd((uint8_t)time->tm_min);
    w[2] = bin_to_bcd((uint8_t)time->tm_hour);
    w[5] = (uint8_t)(bin_to_bcd((uint8_t)(time->tm_mon + 1)) | century);
    w[6] = bin_to_bcd((uint8_t)year);
    if (handle->type == RTC_EXT_TYPE_DS3231) {
        w[3] = (uint8_t)((time->tm_wday % 7) + 1);
        w[4] = bin_to_bcd((uint8_t)time->tm_mday);
        esp_err_t ret = write_regs(handle, DS3231_REG_SECONDS, w, 7);
        if (ret != ESP_OK) return ret;
        uint8_t status = 0;
        if (read_regs(handle, DS3231_REG_STATUS, &status, 1) == ESP_OK) {
            status &= (uint8_t)~DS3231_STATUS_OSF;
            write_regs(handle, DS3231_REG_STATUS, &status, 1);
        }
        return ESP_OK;
    }
    if (handle->type == RTC_EXT_TYPE_PCF8563) {
        w[3] = bin_to_bcd((uint8_t)time->tm_mday);
        w[4] = (uint8_t)(time->tm_wday % 7);
        return write_regs(handle, PCF8563_REG_SECONDS, w, 7);
    }
    return ESP_ERR_INVALID_STATE;
}
esp_err_t rtc_ext_get_temperature(rtc_ext_handle_t handle, float *out_temp_c)
{
    if (!handle || !out_temp_c) return ESP_ERR_INVALID_ARG;
    if (handle->type != RTC_EXT_TYPE_DS3231) return ESP_ERR_NOT_SUPPORTED;
    uint8_t t[2] = {0};
    esp_err_t ret = read_regs(handle, DS3231_REG_TEMP_MSB, t, sizeof(t));
    if (ret != ESP_OK) return ret;
    *out_temp_c = (float)(int8_t)t[0] + (float)(t[1] >> 6) * 0.25f;
    return ESP_OK;
}
