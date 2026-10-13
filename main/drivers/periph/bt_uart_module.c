#include "bt_uart_module.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#define BT_UART_RX_BUFFER_SIZE 1024
#define BT_UART_AT_REPLY_MAX 96
#define BT_UART_AT_COMMAND_MAX 64
#define BT_UART_AT_TIMEOUT_MS 1000
#define BT_UART_HC06_TIMEOUT_MS 1500
#define BT_UART_KEY_SETTLE_MS 100
#define BT_UART_REBOOT_WAIT_MS 1000
#define BT_UART_ADDR_HEX_LEN 12

static const char *TAG = "BtUart";

static bool s_ready = false;
static uart_port_t s_port = UART_NUM_1;
static gpio_num_t s_state_pin = (gpio_num_t)-1;
static SemaphoreHandle_t s_write_mutex = NULL;

static bool at_exchange(const char *command, const char *line_end, const char *expect, uint32_t timeout_ms)
{
    char reply[BT_UART_AT_REPLY_MAX + 1] = {0};
    size_t reply_len = 0;
    uart_flush_input(s_port);
    uart_write_bytes(s_port, command, strlen(command));
    if (line_end[0] != '\0') {
        uart_write_bytes(s_port, line_end, strlen(line_end));
    }
    TickType_t start = xTaskGetTickCount();
    TickType_t limit = pdMS_TO_TICKS(timeout_ms);
    while ((xTaskGetTickCount() - start) < limit && reply_len < BT_UART_AT_REPLY_MAX) {
        int got = uart_read_bytes(s_port, (uint8_t *)reply + reply_len, BT_UART_AT_REPLY_MAX - reply_len,
                                  pdMS_TO_TICKS(50));
        if (got > 0) {
            reply_len += (size_t)got;
            reply[reply_len] = '\0';
            if (strstr(reply, expect) != NULL) {
                break;
            }
        }
    }
    for (size_t i = 0; i < reply_len; ++i) {
        if (reply[i] == '\r' || reply[i] == '\n') {
            reply[i] = ' ';
        }
    }
    bool ok = strstr(reply, expect) != NULL;
    if (ok) {
        ESP_LOGI(TAG, "%s -> %s", command, reply);
    } else {
        ESP_LOGW(TAG, "%s -> no '%s' (reply: '%s')", command, expect, reply);
    }
    return ok;
}

static bool normalize_address(const char *input, char *hex_out)
{
    size_t count = 0;
    if (input == NULL) {
        return false;
    }
    for (const char *p = input; *p != '\0'; ++p) {
        if (*p == ':' || *p == '-' || *p == ',' || *p == ' ') {
            continue;
        }
        if (!isxdigit((unsigned char)*p) || count >= BT_UART_ADDR_HEX_LEN) {
            return false;
        }
        hex_out[count++] = (char)toupper((unsigned char)*p);
    }
    hex_out[count] = '\0';
    return count == BT_UART_ADDR_HEX_LEN;
}

static bool has_text(const char *text)
{
    return text != NULL && text[0] != '\0';
}

static void apply_hc05(const bt_uart_module_config_t *config)
{
    char command[BT_UART_AT_COMMAND_MAX];
    if (!GPIO_IS_VALID_OUTPUT_GPIO(config->key_pin)) {
        ESP_LOGW(TAG, "HC-05 needs the KEY pin to accept AT commands; settings not applied");
        return;
    }
    gpio_set_level(config->key_pin, 1);
    vTaskDelay(pdMS_TO_TICKS(BT_UART_KEY_SETTLE_MS));
    if (!at_exchange("AT", "\r\n", "OK", BT_UART_AT_TIMEOUT_MS)) {
        ESP_LOGW(TAG, "HC-05 does not answer AT; check KEY wiring and baud rate");
        gpio_set_level(config->key_pin, 0);
        return;
    }
    if (has_text(config->name)) {
        snprintf(command, sizeof(command), "AT+NAME=%s", config->name);
        at_exchange(command, "\r\n", "OK", BT_UART_AT_TIMEOUT_MS);
    }
    if (has_text(config->pin_code)) {
        snprintf(command, sizeof(command), "AT+PSWD=%s", config->pin_code);
        if (!at_exchange(command, "\r\n", "OK", BT_UART_AT_TIMEOUT_MS)) {
            snprintf(command, sizeof(command), "AT+PSWD=\"%s\"", config->pin_code);
            at_exchange(command, "\r\n", "OK", BT_UART_AT_TIMEOUT_MS);
        }
    }
    snprintf(command, sizeof(command), "AT+ROLE=%d", config->role == BT_UART_ROLE_MASTER ? 1 : 0);
    at_exchange(command, "\r\n", "OK", BT_UART_AT_TIMEOUT_MS);
    if (config->role == BT_UART_ROLE_MASTER) {
        char hex[BT_UART_ADDR_HEX_LEN + 1];
        if (normalize_address(config->remote_addr, hex)) {
            at_exchange("AT+CMODE=0", "\r\n", "OK", BT_UART_AT_TIMEOUT_MS);
            snprintf(command, sizeof(command), "AT+BIND=%.4s,%.2s,%.6s", hex, hex + 4, hex + 6);
            at_exchange(command, "\r\n", "OK", BT_UART_AT_TIMEOUT_MS);
        } else {
            ESP_LOGW(TAG, "Remote address '%s' is not 12 hex digits; master will not bind",
                     has_text(config->remote_addr) ? config->remote_addr : "");
        }
    }
    at_exchange("AT+RESET", "\r\n", "OK", BT_UART_AT_TIMEOUT_MS);
    gpio_set_level(config->key_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(BT_UART_REBOOT_WAIT_MS));
}

static void apply_hc06(const bt_uart_module_config_t *config)
{
    char command[BT_UART_AT_COMMAND_MAX];
    if (!at_exchange("AT", "", "OK", BT_UART_HC06_TIMEOUT_MS)) {
        ESP_LOGW(TAG, "HC-06 does not answer AT; it only accepts commands while not connected");
        return;
    }
    if (has_text(config->name)) {
        snprintf(command, sizeof(command), "AT+NAME%s", config->name);
        at_exchange(command, "", "OK", BT_UART_HC06_TIMEOUT_MS);
    }
    if (has_text(config->pin_code)) {
        snprintf(command, sizeof(command), "AT+PIN%s", config->pin_code);
        at_exchange(command, "", "OK", BT_UART_HC06_TIMEOUT_MS);
    }
}

static void apply_hm10(const bt_uart_module_config_t *config)
{
    char command[BT_UART_AT_COMMAND_MAX];
    if (!at_exchange("AT", "", "OK", BT_UART_AT_TIMEOUT_MS)) {
        ESP_LOGW(TAG, "HM-10 does not answer AT; it only accepts commands while not connected");
        return;
    }
    if (has_text(config->name)) {
        snprintf(command, sizeof(command), "AT+NAME%s", config->name);
        at_exchange(command, "", "OK+Set", BT_UART_AT_TIMEOUT_MS);
    }
    if (has_text(config->pin_code)) {
        snprintf(command, sizeof(command), "AT+PASS%s", config->pin_code);
        at_exchange(command, "", "OK+Set", BT_UART_AT_TIMEOUT_MS);
        at_exchange("AT+TYPE2", "", "OK+Set", BT_UART_AT_TIMEOUT_MS);
    }
    if (GPIO_IS_VALID_GPIO(config->state_pin)) {
        at_exchange("AT+PIO11", "", "OK+Set", BT_UART_AT_TIMEOUT_MS);
    }
    bool master = config->role == BT_UART_ROLE_MASTER;
    char hex[BT_UART_ADDR_HEX_LEN + 1] = {0};
    bool has_remote = master && normalize_address(config->remote_addr, hex);
    if (master && !has_remote) {
        ESP_LOGW(TAG, "Remote address '%s' is not 12 hex digits; master will not connect",
                 has_text(config->remote_addr) ? config->remote_addr : "");
    }
    at_exchange(master ? "AT+IMME1" : "AT+IMME0", "", "OK+Set", BT_UART_AT_TIMEOUT_MS);
    at_exchange(master ? "AT+ROLE1" : "AT+ROLE0", "", "OK+Set", BT_UART_AT_TIMEOUT_MS);
    at_exchange("AT+RESET", "", "OK+RESET", BT_UART_AT_TIMEOUT_MS);
    vTaskDelay(pdMS_TO_TICKS(BT_UART_REBOOT_WAIT_MS));
    if (has_remote) {
        snprintf(command, sizeof(command), "AT+CON%s", hex);
        at_exchange(command, "", "OK+CONN", BT_UART_AT_TIMEOUT_MS);
    }
}

static esp_err_t configure_control_pins(const bt_uart_module_config_t *config)
{
    if (GPIO_IS_VALID_GPIO(config->state_pin)) {
        gpio_config_t state_cfg = {
            .pin_bit_mask = 1ULL << config->state_pin,
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_ENABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        esp_err_t err = gpio_config(&state_cfg);
        if (err != ESP_OK) {
            return err;
        }
    }
    if (GPIO_IS_VALID_OUTPUT_GPIO(config->key_pin)) {
        gpio_config_t key_cfg = {
            .pin_bit_mask = 1ULL << config->key_pin,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        esp_err_t err = gpio_config(&key_cfg);
        if (err != ESP_OK) {
            return err;
        }
        gpio_set_level(config->key_pin, 0);
    }
    return ESP_OK;
}

esp_err_t bt_uart_module_init(const bt_uart_module_config_t *config)
{
    if (s_ready) {
        return ESP_OK;
    }
    if (config == NULL || !GPIO_IS_VALID_OUTPUT_GPIO(config->tx_pin) || !GPIO_IS_VALID_GPIO(config->rx_pin)) {
        ESP_LOGE(TAG, "Bluetooth module needs valid TX and RX pins");
        return ESP_ERR_INVALID_ARG;
    }
    if (uart_is_driver_installed(config->port)) {
        ESP_LOGE(TAG, "UART%d is already used by another feature; pick a different port for the Bluetooth module",
                 (int)config->port);
        return ESP_ERR_INVALID_STATE;
    }
    uart_config_t uart_cfg = {
        .baud_rate = config->baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    esp_err_t err = uart_param_config(config->port, &uart_cfg);
    if (err == ESP_OK) {
        err = uart_set_pin(config->port, config->tx_pin, config->rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    }
    if (err == ESP_OK) {
        err = uart_driver_install(config->port, BT_UART_RX_BUFFER_SIZE, 0, 0, NULL, 0);
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "UART%d setup failed: %s", (int)config->port, esp_err_to_name(err));
        return err;
    }
    s_port = config->port;
    s_state_pin = config->state_pin;
    err = configure_control_pins(config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "STATE/KEY pin setup failed: %s", esp_err_to_name(err));
        uart_driver_delete(config->port);
        return err;
    }
    s_write_mutex = xSemaphoreCreateMutex();
    if (s_write_mutex == NULL) {
        uart_driver_delete(config->port);
        return ESP_ERR_NO_MEM;
    }
    if (config->apply_settings && config->type != BT_UART_TYPE_PRECONFIGURED) {
        if (bt_uart_module_link_state() == BT_UART_LINK_CONNECTED) {
            ESP_LOGW(TAG, "Module is already connected; AT settings skipped");
        } else if (config->type == BT_UART_TYPE_HC05) {
            apply_hc05(config);
        } else if (config->type == BT_UART_TYPE_HC06) {
            apply_hc06(config);
        } else {
            apply_hm10(config);
        }
        uart_flush_input(s_port);
    }
    s_ready = true;
    ESP_LOGI(TAG, "%s on UART%d (TX=%d, RX=%d, %d baud, STATE=%d)", bt_uart_module_type_name(config->type),
             (int)config->port, (int)config->tx_pin, (int)config->rx_pin, config->baud_rate, (int)config->state_pin);
    return ESP_OK;
}

bool bt_uart_module_is_ready(void)
{
    return s_ready;
}

uart_port_t bt_uart_module_port(void)
{
    return s_port;
}

bt_uart_link_state_t bt_uart_module_link_state(void)
{
    if (!GPIO_IS_VALID_GPIO(s_state_pin)) {
        return BT_UART_LINK_UNKNOWN;
    }
    return gpio_get_level(s_state_pin) ? BT_UART_LINK_CONNECTED : BT_UART_LINK_DISCONNECTED;
}

int bt_uart_module_write(const uint8_t *data, size_t length)
{
    if (!s_ready || data == NULL) {
        return -1;
    }
    xSemaphoreTake(s_write_mutex, portMAX_DELAY);
    int written = uart_write_bytes(s_port, data, length);
    xSemaphoreGive(s_write_mutex);
    return written;
}

int bt_uart_module_read(uint8_t *buffer, size_t capacity, uint32_t timeout_ms)
{
    if (!s_ready || buffer == NULL || capacity == 0) {
        return -1;
    }
    return uart_read_bytes(s_port, buffer, (uint32_t)capacity, pdMS_TO_TICKS(timeout_ms));
}

const char *bt_uart_module_type_name(bt_uart_type_t type)
{
    switch (type) {
    case BT_UART_TYPE_HC05:
        return "HC-05";
    case BT_UART_TYPE_HC06:
        return "HC-06";
    case BT_UART_TYPE_HM10:
        return "HM-10";
    default:
        return "Preconfigured module";
    }
}
