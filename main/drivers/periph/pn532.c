#include "drivers/periph/pn532.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <stdlib.h>
#include <string.h>
#define TAG "PN532"
#define I2C_TIMEOUT_MS 100
#define PN532_HOST_TO_PN532 0xD4
#define PN532_PN532_TO_HOST 0xD5
#define PN532_CMD_GET_FIRMWARE_VERSION 0x02
#define PN532_CMD_SAM_CONFIGURATION    0x14
#define PN532_CMD_RF_CONFIGURATION     0x32
#define PN532_CMD_IN_LIST_PASSIVE      0x4A
#define PN532_MAX_FRAME 64
typedef struct pn532_dev_s {
    i2c_master_dev_handle_t i2c_dev;
    gpio_num_t              irq_pin;
    gpio_num_t              rst_pin;
    SemaphoreHandle_t       lock;
} pn532_dev_s;
static const uint8_t s_ack_frame[6] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
static bool timed_out(int64_t start_us, uint32_t timeout_ms)
{
    return (esp_timer_get_time() - start_us) > ((int64_t)timeout_ms * 1000);
}
static esp_err_t wait_ready(pn532_dev_s *dev, uint32_t timeout_ms)
{
    int64_t start = esp_timer_get_time();
    while (true) {
        if (GPIO_IS_VALID_GPIO(dev->irq_pin)) {
            if (gpio_get_level(dev->irq_pin) == 0) return ESP_OK;
        } else {
            uint8_t status = 0;
            if (i2c_master_receive(dev->i2c_dev, &status, 1, I2C_TIMEOUT_MS) == ESP_OK && (status & 0x01)) {
                return ESP_OK;
            }
        }
        if (timed_out(start, timeout_ms)) return ESP_ERR_TIMEOUT;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
static esp_err_t send_command(pn532_dev_s *dev, uint8_t cmd, const uint8_t *params, size_t plen)
{
    uint8_t frame[PN532_MAX_FRAME];
    size_t len = plen + 2;
    if (len + 7 > sizeof(frame)) return ESP_ERR_INVALID_SIZE;
    size_t i = 0;
    frame[i++] = 0x00;
    frame[i++] = 0x00;
    frame[i++] = 0xFF;
    frame[i++] = (uint8_t)len;
    frame[i++] = (uint8_t)(~len + 1);
    frame[i++] = PN532_HOST_TO_PN532;
    frame[i++] = cmd;
    uint8_t sum = (uint8_t)(PN532_HOST_TO_PN532 + cmd);
    for (size_t p = 0; p < plen; p++) {
        frame[i++] = params[p];
        sum = (uint8_t)(sum + params[p]);
    }
    frame[i++] = (uint8_t)(~sum + 1);
    frame[i++] = 0x00;
    return i2c_master_transmit(dev->i2c_dev, frame, i, I2C_TIMEOUT_MS);
}
static esp_err_t read_ack(pn532_dev_s *dev)
{
    esp_err_t ret = wait_ready(dev, 100);
    if (ret != ESP_OK) return ret;
    uint8_t rx[7] = {0};
    ret = i2c_master_receive(dev->i2c_dev, rx, sizeof(rx), I2C_TIMEOUT_MS);
    if (ret != ESP_OK) return ret;
    return memcmp(&rx[1], s_ack_frame, sizeof(s_ack_frame)) == 0 ? ESP_OK : ESP_ERR_INVALID_RESPONSE;
}
static esp_err_t read_response(pn532_dev_s *dev, uint8_t cmd, uint8_t *data, size_t max_len, size_t *out_len,
                               uint32_t timeout_ms)
{
    esp_err_t ret = wait_ready(dev, timeout_ms);
    if (ret != ESP_OK) return ret;
    uint8_t rx[PN532_MAX_FRAME] = {0};
    size_t want = max_len + 10;
    if (want > sizeof(rx)) want = sizeof(rx);
    ret = i2c_master_receive(dev->i2c_dev, rx, want, I2C_TIMEOUT_MS);
    if (ret != ESP_OK) return ret;
    if (rx[1] != 0x00 || rx[2] != 0x00 || rx[3] != 0xFF) return ESP_ERR_INVALID_RESPONSE;
    uint8_t len = rx[4];
    if ((uint8_t)(len + rx[5]) != 0 || len < 2) return ESP_ERR_INVALID_RESPONSE;
    if (rx[6] != PN532_PN532_TO_HOST || rx[7] != (uint8_t)(cmd + 1)) return ESP_ERR_INVALID_RESPONSE;
    size_t data_len = (size_t)len - 2;
    if (data_len > max_len || 8 + data_len + 1 > want) return ESP_ERR_INVALID_SIZE;
    uint8_t sum = (uint8_t)(rx[6] + rx[7]);
    for (size_t i = 0; i < data_len; i++) {
        data[i] = rx[8 + i];
        sum = (uint8_t)(sum + rx[8 + i]);
    }
    if ((uint8_t)(sum + rx[8 + data_len]) != 0) return ESP_ERR_INVALID_CRC;
    if (out_len) *out_len = data_len;
    return ESP_OK;
}
static esp_err_t transceive(pn532_dev_s *dev, uint8_t cmd, const uint8_t *params, size_t plen, uint8_t *resp,
                            size_t resp_max, size_t *resp_len, uint32_t timeout_ms)
{
    esp_err_t ret = send_command(dev, cmd, params, plen);
    if (ret != ESP_OK) return ret;
    ret = read_ack(dev);
    if (ret != ESP_OK) return ret;
    return read_response(dev, cmd, resp, resp_max, resp_len, timeout_ms);
}
esp_err_t pn532_get_firmware_version(pn532_handle_t handle, uint32_t *out_version)
{
    if (!handle || !out_version) return ESP_ERR_INVALID_ARG;
    uint8_t resp[4] = {0};
    size_t len = 0;
    if (xSemaphoreTake(handle->lock, pdMS_TO_TICKS(1000)) != pdTRUE) return ESP_ERR_TIMEOUT;
    esp_err_t ret = transceive(handle, PN532_CMD_GET_FIRMWARE_VERSION, NULL, 0, resp, sizeof(resp), &len, 200);
    xSemaphoreGive(handle->lock);
    if (ret != ESP_OK) return ret;
    if (len < 4) return ESP_ERR_INVALID_RESPONSE;
    *out_version = ((uint32_t)resp[0] << 24) | ((uint32_t)resp[1] << 16) | ((uint32_t)resp[2] << 8) | resp[3];
    return ESP_OK;
}
esp_err_t pn532_init(const pn532_config_t *config, pn532_handle_t *out_handle)
{
    if (!config || !config->i2c_bus || !out_handle) return ESP_ERR_INVALID_ARG;
    *out_handle = NULL;
    pn532_dev_s *dev = calloc(1, sizeof(pn532_dev_s));
    if (!dev) return ESP_ERR_NO_MEM;
    dev->irq_pin = config->irq_pin;
    dev->rst_pin = config->rst_pin;
    dev->lock = xSemaphoreCreateMutex();
    if (!dev->lock) {
        free(dev);
        return ESP_ERR_NO_MEM;
    }
    if (GPIO_IS_VALID_OUTPUT_GPIO(dev->rst_pin)) {
        gpio_config_t rst_conf = {
            .pin_bit_mask = (1ULL << dev->rst_pin),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&rst_conf);
        gpio_set_level(dev->rst_pin, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
        gpio_set_level(dev->rst_pin, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    if (GPIO_IS_VALID_GPIO(dev->irq_pin)) {
        gpio_config_t irq_conf = {
            .pin_bit_mask = (1ULL << dev->irq_pin),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&irq_conf);
    }
    esp_err_t ret = ESP_FAIL;
    for (int attempt = 0; attempt < 3 && ret != ESP_OK; attempt++) {
        ret = i2c_master_probe(config->i2c_bus, PN532_I2C_ADDR, 50);
        if (ret != ESP_OK) vTaskDelay(pdMS_TO_TICKS(50));
    }
    if (ret != ESP_OK) {
        vSemaphoreDelete(dev->lock);
        free(dev);
        return ESP_ERR_NOT_FOUND;
    }
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PN532_I2C_ADDR,
        .scl_speed_hz = 100000,
    };
    ret = i2c_master_bus_add_device(config->i2c_bus, &cfg, &dev->i2c_dev);
    if (ret != ESP_OK) {
        vSemaphoreDelete(dev->lock);
        free(dev);
        return ret;
    }
    uint32_t version = 0;
    ret = ESP_FAIL;
    for (int attempt = 0; attempt < 3 && ret != ESP_OK; attempt++) {
        ret = pn532_get_firmware_version(dev, &version);
        if (ret != ESP_OK) vTaskDelay(pdMS_TO_TICKS(50));
    }
    if (ret != ESP_OK || ((version >> 24) & 0xFF) != 0x32) {
        ESP_LOGD(TAG, "PN532 not detected (err: %s, version 0x%08lX)", esp_err_to_name(ret), (unsigned long)version);
        pn532_deinit(dev);
        return ESP_ERR_NOT_FOUND;
    }
    uint8_t sam[3] = {0x01, 0x14, (uint8_t)(GPIO_IS_VALID_GPIO(dev->irq_pin) ? 0x01 : 0x00)};
    uint8_t resp[4];
    size_t len = 0;
    ret = transceive(dev, PN532_CMD_SAM_CONFIGURATION, sam, sizeof(sam), resp, sizeof(resp), &len, 200);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SAMConfiguration failed: %s", esp_err_to_name(ret));
        pn532_deinit(dev);
        return ret;
    }
    uint8_t retries[4] = {0x05, 0xFF, 0x01, 0x02};
    transceive(dev, PN532_CMD_RF_CONFIGURATION, retries, sizeof(retries), resp, sizeof(resp), &len, 200);
    ESP_LOGI(TAG, "PN532 initialized (firmware %u.%u)", (unsigned)((version >> 16) & 0xFF),
             (unsigned)((version >> 8) & 0xFF));
    *out_handle = dev;
    return ESP_OK;
}
void pn532_deinit(pn532_handle_t handle)
{
    if (!handle) return;
    if (handle->i2c_dev) i2c_master_bus_rm_device(handle->i2c_dev);
    if (handle->lock) vSemaphoreDelete(handle->lock);
    free(handle);
}
esp_err_t pn532_read_passive_target(pn532_handle_t handle, uint32_t timeout_ms, pn532_card_t *out_card)
{
    if (!handle || !out_card) return ESP_ERR_INVALID_ARG;
    memset(out_card, 0, sizeof(*out_card));
    if (xSemaphoreTake(handle->lock, pdMS_TO_TICKS(1000)) != pdTRUE) return ESP_ERR_TIMEOUT;
    uint8_t params[2] = {0x01, 0x00};
    uint8_t resp[6 + PN532_MAX_UID_LEN];
    esp_err_t ret = ESP_ERR_NOT_FOUND;
    int64_t start = esp_timer_get_time();
    do {
        size_t len = 0;
        esp_err_t r = transceive(handle, PN532_CMD_IN_LIST_PASSIVE, params, sizeof(params), resp, sizeof(resp),
                                 &len, 1000);
        if (r == ESP_OK && len >= 6 && resp[0] >= 1) {
            uint8_t uid_len = resp[5];
            if (uid_len > PN532_MAX_UID_LEN || (size_t)6 + uid_len > len) {
                ret = ESP_ERR_INVALID_RESPONSE;
                break;
            }
            out_card->atqa = (uint16_t)(resp[2] << 8 | resp[3]);
            out_card->sak = resp[4];
            out_card->uid_len = uid_len;
            memcpy(out_card->uid, &resp[6], uid_len);
            ret = ESP_OK;
            break;
        }
        if (r == ESP_ERR_TIMEOUT) {
            i2c_master_transmit(handle->i2c_dev, s_ack_frame, sizeof(s_ack_frame), I2C_TIMEOUT_MS);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    } while (!timed_out(start, timeout_ms));
    xSemaphoreGive(handle->lock);
    return ret;
}
const char *pn532_card_type_name(const pn532_card_t *card)
{
    if (!card) return "unknown";
    switch (card->sak) {
        case 0x00: return "MIFARE Ultralight / NTAG";
        case 0x08: return "MIFARE Classic 1K";
        case 0x09: return "MIFARE Mini";
        case 0x18: return "MIFARE Classic 4K";
        case 0x20: return "ISO14443-4 (DESFire / smart card)";
        case 0x28: return "MIFARE Classic 1K (emulated)";
        case 0x38: return "MIFARE Classic 4K (emulated)";
        default:   return "ISO14443A";
    }
}
