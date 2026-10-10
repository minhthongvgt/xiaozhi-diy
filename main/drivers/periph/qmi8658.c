#include "drivers/periph/qmi8658.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>
#include <stdlib.h>
#define TAG "QMI8658"
#define I2C_TIMEOUT_MS 100
#define REG_WHO_AM_I    0x00
#define REG_CTRL1       0x02
#define REG_CTRL2       0x03
#define REG_CTRL3       0x04
#define REG_CTRL7       0x08
#define REG_STATUS0     0x2E
#define REG_TEMP_L      0x33
#define REG_ACCEL_L     0x35
#define REG_GYRO_L      0x3B
#define WHO_AM_I_VALUE  0x05
#define CTRL1_ADDR_AI   0x40
#define ACCEL_FS_2G     0x00
#define ACCEL_ODR_1KHZ  0x03
#define GYRO_FS_512DPS  0x50
#define GYRO_ODR_1KHZ   0x03
#define CTRL7_ACC_GYRO_EN 0x03
#define STATUS_ACC_DA   0x01
#define STATUS_GYR_DA   0x02
#define ACCEL_LSB_PER_G   16384.0f
#define GYRO_LSB_PER_DPS  64.0f
#define TEMP_LSB_PER_C    256.0f
typedef struct qmi8658_dev_s {
    i2c_master_dev_handle_t i2c_dev;
    uint8_t                 addr;
} qmi8658_dev_s;
static esp_err_t write_reg(qmi8658_dev_s *dev, uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = {reg, val};
    return i2c_master_transmit(dev->i2c_dev, tx, sizeof(tx), I2C_TIMEOUT_MS);
}
static esp_err_t read_regs(qmi8658_dev_s *dev, uint8_t reg, uint8_t *buf, size_t len)
{
    return i2c_master_transmit_receive(dev->i2c_dev, &reg, 1, buf, len, I2C_TIMEOUT_MS);
}
static int16_t le16(const uint8_t *p)
{
    return (int16_t)((uint16_t)p[1] << 8 | p[0]);
}
esp_err_t qmi8658_init(i2c_master_bus_handle_t i2c_bus, qmi8658_handle_t *out_handle)
{
    if (!i2c_bus || !out_handle) return ESP_ERR_INVALID_ARG;
    *out_handle = NULL;
    const uint8_t candidates[2] = {QMI8658_I2C_ADDR_PRIMARY, QMI8658_I2C_ADDR_SECONDARY};
    for (int i = 0; i < 2; i++) {
        if (i2c_master_probe(i2c_bus, candidates[i], 50) != ESP_OK) continue;
        qmi8658_dev_s *dev = calloc(1, sizeof(qmi8658_dev_s));
        if (!dev) return ESP_ERR_NO_MEM;
        dev->addr = candidates[i];
        i2c_device_config_t cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = dev->addr,
            .scl_speed_hz = 400000,
        };
        if (i2c_master_bus_add_device(i2c_bus, &cfg, &dev->i2c_dev) != ESP_OK) {
            free(dev);
            continue;
        }
        uint8_t id = 0;
        if (read_regs(dev, REG_WHO_AM_I, &id, 1) != ESP_OK || id != WHO_AM_I_VALUE) {
            ESP_LOGD(TAG, "Device at 0x%02X is not QMI8658 (id 0x%02X)", dev->addr, id);
            qmi8658_deinit(dev);
            continue;
        }
        write_reg(dev, REG_CTRL1, CTRL1_ADDR_AI);
        write_reg(dev, REG_CTRL2, (uint8_t)(ACCEL_FS_2G | ACCEL_ODR_1KHZ));
        write_reg(dev, REG_CTRL3, (uint8_t)(GYRO_FS_512DPS | GYRO_ODR_1KHZ));
        write_reg(dev, REG_CTRL7, CTRL7_ACC_GYRO_EN);
        ESP_LOGI(TAG, "QMI8658 IMU initialized at 0x%02X (accel +/-2g, gyro +/-512 dps)", dev->addr);
        *out_handle = dev;
        return ESP_OK;
    }
    return ESP_ERR_NOT_FOUND;
}
void qmi8658_deinit(qmi8658_handle_t handle)
{
    if (!handle) return;
    if (handle->i2c_dev) i2c_master_bus_rm_device(handle->i2c_dev);
    free(handle);
}
esp_err_t qmi8658_read(qmi8658_handle_t handle, qmi8658_sample_t *out_sample)
{
    if (!handle || !out_sample) return ESP_ERR_INVALID_ARG;
    uint8_t status = 0;
    esp_err_t ret = read_regs(handle, REG_STATUS0, &status, 1);
    if (ret != ESP_OK) return ret;
    if (!(status & (STATUS_ACC_DA | STATUS_GYR_DA))) return ESP_ERR_NOT_FINISHED;
    uint8_t raw[14] = {0};
    ret = read_regs(handle, REG_TEMP_L, raw, sizeof(raw));
    if (ret != ESP_OK) return ret;
    out_sample->temperature_c = le16(&raw[0]) / TEMP_LSB_PER_C;
    for (int i = 0; i < 3; i++) {
        out_sample->accel_g[i] = le16(&raw[2 + i * 2]) / ACCEL_LSB_PER_G;
    }
    for (int i = 0; i < 3; i++) {
        out_sample->gyro_dps[i] = le16(&raw[8 + i * 2]) / GYRO_LSB_PER_DPS;
    }
    float ax = out_sample->accel_g[0], ay = out_sample->accel_g[1], az = out_sample->accel_g[2];
    out_sample->pitch_deg = atan2f(-ax, sqrtf(ay * ay + az * az)) * 180.0f / (float)M_PI;
    out_sample->roll_deg = atan2f(ay, az) * 180.0f / (float)M_PI;
    return ESP_OK;
}
