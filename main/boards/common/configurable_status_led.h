#ifndef CONFIGURABLE_STATUS_LED_H
#define CONFIGURABLE_STATUS_LED_H
#include "application.h"
#include "device_state.h"
#include "led/circular_strip.h"
#include "led/gpio_led.h"
#include "led/single_led.h"
#include <sdkconfig.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <mutex>
#include <vector>

enum class StatusLedEffect : int {
    kOff = 0,
    kSteady = 1,
    kBlinkFast = 2,
    kBlinkSlow = 3,
    kBreathe = 4,
    kRainbow = 5,
    kScroll = 6,
    kScanner = 7,
    kWipe = 8,
    kRainbowChase = 9,
    kAudioBar = 10,
    kAudioMirror = 11,
    kAudioPulse = 12,
};

typedef struct StatusLedStyle {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    StatusLedEffect effect;
    uint8_t level;
} StatusLedStyle;

#if defined(CONFIG_CUSTOM_LED_BRIGHTNESS_NORMAL)
#define STATUS_LED_LEVEL_NORMAL CONFIG_CUSTOM_LED_BRIGHTNESS_NORMAL
#else
#define STATUS_LED_LEVEL_NORMAL 10
#endif
#if defined(CONFIG_CUSTOM_LED_BRIGHTNESS_LOW)
#define STATUS_LED_LEVEL_LOW CONFIG_CUSTOM_LED_BRIGHTNESS_LOW
#else
#define STATUS_LED_LEVEL_LOW 2
#endif
#if defined(CONFIG_CUSTOM_LED_BRIGHTNESS_HIGH)
#define STATUS_LED_LEVEL_HIGH CONFIG_CUSTOM_LED_BRIGHTNESS_HIGH
#else
#define STATUS_LED_LEVEL_HIGH 40
#endif

#if defined(CONFIG_CUSTOM_LED_EFFECT_SPEED_MS)
#define STATUS_LED_SPEED_MS CONFIG_CUSTOM_LED_EFFECT_SPEED_MS
#else
#define STATUS_LED_SPEED_MS 50
#endif
#if defined(CONFIG_CUSTOM_LED_AUDIO_DELAY_MS)
#define STATUS_LED_AUDIO_DELAY_MS CONFIG_CUSTOM_LED_AUDIO_DELAY_MS
#else
#define STATUS_LED_AUDIO_DELAY_MS 200
#endif

#define STATUS_LED_EFFECT_OF(state, fallback) STATUS_LED_EFFECT_##state(fallback)
#define STATUS_LED_COLOR_OF(state, fallback) STATUS_LED_COLOR_##state(fallback)

#if defined(CONFIG_CUSTOM_LED_STATE_STARTING_EFFECT_ID)
#define STATUS_LED_EFFECT_STARTING(fallback) CONFIG_CUSTOM_LED_STATE_STARTING_EFFECT_ID
#else
#define STATUS_LED_EFFECT_STARTING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_EFFECT_ID)
#define STATUS_LED_EFFECT_WIFI_CONFIG(fallback) CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_EFFECT_ID
#else
#define STATUS_LED_EFFECT_WIFI_CONFIG(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_IDLE_EFFECT_ID)
#define STATUS_LED_EFFECT_IDLE(fallback) CONFIG_CUSTOM_LED_STATE_IDLE_EFFECT_ID
#else
#define STATUS_LED_EFFECT_IDLE(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_CONNECTING_EFFECT_ID)
#define STATUS_LED_EFFECT_CONNECTING(fallback) CONFIG_CUSTOM_LED_STATE_CONNECTING_EFFECT_ID
#else
#define STATUS_LED_EFFECT_CONNECTING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_LISTENING_EFFECT_ID)
#define STATUS_LED_EFFECT_LISTENING(fallback) CONFIG_CUSTOM_LED_STATE_LISTENING_EFFECT_ID
#else
#define STATUS_LED_EFFECT_LISTENING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_SPEAKING_EFFECT_ID)
#define STATUS_LED_EFFECT_SPEAKING(fallback) CONFIG_CUSTOM_LED_STATE_SPEAKING_EFFECT_ID
#else
#define STATUS_LED_EFFECT_SPEAKING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_UPGRADING_EFFECT_ID)
#define STATUS_LED_EFFECT_UPGRADING(fallback) CONFIG_CUSTOM_LED_STATE_UPGRADING_EFFECT_ID
#else
#define STATUS_LED_EFFECT_UPGRADING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ACTIVATING_EFFECT_ID)
#define STATUS_LED_EFFECT_ACTIVATING(fallback) CONFIG_CUSTOM_LED_STATE_ACTIVATING_EFFECT_ID
#else
#define STATUS_LED_EFFECT_ACTIVATING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ERROR_EFFECT_ID)
#define STATUS_LED_EFFECT_ERROR(fallback) CONFIG_CUSTOM_LED_STATE_ERROR_EFFECT_ID
#else
#define STATUS_LED_EFFECT_ERROR(fallback) fallback
#endif

#if defined(CONFIG_CUSTOM_LED_STATE_STARTING_COLOR)
#define STATUS_LED_COLOR_STARTING(fallback) CONFIG_CUSTOM_LED_STATE_STARTING_COLOR
#else
#define STATUS_LED_COLOR_STARTING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_COLOR)
#define STATUS_LED_COLOR_WIFI_CONFIG(fallback) CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_COLOR
#else
#define STATUS_LED_COLOR_WIFI_CONFIG(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_IDLE_COLOR)
#define STATUS_LED_COLOR_IDLE(fallback) CONFIG_CUSTOM_LED_STATE_IDLE_COLOR
#else
#define STATUS_LED_COLOR_IDLE(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_CONNECTING_COLOR)
#define STATUS_LED_COLOR_CONNECTING(fallback) CONFIG_CUSTOM_LED_STATE_CONNECTING_COLOR
#else
#define STATUS_LED_COLOR_CONNECTING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_LISTENING_COLOR)
#define STATUS_LED_COLOR_LISTENING(fallback) CONFIG_CUSTOM_LED_STATE_LISTENING_COLOR
#else
#define STATUS_LED_COLOR_LISTENING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_SPEAKING_COLOR)
#define STATUS_LED_COLOR_SPEAKING(fallback) CONFIG_CUSTOM_LED_STATE_SPEAKING_COLOR
#else
#define STATUS_LED_COLOR_SPEAKING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_UPGRADING_COLOR)
#define STATUS_LED_COLOR_UPGRADING(fallback) CONFIG_CUSTOM_LED_STATE_UPGRADING_COLOR
#else
#define STATUS_LED_COLOR_UPGRADING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ACTIVATING_COLOR)
#define STATUS_LED_COLOR_ACTIVATING(fallback) CONFIG_CUSTOM_LED_STATE_ACTIVATING_COLOR
#else
#define STATUS_LED_COLOR_ACTIVATING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ERROR_COLOR)
#define STATUS_LED_COLOR_ERROR(fallback) CONFIG_CUSTOM_LED_STATE_ERROR_COLOR
#else
#define STATUS_LED_COLOR_ERROR(fallback) fallback
#endif

inline uint32_t ParseStatusLedColor(const char* text, uint32_t fallback) {
    if (text == nullptr) {
        return fallback;
    }
    if (text[0] == '#') {
        text++;
    }
    if (std::strlen(text) != 6) {
        return fallback;
    }
    char* end = nullptr;
    unsigned long value = std::strtoul(text, &end, 16);
    return (end != nullptr && *end == '\0') ? static_cast<uint32_t>(value) : fallback;
}

inline StatusLedStyle MakeStatusLedStyle(const char* color, uint32_t fallback_color, int effect_id, uint8_t level) {
    const uint32_t rgb = ParseStatusLedColor(color, fallback_color);
    const bool known = effect_id >= static_cast<int>(StatusLedEffect::kOff) && effect_id <= static_cast<int>(StatusLedEffect::kAudioPulse);
    return StatusLedStyle{
        static_cast<uint8_t>((rgb >> 16) & 0xFF),
        static_cast<uint8_t>((rgb >> 8) & 0xFF),
        static_cast<uint8_t>(rgb & 0xFF),
        known ? static_cast<StatusLedEffect>(effect_id) : StatusLedEffect::kSteady,
        level,
    };
}

inline bool CurrentStatusLedStyle(StatusLedStyle& style) {
    auto& app = Application::GetInstance();
    switch (app.GetDeviceState()) {
        case kDeviceStateStarting:
            style = MakeStatusLedStyle(STATUS_LED_COLOR_OF(STARTING, "0000FF"), 0x0000FF, STATUS_LED_EFFECT_OF(STARTING, 2), STATUS_LED_LEVEL_NORMAL);
            return true;
        case kDeviceStateWifiConfiguring:
            style = MakeStatusLedStyle(STATUS_LED_COLOR_OF(WIFI_CONFIG, "0000FF"), 0x0000FF, STATUS_LED_EFFECT_OF(WIFI_CONFIG, 3), STATUS_LED_LEVEL_NORMAL);
            return true;
        case kDeviceStateIdle:
            style = MakeStatusLedStyle(STATUS_LED_COLOR_OF(IDLE, "0000FF"), 0x0000FF, STATUS_LED_EFFECT_OF(IDLE, 0), STATUS_LED_LEVEL_LOW);
            return true;
        case kDeviceStateConnecting:
            style = MakeStatusLedStyle(STATUS_LED_COLOR_OF(CONNECTING, "0000FF"), 0x0000FF, STATUS_LED_EFFECT_OF(CONNECTING, 1), STATUS_LED_LEVEL_NORMAL);
            return true;
        case kDeviceStateListening:
        case kDeviceStateAudioTesting:
            style = MakeStatusLedStyle(STATUS_LED_COLOR_OF(LISTENING, "FF0000"), 0xFF0000, STATUS_LED_EFFECT_OF(LISTENING, 1),
                                       app.IsVoiceDetected() ? STATUS_LED_LEVEL_HIGH : STATUS_LED_LEVEL_NORMAL);
            return true;
        case kDeviceStateSpeaking:
        case kDeviceStateNotifying:
            style = MakeStatusLedStyle(STATUS_LED_COLOR_OF(SPEAKING, "00FF00"), 0x00FF00, STATUS_LED_EFFECT_OF(SPEAKING, 1), STATUS_LED_LEVEL_NORMAL);
            return true;
        case kDeviceStateUpgrading:
            style = MakeStatusLedStyle(STATUS_LED_COLOR_OF(UPGRADING, "00FF00"), 0x00FF00, STATUS_LED_EFFECT_OF(UPGRADING, 2), STATUS_LED_LEVEL_NORMAL);
            return true;
        case kDeviceStateActivating:
            style = MakeStatusLedStyle(STATUS_LED_COLOR_OF(ACTIVATING, "00FF00"), 0x00FF00, STATUS_LED_EFFECT_OF(ACTIVATING, 3), STATUS_LED_LEVEL_NORMAL);
            return true;
        case kDeviceStateFatalError:
            style = MakeStatusLedStyle(STATUS_LED_COLOR_OF(ERROR, "FF0000"), 0xFF0000, STATUS_LED_EFFECT_OF(ERROR, 2), STATUS_LED_LEVEL_HIGH);
            return true;
        default:
            return false;
    }
}

inline uint8_t StatusLedPercentTo255(uint8_t percent) {
    return static_cast<uint8_t>(static_cast<uint32_t>(percent > 100 ? 100 : percent) * 255 / 100);
}

inline int StatusLedBlinkInterval(StatusLedEffect effect) {
    return effect == StatusLedEffect::kBlinkFast ? 100 : 500;
}

inline bool IsStatusLedAudioEffect(StatusLedEffect effect) {
    return effect == StatusLedEffect::kAudioBar || effect == StatusLedEffect::kAudioMirror || effect == StatusLedEffect::kAudioPulse;
}

class StatusLedAudioFollower {
public:
    using DrawCallback = std::function<void(uint8_t)>;

    StatusLedAudioFollower() {
        const esp_timer_create_args_t args = {
            .callback = [](void* arg) { static_cast<StatusLedAudioFollower*>(arg)->OnTick(); },
            .arg = this,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "led_audio",
            .skip_unhandled_events = true,
        };
        if (esp_timer_create(&args, &timer_) != ESP_OK) {
            timer_ = nullptr;
        }
    }

    void Start(DrawCallback draw) {
        Stop();
        std::lock_guard<std::mutex> lock(mutex_);
        draw_ = std::move(draw);
        history_.assign(kHistorySize, 0);
        index_ = 0;
        shown_ = 0;
        if (timer_ != nullptr) {
            esp_timer_start_periodic(timer_, static_cast<uint64_t>(kTickMs) * 1000);
        }
    }

    void Stop() {
        if (timer_ != nullptr) {
            esp_timer_stop(timer_);
        }
        std::lock_guard<std::mutex> lock(mutex_);
        draw_ = nullptr;
    }

private:
    static constexpr int kTickMs = 30;
    static constexpr size_t kHistorySize = 32;
    static constexpr int32_t kFullScalePeak = 24000;
    esp_timer_handle_t timer_ = nullptr;
    std::mutex mutex_;
    DrawCallback draw_;
    std::vector<uint8_t> history_;
    size_t index_ = 0;
    int shown_ = 0;

    void OnTick() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!draw_) {
            return;
        }
        const int32_t peak = Application::GetInstance().GetAudioService().GetOutputLevel();
        const int32_t level = peak >= kFullScalePeak ? 255 : peak * 255 / kFullScalePeak;
        history_[index_] = static_cast<uint8_t>(level);
        const size_t delay_steps = static_cast<size_t>(STATUS_LED_AUDIO_DELAY_MS / kTickMs);
        const size_t steps = delay_steps < kHistorySize ? delay_steps : kHistorySize - 1;
        const uint8_t delayed = history_[(index_ + kHistorySize - steps) % kHistorySize];
        index_ = (index_ + 1) % kHistorySize;
        shown_ = delayed > shown_ ? delayed : shown_ * 4 / 5;
        draw_(static_cast<uint8_t>(shown_));
    }
};

inline StripColor ScaleStripColor(uint8_t red, uint8_t green, uint8_t blue, uint32_t scale) {
    return StripColor{static_cast<uint8_t>(red * scale / 255), static_cast<uint8_t>(green * scale / 255),
                      static_cast<uint8_t>(blue * scale / 255)};
}

class ConfigurableSingleLed : public SingleLed {
public:
    explicit ConfigurableSingleLed(gpio_num_t gpio) : SingleLed(gpio) {}

    void OnStateChanged() override {
        StatusLedStyle style;
        if (IsCustomMode() || !CurrentStatusLedStyle(style)) {
            return;
        }
        follower_.Stop();
        SetColor(style.red, style.green, style.blue);
        SetBrightness(StatusLedPercentTo255(style.level));
        if (IsStatusLedAudioEffect(style.effect)) {
            const uint8_t full = StatusLedPercentTo255(style.level);
            follower_.Start([this, full](uint8_t level) {
                SetBrightness(static_cast<uint8_t>(static_cast<uint32_t>(full) * level / 255));
                TurnOn();
            });
            return;
        }
        switch (style.effect) {
            case StatusLedEffect::kOff:
                TurnOff();
                break;
            case StatusLedEffect::kBlinkFast:
            case StatusLedEffect::kBlinkSlow:
                StartContinuousBlink(StatusLedBlinkInterval(style.effect));
                break;
            case StatusLedEffect::kScroll:
            case StatusLedEffect::kScanner:
            case StatusLedEffect::kWipe:
                StartContinuousBlink(500);
                break;
            case StatusLedEffect::kBreathe:
                SingleLed::StartBreathe(20);
                break;
            case StatusLedEffect::kRainbow:
            case StatusLedEffect::kRainbowChase:
                SingleLed::StartRainbow(25, StatusLedPercentTo255(style.level));
                break;
            default:
                TurnOn();
                break;
        }
    }

private:
    StatusLedAudioFollower follower_;
};

class ConfigurableCircularStrip : public CircularStrip {
public:
    ConfigurableCircularStrip(gpio_num_t gpio, uint16_t max_leds) : CircularStrip(gpio, max_leds) {}

    void OnStateChanged() override {
        StatusLedStyle style;
        if (IsCustomMode() || !CurrentStatusLedStyle(style)) {
            return;
        }
        follower_.Stop();
        const uint8_t level = StatusLedPercentTo255(style.level);
        const StripColor color = ScaleStripColor(style.red, style.green, style.blue, level);
        const int speed = STATUS_LED_SPEED_MS;
        switch (style.effect) {
            case StatusLedEffect::kOff:
                FadeOut(50);
                break;
            case StatusLedEffect::kBlinkFast:
            case StatusLedEffect::kBlinkSlow:
                Blink(color, StatusLedBlinkInterval(style.effect));
                break;
            case StatusLedEffect::kBreathe:
                Breathe({0, 0, 0}, color, 30);
                break;
            case StatusLedEffect::kRainbow:
                CircularStrip::SetBrightness(level, static_cast<uint8_t>(level / 8));
                Rainbow(speed);
                break;
            case StatusLedEffect::kRainbowChase:
                CircularStrip::SetBrightness(level, static_cast<uint8_t>(level / 8));
                RainbowChase(speed);
                break;
            case StatusLedEffect::kScroll:
                Scroll({0, 0, 0}, color, 3, speed);
                break;
            case StatusLedEffect::kScanner:
                Scanner(color, 3, speed);
                break;
            case StatusLedEffect::kWipe:
                ColorWipe(color, speed);
                break;
            case StatusLedEffect::kAudioBar:
            case StatusLedEffect::kAudioMirror:
            case StatusLedEffect::kAudioPulse:
                StartAudioEffect(style, level);
                break;
            default:
                SetAllColor(color);
                break;
        }
    }

private:
    StatusLedAudioFollower follower_;
    std::vector<StripColor> frame_;

    void StartAudioEffect(const StatusLedStyle& style, uint8_t full) {
        const int count = GetLedCount();
        frame_.assign(count, StripColor{});
        const StatusLedEffect effect = style.effect;
        const uint8_t red = style.red;
        const uint8_t green = style.green;
        const uint8_t blue = style.blue;
        follower_.Start([this, effect, red, green, blue, full, count](uint8_t level) {
            for (int i = 0; i < count; i++) {
                frame_[i] = StripColor{};
            }
            if (effect == StatusLedEffect::kAudioPulse) {
                const StripColor color = ScaleStripColor(red, green, blue, static_cast<uint32_t>(full) * level / 255);
                for (int i = 0; i < count; i++) {
                    frame_[i] = color;
                }
            } else if (effect == StatusLedEffect::kAudioBar) {
                const int lit = level * count / 255;
                for (int i = 0; i < lit; i++) {
                    const int position = count > 1 ? i * 255 / (count - 1) : 0;
                    const uint8_t r = static_cast<uint8_t>(position < 128 ? position * 2 : 255);
                    const uint8_t g = static_cast<uint8_t>(position < 128 ? 255 : (255 - position) * 2);
                    frame_[i] = ScaleStripColor(r, g, 0, full);
                }
            } else {
                const int half = (level * count / 255 + 1) / 2;
                const StripColor color = ScaleStripColor(red, green, blue, full);
                for (int i = 0; i < half; i++) {
                    frame_[i % count] = color;
                    frame_[(count - 1 - i + count) % count] = color;
                }
            }
            SetMultiColors(frame_);
        });
    }
};

class ConfigurableGpioLed : public GpioLed {
public:
    ConfigurableGpioLed(gpio_num_t gpio, ledc_timer_t timer, ledc_channel_t channel) : GpioLed(gpio, 0, timer, channel) {}

    void OnStateChanged() override {
        StatusLedStyle style;
        if (IsCustomMode() || !CurrentStatusLedStyle(style)) {
            return;
        }
        follower_.Stop();
        const uint8_t full = StatusLedPercentTo255(style.level);
        GpioLed::SetBrightness(full);
        if (IsStatusLedAudioEffect(style.effect)) {
            follower_.Start([this, full](uint8_t level) {
                GpioLed::SetBrightness(static_cast<uint8_t>(static_cast<uint32_t>(full) * level / 255));
                TurnOn();
            });
            return;
        }
        switch (style.effect) {
            case StatusLedEffect::kOff:
                TurnOff();
                break;
            case StatusLedEffect::kBlinkFast:
            case StatusLedEffect::kBlinkSlow:
                GpioLed::StartBlink(StatusLedBlinkInterval(style.effect));
                break;
            case StatusLedEffect::kScroll:
            case StatusLedEffect::kScanner:
            case StatusLedEffect::kWipe:
                GpioLed::StartBlink(500);
                break;
            case StatusLedEffect::kBreathe:
                GpioLed::StartBreathe();
                break;
            default:
                TurnOn();
                break;
        }
    }

private:
    StatusLedAudioFollower follower_;
};
#endif
