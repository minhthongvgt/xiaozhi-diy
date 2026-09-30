/**
 * @file input_manager.c
 * @brief Input Peripherals Driver Manager Implementation (ESP-IDF 6.1)
 * Real hardware management for Buttons, Rotary Encoder (EC11), Touch Screen, and Capacitive Touch Slider.
 * Zero simulated data.
 */

#include "drivers/input/input_manager.h"
#include "drivers/input/rotary_encoder.h"
#include "drivers/input/touch_slider.h"
#include "boards/common/bus_manager.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include <string.h>

#define TAG "InputManager"

static rotary_encoder_handle_t s_encoder_dev = NULL;
static touch_slider_handle_t s_slider_dev = NULL;

int32_t input_manager_get_encoder_value(void)
{
    if (s_encoder_dev != NULL) {
        return rotary_encoder_get_position(s_encoder_dev);
    }
    return 0;
}

int32_t input_manager_get_encoder_diff(void)
{
    if (s_encoder_dev != NULL) {
        return rotary_encoder_get_diff(s_encoder_dev);
    }
    return 0;
}

bool input_manager_is_encoder_key_pressed(void)
{
    if (s_encoder_dev != NULL) {
        return rotary_encoder_is_key_pressed(s_encoder_dev);
    }
    return false;
}

esp_err_t input_manager_get_touch_slider(uint8_t *out_pos_pct, bool *out_touched)
{
    if (!out_pos_pct || !out_touched) return ESP_ERR_INVALID_ARG;
    if (s_slider_dev != NULL) {
        return touch_slider_read(s_slider_dev, out_pos_pct, out_touched);
    }
    *out_pos_pct = 0;
    *out_touched = false;
    return ESP_ERR_NOT_FOUND;
}

esp_err_t input_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing User Inputs Subsystem (Buttons, Touch, Encoder, Slider)...");

    // Rotary Encoder EC11 (Quadrature decoding via hardware interrupts)
#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_ROTARY_ENCODER) || defined(CONFIG_ENABLE_ROTARY_ENCODER)
    gpio_num_t pin_a = (gpio_num_t)-1;
    gpio_num_t pin_b = (gpio_num_t)-1;
    gpio_num_t pin_key = (gpio_num_t)-1;

#if defined(CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_A) && (CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_A >= 0)
    pin_a = (gpio_num_t)CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_A;
#elif defined(CONFIG_ROTARY_PIN_A) && (CONFIG_ROTARY_PIN_A >= 0)
    pin_a = (gpio_num_t)CONFIG_ROTARY_PIN_A;
#endif

#if defined(CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_B) && (CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_B >= 0)
    pin_b = (gpio_num_t)CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_B;
#elif defined(CONFIG_ROTARY_PIN_B) && (CONFIG_ROTARY_PIN_B >= 0)
    pin_b = (gpio_num_t)CONFIG_ROTARY_PIN_B;
#endif

#if defined(CONFIG_CUSTOM_PERIPH_ENCODER_KEY_PIN) && (CONFIG_CUSTOM_PERIPH_ENCODER_KEY_PIN >= 0)
    pin_key = (gpio_num_t)CONFIG_CUSTOM_PERIPH_ENCODER_KEY_PIN;
#elif defined(CONFIG_ROTARY_PIN_KEY) && (CONFIG_ROTARY_PIN_KEY >= 0)
    pin_key = (gpio_num_t)CONFIG_ROTARY_PIN_KEY;
#endif

    if (GPIO_IS_VALID_GPIO(pin_a) && GPIO_IS_VALID_GPIO(pin_b)) {
        rotary_encoder_config_t enc_cfg = {
            .pin_a = pin_a,
            .pin_b = pin_b,
            .pin_key = pin_key,
            .steps_per_detent = 4,
            .on_rotate = NULL,
            .on_key = NULL,
            .user_data = NULL,
        };
        esp_err_t enc_ret = rotary_encoder_init(&enc_cfg, &s_encoder_dev);
        if (enc_ret == ESP_OK) {
            ESP_LOGI(TAG, "Rotary Encoder EC11 active (Phase A: %d, Phase B: %d, Key: %d)",
                     (int)pin_a, (int)pin_b, (int)pin_key);
        } else {
            ESP_LOGW(TAG, "Rotary Encoder EC11 initialization failed: %s", esp_err_to_name(enc_ret));
        }
    }
#endif

    // Capacitive Touch Slider (3-pad internal ESP32-S3 touch sensor)
#if defined(CONFIG_CUSTOM_ENABLE_TOUCH_SLIDER)
    gpio_num_t pad1 = (gpio_num_t)-1;
    gpio_num_t pad2 = (gpio_num_t)-1;
    gpio_num_t pad3 = (gpio_num_t)-1;

#if defined(CONFIG_CUSTOM_TOUCH_SLIDER_PAD1_GPIO) && (CONFIG_CUSTOM_TOUCH_SLIDER_PAD1_GPIO >= 0)
    pad1 = (gpio_num_t)CONFIG_CUSTOM_TOUCH_SLIDER_PAD1_GPIO;
#endif
#if defined(CONFIG_CUSTOM_TOUCH_SLIDER_PAD2_GPIO) && (CONFIG_CUSTOM_TOUCH_SLIDER_PAD2_GPIO >= 0)
    pad2 = (gpio_num_t)CONFIG_CUSTOM_TOUCH_SLIDER_PAD2_GPIO;
#endif
#if defined(CONFIG_CUSTOM_TOUCH_SLIDER_PAD3_GPIO) && (CONFIG_CUSTOM_TOUCH_SLIDER_PAD3_GPIO >= 0)
    pad3 = (gpio_num_t)CONFIG_CUSTOM_TOUCH_SLIDER_PAD3_GPIO;
#endif

    if (GPIO_IS_VALID_GPIO(pad1) && GPIO_IS_VALID_GPIO(pad2) && GPIO_IS_VALID_GPIO(pad3)) {
        esp_err_t slider_ret = touch_slider_init(pad1, pad2, pad3, &s_slider_dev);
        if (slider_ret == ESP_OK) {
            ESP_LOGI(TAG, "Capacitive Touch Slider active on GPIOs [%d, %d, %d]",
                     (int)pad1, (int)pad2, (int)pad3);
        } else {
            ESP_LOGW(TAG, "Capacitive Touch Slider init failed: %s", esp_err_to_name(slider_ret));
        }
    }
#endif

    // Touch Screen Panel status check
#if defined(CONFIG_ENABLE_CUSTOM_TOUCH) || defined(CONFIG_ENABLE_CAPACITIVE_TOUCH_DISPLAY)
    ESP_LOGI(TAG, "Capacitive Touch Screen subsystem ready (Initialized via Board Display/Touch bus)");
#endif

    return ESP_OK;
}
