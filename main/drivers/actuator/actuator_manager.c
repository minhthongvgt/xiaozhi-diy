/**
 * @file actuator_manager.c
 * @brief Actuators Subsystem Manager Implementation (ESP-IDF 6.1)
 */

#include "actuator_manager.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "ActuatorManager"

static gpio_num_t s_relay_pin = GPIO_NUM_NC;
static bool s_relay_state = false;
static gpio_num_t s_buzzer_pin = GPIO_NUM_NC;
static gpio_num_t s_haptic_pin = GPIO_NUM_NC;
static gpio_num_t s_servo_pin = GPIO_NUM_NC;
static uint8_t s_servo_angle = 90;

static gpio_num_t s_motor_pwma = GPIO_NUM_NC;
static gpio_num_t s_motor_dira = GPIO_NUM_NC;
static gpio_num_t s_motor_pwmb = GPIO_NUM_NC;
static gpio_num_t s_motor_dirb = GPIO_NUM_NC;

void actuator_set_relay(bool state)
{
    s_relay_state = state;
    if (s_relay_pin != GPIO_NUM_NC) {
        gpio_set_level(s_relay_pin, state ? 1 : 0);
        ESP_LOGI(TAG, "Relay state changed to: %s", state ? "ON" : "OFF");
    }
}

bool actuator_get_relay(void)
{
    return s_relay_state;
}

void actuator_set_servo_angle(uint8_t angle_deg)
{
    if (angle_deg > 180) angle_deg = 180;
    s_servo_angle = angle_deg;

    if (s_servo_pin != GPIO_NUM_NC) {
        // Map 0..180 deg to 500us..2500us (410..2048 in 14-bit duty at 50Hz)
        uint32_t duty = 410 + (uint32_t)(((float)angle_deg / 180.0f) * (2048 - 410));
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
        ESP_LOGI(TAG, "Servo angle set to %d deg (duty: %lu)", angle_deg, (unsigned long)duty);
    } else {
        ESP_LOGD(TAG, "Virtual Servo angle recorded: %d deg", angle_deg);
    }
}

uint8_t actuator_get_servo_angle(void)
{
    return s_servo_angle;
}

void actuator_set_dc_motor(int motor_id, int speed_pct)
{
    if (speed_pct > 100) speed_pct = 100;
    if (speed_pct < -100) speed_pct = -100;

    ESP_LOGI(TAG, "Setting Motor %d to %d%% speed", motor_id, speed_pct);

    if (motor_id == 1 && s_motor_pwma != GPIO_NUM_NC && s_motor_dira != GPIO_NUM_NC) {
        gpio_set_level(s_motor_dira, (speed_pct >= 0) ? 1 : 0);
        gpio_set_level(s_motor_pwma, (speed_pct != 0) ? 1 : 0);
    } else if (motor_id == 2 && s_motor_pwmb != GPIO_NUM_NC && s_motor_dirb != GPIO_NUM_NC) {
        gpio_set_level(s_motor_dirb, (speed_pct >= 0) ? 1 : 0);
        gpio_set_level(s_motor_pwmb, (speed_pct != 0) ? 1 : 0);
    }
}

void actuator_beep(uint32_t freq_hz, uint32_t duration_ms)
{
    if (s_buzzer_pin != GPIO_NUM_NC) {
        gpio_set_level(s_buzzer_pin, 1);
        vTaskDelay(pdMS_TO_TICKS(duration_ms > 0 ? duration_ms : 100));
        gpio_set_level(s_buzzer_pin, 0);
    }
    ESP_LOGI(TAG, "Buzzer beep executed (%lu Hz, %lu ms)", (unsigned long)freq_hz, (unsigned long)duration_ms);
}

void actuator_vibrate(uint32_t duration_ms)
{
    if (s_haptic_pin != GPIO_NUM_NC) {
        gpio_set_level(s_haptic_pin, 1);
        vTaskDelay(pdMS_TO_TICKS(duration_ms > 0 ? duration_ms : 150));
        gpio_set_level(s_haptic_pin, 0);
    }
    ESP_LOGI(TAG, "Haptic vibration executed (%lu ms)", (unsigned long)duration_ms);
}

static void __attribute__((unused)) init_output_pin(gpio_num_t pin, const char *name)
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
    s_relay_pin = GPIO_NUM_13;
#if defined(CONFIG_CUSTOM_PERIPH_RELAY_GPIO)
    s_relay_pin = (gpio_num_t)CONFIG_CUSTOM_PERIPH_RELAY_GPIO;
#elif defined(CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO)
    s_relay_pin = (gpio_num_t)CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO;
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

#if defined(CONFIG_ENABLE_SERVO) || defined(CONFIG_CUSTOM_ENABLE_SERVO_DOG)
    s_servo_pin = GPIO_NUM_48;
#if defined(CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO)
    s_servo_pin = (gpio_num_t)CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO;
#endif
    // Configure LEDC Timer for Servo PWM (50Hz)
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_1,
        .duty_resolution  = LEDC_TIMER_14_BIT,
        .freq_hz          = 50,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_1,
        .timer_sel      = LEDC_TIMER_1,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = s_servo_pin,
        .duty           = 1229, // 90 degrees default
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
    ESP_LOGI(TAG, "Servo PWM configured on GPIO %d (50Hz, LEDC Timer1 Ch1)", s_servo_pin);
#endif

#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE) || defined(CONFIG_ENABLE_MOTOR_DRIVER)
    s_motor_pwma = GPIO_NUM_1;
    s_motor_dira = GPIO_NUM_2;
    s_motor_pwmb = GPIO_NUM_41;
    s_motor_dirb = GPIO_NUM_42;

    init_output_pin(s_motor_pwma, "Motor DC PWMA");
    init_output_pin(s_motor_dira, "Motor DC DIRA");
    init_output_pin(s_motor_pwmb, "Motor DC PWMB");
    init_output_pin(s_motor_dirb, "Motor DC DIRB");
    ESP_LOGI(TAG, "DC Motor H-Bridge Driver TB6612 configured (PWMA: 1, DIRA: 2, PWMB: 41, DIRB: 42)");
#endif

    ESP_LOGI(TAG, "Actuators Subsystem initialized successfully.");
    return ESP_OK;
}
