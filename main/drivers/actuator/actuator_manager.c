#include "drivers/actuator/actuator_manager.h"
#include "utils/config_utils.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <driver/rmt_tx.h>
#include <driver/rmt_encoder.h>
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
#define MOTOR_LEDC_MODE         LEDC_LOW_SPEED_MODE
#define MOTOR_LEDC_TIMER        LEDC_TIMER_3
#define MOTOR_LEDC_CHANNEL_A    LEDC_CHANNEL_4
#define MOTOR_LEDC_CHANNEL_B    LEDC_CHANNEL_5
#define MOTOR_LEDC_RES          LEDC_TIMER_10_BIT
#define MOTOR_LEDC_FREQ_HZ      20000
#define MOTOR_LEDC_DUTY_MAX     ((1U << 10) - 1)
static bool s_motor_pwm_ready = false;
static gpio_num_t s_relay_pin = (gpio_num_t)-1;
static bool s_relay_state = false;
static gpio_num_t s_buzzer_pin = (gpio_num_t)-1;
#define MAX_EXTRA_BUZZERS 4
static extra_gpio_config_t s_extra_buzzers[MAX_EXTRA_BUZZERS];
static int s_num_extra_buzzers = 0;
#define BUZZER_OUTPUT_COUNT (1 + MAX_EXTRA_BUZZERS)
#define BUZZER_DEFAULT_DURATION_MS 100
#if defined(CONFIG_BUZZER_ACTIVE_LOW)
#define BUZZER_ON_LEVEL 0
#else
#define BUZZER_ON_LEVEL 1
#endif
#if defined(CONFIG_BUZZER_TYPE_PASSIVE)
#define BUZZER_RMT_RESOLUTION_HZ 1000000
#define BUZZER_RMT_MEM_SYMBOLS 48
#define BUZZER_MIN_FREQ_HZ 20
#define BUZZER_MAX_FREQ_HZ 20000
typedef struct buzzer_tone_output_t {
    gpio_num_t pin;
    rmt_channel_handle_t channel;
    rmt_encoder_handle_t encoder;
} buzzer_tone_output_t;
static __attribute__((unused)) buzzer_tone_output_t s_buzzer_tones[BUZZER_OUTPUT_COUNT];
static __attribute__((unused)) int s_num_buzzer_tones = 0;
static __attribute__((unused)) rmt_symbol_word_t s_buzzer_tone_symbol;
#endif
static gpio_num_t s_haptic_pin = (gpio_num_t)-1;
static gpio_num_t s_servo_pin = (gpio_num_t)-1;
static uint8_t s_servo_angle = 90;
static uint16_t s_servo_min_us = DEFAULT_SERVO_MIN_US;
static uint16_t s_servo_max_us = DEFAULT_SERVO_MAX_US;
static bool s_servo_initialized = false;
static bool s_servo_attached = false;
static gpio_num_t s_motor_pwma = (gpio_num_t)-1;
static gpio_num_t s_motor_dira = (gpio_num_t)-1;
static gpio_num_t s_motor_pwmb = (gpio_num_t)-1;
static gpio_num_t s_motor_dirb = (gpio_num_t)-1;
static inline uint32_t servo_pulse_us_to_duty(uint32_t pulse_us)
{
    return (uint32_t)((pulse_us * 16384ULL + 10000) / 20000);
}
void actuator_set_relay(bool state)
{
    if (!GPIO_IS_VALID_GPIO(s_relay_pin)) {
        ESP_LOGW(TAG, "Relay hardware not configured/invalid pin, action ignored");
        return;
    }
    s_relay_state = state;
    gpio_set_level(s_relay_pin, state ? 1 : 0);
    ESP_LOGI(TAG, "Relay state changed to: %s", state ? "ON" : "OFF");
}
bool actuator_has_relay(void)
{
    return GPIO_IS_VALID_GPIO(s_relay_pin);
}
bool actuator_has_servo(void)
{
    return s_servo_initialized && GPIO_IS_VALID_GPIO(s_servo_pin);
}
bool actuator_has_motor(int motor_id)
{
    if (motor_id == 1) {
        return GPIO_IS_VALID_GPIO(s_motor_pwma) && GPIO_IS_VALID_GPIO(s_motor_dira);
    }
    if (motor_id == 2) {
        return GPIO_IS_VALID_GPIO(s_motor_pwmb) && GPIO_IS_VALID_GPIO(s_motor_dirb);
    }
    return false;
}
bool actuator_has_buzzer(void)
{
    if (GPIO_IS_VALID_GPIO(s_buzzer_pin)) {
        return true;
    }
    for (int i = 0; i < s_num_extra_buzzers; i++) {
        if (GPIO_IS_VALID_GPIO(s_extra_buzzers[i].pin1)) {
            return true;
        }
    }
    return false;
}
bool actuator_has_haptic(void)
{
    return GPIO_IS_VALID_GPIO(s_haptic_pin);
}
bool actuator_get_relay(void)
{
    if (!GPIO_IS_VALID_GPIO(s_relay_pin)) {
        return false;
    }
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
    if (s_servo_initialized && GPIO_IS_VALID_GPIO(s_servo_pin)) {
        s_servo_angle = angle_deg;
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
        ESP_LOGW(TAG, "Servo hardware uninitialized or invalid pin (%d), action ignored", (int)s_servo_pin);
    }
}
uint8_t actuator_get_servo_angle(void)
{
    if (!s_servo_initialized || !GPIO_IS_VALID_GPIO(s_servo_pin)) {
        return 0;
    }
    return s_servo_angle;
}
void actuator_servo_detach(void)
{
    if (s_servo_initialized && GPIO_IS_VALID_GPIO(s_servo_pin)) {
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
static void __attribute__((unused)) motor_pwm_init(void)
{
    if (!GPIO_IS_VALID_OUTPUT_GPIO(s_motor_pwma) && !GPIO_IS_VALID_OUTPUT_GPIO(s_motor_pwmb)) {
        return;
    }
    ledc_timer_config_t timer_cfg = {
        .speed_mode = MOTOR_LEDC_MODE,
        .duty_resolution = MOTOR_LEDC_RES,
        .timer_num = MOTOR_LEDC_TIMER,
        .freq_hz = MOTOR_LEDC_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    esp_err_t err = ledc_timer_config(&timer_cfg);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Motor PWM timer init failed (%s), falling back to on/off control", esp_err_to_name(err));
        return;
    }
    gpio_num_t pins[2] = {s_motor_pwma, s_motor_pwmb};
    ledc_channel_t channels[2] = {MOTOR_LEDC_CHANNEL_A, MOTOR_LEDC_CHANNEL_B};
    for (int i = 0; i < 2; i++) {
        if (!GPIO_IS_VALID_OUTPUT_GPIO(pins[i])) continue;
        ledc_channel_config_t ch_cfg = {
            .gpio_num = pins[i],
            .speed_mode = MOTOR_LEDC_MODE,
            .channel = channels[i],
            .timer_sel = MOTOR_LEDC_TIMER,
            .duty = 0,
            .hpoint = 0,
        };
        err = ledc_channel_config(&ch_cfg);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "Motor PWM channel %d init failed (%s)", (int)channels[i], esp_err_to_name(err));
            return;
        }
    }
    s_motor_pwm_ready = true;
    ESP_LOGI(TAG, "Motor PWM ready (%d Hz, LEDC timer %d)", MOTOR_LEDC_FREQ_HZ, (int)MOTOR_LEDC_TIMER);
}
static void motor_apply(gpio_num_t pwm_pin, gpio_num_t dir_pin, ledc_channel_t channel, int speed_pct)
{
    int magnitude = speed_pct < 0 ? -speed_pct : speed_pct;
    bool reverse = speed_pct < 0;
#if defined(CONFIG_CUSTOM_MOTOR_DRIVER_L9110S) || defined(CONFIG_CUSTOM_MOTOR_DRIVER_DRV8833)
    int duty_pct = (speed_pct == 0) ? 0 : (reverse ? 100 - magnitude : magnitude);
    gpio_set_level(dir_pin, (speed_pct != 0 && reverse) ? 1 : 0);
#else
    int duty_pct = magnitude;
    gpio_set_level(dir_pin, reverse ? 0 : 1);
#endif
    if (s_motor_pwm_ready) {
        uint32_t duty = (uint32_t)duty_pct * MOTOR_LEDC_DUTY_MAX / 100;
        ledc_set_duty(MOTOR_LEDC_MODE, channel, duty);
        ledc_update_duty(MOTOR_LEDC_MODE, channel);
    } else {
        gpio_set_level(pwm_pin, duty_pct >= 50 ? 1 : 0);
    }
}
void actuator_set_dc_motor(int motor_id, int speed_pct)
{
    if (speed_pct > 100) speed_pct = 100;
    if (speed_pct < -100) speed_pct = -100;
    ESP_LOGI(TAG, "Setting Motor %d to %d%% speed", motor_id, speed_pct);
    if (motor_id == 1 && GPIO_IS_VALID_GPIO(s_motor_pwma) && GPIO_IS_VALID_GPIO(s_motor_dira)) {
        motor_apply(s_motor_pwma, s_motor_dira, MOTOR_LEDC_CHANNEL_A, speed_pct);
    } else if (motor_id == 2 && GPIO_IS_VALID_GPIO(s_motor_pwmb) && GPIO_IS_VALID_GPIO(s_motor_dirb)) {
        motor_apply(s_motor_pwmb, s_motor_dirb, MOTOR_LEDC_CHANNEL_B, speed_pct);
    }
}
static int buzzer_collect_pins(gpio_num_t *pins)
{
    int count = 0;
    if (GPIO_IS_VALID_GPIO(s_buzzer_pin)) {
        pins[count++] = s_buzzer_pin;
    }
    for (int i = 0; i < s_num_extra_buzzers && count < BUZZER_OUTPUT_COUNT; i++) {
        if (GPIO_IS_VALID_GPIO(s_extra_buzzers[i].pin1)) {
            pins[count++] = s_extra_buzzers[i].pin1;
        }
    }
    return count;
}
static void buzzer_set_all_dc(const gpio_num_t *pins, int count, bool on)
{
    for (int i = 0; i < count; i++) {
        gpio_set_level(pins[i], on ? BUZZER_ON_LEVEL : !BUZZER_ON_LEVEL);
    }
}
#if defined(CONFIG_BUZZER_TYPE_PASSIVE)
static __attribute__((unused)) const buzzer_tone_output_t *buzzer_find_tone(gpio_num_t pin)
{
    for (int i = 0; i < s_num_buzzer_tones; i++) {
        if (s_buzzer_tones[i].pin == pin) {
            return &s_buzzer_tones[i];
        }
    }
    return NULL;
}
static __attribute__((unused)) void buzzer_play_tone(const gpio_num_t *pins, int count, uint32_t freq_hz, uint32_t duration_ms)
{
    if (freq_hz < BUZZER_MIN_FREQ_HZ) freq_hz = BUZZER_MIN_FREQ_HZ;
    if (freq_hz > BUZZER_MAX_FREQ_HZ) freq_hz = BUZZER_MAX_FREQ_HZ;
    uint32_t half_period_ticks = BUZZER_RMT_RESOLUTION_HZ / (2 * freq_hz);
    s_buzzer_tone_symbol.level0 = 1;
    s_buzzer_tone_symbol.duration0 = half_period_ticks;
    s_buzzer_tone_symbol.level1 = 0;
    s_buzzer_tone_symbol.duration1 = half_period_ticks;
    uint32_t cycles = (uint32_t)(((uint64_t)freq_hz * duration_ms) / 1000);
    rmt_transmit_config_t tx_config = {
        .loop_count = cycles > 1 ? (int)cycles : 1,
        .flags.eot_level = 0,
    };
    gpio_num_t dc_pins[BUZZER_OUTPUT_COUNT];
    int dc_count = 0;
    for (int i = 0; i < count; i++) {
        const buzzer_tone_output_t *tone = buzzer_find_tone(pins[i]);
        esp_err_t err = ESP_FAIL;
        if (tone != NULL) {
            err = rmt_transmit(tone->channel, tone->encoder, &s_buzzer_tone_symbol, sizeof(s_buzzer_tone_symbol), &tx_config);
        }
        if (err != ESP_OK) {
            dc_pins[dc_count++] = pins[i];
        }
    }
    buzzer_set_all_dc(dc_pins, dc_count, true);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    buzzer_set_all_dc(dc_pins, dc_count, false);
    for (int i = 0; i < s_num_buzzer_tones; i++) {
        esp_err_t err = rmt_tx_wait_all_done(s_buzzer_tones[i].channel, 200);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "Buzzer tone on GPIO %d did not finish: %s", s_buzzer_tones[i].pin, esp_err_to_name(err));
        }
    }
}
#endif
void actuator_beep(uint32_t freq_hz, uint32_t duration_ms)
{
    gpio_num_t pins[BUZZER_OUTPUT_COUNT];
    int count = buzzer_collect_pins(pins);
    if (count == 0) {
        ESP_LOGW(TAG, "Buzzer hardware not configured/invalid pin, action ignored");
        return;
    }
    if (duration_ms == 0) {
        duration_ms = BUZZER_DEFAULT_DURATION_MS;
    }
#if defined(CONFIG_BUZZER_TYPE_PASSIVE)
    buzzer_play_tone(pins, count, freq_hz, duration_ms);
    ESP_LOGI(TAG, "Passive buzzer tone executed (%lu Hz, %lu ms)", (unsigned long)freq_hz, (unsigned long)duration_ms);
#else
    buzzer_set_all_dc(pins, count, true);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    buzzer_set_all_dc(pins, count, false);
    ESP_LOGI(TAG, "Active buzzer beep executed (%lu ms, requested %lu Hz ignored)", (unsigned long)duration_ms, (unsigned long)freq_hz);
#endif
}
void actuator_vibrate(uint32_t duration_ms)
{
    if (GPIO_IS_VALID_GPIO(s_haptic_pin)) {
        gpio_set_level(s_haptic_pin, 1);
        vTaskDelay(pdMS_TO_TICKS(duration_ms > 0 ? duration_ms : 150));
        gpio_set_level(s_haptic_pin, 0);
        ESP_LOGI(TAG, "Haptic vibration executed (%lu ms)", (unsigned long)duration_ms);
    } else {
        ESP_LOGW(TAG, "Haptic hardware not configured/invalid pin (%d), action ignored", (int)s_haptic_pin);
    }
}
static void __attribute__((unused)) init_output_pin_level(gpio_num_t pin, const char *name, int idle_level)
{
    if (!GPIO_IS_VALID_GPIO(pin)) return;
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
    gpio_set_level(pin, idle_level);
    ESP_LOGI(TAG, "Configured actuator output '%s' on GPIO %d", name, pin);
}
static void __attribute__((unused)) init_output_pin(gpio_num_t pin, const char *name)
{
    init_output_pin_level(pin, name, 0);
}
#if defined(CONFIG_BUZZER_TYPE_PASSIVE)
static __attribute__((unused)) void buzzer_init_tone_output(gpio_num_t pin)
{
    if (s_num_buzzer_tones >= BUZZER_OUTPUT_COUNT) {
        return;
    }
    rmt_tx_channel_config_t channel_config = {
        .gpio_num = pin,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = BUZZER_RMT_RESOLUTION_HZ,
        .mem_block_symbols = BUZZER_RMT_MEM_SYMBOLS,
        .trans_queue_depth = 1,
        .flags.invert_out = (BUZZER_ON_LEVEL == 0),
    };
    rmt_channel_handle_t channel = NULL;
    esp_err_t err = rmt_new_tx_channel(&channel_config, &channel);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "No RMT channel for passive buzzer on GPIO %d (%s), it will only click", pin, esp_err_to_name(err));
        return;
    }
    rmt_copy_encoder_config_t encoder_config = {};
    rmt_encoder_handle_t encoder = NULL;
    err = rmt_new_copy_encoder(&encoder_config, &encoder);
    if (err == ESP_OK) {
        err = rmt_enable(channel);
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start passive buzzer on GPIO %d: %s", pin, esp_err_to_name(err));
        if (encoder != NULL) {
            rmt_del_encoder(encoder);
        }
        rmt_del_channel(channel);
        return;
    }
    s_buzzer_tones[s_num_buzzer_tones].pin = pin;
    s_buzzer_tones[s_num_buzzer_tones].channel = channel;
    s_buzzer_tones[s_num_buzzer_tones].encoder = encoder;
    s_num_buzzer_tones++;
    ESP_LOGI(TAG, "Passive buzzer on GPIO %d driven by RMT square wave", pin);
}
#endif
esp_err_t actuator_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing Actuators Subsystem (Relay, Servo, DC Motor, Buzzer)...");
#if defined(CONFIG_CUSTOM_PERIPH_RELAY_ENABLE)
#if defined(CONFIG_CUSTOM_PERIPH_RELAY_GPIO)
    s_relay_pin = (gpio_num_t)CONFIG_CUSTOM_PERIPH_RELAY_GPIO;
#else
    s_relay_pin = (gpio_num_t)-1;
#endif
    if (GPIO_IS_VALID_GPIO(s_relay_pin)) {
        init_output_pin(s_relay_pin, "Relay 220V");
    }
#endif
#if defined(CONFIG_ENABLE_BUZZER)
#if defined(CONFIG_BUZZER_PIN) && (CONFIG_BUZZER_PIN >= 0)
    s_buzzer_pin = (gpio_num_t)CONFIG_BUZZER_PIN;
#else
    s_buzzer_pin = (gpio_num_t)-1;
#endif
#if defined(CONFIG_BUZZER_EXTRA_PINS)
    s_num_extra_buzzers = parse_extra_gpios(CONFIG_BUZZER_EXTRA_PINS, s_extra_buzzers, MAX_EXTRA_BUZZERS);
#endif
    {
        gpio_num_t buzzer_pins[BUZZER_OUTPUT_COUNT];
        int buzzer_count = buzzer_collect_pins(buzzer_pins);
        for (int i = 0; i < buzzer_count; i++) {
            init_output_pin_level(buzzer_pins[i], "Buzzer", !BUZZER_ON_LEVEL);
#if defined(CONFIG_BUZZER_TYPE_PASSIVE)
            buzzer_init_tone_output(buzzer_pins[i]);
#endif
        }
    }
#endif
#if defined(CONFIG_ENABLE_HAPTIC_MOTOR)
#if defined(CONFIG_HAPTIC_PIN) && (CONFIG_HAPTIC_PIN >= 0)
    s_haptic_pin = (gpio_num_t)CONFIG_HAPTIC_PIN;
#else
    s_haptic_pin = (gpio_num_t)-1;
#endif
    if (GPIO_IS_VALID_GPIO(s_haptic_pin)) {
        init_output_pin(s_haptic_pin, "Haptic Vibration Motor");
    }
#endif
#if defined(CONFIG_CUSTOM_ENABLE_SERVO_DOG)
#if defined(CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO)
    s_servo_pin = (gpio_num_t)CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO;
#else
    s_servo_pin = (gpio_num_t)-1;
#endif
    if (GPIO_IS_VALID_GPIO(s_servo_pin)) {
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
            s_servo_pin = (gpio_num_t)-1;
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
                s_servo_pin = (gpio_num_t)-1;
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
    s_motor_pwma = (gpio_num_t)-1;
#endif
#if defined(CONFIG_CUSTOM_PERIPH_MOTOR_DIRA_PIN) && (CONFIG_CUSTOM_PERIPH_MOTOR_DIRA_PIN >= 0)
    s_motor_dira = (gpio_num_t)CONFIG_CUSTOM_PERIPH_MOTOR_DIRA_PIN;
#else
    s_motor_dira = (gpio_num_t)-1;
#endif
#if defined(CONFIG_CUSTOM_PERIPH_MOTOR_PWMB_PIN) && (CONFIG_CUSTOM_PERIPH_MOTOR_PWMB_PIN >= 0)
    s_motor_pwmb = (gpio_num_t)CONFIG_CUSTOM_PERIPH_MOTOR_PWMB_PIN;
#else
    s_motor_pwmb = (gpio_num_t)-1;
#endif
#if defined(CONFIG_CUSTOM_PERIPH_MOTOR_DIRB_PIN) && (CONFIG_CUSTOM_PERIPH_MOTOR_DIRB_PIN >= 0)
    s_motor_dirb = (gpio_num_t)CONFIG_CUSTOM_PERIPH_MOTOR_DIRB_PIN;
#else
    s_motor_dirb = (gpio_num_t)-1;
#endif
    if (GPIO_IS_VALID_GPIO(s_motor_pwma)) init_output_pin(s_motor_pwma, "Motor DC PWMA");
    if (GPIO_IS_VALID_GPIO(s_motor_dira)) init_output_pin(s_motor_dira, "Motor DC DIRA");
    if (GPIO_IS_VALID_GPIO(s_motor_pwmb)) init_output_pin(s_motor_pwmb, "Motor DC PWMB");
    if (GPIO_IS_VALID_GPIO(s_motor_dirb)) init_output_pin(s_motor_dirb, "Motor DC DIRB");
    motor_pwm_init();
    ESP_LOGI(TAG, "DC Motor H-Bridge Driver configured (PWMA: %d, DIRA: %d, PWMB: %d, DIRB: %d)",
             s_motor_pwma, s_motor_dira, s_motor_pwmb, s_motor_dirb);
#endif
    ESP_LOGI(TAG, "Actuators Subsystem initialized successfully.");
    return ESP_OK;
}
