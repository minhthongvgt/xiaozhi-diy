/**
 * @file rotary_encoder.c
 * @brief EC11 Rotary Encoder Driver Implementation (Real Quadrature Decoding)
 */

#include "drivers/input/rotary_encoder.h"
#include <esp_log.h>
#include <esp_attr.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <stdlib.h>
#include <string.h>

#define TAG "RotaryEncoder"

// Standard 4-bit Gray code transition table for quadrature encoders:
// Index = (prev_state << 2) | curr_state
// Values: 0 = no change/invalid/jitter, +1 = Clockwise, -1 = Counter-Clockwise
static const int8_t s_quad_lut[16] = {
     0,  1, -1,  0,
    -1,  0,  0,  1,
     1,  0,  0, -1,
     0, -1,  1,  0
};

typedef struct rotary_encoder_dev_t {
    gpio_num_t pin_a;
    gpio_num_t pin_b;
    gpio_num_t pin_key;
    uint8_t steps_per_detent;

    volatile uint8_t prev_state;
    volatile int8_t step_acc;
    volatile int32_t position;
    volatile int32_t last_read_pos;

    rotary_encoder_rotate_cb_t on_rotate;
    rotary_encoder_key_cb_t on_key;
    void *user_data;

    portMUX_TYPE spinlock;
} rotary_encoder_dev_t;

static void IRAM_ATTR rotary_isr_handler(void *arg)
{
    rotary_encoder_dev_t *dev = (rotary_encoder_dev_t *)arg;
    if (!dev) return;

    int a = gpio_get_level(dev->pin_a);
    int b = gpio_get_level(dev->pin_b);
    uint8_t curr = (uint8_t)(((a & 1) << 1) | (b & 1));

    portENTER_CRITICAL_ISR(&dev->spinlock);

    uint8_t trans = (uint8_t)((dev->prev_state << 2) | curr);
    dev->prev_state = curr;

    int8_t step = s_quad_lut[trans & 0x0F];
    if (step != 0) {
        dev->step_acc += step;
        int threshold = dev->steps_per_detent ? dev->steps_per_detent : 4;

        if (dev->step_acc >= threshold) {
            dev->position++;
            dev->step_acc = 0;
            if (dev->on_rotate) {
                dev->on_rotate(dev->position, 1, dev->user_data);
            }
        } else if (dev->step_acc <= -threshold) {
            dev->position--;
            dev->step_acc = 0;
            if (dev->on_rotate) {
                dev->on_rotate(dev->position, -1, dev->user_data);
            }
        }
    }

    portEXIT_CRITICAL_ISR(&dev->spinlock);
}

static void IRAM_ATTR key_isr_handler(void *arg)
{
    rotary_encoder_dev_t *dev = (rotary_encoder_dev_t *)arg;
    if (!dev || !GPIO_IS_VALID_GPIO(dev->pin_key)) return;

    bool pressed = (gpio_get_level(dev->pin_key) == 0); // Active low with internal pull-up
    if (dev->on_key) {
        dev->on_key(pressed, dev->user_data);
    }
}

esp_err_t rotary_encoder_init(const rotary_encoder_config_t *config, rotary_encoder_handle_t *out_handle)
{
    if (!config || !out_handle) return ESP_ERR_INVALID_ARG;
    if (!GPIO_IS_VALID_GPIO(config->pin_a) || !GPIO_IS_VALID_GPIO(config->pin_b)) {
        ESP_LOGE(TAG, "Invalid encoder pins (Phase A: %d, Phase B: %d)", (int)config->pin_a, (int)config->pin_b);
        return ESP_ERR_INVALID_ARG;
    }

    rotary_encoder_dev_t *dev = (rotary_encoder_dev_t *)calloc(1, sizeof(rotary_encoder_dev_t));
    if (!dev) return ESP_ERR_NO_MEM;

    dev->pin_a = config->pin_a;
    dev->pin_b = config->pin_b;
    dev->pin_key = config->pin_key;
    dev->steps_per_detent = config->steps_per_detent ? config->steps_per_detent : 4;
    dev->on_rotate = config->on_rotate;
    dev->on_key = config->on_key;
    dev->user_data = config->user_data;
    dev->spinlock = (portMUX_TYPE)portMUX_INITIALIZER_UNLOCKED;

    // 1. Configure Phase A & Phase B as inputs with pull-up and interrupt on all edges
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << dev->pin_a) | (1ULL << dev->pin_b),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO pins for Phase A/B: %s", esp_err_to_name(err));
        free(dev);
        return err;
    }

    // Read initial state
    int init_a = gpio_get_level(dev->pin_a);
    int init_b = gpio_get_level(dev->pin_b);
    dev->prev_state = (uint8_t)(((init_a & 1) << 1) | (init_b & 1));

    // 2. Install GPIO ISR service if not already installed
    gpio_install_isr_service(0);

    err = gpio_isr_handler_add(dev->pin_a, rotary_isr_handler, dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to attach ISR handler to Phase A: %s", esp_err_to_name(err));
        free(dev);
        return err;
    }
    err = gpio_isr_handler_add(dev->pin_b, rotary_isr_handler, dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to attach ISR handler to Phase B: %s", esp_err_to_name(err));
        gpio_isr_handler_remove(dev->pin_a);
        free(dev);
        return err;
    }

    // 3. Configure Key Pin (Optional push button)
    if (GPIO_IS_VALID_GPIO(dev->pin_key)) {
        gpio_config_t key_conf = {
            .pin_bit_mask = (1ULL << dev->pin_key),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = (dev->on_key != NULL) ? GPIO_INTR_ANYEDGE : GPIO_INTR_DISABLE,
        };
        gpio_config(&key_conf);
        if (dev->on_key != NULL) {
            gpio_isr_handler_add(dev->pin_key, key_isr_handler, dev);
        }
        ESP_LOGI(TAG, "Rotary Encoder Key configured on GPIO %d", (int)dev->pin_key);
    }

    *out_handle = dev;
    ESP_LOGI(TAG, "Rotary Encoder initialized (Phase A: %d, Phase B: %d, Steps/Detent: %d)",
             (int)dev->pin_a, (int)dev->pin_b, dev->steps_per_detent);
    return ESP_OK;
}

int32_t rotary_encoder_get_position(rotary_encoder_handle_t handle)
{
    if (!handle) return 0;
    portENTER_CRITICAL(&handle->spinlock);
    int32_t pos = handle->position;
    portEXIT_CRITICAL(&handle->spinlock);
    return pos;
}

int32_t rotary_encoder_get_diff(rotary_encoder_handle_t handle)
{
    if (!handle) return 0;
    portENTER_CRITICAL(&handle->spinlock);
    int32_t current = handle->position;
    int32_t diff = current - handle->last_read_pos;
    handle->last_read_pos = current;
    portEXIT_CRITICAL(&handle->spinlock);
    return diff;
}

void rotary_encoder_set_position(rotary_encoder_handle_t handle, int32_t position)
{
    if (!handle) return;
    portENTER_CRITICAL(&handle->spinlock);
    handle->position = position;
    handle->last_read_pos = position;
    handle->step_acc = 0;
    portEXIT_CRITICAL(&handle->spinlock);
}

bool rotary_encoder_is_key_pressed(rotary_encoder_handle_t handle)
{
    if (!handle || !GPIO_IS_VALID_GPIO(handle->pin_key)) {
        return false;
    }
    return (gpio_get_level(handle->pin_key) == 0);
}

esp_err_t rotary_encoder_deinit(rotary_encoder_handle_t handle)
{
    if (!handle) return ESP_ERR_INVALID_ARG;

    gpio_isr_handler_remove(handle->pin_a);
    gpio_isr_handler_remove(handle->pin_b);
    if (GPIO_IS_VALID_GPIO(handle->pin_key)) {
        gpio_isr_handler_remove(handle->pin_key);
    }

    free(handle);
    ESP_LOGI(TAG, "Rotary Encoder deinitialized");
    return ESP_OK;
}
