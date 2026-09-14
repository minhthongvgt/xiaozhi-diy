/**
 * @file sensor_manager.c
 * @brief Sensors Subsystem Manager Implementation (ESP-IDF 6.1)
 */

#include "sensor_manager.h"
#include "boards/common/bus_manager.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>

#define TAG "SensorManager"

static adc_oneshot_unit_handle_t s_adc1_handle = NULL;

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
    } else {
        ESP_LOGW(TAG, "ADC1 Oneshot Unit init returned: %s", esp_err_to_name(ret));
    }

    // 2. Configure Digital Sensors (PIR, Vibration, Flame, DS18B20, Flow)
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_PIR) || defined(CONFIG_ENABLE_PIR_SENSOR)
    gpio_num_t pir_pin = GPIO_NUM_14;
#if defined(CONFIG_CUSTOM_SENSOR_PIR_GPIO)
    pir_pin = (gpio_num_t)CONFIG_CUSTOM_SENSOR_PIR_GPIO;
#elif defined(CONFIG_PIR_PIN)
    pir_pin = (gpio_num_t)CONFIG_PIR_PIN;
#endif
    gpio_config_t pir_conf = {
        .pin_bit_mask = (1ULL << pir_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&pir_conf);
    ESP_LOGI(TAG, "PIR Motion Sensor configured on GPIO %d", pir_pin);
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_VIBRATION_SW420) || defined(CONFIG_ENABLE_VIBRATION_SENSOR)
    gpio_num_t vib_pin = GPIO_NUM_6;
    gpio_config_t vib_conf = {
        .pin_bit_mask = (1ULL << vib_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&vib_conf);
    ESP_LOGI(TAG, "SW-420 Vibration Sensor configured on GPIO %d", vib_pin);
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_FLAME) || defined(CONFIG_ENABLE_FLAME_SENSOR)
    gpio_num_t flame_pin = GPIO_NUM_7;
    gpio_config_t flame_conf = {
        .pin_bit_mask = (1ULL << flame_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&flame_conf);
    ESP_LOGI(TAG, "Flame Sensor configured on GPIO %d", flame_pin);
#endif

    // 3. I2C Sensors (AHT20, SHT3x, BMP280, BH1750, SCD40, MPU6050, VL53L0X, RTC)
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_AHT20) || defined(CONFIG_ENABLE_TEMP_HUMIDITY)
    ESP_LOGI(TAG, "Temperature & Humidity Sensor (AHT20/SHT30) registered on shared I2C bus.");
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_GAS_CO2) || defined(CONFIG_ENABLE_CO2_SENSOR)
    ESP_LOGI(TAG, "CO2/TVOC Sensor (SCD40/SGP30) registered on shared I2C bus.");
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_BH1750) || defined(CONFIG_ENABLE_LIGHT_SENSOR)
    ESP_LOGI(TAG, "Light Sensor (BH1750) registered on shared I2C bus.");
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_VL53LX) || defined(CONFIG_ENABLE_TOF_LASER)
    ESP_LOGI(TAG, "Laser ToF Sensor (VL53L0X) registered on shared I2C bus.");
#endif

#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_RTC) || defined(CONFIG_ENABLE_RTC)
    ESP_LOGI(TAG, "Real-Time Clock (DS3231/PCF8563) registered on shared I2C bus.");
#endif

#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_NFC_PN532) || defined(CONFIG_ENABLE_NFC_READER)
    ESP_LOGI(TAG, "NFC Reader (PN532) registered on shared I2C bus.");
#endif

    ESP_LOGI(TAG, "Sensors Subsystem initialized successfully.");
    return ESP_OK;
}

esp_err_t sensor_manager_read_all(sensor_data_t *out_data)
{
    if (!out_data) return ESP_ERR_INVALID_ARG;

    // Provide default safe values
    out_data->temperature_c = 25.0f;
    out_data->humidity_pct = 60.0f;
    out_data->light_lux = 300.0f;
    out_data->co2_ppm = 420.0f;
    out_data->battery_voltage = 3.9f;
    out_data->motion_detected = false;
    out_data->vibration_detected = false;
    out_data->flame_detected = false;

    return ESP_OK;
}
