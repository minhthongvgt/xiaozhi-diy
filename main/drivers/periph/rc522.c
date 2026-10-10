#include "drivers/periph/rc522.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <stdlib.h>
#include <string.h>
#define TAG "RC522"
#define RC522_DEFAULT_SPI_HZ 4000000
#define REG_COMMAND        0x01
#define REG_COMIEN         0x02
#define REG_DIVIEN         0x03
#define REG_COMIRQ         0x04
#define REG_DIVIRQ         0x05
#define REG_ERROR          0x06
#define REG_FIFO_DATA      0x09
#define REG_FIFO_LEVEL     0x0A
#define REG_CONTROL        0x0C
#define REG_BIT_FRAMING    0x0D
#define REG_COLL           0x0E
#define REG_MODE           0x11
#define REG_TX_MODE        0x12
#define REG_RX_MODE        0x13
#define REG_TX_CONTROL     0x14
#define REG_TX_ASK         0x15
#define REG_CRC_RESULT_H   0x21
#define REG_CRC_RESULT_L   0x22
#define REG_TMODE          0x2A
#define REG_TPRESCALER     0x2B
#define REG_TRELOAD_H      0x2C
#define REG_TRELOAD_L      0x2D
#define REG_VERSION        0x37
#define CMD_IDLE           0x00
#define CMD_CALC_CRC       0x03
#define CMD_TRANSCEIVE     0x0C
#define CMD_SOFT_RESET     0x0F
#define PICC_REQA          0x26
#define PICC_SEL_CL1       0x93
#define PICC_SEL_CL2       0x95
#define PICC_SEL_CL3       0x97
#define PICC_HALT          0x50
#define CASCADE_TAG        0x88
#define ERR_BUFFER_OVF     0x10
#define ERR_COLL           0x08
#define ERR_PARITY         0x02
#define ERR_PROTOCOL       0x01
typedef struct rc522_dev_s {
    spi_device_handle_t spi;
    gpio_num_t          rst_pin;
    SemaphoreHandle_t   lock;
} rc522_dev_s;
static esp_err_t reg_write(rc522_dev_s *dev, uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = {(uint8_t)((reg << 1) & 0x7E), val};
    spi_transaction_t t = {.length = 16, .tx_buffer = tx};
    return spi_device_polling_transmit(dev->spi, &t);
}
static uint8_t reg_read(rc522_dev_s *dev, uint8_t reg)
{
    uint8_t tx[2] = {(uint8_t)(((reg << 1) & 0x7E) | 0x80), 0};
    uint8_t rx[2] = {0, 0};
    spi_transaction_t t = {.length = 16, .tx_buffer = tx, .rx_buffer = rx};
    spi_device_polling_transmit(dev->spi, &t);
    return rx[1];
}
static void reg_set_bits(rc522_dev_s *dev, uint8_t reg, uint8_t mask)
{
    reg_write(dev, reg, reg_read(dev, reg) | mask);
}
static void reg_clear_bits(rc522_dev_s *dev, uint8_t reg, uint8_t mask)
{
    reg_write(dev, reg, reg_read(dev, reg) & (uint8_t)~mask);
}
static void fifo_read(rc522_dev_s *dev, uint8_t *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        buf[i] = reg_read(dev, REG_FIFO_DATA);
    }
}
static esp_err_t calc_crc(rc522_dev_s *dev, const uint8_t *data, size_t len, uint8_t out[2])
{
    reg_write(dev, REG_COMMAND, CMD_IDLE);
    reg_write(dev, REG_DIVIRQ, 0x7F);
    reg_set_bits(dev, REG_FIFO_LEVEL, 0x80);
    for (size_t i = 0; i < len; i++) {
        reg_write(dev, REG_FIFO_DATA, data[i]);
    }
    reg_write(dev, REG_COMMAND, CMD_CALC_CRC);
    int64_t start = esp_timer_get_time();
    while (!(reg_read(dev, REG_DIVIRQ) & 0x04)) {
        if (esp_timer_get_time() - start > 90000) return ESP_ERR_TIMEOUT;
    }
    reg_write(dev, REG_COMMAND, CMD_IDLE);
    out[0] = reg_read(dev, REG_CRC_RESULT_L);
    out[1] = reg_read(dev, REG_CRC_RESULT_H);
    return ESP_OK;
}
static esp_err_t transceive(rc522_dev_s *dev, const uint8_t *send, size_t send_len, uint8_t valid_bits,
                            uint8_t *back, size_t *back_len, uint8_t rx_align, uint32_t timeout_ms)
{
    uint8_t wait_irq = 0x30;
    reg_write(dev, REG_COMMAND, CMD_IDLE);
    reg_write(dev, REG_COMIEN, 0x77 | 0x80);
    reg_clear_bits(dev, REG_COMIRQ, 0x80);
    reg_set_bits(dev, REG_FIFO_LEVEL, 0x80);
    for (size_t i = 0; i < send_len; i++) {
        reg_write(dev, REG_FIFO_DATA, send[i]);
    }
    reg_write(dev, REG_COMMAND, CMD_TRANSCEIVE);
    reg_write(dev, REG_BIT_FRAMING, (uint8_t)((rx_align << 4) | valid_bits));
    reg_set_bits(dev, REG_BIT_FRAMING, 0x80);
    int64_t start = esp_timer_get_time();
    uint8_t irq = 0;
    do {
        irq = reg_read(dev, REG_COMIRQ);
        if (irq & 0x01) return ESP_ERR_TIMEOUT;
        if (irq & wait_irq) break;
        if (esp_timer_get_time() - start > (int64_t)timeout_ms * 1000) return ESP_ERR_TIMEOUT;
    } while (true);
    reg_clear_bits(dev, REG_BIT_FRAMING, 0x80);
    uint8_t err = reg_read(dev, REG_ERROR);
    if (err & (ERR_BUFFER_OVF | ERR_PARITY | ERR_PROTOCOL)) return ESP_ERR_INVALID_RESPONSE;
    if (err & ERR_COLL) return ESP_ERR_INVALID_STATE;
    if (back && back_len) {
        uint8_t n = reg_read(dev, REG_FIFO_LEVEL);
        if (n > *back_len) n = (uint8_t)*back_len;
        fifo_read(dev, back, n);
        *back_len = n;
    }
    return ESP_OK;
}
static esp_err_t request_a(rc522_dev_s *dev, uint8_t atqa[2])
{
    reg_clear_bits(dev, REG_COLL, 0x80);
    uint8_t cmd = PICC_REQA;
    size_t len = 2;
    esp_err_t ret = transceive(dev, &cmd, 1, 7, atqa, &len, 0, 25);
    if (ret != ESP_OK) return ret;
    return len == 2 ? ESP_OK : ESP_ERR_INVALID_RESPONSE;
}
static esp_err_t anticoll(rc522_dev_s *dev, uint8_t sel, uint8_t out[5])
{
    reg_clear_bits(dev, REG_COLL, 0x80);
    uint8_t send[2] = {sel, 0x20};
    size_t len = 5;
    esp_err_t ret = transceive(dev, send, 2, 0, out, &len, 0, 25);
    if (ret != ESP_OK || len != 5) return ret != ESP_OK ? ret : ESP_ERR_INVALID_RESPONSE;
    if ((out[0] ^ out[1] ^ out[2] ^ out[3]) != out[4]) return ESP_ERR_INVALID_CRC;
    return ESP_OK;
}
static esp_err_t select_level(rc522_dev_s *dev, uint8_t sel, const uint8_t uid5[5], uint8_t *sak)
{
    uint8_t send[9];
    send[0] = sel;
    send[1] = 0x70;
    memcpy(&send[2], uid5, 5);
    if (calc_crc(dev, send, 7, &send[7]) != ESP_OK) return ESP_ERR_TIMEOUT;
    uint8_t back[3] = {0};
    size_t len = 3;
    esp_err_t ret = transceive(dev, send, 9, 0, back, &len, 0, 25);
    if (ret != ESP_OK) return ret;
    if (len < 1) return ESP_ERR_INVALID_RESPONSE;
    *sak = back[0];
    return ESP_OK;
}
static void halt(rc522_dev_s *dev)
{
    uint8_t send[4] = {PICC_HALT, 0x00, 0, 0};
    if (calc_crc(dev, send, 2, &send[2]) != ESP_OK) return;
    transceive(dev, send, 4, 0, NULL, NULL, 0, 10);
    reg_write(dev, REG_COMMAND, CMD_IDLE);
}
esp_err_t rc522_init(const rc522_config_t *config, rc522_handle_t *out_handle)
{
    if (!config || !out_handle) return ESP_ERR_INVALID_ARG;
    *out_handle = NULL;
    if (!GPIO_IS_VALID_GPIO(config->sck_pin) || !GPIO_IS_VALID_GPIO(config->mosi_pin) ||
        !GPIO_IS_VALID_GPIO(config->miso_pin) || !GPIO_IS_VALID_GPIO(config->cs_pin)) {
        return ESP_ERR_INVALID_ARG;
    }
    rc522_dev_s *dev = calloc(1, sizeof(rc522_dev_s));
    if (!dev) return ESP_ERR_NO_MEM;
    dev->rst_pin = config->rst_pin;
    dev->lock = xSemaphoreCreateMutex();
    if (!dev->lock) {
        free(dev);
        return ESP_ERR_NO_MEM;
    }
    spi_bus_config_t buscfg = {
        .mosi_io_num = config->mosi_pin,
        .miso_io_num = config->miso_pin,
        .sclk_io_num = config->sck_pin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 64,
    };
    esp_err_t ret = spi_bus_initialize(config->host, &buscfg, SPI_DMA_DISABLED);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        vSemaphoreDelete(dev->lock);
        free(dev);
        return ret;
    }
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = config->spi_clock_hz > 0 ? config->spi_clock_hz : RC522_DEFAULT_SPI_HZ,
        .mode = 0,
        .spics_io_num = config->cs_pin,
        .queue_size = 1,
    };
    ret = spi_bus_add_device(config->host, &devcfg, &dev->spi);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI device add failed: %s", esp_err_to_name(ret));
        vSemaphoreDelete(dev->lock);
        free(dev);
        return ret;
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
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(dev->rst_pin, 1);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    reg_write(dev, REG_COMMAND, CMD_SOFT_RESET);
    vTaskDelay(pdMS_TO_TICKS(50));
    uint8_t version = reg_read(dev, REG_VERSION);
    if (version != 0x91 && version != 0x92 && version != 0x88 && version != 0x90) {
        ESP_LOGE(TAG, "Unexpected MFRC522 version 0x%02X (wiring or CS pin?)", version);
        rc522_deinit(dev);
        return ESP_ERR_NOT_FOUND;
    }
    reg_write(dev, REG_TMODE, 0x8D);
    reg_write(dev, REG_TPRESCALER, 0x3E);
    reg_write(dev, REG_TRELOAD_L, 30);
    reg_write(dev, REG_TRELOAD_H, 0);
    reg_write(dev, REG_TX_ASK, 0x40);
    reg_write(dev, REG_MODE, 0x3D);
    reg_set_bits(dev, REG_TX_CONTROL, 0x03);
    ESP_LOGI(TAG, "MFRC522 initialized (version 0x%02X)", version);
    *out_handle = dev;
    return ESP_OK;
}
void rc522_deinit(rc522_handle_t handle)
{
    if (!handle) return;
    if (handle->spi) spi_bus_remove_device(handle->spi);
    if (handle->lock) vSemaphoreDelete(handle->lock);
    free(handle);
}
esp_err_t rc522_get_version(rc522_handle_t handle, uint8_t *out_version)
{
    if (!handle || !out_version) return ESP_ERR_INVALID_ARG;
    *out_version = reg_read(handle, REG_VERSION);
    return ESP_OK;
}
esp_err_t rc522_read_card(rc522_handle_t handle, uint32_t timeout_ms, rc522_card_t *out_card)
{
    if (!handle || !out_card) return ESP_ERR_INVALID_ARG;
    memset(out_card, 0, sizeof(*out_card));
    if (xSemaphoreTake(handle->lock, pdMS_TO_TICKS(1000)) != pdTRUE) return ESP_ERR_TIMEOUT;
    esp_err_t ret = ESP_ERR_NOT_FOUND;
    int64_t start = esp_timer_get_time();
    while (esp_timer_get_time() - start < (int64_t)timeout_ms * 1000) {
        uint8_t atqa[2] = {0, 0};
        if (request_a(handle, atqa) != ESP_OK) {
            vTaskDelay(pdMS_TO_TICKS(30));
            continue;
        }
        memcpy(out_card->atqa, atqa, 2);
        uint8_t uid_len = 0;
        uint8_t uid[RC522_MAX_UID_LEN] = {0};
        uint8_t sak = 0;
        bool ok = true;
        const uint8_t levels[3] = {PICC_SEL_CL1, PICC_SEL_CL2, PICC_SEL_CL3};
        for (int level = 0; level < 3 && ok; level++) {
            uint8_t bcd[5] = {0};
            if (anticoll(handle, levels[level], bcd) != ESP_OK) {
                ok = false;
                break;
            }
            if (bcd[0] == CASCADE_TAG) {
                memcpy(&uid[uid_len], &bcd[1], 3);
                uid_len += 3;
            } else {
                memcpy(&uid[uid_len], bcd, 4);
                uid_len += 4;
            }
            if (select_level(handle, levels[level], bcd, &sak) != ESP_OK) {
                ok = false;
                break;
            }
            if (!(sak & 0x04)) break;
        }
        if (ok && uid_len > 0 && uid_len <= RC522_MAX_UID_LEN) {
            memcpy(out_card->uid, uid, uid_len);
            out_card->uid_len = uid_len;
            out_card->sak = sak;
            ret = ESP_OK;
            halt(handle);
            break;
        }
        halt(handle);
        vTaskDelay(pdMS_TO_TICKS(30));
    }
    xSemaphoreGive(handle->lock);
    return ret;
}
const char *rc522_card_type_name(uint8_t sak)
{
    switch (sak) {
        case 0x00: return "MIFARE Ultralight / NTAG";
        case 0x08: return "MIFARE Classic 1K";
        case 0x09: return "MIFARE Mini";
        case 0x18: return "MIFARE Classic 4K";
        case 0x20: return "ISO14443-4 (DESFire / smart card)";
        default:   return "ISO14443A";
    }
}
