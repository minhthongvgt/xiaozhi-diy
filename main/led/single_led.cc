#include "single_led.h"
#include "application.h"
#include <esp_log.h>
#include <sdkconfig.h>

#define TAG "SingleLed"

#define DEFAULT_BRIGHTNESS 16
#define HIGH_BRIGHTNESS 32
#define LOW_BRIGHTNESS 4

#define BLINK_INFINITE -1

// Fast 256-step integer color wheel for rainbow effects (zero float, zero heap allocation)
static inline void GetRainbowColor(uint8_t pos, uint8_t brightness, uint8_t& r, uint8_t& g, uint8_t& b) {
    pos = 255 - pos;
    if (pos < 85) {
        r = (uint16_t)(255 - pos * 3) * brightness / 255;
        g = 0;
        b = (uint16_t)(pos * 3) * brightness / 255;
    } else if (pos < 170) {
        pos -= 85;
        r = 0;
        g = (uint16_t)(pos * 3) * brightness / 255;
        b = (uint16_t)(255 - pos * 3) * brightness / 255;
    } else {
        pos -= 170;
        r = (uint16_t)(pos * 3) * brightness / 255;
        g = (uint16_t)(255 - pos * 3) * brightness / 255;
        b = 0;
    }
}

SingleLed::SingleLed(gpio_num_t gpio) {
    if (gpio == GPIO_NUM_NC) {
        ESP_LOGW(TAG, "SingleLed initialized with GPIO_NUM_NC, LED will not function");
        return;
    }

    led_strip_config_t strip_config = {};
    strip_config.strip_gpio_num = gpio;
    strip_config.max_leds = 1;
    strip_config.color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB;
    strip_config.led_model = LED_MODEL_WS2812;

    led_strip_rmt_config_t rmt_config = {};
    rmt_config.resolution_hz = 10 * 1000 * 1000; // 10MHz

    esp_err_t ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create SingleLed RMT device on GPIO %d: %s", gpio, esp_err_to_name(ret));
        led_strip_ = nullptr;
        return;
    }
    led_strip_clear(led_strip_);

    esp_timer_create_args_t timer_args = {
        .callback = [](void *arg) {
            auto led = static_cast<SingleLed*>(arg);
            led->OnTimer();
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "single_led_timer",
        .skip_unhandled_events = false,
    };
    ret = esp_timer_create(&timer_args, &timer_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create single_led_timer: %s", esp_err_to_name(ret));
        timer_ = nullptr;
    }
}

SingleLed::~SingleLed() {
    if (timer_ != nullptr) {
        esp_timer_stop(timer_);
        esp_timer_delete(timer_);
        timer_ = nullptr;
    }
    if (led_strip_ != nullptr) {
        led_strip_clear(led_strip_);
        led_strip_del(led_strip_);
        led_strip_ = nullptr;
    }
}

void SingleLed::SetColor(uint8_t r, uint8_t g, uint8_t b) {
    std::lock_guard<std::mutex> lock(mutex_);
    r_ = r;
    g_ = g;
    b_ = b;
}

void SingleLed::SetBrightness(uint8_t brightness) {
    rainbow_brightness_ = brightness;
    if (custom_mode_ && mode_ == EffectMode::kNone && led_strip_ != nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        uint8_t r = (uint16_t)r_ * rainbow_brightness_ / 255;
        uint8_t g = (uint16_t)g_ * rainbow_brightness_ / 255;
        uint8_t b = (uint16_t)b_ * rainbow_brightness_ / 255;
        led_strip_set_pixel(led_strip_, 0, r, g, b);
        led_strip_refresh(led_strip_);
    }
}

void SingleLed::TurnOn() {
    if (led_strip_ == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (timer_ != nullptr) {
        esp_timer_stop(timer_);
    }
    mode_ = EffectMode::kNone;
    uint8_t r = (uint16_t)r_ * rainbow_brightness_ / 255;
    uint8_t g = (uint16_t)g_ * rainbow_brightness_ / 255;
    uint8_t b = (uint16_t)b_ * rainbow_brightness_ / 255;
    if ((r_ || g_ || b_) && !r && !g && !b && rainbow_brightness_ > 0) {
        r = r_ ? 1 : 0;
        g = g_ ? 1 : 0;
        b = b_ ? 1 : 0;
    }
    led_strip_set_pixel(led_strip_, 0, r, g, b);
    led_strip_refresh(led_strip_);
}

void SingleLed::TurnOff() {
    if (led_strip_ == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (timer_ != nullptr) {
        esp_timer_stop(timer_);
    }
    mode_ = EffectMode::kNone;
    led_strip_clear(led_strip_);
}

void SingleLed::StopEffect() {
    TurnOff();
}

void SingleLed::BlinkOnce() {
    Blink(1, 100);
}

void SingleLed::Blink(int times, int interval_ms) {
    StartBlinkTask(times, interval_ms);
}

void SingleLed::StartContinuousBlink(int interval_ms) {
    StartBlinkTask(BLINK_INFINITE, interval_ms);
}

void SingleLed::StartBlinkTask(int times, int interval_ms) {
    if (led_strip_ == nullptr || timer_ == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    esp_timer_stop(timer_);

    mode_ = EffectMode::kBlink;
    blink_counter_ = times * 2;
    blink_interval_ms_ = interval_ms;
    esp_timer_start_periodic(timer_, interval_ms * 1000);
}

void SingleLed::StartRainbow(int interval_ms, uint8_t brightness) {
    if (led_strip_ == nullptr || timer_ == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    esp_timer_stop(timer_);

    mode_ = EffectMode::kRainbow;
    rainbow_pos_ = 0;
    rainbow_brightness_ = brightness;
    esp_timer_start_periodic(timer_, interval_ms * 1000);
}

void SingleLed::StartBreathe(int interval_ms) {
    if (led_strip_ == nullptr || timer_ == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    esp_timer_stop(timer_);

    mode_ = EffectMode::kBreathe;
    breathe_up_ = true;
    breathe_val_ = 0;
    if (r_ == 0 && g_ == 0 && b_ == 0) {
        r_ = 0;
        g_ = rainbow_brightness_;
        b_ = rainbow_brightness_;
    }
    esp_timer_start_periodic(timer_, interval_ms * 1000);
}

void SingleLed::OnTimer() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (led_strip_ == nullptr) {
        return;
    }

    if (mode_ == EffectMode::kBlink) {
        blink_counter_--;
        if (blink_counter_ & 1) {
            uint8_t r = (uint16_t)r_ * rainbow_brightness_ / 255;
            uint8_t g = (uint16_t)g_ * rainbow_brightness_ / 255;
            uint8_t b = (uint16_t)b_ * rainbow_brightness_ / 255;
            led_strip_set_pixel(led_strip_, 0, r, g, b);
            led_strip_refresh(led_strip_);
        } else {
            led_strip_clear(led_strip_);
            if (blink_counter_ == 0) {
                esp_timer_stop(timer_);
                mode_ = EffectMode::kNone;
            }
        }
    } else if (mode_ == EffectMode::kRainbow) {
        rainbow_pos_ += 4; // Advance hue smoothly
        uint8_t r = 0, g = 0, b = 0;
        GetRainbowColor(rainbow_pos_, rainbow_brightness_, r, g, b);
        led_strip_set_pixel(led_strip_, 0, r, g, b);
        led_strip_refresh(led_strip_);
    } else if (mode_ == EffectMode::kBreathe) {
        if (breathe_up_) {
            if (breathe_val_ >= 250) {
                breathe_up_ = false;
            } else {
                breathe_val_ += 5;
            }
        } else {
            if (breathe_val_ <= 5) {
                breathe_up_ = true;
            } else {
                breathe_val_ -= 5;
            }
        }
        uint32_t scale = (uint32_t)rainbow_brightness_ * breathe_val_ / 255;
        uint8_t r = (uint32_t)r_ * scale / 255;
        uint8_t g = (uint32_t)g_ * scale / 255;
        uint8_t b = (uint32_t)b_ * scale / 255;
        led_strip_set_pixel(led_strip_, 0, r, g, b);
        led_strip_refresh(led_strip_);
    }
}

void SingleLed::OnStateChanged() {
    if (custom_mode_) {
        return;
    }
    auto& app = Application::GetInstance();
    auto device_state = app.GetDeviceState();
    switch (device_state) {
        case kDeviceStateStarting:
            SetColor(0, 0, DEFAULT_BRIGHTNESS);
            StartContinuousBlink(100);
            break;
        case kDeviceStateWifiConfiguring:
            SetColor(0, 0, DEFAULT_BRIGHTNESS);
            StartContinuousBlink(500);
            break;
        case kDeviceStateIdle:
            TurnOff();
            break;
        case kDeviceStateConnecting:
            SetColor(0, 0, DEFAULT_BRIGHTNESS);
            TurnOn();
            break;
        case kDeviceStateListening:
        case kDeviceStateAudioTesting:
            if (app.IsVoiceDetected()) {
                SetColor(HIGH_BRIGHTNESS, 0, 0);
            } else {
                SetColor(LOW_BRIGHTNESS, 0, 0);
            }
            TurnOn();
            break;
        case kDeviceStateSpeaking:
#if defined(CONFIG_CUSTOM_LED_FAST_RAINBOW_EFFECT)
            StartRainbow(25, DEFAULT_BRIGHTNESS);
#else
            SetColor(0, DEFAULT_BRIGHTNESS, 0);
            TurnOn();
#endif
            break;
        case kDeviceStateNotifying:
            SetColor(0, DEFAULT_BRIGHTNESS, 0);
            TurnOn();
            break;
        case kDeviceStateUpgrading:
            SetColor(0, DEFAULT_BRIGHTNESS, 0);
            StartContinuousBlink(100);
            break;
        case kDeviceStateActivating:
            SetColor(0, DEFAULT_BRIGHTNESS, 0);
            StartContinuousBlink(500);
            break;
        case kDeviceStateFatalError:
            SetColor(HIGH_BRIGHTNESS, 0, 0);
            StartContinuousBlink(100);
            break;
        default:
            ESP_LOGW(TAG, "Unknown device state for led: %d", device_state);
            return;
    }
}
