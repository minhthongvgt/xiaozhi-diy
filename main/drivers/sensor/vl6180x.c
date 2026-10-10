#include "drivers/sensor/vl6180x.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_rom_sys.h>
#include <string.h>
#include <stdlib.h>
#define TAG "VL6180X"
#define I2C_TIMEOUT_MS 100
typedef struct vl6180x_dev_s {
    i2c_master_dev_handle_t i2c_dev;
    i2c_master_bus_handle_t i2c_bus;
    uint8_t                 i2c_addr;
    gpio_num_t              xshut_pin;
    gpio_num_t              gpio1_pin;
    uint8_t                 scaling;
    vl6180x_als_gain_t      als_gain;
    uint16_t                als_integration_ms;
} vl6180x_dev_s;
static const struct {
    uint16_t reg;
    uint8_t  val;
} s_mandatory_tuning[] = {
    {0x0207, 0x01}, {0x0208, 0x01}, {0x0096, 0x00}, {0x0097, 0xFD},
    {0x00E3, 0x01}, {0x00E4, 0x03}, {0x00E5, 0x02}, {0x00E6, 0x01},
    {0x00E7, 0x03}, {0x00F5, 0x02}, {0x00D9, 0x05}, {0x00DB, 0xCE},
    {0x00DC, 0x03}, {0x00DD, 0xF8}, {0x009F, 0x00}, {0x00A3, 0x3C},
    {0x00B7, 0x00}, {0x00BB, 0x3C}, {0x00B2, 0x09}, {0x00CA, 0x09},
    {0x0198, 0x01}, {0x01B0, 0x17}, {0x01AD, 0x00}, {0x00FF, 0x05},
    {0x0100, 0x05}, {0x0199, 0x05}, {0x01A6, 0x1B}, {0x01AC, 0x3E},
    {0x01A7, 0x1F}, {0x0030, 0x00}
};
static esp_err_t vl6180x_write_reg8(vl6180x_handle_t dev, uint16_t reg, uint8_t val)
{
    uint8_t tx_buf[3];
    tx_buf[0] = (uint8_t)((reg >> 8) & 0xFF);
    tx_buf[1] = (uint8_t)(reg & 0xFF);
    tx_buf[2] = val;
    return i2c_master_transmit(dev->i2c_dev, tx_buf, sizeof(tx_buf), I2C_TIMEOUT_MS);
}
static esp_err_t vl6180x_write_reg16(vl6180x_handle_t dev, uint16_t reg, uint16_t val)
{
    uint8_t tx_buf[4];
    tx_buf[0] = (uint8_t)((reg >> 8) & 0xFF);
    tx_buf[1] = (uint8_t)(reg & 0xFF);
    tx_buf[2] = (uint8_t)((val >> 8) & 0xFF);
    tx_buf[3] = (uint8_t)(val & 0xFF);
    return i2c_master_transmit(dev->i2c_dev, tx_buf, sizeof(tx_buf), I2C_TIMEOUT_MS);
}
static esp_err_t vl6180x_read_reg8(vl6180x_handle_t dev, uint16_t reg, uint8_t *val)
{
    uint8_t reg_addr[2];
    reg_addr[0] = (uint8_t)((reg >> 8) & 0xFF);
    reg_addr[1] = (uint8_t)(reg & 0xFF);
    return i2c_master_transmit_receive(dev->i2c_dev, reg_addr, sizeof(reg_addr), val, 1, I2C_TIMEOUT_MS);
}
static esp_err_t vl6180x_read_reg16(vl6180x_handle_t dev, uint16_t reg, uint16_t *val)
{
    uint8_t reg_addr[2];
    uint8_t rx_buf[2];
    reg_addr[0] = (uint8_t)((reg >> 8) & 0xFF);
    reg_addr[1] = (uint8_t)(reg & 0xFF);
    esp_err_t ret = i2c_master_transmit_receive(dev->i2c_dev, reg_addr, sizeof(reg_addr), rx_buf, 2, I2C_TIMEOUT_MS);
    if (ret == ESP_OK) {
        *val = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];
    }
    return ret;
}
static float vl6180x_get_gain_multiplier(vl6180x_als_gain_t gain)
{
    switch (gain) {
        case VL6180X_ALS_GAIN_20:   return 20.0f;
        case VL6180X_ALS_GAIN_10:   return 10.0f;
        case VL6180X_ALS_GAIN_5:    return 5.0f;
        case VL6180X_ALS_GAIN_2_5:  return 2.5f;
        case VL6180X_ALS_GAIN_1_67: return 1.67f;
        case VL6180X_ALS_GAIN_1_25: return 1.25f;
        case VL6180X_ALS_GAIN_1:    return 1.0f;
        case VL6180X_ALS_GAIN_40:   return 40.0f;
        default:                    return 1.0f;
    }
}
esp_err_t vl6180x_init(const vl6180x_config_t *config, vl6180x_handle_t *out_handle)
{
    if (!config || !config->i2c_bus || !out_handle) {
        return ESP_ERR_INVALID_ARG;
    }
    struct vl6180x_dev_s *dev = calloc(1, sizeof(struct vl6180x_dev_s));
    if (!dev) {
        return ESP_ERR_NO_MEM;
    }
    dev->i2c_bus = config->i2c_bus;
    dev->i2c_addr = config->i2c_addr ? config->i2c_addr : VL6180X_DEFAULT_I2C_ADDR;
    dev->xshut_pin = config->xshut_pin;
    dev->gpio1_pin = config->gpio1_pin;
    dev->scaling = (config->scaling > 0 && config->scaling <= 3) ? config->scaling : 1;
    dev->als_gain = VL6180X_ALS_GAIN_1;
    dev->als_integration_ms = 100;
    if (dev->xshut_pin >= 0) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << dev->xshut_pin),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&io_conf);
        gpio_set_level(dev->xshut_pin, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(dev->xshut_pin, 1);
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = dev->i2c_addr,
        .scl_speed_hz = 400000,
    };
    esp_err_t ret = i2c_master_bus_add_device(dev->i2c_bus, &dev_cfg, &dev->i2c_dev);
    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "VL6180X I2C device registration failed: %s", esp_err_to_name(ret));
        free(dev);
        *out_handle = NULL;
        return ret;
    }
    uint8_t model_id = 0;
    ret = vl6180x_read_reg8(dev, VL6180X_REG_IDENTIFICATION_MODEL_ID, &model_id);
    if (ret != ESP_OK || model_id != VL6180X_EXPECTED_MODEL_ID) {
        ESP_LOGE(TAG, "VL6180X identification failed: reg=0x%02X, expected=0x%02X (err: %s)",
                 model_id, VL6180X_EXPECTED_MODEL_ID, esp_err_to_name(ret));
        i2c_master_bus_rm_device(dev->i2c_dev);
        free(dev);
        return (ret == ESP_OK) ? ESP_ERR_NOT_FOUND : ret;
    }
    ESP_LOGI(TAG, "Found STMicroelectronics VL6180X (Model ID: 0x%02X) on I2C address 0x%02X",
             model_id, dev->i2c_addr);
    uint8_t fresh_reset = 0;
    vl6180x_read_reg8(dev, VL6180X_REG_SYSTEM_FRESH_OUT_OF_RESET, &fresh_reset);
    if (fresh_reset == 0x01) {
        ESP_LOGD(TAG, "Device fresh out of reset. Applying ST AN4545 mandatory tuning sequence...");
        for (size_t i = 0; i < sizeof(s_mandatory_tuning) / sizeof(s_mandatory_tuning[0]); i++) {
            vl6180x_write_reg8(dev, s_mandatory_tuning[i].reg, s_mandatory_tuning[i].val);
        }
        vl6180x_write_reg8(dev, VL6180X_REG_SYSTEM_FRESH_OUT_OF_RESET, 0x00);
    }
    vl6180x_write_reg8(dev, VL6180X_REG_READOUT_AVERAGING_SAMPLE_PERIOD, 0x30);
    vl6180x_write_reg8(dev, VL6180X_REG_SYSALS_ANALOGUE_GAIN, 0x46);
    vl6180x_write_reg8(dev, VL6180X_REG_SYSRANGE_VHV_REPEAT_RATE, 0xFF);
    vl6180x_write_reg16(dev, VL6180X_REG_SYSALS_INTEGRATION_PERIOD, 0x0063);
    vl6180x_write_reg8(dev, VL6180X_REG_SYSRANGE_VHV_RECALIBRATE, 0x01);
    vl6180x_write_reg8(dev, VL6180X_REG_SYSRANGE_INTERMEASUREMENT_PERIOD, 0x09);
    vl6180x_write_reg8(dev, VL6180X_REG_SYSALS_INTERMEASUREMENT_PERIOD, 0x31);
    vl6180x_write_reg8(dev, VL6180X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x24);
    vl6180x_set_scaling(dev, dev->scaling);
    *out_handle = dev;
    ESP_LOGI(TAG, "VL6180X initialized successfully (Scaling: %ux, Range: 0..%umm, ALS: 0.1..100k Lux)",
             dev->scaling, dev->scaling * 100);
    return ESP_OK;
}
esp_err_t vl6180x_deinit(vl6180x_handle_t handle)
{
    if (!handle) return ESP_ERR_INVALID_ARG;
    if (handle->xshut_pin >= 0) {
        gpio_set_level(handle->xshut_pin, 0);
    }
    if (handle->i2c_dev) {
        i2c_master_bus_rm_device(handle->i2c_dev);
    }
    free(handle);
    return ESP_OK;
}
esp_err_t vl6180x_read_distance_mm(vl6180x_handle_t handle, float *out_distance_mm)
{
    if (!handle || !out_distance_mm) return ESP_ERR_INVALID_ARG;
    uint8_t status = 0;
    int timeout = 50;
    while (timeout--) {
        vl6180x_read_reg8(handle, VL6180X_REG_RESULT_RANGE_STATUS, &status);
        if (status & 0x01) break; 
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    esp_err_t ret = vl6180x_write_reg8(handle, VL6180X_REG_SYSRANGE_START, 0x01);
    if (ret != ESP_OK) return ret;
    timeout = 100; 
    bool ready = false;
    while (timeout--) {
        uint8_t intr_status = 0;
        vl6180x_read_reg8(handle, VL6180X_REG_RESULT_INTERRUPT_STATUS_GPIO, &intr_status);
        if ((intr_status & 0x04) != 0) { 
            ready = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    if (!ready) {
        vl6180x_write_reg8(handle, VL6180X_REG_SYSTEM_INTERRUPT_CLEAR, 0x07);
        return ESP_ERR_TIMEOUT;
    }
    vl6180x_read_reg8(handle, VL6180X_REG_RESULT_RANGE_STATUS, &status);
    uint8_t err_code = (status >> 4) & 0x0F;
    uint8_t raw_mm = 0;
    vl6180x_read_reg8(handle, VL6180X_REG_RESULT_RANGE_VAL, &raw_mm);
    vl6180x_write_reg8(handle, VL6180X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    if (err_code != VL6180X_ERROR_NONE) {
        if (err_code == VL6180X_ERROR_RANGEOFL || err_code == VL6180X_ERROR_RAWUFL) {
            *out_distance_mm = (float)(handle->scaling * 255);
        } else {
            *out_distance_mm = (float)(raw_mm * handle->scaling);
        }
        return ESP_FAIL;
    }
    *out_distance_mm = (float)(raw_mm * handle->scaling);
    return ESP_OK;
}
esp_err_t vl6180x_read_ambient_lux(vl6180x_handle_t handle, float *out_lux)
{
    if (!handle || !out_lux) return ESP_ERR_INVALID_ARG;
    esp_err_t ret = vl6180x_write_reg8(handle, VL6180X_REG_SYSALS_START, 0x01);
    if (ret != ESP_OK) return ret;
    int timeout = 200; 
    bool ready = false;
    while (timeout--) {
        uint8_t intr_status = 0;
        vl6180x_read_reg8(handle, VL6180X_REG_RESULT_INTERRUPT_STATUS_GPIO, &intr_status);
        if ((intr_status & 0x20) != 0) { 
            ready = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    if (!ready) {
        vl6180x_write_reg8(handle, VL6180X_REG_SYSTEM_INTERRUPT_CLEAR, 0x07);
        return ESP_ERR_TIMEOUT;
    }
    uint16_t als_raw = 0;
    vl6180x_read_reg16(handle, VL6180X_REG_RESULT_ALS_VAL, &als_raw);
    vl6180x_write_reg8(handle, VL6180X_REG_SYSTEM_INTERRUPT_CLEAR, 0x02);
    float gain_val = vl6180x_get_gain_multiplier(handle->als_gain);
    float integration_factor = (float)handle->als_integration_ms / 100.0f;
    *out_lux = (0.32f * (float)als_raw) / (gain_val * integration_factor);
    return ESP_OK;
}
esp_err_t vl6180x_set_als_gain(vl6180x_handle_t handle, vl6180x_als_gain_t gain)
{
    if (!handle) return ESP_ERR_INVALID_ARG;
    uint8_t reg_val = 0x40 | (gain & 0x07);
    esp_err_t ret = vl6180x_write_reg8(handle, VL6180X_REG_SYSALS_ANALOGUE_GAIN, reg_val);
    if (ret == ESP_OK) {
        handle->als_gain = gain;
    }
    return ret;
}
esp_err_t vl6180x_set_i2c_address(vl6180x_handle_t handle, uint8_t new_addr)
{
    if (!handle || new_addr == 0 || new_addr > 0x7F) return ESP_ERR_INVALID_ARG;
    esp_err_t ret = vl6180x_write_reg8(handle, VL6180X_REG_I2C_SLAVE_DEVICE_ADDRESS, new_addr);
    if (ret == ESP_OK) {
        handle->i2c_addr = new_addr;
        ESP_LOGI(TAG, "VL6180X address changed to 0x%02X", new_addr);
    }
    return ret;
}
esp_err_t vl6180x_set_scaling(vl6180x_handle_t handle, uint8_t scaling)
{
    if (!handle || scaling < 1 || scaling > 3) return ESP_ERR_INVALID_ARG;
    uint16_t ece_factor = 0x60 / scaling;
    vl6180x_write_reg8(handle, VL6180X_REG_SYSRANGE_CROSSTALK_VALID_HEIGHT, (uint8_t)(20 / scaling));
    vl6180x_write_reg8(handle, VL6180X_REG_SYSRANGE_EARLY_CONVERGENCE_ESTIMATE, (uint8_t)ece_factor);
    handle->scaling = scaling;
    ESP_LOGD(TAG, "Range scaling set to %ux (Max range: %dmm)", scaling, scaling * 100);
    return ESP_OK;
}
