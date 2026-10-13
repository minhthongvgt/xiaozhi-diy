#ifndef BT_UART_MODULE_H
#define BT_UART_MODULE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/uart.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum bt_uart_type_t {
    BT_UART_TYPE_HC05 = 0,
    BT_UART_TYPE_HC06,
    BT_UART_TYPE_HM10,
    BT_UART_TYPE_PRECONFIGURED,
} bt_uart_type_t;

typedef enum bt_uart_role_t {
    BT_UART_ROLE_SLAVE = 0,
    BT_UART_ROLE_MASTER,
} bt_uart_role_t;

typedef enum bt_uart_link_state_t {
    BT_UART_LINK_UNKNOWN = 0,
    BT_UART_LINK_DISCONNECTED,
    BT_UART_LINK_CONNECTED,
} bt_uart_link_state_t;

typedef struct bt_uart_module_config_t {
    uart_port_t port;
    gpio_num_t tx_pin;
    gpio_num_t rx_pin;
    gpio_num_t state_pin;
    gpio_num_t key_pin;
    int baud_rate;
    bt_uart_type_t type;
    bool apply_settings;
    const char *name;
    const char *pin_code;
    bt_uart_role_t role;
    const char *remote_addr;
} bt_uart_module_config_t;

esp_err_t bt_uart_module_init(const bt_uart_module_config_t *config);
bool bt_uart_module_is_ready(void);
uart_port_t bt_uart_module_port(void);
bt_uart_link_state_t bt_uart_module_link_state(void);
int bt_uart_module_write(const uint8_t *data, size_t length);
int bt_uart_module_read(uint8_t *buffer, size_t capacity, uint32_t timeout_ms);
const char *bt_uart_module_type_name(bt_uart_type_t type);

#ifdef __cplusplus
}
#endif
#endif
