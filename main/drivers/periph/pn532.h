#ifndef PN532_H
#define PN532_H
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define PN532_I2C_ADDR 0x24
#define PN532_MAX_UID_LEN 10
typedef struct pn532_config_t {
    i2c_master_bus_handle_t i2c_bus;
    gpio_num_t              irq_pin;
    gpio_num_t              rst_pin;
} pn532_config_t;
typedef struct pn532_card_t {
    uint8_t  uid[PN532_MAX_UID_LEN];
    uint8_t  uid_len;
    uint16_t atqa;
    uint8_t  sak;
} pn532_card_t;
typedef struct pn532_dev_s *pn532_handle_t;
esp_err_t pn532_init(const pn532_config_t *config, pn532_handle_t *out_handle);
void pn532_deinit(pn532_handle_t handle);
esp_err_t pn532_get_firmware_version(pn532_handle_t handle, uint32_t *out_version);
esp_err_t pn532_read_passive_target(pn532_handle_t handle, uint32_t timeout_ms, pn532_card_t *out_card);
const char *pn532_card_type_name(const pn532_card_t *card);
#ifdef __cplusplus
}
#endif
#endif
