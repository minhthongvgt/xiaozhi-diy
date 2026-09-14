/**
 * @file actuator_manager.c
 * @brief Actuators Subsystem Manager Implementation (ESP-IDF 6.1)
 */

#include "actuator_manager.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "ActuatorManager"

static gpio_num_t s_relay_pin = GPIO_NUM_NC;
static gpio_num_t s_buzzer_pin = GPIO_NUM_NC;
static gpio_num_t s_haptic_pin = GPIO_NUM_NC;

void actuator_set_relay(bool state)
{
    if (s_relay_pin != GPIO_NUM_NC) {
        gpio_set_level(s_relay_pin, state ? 1 : 0);
        ESP_LOGI(TAG, "Relay state changed to: %s", state ? "ON" : "OFF");
    }
}

void actuator_set_servo_angle(uint8_t angle_deg)
{
    ESP_LOGD(TAG, "Setting Servo angle to %d deg", angle_deg);
}

void actuator_set_dc_motor(int motor_id, int speed_pct)
{
    ESP_LOGD(TAG, "Setting Motor %d to %d%% speed", motor_id, speed_pct);
}

void actuator_beep(uint32_t freq_hz, uint32_t duration_ms)
{
    if (s_buzzer_pin != GPIO_NUM_NC) {
        gpio_set_level(s_buzzer_pin, 1);
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        gpio_set_level(s_buzzer_pin, 0);
    }
}

void actuator_vibrate(uint32_t duration_ms)
{
    if (s_haptic_pin != GPIO_NUM_NC) {
        gpio_set_level(s_haptic_pin, 1);
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        gpio_set_level(s_haptic_pin, 0);
    }
}

static void init_output_pin(gpio_num_t pin, const char *name)
{
    if (pin < 0) return;
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret == ESP_OK) {
        gpio_set_level(pin, 0);
        ESP_LOGI(TAG, "Configured actuator output '%s' on GPIO %d", name, pin);
    } else {
        ESP_LOGE(TAG, "Failed to configure output '%s' on GPIO %d: %s", name, pin, esp_err_to_name(ret));
    }
}

esp_err_t actuator_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing Actuators Subsystem (Relay, Servo, DC Motor, Buzzer)...");

#if defined(CONFIG_CUSTOM_PERIPH_RELAY_ENABLE) || defined(CONFIG_ENABLE_RELAY)
    s_relay_pin = GPIO_NUM_45;
#if defined(CONFIG_CUSTOM_PERIPH_RELAY_GPIO)
    s_relay_pin = (gpio_num_t)CONFIG_CUSTOM_PERIPH_RELAY_GPIO;
#endif
    init_output_pin(s_relay_pin, "Relay 220V");
#endif

#if defined(CONFIG_ENABLE_BUZZER)
    s_buzzer_pin = GPIO_NUM_41;
#if defined(CONFIG_BUZZER_PIN)
    s_buzzer_pin = (gpio_num_t)CONFIG_BUZZER_PIN;
#endif
    init_output_pin(s_buzzer_pin, "Buzzer Alarm");
#endif

#if defined(CONFIG_ENABLE_HAPTIC_MOTOR)
    s_haptic_pin = GPIO_NUM_42;
#if defined(CONFIG_HAPTIC_PIN)
    s_haptic_pin = (gpio_num_t)CONFIG_HAPTIC_PIN;
#endif
    init_output_pin(s_haptic_pin, "Haptic Vibration Motor");
#endif

#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE) || defined(CONFIG_ENABLE_MOTOR_DRIVER)
    init_output_pin(GPIO_NUM_1, "Motor DC PWMA");
    init_output_pin(GPIO_NUM_2, "Motor DC DIRA");
    init_output_pin(GPIO_NUM_41, "Motor DC PWMB");
    init_output_pin(GPIO_NUM_42, "Motor DC DIRB");
    ESP_LOGI(TAG, "DC Motor H-Bridge Driver TB6612 configured (PWMA: 1, DIRA: 2, PWMB: 41, DIRB: 42)");
#endif

#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_PCA9685) || defined(CONFIG_ENABLE_PWM_EXPANDER)
    ESP_LOGI(TAG, "PCA9685 16-channel PWM controller mapped to shared I2C bus.");
#endif

    ESP_LOGI(TAG, "Actuators Subsystem initialized successfully.");
    return ESP_OK;
}
