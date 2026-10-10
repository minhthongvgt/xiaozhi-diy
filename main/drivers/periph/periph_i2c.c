#include "drivers/periph/periph_i2c.h"
#include "boards/common/bus_manager.h"
#include <esp_log.h>
#include <sdkconfig.h>
#define TAG "PeriphI2c"
#define PERIPH_I2C_EXTRA_MAX 2
#define PERIPH_I2C_EXTRA_PORT I2C_NUM_1
typedef struct {
    gpio_num_t sda;
    gpio_num_t scl;
    i2c_master_bus_handle_t bus;
} periph_i2c_entry_t;
static periph_i2c_entry_t s_extra[PERIPH_I2C_EXTRA_MAX];
static int s_extra_count = 0;
i2c_master_bus_handle_t periph_i2c_get_bus(gpio_num_t sda_pin, gpio_num_t scl_pin)
{
    if (!GPIO_IS_VALID_GPIO(sda_pin) || !GPIO_IS_VALID_GPIO(scl_pin)) {
        return NULL;
    }
    gpio_num_t main_sda = (gpio_num_t)-1;
    gpio_num_t main_scl = (gpio_num_t)-1;
    bus_manager_get_i2c_pins(&main_sda, &main_scl);
    if (sda_pin == main_sda && scl_pin == main_scl) {
        return bus_manager_get_i2c_bus();
    }
    for (int i = 0; i < s_extra_count; i++) {
        if (s_extra[i].sda == sda_pin && s_extra[i].scl == scl_pin) {
            return s_extra[i].bus;
        }
    }
    if (s_extra_count >= PERIPH_I2C_EXTRA_MAX) {
        ESP_LOGE(TAG, "No free secondary I2C bus for SDA=%d SCL=%d", (int)sda_pin, (int)scl_pin);
        return NULL;
    }
    i2c_master_bus_config_t cfg = {
        .i2c_port = PERIPH_I2C_EXTRA_PORT,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = true,
        },
    };
    i2c_master_bus_handle_t bus = NULL;
    esp_err_t ret = i2c_new_master_bus(&cfg, &bus);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create secondary I2C bus (SDA=%d SCL=%d): %s", (int)sda_pin, (int)scl_pin,
                 esp_err_to_name(ret));
        return NULL;
    }
    s_extra[s_extra_count].sda = sda_pin;
    s_extra[s_extra_count].scl = scl_pin;
    s_extra[s_extra_count].bus = bus;
    s_extra_count++;
    ESP_LOGI(TAG, "Secondary I2C bus created on SDA=%d SCL=%d", (int)sda_pin, (int)scl_pin);
    return bus;
}
