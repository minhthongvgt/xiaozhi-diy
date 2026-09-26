#include "gpio_led.h"
#include "application.h"
#include "device_state.h"
#include <esp_log.h>

#define TAG "GpioLed"

#define DEFAULT_BRIGHTNESS 128
#define HIGH_BRIGHTNESS 255
#define LOW_BRIGHTNESS 25

#define IDLE_BRIGHTNESS 12
#define SPEAKING_BRIGHTNESS 192
#define UPGRADING_BRIGHTNESS 64
#define ACTIVATING_BRIGHTNESS 90

#define BLINK_INFINITE -1

// GPIO_LED
#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH0_CHANNEL    LEDC_CHANNEL_0

#define LEDC_DUTY              (8191)
#define LEDC_FADE_TIME         (1000)
// GPIO_LED

GpioLed::GpioLed(gpio_num_t gpio)
        : GpioLed(gpio, 0, LEDC_LS_TIMER, LEDC_LS_CH0_CHANNEL) {
}

GpioLed::GpioLed(gpio_num_t gpio, int output_invert)
        : GpioLed(gpio, output_invert, LEDC_LS_TIMER, LEDC_LS_CH0_CHANNEL) {
}

GpioLed::GpioLed(gpio_num_t gpio, int output_invert, ledc_timer_t timer_num, ledc_channel_t channel) {
    if (gpio == GPIO_NUM_NC) {
        ESP_LOGW(TAG, "GpioLed initialized with GPIO_NUM_NC, LED will not function");
        return;
    }

    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {};
    ledc_timer.duty_resolution = LEDC_TIMER_13_BIT;  // resolution of PWM duty
    ledc_timer.freq_hz = 4000;                      // frequency of PWM signal
    ledc_timer.speed_mode = LEDC_LS_MODE;           // timer mode
    ledc_timer.timer_num = timer_num;               // timer index
    ledc_timer.clk_cfg = LEDC_AUTO_CLK;              // Auto select the source clock

    esp_err_t err = ledc_timer_config(&ledc_timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_timer_config failed: %s", esp_err_to_name(err));
        return;
    }

    ledc_channel_.channel    = channel;
    ledc_channel_.duty       = 0;
    ledc_channel_.gpio_num   = gpio;
    ledc_channel_.speed_mode = LEDC_LS_MODE;
    ledc_channel_.hpoint     = 0;
    ledc_channel_.timer_sel  = timer_num;
    ledc_channel_.flags.output_invert = output_invert & 0x01;

    // Set LED Controller with previously prepared configuration
    err = ledc_channel_config(&ledc_channel_);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_channel_config failed: %s", esp_err_to_name(err));
        return;
    }

    // Initialize fade service safely (ignore if already installed by another module)
    err = ledc_fade_func_install(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "ledc_fade_func_install warning: %s", esp_err_to_name(err));
    }

    // When the callback registered by ledc_cb_register is called, run led->OnFadeEnd()
    ledc_cbs_t ledc_callbacks = {
        .fade_cb = FadeCallback
    };
    err = ledc_cb_register(ledc_channel_.speed_mode, ledc_channel_.channel, &ledc_callbacks, this);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "ledc_cb_register failed: %s", esp_err_to_name(err));
    }

    esp_timer_create_args_t blink_timer_args = {
        .callback = [](void *arg) {
            auto led = static_cast<GpioLed*>(arg);
            led->OnBlinkTimer();
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "Blink Timer",
        .skip_unhandled_events = false,
    };
    err = esp_timer_create(&blink_timer_args, &blink_timer_);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_timer_create blink_timer failed: %s", esp_err_to_name(err));
        blink_timer_ = nullptr;
    }

    BaseType_t task_ret = xTaskCreatePinnedToCore(EventTask, "LedEvent", 4096, this, 
            tskIDLE_PRIORITY + 2, &event_task_handle_, 1);
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LedEvent task");
        event_task_handle_ = nullptr;
    }

    SetBrightness(DEFAULT_BRIGHTNESS);
    ledc_initialized_ = true;
}

GpioLed::~GpioLed() {
    if (blink_timer_ != nullptr) {
        esp_timer_stop(blink_timer_);
        esp_timer_delete(blink_timer_);
        blink_timer_ = nullptr;
    }
    if (ledc_initialized_) {
        ledc_fade_stop(ledc_channel_.speed_mode, ledc_channel_.channel);
    }
    if (event_task_handle_ != nullptr) {
        TaskHandle_t task = event_task_handle_;
        event_task_handle_ = nullptr;
        vTaskDelete(task);
    }
}

void GpioLed::SetBrightness(uint8_t brightness) {
    brightness_ = brightness;
    duty_ = static_cast<uint32_t>(brightness_) * LEDC_DUTY / 255;
    if (ledc_initialized_ && is_on_) {
        std::lock_guard<std::mutex> lock(mutex_);
        ledc_set_duty(ledc_channel_.speed_mode, ledc_channel_.channel, duty_);
        ledc_update_duty(ledc_channel_.speed_mode, ledc_channel_.channel);
    }
}

void GpioLed::TurnOn() {
    if (!ledc_initialized_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (blink_timer_ != nullptr) {
        esp_timer_stop(blink_timer_);
    }
    ledc_fade_stop(ledc_channel_.speed_mode, ledc_channel_.channel);
    is_on_ = true;
    ledc_set_duty(ledc_channel_.speed_mode, ledc_channel_.channel, duty_);
    ledc_update_duty(ledc_channel_.speed_mode, ledc_channel_.channel);
}

void GpioLed::TurnOff() {
    if (!ledc_initialized_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (blink_timer_ != nullptr) {
        esp_timer_stop(blink_timer_);
    }
    ledc_fade_stop(ledc_channel_.speed_mode, ledc_channel_.channel);
    is_on_ = false;
    ledc_set_duty(ledc_channel_.speed_mode, ledc_channel_.channel, 0);
    ledc_update_duty(ledc_channel_.speed_mode, ledc_channel_.channel);
}

void GpioLed::BlinkOnce() {
    Blink(1, 100);
}

void GpioLed::Blink(int times, int interval_ms) {
    StartBlinkTask(times, interval_ms);
}

void GpioLed::StartContinuousBlink(int interval_ms) {
    StartBlinkTask(BLINK_INFINITE, interval_ms);
}

void GpioLed::StartBlinkTask(int times, int interval_ms) {
    if (!ledc_initialized_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    esp_timer_stop(blink_timer_);
    ledc_fade_stop(ledc_channel_.speed_mode, ledc_channel_.channel);

    blink_counter_ = times * 2;
    blink_interval_ms_ = interval_ms;
    esp_timer_start_periodic(blink_timer_, interval_ms * 1000);
}

void GpioLed::OnBlinkTimer() {
    std::lock_guard<std::mutex> lock(mutex_);
    blink_counter_--;
    if (blink_counter_ & 1) {
        ledc_set_duty(ledc_channel_.speed_mode, ledc_channel_.channel, duty_);
    } else {
        ledc_set_duty(ledc_channel_.speed_mode, ledc_channel_.channel, 0);

        if (blink_counter_ == 0 && blink_timer_ != nullptr) {
            esp_timer_stop(blink_timer_);
        }
    }
    ledc_update_duty(ledc_channel_.speed_mode, ledc_channel_.channel);
}

void GpioLed::StartFadeTask() {
    if (!ledc_initialized_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (blink_timer_ != nullptr) {
        esp_timer_stop(blink_timer_);
    }
    ledc_fade_stop(ledc_channel_.speed_mode, ledc_channel_.channel);
    fade_up_ = true;
    ledc_set_fade_with_time(ledc_channel_.speed_mode,
                            ledc_channel_.channel, LEDC_DUTY, LEDC_FADE_TIME);
    ledc_fade_start(ledc_channel_.speed_mode,
                    ledc_channel_.channel, LEDC_FADE_NO_WAIT);
}

void GpioLed::OnFadeEnd() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!ledc_initialized_) return;
    fade_up_ = !fade_up_;
    ledc_set_fade_with_time(ledc_channel_.speed_mode,
                            ledc_channel_.channel, fade_up_ ? LEDC_DUTY : 0, LEDC_FADE_TIME);
    ledc_fade_start(ledc_channel_.speed_mode,
                    ledc_channel_.channel, LEDC_FADE_NO_WAIT);
}

bool IRAM_ATTR GpioLed::FadeCallback(const ledc_cb_param_t *param, void *user_arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (param->event == LEDC_FADE_END_EVT && user_arg != nullptr) {
        auto led = static_cast<GpioLed*>(user_arg);
        if (led->event_task_handle_ != nullptr) {
            xTaskNotifyFromISR(led->event_task_handle_, 0x01, eSetValueWithOverwrite,
                               &xHigherPriorityTaskWoken);
        }
    }
    return xHigherPriorityTaskWoken == pdTRUE;
}

void GpioLed::OnStateChanged() {
    if (custom_mode_) {
        return;
    }
    auto& app = Application::GetInstance();
    auto device_state = app.GetDeviceState();
    switch (device_state) {
        case kDeviceStateStarting:
            SetBrightness(DEFAULT_BRIGHTNESS);
            StartContinuousBlink(100);
            break;
        case kDeviceStateWifiConfiguring:
            SetBrightness(DEFAULT_BRIGHTNESS);
            StartContinuousBlink(500);
            break;
        case kDeviceStateIdle:
            SetBrightness(IDLE_BRIGHTNESS);
            TurnOn();
            // TurnOff();
            break;
        case kDeviceStateConnecting:
            SetBrightness(DEFAULT_BRIGHTNESS);
            TurnOn();
            break;
        case kDeviceStateListening:
        case kDeviceStateAudioTesting:
            if (app.IsVoiceDetected()) {
                SetBrightness(HIGH_BRIGHTNESS);
            } else {
                SetBrightness(LOW_BRIGHTNESS);
            }
            // TurnOn();
            StartFadeTask();
            break;
        case kDeviceStateSpeaking:
        case kDeviceStateNotifying:
            SetBrightness(SPEAKING_BRIGHTNESS);
            TurnOn();
            break;
        case kDeviceStateUpgrading:
            SetBrightness(UPGRADING_BRIGHTNESS);
            StartContinuousBlink(100);
            break;
        case kDeviceStateActivating:
            SetBrightness(ACTIVATING_BRIGHTNESS);
            StartContinuousBlink(500);
            break;
        case kDeviceStateFatalError:
            SetBrightness(HIGH_BRIGHTNESS);
            StartContinuousBlink(100);
            break;
        default:
            ESP_LOGW(TAG, "Unknown gpio led event: %d", device_state);
            return;
    }
}

void GpioLed::EventTask(void* arg) {
    GpioLed* led = static_cast<GpioLed*>(arg);

    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        led->OnFadeEnd();
    }
}