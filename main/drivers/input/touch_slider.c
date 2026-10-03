#include "drivers/input/touch_slider.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdlib.h>
#include <string.h>
#if SOC_TOUCH_SENSOR_SUPPORTED
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#include <driver/touch_pad.h>
#pragma GCC diagnostic pop
#endif
#define TAG "TouchSlider"
#define TOUCH_SLIDER_SAMPLES          8
#define TOUCH_SLIDER_DELTA_THRESHOLD  150  
typedef struct touch_slider_dev_t {
    gpio_num_t pins[3];
#if SOC_TOUCH_SENSOR_SUPPORTED
    touch_pad_t pads[3];
    uint32_t baselines[3];
#endif
    bool initialized;
    uint8_t last_position;
} touch_slider_dev_t;
#if SOC_TOUCH_SENSOR_SUPPORTED
static touch_pad_t gpio_to_touch_pad(gpio_num_t pin)
{
    if (pin >= GPIO_NUM_1 && pin <= GPIO_NUM_14) {
        return (touch_pad_t)pin;
    }
    return TOUCH_PAD_MAX;
}
#endif
esp_err_t touch_slider_init(gpio_num_t pad1_pin, gpio_num_t pad2_pin, gpio_num_t pad3_pin, touch_slider_handle_t *out_handle)
{
    if (!out_handle || !GPIO_IS_VALID_GPIO(pad1_pin) || !GPIO_IS_VALID_GPIO(pad2_pin) || !GPIO_IS_VALID_GPIO(pad3_pin)) {
        return ESP_ERR_INVALID_ARG;
    }
    touch_slider_dev_t *dev = (touch_slider_dev_t *)calloc(1, sizeof(touch_slider_dev_t));
    if (!dev) return ESP_ERR_NO_MEM;
    dev->pins[0] = pad1_pin;
    dev->pins[1] = pad2_pin;
    dev->pins[2] = pad3_pin;
#if SOC_TOUCH_SENSOR_SUPPORTED
    esp_err_t err = touch_pad_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "touch_pad_init failed: %s", esp_err_to_name(err));
        free(dev);
        return err;
    }
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_fsm_start();
    for (int i = 0; i < 3; i++) {
        dev->pads[i] = gpio_to_touch_pad(dev->pins[i]);
        if (dev->pads[i] == TOUCH_PAD_MAX) {
            ESP_LOGE(TAG, "GPIO %d is not a valid ESP32-S3 touch pad (must be GPIO 1..14)", (int)dev->pins[i]);
            free(dev);
            return ESP_ERR_INVALID_ARG;
        }
        touch_pad_config(dev->pads[i]);
    }
    vTaskDelay(pdMS_TO_TICKS(50));
    for (int i = 0; i < 3; i++) {
        uint32_t sum = 0;
        for (int s = 0; s < 16; s++) {
            uint32_t val = 0;
            touch_pad_read_raw_data(dev->pads[i], &val);
            sum += val;
            esp_rom_delay_us(100);
        }
        dev->baselines[i] = sum / 16;
        ESP_LOGI(TAG, "Touch Pad %d (GPIO %d) calibrated baseline: %lu", i + 1, (int)dev->pins[i], (unsigned long)dev->baselines[i]);
    }
    dev->initialized = true;
    *out_handle = dev;
    ESP_LOGI(TAG, "Touch Slider initialized on GPIOs [%d, %d, %d]",
             (int)dev->pins[0], (int)dev->pins[1], (int)dev->pins[2]);
    return ESP_OK;
#else
    ESP_LOGW(TAG, "Capacitive touch sensor is not supported on this target architecture");
    free(dev);
    return ESP_ERR_NOT_SUPPORTED;
#endif
}
esp_err_t touch_slider_read(touch_slider_handle_t handle, uint8_t *out_position_pct, bool *out_is_touched)
{
    if (!handle || !out_position_pct || !out_is_touched) return ESP_ERR_INVALID_ARG;
    if (!handle->initialized) return ESP_ERR_INVALID_STATE;
    *out_is_touched = false;
    *out_position_pct = handle->last_position;
#if SOC_TOUCH_SENSOR_SUPPORTED
    uint32_t deltas[3] = {0, 0, 0};
    bool touched = false;
    for (int i = 0; i < 3; i++) {
        uint32_t sum = 0;
        for (int s = 0; s < TOUCH_SLIDER_SAMPLES; s++) {
            uint32_t val = 0;
            touch_pad_read_raw_data(handle->pads[i], &val);
            sum += val;
        }
        uint32_t avg = sum / TOUCH_SLIDER_SAMPLES;
        if (avg < handle->baselines[i]) {
            deltas[i] = handle->baselines[i] - avg;
        } else {
            deltas[i] = 0;
        }
        if (deltas[i] > TOUCH_SLIDER_DELTA_THRESHOLD) {
            touched = true;
        }
    }
    if (!touched) {
        *out_is_touched = false;
        return ESP_OK;
    }
    uint32_t total_weight = deltas[0] + deltas[1] + deltas[2];
    if (total_weight > 0) {
        uint32_t weighted_pos = (deltas[0] * 0) + (deltas[1] * 50) + (deltas[2] * 100);
        uint8_t pos = (uint8_t)(weighted_pos / total_weight);
        if (pos > 100) pos = 100;
        handle->last_position = pos;
        *out_position_pct = pos;
        *out_is_touched = true;
    }
    return ESP_OK;
#else
    return ESP_ERR_NOT_SUPPORTED;
#endif
}
esp_err_t touch_slider_deinit(touch_slider_handle_t handle)
{
    if (!handle) return ESP_ERR_INVALID_ARG;
#if SOC_TOUCH_SENSOR_SUPPORTED
    touch_pad_deinit();
#endif
    free(handle);
    ESP_LOGI(TAG, "Touch Slider deinitialized");
    return ESP_OK;
}
