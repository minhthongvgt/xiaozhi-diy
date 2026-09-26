/**
 * @file dht.c
 * @brief DHT11 / DHT22 Single-Wire Temperature & Humidity Sensor Driver Implementation (ESP-IDF 6.1)
 */

#include "drivers/sensor/dht.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <rom/ets_sys.h>

#define TAG "DHT"

static portMUX_TYPE s_dht_mux = portMUX_INITIALIZER_UNLOCKED;

// Cache to prevent polling the sensor faster than its physical limit (1.5 - 2s)
static int64_t s_last_read_time_us = 0;
static float s_cached_temp = 25.0f;
static float s_cached_humidity = 50.0f;
static bool s_has_cached_value = false;
static gpio_num_t s_cached_pin = GPIO_NUM_NC;

esp_err_t dht_init(gpio_num_t pin)
{
    if (pin < 0) {
        return ESP_ERR_INVALID_ARG;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "DHT driver ready on GPIO %d (Internal Pull-Up enabled)", pin);
    }
    return ret;
}

static inline int wait_for_level(gpio_num_t pin, int level, uint32_t timeout_us)
{
    int64_t start = esp_timer_get_time();
    while (gpio_get_level(pin) == level) {
        if ((esp_timer_get_time() - start) > timeout_us) {
            return -1; // Timeout
        }
        ets_delay_us(1);
    }
    return (int)(esp_timer_get_time() - start);
}

esp_err_t dht_read_raw(gpio_num_t pin, dht_type_t type, float *out_temp, float *out_humidity)
{
    if (pin < 0 || !out_temp || !out_humidity) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[5] = {0};

    // 1. Send Host Start Signal
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0); // Pull LOW
    vTaskDelay(pdMS_TO_TICKS(20)); // DHT11 needs at least 18ms LOW

    // 2. Critical Section for microsecond pulse timings
    portENTER_CRITICAL(&s_dht_mux);

    gpio_set_level(pin, 1);
    ets_delay_us(30); // Float for 30us
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    // 3. Wait for sensor response: LOW ~80us, then HIGH ~80us
    if (wait_for_level(pin, 1, 100) < 0) {
        portEXIT_CRITICAL(&s_dht_mux);
        return ESP_ERR_TIMEOUT;
    }
    if (wait_for_level(pin, 0, 100) < 0) {
        portEXIT_CRITICAL(&s_dht_mux);
        return ESP_ERR_TIMEOUT;
    }
    if (wait_for_level(pin, 1, 100) < 0) {
        portEXIT_CRITICAL(&s_dht_mux);
        return ESP_ERR_TIMEOUT;
    }

    // 4. Read 40 data bits
    for (int i = 0; i < 40; ++i) {
        // Wait for leading 50us LOW pulse to end
        if (wait_for_level(pin, 0, 85) < 0) {
            portEXIT_CRITICAL(&s_dht_mux);
            return ESP_ERR_TIMEOUT;
        }

        // Measure HIGH pulse length: ~26-28us = 0, ~70us = 1
        int high_duration = wait_for_level(pin, 1, 100);
        if (high_duration < 0) {
            portEXIT_CRITICAL(&s_dht_mux);
            return ESP_ERR_TIMEOUT;
        }

        if (high_duration > 42) {
            data[i / 8] |= (1 << (7 - (i % 8)));
        }
    }

    portEXIT_CRITICAL(&s_dht_mux);

    // 5. Verify Checksum
    uint8_t checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
    if (data[4] != checksum) {
        ESP_LOGD(TAG, "Checksum mismatch on GPIO %d: calc 0x%02X != recv 0x%02X", pin, checksum, data[4]);
        return ESP_ERR_INVALID_CRC;
    }

    // 6. Decode temperature and humidity
    float hum = 0.0f;
    float temp = 0.0f;

    // DHT-BUG-02 fix: Improved AUTO detection
    // DHT22 uses 16-bit encoding where humidity = (data[0]<<8 | data[1]) * 0.1
    // DHT11 uses 8-bit integer: humidity = data[0] (1-99%), data[1] = decimal (usually 0)
    // Key difference: in DHT22 mode, the 16-bit combined value is typically > 100 (in tenths)
    //   e.g. 60.5% RH → 0x0259 = 601. data[0]=2, data[1]=89 (data[1] is rarely 0 in DHT22)
    // In DHT11 mode: data[1] is decimal part of humidity, usually 0 (DHT11 has no decimal)
    // Reliable heuristic: detect DHT22 if combined 16-bit humidity > 1000 (> 100.0%RH threshold
    //   is impossible so it can't be DHT11 integer), OR if data[1] > 9 (decimal part too large for DHT11)
    bool is_dht22 = false;
    if (type == DHT_TYPE_DHT22) {
        is_dht22 = true;
    } else if (type == DHT_TYPE_AUTO) {
        uint16_t combined_hum16 = ((uint16_t)data[0] << 8) | data[1];
        // DHT22: 16-bit value > 1000 means > 100.0% — impossible for DHT11 integer
        // DHT22: data[1] > 9 means decimal part is too large for DHT11 (DHT11 decimal is 0 or single digit)
        // DHT22: data[2] high byte (for temperature) having data[3] > 9 also suggests 16-bit encoding
        is_dht22 = (combined_hum16 > 1000) || (data[1] > 9) || (data[3] > 9);
    }

    if (is_dht22) {
        // DHT22 (AM2302) 16-bit encoding
        hum = (float)((data[0] << 8) | data[1]) * 0.1f;
        int16_t raw_temp = (int16_t)(((data[2] & 0x7F) << 8) | data[3]);
        if (data[2] & 0x80) {
            raw_temp = -raw_temp;
        }
        temp = (float)raw_temp * 0.1f;
    } else {
        // DHT11 integer + decimal
        hum = (float)data[0] + (float)data[1] * 0.1f;
        temp = (float)data[2] + (float)data[3] * 0.1f;
    }

    // Sanity checks on range
    if (hum < 0.0f || hum > 100.0f || temp < -40.0f || temp > 80.0f) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    *out_temp = temp;
    *out_humidity = hum;

    return ESP_OK;
}

esp_err_t dht_read_data(gpio_num_t pin, float *out_temp, float *out_humidity)
{
    if (pin < 0 || !out_temp || !out_humidity) {
        return ESP_ERR_INVALID_ARG;
    }

    int64_t now = esp_timer_get_time();

    // Invalidate cache if user reconfigured to a different pin
    if (s_cached_pin != pin) {
        s_has_cached_value = false;
        s_cached_pin = pin;
    }

    // Return cached value if sampled within last 2 seconds
    if (s_has_cached_value && (now - s_last_read_time_us < 2000000LL)) {
        *out_temp = s_cached_temp;
        *out_humidity = s_cached_humidity;
        return ESP_OK;
    }

    // Perform live sensor read
    esp_err_t ret = dht_read_raw(pin, DHT_TYPE_AUTO, out_temp, out_humidity);
    if (ret == ESP_OK) {
        s_cached_temp = *out_temp;
        s_cached_humidity = *out_humidity;
        s_last_read_time_us = now;
        s_has_cached_value = true;
        s_cached_pin = pin;
        ESP_LOGD(TAG, "DHT read success on GPIO %d: %.1f C, %.1f%%", pin, *out_temp, *out_humidity);
        return ESP_OK;
    }

    // If live read failed but we have a recent cached reading, preserve it
    if (s_has_cached_value) {
        *out_temp = s_cached_temp;
        *out_humidity = s_cached_humidity;
        return ESP_OK;
    }

    return ret;
}
