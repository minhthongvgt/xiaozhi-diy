#include "drivers/input/touch_slider.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <soc/soc_caps.h>
#include <stdlib.h>
#include <string.h>
#if SOC_TOUCH_SENSOR_SUPPORTED && SOC_TOUCH_SENSOR_VERSION == 2
#include <driver/touch_sens.h>
#define TOUCH_SLIDER_HW_SUPPORTED 1
#else
#define TOUCH_SLIDER_HW_SUPPORTED 0
#endif
#define TAG "TouchSlider"
#define TOUCH_SLIDER_PAD_COUNT              3
#define TOUCH_SLIDER_SAMPLES                8
#define TOUCH_SLIDER_BASELINE_SAMPLES       16
#define TOUCH_SLIDER_BASELINE_INTERVAL_MS   5
#define TOUCH_SLIDER_SETTLE_MS              100
#define TOUCH_SLIDER_ACTIVE_PERMILLE        15
#define TOUCH_SLIDER_MIN_ACTIVE_DELTA       100
#define TOUCH_SLIDER_CHARGE_TIMES           500
#define TOUCH_SLIDER_CHANNEL_THRESHOLD      2000
typedef struct touch_slider_dev_t {
    gpio_num_t pins[TOUCH_SLIDER_PAD_COUNT];
#if TOUCH_SLIDER_HW_SUPPORTED
    touch_sensor_handle_t sensor;
    touch_channel_handle_t channels[TOUCH_SLIDER_PAD_COUNT];
    uint32_t baselines[TOUCH_SLIDER_PAD_COUNT];
    uint32_t active_deltas[TOUCH_SLIDER_PAD_COUNT];
    bool scanning;
#endif
    bool initialized;
    uint8_t last_position;
} touch_slider_dev_t;
#if TOUCH_SLIDER_HW_SUPPORTED
static bool touch_slider_pin_is_touch_channel(gpio_num_t pin)
{
    return (int)pin >= (int)SOC_TOUCH_MIN_CHAN_ID && (int)pin <= (int)SOC_TOUCH_MAX_CHAN_ID;
}
static void touch_slider_release_hw(touch_slider_dev_t *dev)
{
    if (dev->sensor == NULL) {
        return;
    }
    if (dev->scanning) {
        touch_sensor_stop_continuous_scanning(dev->sensor);
        touch_sensor_disable(dev->sensor);
        dev->scanning = false;
    }
    for (int i = 0; i < TOUCH_SLIDER_PAD_COUNT; i++) {
        if (dev->channels[i] != NULL) {
            touch_sensor_del_channel(dev->channels[i]);
            dev->channels[i] = NULL;
        }
    }
    touch_sensor_del_controller(dev->sensor);
    dev->sensor = NULL;
}
static uint32_t touch_slider_read_average(touch_channel_handle_t channel, int samples)
{
    uint64_t sum = 0;
    int valid = 0;
    for (int s = 0; s < samples; s++) {
        uint32_t value = 0;
        if (touch_channel_read_data(channel, TOUCH_CHAN_DATA_TYPE_SMOOTH, &value) == ESP_OK) {
            sum += value;
            valid++;
        }
    }
    return valid > 0 ? (uint32_t)(sum / (uint64_t)valid) : 0;
}
static esp_err_t touch_slider_start_hw(touch_slider_dev_t *dev)
{
    touch_sensor_sample_config_t sample_cfg[1] = {
        TOUCH_SENSOR_V2_DEFAULT_SAMPLE_CONFIG(TOUCH_SLIDER_CHARGE_TIMES, TOUCH_VOLT_LIM_L_0V5, TOUCH_VOLT_LIM_H_2V2),
    };
    touch_sensor_config_t sensor_cfg = TOUCH_SENSOR_DEFAULT_BASIC_CONFIG(1, sample_cfg);
    esp_err_t err = touch_sensor_new_controller(&sensor_cfg, &dev->sensor);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "touch_sensor_new_controller failed: %s", esp_err_to_name(err));
        dev->sensor = NULL;
        return err;
    }
    touch_channel_config_t channel_cfg = {
        .active_thresh = {TOUCH_SLIDER_CHANNEL_THRESHOLD},
        .charge_speed = TOUCH_CHARGE_SPEED_7,
        .init_charge_volt = TOUCH_INIT_CHARGE_VOLT_DEFAULT,
    };
    for (int i = 0; i < TOUCH_SLIDER_PAD_COUNT; i++) {
        err = touch_sensor_new_channel(dev->sensor, (int)dev->pins[i], &channel_cfg, &dev->channels[i]);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "touch_sensor_new_channel(GPIO %d) failed: %s", (int)dev->pins[i], esp_err_to_name(err));
            return err;
        }
    }
    touch_sensor_filter_config_t filter_cfg = TOUCH_SENSOR_DEFAULT_FILTER_CONFIG();
    err = touch_sensor_config_filter(dev->sensor, &filter_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "touch_sensor_config_filter failed: %s", esp_err_to_name(err));
        return err;
    }
    err = touch_sensor_enable(dev->sensor);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "touch_sensor_enable failed: %s", esp_err_to_name(err));
        return err;
    }
    err = touch_sensor_start_continuous_scanning(dev->sensor);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "touch_sensor_start_continuous_scanning failed: %s", esp_err_to_name(err));
        touch_sensor_disable(dev->sensor);
        return err;
    }
    dev->scanning = true;
    return ESP_OK;
}
static void touch_slider_calibrate(touch_slider_dev_t *dev)
{
    vTaskDelay(pdMS_TO_TICKS(TOUCH_SLIDER_SETTLE_MS));
    for (int i = 0; i < TOUCH_SLIDER_PAD_COUNT; i++) {
        uint64_t sum = 0;
        for (int s = 0; s < TOUCH_SLIDER_BASELINE_SAMPLES; s++) {
            sum += touch_slider_read_average(dev->channels[i], 1);
            vTaskDelay(pdMS_TO_TICKS(TOUCH_SLIDER_BASELINE_INTERVAL_MS));
        }
        dev->baselines[i] = (uint32_t)(sum / TOUCH_SLIDER_BASELINE_SAMPLES);
        uint32_t ratio_delta = (uint32_t)(((uint64_t)dev->baselines[i] * TOUCH_SLIDER_ACTIVE_PERMILLE) / 1000U);
        dev->active_deltas[i] = ratio_delta > TOUCH_SLIDER_MIN_ACTIVE_DELTA ? ratio_delta : TOUCH_SLIDER_MIN_ACTIVE_DELTA;
        ESP_LOGI(TAG, "Touch Pad %d (GPIO %d) baseline %lu, active delta %lu", i + 1, (int)dev->pins[i],
                 (unsigned long)dev->baselines[i], (unsigned long)dev->active_deltas[i]);
    }
}
#endif
esp_err_t touch_slider_init(gpio_num_t pad1_pin, gpio_num_t pad2_pin, gpio_num_t pad3_pin, touch_slider_handle_t *out_handle)
{
    if (!out_handle || !GPIO_IS_VALID_GPIO(pad1_pin) || !GPIO_IS_VALID_GPIO(pad2_pin) || !GPIO_IS_VALID_GPIO(pad3_pin)) {
        return ESP_ERR_INVALID_ARG;
    }
#if TOUCH_SLIDER_HW_SUPPORTED
    touch_slider_dev_t *dev = (touch_slider_dev_t *)calloc(1, sizeof(touch_slider_dev_t));
    if (!dev) {
        return ESP_ERR_NO_MEM;
    }
    dev->pins[0] = pad1_pin;
    dev->pins[1] = pad2_pin;
    dev->pins[2] = pad3_pin;
    for (int i = 0; i < TOUCH_SLIDER_PAD_COUNT; i++) {
        if (!touch_slider_pin_is_touch_channel(dev->pins[i])) {
            ESP_LOGE(TAG, "GPIO %d is not a touch pad (must be GPIO %d..%d)", (int)dev->pins[i],
                     (int)SOC_TOUCH_MIN_CHAN_ID, (int)SOC_TOUCH_MAX_CHAN_ID);
            free(dev);
            return ESP_ERR_INVALID_ARG;
        }
    }
    esp_err_t err = touch_slider_start_hw(dev);
    if (err != ESP_OK) {
        touch_slider_release_hw(dev);
        free(dev);
        return err;
    }
    touch_slider_calibrate(dev);
    dev->initialized = true;
    *out_handle = dev;
    ESP_LOGI(TAG, "Touch Slider initialized on GPIOs [%d, %d, %d]",
             (int)dev->pins[0], (int)dev->pins[1], (int)dev->pins[2]);
    return ESP_OK;
#else
    ESP_LOGW(TAG, "Touch slider requires a touch sensor v2 target (ESP32-S2/S3)");
    return ESP_ERR_NOT_SUPPORTED;
#endif
}
esp_err_t touch_slider_read(touch_slider_handle_t handle, uint8_t *out_position_pct, bool *out_is_touched)
{
    if (!handle || !out_position_pct || !out_is_touched) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!handle->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    *out_is_touched = false;
    *out_position_pct = handle->last_position;
#if TOUCH_SLIDER_HW_SUPPORTED
    uint32_t deltas[TOUCH_SLIDER_PAD_COUNT] = {0};
    bool touched = false;
    for (int i = 0; i < TOUCH_SLIDER_PAD_COUNT; i++) {
        uint32_t average = touch_slider_read_average(handle->channels[i], TOUCH_SLIDER_SAMPLES);
        deltas[i] = average > handle->baselines[i] ? average - handle->baselines[i] : 0;
        if (deltas[i] > handle->active_deltas[i]) {
            touched = true;
        }
    }
    if (!touched) {
        return ESP_OK;
    }
    uint32_t total_weight = deltas[0] + deltas[1] + deltas[2];
    if (total_weight > 0) {
        uint32_t weighted_pos = (deltas[1] * 50U) + (deltas[2] * 100U);
        uint32_t position = weighted_pos / total_weight;
        handle->last_position = (uint8_t)(position > 100U ? 100U : position);
        *out_position_pct = handle->last_position;
        *out_is_touched = true;
    }
    return ESP_OK;
#else
    return ESP_ERR_NOT_SUPPORTED;
#endif
}
esp_err_t touch_slider_deinit(touch_slider_handle_t handle)
{
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
#if TOUCH_SLIDER_HW_SUPPORTED
    touch_slider_release_hw(handle);
#endif
    free(handle);
    ESP_LOGI(TAG, "Touch Slider deinitialized");
    return ESP_OK;
}
