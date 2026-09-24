/**
 * @file input_manager.c
 * @brief Input Peripherals Driver Manager Implementation (ESP-IDF 6.1)
 */

#include "input_manager.h"
#include "boards/common/bus_manager.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <driver/gpio.h>

#define TAG "InputManager"

static button_event_cb_t s_button_cb = NULL;
static volatile int32_t s_encoder_value = 0;

void input_manager_register_button_callback(button_event_cb_t cb)
{
    s_button_cb = cb;
}

int32_t input_manager_get_encoder_value(void)
{
    return s_encoder_value;
}

__attribute__((unused)) static void init_button(gpio_num_t pin, const char* name)
{
    if (pin < 0) return;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "Configured input button '%s' on GPIO %d (Pull-Up enabled)", name, pin);
}

esp_err_t input_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing User Inputs Subsystem (Buttons, Touch, Encoder)...");

    // 1. Physical Buttons (On Custom N16R8, buttons are owned by C++ Button instances)
#if !defined(CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM) && !defined(CONFIG_BOARD_TYPE_CUSTOM_S3_N16R8)
#if defined(CONFIG_CUSTOM_ENABLE_BUTTON_BOOT) || defined(CONFIG_ENABLE_BUTTON_BOOT)
#if defined(CONFIG_CUSTOM_BUTTON_BOOT_GPIO)
    init_button((gpio_num_t)CONFIG_CUSTOM_BUTTON_BOOT_GPIO, "BOOT Button");
#else
    init_button(GPIO_NUM_0, "BOOT Button");
#endif
#endif

#if defined(CONFIG_CUSTOM_ENABLE_BUTTON_TOUCH)
#if defined(CONFIG_CUSTOM_BUTTON_TOUCH_GPIO)
    init_button((gpio_num_t)CONFIG_CUSTOM_BUTTON_TOUCH_GPIO, "TOUCH Button");
#else
    init_button(GPIO_NUM_1, "TOUCH Button");
#endif
#endif

#if defined(CONFIG_ENABLE_BUTTON_WAKE)
    init_button(GPIO_NUM_47, "WAKE Button");
#endif
#endif

    // 2. Rotary Encoder EC11
#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_ROTARY_ENCODER) || defined(CONFIG_ENABLE_ROTARY_ENCODER)
    gpio_num_t pin_a = GPIO_NUM_17;
    gpio_num_t pin_b = GPIO_NUM_18;
    gpio_num_t pin_key = GPIO_NUM_21;

#if defined(CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_A)
        pin_a = (gpio_num_t)CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_A;
#elif defined(CONFIG_ROTARY_PIN_A)
    pin_a = (gpio_num_t)CONFIG_ROTARY_PIN_A;
#endif

#if defined(CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_B)
        pin_b = (gpio_num_t)CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_B;
#elif defined(CONFIG_ROTARY_PIN_B)
    pin_b = (gpio_num_t)CONFIG_ROTARY_PIN_B;
#endif

#if defined(CONFIG_CUSTOM_PERIPH_ENCODER_KEY_PIN)
        pin_key = (gpio_num_t)CONFIG_CUSTOM_PERIPH_ENCODER_KEY_PIN;
#elif defined(CONFIG_ROTARY_PIN_KEY)
    pin_key = (gpio_num_t)CONFIG_ROTARY_PIN_KEY;
#endif

    init_button(pin_a, "Rotary Phase A");
    init_button(pin_b, "Rotary Phase B");
    init_button(pin_key, "Rotary Key");
    ESP_LOGI(TAG, "Rotary Encoder initialized (A: %d, B: %d, KEY: %d)", pin_a, pin_b, pin_key);
#endif

    // 3. Capacitive Touch Screen & Gesture Sensor (via I2C Bus Manager)
#if defined(CONFIG_ENABLE_CUSTOM_TOUCH) || defined(CONFIG_ENABLE_CAPACITIVE_TOUCH_DISPLAY)
    ESP_LOGI(TAG, "Touch Screen controller mapped to I2C Bus (SDA: 8, SCL: 9, INT: 3, RST: 21)");
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_APDS9960) || defined(CONFIG_ENABLE_GESTURE_SENSOR)
    ESP_LOGI(TAG, "APDS-9960 Gesture sensor mapped to I2C Bus (SDA: 8, SCL: 9, INT: 21)");
#endif

    return ESP_OK;
}
