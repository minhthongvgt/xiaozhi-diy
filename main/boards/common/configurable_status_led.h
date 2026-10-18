#ifndef CONFIGURABLE_STATUS_LED_H
#define CONFIGURABLE_STATUS_LED_H
#include "application.h"
#include "device_state.h"
#include "led/circular_strip.h"
#include "led/gpio_led.h"
#include "led/single_led.h"
#include <sdkconfig.h>
#include <algorithm>
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
    kScroll = 6,
    kScanner = 7,
    kWipe = 8,
    kAudioBar = 10,
    kAudioMirror = 11,
    kAudioPulse = 12,
};

enum class StatusLedColorSource : uint8_t {
    kFixed = 0,
    kRainbowWhole = 1,
    kRainbowChase = 2,
};

typedef struct StatusLedStyle {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    StatusLedEffect effect;
    uint8_t level;
    StatusLedColorSource source;
    int rainbow_step_ms;
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

#define STATUS_LED_SOURCE_OF(state) STATUS_LED_SOURCE_##state
#define STATUS_LED_RAINBOW_STEP_OF(state) STATUS_LED_RAINBOW_STEP_##state

#if defined(CONFIG_CUSTOM_LED_STATE_STARTING_COLOR_SOURCE_RAINBOW) && defined(CONFIG_CUSTOM_LED_STATE_STARTING_RAINBOW_MODE_CHASE)
#define STATUS_LED_SOURCE_STARTING StatusLedColorSource::kRainbowChase
#elif defined(CONFIG_CUSTOM_LED_STATE_STARTING_COLOR_SOURCE_RAINBOW)
#define STATUS_LED_SOURCE_STARTING StatusLedColorSource::kRainbowWhole
#else
#define STATUS_LED_SOURCE_STARTING StatusLedColorSource::kFixed
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_STARTING_RAINBOW_SPEED_MS)
#define STATUS_LED_RAINBOW_STEP_STARTING CONFIG_CUSTOM_LED_STATE_STARTING_RAINBOW_SPEED_MS
#else
#define STATUS_LED_RAINBOW_STEP_STARTING STATUS_LED_SPEED_MS
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_COLOR_SOURCE_RAINBOW) && defined(CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_RAINBOW_MODE_CHASE)
#define STATUS_LED_SOURCE_WIFI_CONFIG StatusLedColorSource::kRainbowChase
#elif defined(CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_COLOR_SOURCE_RAINBOW)
#define STATUS_LED_SOURCE_WIFI_CONFIG StatusLedColorSource::kRainbowWhole
#else
#define STATUS_LED_SOURCE_WIFI_CONFIG StatusLedColorSource::kFixed
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_RAINBOW_SPEED_MS)
#define STATUS_LED_RAINBOW_STEP_WIFI_CONFIG CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_RAINBOW_SPEED_MS
#else
#define STATUS_LED_RAINBOW_STEP_WIFI_CONFIG STATUS_LED_SPEED_MS
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_IDLE_COLOR_SOURCE_RAINBOW) && defined(CONFIG_CUSTOM_LED_STATE_IDLE_RAINBOW_MODE_CHASE)
#define STATUS_LED_SOURCE_IDLE StatusLedColorSource::kRainbowChase
#elif defined(CONFIG_CUSTOM_LED_STATE_IDLE_COLOR_SOURCE_RAINBOW)
#define STATUS_LED_SOURCE_IDLE StatusLedColorSource::kRainbowWhole
#else
#define STATUS_LED_SOURCE_IDLE StatusLedColorSource::kFixed
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_IDLE_RAINBOW_SPEED_MS)
#define STATUS_LED_RAINBOW_STEP_IDLE CONFIG_CUSTOM_LED_STATE_IDLE_RAINBOW_SPEED_MS
#else
#define STATUS_LED_RAINBOW_STEP_IDLE STATUS_LED_SPEED_MS
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_CONNECTING_COLOR_SOURCE_RAINBOW) && defined(CONFIG_CUSTOM_LED_STATE_CONNECTING_RAINBOW_MODE_CHASE)
#define STATUS_LED_SOURCE_CONNECTING StatusLedColorSource::kRainbowChase
#elif defined(CONFIG_CUSTOM_LED_STATE_CONNECTING_COLOR_SOURCE_RAINBOW)
#define STATUS_LED_SOURCE_CONNECTING StatusLedColorSource::kRainbowWhole
#else
#define STATUS_LED_SOURCE_CONNECTING StatusLedColorSource::kFixed
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_CONNECTING_RAINBOW_SPEED_MS)
#define STATUS_LED_RAINBOW_STEP_CONNECTING CONFIG_CUSTOM_LED_STATE_CONNECTING_RAINBOW_SPEED_MS
#else
#define STATUS_LED_RAINBOW_STEP_CONNECTING STATUS_LED_SPEED_MS
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_LISTENING_COLOR_SOURCE_RAINBOW) && defined(CONFIG_CUSTOM_LED_STATE_LISTENING_RAINBOW_MODE_CHASE)
#define STATUS_LED_SOURCE_LISTENING StatusLedColorSource::kRainbowChase
#elif defined(CONFIG_CUSTOM_LED_STATE_LISTENING_COLOR_SOURCE_RAINBOW)
#define STATUS_LED_SOURCE_LISTENING StatusLedColorSource::kRainbowWhole
#else
#define STATUS_LED_SOURCE_LISTENING StatusLedColorSource::kFixed
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_LISTENING_RAINBOW_SPEED_MS)
#define STATUS_LED_RAINBOW_STEP_LISTENING CONFIG_CUSTOM_LED_STATE_LISTENING_RAINBOW_SPEED_MS
#else
#define STATUS_LED_RAINBOW_STEP_LISTENING STATUS_LED_SPEED_MS
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_SPEAKING_COLOR_SOURCE_RAINBOW) && defined(CONFIG_CUSTOM_LED_STATE_SPEAKING_RAINBOW_MODE_CHASE)
#define STATUS_LED_SOURCE_SPEAKING StatusLedColorSource::kRainbowChase
#elif defined(CONFIG_CUSTOM_LED_STATE_SPEAKING_COLOR_SOURCE_RAINBOW)
#define STATUS_LED_SOURCE_SPEAKING StatusLedColorSource::kRainbowWhole
#else
#define STATUS_LED_SOURCE_SPEAKING StatusLedColorSource::kFixed
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_SPEAKING_RAINBOW_SPEED_MS)
#define STATUS_LED_RAINBOW_STEP_SPEAKING CONFIG_CUSTOM_LED_STATE_SPEAKING_RAINBOW_SPEED_MS
#else
#define STATUS_LED_RAINBOW_STEP_SPEAKING STATUS_LED_SPEED_MS
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_UPGRADING_COLOR_SOURCE_RAINBOW) && defined(CONFIG_CUSTOM_LED_STATE_UPGRADING_RAINBOW_MODE_CHASE)
#define STATUS_LED_SOURCE_UPGRADING StatusLedColorSource::kRainbowChase
#elif defined(CONFIG_CUSTOM_LED_STATE_UPGRADING_COLOR_SOURCE_RAINBOW)
#define STATUS_LED_SOURCE_UPGRADING StatusLedColorSource::kRainbowWhole
#else
#define STATUS_LED_SOURCE_UPGRADING StatusLedColorSource::kFixed
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_UPGRADING_RAINBOW_SPEED_MS)
#define STATUS_LED_RAINBOW_STEP_UPGRADING CONFIG_CUSTOM_LED_STATE_UPGRADING_RAINBOW_SPEED_MS
#else
#define STATUS_LED_RAINBOW_STEP_UPGRADING STATUS_LED_SPEED_MS
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ACTIVATING_COLOR_SOURCE_RAINBOW) && defined(CONFIG_CUSTOM_LED_STATE_ACTIVATING_RAINBOW_MODE_CHASE)
#define STATUS_LED_SOURCE_ACTIVATING StatusLedColorSource::kRainbowChase
#elif defined(CONFIG_CUSTOM_LED_STATE_ACTIVATING_COLOR_SOURCE_RAINBOW)
#define STATUS_LED_SOURCE_ACTIVATING StatusLedColorSource::kRainbowWhole
#else
#define STATUS_LED_SOURCE_ACTIVATING StatusLedColorSource::kFixed
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ACTIVATING_RAINBOW_SPEED_MS)
#define STATUS_LED_RAINBOW_STEP_ACTIVATING CONFIG_CUSTOM_LED_STATE_ACTIVATING_RAINBOW_SPEED_MS
#else
#define STATUS_LED_RAINBOW_STEP_ACTIVATING STATUS_LED_SPEED_MS
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ERROR_COLOR_SOURCE_RAINBOW) && defined(CONFIG_CUSTOM_LED_STATE_ERROR_RAINBOW_MODE_CHASE)
#define STATUS_LED_SOURCE_ERROR StatusLedColorSource::kRainbowChase
#elif defined(CONFIG_CUSTOM_LED_STATE_ERROR_COLOR_SOURCE_RAINBOW)
#define STATUS_LED_SOURCE_ERROR StatusLedColorSource::kRainbowWhole
#else
#define STATUS_LED_SOURCE_ERROR StatusLedColorSource::kFixed
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ERROR_RAINBOW_SPEED_MS)
#define STATUS_LED_RAINBOW_STEP_ERROR CONFIG_CUSTOM_LED_STATE_ERROR_RAINBOW_SPEED_MS
#else
#define STATUS_LED_RAINBOW_STEP_ERROR STATUS_LED_SPEED_MS
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

inline bool IsKnownStatusLedEffect(int effect_id) {
    switch (static_cast<StatusLedEffect>(effect_id)) {
        case StatusLedEffect::kOff:
        case StatusLedEffect::kSteady:
        case StatusLedEffect::kBlinkFast:
        case StatusLedEffect::kBlinkSlow:
        case StatusLedEffect::kBreathe:
        case StatusLedEffect::kScroll:
        case StatusLedEffect::kScanner:
        case StatusLedEffect::kWipe:
        case StatusLedEffect::kAudioBar:
        case StatusLedEffect::kAudioMirror:
        case StatusLedEffect::kAudioPulse:
            return true;
        default:
            return false;
    }
}

inline StatusLedStyle MakeStatusLedStyle(const char* color, uint32_t fallback_color, int effect_id, uint8_t level,
                                         StatusLedColorSource source, int rainbow_step_ms) {
    const uint32_t rgb = ParseStatusLedColor(color, fallback_color);
    return StatusLedStyle{
        static_cast<uint8_t>((rgb >> 16) & 0xFF),
        static_cast<uint8_t>((rgb >> 8) & 0xFF),
        static_cast<uint8_t>(rgb & 0xFF),
        IsKnownStatusLedEffect(effect_id) ? static_cast<StatusLedEffect>(effect_id) : StatusLedEffect::kSteady,
        level,
        source,
        rainbow_step_ms > 0 ? rainbow_step_ms : 1,
    };
}

#define STATUS_LED_STYLE_OF(state, color, rgb, effect, level) \
    MakeStatusLedStyle(STATUS_LED_COLOR_OF(state, color), rgb, STATUS_LED_EFFECT_OF(state, effect), level, \
                       STATUS_LED_SOURCE_OF(state), STATUS_LED_RAINBOW_STEP_OF(state))

inline bool CurrentStatusLedStyle(StatusLedStyle& style) {
    auto& app = Application::GetInstance();
    switch (app.GetDeviceState()) {
        case kDeviceStateStarting:
            style = STATUS_LED_STYLE_OF(STARTING, "0000FF", 0x0000FF, 2, STATUS_LED_LEVEL_NORMAL);
            return true;
        case kDeviceStateWifiConfiguring:
            style = STATUS_LED_STYLE_OF(WIFI_CONFIG, "0000FF", 0x0000FF, 3, STATUS_LED_LEVEL_NORMAL);
            return true;
        case kDeviceStateIdle:
            style = STATUS_LED_STYLE_OF(IDLE, "0000FF", 0x0000FF, 0, STATUS_LED_LEVEL_LOW);
            return true;
        case kDeviceStateConnecting:
            style = STATUS_LED_STYLE_OF(CONNECTING, "0000FF", 0x0000FF, 1, STATUS_LED_LEVEL_NORMAL);
            return true;
        case kDeviceStateListening:
        case kDeviceStateAudioTesting:
            style = STATUS_LED_STYLE_OF(LISTENING, "FF0000", 0xFF0000, 1,
                                        app.IsVoiceDetected() ? STATUS_LED_LEVEL_HIGH : STATUS_LED_LEVEL_NORMAL);
            return true;
        case kDeviceStateSpeaking:
        case kDeviceStateNotifying:
            style = STATUS_LED_STYLE_OF(SPEAKING, "00FF00", 0x00FF00, 1, STATUS_LED_LEVEL_NORMAL);
            return true;
        case kDeviceStateUpgrading:
            style = STATUS_LED_STYLE_OF(UPGRADING, "00FF00", 0x00FF00, 2, STATUS_LED_LEVEL_NORMAL);
            return true;
        case kDeviceStateActivating:
            style = STATUS_LED_STYLE_OF(ACTIVATING, "00FF00", 0x00FF00, 3, STATUS_LED_LEVEL_NORMAL);
            return true;
        case kDeviceStateFatalError:
            style = STATUS_LED_STYLE_OF(ERROR, "FF0000", 0xFF0000, 2, STATUS_LED_LEVEL_HIGH);
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

inline StripColor StatusLedWheel(uint8_t position) {
    const uint8_t pos = static_cast<uint8_t>(255 - position);
    if (pos < 85) {
        return StripColor{static_cast<uint8_t>(255 - pos * 3), 0, static_cast<uint8_t>(pos * 3)};
    }
    if (pos < 170) {
        const uint8_t shifted = static_cast<uint8_t>(pos - 85);
        return StripColor{0, static_cast<uint8_t>(shifted * 3), static_cast<uint8_t>(255 - shifted * 3)};
    }
    const uint8_t shifted = static_cast<uint8_t>(pos - 170);
    return StripColor{static_cast<uint8_t>(shifted * 3), static_cast<uint8_t>(255 - shifted * 3), 0};
}

inline uint8_t StatusLedRainbowHue(int64_t elapsed_ms, int step_ms) {
    return static_cast<uint8_t>(((elapsed_ms / step_ms) * 4) & 0xFF);
}

inline StripColor StatusLedBaseColor(const StatusLedStyle& style, int index, int count, uint8_t hue) {
    switch (style.source) {
        case StatusLedColorSource::kRainbowWhole:
            return StatusLedWheel(hue);
        case StatusLedColorSource::kRainbowChase: {
            const int cycle = count > 64 ? 48 : (count > 0 ? count : 1);
            return StatusLedWheel(static_cast<uint8_t>(hue + (index % cycle) * 256 / cycle));
        }
        default:
            return StripColor{style.red, style.green, style.blue};
    }
}

inline uint8_t StatusLedEffectMask(StatusLedEffect effect, int index, int count, int64_t elapsed_ms, int step_ms) {
    static constexpr int64_t kBreathePeriodMs = 1800;
    const int64_t step = elapsed_ms / step_ms;
    switch (effect) {
        case StatusLedEffect::kOff:
            return 0;
        case StatusLedEffect::kBlinkFast:
        case StatusLedEffect::kBlinkSlow:
            return ((elapsed_ms / StatusLedBlinkInterval(effect)) % 2 == 0) ? 255 : 0;
        case StatusLedEffect::kBreathe: {
            const int64_t phase = elapsed_ms % kBreathePeriodMs;
            const int64_t half = kBreathePeriodMs / 2;
            return static_cast<uint8_t>((phase < half ? phase : kBreathePeriodMs - phase) * 255 / half);
        }
        case StatusLedEffect::kScroll: {
            const int length = count > 3 ? 3 : std::max(1, count / 2);
            const int head = static_cast<int>(step % count);
            return ((index - head + count) % count) < length ? 255 : 0;
        }
        case StatusLedEffect::kScanner: {
            const int length = count > 3 ? 3 : std::max(1, count / 4);
            const int span = std::max(1, count - length);
            const int bounce = static_cast<int>(step % (2 * span));
            const int head = bounce < span ? bounce : 2 * span - bounce;
            const int distance = index < head ? head - index : (index >= head + length ? index - (head + length - 1) : 0);
            return distance == 0 ? 255 : (distance < 8 ? static_cast<uint8_t>(255 >> distance) : 0);
        }
        case StatusLedEffect::kWipe: {
            const int position = static_cast<int>(step % (2 * count));
            return position < count ? (index < position ? 255 : 0) : (index < position - count ? 0 : 255);
        }
        default:
            return 255;
    }
}

class StatusLedTicker {
public:
    using TickCallback = std::function<void(int64_t)>;

    StatusLedTicker() {
        const esp_timer_create_args_t args = {
            .callback = [](void* arg) { static_cast<StatusLedTicker*>(arg)->OnTick(); },
            .arg = this,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "led_rainbow",
            .skip_unhandled_events = true,
        };
        if (esp_timer_create(&args, &timer_) != ESP_OK) {
            timer_ = nullptr;
        }
    }

    void Start(int interval_ms, TickCallback tick) {
        Stop();
        std::lock_guard<std::mutex> lock(mutex_);
        tick_ = std::move(tick);
        start_us_ = esp_timer_get_time();
        if (timer_ != nullptr) {
            esp_timer_start_periodic(timer_, static_cast<uint64_t>(interval_ms) * 1000);
        }
    }

    void Stop() {
        if (timer_ != nullptr) {
            esp_timer_stop(timer_);
        }
        std::lock_guard<std::mutex> lock(mutex_);
        tick_ = nullptr;
    }

private:
    esp_timer_handle_t timer_ = nullptr;
    std::mutex mutex_;
    TickCallback tick_;
    int64_t start_us_ = 0;

    void OnTick() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (tick_) {
            tick_((esp_timer_get_time() - start_us_) / 1000);
        }
    }
};

inline int StatusLedFrameMs(int count) {
    static constexpr int kMinFrameMs = 30;
    return std::max(kMinFrameMs, count * 3 / 40);
}

class ConfigurableSingleLed : public SingleLed {
public:
    explicit ConfigurableSingleLed(gpio_num_t gpio) : SingleLed(gpio) {}

    void TurnOff() override {
        StopAnimations();
        SingleLed::TurnOff();
    }

    void OnStateChanged() override {
        StatusLedStyle style;
        if (IsCustomMode() || !CurrentStatusLedStyle(style)) {
            return;
        }
        StopAnimations();
        SetColor(style.red, style.green, style.blue);
        SetBrightness(StatusLedPercentTo255(style.level));
        if (IsStatusLedAudioEffect(style.effect)) {
            const uint8_t full = StatusLedPercentTo255(style.level);
            const int64_t start_ms = esp_timer_get_time() / 1000;
            follower_.Start([this, full, style, start_ms](uint8_t level) {
                if (IsCustomMode()) {
                    return;
                }
                if (style.source != StatusLedColorSource::kFixed) {
                    const uint8_t hue = StatusLedRainbowHue(esp_timer_get_time() / 1000 - start_ms, style.rainbow_step_ms);
                    const StripColor color = StatusLedWheel(hue);
                    SetColor(color.red, color.green, color.blue);
                }
                SetBrightness(static_cast<uint8_t>(static_cast<uint32_t>(full) * level / 255));
                TurnOn();
            });
            return;
        }
        if (style.effect != StatusLedEffect::kOff && style.source != StatusLedColorSource::kFixed) {
            StartRainbowEffect(style);
            return;
        }
        switch (style.effect) {
            case StatusLedEffect::kOff:
                SingleLed::TurnOff();
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
            default:
                TurnOn();
                break;
        }
    }

private:
    StatusLedAudioFollower follower_;
    StatusLedTicker ticker_;

    void StopAnimations() {
        follower_.Stop();
        ticker_.Stop();
    }

    void StartRainbowEffect(const StatusLedStyle& style) {
        const bool moving = style.effect == StatusLedEffect::kScroll || style.effect == StatusLedEffect::kScanner ||
                            style.effect == StatusLedEffect::kWipe;
        const StatusLedEffect effect = moving ? StatusLedEffect::kBlinkSlow : style.effect;
        const int step_ms = style.rainbow_step_ms;
        ticker_.Start(StatusLedFrameMs(1), [this, effect, step_ms](int64_t elapsed_ms) {
            if (IsCustomMode()) {
                return;
            }
            const uint8_t mask = StatusLedEffectMask(effect, 0, 1, elapsed_ms, STATUS_LED_SPEED_MS);
            if (mask == 0) {
                SingleLed::TurnOff();
                return;
            }
            const StripColor color = StatusLedWheel(StatusLedRainbowHue(elapsed_ms, step_ms));
            SetColor(static_cast<uint8_t>(color.red * mask / 255), static_cast<uint8_t>(color.green * mask / 255),
                     static_cast<uint8_t>(color.blue * mask / 255));
            TurnOn();
        });
    }
};

class ConfigurableCircularStrip : public CircularStrip {
public:
    ConfigurableCircularStrip(gpio_num_t gpio, uint16_t max_leds) : CircularStrip(gpio, max_leds) {}

    void TurnOff() override {
        StopAnimations();
        CircularStrip::TurnOff();
    }

    void OnStateChanged() override {
        StatusLedStyle style;
        if (IsCustomMode() || !CurrentStatusLedStyle(style)) {
            return;
        }
        StopAnimations();
        const uint8_t level = StatusLedPercentTo255(style.level);
        SetBrightnessLevels(level, static_cast<uint8_t>(level / 8));
        if (style.effect == StatusLedEffect::kOff) {
            FadeOut(50);
            return;
        }
        if (IsStatusLedAudioEffect(style.effect)) {
            StartAudioEffect(style, level);
            return;
        }
        if (style.source != StatusLedColorSource::kFixed) {
            StartRainbowEffect(style, level);
            return;
        }
        const StripColor color = ScaleStripColor(style.red, style.green, style.blue, level);
        const int speed = STATUS_LED_SPEED_MS;
        switch (style.effect) {
            case StatusLedEffect::kBlinkFast:
            case StatusLedEffect::kBlinkSlow:
                Blink(color, StatusLedBlinkInterval(style.effect));
                break;
            case StatusLedEffect::kBreathe:
                Breathe({0, 0, 0}, color, 30);
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
            default:
                SetAllColor(color);
                break;
        }
    }

private:
    StatusLedAudioFollower follower_;
    StatusLedTicker ticker_;
    std::vector<StripColor> frame_;

    void StopAnimations() {
        follower_.Stop();
        ticker_.Stop();
    }

    void StartRainbowEffect(const StatusLedStyle& style, uint8_t full) {
        const int count = GetLedCount();
        frame_.assign(count, StripColor{});
        ticker_.Start(StatusLedFrameMs(count), [this, style, full, count](int64_t elapsed_ms) {
            if (IsCustomMode()) {
                return;
            }
            const uint8_t hue = StatusLedRainbowHue(elapsed_ms, style.rainbow_step_ms);
            for (int i = 0; i < count; i++) {
                const uint8_t mask = StatusLedEffectMask(style.effect, i, count, elapsed_ms, STATUS_LED_SPEED_MS);
                const StripColor base = StatusLedBaseColor(style, i, count, hue);
                frame_[i] = ScaleStripColor(base.red, base.green, base.blue, static_cast<uint32_t>(full) * mask / 255);
            }
            SetMultiColors(frame_);
        });
    }

    void StartAudioEffect(const StatusLedStyle& style, uint8_t full) {
        const int count = GetLedCount();
        frame_.assign(count, StripColor{});
        const int64_t start_ms = esp_timer_get_time() / 1000;
        follower_.Start([this, style, full, count, start_ms](uint8_t level) {
            if (IsCustomMode()) {
                return;
            }
            const bool rainbow = style.source != StatusLedColorSource::kFixed;
            const uint8_t hue = rainbow ? StatusLedRainbowHue(esp_timer_get_time() / 1000 - start_ms, style.rainbow_step_ms) : 0;
            for (int i = 0; i < count; i++) {
                frame_[i] = StripColor{};
            }
            if (style.effect == StatusLedEffect::kAudioPulse) {
                const uint32_t scale = static_cast<uint32_t>(full) * level / 255;
                for (int i = 0; i < count; i++) {
                    const StripColor base = StatusLedBaseColor(style, i, count, hue);
                    frame_[i] = ScaleStripColor(base.red, base.green, base.blue, scale);
                }
            } else if (style.effect == StatusLedEffect::kAudioBar) {
                const int lit = level * count / 255;
                for (int i = 0; i < lit; i++) {
                    if (rainbow) {
                        const StripColor base = StatusLedBaseColor(style, i, count, hue);
                        frame_[i] = ScaleStripColor(base.red, base.green, base.blue, full);
                        continue;
                    }
                    const int position = count > 1 ? i * 255 / (count - 1) : 0;
                    const uint8_t r = static_cast<uint8_t>(position < 128 ? position * 2 : 255);
                    const uint8_t g = static_cast<uint8_t>(position < 128 ? 255 : (255 - position) * 2);
                    frame_[i] = ScaleStripColor(r, g, 0, full);
                }
            } else {
                const int half = (level * count / 255 + 1) / 2;
                for (int i = 0; i < half; i++) {
                    const int left = i % count;
                    const int right = (count - 1 - i + count) % count;
                    const StripColor left_base = StatusLedBaseColor(style, left, count, hue);
                    const StripColor right_base = StatusLedBaseColor(style, right, count, hue);
                    frame_[left] = ScaleStripColor(left_base.red, left_base.green, left_base.blue, full);
                    frame_[right] = ScaleStripColor(right_base.red, right_base.green, right_base.blue, full);
                }
            }
            SetMultiColors(frame_);
        });
    }
};

class ConfigurableGpioLed : public GpioLed {
public:
    ConfigurableGpioLed(gpio_num_t gpio, ledc_timer_t timer, ledc_channel_t channel) : GpioLed(gpio, 0, timer, channel) {}

    void TurnOff() override {
        follower_.Stop();
        GpioLed::TurnOff();
    }

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
                if (IsCustomMode()) {
                    return;
                }
                GpioLed::SetBrightness(static_cast<uint8_t>(static_cast<uint32_t>(full) * level / 255));
                TurnOn();
            });
            return;
        }
        switch (style.effect) {
            case StatusLedEffect::kOff:
                GpioLed::TurnOff();
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
