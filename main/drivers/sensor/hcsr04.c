/**
 * @file hcsr04.c
 * @brief HC-SR04 Ultrasonic Distance Sensor Driver (ESP-IDF 6.1)
 * Measures genuine distance using microsecond timing with zero simulated data.
 */

#include "hcsr04.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <esp_rom_sys.h>
#include <driver/gpio.h>
#include <stdlib.h>

#define TAG "Hcsr04"

#define HCSR04_TIMEOUT_START_US  5000   // 5ms max wait for echo HIGH to start
#define HCSR04_TIMEOUT_ECHO_US   30000  // 30ms max echo duration (~5.1 meters)

typedef struct hcsr04_dev_t {
    gpio_num_t trig_pin;
    gpio_num_t echo_pin;
} hcsr04_dev_t;

esp_err_t hcsr04_init(gpio_num_t trig_pin, gpio_num_t echo_pin, hcsr04_handle_t *out_handle)
{
    if (!out_handle || !GPIO_IS_VALID_GPIO(trig_pin) || !GPIO_IS_VALID_GPIO(echo_pin)) {
        return ESP_ERR_INVALID_ARG;
    }
    *out_handle = NULL;

    // Configure Trigger Pin: Output, Pull-down
    gpio_config_t trig_cfg = {
        .pin_bit_mask = (1ULL << trig_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&trig_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure Trigger GPIO %d: %s", (int)trig_pin, esp_err_to_name(err));
        return err;
    }
    gpio_set_level(trig_pin, 0);

    // Configure Echo Pin: Input, Pull-down
    gpio_config_t echo_cfg = {
        .pin_bit_mask = (1ULL << echo_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    err = gpio_config(&echo_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure Echo GPIO %d: %s", (int)echo_pin, esp_err_to_name(err));
        return err;
    }

    struct hcsr04_dev_t *dev = (struct hcsr04_dev_t *)calloc(1, sizeof(struct hcsr04_dev_t));
    if (!dev) return ESP_ERR_NO_MEM;

    dev->trig_pin = trig_pin;
    dev->echo_pin = echo_pin;
    *out_handle = dev;

    ESP_LOGI(TAG, "HC-SR04 Ultrasonic Sensor initialized (Trig: GPIO %d, Echo: GPIO %d)",
             (int)trig_pin, (int)echo_pin);
    return ESP_OK;
}

esp_err_t hcsr04_read_distance(hcsr04_handle_t dev, float *out_distance_cm)
{
    if (!dev || !out_distance_cm) return ESP_ERR_INVALID_ARG;
    *out_distance_cm = 0.0f;

    // Send 10µs HIGH pulse on Trigger pin
    gpio_set_level(dev->trig_pin, 0);
    esp_rom_delay_us(4);
    gpio_set_level(dev->trig_pin, 1);
    esp_rom_delay_us(10);
    gpio_set_level(dev->trig_pin, 0);

    // Wait for Echo to go HIGH
    int64_t t_wait = esp_timer_get_time();
    while (gpio_get_level(dev->echo_pin) == 0) {
        if ((esp_timer_get_time() - t_wait) > HCSR04_TIMEOUT_START_US) {
            return ESP_ERR_TIMEOUT;
        }
    }

    int64_t t_start = esp_timer_get_time();

    // Wait for Echo to go LOW
    while (gpio_get_level(dev->echo_pin) == 1) {
        if ((esp_timer_get_time() - t_start) > HCSR04_TIMEOUT_ECHO_US) {
            return ESP_ERR_TIMEOUT;
        }
    }

    int64_t t_end = esp_timer_get_time();
    int64_t duration_us = t_end - t_start;

    // Speed of sound: 343 m/s => distance = (duration * 0.0343) / 2 = duration / 58.2
    float dist = (float)duration_us / 58.0f;

    // Valid physical measurement range: 2cm to 400cm
    if (dist < 2.0f || dist > 400.0f) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    *out_distance_cm = dist;
    return ESP_OK;
}

void hcsr04_deinit(hcsr04_handle_t handle)
{
    if (handle) {
        free(handle);
    }
}
