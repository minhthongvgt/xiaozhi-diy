#include "drivers/periph/led_ic.h"
#include <esp_log.h>
#include <stdlib.h>
#define TAG "LedIc"
#define I2C_TIMEOUT_MS 100
#define AW9523B_REG_ID       0x10
#define AW9523B_REG_GCR      0x11
#define AW9523B_REG_LEDMS_P0 0x12
#define AW9523B_REG_LEDMS_P1 0x13
#define AW9523B_REG_DIM0     0x20
#define AW9523B_REG_RESET    0x7F
#define AW9523B_CHIP_ID      0x23
#define AW9523B_CHANNELS     16
#define IS31FL3236_REG_SHUTDOWN 0x00
#define IS31FL3236_REG_PWM1     0x01
#define IS31FL3236_REG_UPDATE   0x25
#define IS31FL3236_REG_CTRL1    0x26
#define IS31FL3236_REG_GLOBAL   0x4A
#define IS31FL3236_REG_RESET    0x4F
#define IS31FL3236_CHANNELS     36
typedef struct led_ic_dev_s {
    i2c_master_dev_handle_t i2c_dev;
    led_ic_type_t           type;
    uint8_t                 addr;
} led_ic_dev_s;
static esp_err_t write_reg(led_ic_dev_s *dev, uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = {reg, val};
    return i2c_master_transmit(dev->i2c_dev, tx, sizeof(tx), I2C_TIMEOUT_MS);
}
static esp_err_t read_reg(led_ic_dev_s *dev, uint8_t reg, uint8_t *val)
{
    return i2c_master_transmit_receive(dev->i2c_dev, &reg, 1, val, 1, I2C_TIMEOUT_MS);
}
static esp_err_t attach(i2c_master_bus_handle_t bus, uint8_t addr, led_ic_dev_s **out)
{
    if (i2c_master_probe(bus, addr, 50) != ESP_OK) return ESP_ERR_NOT_FOUND;
    led_ic_dev_s *dev = calloc(1, sizeof(led_ic_dev_s));
    if (!dev) return ESP_ERR_NO_MEM;
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = 400000,
    };
    esp_err_t ret = i2c_master_bus_add_device(bus, &cfg, &dev->i2c_dev);
    if (ret != ESP_OK) {
        free(dev);
        return ret;
    }
    dev->addr = addr;
    *out = dev;
    return ESP_OK;
}
static uint8_t aw9523b_dim_reg(uint8_t channel)
{
    if (channel < 8) return (uint8_t)(0x24 + channel);
    if (channel < 12) return (uint8_t)(0x20 + channel - 8);
    return (uint8_t)(0x2C + channel - 12);
}
static esp_err_t aw9523b_setup(led_ic_dev_s *dev)
{
    write_reg(dev, AW9523B_REG_RESET, 0x00);
    esp_err_t ret = write_reg(dev, AW9523B_REG_LEDMS_P0, 0x00);
    if (ret != ESP_OK) return ret;
    write_reg(dev, AW9523B_REG_LEDMS_P1, 0x00);
    write_reg(dev, AW9523B_REG_GCR, 0x13);
    for (uint8_t ch = 0; ch < AW9523B_CHANNELS; ch++) {
        write_reg(dev, aw9523b_dim_reg(ch), 0x00);
    }
    return ESP_OK;
}
static esp_err_t is31fl3236_setup(led_ic_dev_s *dev)
{
    esp_err_t ret = write_reg(dev, IS31FL3236_REG_RESET, 0x00);
    if (ret != ESP_OK) return ret;
    write_reg(dev, IS31FL3236_REG_SHUTDOWN, 0x01);
    for (uint8_t ch = 0; ch < IS31FL3236_CHANNELS; ch++) {
        write_reg(dev, (uint8_t)(IS31FL3236_REG_PWM1 + ch), 0x00);
        write_reg(dev, (uint8_t)(IS31FL3236_REG_CTRL1 + ch), 0x01);
    }
    write_reg(dev, IS31FL3236_REG_GLOBAL, 0x00);
    return write_reg(dev, IS31FL3236_REG_UPDATE, 0x00);
}
esp_err_t led_ic_init(const led_ic_config_t *config, led_ic_handle_t *out_handle)
{
    if (!config || !config->i2c_bus || !out_handle) return ESP_ERR_INVALID_ARG;
    *out_handle = NULL;
    led_ic_dev_s *dev = NULL;
    for (uint8_t addr = 0x58; addr <= 0x5B && !dev; addr++) {
        if (attach(config->i2c_bus, addr, &dev) != ESP_OK) continue;
        uint8_t id = 0;
        if (read_reg(dev, AW9523B_REG_ID, &id) == ESP_OK && id == AW9523B_CHIP_ID && aw9523b_setup(dev) == ESP_OK) {
            dev->type = LED_IC_TYPE_AW9523B;
        } else {
            led_ic_deinit(dev);
            dev = NULL;
        }
    }
    const uint8_t is31_addrs[] = {0x3F, 0x3E, 0x3D, 0x3C};
    for (size_t i = 0; i < sizeof(is31_addrs) && !dev; i++) {
        if (!config->allow_oled_addresses && is31_addrs[i] <= 0x3D) continue;
        if (attach(config->i2c_bus, is31_addrs[i], &dev) != ESP_OK) continue;
        if (is31fl3236_setup(dev) == ESP_OK) {
            dev->type = LED_IC_TYPE_IS31FL3236;
        } else {
            led_ic_deinit(dev);
            dev = NULL;
        }
    }
    if (!dev) return ESP_ERR_NOT_FOUND;
    ESP_LOGI(TAG, "%s LED driver detected at 0x%02X (%u channels)", led_ic_type_name(dev->type), dev->addr,
             led_ic_get_channel_count(dev));
    *out_handle = dev;
    return ESP_OK;
}
void led_ic_deinit(led_ic_handle_t handle)
{
    if (!handle) return;
    if (handle->i2c_dev) i2c_master_bus_rm_device(handle->i2c_dev);
    free(handle);
}
led_ic_type_t led_ic_get_type(led_ic_handle_t handle)
{
    return handle ? handle->type : LED_IC_TYPE_UNKNOWN;
}
const char *led_ic_type_name(led_ic_type_t type)
{
    switch (type) {
        case LED_IC_TYPE_AW9523B:    return "AW9523B";
        case LED_IC_TYPE_IS31FL3236: return "IS31FL3236";
        default:                     return "unknown";
    }
}
uint8_t led_ic_get_channel_count(led_ic_handle_t handle)
{
    if (!handle) return 0;
    return handle->type == LED_IC_TYPE_AW9523B ? AW9523B_CHANNELS : IS31FL3236_CHANNELS;
}
uint8_t led_ic_get_address(led_ic_handle_t handle)
{
    return handle ? handle->addr : 0;
}
esp_err_t led_ic_set_channel(led_ic_handle_t handle, uint8_t channel, uint8_t brightness)
{
    if (!handle || channel >= led_ic_get_channel_count(handle)) return ESP_ERR_INVALID_ARG;
    if (handle->type == LED_IC_TYPE_AW9523B) {
        return write_reg(handle, aw9523b_dim_reg(channel), brightness);
    }
    esp_err_t ret = write_reg(handle, (uint8_t)(IS31FL3236_REG_PWM1 + channel), brightness);
    if (ret != ESP_OK) return ret;
    return write_reg(handle, IS31FL3236_REG_UPDATE, 0x00);
}
esp_err_t led_ic_set_all(led_ic_handle_t handle, uint8_t brightness)
{
    if (!handle) return ESP_ERR_INVALID_ARG;
    uint8_t count = led_ic_get_channel_count(handle);
    esp_err_t ret = ESP_OK;
    for (uint8_t ch = 0; ch < count && ret == ESP_OK; ch++) {
        if (handle->type == LED_IC_TYPE_AW9523B) {
            ret = write_reg(handle, aw9523b_dim_reg(ch), brightness);
        } else {
            ret = write_reg(handle, (uint8_t)(IS31FL3236_REG_PWM1 + ch), brightness);
        }
    }
    if (ret == ESP_OK && handle->type == LED_IC_TYPE_IS31FL3236) {
        ret = write_reg(handle, IS31FL3236_REG_UPDATE, 0x00);
    }
    return ret;
}
