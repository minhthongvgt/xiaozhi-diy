#ifndef W5500_ETH_H
#define W5500_ETH_H
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct w5500_eth_config_t {
    spi_host_device_t host;
    gpio_num_t        sck_pin;
    gpio_num_t        mosi_pin;
    gpio_num_t        miso_pin;
    gpio_num_t        cs_pin;
    gpio_num_t        int_pin;
    gpio_num_t        rst_pin;
    int               spi_clock_hz;
} w5500_eth_config_t;
typedef struct w5500_eth_status_t {
    bool     link_up;
    bool     got_ip;
    uint32_t ip;
    uint32_t netmask;
    uint32_t gateway;
} w5500_eth_status_t;
esp_err_t w5500_eth_init(const w5500_eth_config_t *config);
void w5500_eth_deinit(void);
esp_err_t w5500_eth_get_status(w5500_eth_status_t *out_status);
#ifdef __cplusplus
}
#endif
#endif
