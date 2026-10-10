#include "drivers/sensor/vl53lx.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdlib.h>
#include <string.h>
#define TAG "VL53LX"
#define I2C_TIMEOUT_MS 100
#define VL53L0X_REG_SYSRANGE_START                0x00
#define VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG        0x01
#define VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO  0x0A
#define VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR        0x0B
#define VL53L0X_REG_RESULT_INTERRUPT_STATUS       0x13
#define VL53L0X_REG_RESULT_RANGE_STATUS           0x14
#define VL53L0X_REG_FINAL_RANGE_MIN_COUNT_RATE    0x44
#define VL53L0X_REG_MSRC_CONFIG_CONTROL           0x60
#define VL53L0X_REG_GPIO_HV_MUX_ACTIVE_HIGH       0x84
#define VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP 0x89
#define VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES    0xB0
#define VL53L0X_REG_GLOBAL_CONFIG_REF_EN_START    0xB6
#define VL53L0X_REG_DYNAMIC_SPAD_NUM_REQUESTED    0x4E
#define VL53L0X_REG_DYNAMIC_SPAD_REF_EN_START     0x4F
#define VL53L0X_REG_IDENTIFICATION_MODEL_ID       0xC0
#define VL53L0X_EXPECTED_MODEL_ID                 0xEE
#define VL53L1X_REG_SOFT_RESET                    0x0000
#define VL53L1X_REG_VHV_TIMEOUT_MACROP_LOOP_BOUND 0x0008
#define VL53L1X_REG_GPIO_HV_MUX_CTRL              0x0030
#define VL53L1X_REG_GPIO_TIO_HV_STATUS            0x0031
#define VL53L1X_REG_SYSTEM_INTERRUPT_CLEAR        0x0086
#define VL53L1X_REG_SYSTEM_MODE_START             0x0087
#define VL53L1X_REG_RESULT_RANGE_STATUS           0x0089
#define VL53L1X_REG_RESULT_DISTANCE_MM            0x0096
#define VL53L1X_REG_FIRMWARE_SYSTEM_STATUS        0x00E5
#define VL53L1X_REG_MODEL_ID                      0x010F
#define VL53L1X_EXPECTED_MODEL_ID                 0xEACC
#define VL53L1X_DEFAULT_CONFIG_START              0x002D
typedef struct vl53lx_dev_s {
    i2c_master_dev_handle_t i2c_dev;
    uint8_t                 i2c_addr;
    gpio_num_t              xshut_pin;
    vl53lx_type_t           type;
    uint8_t                 stop_variable;
    uint8_t                 l1x_int_polarity;
} vl53lx_dev_s;
static const uint8_t s_l1x_default_config[] = {
    0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x02, 0x08, 0x00, 0x08, 0x10, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0xFF, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0B, 0x00, 0x00, 0x02, 0x0A, 0x21,
    0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x00, 0x00, 0x38, 0xFF, 0x01, 0x00, 0x08, 0x00,
    0x00, 0x01, 0xCC, 0x0F, 0x01, 0xF1, 0x0D, 0x01, 0x68, 0x00, 0x80, 0x08, 0xB8, 0x00, 0x00, 0x00,
    0x00, 0x0F, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0F, 0x0D, 0x0E, 0x0E, 0x00,
    0x00, 0x02, 0xC7, 0xFF, 0x9B, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
};
static const uint8_t s_l1x_range_status_map[24] = {
    255, 255, 255, 5, 2, 4, 1, 7, 3, 0, 255, 255, 9, 13, 255, 255, 255, 255, 10, 6, 255, 255, 11, 12,
};
static const struct {
    uint8_t reg;
    uint8_t val;
} s_l0x_tuning[] = {
    {0xFF, 0x01}, {0x00, 0x00}, {0xFF, 0x00}, {0x09, 0x00}, {0x10, 0x00}, {0x11, 0x00},
    {0x24, 0x01}, {0x25, 0xFF}, {0x75, 0x00}, {0xFF, 0x01}, {0x4E, 0x2C}, {0x48, 0x00},
    {0x30, 0x20}, {0xFF, 0x00}, {0x30, 0x09}, {0x54, 0x00}, {0x31, 0x04}, {0x32, 0x03},
    {0x40, 0x83}, {0x46, 0x25}, {0x60, 0x00}, {0x27, 0x00}, {0x50, 0x06}, {0x51, 0x00},
    {0x52, 0x96}, {0x56, 0x08}, {0x57, 0x30}, {0x61, 0x00}, {0x62, 0x00}, {0x64, 0x00},
    {0x65, 0x00}, {0x66, 0xA0}, {0xFF, 0x01}, {0x22, 0x32}, {0x47, 0x14}, {0x49, 0xFF},
    {0x4A, 0x00}, {0xFF, 0x00}, {0x7A, 0x0A}, {0x7B, 0x00}, {0x78, 0x21}, {0xFF, 0x01},
    {0x23, 0x34}, {0x42, 0x00}, {0x44, 0xFF}, {0x45, 0x26}, {0x46, 0x05}, {0x40, 0x40},
    {0x0E, 0x06}, {0x20, 0x1A}, {0x43, 0x40}, {0xFF, 0x00}, {0x34, 0x03}, {0x35, 0x44},
    {0xFF, 0x01}, {0x31, 0x04}, {0x4B, 0x09}, {0x4C, 0x05}, {0x4D, 0x04}, {0xFF, 0x00},
    {0x44, 0x00}, {0x45, 0x20}, {0x47, 0x08}, {0x48, 0x28}, {0x67, 0x00}, {0x70, 0x04},
    {0x71, 0x01}, {0x72, 0xFE}, {0x76, 0x00}, {0x77, 0x00}, {0xFF, 0x01}, {0x0D, 0x01},
    {0xFF, 0x00}, {0x80, 0x01}, {0x01, 0xF8}, {0xFF, 0x01}, {0x8E, 0x01}, {0x00, 0x01},
    {0xFF, 0x00}, {0x80, 0x00},
};
static esp_err_t l0x_write8(vl53lx_dev_s *dev, uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = {reg, val};
    return i2c_master_transmit(dev->i2c_dev, tx, sizeof(tx), I2C_TIMEOUT_MS);
}
static esp_err_t l0x_write16(vl53lx_dev_s *dev, uint8_t reg, uint16_t val)
{
    uint8_t tx[3] = {reg, (uint8_t)(val >> 8), (uint8_t)(val & 0xFF)};
    return i2c_master_transmit(dev->i2c_dev, tx, sizeof(tx), I2C_TIMEOUT_MS);
}
static esp_err_t l0x_read8(vl53lx_dev_s *dev, uint8_t reg, uint8_t *val)
{
    return i2c_master_transmit_receive(dev->i2c_dev, &reg, 1, val, 1, I2C_TIMEOUT_MS);
}
static esp_err_t l0x_read16(vl53lx_dev_s *dev, uint8_t reg, uint16_t *val)
{
    uint8_t rx[2] = {0};
    esp_err_t ret = i2c_master_transmit_receive(dev->i2c_dev, &reg, 1, rx, 2, I2C_TIMEOUT_MS);
    if (ret == ESP_OK) {
        *val = ((uint16_t)rx[0] << 8) | rx[1];
    }
    return ret;
}
static esp_err_t l0x_read_multi(vl53lx_dev_s *dev, uint8_t reg, uint8_t *buf, size_t len)
{
    return i2c_master_transmit_receive(dev->i2c_dev, &reg, 1, buf, len, I2C_TIMEOUT_MS);
}
static esp_err_t l0x_write_multi(vl53lx_dev_s *dev, uint8_t reg, const uint8_t *buf, size_t len)
{
    uint8_t tx[8];
    if (len > sizeof(tx) - 1) return ESP_ERR_INVALID_SIZE;
    tx[0] = reg;
    memcpy(&tx[1], buf, len);
    return i2c_master_transmit(dev->i2c_dev, tx, len + 1, I2C_TIMEOUT_MS);
}
static esp_err_t l1x_write8(vl53lx_dev_s *dev, uint16_t reg, uint8_t val)
{
    uint8_t tx[3] = {(uint8_t)(reg >> 8), (uint8_t)(reg & 0xFF), val};
    return i2c_master_transmit(dev->i2c_dev, tx, sizeof(tx), I2C_TIMEOUT_MS);
}
static esp_err_t l1x_read8(vl53lx_dev_s *dev, uint16_t reg, uint8_t *val)
{
    uint8_t addr[2] = {(uint8_t)(reg >> 8), (uint8_t)(reg & 0xFF)};
    return i2c_master_transmit_receive(dev->i2c_dev, addr, sizeof(addr), val, 1, I2C_TIMEOUT_MS);
}
static esp_err_t l1x_read16(vl53lx_dev_s *dev, uint16_t reg, uint16_t *val)
{
    uint8_t addr[2] = {(uint8_t)(reg >> 8), (uint8_t)(reg & 0xFF)};
    uint8_t rx[2] = {0};
    esp_err_t ret = i2c_master_transmit_receive(dev->i2c_dev, addr, sizeof(addr), rx, 2, I2C_TIMEOUT_MS);
    if (ret == ESP_OK) {
        *val = ((uint16_t)rx[0] << 8) | rx[1];
    }
    return ret;
}
static esp_err_t l1x_write_block(vl53lx_dev_s *dev, uint16_t reg, const uint8_t *data, size_t len)
{
    uint8_t *tx = malloc(len + 2);
    if (!tx) return ESP_ERR_NO_MEM;
    tx[0] = (uint8_t)(reg >> 8);
    tx[1] = (uint8_t)(reg & 0xFF);
    memcpy(&tx[2], data, len);
    esp_err_t ret = i2c_master_transmit(dev->i2c_dev, tx, len + 2, I2C_TIMEOUT_MS);
    free(tx);
    return ret;
}
static bool timed_out(int64_t start_us, uint32_t timeout_ms)
{
    return (esp_timer_get_time() - start_us) > ((int64_t)timeout_ms * 1000);
}
static esp_err_t l0x_get_spad_info(vl53lx_dev_s *dev, uint8_t *count, bool *is_aperture)
{
    uint8_t tmp = 0;
    l0x_write8(dev, 0x80, 0x01);
    l0x_write8(dev, 0xFF, 0x01);
    l0x_write8(dev, 0x00, 0x00);
    l0x_write8(dev, 0xFF, 0x06);
    l0x_read8(dev, 0x83, &tmp);
    l0x_write8(dev, 0x83, tmp | 0x04);
    l0x_write8(dev, 0xFF, 0x07);
    l0x_write8(dev, 0x81, 0x01);
    l0x_write8(dev, 0x80, 0x01);
    l0x_write8(dev, 0x94, 0x6B);
    l0x_write8(dev, 0x83, 0x00);
    int64_t start = esp_timer_get_time();
    do {
        if (l0x_read8(dev, 0x83, &tmp) != ESP_OK) return ESP_FAIL;
        if (timed_out(start, 500)) return ESP_ERR_TIMEOUT;
    } while (tmp == 0x00);
    l0x_write8(dev, 0x83, 0x01);
    l0x_read8(dev, 0x92, &tmp);
    *count = tmp & 0x7F;
    *is_aperture = ((tmp >> 7) & 0x01) != 0;
    l0x_write8(dev, 0x81, 0x00);
    l0x_write8(dev, 0xFF, 0x06);
    l0x_read8(dev, 0x83, &tmp);
    l0x_write8(dev, 0x83, tmp & ~0x04);
    l0x_write8(dev, 0xFF, 0x01);
    l0x_write8(dev, 0x00, 0x01);
    l0x_write8(dev, 0xFF, 0x00);
    l0x_write8(dev, 0x80, 0x00);
    return ESP_OK;
}
static esp_err_t l0x_single_ref_calibration(vl53lx_dev_s *dev, uint8_t vhv_init_byte)
{
    l0x_write8(dev, VL53L0X_REG_SYSRANGE_START, 0x01 | vhv_init_byte);
    int64_t start = esp_timer_get_time();
    uint8_t status = 0;
    do {
        if (l0x_read8(dev, VL53L0X_REG_RESULT_INTERRUPT_STATUS, &status) != ESP_OK) return ESP_FAIL;
        if (timed_out(start, 500)) return ESP_ERR_TIMEOUT;
    } while ((status & 0x07) == 0);
    l0x_write8(dev, VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    l0x_write8(dev, VL53L0X_REG_SYSRANGE_START, 0x00);
    return ESP_OK;
}
static esp_err_t l0x_init(vl53lx_dev_s *dev)
{
    uint8_t tmp = 0;
    l0x_read8(dev, VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP, &tmp);
    l0x_write8(dev, VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP, tmp | 0x01);
    l0x_write8(dev, 0x88, 0x00);
    l0x_write8(dev, 0x80, 0x01);
    l0x_write8(dev, 0xFF, 0x01);
    l0x_write8(dev, 0x00, 0x00);
    l0x_read8(dev, 0x91, &dev->stop_variable);
    l0x_write8(dev, 0x00, 0x01);
    l0x_write8(dev, 0xFF, 0x00);
    l0x_write8(dev, 0x80, 0x00);
    l0x_read8(dev, VL53L0X_REG_MSRC_CONFIG_CONTROL, &tmp);
    l0x_write8(dev, VL53L0X_REG_MSRC_CONFIG_CONTROL, tmp | 0x12);
    l0x_write16(dev, VL53L0X_REG_FINAL_RANGE_MIN_COUNT_RATE, (uint16_t)(0.25f * (1 << 7)));
    l0x_write8(dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0xFF);
    uint8_t spad_count = 0;
    bool spad_is_aperture = false;
    esp_err_t ret = l0x_get_spad_info(dev, &spad_count, &spad_is_aperture);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "VL53L0X SPAD info read failed: %s", esp_err_to_name(ret));
        return ret;
    }
    uint8_t ref_spad_map[6] = {0};
    l0x_read_multi(dev, VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES, ref_spad_map, sizeof(ref_spad_map));
    l0x_write8(dev, 0xFF, 0x01);
    l0x_write8(dev, VL53L0X_REG_DYNAMIC_SPAD_REF_EN_START, 0x00);
    l0x_write8(dev, VL53L0X_REG_DYNAMIC_SPAD_NUM_REQUESTED, 0x2C);
    l0x_write8(dev, 0xFF, 0x00);
    l0x_write8(dev, VL53L0X_REG_GLOBAL_CONFIG_REF_EN_START, 0xB4);
    uint8_t first_spad = spad_is_aperture ? 12 : 0;
    uint8_t spads_enabled = 0;
    for (uint8_t i = 0; i < 48; i++) {
        if (i < first_spad || spads_enabled == spad_count) {
            ref_spad_map[i / 8] &= (uint8_t)~(1 << (i % 8));
        } else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x01) {
            spads_enabled++;
        }
    }
    l0x_write_multi(dev, VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES, ref_spad_map, sizeof(ref_spad_map));
    for (size_t i = 0; i < sizeof(s_l0x_tuning) / sizeof(s_l0x_tuning[0]); i++) {
        l0x_write8(dev, s_l0x_tuning[i].reg, s_l0x_tuning[i].val);
    }
    l0x_write8(dev, VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
    l0x_read8(dev, VL53L0X_REG_GPIO_HV_MUX_ACTIVE_HIGH, &tmp);
    l0x_write8(dev, VL53L0X_REG_GPIO_HV_MUX_ACTIVE_HIGH, tmp & ~0x10);
    l0x_write8(dev, VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    l0x_write8(dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0xE8);
    l0x_write8(dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0x01);
    ret = l0x_single_ref_calibration(dev, 0x40);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "VL53L0X VHV calibration failed: %s", esp_err_to_name(ret));
        return ret;
    }
    l0x_write8(dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0x02);
    ret = l0x_single_ref_calibration(dev, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "VL53L0X phase calibration failed: %s", esp_err_to_name(ret));
        return ret;
    }
    l0x_write8(dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0xE8);
    return ESP_OK;
}
static esp_err_t l0x_read_distance(vl53lx_dev_s *dev, float *out_mm)
{
    l0x_write8(dev, 0x80, 0x01);
    l0x_write8(dev, 0xFF, 0x01);
    l0x_write8(dev, 0x00, 0x00);
    l0x_write8(dev, 0x91, dev->stop_variable);
    l0x_write8(dev, 0x00, 0x01);
    l0x_write8(dev, 0xFF, 0x00);
    l0x_write8(dev, 0x80, 0x00);
    esp_err_t ret = l0x_write8(dev, VL53L0X_REG_SYSRANGE_START, 0x01);
    if (ret != ESP_OK) return ret;
    uint8_t tmp = 0;
    int64_t start = esp_timer_get_time();
    do {
        if (l0x_read8(dev, VL53L0X_REG_SYSRANGE_START, &tmp) != ESP_OK) return ESP_FAIL;
        if (timed_out(start, 500)) return ESP_ERR_TIMEOUT;
    } while (tmp & 0x01);
    start = esp_timer_get_time();
    do {
        if (l0x_read8(dev, VL53L0X_REG_RESULT_INTERRUPT_STATUS, &tmp) != ESP_OK) return ESP_FAIL;
        if (timed_out(start, 500)) return ESP_ERR_TIMEOUT;
        vTaskDelay(pdMS_TO_TICKS(5));
    } while ((tmp & 0x07) == 0);
    uint16_t range = 0;
    ret = l0x_read16(dev, VL53L0X_REG_RESULT_RANGE_STATUS + 10, &range);
    l0x_write8(dev, VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    if (ret != ESP_OK) return ret;
    *out_mm = (float)range;
    if (range >= 8190) return ESP_ERR_NOT_FOUND;
    return ESP_OK;
}
static bool l1x_data_ready(vl53lx_dev_s *dev)
{
    uint8_t tmp = 0;
    if (l1x_read8(dev, VL53L1X_REG_GPIO_TIO_HV_STATUS, &tmp) != ESP_OK) return false;
    return (tmp & 0x01) == dev->l1x_int_polarity;
}
static esp_err_t l1x_wait_data_ready(vl53lx_dev_s *dev, uint32_t timeout_ms)
{
    int64_t start = esp_timer_get_time();
    while (!l1x_data_ready(dev)) {
        if (timed_out(start, timeout_ms)) return ESP_ERR_TIMEOUT;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    return ESP_OK;
}
static esp_err_t l1x_init(vl53lx_dev_s *dev)
{
    uint8_t state = 0;
    int64_t start = esp_timer_get_time();
    do {
        l1x_read8(dev, VL53L1X_REG_FIRMWARE_SYSTEM_STATUS, &state);
        if (timed_out(start, 500)) {
            ESP_LOGE(TAG, "VL53L1X boot timeout");
            return ESP_ERR_TIMEOUT;
        }
        if (!(state & 0x01)) vTaskDelay(pdMS_TO_TICKS(2));
    } while (!(state & 0x01));
    esp_err_t ret = l1x_write_block(dev, VL53L1X_DEFAULT_CONFIG_START, s_l1x_default_config,
                                    sizeof(s_l1x_default_config));
    if (ret != ESP_OK) return ret;
    uint8_t mux = 0;
    l1x_read8(dev, VL53L1X_REG_GPIO_HV_MUX_CTRL, &mux);
    dev->l1x_int_polarity = ((mux & 0x10) >> 4) ? 0 : 1;
    l1x_write8(dev, VL53L1X_REG_SYSTEM_MODE_START, 0x40);
    ret = l1x_wait_data_ready(dev, 1000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "VL53L1X first measurement timeout");
        return ret;
    }
    l1x_write8(dev, VL53L1X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    l1x_write8(dev, VL53L1X_REG_SYSTEM_MODE_START, 0x00);
    l1x_write8(dev, VL53L1X_REG_VHV_TIMEOUT_MACROP_LOOP_BOUND, 0x09);
    l1x_write8(dev, 0x000B, 0x00);
    l1x_write8(dev, VL53L1X_REG_SYSTEM_MODE_START, 0x40);
    return ESP_OK;
}
static esp_err_t l1x_read_distance(vl53lx_dev_s *dev, float *out_mm)
{
    esp_err_t ret = l1x_wait_data_ready(dev, 500);
    if (ret != ESP_OK) return ret;
    uint8_t raw_status = 0;
    uint16_t distance = 0;
    l1x_read8(dev, VL53L1X_REG_RESULT_RANGE_STATUS, &raw_status);
    ret = l1x_read16(dev, VL53L1X_REG_RESULT_DISTANCE_MM, &distance);
    l1x_write8(dev, VL53L1X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
    if (ret != ESP_OK) return ret;
    raw_status &= 0x1F;
    uint8_t status = (raw_status < sizeof(s_l1x_range_status_map)) ? s_l1x_range_status_map[raw_status] : 255;
    *out_mm = (float)distance;
    if (status != 0) {
        ESP_LOGD(TAG, "VL53L1X range status %u (raw %u)", status, raw_status);
        return ESP_ERR_NOT_FOUND;
    }
    return ESP_OK;
}
static vl53lx_type_t detect_type(vl53lx_dev_s *dev)
{
    uint8_t model8 = 0;
    if (l0x_read8(dev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &model8) == ESP_OK &&
        model8 == VL53L0X_EXPECTED_MODEL_ID) {
        return VL53LX_TYPE_VL53L0X;
    }
    uint16_t model16 = 0;
    if (l1x_read16(dev, VL53L1X_REG_MODEL_ID, &model16) == ESP_OK && model16 == VL53L1X_EXPECTED_MODEL_ID) {
        return VL53LX_TYPE_VL53L1X;
    }
    return VL53LX_TYPE_UNKNOWN;
}
esp_err_t vl53lx_init(const vl53lx_config_t *config, vl53lx_handle_t *out_handle)
{
    if (!config || !config->i2c_bus || !out_handle) {
        return ESP_ERR_INVALID_ARG;
    }
    *out_handle = NULL;
    vl53lx_dev_s *dev = calloc(1, sizeof(vl53lx_dev_s));
    if (!dev) {
        return ESP_ERR_NO_MEM;
    }
    dev->i2c_addr = config->i2c_addr ? config->i2c_addr : VL53LX_DEFAULT_I2C_ADDR;
    dev->xshut_pin = config->xshut_pin;
    if (GPIO_IS_VALID_OUTPUT_GPIO(dev->xshut_pin)) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << dev->xshut_pin),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&io_conf);
        gpio_set_level(dev->xshut_pin, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(dev->xshut_pin, 1);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    esp_err_t ret = i2c_master_probe(config->i2c_bus, dev->i2c_addr, 50);
    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "No device at I2C address 0x%02X", dev->i2c_addr);
        free(dev);
        return ESP_ERR_NOT_FOUND;
    }
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = dev->i2c_addr,
        .scl_speed_hz = 400000,
    };
    ret = i2c_master_bus_add_device(config->i2c_bus, &dev_cfg, &dev->i2c_dev);
    if (ret != ESP_OK) {
        free(dev);
        return ret;
    }
    dev->type = detect_type(dev);
    if (dev->type == VL53LX_TYPE_VL53L0X) {
        ret = l0x_init(dev);
    } else if (dev->type == VL53LX_TYPE_VL53L1X) {
        ret = l1x_init(dev);
    } else {
        ESP_LOGW(TAG, "Device at 0x%02X is not a VL53L0X/VL53L1X", dev->i2c_addr);
        ret = ESP_ERR_NOT_FOUND;
    }
    if (ret != ESP_OK) {
        i2c_master_bus_rm_device(dev->i2c_dev);
        free(dev);
        return ret;
    }
    ESP_LOGI(TAG, "%s initialized at I2C address 0x%02X", vl53lx_type_name(dev->type), dev->i2c_addr);
    *out_handle = dev;
    return ESP_OK;
}
esp_err_t vl53lx_deinit(vl53lx_handle_t handle)
{
    if (!handle) return ESP_ERR_INVALID_ARG;
    if (handle->type == VL53LX_TYPE_VL53L1X) {
        l1x_write8(handle, VL53L1X_REG_SYSTEM_MODE_START, 0x00);
    }
    if (GPIO_IS_VALID_OUTPUT_GPIO(handle->xshut_pin)) {
        gpio_set_level(handle->xshut_pin, 0);
    }
    i2c_master_bus_rm_device(handle->i2c_dev);
    free(handle);
    return ESP_OK;
}
esp_err_t vl53lx_read_distance_mm(vl53lx_handle_t handle, float *out_distance_mm)
{
    if (!handle || !out_distance_mm) return ESP_ERR_INVALID_ARG;
    if (handle->type == VL53LX_TYPE_VL53L0X) {
        return l0x_read_distance(handle, out_distance_mm);
    }
    if (handle->type == VL53LX_TYPE_VL53L1X) {
        return l1x_read_distance(handle, out_distance_mm);
    }
    return ESP_ERR_INVALID_STATE;
}
vl53lx_type_t vl53lx_get_type(vl53lx_handle_t handle)
{
    return handle ? handle->type : VL53LX_TYPE_UNKNOWN;
}
const char *vl53lx_type_name(vl53lx_type_t type)
{
    switch (type) {
        case VL53LX_TYPE_VL53L0X: return "VL53L0X";
        case VL53LX_TYPE_VL53L1X: return "VL53L1X";
        default:                  return "unknown";
    }
}
