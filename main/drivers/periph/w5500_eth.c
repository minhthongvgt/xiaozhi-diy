#include "drivers/periph/w5500_eth.h"
#include <esp_log.h>
#include <esp_eth.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_netif_defaults.h>
#include <esp_eth_mac_w5500.h>
#include <esp_eth_phy_w5500.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#define TAG "W5500Eth"
#define W5500_DEFAULT_SPI_HZ (20 * 1000 * 1000)
#define W5500_POLL_PERIOD_MS 10
typedef struct w5500_eth_dev_s {
    esp_eth_handle_t          eth;
    esp_netif_t              *netif;
    esp_eth_netif_glue_handle_t glue;
    bool                      link_up;
    bool                      got_ip;
    esp_netif_ip_info_t       ip_info;
    spi_host_device_t         host;
    gpio_num_t                rst_pin;
} w5500_eth_dev_s;
static w5500_eth_dev_s *s_dev = NULL;
static void eth_event_handler(void *arg, esp_event_base_t base, int32_t event_id, void *event_data)
{
    (void)arg;
    (void)base;
    (void)event_data;
    if (event_id == ETHERNET_EVENT_CONNECTED) {
        s_dev->link_up = true;
        ESP_LOGI(TAG, "Ethernet link up");
    } else if (event_id == ETHERNET_EVENT_DISCONNECTED) {
        s_dev->link_up = false;
        s_dev->got_ip = false;
        ESP_LOGW(TAG, "Ethernet link down");
    }
}
static void ip_event_handler(void *arg, esp_event_base_t base, int32_t event_id, void *event_data)
{
    (void)arg;
    (void)base;
    if (event_id == IP_EVENT_ETH_GOT_IP && s_dev) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        s_dev->ip_info = event->ip_info;
        s_dev->got_ip = true;
        ESP_LOGI(TAG, "Got IP " IPSTR, IP2STR(&event->ip_info.ip));
    }
}
esp_err_t w5500_eth_init(const w5500_eth_config_t *config)
{
    if (!config) return ESP_ERR_INVALID_ARG;
    if (s_dev) return ESP_ERR_INVALID_STATE;
    if (!GPIO_IS_VALID_GPIO(config->sck_pin) || !GPIO_IS_VALID_GPIO(config->mosi_pin) ||
        !GPIO_IS_VALID_GPIO(config->miso_pin) || !GPIO_IS_VALID_GPIO(config->cs_pin)) {
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t ret = esp_netif_init();
    if (ret != ESP_OK) return ret;
    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) return ret;

    if (GPIO_IS_VALID_OUTPUT_GPIO(config->rst_pin)) {
        gpio_config_t rst_conf = {
            .pin_bit_mask = (1ULL << config->rst_pin),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&rst_conf);
        gpio_set_level(config->rst_pin, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(config->rst_pin, 1);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    spi_bus_config_t buscfg = {
        .mosi_io_num = config->mosi_pin,
        .miso_io_num = config->miso_pin,
        .sclk_io_num = config->sck_pin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 2048,
    };
    ret = spi_bus_initialize(config->host, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    spi_device_interface_config_t spi_devcfg = {
        .mode = 0,
        .clock_speed_hz = config->spi_clock_hz > 0 ? config->spi_clock_hz : W5500_DEFAULT_SPI_HZ,
        .queue_size = 20,
        .spics_io_num = config->cs_pin,
    };
    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(config->host, &spi_devcfg);
    if (GPIO_IS_VALID_GPIO(config->int_pin)) {
        w5500_config.base.int_gpio_num = config->int_pin;
    } else {
        w5500_config.base.int_gpio_num = -1;
        w5500_config.base.poll_period_ms = W5500_POLL_PERIOD_MS;
    }

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    mac_config.rx_task_stack_size = 4096;
    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
    if (!mac) {
        ESP_LOGE(TAG, "Failed to create W5500 MAC (check SPI wiring and CS pin)");
        return ESP_FAIL;
    }

    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.reset_gpio_num = -1;
    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);
    if (!phy) {
        ESP_LOGE(TAG, "Failed to create W5500 PHY");
        return ESP_FAIL;
    }

    w5500_eth_dev_s *dev = calloc(1, sizeof(w5500_eth_dev_s));
    if (!dev) return ESP_ERR_NO_MEM;
    dev->host = config->host;
    dev->rst_pin = config->rst_pin;

    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
    ret = esp_eth_driver_install(&eth_config, &dev->eth);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Ethernet driver install failed: %s", esp_err_to_name(ret));
        free(dev);
        return ret;
    }

    esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_ETH();
    dev->netif = esp_netif_new(&netif_config);
    dev->glue = esp_eth_new_netif_glue(dev->eth);
    esp_netif_attach(dev->netif, dev->glue);

    s_dev = dev;
    esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, eth_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, ip_event_handler, NULL);

    ret = esp_eth_start(dev->eth);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Ethernet start failed: %s", esp_err_to_name(ret));
        w5500_eth_deinit();
        return ret;
    }
    ESP_LOGI(TAG, "W5500 Ethernet started (SPI host %d, %d Hz)", (int)config->host, (int)spi_devcfg.clock_speed_hz);
    return ESP_OK;
}

void w5500_eth_deinit(void)
{
    if (!s_dev) return;
    esp_eth_stop(s_dev->eth);
    esp_event_handler_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, eth_event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_ETH_GOT_IP, ip_event_handler);
    esp_eth_del_netif_glue(s_dev->glue);
    esp_netif_destroy(s_dev->netif);
    esp_eth_driver_uninstall(s_dev->eth);
    free(s_dev);
    s_dev = NULL;
}

esp_err_t w5500_eth_get_status(w5500_eth_status_t *out_status)
{
    if (!out_status) return ESP_ERR_INVALID_ARG;
    memset(out_status, 0, sizeof(*out_status));
    if (!s_dev) return ESP_ERR_INVALID_STATE;
    out_status->link_up = s_dev->link_up;
    out_status->got_ip = s_dev->got_ip;
    out_status->ip = s_dev->ip_info.ip.addr;
    out_status->netmask = s_dev->ip_info.netmask.addr;
    out_status->gateway = s_dev->ip_info.gw.addr;
    return ESP_OK;
}
