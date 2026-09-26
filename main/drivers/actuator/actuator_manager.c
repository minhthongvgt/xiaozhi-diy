/**
 * @file actuator_manager.c
 * @brief Actuators Subsystem Manager Implementation (ESP-IDF 6.1)
 */

#include "drivers/actuator/actuator_manager.h"
#include "pin_config.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "ActuatorManager"

#define SERVO_LEDC_MODE         LEDC_LOW_SPEED_MODE
#define SERVO_LEDC_TIMER        LEDC_TIMER_2
#define SERVO_LEDC_CHANNEL      LEDC_CHANNEL_2
#define SERVO_LEDC_RES          LEDC_TIMER_14_BIT
#define SERVO_LEDC_FREQ_HZ      50

#define DEFAULT_SERVO_MIN_US    500
#define DEFAULT_SERVO_MAX_US    2500

static gpio_num_t s_relay_pin = GPIO_NUM_NC;
static bool s_relay_state = false;
static gpio_num_t s_buzzer_pin = GPIO_NUM_NC;
static gpio_num_t s_haptic_pin = GPIO_NUM_NC;
static gpio_num_t s_servo_pin = GPIO_NUM_NC;
static uint8_t s_servo_angle = 90;
static uint16_t s_servo_min_us = DEFAULT_SERVO_MIN_US;
static uint16_t s_servo_max_us = DEFAULT_SERVO_MAX_US;
static bool s_servo_initialized = false;
static bool s_servo_attached = false;

static gpio_num_t s_motor_pwma = GPIO_NUM_NC;
static gpio_num_t s_motor_dira = GPIO_NUM_NC;
static gpio_num_t s_motor_pwmb = GPIO_NUM_NC;
static gpio_num_t s_motor_dirb = GPIO_NUM_NC;

static inline uint32_t servo_pulse_us_to_duty(uint32_t pulse_us)
{
    // 14-bit duty at 50Hz (20,000 us): duty = (pulse_us * 16384 + 10000) / 20000
    return (uint32_t)((pulse_us * 16384ULL + 10000) / 20000);
}

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

void actuator_set_servo_pulse_range(uint16_t min_us, uint16_t max_us)
{
    if (min_us < 200) min_us = 200;
    if (max_us > 3000) max_us = 3000;
    if (min_us >= max_us) return;
    s_servo_min_us = min_us;
    s_servo_max_us = max_us;
    ESP_LOGI(TAG, "Servo pulse range configured: [%u us, %u us]", min_us, max_us);
    if (s_servo_attached) {
        actuator_set_servo_angle(s_servo_angle);
    }
}

void actuator_set_servo_angle(uint8_t angle_deg)
{
    if (angle_deg > 180) angle_deg = 180;
    s_servo_angle = angle_deg;

    if (s_servo_initialized && s_servo_pin != GPIO_NUM_NC) {
        uint32_t pulse_us = s_servo_min_us + ((uint32_t)angle_deg * (s_servo_max_us - s_servo_min_us) + 90) / 180;
        uint32_t duty = servo_pulse_us_to_duty(pulse_us);
        esp_err_t err = ledc_set_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL, duty);
        if (err == ESP_OK) {
            err = ledc_update_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL);
        }
        if (err == ESP_OK) {
            s_servo_attached = true;
            ESP_LOGI(TAG, "Servo angle set to %d deg (pulse: %lu us, duty: %lu)",
                     angle_deg, (unsigned long)pulse_us, (unsigned long)duty);
        } else {
            ESP_LOGE(TAG, "Failed to update servo PWM: %s", esp_err_to_name(err));
        }
    } else {
        ESP_LOGD(TAG, "Virtual/Uninitialized Servo angle recorded: %d deg", angle_deg);
    }
}

uint8_t actuator_get_servo_angle(void)
{
    return s_servo_angle;
}

void actuator_servo_detach(void)
{
    if (s_servo_initialized && s_servo_pin != GPIO_NUM_NC) {
        esp_err_t err = ledc_set_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL, 0);
        if (err == ESP_OK) {
            ledc_update_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL);
        }
        s_servo_attached = false;
        ESP_LOGI(TAG, "Servo PWM detached (duty set to 0, holding torque released)");
    }
}

bool actuator_is_servo_attached(void)
{
    return s_servo_attached;
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
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure output '%s' on GPIO %d: %s", name, pin, esp_err_to_name(err));
        return;
    }
    gpio_set_level(pin, 0);
    ESP_LOGI(TAG, "Configured actuator output '%s' on GPIO %d", name, pin);
}

esp_err_t actuator_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing Actuators Subsystem (Relay, Servo, DC Motor, Buzzer)...");

#if defined(CONFIG_CUSTOM_PERIPH_RELAY_ENABLE)
#if defined(CONFIG_CUSTOM_PERIPH_RELAY_GPIO) && (CONFIG_CUSTOM_PERIPH_RELAY_GPIO >= 0)
    s_relay_pin = (gpio_num_t)CONFIG_CUSTOM_PERIPH_RELAY_GPIO;
#elif defined(CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO) && (CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO >= 0)
    s_relay_pin = (gpio_num_t)CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO;
#else
    s_relay_pin = GPIO_NUM_NC;
#endif
    if (s_relay_pin != GPIO_NUM_NC) {
        init_output_pin(s_relay_pin, "Relay 220V");
    }
#endif

#if defined(CONFIG_ENABLE_BUZZER)
#if defined(CONFIG_BUZZER_PIN) && (CONFIG_BUZZER_PIN >= 0)
    s_buzzer_pin = (gpio_num_t)CONFIG_BUZZER_PIN;
#else
    s_buzzer_pin = GPIO_NUM_NC;
#endif
    if (s_buzzer_pin != GPIO_NUM_NC) {
        init_output_pin(s_buzzer_pin, "Buzzer Alarm");
    }
#endif

#if defined(CONFIG_ENABLE_HAPTIC_MOTOR)
#if defined(CONFIG_HAPTIC_PIN) && (CONFIG_HAPTIC_PIN >= 0)
    s_haptic_pin = (gpio_num_t)CONFIG_HAPTIC_PIN;
#else
    s_haptic_pin = GPIO_NUM_NC;
#endif
    if (s_haptic_pin != GPIO_NUM_NC) {
        init_output_pin(s_haptic_pin, "Haptic Vibration Motor");
    }
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SERVO_DOG)
#if defined(CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO) && (CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO >= 0)
    s_servo_pin = (gpio_num_t)CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO;
#elif defined(PIN_SERVO_PWM) && (PIN_SERVO_PWM >= 0)
    s_servo_pin = PIN_SERVO_PWM;
#else
    s_servo_pin = GPIO_NUM_NC;
#endif
    if (s_servo_pin != GPIO_NUM_NC) {
        // Configure LEDC Timer for Servo PWM (50Hz) on LEDC_TIMER_2 (avoids conflict with Timer 0 display and Timer 1 gpio_led)
        ledc_timer_config_t ledc_timer = {
            .speed_mode       = SERVO_LEDC_MODE,
            .timer_num        = SERVO_LEDC_TIMER,
            .duty_resolution  = SERVO_LEDC_RES,
            .freq_hz          = SERVO_LEDC_FREQ_HZ,
            .clk_cfg          = LEDC_AUTO_CLK
        };
        esp_err_t err = ledc_timer_config(&ledc_timer);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to configure LEDC timer %d for servo: %s", SERVO_LEDC_TIMER, esp_err_to_name(err));
            s_servo_pin = GPIO_NUM_NC;
        } else {
            uint32_t pulse_us = s_servo_min_us + ((uint32_t)s_servo_angle * (s_servo_max_us - s_servo_min_us) + 90) / 180;
            uint32_t init_duty = servo_pulse_us_to_duty(pulse_us);

            ledc_channel_config_t ledc_channel = {
                .speed_mode     = SERVO_LEDC_MODE,
                .channel        = SERVO_LEDC_CHANNEL,
                .timer_sel      = SERVO_LEDC_TIMER,
                .intr_type      = LEDC_INTR_DISABLE,
                .gpio_num       = s_servo_pin,
                .duty           = init_duty,
                .hpoint         = 0
            };
            err = ledc_channel_config(&ledc_channel);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to configure LEDC channel %d for servo: %s", SERVO_LEDC_CHANNEL, esp_err_to_name(err));
                s_servo_pin = GPIO_NUM_NC;
            } else {
                s_servo_initialized = true;
                s_servo_attached = true;
                ESP_LOGI(TAG, "Servo PWM configured on GPIO %d (50Hz, LEDC Timer %d Ch %d, initial angle: %d deg, duty: %lu)",
                         s_servo_pin, SERVO_LEDC_TIMER, SERVO_LEDC_CHANNEL, s_servo_angle, (unsigned long)init_duty);
            }
        }
    }
#endif

#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE)
#if defined(CONFIG_CUSTOM_PERIPH_MOTOR_PWMA_PIN) && (CONFIG_CUSTOM_PERIPH_MOTOR_PWMA_PIN >= 0)
    s_motor_pwma = (gpio_num_t)CONFIG_CUSTOM_PERIPH_MOTOR_PWMA_PIN;
#else
    s_motor_pwma = GPIO_NUM_NC;
#endif
#if defined(CONFIG_CUSTOM_PERIPH_MOTOR_DIRA_PIN) && (CONFIG_CUSTOM_PERIPH_MOTOR_DIRA_PIN >= 0)
    s_motor_dira = (gpio_num_t)CONFIG_CUSTOM_PERIPH_MOTOR_DIRA_PIN;
#else
    s_motor_dira = GPIO_NUM_NC;
#endif
#if defined(CONFIG_CUSTOM_PERIPH_MOTOR_PWMB_PIN) && (CONFIG_CUSTOM_PERIPH_MOTOR_PWMB_PIN >= 0)
    s_motor_pwmb = (gpio_num_t)CONFIG_CUSTOM_PERIPH_MOTOR_PWMB_PIN;
#else
    s_motor_pwmb = GPIO_NUM_NC;
#endif
#if defined(CONFIG_CUSTOM_PERIPH_MOTOR_DIRB_PIN) && (CONFIG_CUSTOM_PERIPH_MOTOR_DIRB_PIN >= 0)
    s_motor_dirb = (gpio_num_t)CONFIG_CUSTOM_PERIPH_MOTOR_DIRB_PIN;
#else
    s_motor_dirb = GPIO_NUM_NC;
#endif

    if (s_motor_pwma != GPIO_NUM_NC) init_output_pin(s_motor_pwma, "Motor DC PWMA");
    if (s_motor_dira != GPIO_NUM_NC) init_output_pin(s_motor_dira, "Motor DC DIRA");
    if (s_motor_pwmb != GPIO_NUM_NC) init_output_pin(s_motor_pwmb, "Motor DC PWMB");
    if (s_motor_dirb != GPIO_NUM_NC) init_output_pin(s_motor_dirb, "Motor DC DIRB");
    ESP_LOGI(TAG, "DC Motor H-Bridge Driver configured (PWMA: %d, DIRA: %d, PWMB: %d, DIRB: %d)",
             s_motor_pwma, s_motor_dira, s_motor_pwmb, s_motor_dirb);
#endif

    ESP_LOGI(TAG, "Actuators Subsystem initialized successfully.");
    return ESP_OK;
}
