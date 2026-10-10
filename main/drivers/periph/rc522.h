#ifndef RC522_H
#define RC522_H
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RC522_MAX_UID_LEN 10
typedef struct rc522_config_t {
    spi_host_device_t host;
    gpio_num_t        sck_pin;
    gpio_num_t        mosi_pin;
    gpio_num_t        miso_pin;
    gpio_num_t        cs_pin;
    gpio_num_t        rst_pin;
    int               spi_clock_hz;
} rc522_config_t;
typedef struct rc522_card_t {
    uint8_t uid[RC522_MAX_UID_LEN];
    uint8_t uid_len;
    uint8_t sak;
    uint8_t atqa[2];
} rc522_card_t;
typedef struct rc522_dev_s *rc522_handle_t;
esp_err_t rc522_init(const rc522_config_t *config, rc522_handle_t *out_handle);
void rc522_deinit(rc522_handle_t handle);
esp_err_t rc522_get_version(rc522_handle_t handle, uint8_t *out_version);
esp_err_t rc522_read_card(rc522_handle_t handle, uint32_t timeout_ms, rc522_card_t *out_card);
const char *rc522_card_type_name(uint8_t sak);
#ifdef __cplusplus
}
#endif
#endif
