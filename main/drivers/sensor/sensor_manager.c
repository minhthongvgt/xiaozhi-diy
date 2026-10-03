#include "drivers/sensor/sensor_manager.h"
#include "drivers/sensor/vl6180x.h"
#include "drivers/sensor/dht.h"
#include "drivers/sensor/mq_sensor.h"
#include "drivers/sensor/scd4x.h"
#include "drivers/sensor/bmp280.h"
#include "drivers/sensor/aht20.h"
#include "drivers/sensor/bh1750.h"
#include "drivers/sensor/hcsr04.h"
#include "drivers/sensor/ina219.h"
#include "boards/common/bus_manager.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <string.h>
#define TAG "SensorManager"
static adc_oneshot_unit_handle_t s_adc1_handle = NULL;
static vl6180x_handle_t s_vl6180x_dev = NULL;
static mq_sensor_handle_t s_mq_dev = NULL;
static scd4x_handle_t s_scd4x_dev = NULL;
static bmp280_handle_t s_bmp280_dev = NULL;
static aht20_handle_t s_aht20_dev = NULL;
static bh1750_handle_t s_bh1750_dev = NULL;
static hcsr04_handle_t s_hcsr04_dev = NULL;
static ina219_handle_t s_ina219_dev = NULL;
static gpio_num_t s_dht_pin = (gpio_num_t)-1;
static gpio_num_t s_mq_pin = (gpio_num_t)-1;
static gpio_num_t s_pir_pin = (gpio_num_t)-1;
static gpio_num_t s_vib_pin = (gpio_num_t)-1;
static gpio_num_t s_flame_pin = (gpio_num_t)-1;
static gpio_num_t s_chrg_pin = (gpio_num_t)-1;
static gpio_num_t s_hcsr04_trig = (gpio_num_t)-1;
static gpio_num_t s_hcsr04_echo = (gpio_num_t)-1;
esp_err_t sensor_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing Sensors Subsystem...");
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    esp_err_t ret = adc_oneshot_new_unit(&init_config1, &s_adc1_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "ADC1 Oneshot Unit initialized successfully for analog sensors.");
        adc_oneshot_chan_cfg_t chan_cfg = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        esp_err_t ch_ret = adc_oneshot_config_channel(s_adc1_handle, ADC_CHANNEL_1, &chan_cfg);
        if (ch_ret != ESP_OK) {
            ESP_LOGW(TAG, "ADC1 CH1 (LDR) config failed: %s", esp_err_to_name(ch_ret));
        }
    } else {
        ESP_LOGW(TAG, "ADC1 Oneshot Unit init returned: %s — analog sensors will not be available",
                 esp_err_to_name(ret));
        s_adc1_handle = NULL;
    }
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_GAS_ANALOG_MQ)
#if defined(CONFIG_CUSTOM_SENSOR_MQ_ANALOG_PIN) && (CONFIG_CUSTOM_SENSOR_MQ_ANALOG_PIN >= 0)
    s_mq_pin = (gpio_num_t)CONFIG_CUSTOM_SENSOR_MQ_ANALOG_PIN;
#else
    s_mq_pin = (gpio_num_t)-1;
#endif
    if (GPIO_IS_VALID_GPIO(s_mq_pin) && s_adc1_handle != NULL) {
        mq_sensor_config_t mq_cfg = {
            .pin = s_mq_pin,
            .type = MQ_TYPE_MQ2,
            .rl_kohm = 10.0f,
            .r0_kohm = 10.0f,
        };
        esp_err_t mq_ret = mq_sensor_init(s_adc1_handle, &mq_cfg, &s_mq_dev);
        if (mq_ret == ESP_OK) {
            ESP_LOGI(TAG, "MQ-2 Gas Sensor driver initialized on GPIO %d", (int)s_mq_pin);
        } else {
            ESP_LOGW(TAG, "MQ-2 Gas Sensor init failed on GPIO %d: %s", (int)s_mq_pin, esp_err_to_name(mq_ret));
            s_mq_dev = NULL;
        }
    }
#else
    s_mq_pin = (gpio_num_t)-1;
    s_mq_dev = NULL;
#endif
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_DHT11_22)
#if defined(CONFIG_CUSTOM_SENSOR_DHT_GPIO) && (CONFIG_CUSTOM_SENSOR_DHT_GPIO >= 0)
    s_dht_pin = (gpio_num_t)CONFIG_CUSTOM_SENSOR_DHT_GPIO;
#else
    s_dht_pin = (gpio_num_t)-1;
#endif
    if (GPIO_IS_VALID_GPIO(s_dht_pin)) {
        dht_init(s_dht_pin);
        ESP_LOGI(TAG, "DHT11/22 Temperature & Humidity Sensor configured on user GPIO %d", s_dht_pin);
    }
#endif
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_PIR)
#if defined(CONFIG_CUSTOM_SENSOR_PIR_GPIO) && (CONFIG_CUSTOM_SENSOR_PIR_GPIO >= 0)
    s_pir_pin = (gpio_num_t)CONFIG_CUSTOM_SENSOR_PIR_GPIO;
#else
    s_pir_pin = (gpio_num_t)-1;
#endif
    if (GPIO_IS_VALID_GPIO(s_pir_pin)) {
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
    s_vib_pin = (gpio_num_t)-1;
#endif
    if (GPIO_IS_VALID_GPIO(s_vib_pin)) {
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
    s_flame_pin = (gpio_num_t)-1;
#endif
    if (GPIO_IS_VALID_GPIO(s_flame_pin)) {
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
    s_chrg_pin = (gpio_num_t)-1;
#endif
    if (GPIO_IS_VALID_GPIO(s_chrg_pin)) {
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
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_HCSR04)
#if defined(CONFIG_CUSTOM_SENSOR_HCSR04_TRIG_GPIO) && (CONFIG_CUSTOM_SENSOR_HCSR04_TRIG_GPIO >= 0)
    s_hcsr04_trig = (gpio_num_t)CONFIG_CUSTOM_SENSOR_HCSR04_TRIG_GPIO;
#endif
#if defined(CONFIG_CUSTOM_SENSOR_HCSR04_ECHO_GPIO) && (CONFIG_CUSTOM_SENSOR_HCSR04_ECHO_GPIO >= 0)
    s_hcsr04_echo = (gpio_num_t)CONFIG_CUSTOM_SENSOR_HCSR04_ECHO_GPIO;
#endif
    if (GPIO_IS_VALID_GPIO(s_hcsr04_trig) && GPIO_IS_VALID_GPIO(s_hcsr04_echo)) {
        hcsr04_init(s_hcsr04_trig, s_hcsr04_echo, &s_hcsr04_dev);
    }
#endif
    i2c_master_bus_handle_t i2c_check = bus_manager_get_i2c_bus();
    if (i2c_check != NULL) {
        ESP_LOGI(TAG, "I2C Bus is available — I2C sensors will be probed on first read.");
    } else {
        ESP_LOGW(TAG, "I2C Bus not yet registered by board — I2C sensors will probe when bus is ready.");
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
        return NULL;
    }
    vl6180x_config_t cfg = {
        .i2c_bus = i2c_bus,
        .i2c_addr = 0x29,
        .xshut_pin = (gpio_num_t)-1,
        .gpio1_pin = (gpio_num_t)-1,
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
    if (GPIO_IS_VALID_GPIO(pin)) {
        dht_init(pin);
        ESP_LOGI(TAG, "DHT11/22 dynamically reconfigured to user GPIO %d", pin);
    } else {
        ESP_LOGI(TAG, "DHT11/22 Sensor disabled");
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
    out_env->valid = false;
    bool has_real_data = false;
    i2c_master_bus_handle_t i2c_bus = bus_manager_get_i2c_bus();
    if (i2c_bus != NULL) {
        if (s_aht20_dev == NULL) {
            aht20_init(i2c_bus, &s_aht20_dev);
        }
        if (s_aht20_dev != NULL) {
            float aht_t = 0.0f;
            float aht_h = 0.0f;
            if (aht20_read(s_aht20_dev, &aht_t, &aht_h) == ESP_OK) {
                out_env->temperature_c = aht_t;
                out_env->humidity_pct = aht_h;
                has_real_data = true;
                ESP_LOGD(TAG, "AHT20 read OK: Temp=%.1f C, Hum=%.1f%%", aht_t, aht_h);
            }
        }
    }
    if (out_env->temperature_c == 0.0f && GPIO_IS_VALID_GPIO(s_dht_pin)) {
        float dht_temp = 0.0f;
        float dht_hum = 0.0f;
        if (dht_read_data(s_dht_pin, &dht_temp, &dht_hum) == ESP_OK) {
            out_env->temperature_c = dht_temp;
            out_env->humidity_pct = dht_hum;
            has_real_data = true;
            ESP_LOGD(TAG, "DHT read OK on GPIO %d: Temp=%.1f C, Hum=%.1f%%", s_dht_pin, dht_temp, dht_hum);
        }
    }
    if (i2c_bus != NULL) {
        if (s_bmp280_dev == NULL) {
            bmp280_init(i2c_bus, &s_bmp280_dev);
        }
        if (s_bmp280_dev != NULL) {
            float bmp_press = 0.0f;
            float bmp_temp = 0.0f;
            if (bmp280_read(s_bmp280_dev, &bmp_press, &bmp_temp) == ESP_OK) {
                out_env->pressure_hpa = bmp_press;
                has_real_data = true;
                if (out_env->temperature_c == 0.0f && bmp_temp != 0.0f) {
                    out_env->temperature_c = bmp_temp;
                }
                ESP_LOGD(TAG, "BMP280 read OK: Press=%.2f hPa, Temp=%.1f C", bmp_press, bmp_temp);
            }
        }
    }
    bool lux_read_ok = false;
    if (i2c_bus != NULL) {
        if (s_bh1750_dev == NULL) {
            bh1750_init(i2c_bus, &s_bh1750_dev);
        }
        if (s_bh1750_dev != NULL) {
            float bh_lux = 0.0f;
            if (bh1750_read_lux(s_bh1750_dev, &bh_lux) == ESP_OK) {
                out_env->light_lux = bh_lux;
                lux_read_ok = true;
                has_real_data = true;
                ESP_LOGD(TAG, "BH1750 read OK: Lux=%.1f", bh_lux);
            }
        }
    }
    if (!lux_read_ok) {
        vl6180x_handle_t tof = get_vl6180x_dev();
        if (tof != NULL) {
            float tof_lux = 0.0f;
            if (vl6180x_read_ambient_lux(tof, &tof_lux) == ESP_OK) {
                out_env->light_lux = tof_lux;
                lux_read_ok = true;
                has_real_data = true;
            }
        }
    }
    if (!lux_read_ok && s_adc1_handle) {
        int raw_val = 0;
        if (adc_oneshot_read(s_adc1_handle, ADC_CHANNEL_1, &raw_val) == ESP_OK) {
            if (raw_val > 50 && raw_val < 4050) {
                out_env->light_lux = (float)raw_val * (1000.0f / 4095.0f);
                has_real_data = true;
            }
        }
    }
    if (i2c_bus != NULL) {
        if (s_scd4x_dev == NULL) {
            scd4x_init(i2c_bus, &s_scd4x_dev);
        }
        if (s_scd4x_dev != NULL) {
            float co2 = 0.0f;
            float scd_t = 0.0f;
            float scd_h = 0.0f;
            if (scd4x_read_measurement(s_scd4x_dev, &co2, &scd_t, &scd_h) == ESP_OK) {
                out_env->co2_ppm = co2;
                has_real_data = true;
                if (out_env->temperature_c == 0.0f && scd_t != 0.0f) {
                    out_env->temperature_c = scd_t;
                    out_env->humidity_pct = scd_h;
                }
                ESP_LOGD(TAG, "SCD40 read OK: CO2=%.0f PPM, Temp=%.1f C, Hum=%.1f%%", co2, scd_t, scd_h);
            }
        }
    }
    out_env->valid = has_real_data;
    if (!has_real_data) {
        ESP_LOGD(TAG, "No real environmental sensor data available.");
    }
    return ESP_OK;
}
esp_err_t sensor_read_distance(sensor_distance_t *out_dist)
{
    if (!out_dist) return ESP_ERR_INVALID_ARG;
    memset(out_dist, 0, sizeof(sensor_distance_t));
    out_dist->valid = false;
    vl6180x_handle_t tof = get_vl6180x_dev();
    if (tof != NULL) {
        float real_dist_mm = 0.0f;
        if (vl6180x_read_distance_mm(tof, &real_dist_mm) == ESP_OK) {
            out_dist->distance_laser_mm = real_dist_mm;
            out_dist->valid = true;
        } else {
            ESP_LOGD(TAG, "VL6180X distance read failed");
        }
    }
    if (s_hcsr04_dev != NULL) {
        float real_dist_cm = 0.0f;
        if (hcsr04_read_distance(s_hcsr04_dev, &real_dist_cm) == ESP_OK) {
            out_dist->distance_ultrasonic_cm = real_dist_cm;
            out_dist->valid = true;
            ESP_LOGD(TAG, "HC-SR04 read OK: Dist=%.1f cm", real_dist_cm);
        }
    }
    return ESP_OK;
}
esp_err_t sensor_read_security(sensor_security_t *out_sec)
{
    if (!out_sec) return ESP_ERR_INVALID_ARG;
    memset(out_sec, 0, sizeof(sensor_security_t));
    bool has_real_data = false;
    if (GPIO_IS_VALID_GPIO(s_pir_pin)) {
        out_sec->motion_detected = (gpio_get_level(s_pir_pin) == 1);
        has_real_data = true;
    }
    if (GPIO_IS_VALID_GPIO(s_vib_pin)) {
        out_sec->vibration_detected = (gpio_get_level(s_vib_pin) == 1);
        has_real_data = true;
    }
    if (GPIO_IS_VALID_GPIO(s_flame_pin)) {
        out_sec->flame_detected = (gpio_get_level(s_flame_pin) == 0); 
        has_real_data = true;
    }
    out_sec->gas_level_ppm = 0.0f;
    out_sec->gas_leak_alert = false;
    if (s_mq_dev != NULL) {
        float real_ppm = 0.0f;
        float real_mv = 0.0f;
        esp_err_t mq_ret = mq_sensor_read(s_mq_dev, &real_ppm, &real_mv);
        if (mq_ret == ESP_OK) {
            out_sec->gas_level_ppm = real_ppm;
            out_sec->gas_leak_alert = mq_sensor_is_leak_alert(s_mq_dev, real_ppm);
            has_real_data = true;
            ESP_LOGD(TAG, "MQ Gas real reading: %.1f PPM (%.1f mV), Alert=%d",
                     real_ppm, real_mv, out_sec->gas_leak_alert);
        } else {
            ESP_LOGD(TAG, "MQ Gas sensor not detected or disconnected: %s", esp_err_to_name(mq_ret));
        }
    }
    out_sec->valid = has_real_data;
    return ESP_OK;
}
esp_err_t sensor_read_power(sensor_power_t *out_pwr)
{
    if (!out_pwr) return ESP_ERR_INVALID_ARG;
    memset(out_pwr, 0, sizeof(sensor_power_t));
    out_pwr->valid = false;
    if (GPIO_IS_VALID_GPIO(s_chrg_pin)) {
        out_pwr->is_charging = (gpio_get_level(s_chrg_pin) == 0);
        out_pwr->valid = true;
    }
    i2c_master_bus_handle_t i2c_bus = bus_manager_get_i2c_bus();
    if (i2c_bus != NULL) {
        if (s_ina219_dev == NULL) {
            ina219_init(i2c_bus, &s_ina219_dev);
        }
        if (s_ina219_dev != NULL) {
            float v_bus = 0.0f;
            float cur_ma = 0.0f;
            float pwr_mw = 0.0f;
            if (ina219_read(s_ina219_dev, &v_bus, &cur_ma, &pwr_mw) == ESP_OK) {
                out_pwr->battery_voltage = v_bus;
                out_pwr->bus_current_ma = cur_ma;
                out_pwr->bus_power_mw = pwr_mw;
                out_pwr->valid = true;
                ESP_LOGD(TAG, "INA219 read OK: Voltage=%.2f V, Current=%.1f mA, Power=%.1f mW",
                         v_bus, cur_ma, pwr_mw);
            }
        }
    }
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
void sensor_set_mq_pin(gpio_num_t pin)
{
    if (s_mq_dev != NULL) {
        mq_sensor_deinit(s_mq_dev);
        s_mq_dev = NULL;
    }
    s_mq_pin = pin;
    if (GPIO_IS_VALID_GPIO(pin) && s_adc1_handle != NULL) {
        mq_sensor_config_t mq_cfg = {
            .pin = s_mq_pin,
            .type = MQ_TYPE_MQ2,
            .rl_kohm = 10.0f,
            .r0_kohm = 10.0f,
        };
        esp_err_t ret = mq_sensor_init(s_adc1_handle, &mq_cfg, &s_mq_dev);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "MQ Gas sensor dynamically configured to GPIO %d", (int)pin);
        } else {
            ESP_LOGW(TAG, "MQ Gas sensor dynamic init failed on GPIO %d: %s", (int)pin, esp_err_to_name(ret));
            s_mq_dev = NULL;
        }
    } else {
        ESP_LOGI(TAG, "MQ Gas sensor disabled");
    }
}
gpio_num_t sensor_get_mq_pin(void)
{
    return s_mq_pin;
}
void sensor_set_hcsr04_pins(gpio_num_t trig_pin, gpio_num_t echo_pin)
{
    if (s_hcsr04_dev != NULL) {
        hcsr04_deinit(s_hcsr04_dev);
        s_hcsr04_dev = NULL;
    }
    s_hcsr04_trig = trig_pin;
    s_hcsr04_echo = echo_pin;
    if (GPIO_IS_VALID_GPIO(trig_pin) && GPIO_IS_VALID_GPIO(echo_pin)) {
        hcsr04_init(trig_pin, echo_pin, &s_hcsr04_dev);
    }
}
