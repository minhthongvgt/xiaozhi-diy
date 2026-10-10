#include "drivers/periph/pca9685.h"
#include <esp_log.h>
#include <esp_rom_sys.h>
#include <math.h>
#include <stdlib.h>
#define TAG "PCA9685"
#define I2C_TIMEOUT_MS 100
#define PCA9685_REG_MODE1     0x00
#define PCA9685_REG_MODE2     0x01
#define PCA9685_REG_LED0_ON_L 0x06
#define PCA9685_REG_ALL_ON_L  0xFA
#define PCA9685_REG_PRESCALE  0xFE
#define PCA9685_MODE1_RESTART 0x80
#define PCA9685_MODE1_AI      0x20
#define PCA9685_MODE1_SLEEP   0x10
#define PCA9685_MODE1_ALLCALL 0x01
#define PCA9685_MODE2_OUTDRV  0x04
#define PCA9685_OSC_HZ        25000000.0f
#define PCA9685_FULL_BIT      0x1000
typedef struct pca9685_dev_s {
    i2c_master_dev_handle_t i2c_dev;
    gpio_num_t              oe_pin;
    uint16_t                freq_hz;
} pca9685_dev_s;
static esp_err_t write_reg(pca9685_dev_s *dev, uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = {reg, val};
    return i2c_master_transmit(dev->i2c_dev, tx, sizeof(tx), I2C_TIMEOUT_MS);
}
static esp_err_t read_reg(pca9685_dev_s *dev, uint8_t reg, uint8_t *val)
{
    return i2c_master_transmit_receive(dev->i2c_dev, &reg, 1, val, 1, I2C_TIMEOUT_MS);
}
esp_err_t pca9685_set_frequency(pca9685_handle_t handle, uint16_t freq_hz)
{
    if (!handle) return ESP_ERR_INVALID_ARG;
    if (freq_hz < 24) freq_hz = 24;
    if (freq_hz > 1526) freq_hz = 1526;
    float prescale_f = roundf(PCA9685_OSC_HZ / (4096.0f * (float)freq_hz)) - 1.0f;
    if (prescale_f < 3.0f) prescale_f = 3.0f;
    if (prescale_f > 255.0f) prescale_f = 255.0f;
    uint8_t mode1 = 0;
    esp_err_t ret = read_reg(handle, PCA9685_REG_MODE1, &mode1);
    if (ret != ESP_OK) return ret;
    uint8_t sleep_mode = (uint8_t)((mode1 & ~PCA9685_MODE1_RESTART) | PCA9685_MODE1_SLEEP);
    write_reg(handle, PCA9685_REG_MODE1, sleep_mode);
    write_reg(handle, PCA9685_REG_PRESCALE, (uint8_t)prescale_f);
    uint8_t awake = (uint8_t)((mode1 & ~(PCA9685_MODE1_SLEEP | PCA9685_MODE1_RESTART)) | PCA9685_MODE1_AI);
    write_reg(handle, PCA9685_REG_MODE1, awake);
    esp_rom_delay_us(600);
    ret = write_reg(handle, PCA9685_REG_MODE1, (uint8_t)(awake | PCA9685_MODE1_RESTART));
    if (ret == ESP_OK) {
        handle->freq_hz = freq_hz;
    }
    return ret;
}
uint16_t pca9685_get_frequency(pca9685_handle_t handle)
{
    return handle ? handle->freq_hz : 0;
}
esp_err_t pca9685_init(const pca9685_config_t *config, pca9685_handle_t *out_handle)
{
    if (!config || !config->i2c_bus || !out_handle) return ESP_ERR_INVALID_ARG;
    *out_handle = NULL;
    uint8_t addr = config->i2c_addr ? config->i2c_addr : PCA9685_I2C_ADDR_DEFAULT;
    if (i2c_master_probe(config->i2c_bus, addr, 50) != ESP_OK) return ESP_ERR_NOT_FOUND;
    pca9685_dev_s *dev = calloc(1, sizeof(pca9685_dev_s));
    if (!dev) return ESP_ERR_NO_MEM;
    dev->oe_pin = config->oe_pin;
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = 400000,
    };
    esp_err_t ret = i2c_master_bus_add_device(config->i2c_bus, &cfg, &dev->i2c_dev);
    if (ret != ESP_OK) {
        free(dev);
        return ret;
    }
    uint8_t mode1 = 0;
    ret = read_reg(dev, PCA9685_REG_MODE1, &mode1);
    if (ret != ESP_OK || mode1 == 0x39 || (mode1 & 0x0E) != 0) {
        ESP_LOGW(TAG, "Device at 0x%02X does not look like a PCA9685 (MODE1=0x%02X)", addr, mode1);
        pca9685_deinit(dev);
        return ESP_ERR_NOT_FOUND;
    }
    if (GPIO_IS_VALID_OUTPUT_GPIO(dev->oe_pin)) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << dev->oe_pin),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&io_conf);
        gpio_set_level(dev->oe_pin, 1);
    }
    write_reg(dev, PCA9685_REG_MODE1, PCA9685_MODE1_AI | PCA9685_MODE1_ALLCALL);
    write_reg(dev, PCA9685_REG_MODE2, PCA9685_MODE2_OUTDRV);
    pca9685_all_off(dev);
    ret = pca9685_set_frequency(dev, config->pwm_freq_hz ? config->pwm_freq_hz : 50);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set PWM frequency: %s", esp_err_to_name(ret));
        pca9685_deinit(dev);
        return ret;
    }
    pca9685_set_output_enabled(dev, true);
    ESP_LOGI(TAG, "PCA9685 initialized at 0x%02X (%u Hz)", addr, dev->freq_hz);
    *out_handle = dev;
    return ESP_OK;
}
void pca9685_deinit(pca9685_handle_t handle)
{
    if (!handle) return;
    if (handle->i2c_dev) i2c_master_bus_rm_device(handle->i2c_dev);
    free(handle);
}
esp_err_t pca9685_set_pwm(pca9685_handle_t handle, uint8_t channel, uint16_t on_tick, uint16_t off_tick)
{
    if (!handle || channel >= PCA9685_CHANNEL_COUNT) return ESP_ERR_INVALID_ARG;
    uint8_t tx[5] = {
        (uint8_t)(PCA9685_REG_LED0_ON_L + 4 * channel),
        (uint8_t)(on_tick & 0xFF),
        (uint8_t)((on_tick >> 8) & 0x1F),
        (uint8_t)(off_tick & 0xFF),
        (uint8_t)((off_tick >> 8) & 0x1F),
    };
    return i2c_master_transmit(handle->i2c_dev, tx, sizeof(tx), I2C_TIMEOUT_MS);
}
esp_err_t pca9685_set_duty(pca9685_handle_t handle, uint8_t channel, float duty_pct)
{
    if (duty_pct <= 0.0f) return pca9685_set_pwm(handle, channel, 0, PCA9685_FULL_BIT);
    if (duty_pct >= 100.0f) return pca9685_set_pwm(handle, channel, PCA9685_FULL_BIT, 0);
    uint16_t off = (uint16_t)lroundf(duty_pct * 4095.0f / 100.0f);
    return pca9685_set_pwm(handle, channel, 0, off);
}
esp_err_t pca9685_set_pulse_us(pca9685_handle_t handle, uint8_t channel, uint32_t pulse_us)
{
    if (!handle || handle->freq_hz == 0) return ESP_ERR_INVALID_STATE;
    float period_us = 1000000.0f / (float)handle->freq_hz;
    float ticks = (float)pulse_us * 4096.0f / period_us;
    if (ticks > 4095.0f) ticks = 4095.0f;
    return pca9685_set_pwm(handle, channel, 0, (uint16_t)lroundf(ticks));
}
esp_err_t pca9685_set_servo_angle(pca9685_handle_t handle, uint8_t channel, float angle_deg,
                                  uint32_t min_us, uint32_t max_us)
{
    if (angle_deg < 0.0f) angle_deg = 0.0f;
    if (angle_deg > 180.0f) angle_deg = 180.0f;
    if (max_us <= min_us) return ESP_ERR_INVALID_ARG;
    uint32_t pulse = min_us + (uint32_t)lroundf((float)(max_us - min_us) * angle_deg / 180.0f);
    return pca9685_set_pulse_us(handle, channel, pulse);
}
esp_err_t pca9685_set_output_enabled(pca9685_handle_t handle, bool enabled)
{
    if (!handle) return ESP_ERR_INVALID_ARG;
    if (!GPIO_IS_VALID_OUTPUT_GPIO(handle->oe_pin)) return ESP_OK;
    return gpio_set_level(handle->oe_pin, enabled ? 0 : 1);
}
esp_err_t pca9685_all_off(pca9685_handle_t handle)
{
    if (!handle) return ESP_ERR_INVALID_ARG;
    uint8_t tx[5] = {PCA9685_REG_ALL_ON_L, 0x00, 0x00, 0x00, 0x10};
    return i2c_master_transmit(handle->i2c_dev, tx, sizeof(tx), I2C_TIMEOUT_MS);
}
