#include "drivers/sensor/sensor_manager.h"
#include "drivers/sensor/dht.h"
#include "drivers/sensor/mq_sensor.h"
#include "drivers/sensor/scd4x.h"
#include "drivers/sensor/bmp280.h"
#include "drivers/sensor/aht20.h"
#include "drivers/sensor/bh1750.h"
#include "drivers/sensor/hcsr04.h"
#include "drivers/sensor/ina219.h"
#include "drivers/sensor/vl53lx.h"
#include "drivers/sensor/apds9960.h"
#include "drivers/sensor/bq27220.h"
#include "boards/common/bus_manager.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <string.h>
#define TAG "SensorManager"
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_LDR) && defined(CONFIG_CUSTOM_SENSOR_LDR_ADC_CHANNEL) && (CONFIG_CUSTOM_SENSOR_LDR_ADC_CHANNEL >= 0)
#define LDR_ADC_CHANNEL ((adc_channel_t)CONFIG_CUSTOM_SENSOR_LDR_ADC_CHANNEL)
#endif
static adc_oneshot_unit_handle_t s_adc1_handle = NULL;
static mq_sensor_handle_t s_mq_dev = NULL;
static scd4x_handle_t __attribute__((unused)) s_scd4x_dev = NULL;
static bmp280_handle_t __attribute__((unused)) s_bmp280_dev = NULL;
static aht20_handle_t __attribute__((unused)) s_aht20_dev = NULL;
static bh1750_handle_t __attribute__((unused)) s_bh1750_dev = NULL;
static bool s_ldr_ready = false;
static hcsr04_handle_t s_hcsr04_dev = NULL;
static ina219_handle_t __attribute__((unused)) s_ina219_dev = NULL;
static vl53lx_handle_t __attribute__((unused)) s_vl53lx_dev = NULL;
static apds9960_handle_t __attribute__((unused)) s_apds9960_dev = NULL;
static bq27220_handle_t __attribute__((unused)) s_bq27220_dev = NULL;
static int64_t __attribute__((unused)) s_vl53lx_last_probe_us = 0;
static int64_t __attribute__((unused)) s_apds9960_last_probe_us = 0;
static int64_t __attribute__((unused)) s_bq27220_last_probe_us = 0;
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
#if defined(LDR_ADC_CHANNEL) || defined(CONFIG_CUSTOM_ENABLE_SENSOR_GAS_ANALOG_MQ)
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    esp_err_t ret = adc_oneshot_new_unit(&init_config1, &s_adc1_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "ADC1 Oneshot Unit init returned: %s - analog sensors will not be available",
                 esp_err_to_name(ret));
        s_adc1_handle = NULL;
    }
#if defined(LDR_ADC_CHANNEL)
    if (s_adc1_handle != NULL) {
        adc_oneshot_chan_cfg_t chan_cfg = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        esp_err_t ch_ret = adc_oneshot_config_channel(s_adc1_handle, LDR_ADC_CHANNEL, &chan_cfg);
        s_ldr_ready = (ch_ret == ESP_OK);
        if (!s_ldr_ready) {
            ESP_LOGW(TAG, "LDR ADC channel config failed: %s", esp_err_to_name(ch_ret));
        }
    }
#endif
#endif
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
#define SENSOR_REPROBE_INTERVAL_US (10LL * 1000 * 1000)
static bool __attribute__((unused)) probe_due(int64_t *last_probe_us)
{
    int64_t now = esp_timer_get_time();
    if (*last_probe_us != 0 && (now - *last_probe_us) < SENSOR_REPROBE_INTERVAL_US) {
        return false;
    }
    *last_probe_us = now;
    return true;
}
static vl53lx_handle_t get_vl53lx_dev(void)
{
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_VL53LX)
    if (s_vl53lx_dev != NULL) {
        return s_vl53lx_dev;
    }
    i2c_master_bus_handle_t i2c_bus = bus_manager_get_i2c_bus();
    if (i2c_bus == NULL || !probe_due(&s_vl53lx_last_probe_us)) {
        return NULL;
    }
    vl53lx_config_t cfg = {
        .i2c_bus = i2c_bus,
        .i2c_addr = VL53LX_DEFAULT_I2C_ADDR,
#if defined(CONFIG_CUSTOM_SENSOR_VL53LX_XSHUT_PIN) && (CONFIG_CUSTOM_SENSOR_VL53LX_XSHUT_PIN >= 0)
        .xshut_pin = (gpio_num_t)CONFIG_CUSTOM_SENSOR_VL53LX_XSHUT_PIN,
#else
        .xshut_pin = (gpio_num_t)-1,
#endif
    };
    esp_err_t ret = vl53lx_init(&cfg, &s_vl53lx_dev);
    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "VL53L0X/VL53L1X not detected: %s", esp_err_to_name(ret));
        s_vl53lx_dev = NULL;
    }
    return s_vl53lx_dev;
#else
    return NULL;
#endif
}
static apds9960_handle_t get_apds9960_dev(void)
{
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_APDS9960)
    if (s_apds9960_dev != NULL) {
        return s_apds9960_dev;
    }
    i2c_master_bus_handle_t i2c_bus = bus_manager_get_i2c_bus();
    if (i2c_bus == NULL || !probe_due(&s_apds9960_last_probe_us)) {
        return NULL;
    }
#if defined(CONFIG_CUSTOM_SENSOR_APDS9960_INT_PIN) && (CONFIG_CUSTOM_SENSOR_APDS9960_INT_PIN >= 0)
    gpio_num_t int_pin = (gpio_num_t)CONFIG_CUSTOM_SENSOR_APDS9960_INT_PIN;
#else
    gpio_num_t int_pin = (gpio_num_t)-1;
#endif
    esp_err_t ret = apds9960_init(i2c_bus, int_pin, &s_apds9960_dev);
    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "APDS-9960 not detected: %s", esp_err_to_name(ret));
        s_apds9960_dev = NULL;
    }
    return s_apds9960_dev;
#else
    return NULL;
#endif
}
static bq27220_handle_t get_bq27220_dev(void)
{
#if defined(CONFIG_CUSTOM_BATTERY_MONITOR_BQ27220)
    if (s_bq27220_dev != NULL) {
        return s_bq27220_dev;
    }
    i2c_master_bus_handle_t i2c_bus = bus_manager_get_i2c_bus();
    if (i2c_bus == NULL || !probe_due(&s_bq27220_last_probe_us)) {
        return NULL;
    }
    esp_err_t ret = bq27220_init(i2c_bus, &s_bq27220_dev);
    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "BQ27220 not detected: %s", esp_err_to_name(ret));
        s_bq27220_dev = NULL;
    }
    return s_bq27220_dev;
#else
    return NULL;
#endif
}
static aht20_handle_t get_aht20_dev(void)
{
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_AHT20)
    i2c_master_bus_handle_t i2c_bus = bus_manager_get_i2c_bus();
    if (s_aht20_dev == NULL && i2c_bus != NULL) {
        aht20_init(i2c_bus, &s_aht20_dev);
    }
    return s_aht20_dev;
#else
    return NULL;
#endif
}
static bmp280_handle_t get_bmp280_dev(void)
{
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_BMP280)
    i2c_master_bus_handle_t i2c_bus = bus_manager_get_i2c_bus();
    if (s_bmp280_dev == NULL && i2c_bus != NULL) {
        bmp280_init(i2c_bus, &s_bmp280_dev);
    }
    return s_bmp280_dev;
#else
    return NULL;
#endif
}
static bh1750_handle_t get_bh1750_dev(void)
{
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_BH1750)
    i2c_master_bus_handle_t i2c_bus = bus_manager_get_i2c_bus();
    if (s_bh1750_dev == NULL && i2c_bus != NULL) {
        bh1750_init(i2c_bus, &s_bh1750_dev);
    }
    return s_bh1750_dev;
#else
    return NULL;
#endif
}
static scd4x_handle_t get_scd4x_dev(void)
{
#if defined(CONFIG_CUSTOM_SENSOR_GAS_SCD4X)
    i2c_master_bus_handle_t i2c_bus = bus_manager_get_i2c_bus();
    if (s_scd4x_dev == NULL && i2c_bus != NULL) {
        scd4x_init(i2c_bus, &s_scd4x_dev);
    }
    return s_scd4x_dev;
#else
    return NULL;
#endif
}
static ina219_handle_t get_ina219_dev(void)
{
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_INA2XX)
    i2c_master_bus_handle_t i2c_bus = bus_manager_get_i2c_bus();
    if (s_ina219_dev == NULL && i2c_bus != NULL) {
        ina219_init(i2c_bus, &s_ina219_dev);
    }
    return s_ina219_dev;
#else
    return NULL;
#endif
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
static void set_temperature_humidity(sensor_environment_t *out_env, float temperature_c, float humidity_pct, bool has_humidity)
{
    if (!out_env->has_temperature) {
        out_env->temperature_c = temperature_c;
        out_env->has_temperature = true;
    }
    if (has_humidity && !out_env->has_humidity) {
        out_env->humidity_pct = humidity_pct;
        out_env->has_humidity = true;
    }
}
esp_err_t sensor_read_environment(sensor_environment_t *out_env)
{
    if (!out_env) return ESP_ERR_INVALID_ARG;
    memset(out_env, 0, sizeof(sensor_environment_t));
    aht20_handle_t aht = get_aht20_dev();
    if (aht != NULL) {
        float aht_t = 0.0f;
        float aht_h = 0.0f;
        if (aht20_read(aht, &aht_t, &aht_h) == ESP_OK) {
            set_temperature_humidity(out_env, aht_t, aht_h, true);
        }
    }
    if (!out_env->has_temperature && GPIO_IS_VALID_GPIO(s_dht_pin)) {
        float dht_temp = 0.0f;
        float dht_hum = 0.0f;
        if (dht_read_data(s_dht_pin, &dht_temp, &dht_hum) == ESP_OK) {
            set_temperature_humidity(out_env, dht_temp, dht_hum, true);
        }
    }
    bmp280_handle_t bmp = get_bmp280_dev();
    if (bmp != NULL) {
        float bmp_press = 0.0f;
        float bmp_temp = 0.0f;
        if (bmp280_read(bmp, &bmp_press, &bmp_temp) == ESP_OK) {
            out_env->pressure_hpa = bmp_press;
            out_env->has_pressure = true;
            set_temperature_humidity(out_env, bmp_temp, 0.0f, false);
        }
    }
    bh1750_handle_t bh = get_bh1750_dev();
    if (bh != NULL) {
        float bh_lux = 0.0f;
        if (bh1750_read_lux(bh, &bh_lux) == ESP_OK) {
            out_env->light_lux = bh_lux;
            out_env->has_light = true;
        }
    }
    if (!out_env->has_light) {
        apds9960_handle_t apds = get_apds9960_dev();
        apds9960_color_t color;
        if (apds != NULL && apds9960_read_color(apds, &color) == ESP_OK) {
            out_env->light_lux = apds9960_color_to_lux(&color);
            out_env->has_light = true;
        }
    }
#if defined(LDR_ADC_CHANNEL)
    if (!out_env->has_light && s_ldr_ready) {
        int raw_val = 0;
        if (adc_oneshot_read(s_adc1_handle, LDR_ADC_CHANNEL, &raw_val) == ESP_OK && raw_val > 50 && raw_val < 4050) {
            out_env->light_lux = (float)raw_val * (1000.0f / 4095.0f);
            out_env->has_light = true;
        }
    }
#endif
    scd4x_handle_t scd = get_scd4x_dev();
    if (scd != NULL) {
        float co2 = 0.0f;
        float scd_t = 0.0f;
        float scd_h = 0.0f;
        if (scd4x_read_measurement(scd, &co2, &scd_t, &scd_h) == ESP_OK) {
            out_env->co2_ppm = co2;
            out_env->has_co2 = true;
            set_temperature_humidity(out_env, scd_t, scd_h, true);
        }
    }
    out_env->valid = out_env->has_temperature || out_env->has_humidity || out_env->has_pressure ||
                     out_env->has_light || out_env->has_co2;
    return ESP_OK;
}
esp_err_t sensor_read_distance(sensor_distance_t *out_dist)
{
    if (!out_dist) return ESP_ERR_INVALID_ARG;
    memset(out_dist, 0, sizeof(sensor_distance_t));
    vl53lx_handle_t vl53 = get_vl53lx_dev();
    if (vl53 != NULL) {
        float mm = 0.0f;
        esp_err_t vl_ret = vl53lx_read_distance_mm(vl53, &mm);
        if (vl_ret == ESP_OK) {
            out_dist->distance_laser_mm = mm;
            out_dist->has_laser = true;
        } else {
            ESP_LOGD(TAG, "%s distance read failed: %s", vl53lx_type_name(vl53lx_get_type(vl53)),
                     esp_err_to_name(vl_ret));
        }
    }
    if (s_hcsr04_dev != NULL) {
        float real_dist_cm = 0.0f;
        if (hcsr04_read_distance(s_hcsr04_dev, &real_dist_cm) == ESP_OK) {
            out_dist->distance_ultrasonic_cm = real_dist_cm;
            out_dist->has_ultrasonic = true;
        }
    }
    out_dist->valid = out_dist->has_laser || out_dist->has_ultrasonic;
    return ESP_OK;
}
esp_err_t sensor_read_security(sensor_security_t *out_sec)
{
    if (!out_sec) return ESP_ERR_INVALID_ARG;
    memset(out_sec, 0, sizeof(sensor_security_t));
    if (GPIO_IS_VALID_GPIO(s_pir_pin)) {
        out_sec->motion_detected = (gpio_get_level(s_pir_pin) == 1);
        out_sec->has_motion = true;
    }
    if (GPIO_IS_VALID_GPIO(s_vib_pin)) {
        out_sec->vibration_detected = (gpio_get_level(s_vib_pin) == 1);
        out_sec->has_vibration = true;
    }
    if (GPIO_IS_VALID_GPIO(s_flame_pin)) {
        out_sec->flame_detected = (gpio_get_level(s_flame_pin) == 0);
        out_sec->has_flame = true;
    }
    if (s_mq_dev != NULL) {
        float real_ppm = 0.0f;
        float real_mv = 0.0f;
        esp_err_t mq_ret = mq_sensor_read(s_mq_dev, &real_ppm, &real_mv);
        if (mq_ret == ESP_OK) {
            out_sec->gas_level_ppm = real_ppm;
            out_sec->gas_leak_alert = mq_sensor_is_leak_alert(s_mq_dev, real_ppm);
            out_sec->has_gas = true;
        } else {
            ESP_LOGD(TAG, "MQ Gas sensor read failed: %s", esp_err_to_name(mq_ret));
        }
    }
    out_sec->valid = out_sec->has_motion || out_sec->has_vibration || out_sec->has_flame || out_sec->has_gas;
    return ESP_OK;
}
esp_err_t sensor_read_power(sensor_power_t *out_pwr)
{
    if (!out_pwr) return ESP_ERR_INVALID_ARG;
    memset(out_pwr, 0, sizeof(sensor_power_t));
    if (GPIO_IS_VALID_GPIO(s_chrg_pin)) {
        out_pwr->is_charging = (gpio_get_level(s_chrg_pin) == 0);
        out_pwr->has_charging = true;
    }
    bq27220_handle_t gauge = get_bq27220_dev();
    if (gauge != NULL) {
        bq27220_data_t gd;
        if (bq27220_read(gauge, &gd) == ESP_OK) {
            out_pwr->battery_voltage = gd.voltage_v;
            out_pwr->battery_percentage = gd.state_of_charge_pct;
            out_pwr->bus_current_ma = gd.current_ma;
            out_pwr->bus_power_mw = gd.voltage_v * gd.current_ma;
            out_pwr->is_charging = out_pwr->is_charging || gd.is_charging;
            out_pwr->has_voltage = true;
            out_pwr->has_percentage = true;
            out_pwr->has_current = true;
            out_pwr->has_charging = true;
        }
    }
    ina219_handle_t ina = get_ina219_dev();
    if (ina != NULL && !out_pwr->has_voltage) {
        float v_bus = 0.0f;
        float cur_ma = 0.0f;
        float pwr_mw = 0.0f;
        if (ina219_read(ina, &v_bus, &cur_ma, &pwr_mw) == ESP_OK) {
            out_pwr->battery_voltage = v_bus;
            out_pwr->bus_current_ma = cur_ma;
            out_pwr->bus_power_mw = pwr_mw;
            out_pwr->has_voltage = true;
            out_pwr->has_current = true;
        }
    }
    out_pwr->valid = out_pwr->has_voltage || out_pwr->has_percentage || out_pwr->has_charging || out_pwr->has_current;
    return ESP_OK;
}
bool sensor_environment_available(void)
{
    return GPIO_IS_VALID_GPIO(s_dht_pin) || s_ldr_ready || get_aht20_dev() != NULL || get_bmp280_dev() != NULL ||
           get_bh1750_dev() != NULL || get_scd4x_dev() != NULL || get_apds9960_dev() != NULL;
}
bool sensor_distance_available(void)
{
    return s_hcsr04_dev != NULL || get_vl53lx_dev() != NULL;
}
bool sensor_security_available(void)
{
    return GPIO_IS_VALID_GPIO(s_pir_pin) || GPIO_IS_VALID_GPIO(s_vib_pin) || GPIO_IS_VALID_GPIO(s_flame_pin) ||
           s_mq_dev != NULL;
}
bool sensor_power_available(void)
{
    return GPIO_IS_VALID_GPIO(s_chrg_pin) || get_bq27220_dev() != NULL || get_ina219_dev() != NULL;
}
bool sensor_color_available(void)
{
    return get_apds9960_dev() != NULL;
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
esp_err_t sensor_read_proximity_color(sensor_color_t *out_color)
{
    if (!out_color) return ESP_ERR_INVALID_ARG;
    memset(out_color, 0, sizeof(sensor_color_t));
    apds9960_handle_t apds = get_apds9960_dev();
    if (apds == NULL) return ESP_ERR_NOT_FOUND;
    uint8_t prox = 0;
    if (apds9960_read_proximity(apds, &prox) == ESP_OK) {
        out_color->proximity = prox;
        out_color->valid = true;
    }
    apds9960_color_t c;
    if (apds9960_read_color(apds, &c) == ESP_OK) {
        out_color->clear = c.clear;
        out_color->red = c.red;
        out_color->green = c.green;
        out_color->blue = c.blue;
        out_color->light_lux = apds9960_color_to_lux(&c);
        out_color->valid = true;
    }
    return out_color->valid ? ESP_OK : ESP_FAIL;
}
esp_err_t sensor_read_gesture(uint32_t timeout_ms, const char **out_gesture)
{
    if (!out_gesture) return ESP_ERR_INVALID_ARG;
    *out_gesture = "none";
    apds9960_handle_t apds = get_apds9960_dev();
    if (apds == NULL) return ESP_ERR_NOT_FOUND;
    apds9960_gesture_t gesture = APDS9960_GESTURE_NONE;
    esp_err_t ret = apds9960_read_gesture(apds, timeout_ms, &gesture);
    *out_gesture = apds9960_gesture_name(gesture);
    return ret;
}
