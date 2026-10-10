#include "drivers/sensor/apds9960.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <stdlib.h>
#include <string.h>
#define TAG "APDS9960"
#define I2C_TIMEOUT_MS 100
#define APDS9960_REG_ENABLE    0x80
#define APDS9960_REG_ATIME     0x81
#define APDS9960_REG_WTIME     0x83
#define APDS9960_REG_PERS      0x8C
#define APDS9960_REG_CONFIG1   0x8D
#define APDS9960_REG_PPULSE    0x8E
#define APDS9960_REG_CONTROL   0x8F
#define APDS9960_REG_CONFIG2   0x90
#define APDS9960_REG_ID        0x92
#define APDS9960_REG_STATUS    0x93
#define APDS9960_REG_CDATAL    0x94
#define APDS9960_REG_PDATA     0x9C
#define APDS9960_REG_POFFSET_UR 0x9D
#define APDS9960_REG_POFFSET_DL 0x9E
#define APDS9960_REG_CONFIG3   0x9F
#define APDS9960_REG_GPENTH    0xA0
#define APDS9960_REG_GEXTH     0xA1
#define APDS9960_REG_GCONF1    0xA2
#define APDS9960_REG_GCONF2    0xA3
#define APDS9960_REG_GOFFSET_U 0xA4
#define APDS9960_REG_GOFFSET_D 0xA5
#define APDS9960_REG_GPULSE    0xA6
#define APDS9960_REG_GOFFSET_L 0xA7
#define APDS9960_REG_GOFFSET_R 0xA9
#define APDS9960_REG_GCONF3    0xAA
#define APDS9960_REG_GCONF4    0xAB
#define APDS9960_REG_GFLVL     0xAE
#define APDS9960_REG_GSTATUS   0xAF
#define APDS9960_REG_AICLEAR   0xE7
#define APDS9960_REG_GFIFO_U   0xFC
#define APDS9960_ENABLE_PON 0x01
#define APDS9960_ENABLE_AEN 0x02
#define APDS9960_ENABLE_PEN 0x04
#define APDS9960_ENABLE_WEN 0x08
#define APDS9960_ENABLE_GEN 0x40
#define APDS9960_STATUS_AVALID 0x01
#define APDS9960_STATUS_PVALID 0x02
#define APDS9960_GESTURE_THRESHOLD 10
#define APDS9960_GESTURE_SENSITIVITY 40
typedef struct apds9960_dev_s {
    i2c_master_dev_handle_t i2c_dev;
    gpio_num_t              int_pin;
    SemaphoreHandle_t       lock;
} apds9960_dev_s;
static esp_err_t write_reg(apds9960_dev_s *dev, uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = {reg, val};
    return i2c_master_transmit(dev->i2c_dev, tx, sizeof(tx), I2C_TIMEOUT_MS);
}
static esp_err_t read_regs(apds9960_dev_s *dev, uint8_t reg, uint8_t *buf, size_t len)
{
    return i2c_master_transmit_receive(dev->i2c_dev, &reg, 1, buf, len, I2C_TIMEOUT_MS);
}
static esp_err_t read_reg(apds9960_dev_s *dev, uint8_t reg, uint8_t *val)
{
    return read_regs(dev, reg, val, 1);
}
static bool timed_out(int64_t start_us, uint32_t timeout_ms)
{
    return (esp_timer_get_time() - start_us) > ((int64_t)timeout_ms * 1000);
}
static esp_err_t wait_status(apds9960_dev_s *dev, uint8_t mask, uint32_t timeout_ms)
{
    int64_t start = esp_timer_get_time();
    uint8_t status = 0;
    while (true) {
        esp_err_t ret = read_reg(dev, APDS9960_REG_STATUS, &status);
        if (ret != ESP_OK) return ret;
        if (status & mask) return ESP_OK;
        if (timed_out(start, timeout_ms)) return ESP_ERR_TIMEOUT;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
static esp_err_t configure(apds9960_dev_s *dev)
{
    static const struct {
        uint8_t reg;
        uint8_t val;
    } s_init[] = {
        {APDS9960_REG_ENABLE, 0x00},
        {APDS9960_REG_ATIME, 0xDB},
        {APDS9960_REG_WTIME, 0xF6},
        {APDS9960_REG_PPULSE, 0x87},
        {APDS9960_REG_POFFSET_UR, 0x00},
        {APDS9960_REG_POFFSET_DL, 0x00},
        {APDS9960_REG_CONFIG1, 0x60},
        {APDS9960_REG_CONTROL, 0x09},
        {APDS9960_REG_CONFIG2, 0x01},
        {APDS9960_REG_CONFIG3, 0x00},
        {APDS9960_REG_PERS, 0x11},
        {APDS9960_REG_GPENTH, 40},
        {APDS9960_REG_GEXTH, 30},
        {APDS9960_REG_GCONF1, 0x40},
        {APDS9960_REG_GCONF2, 0x41},
        {APDS9960_REG_GOFFSET_U, 0x00},
        {APDS9960_REG_GOFFSET_D, 0x00},
        {APDS9960_REG_GOFFSET_L, 0x00},
        {APDS9960_REG_GOFFSET_R, 0x00},
        {APDS9960_REG_GPULSE, 0xC9},
        {APDS9960_REG_GCONF3, 0x00},
        {APDS9960_REG_GCONF4, 0x00},
    };
    for (size_t i = 0; i < sizeof(s_init) / sizeof(s_init[0]); i++) {
        esp_err_t ret = write_reg(dev, s_init[i].reg, s_init[i].val);
        if (ret != ESP_OK) return ret;
    }
    uint8_t clear_cmd = APDS9960_REG_AICLEAR;
    i2c_master_transmit(dev->i2c_dev, &clear_cmd, 1, I2C_TIMEOUT_MS);
    return write_reg(dev, APDS9960_REG_ENABLE, APDS9960_ENABLE_PON | APDS9960_ENABLE_AEN | APDS9960_ENABLE_PEN);
}
esp_err_t apds9960_init(i2c_master_bus_handle_t i2c_bus, gpio_num_t int_pin, apds9960_handle_t *out_handle)
{
    if (!i2c_bus || !out_handle) return ESP_ERR_INVALID_ARG;
    *out_handle = NULL;
    if (i2c_master_probe(i2c_bus, APDS9960_I2C_ADDR, 50) != ESP_OK) {
        return ESP_ERR_NOT_FOUND;
    }
    apds9960_dev_s *dev = calloc(1, sizeof(apds9960_dev_s));
    if (!dev) return ESP_ERR_NO_MEM;
    dev->int_pin = int_pin;
    dev->lock = xSemaphoreCreateMutex();
    if (!dev->lock) {
        free(dev);
        return ESP_ERR_NO_MEM;
    }
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = APDS9960_I2C_ADDR,
        .scl_speed_hz = 400000,
    };
    esp_err_t ret = i2c_master_bus_add_device(i2c_bus, &dev_cfg, &dev->i2c_dev);
    if (ret != ESP_OK) {
        vSemaphoreDelete(dev->lock);
        free(dev);
        return ret;
    }
    uint8_t id = 0;
    ret = read_reg(dev, APDS9960_REG_ID, &id);
    if (ret != ESP_OK || (id != 0xAB && id != 0xA8 && id != 0x9C)) {
        ESP_LOGW(TAG, "Unexpected chip ID 0x%02X at 0x%02X (err: %s)", id, APDS9960_I2C_ADDR, esp_err_to_name(ret));
        apds9960_deinit(dev);
        return ESP_ERR_NOT_FOUND;
    }
    if (GPIO_IS_VALID_GPIO(int_pin)) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << int_pin),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&io_conf);
    }
    ret = configure(dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Configuration failed: %s", esp_err_to_name(ret));
        apds9960_deinit(dev);
        return ret;
    }
    ESP_LOGI(TAG, "APDS-9960 initialized (ID 0x%02X)", id);
    *out_handle = dev;
    return ESP_OK;
}
void apds9960_deinit(apds9960_handle_t handle)
{
    if (!handle) return;
    if (handle->i2c_dev) {
        write_reg(handle, APDS9960_REG_ENABLE, 0x00);
        i2c_master_bus_rm_device(handle->i2c_dev);
    }
    if (handle->lock) vSemaphoreDelete(handle->lock);
    free(handle);
}
esp_err_t apds9960_read_proximity(apds9960_handle_t handle, uint8_t *out_proximity)
{
    if (!handle || !out_proximity) return ESP_ERR_INVALID_ARG;
    if (xSemaphoreTake(handle->lock, pdMS_TO_TICKS(1000)) != pdTRUE) return ESP_ERR_TIMEOUT;
    esp_err_t ret = wait_status(handle, APDS9960_STATUS_PVALID, 200);
    if (ret == ESP_OK) {
        ret = read_reg(handle, APDS9960_REG_PDATA, out_proximity);
    }
    xSemaphoreGive(handle->lock);
    return ret;
}
esp_err_t apds9960_read_color(apds9960_handle_t handle, apds9960_color_t *out_color)
{
    if (!handle || !out_color) return ESP_ERR_INVALID_ARG;
    if (xSemaphoreTake(handle->lock, pdMS_TO_TICKS(1000)) != pdTRUE) return ESP_ERR_TIMEOUT;
    uint8_t raw[8] = {0};
    esp_err_t ret = wait_status(handle, APDS9960_STATUS_AVALID, 300);
    if (ret == ESP_OK) {
        ret = read_regs(handle, APDS9960_REG_CDATAL, raw, sizeof(raw));
    }
    xSemaphoreGive(handle->lock);
    if (ret != ESP_OK) return ret;
    out_color->clear = (uint16_t)(raw[1] << 8 | raw[0]);
    out_color->red = (uint16_t)(raw[3] << 8 | raw[2]);
    out_color->green = (uint16_t)(raw[5] << 8 | raw[4]);
    out_color->blue = (uint16_t)(raw[7] << 8 | raw[6]);
    return ESP_OK;
}
float apds9960_color_to_lux(const apds9960_color_t *color)
{
    if (!color) return 0.0f;
    float lux = (-0.32466f * color->red) + (1.57837f * color->green) + (-0.73191f * color->blue);
    return lux < 0.0f ? 0.0f : lux;
}
static apds9960_gesture_t decode_gesture(const uint8_t first[4], const uint8_t last[4])
{
    int first_ud = ((int)first[0] - first[1]) * 100 / ((int)first[0] + first[1]);
    int first_lr = ((int)first[2] - first[3]) * 100 / ((int)first[2] + first[3]);
    int last_ud = ((int)last[0] - last[1]) * 100 / ((int)last[0] + last[1]);
    int last_lr = ((int)last[2] - last[3]) * 100 / ((int)last[2] + last[3]);
    int ud_delta = last_ud - first_ud;
    int lr_delta = last_lr - first_lr;
    int abs_ud = abs(ud_delta);
    int abs_lr = abs(lr_delta);
    if (abs_ud < APDS9960_GESTURE_SENSITIVITY && abs_lr < APDS9960_GESTURE_SENSITIVITY) {
        return APDS9960_GESTURE_NONE;
    }
    if (abs_ud >= abs_lr) {
        return ud_delta > 0 ? APDS9960_GESTURE_DOWN : APDS9960_GESTURE_UP;
    }
    return lr_delta > 0 ? APDS9960_GESTURE_RIGHT : APDS9960_GESTURE_LEFT;
}
esp_err_t apds9960_read_gesture(apds9960_handle_t handle, uint32_t timeout_ms, apds9960_gesture_t *out_gesture)
{
    if (!handle || !out_gesture) return ESP_ERR_INVALID_ARG;
    *out_gesture = APDS9960_GESTURE_NONE;
    if (xSemaphoreTake(handle->lock, pdMS_TO_TICKS(1000)) != pdTRUE) return ESP_ERR_TIMEOUT;
    write_reg(handle, APDS9960_REG_GCONF4, 0x04);
    esp_err_t ret = write_reg(handle, APDS9960_REG_ENABLE,
                              APDS9960_ENABLE_PON | APDS9960_ENABLE_PEN | APDS9960_ENABLE_WEN | APDS9960_ENABLE_GEN);
    uint8_t first[4] = {0};
    uint8_t last[4] = {0};
    bool have_first = false;
    bool collecting = false;
    uint8_t fifo[128];
    int64_t start = esp_timer_get_time();
    while (ret == ESP_OK && !timed_out(start, timeout_ms)) {
        uint8_t gstatus = 0;
        ret = read_reg(handle, APDS9960_REG_GSTATUS, &gstatus);
        if (ret != ESP_OK) break;
        if (gstatus & 0x01) {
            uint8_t level = 0;
            read_reg(handle, APDS9960_REG_GFLVL, &level);
            if (level > 32) level = 32;
            if (level > 0 && read_regs(handle, APDS9960_REG_GFIFO_U, fifo, (size_t)level * 4) == ESP_OK) {
                for (int i = 0; i < level; i++) {
                    const uint8_t *d = &fifo[i * 4];
                    if (d[0] > APDS9960_GESTURE_THRESHOLD && d[1] > APDS9960_GESTURE_THRESHOLD &&
                        d[2] > APDS9960_GESTURE_THRESHOLD && d[3] > APDS9960_GESTURE_THRESHOLD) {
                        if (!have_first) {
                            memcpy(first, d, 4);
                            have_first = true;
                        }
                        memcpy(last, d, 4);
                    }
                }
                collecting = true;
            }
        } else if (collecting) {
            uint8_t gconf4 = 0;
            read_reg(handle, APDS9960_REG_GCONF4, &gconf4);
            if (!(gconf4 & 0x01)) break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    write_reg(handle, APDS9960_REG_GCONF4, 0x04);
    write_reg(handle, APDS9960_REG_ENABLE, APDS9960_ENABLE_PON | APDS9960_ENABLE_AEN | APDS9960_ENABLE_PEN);
    xSemaphoreGive(handle->lock);
    if (ret != ESP_OK) return ret;
    if (have_first) {
        *out_gesture = decode_gesture(first, last);
    }
    return ESP_OK;
}
const char *apds9960_gesture_name(apds9960_gesture_t gesture)
{
    switch (gesture) {
        case APDS9960_GESTURE_UP:    return "up";
        case APDS9960_GESTURE_DOWN:  return "down";
        case APDS9960_GESTURE_LEFT:  return "left";
        case APDS9960_GESTURE_RIGHT: return "right";
        default:                     return "none";
    }
}
