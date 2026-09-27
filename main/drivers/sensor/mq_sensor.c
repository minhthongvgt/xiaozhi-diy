/**
 * @file mq_sensor.c
 * @brief MQ-2 / MQ-135 Analog Gas Sensor Driver (ESP-IDF 6.1)
 * Eliminates simulated/fake values by performing physical SnO2 resistance
 * logarithmic curve calculations and hardware disconnection detection.
 */

#include "mq_sensor.h"
#include <esp_log.h>
#include <math.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "MqSensor"

// Operational voltage boundaries for connected MQ-2/135 sensor (in mV)
// Active heater + RL divider yields baseline > 120mV in clean air.
// Anything below 120mV indicates open-circuit, disconnected, or floating pin.
#define MQ_MIN_VALID_VOLTAGE_MV   120.0f
#define MQ_MAX_VALID_VOLTAGE_MV   3250.0f

// Number of oversamples for ADC noise suppression
#define MQ_ADC_OVERSAMPLES        8

typedef struct mq_sensor_dev_t {
    adc_oneshot_unit_handle_t adc_handle;
    adc_channel_t             channel;
    gpio_num_t                pin;
    mq_sensor_type_t          type;
    float                     rl_kohm;
    float                     r0_kohm;
} mq_sensor_dev_t;

static esp_err_t gpio_to_adc1_channel(gpio_num_t pin, adc_channel_t *out_chan)
{
    // ESP32-S3 ADC1 Channel Mapping:
    // GPIO 1 -> ADC1_CH0 ... GPIO 10 -> ADC1_CH9
    if (pin >= GPIO_NUM_1 && pin <= GPIO_NUM_10) {
        *out_chan = (adc_channel_t)(pin - GPIO_NUM_1);
        return ESP_OK;
    }
    return ESP_ERR_INVALID_ARG;
}

esp_err_t mq_sensor_init(adc_oneshot_unit_handle_t adc1_handle,
                         const mq_sensor_config_t *config,
                         mq_sensor_handle_t *out_handle)
{
    if (!config || !out_handle || adc1_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    adc_channel_t chan;
    esp_err_t err = gpio_to_adc1_channel(config->pin, &chan);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GPIO %d is not a valid ADC1 channel on ESP32-S3 (must be GPIO 1 - 10)", (int)config->pin);
        return ESP_ERR_INVALID_ARG;
    }

    // Configure ADC channel with 12dB attenuation (0 - 3.3V range)
    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    err = adc_oneshot_config_channel(adc1_handle, chan, &chan_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ADC1 channel %d (GPIO %d): %s",
                 (int)chan, (int)config->pin, esp_err_to_name(err));
        return err;
    }

    struct mq_sensor_dev_t *dev = (struct mq_sensor_dev_t *)calloc(1, sizeof(struct mq_sensor_dev_t));
    if (!dev) {
        return ESP_ERR_NO_MEM;
    }

    dev->adc_handle = adc1_handle;
    dev->channel = chan;
    dev->pin = config->pin;
    dev->type = config->type;
    dev->rl_kohm = (config->rl_kohm > 0.1f) ? config->rl_kohm : 10.0f;
    dev->r0_kohm = (config->r0_kohm > 0.1f) ? config->r0_kohm : 10.0f;

    *out_handle = dev;
    ESP_LOGI(TAG, "MQ Gas Sensor initialized on GPIO %d (ADC1_CH%d, Model: %s, RL: %.1f kOhm, R0: %.1f kOhm)",
             (int)dev->pin, (int)dev->channel,
             (dev->type == MQ_TYPE_MQ2) ? "MQ-2" : "MQ-135",
             dev->rl_kohm, dev->r0_kohm);

    return ESP_OK;
}

esp_err_t mq_sensor_read(mq_sensor_handle_t handle, float *out_ppm, float *out_voltage_mv)
{
    if (!handle || !out_ppm || !out_voltage_mv) {
        return ESP_ERR_INVALID_ARG;
    }

    *out_ppm = 0.0f;
    *out_voltage_mv = 0.0f;

    // 1. Multi-sample ADC reading to filter high-frequency switching noise
    int raw_sum = 0;
    int valid_samples = 0;
    for (int i = 0; i < MQ_ADC_OVERSAMPLES; i++) {
        int sample = 0;
        if (adc_oneshot_read(handle->adc_handle, handle->channel, &sample) == ESP_OK) {
            raw_sum += sample;
            valid_samples++;
        }
        if (i < MQ_ADC_OVERSAMPLES - 1) {
            vTaskDelay(pdMS_TO_TICKS(2));
        }
    }

    if (valid_samples == 0) {
        ESP_LOGD(TAG, "ADC read failed on channel %d", (int)handle->channel);
        return ESP_FAIL;
    }

    float raw_avg = (float)raw_sum / (float)valid_samples;

    // 2. Real Voltage Calculation: 12-bit (0-4095) with 12dB atten (~3300mV full scale)
    float v_mv = (raw_avg * 3300.0f) / 4095.0f;
    *out_voltage_mv = v_mv;

    // 3. Hardware Disconnection / Open-Circuit Check
    // If voltage is below 120mV or above 3250mV, the sensor is not physically plugged in.
    if (v_mv < MQ_MIN_VALID_VOLTAGE_MV || v_mv > MQ_MAX_VALID_VOLTAGE_MV) {
        ESP_LOGD(TAG, "Sensor disconnected or floating noise on GPIO %d (measured %.1f mV)",
                 (int)handle->pin, v_mv);
        return ESP_ERR_NOT_FOUND;
    }

    // 4. Physical Sensor Resistance (Rs) Calculation
    // Circuit: Vcc (3.3V) -> Rs -> Vout -> RL -> GND
    // Vout = Vcc * (RL / (Rs + RL))  =>  Rs = RL * (Vcc - Vout) / Vout
    float v_volts = v_mv / 1000.0f;
    float rs_kohm = handle->rl_kohm * (3.3f - v_volts) / v_volts;
    if (rs_kohm <= 0.01f) {
        rs_kohm = 0.01f;
    }

    // 5. Resistance Ratio: Rs / R0
    float ratio = rs_kohm / handle->r0_kohm;

    // 6. Real PPM Calculation via Datasheet Power-Law Fit: PPM = A * (Rs/R0)^B
    float ppm = 0.0f;
    if (handle->type == MQ_TYPE_MQ2) {
        // MQ-2 Datasheet curve for Combustible Gas / Smoke:
        // A = 658.71, B = -2.168 (Range: 200 - 10000 ppm)
        // Clean air ratio is typically >= 9.8.
        if (ratio >= 9.5f) {
            ppm = 0.0f; // Ambient clean air, no gas detected
        } else {
            ppm = 658.71f * powf(ratio, -2.168f);
            if (ppm < 10.0f) ppm = 10.0f;
            if (ppm > 10000.0f) ppm = 10000.0f;
        }
    } else {
        // MQ-135 Datasheet curve for Air Quality / Smoke / NH3:
        // A = 116.6, B = -2.76 (Range: 10 - 1000 ppm)
        // Clean air ratio is typically >= 3.6.
        if (ratio >= 3.5f) {
            ppm = 0.0f; // Ambient clean air
        } else {
            ppm = 116.6f * powf(ratio, -2.76f);
            if (ppm < 5.0f) ppm = 5.0f;
            if (ppm > 1000.0f) ppm = 1000.0f;
        }
    }

    *out_ppm = ppm;
    return ESP_OK;
}

bool mq_sensor_is_leak_alert(mq_sensor_handle_t handle, float current_ppm)
{
    if (!handle) return false;

    if (handle->type == MQ_TYPE_MQ2) {
        // MQ-2 danger threshold: LPG / Combustible gas exceeds 800 ppm
        return (current_ppm >= 800.0f);
    } else {
        // MQ-135 danger threshold: Harmful pollutants exceed 300 ppm
        return (current_ppm >= 300.0f);
    }
}

esp_err_t mq_sensor_calibrate(mq_sensor_handle_t handle, float *out_r0)
{
    if (!handle || !out_r0) return ESP_ERR_INVALID_ARG;

    // Take 20 samples over 2 seconds in clean air
    int raw_sum = 0;
    int samples = 20;
    for (int i = 0; i < samples; i++) {
        int raw = 0;
        if (adc_oneshot_read(handle->adc_handle, handle->channel, &raw) != ESP_OK) {
            return ESP_FAIL;
        }
        raw_sum += raw;
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    float raw_avg = (float)raw_sum / (float)samples;
    float v_mv = (raw_avg * 3300.0f) / 4095.0f;

    if (v_mv < MQ_MIN_VALID_VOLTAGE_MV || v_mv > MQ_MAX_VALID_VOLTAGE_MV) {
        return ESP_ERR_INVALID_STATE; // Disconnected or faulty
    }

    float v_volts = v_mv / 1000.0f;
    float rs_clean = handle->rl_kohm * (3.3f - v_volts) / v_volts;

    // R0 in clean air:
    // For MQ-2, clean air Rs/R0 = 9.83 => R0 = Rs / 9.83
    // For MQ-135, clean air Rs/R0 = 3.6  => R0 = Rs / 3.6
    float air_factor = (handle->type == MQ_TYPE_MQ2) ? 9.83f : 3.6f;
    handle->r0_kohm = rs_clean / air_factor;
    *out_r0 = handle->r0_kohm;

    ESP_LOGI(TAG, "MQ Sensor calibrated: Rs_clean=%.2f kOhm, New R0=%.2f kOhm", rs_clean, handle->r0_kohm);
    return ESP_OK;
}

void mq_sensor_deinit(mq_sensor_handle_t handle)
{
    if (handle) {
        free(handle);
    }
}
