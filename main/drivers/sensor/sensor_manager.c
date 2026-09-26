/**
 * @file sensor_manager.c
 * @brief Sensors Subsystem Manager Implementation (ESP-IDF 6.1)
 */

#include "drivers/sensor/sensor_manager.h"
#include "drivers/sensor/vl6180x.h"
#include "drivers/sensor/dht.h"
#include "boards/common/bus_manager.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <string.h>

#define TAG "SensorManager"

static adc_oneshot_unit_handle_t s_adc1_handle = NULL;
static vl6180x_handle_t s_vl6180x_dev = NULL;
static gpio_num_t s_dht_pin = GPIO_NUM_NC;
static gpio_num_t s_pir_pin = GPIO_NUM_NC;
static gpio_num_t s_vib_pin = GPIO_NUM_NC;
static gpio_num_t s_flame_pin = GPIO_NUM_NC;
static gpio_num_t s_chrg_pin = GPIO_NUM_NC;

esp_err_t sensor_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing Sensors Subsystem...");

    // 1. Initialize ADC1 Oneshot Unit for Analog Sensors (Gas MQ, LDR, Battery)
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    esp_err_t ret = adc_oneshot_new_unit(&init_config1, &s_adc1_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "ADC1 Oneshot Unit initialized successfully for analog sensors.");
        // Configure ADC channels with 12dB attenuation for 0-3.3V range
        adc_oneshot_chan_cfg_t chan_cfg = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        // Use graceful error handling instead of ESP_ERROR_CHECK to avoid abort on channel config fail
        esp_err_t ch_ret = adc_oneshot_config_channel(s_adc1_handle, ADC_CHANNEL_0, &chan_cfg);
        if (ch_ret != ESP_OK) {
            ESP_LOGW(TAG, "ADC1 CH0 (Gas/MQ) config failed: %s", esp_err_to_name(ch_ret));
        }
        ch_ret = adc_oneshot_config_channel(s_adc1_handle, ADC_CHANNEL_1, &chan_cfg);
        if (ch_ret != ESP_OK) {
            ESP_LOGW(TAG, "ADC1 CH1 (LDR) config failed: %s", esp_err_to_name(ch_ret));
        }
    } else {
        ESP_LOGW(TAG, "ADC1 Oneshot Unit init returned: %s — analog sensors (Gas, LDR) will use fallback values",
                 esp_err_to_name(ret));
        s_adc1_handle = NULL; // Ensure handle stays NULL if init failed
    }

    // 2. Configure Digital Sensors (DHT11/22, PIR, Vibration, Flame, TP4056)
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_DHT11_22)
#if defined(CONFIG_CUSTOM_SENSOR_DHT_GPIO) && (CONFIG_CUSTOM_SENSOR_DHT_GPIO >= 0)
    s_dht_pin = (gpio_num_t)CONFIG_CUSTOM_SENSOR_DHT_GPIO;
#else
    s_dht_pin = GPIO_NUM_NC;
#endif
    if (s_dht_pin != GPIO_NUM_NC) {
        dht_init(s_dht_pin);
        ESP_LOGI(TAG, "DHT11/22 Temperature & Humidity Sensor configured on user GPIO %d", s_dht_pin);
    }
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_PIR)
#if defined(CONFIG_CUSTOM_SENSOR_PIR_GPIO) && (CONFIG_CUSTOM_SENSOR_PIR_GPIO >= 0)
    s_pir_pin = (gpio_num_t)CONFIG_CUSTOM_SENSOR_PIR_GPIO;
#else
    s_pir_pin = GPIO_NUM_NC;
#endif
    if (s_pir_pin != GPIO_NUM_NC) {
        gpio_config_t pir_conf = {
            .pin_bit_mask = (1ULL << s_pir_pin),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_ENABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&pir_conf));
        ESP_LOGI(TAG, "PIR Motion Sensor configured on GPIO %d", s_pir_pin);
    }
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_VIBRATION_SW420)
#if defined(CONFIG_CUSTOM_SENSOR_VIBRATION_PIN) && (CONFIG_CUSTOM_SENSOR_VIBRATION_PIN >= 0)
    s_vib_pin = (gpio_num_t)CONFIG_CUSTOM_SENSOR_VIBRATION_PIN;
#else
    s_vib_pin = GPIO_NUM_NC;
#endif
    if (s_vib_pin != GPIO_NUM_NC) {
        gpio_config_t vib_conf = {
            .pin_bit_mask = (1ULL << s_vib_pin),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&vib_conf));
        ESP_LOGI(TAG, "SW-420 Vibration Sensor configured on GPIO %d", s_vib_pin);
    }
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_FLAME)
#if defined(CONFIG_CUSTOM_SENSOR_FLAME_PIN) && (CONFIG_CUSTOM_SENSOR_FLAME_PIN >= 0)
    s_flame_pin = (gpio_num_t)CONFIG_CUSTOM_SENSOR_FLAME_PIN;
#else
    s_flame_pin = GPIO_NUM_NC;
#endif
    if (s_flame_pin != GPIO_NUM_NC) {
        gpio_config_t flame_conf = {
            .pin_bit_mask = (1ULL << s_flame_pin),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&flame_conf));
        ESP_LOGI(TAG, "Flame Sensor configured on GPIO %d", s_flame_pin);
    }
#endif

#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_BATTERY_CHARGING_DETECT)
#if defined(CONFIG_CUSTOM_PERIPH_BATTERY_CHRG_PIN) && (CONFIG_CUSTOM_PERIPH_BATTERY_CHRG_PIN >= 0)
    s_chrg_pin = (gpio_num_t)CONFIG_CUSTOM_PERIPH_BATTERY_CHRG_PIN;
#else
    s_chrg_pin = GPIO_NUM_NC;
#endif
    if (s_chrg_pin != GPIO_NUM_NC) {
        gpio_config_t chrg_conf = {
            .pin_bit_mask = (1ULL << s_chrg_pin),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&chrg_conf));
        ESP_LOGI(TAG, "Battery Charging Monitor configured on GPIO %d", s_chrg_pin);
    }
#endif

    // LOG-01: Report I2C bus readiness for lazy-init sensors (VL6180X)
    i2c_master_bus_handle_t i2c_check = bus_manager_get_i2c_bus();
    if (i2c_check != NULL) {
        ESP_LOGI(TAG, "I2C Bus is available — VL6180X ToF/ALS will be initialized on first read.");
    } else {
        ESP_LOGW(TAG, "I2C Bus not yet set by board — VL6180X ToF/ALS will retry on first sensor read.");
    }

    ESP_LOGI(TAG, "Sensors Subsystem initialized successfully.");
    return ESP_OK;
}

static vl6180x_handle_t get_vl6180x_dev(void)
{
    if (s_vl6180x_dev != NULL) {
        return s_vl6180x_dev;
    }

    i2c_master_bus_handle_t i2c_bus = bus_manager_get_i2c_bus();
    if (i2c_bus == NULL) {
        // LOG-02: Clear warning — sensor read called before board set the I2C bus
        ESP_LOGD(TAG, "VL6180X lazy-init skipped: I2C bus not yet registered by board.");
        return NULL;
    }

    vl6180x_config_t cfg = {
        .i2c_bus = i2c_bus,
        .i2c_addr = 0x29,
        .xshut_pin = GPIO_NUM_NC,
        .gpio1_pin = GPIO_NUM_NC,
        .scaling = 1,
    };

    esp_err_t ret = vl6180x_init(&cfg, &s_vl6180x_dev);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "VL6180X Laser ToF Sensor initialized via shared I2C bus.");
    } else {
        ESP_LOGD(TAG, "VL6180X not detected on I2C bus: %s", esp_err_to_name(ret));
        s_vl6180x_dev = NULL;
    }
    return s_vl6180x_dev;
}

void sensor_set_dht_pin(gpio_num_t pin)
{
    s_dht_pin = pin;
    if (pin >= 0) {
        dht_init(pin);
        ESP_LOGI(TAG, "DHT11/22 dynamically reconfigured to user GPIO %d", pin);
    } else {
        ESP_LOGI(TAG, "DHT11/22 Sensor disabled (GPIO_NUM_NC)");
    }
}

gpio_num_t sensor_get_dht_pin(void)
{
    return s_dht_pin;
}

esp_err_t sensor_read_environment(sensor_environment_t *out_env)
{
    if (!out_env) return ESP_ERR_INVALID_ARG;
    memset(out_env, 0, sizeof(sensor_environment_t));

    // Baseline fallback readings (used when no real sensor is active)
    // temperature_c, humidity_pct: set to 0.0 — real values come from DHT below
    // pressure_hpa, co2_ppm, tvoc_ppb: no physical sensor driver exists yet — remain 0.0
    // MGR-BUG-01 fix: valid starts as false; only set true when real data is obtained
    out_env->valid = false;

    bool has_real_data = false;

    // 0. Read real-time Temperature & Humidity from DHT11 / DHT22 on user-selected GPIO
    if (s_dht_pin != GPIO_NUM_NC) {
        float dht_temp = 0.0f;
        float dht_hum = 0.0f;
        if (dht_read_data(s_dht_pin, &dht_temp, &dht_hum) == ESP_OK) {
            out_env->temperature_c = dht_temp;
            out_env->humidity_pct = dht_hum;
            has_real_data = true;
            ESP_LOGD(TAG, "DHT read OK on GPIO %d: Temp=%.1f C, Hum=%.1f%%", s_dht_pin, dht_temp, dht_hum);
        } else {
            ESP_LOGD(TAG, "DHT read failed on GPIO %d — using fallback Temp=%.1f C", s_dht_pin, out_env->temperature_c);
        }
    }

    // 1. Try reading Ambient Light Sensor (ALS) from VL6180X if available
    vl6180x_handle_t tof = get_vl6180x_dev();
    bool lux_read_ok = false;
    if (tof != NULL) {
        float tof_lux = 0.0f;
        if (vl6180x_read_ambient_lux(tof, &tof_lux) == ESP_OK) {
            out_env->light_lux = tof_lux;
            lux_read_ok = true;
            has_real_data = true;
        }
    }

    // 2. Read analog light sensor (LDR) if ADC1 is active and VL6180X lux was not available
    if (!lux_read_ok && s_adc1_handle) {
        int raw_val = 0;
        if (adc_oneshot_read(s_adc1_handle, ADC_CHANNEL_1, &raw_val) == ESP_OK) {
            // Convert ADC 12-bit (0..4095) to estimated lux range (0..1000)
            out_env->light_lux = (float)raw_val * (1000.0f / 4095.0f);
            has_real_data = true;
        }
    }

    out_env->valid = has_real_data;
    if (!has_real_data) {
        ESP_LOGD(TAG, "No real sensor data — environment struct contains fallback baseline values");
    }
    return ESP_OK;
}

esp_err_t sensor_read_distance(sensor_distance_t *out_dist)
{
    if (!out_dist) return ESP_ERR_INVALID_ARG;
    memset(out_dist, 0, sizeof(sensor_distance_t));

    // valid=false by default — only set true when real data is obtained
    out_dist->valid = false;

    // Read real-time distance from VL6180X Laser ToF Sensor
    vl6180x_handle_t tof = get_vl6180x_dev();
    if (tof != NULL) {
        float real_dist_mm = 0.0f;
        if (vl6180x_read_distance_mm(tof, &real_dist_mm) == ESP_OK) {
            out_dist->distance_laser_mm = real_dist_mm;
            out_dist->distance_ultrasonic_cm = real_dist_mm / 10.0f;
            out_dist->valid = true;
        } else {
            ESP_LOGD(TAG, "VL6180X distance read failed");
        }
    }
    // Ultrasonic (HC-SR04) not yet implemented — remains 0.0 when no ToF available

    return ESP_OK;
}

esp_err_t sensor_read_security(sensor_security_t *out_sec)
{
    if (!out_sec) return ESP_ERR_INVALID_ARG;
    memset(out_sec, 0, sizeof(sensor_security_t));

    bool has_real_data = false;

    // Read digital inputs (PIR, vibration, flame)
    if (s_pir_pin != GPIO_NUM_NC) {
        out_sec->motion_detected = (gpio_get_level(s_pir_pin) == 1);
        has_real_data = true;
    }
    if (s_vib_pin != GPIO_NUM_NC) {
        // SW-420 normally-closed: no vibration=LOW, vibration=HIGH (pull-up)
        out_sec->vibration_detected = (gpio_get_level(s_vib_pin) == 1);
        has_real_data = true;
    }
    if (s_flame_pin != GPIO_NUM_NC) {
        out_sec->flame_detected = (gpio_get_level(s_flame_pin) == 0); // Active low
        has_real_data = true;
    }

    // Read Gas Sensor (MQ-2) via ADC1 Channel 0
    // gas_level_ppm stays 0.0 if ADC not available (no hardcoded fake value)
    out_sec->gas_level_ppm = 0.0f;
    out_sec->gas_leak_alert = false;
    if (s_adc1_handle) {
        int raw_gas = 0;
        if (adc_oneshot_read(s_adc1_handle, ADC_CHANNEL_0, &raw_gas) == ESP_OK) {
            // Linear estimate: 0-1000 ppm range (MQ-2 requires calibration for precision)
            out_sec->gas_level_ppm = (float)raw_gas * (1000.0f / 4095.0f);
            if (out_sec->gas_level_ppm > 400.0f) {
                out_sec->gas_leak_alert = true;
            }
            has_real_data = true;
        }
    }

    out_sec->valid = has_real_data;
    return ESP_OK;
}

esp_err_t sensor_read_power(sensor_power_t *out_pwr)
{
    if (!out_pwr) return ESP_ERR_INVALID_ARG;
    memset(out_pwr, 0, sizeof(sensor_power_t));

    // No hardcoded fake values — all fields remain 0 until real hardware provides data
    out_pwr->valid = false;

    // Read charging status from TP4056 CHRG GPIO (active low = charging)
    if (s_chrg_pin != GPIO_NUM_NC) {
        out_pwr->is_charging = (gpio_get_level(s_chrg_pin) == 0);
        out_pwr->valid = true;
    }

    // NOTE: battery_voltage, battery_percentage, bus_current_ma, bus_power_mw
    // require a dedicated ADC battery monitor (e.g. AdcBatteryMonitor class)
    // or an INA219 power monitor on I2C — not yet implemented in C driver layer.
    // Those fields remain 0.0 / 0 until a real measurement path is added.

    return ESP_OK;
}

esp_err_t sensor_manager_read_all(sensor_data_t *out_data)
{
    if (!out_data) return ESP_ERR_INVALID_ARG;

    sensor_environment_t env;
    sensor_security_t sec;
    sensor_power_t pwr;

    sensor_read_environment(&env);
    sensor_read_security(&sec);
    sensor_read_power(&pwr);

    out_data->temperature_c = env.temperature_c;
    out_data->humidity_pct = env.humidity_pct;
    out_data->light_lux = env.light_lux;
    out_data->co2_ppm = env.co2_ppm;
    out_data->battery_voltage = pwr.battery_voltage;
    out_data->motion_detected = sec.motion_detected;
    out_data->vibration_detected = sec.vibration_detected;
    out_data->flame_detected = sec.flame_detected;

    return ESP_OK;
}
