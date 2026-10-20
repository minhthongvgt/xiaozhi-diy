#ifndef CONFIGURABLE_STATUS_LED_H
#define CONFIGURABLE_STATUS_LED_H
#include "application.h"
#include "device_state.h"
#include "led/circular_strip.h"
#include "led/gpio_led.h"
#include "led/single_led.h"
#include <sdkconfig.h>
#include <algorithm>
#include <cstddef>
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
    kHeartbeat = 5,
    kScroll = 6,
    kScanner = 7,
    kWipe = 8,
    kTheater = 9,
    kAudioBar = 10,
    kAudioMirror = 11,
    kAudioPulse = 12,
    kStrobe = 13,
    kFlicker = 14,
    kComet = 15,
    kWave = 16,
    kTwinkle = 17,
    kDualSpin = 18,
};

enum class StatusLedColorSource : uint8_t {
    kFixed = 0,
    kRainbow = 1,
    kGradient = 2,
    kFire = 3,
    kOcean = 4,
    kForest = 5,
    kSunset = 6,
    kIce = 7,
    kPastel = 8,
    kParty = 9,
};

enum class StatusLedColorMode : uint8_t {
    kWhole = 0,
    kChase = 1,
};

typedef struct StatusLedStyle {
    StripColor primary;
    StripColor secondary;
    StatusLedEffect effect;
    uint8_t level;
    StatusLedColorSource source;
    StatusLedColorMode mode;
    int color_speed_percent;
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

#define STATUS_LED_DEFAULT_COLOR_SPEED_PERCENT 20
#define STATUS_LED_MAX_COLOR_SPEED_PERCENT 200

#define STATUS_LED_EFFECT_OF(state, fallback) STATUS_LED_EFFECT_##state(fallback)
#define STATUS_LED_COLOR_OF(state, fallback) STATUS_LED_COLOR_##state(fallback)
#define STATUS_LED_COLOR2_OF(state) STATUS_LED_COLOR2_##state
#define STATUS_LED_SOURCE_OF(state) STATUS_LED_SOURCE_##state
#define STATUS_LED_MODE_OF(state) STATUS_LED_MODE_##state
#define STATUS_LED_COLOR_SPEED_OF(state) STATUS_LED_COLOR_SPEED_##state

#if defined(CONFIG_CUSTOM_LED_STATE_STARTING_EFFECT_ID)
#define STATUS_LED_EFFECT_STARTING(fallback) CONFIG_CUSTOM_LED_STATE_STARTING_EFFECT_ID
#else
#define STATUS_LED_EFFECT_STARTING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_STARTING_COLOR)
#define STATUS_LED_COLOR_STARTING(fallback) CONFIG_CUSTOM_LED_STATE_STARTING_COLOR
#else
#define STATUS_LED_COLOR_STARTING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_STARTING_COLOR2)
#define STATUS_LED_COLOR2_STARTING CONFIG_CUSTOM_LED_STATE_STARTING_COLOR2
#else
#define STATUS_LED_COLOR2_STARTING "FFFFFF"
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_STARTING_COLOR_SOURCE_ID)
#define STATUS_LED_SOURCE_STARTING CONFIG_CUSTOM_LED_STATE_STARTING_COLOR_SOURCE_ID
#else
#define STATUS_LED_SOURCE_STARTING 0
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_STARTING_RAINBOW_MODE_CHASE)
#define STATUS_LED_MODE_STARTING StatusLedColorMode::kChase
#else
#define STATUS_LED_MODE_STARTING StatusLedColorMode::kWhole
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_STARTING_RAINBOW_SPEED)
#define STATUS_LED_COLOR_SPEED_STARTING CONFIG_CUSTOM_LED_STATE_STARTING_RAINBOW_SPEED
#else
#define STATUS_LED_COLOR_SPEED_STARTING STATUS_LED_DEFAULT_COLOR_SPEED_PERCENT
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_EFFECT_ID)
#define STATUS_LED_EFFECT_WIFI_CONFIG(fallback) CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_EFFECT_ID
#else
#define STATUS_LED_EFFECT_WIFI_CONFIG(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_COLOR)
#define STATUS_LED_COLOR_WIFI_CONFIG(fallback) CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_COLOR
#else
#define STATUS_LED_COLOR_WIFI_CONFIG(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_COLOR2)
#define STATUS_LED_COLOR2_WIFI_CONFIG CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_COLOR2
#else
#define STATUS_LED_COLOR2_WIFI_CONFIG "FFFFFF"
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_COLOR_SOURCE_ID)
#define STATUS_LED_SOURCE_WIFI_CONFIG CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_COLOR_SOURCE_ID
#else
#define STATUS_LED_SOURCE_WIFI_CONFIG 0
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_RAINBOW_MODE_CHASE)
#define STATUS_LED_MODE_WIFI_CONFIG StatusLedColorMode::kChase
#else
#define STATUS_LED_MODE_WIFI_CONFIG StatusLedColorMode::kWhole
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_RAINBOW_SPEED)
#define STATUS_LED_COLOR_SPEED_WIFI_CONFIG CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_RAINBOW_SPEED
#else
#define STATUS_LED_COLOR_SPEED_WIFI_CONFIG STATUS_LED_DEFAULT_COLOR_SPEED_PERCENT
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_IDLE_EFFECT_ID)
#define STATUS_LED_EFFECT_IDLE(fallback) CONFIG_CUSTOM_LED_STATE_IDLE_EFFECT_ID
#else
#define STATUS_LED_EFFECT_IDLE(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_IDLE_COLOR)
#define STATUS_LED_COLOR_IDLE(fallback) CONFIG_CUSTOM_LED_STATE_IDLE_COLOR
#else
#define STATUS_LED_COLOR_IDLE(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_IDLE_COLOR2)
#define STATUS_LED_COLOR2_IDLE CONFIG_CUSTOM_LED_STATE_IDLE_COLOR2
#else
#define STATUS_LED_COLOR2_IDLE "FFFFFF"
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_IDLE_COLOR_SOURCE_ID)
#define STATUS_LED_SOURCE_IDLE CONFIG_CUSTOM_LED_STATE_IDLE_COLOR_SOURCE_ID
#else
#define STATUS_LED_SOURCE_IDLE 0
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_IDLE_RAINBOW_MODE_CHASE)
#define STATUS_LED_MODE_IDLE StatusLedColorMode::kChase
#else
#define STATUS_LED_MODE_IDLE StatusLedColorMode::kWhole
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_IDLE_RAINBOW_SPEED)
#define STATUS_LED_COLOR_SPEED_IDLE CONFIG_CUSTOM_LED_STATE_IDLE_RAINBOW_SPEED
#else
#define STATUS_LED_COLOR_SPEED_IDLE STATUS_LED_DEFAULT_COLOR_SPEED_PERCENT
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_CONNECTING_EFFECT_ID)
#define STATUS_LED_EFFECT_CONNECTING(fallback) CONFIG_CUSTOM_LED_STATE_CONNECTING_EFFECT_ID
#else
#define STATUS_LED_EFFECT_CONNECTING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_CONNECTING_COLOR)
#define STATUS_LED_COLOR_CONNECTING(fallback) CONFIG_CUSTOM_LED_STATE_CONNECTING_COLOR
#else
#define STATUS_LED_COLOR_CONNECTING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_CONNECTING_COLOR2)
#define STATUS_LED_COLOR2_CONNECTING CONFIG_CUSTOM_LED_STATE_CONNECTING_COLOR2
#else
#define STATUS_LED_COLOR2_CONNECTING "FFFFFF"
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_CONNECTING_COLOR_SOURCE_ID)
#define STATUS_LED_SOURCE_CONNECTING CONFIG_CUSTOM_LED_STATE_CONNECTING_COLOR_SOURCE_ID
#else
#define STATUS_LED_SOURCE_CONNECTING 0
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_CONNECTING_RAINBOW_MODE_CHASE)
#define STATUS_LED_MODE_CONNECTING StatusLedColorMode::kChase
#else
#define STATUS_LED_MODE_CONNECTING StatusLedColorMode::kWhole
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_CONNECTING_RAINBOW_SPEED)
#define STATUS_LED_COLOR_SPEED_CONNECTING CONFIG_CUSTOM_LED_STATE_CONNECTING_RAINBOW_SPEED
#else
#define STATUS_LED_COLOR_SPEED_CONNECTING STATUS_LED_DEFAULT_COLOR_SPEED_PERCENT
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_LISTENING_EFFECT_ID)
#define STATUS_LED_EFFECT_LISTENING(fallback) CONFIG_CUSTOM_LED_STATE_LISTENING_EFFECT_ID
#else
#define STATUS_LED_EFFECT_LISTENING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_LISTENING_COLOR)
#define STATUS_LED_COLOR_LISTENING(fallback) CONFIG_CUSTOM_LED_STATE_LISTENING_COLOR
#else
#define STATUS_LED_COLOR_LISTENING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_LISTENING_COLOR2)
#define STATUS_LED_COLOR2_LISTENING CONFIG_CUSTOM_LED_STATE_LISTENING_COLOR2
#else
#define STATUS_LED_COLOR2_LISTENING "FFFFFF"
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_LISTENING_COLOR_SOURCE_ID)
#define STATUS_LED_SOURCE_LISTENING CONFIG_CUSTOM_LED_STATE_LISTENING_COLOR_SOURCE_ID
#else
#define STATUS_LED_SOURCE_LISTENING 0
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_LISTENING_RAINBOW_MODE_CHASE)
#define STATUS_LED_MODE_LISTENING StatusLedColorMode::kChase
#else
#define STATUS_LED_MODE_LISTENING StatusLedColorMode::kWhole
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_LISTENING_RAINBOW_SPEED)
#define STATUS_LED_COLOR_SPEED_LISTENING CONFIG_CUSTOM_LED_STATE_LISTENING_RAINBOW_SPEED
#else
#define STATUS_LED_COLOR_SPEED_LISTENING STATUS_LED_DEFAULT_COLOR_SPEED_PERCENT
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_SPEAKING_EFFECT_ID)
#define STATUS_LED_EFFECT_SPEAKING(fallback) CONFIG_CUSTOM_LED_STATE_SPEAKING_EFFECT_ID
#else
#define STATUS_LED_EFFECT_SPEAKING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_SPEAKING_COLOR)
#define STATUS_LED_COLOR_SPEAKING(fallback) CONFIG_CUSTOM_LED_STATE_SPEAKING_COLOR
#else
#define STATUS_LED_COLOR_SPEAKING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_SPEAKING_COLOR2)
#define STATUS_LED_COLOR2_SPEAKING CONFIG_CUSTOM_LED_STATE_SPEAKING_COLOR2
#else
#define STATUS_LED_COLOR2_SPEAKING "FFFFFF"
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_SPEAKING_COLOR_SOURCE_ID)
#define STATUS_LED_SOURCE_SPEAKING CONFIG_CUSTOM_LED_STATE_SPEAKING_COLOR_SOURCE_ID
#else
#define STATUS_LED_SOURCE_SPEAKING 0
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_SPEAKING_RAINBOW_MODE_CHASE)
#define STATUS_LED_MODE_SPEAKING StatusLedColorMode::kChase
#else
#define STATUS_LED_MODE_SPEAKING StatusLedColorMode::kWhole
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_SPEAKING_RAINBOW_SPEED)
#define STATUS_LED_COLOR_SPEED_SPEAKING CONFIG_CUSTOM_LED_STATE_SPEAKING_RAINBOW_SPEED
#else
#define STATUS_LED_COLOR_SPEED_SPEAKING STATUS_LED_DEFAULT_COLOR_SPEED_PERCENT
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_UPGRADING_EFFECT_ID)
#define STATUS_LED_EFFECT_UPGRADING(fallback) CONFIG_CUSTOM_LED_STATE_UPGRADING_EFFECT_ID
#else
#define STATUS_LED_EFFECT_UPGRADING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_UPGRADING_COLOR)
#define STATUS_LED_COLOR_UPGRADING(fallback) CONFIG_CUSTOM_LED_STATE_UPGRADING_COLOR
#else
#define STATUS_LED_COLOR_UPGRADING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_UPGRADING_COLOR2)
#define STATUS_LED_COLOR2_UPGRADING CONFIG_CUSTOM_LED_STATE_UPGRADING_COLOR2
#else
#define STATUS_LED_COLOR2_UPGRADING "FFFFFF"
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_UPGRADING_COLOR_SOURCE_ID)
#define STATUS_LED_SOURCE_UPGRADING CONFIG_CUSTOM_LED_STATE_UPGRADING_COLOR_SOURCE_ID
#else
#define STATUS_LED_SOURCE_UPGRADING 0
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_UPGRADING_RAINBOW_MODE_CHASE)
#define STATUS_LED_MODE_UPGRADING StatusLedColorMode::kChase
#else
#define STATUS_LED_MODE_UPGRADING StatusLedColorMode::kWhole
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_UPGRADING_RAINBOW_SPEED)
#define STATUS_LED_COLOR_SPEED_UPGRADING CONFIG_CUSTOM_LED_STATE_UPGRADING_RAINBOW_SPEED
#else
#define STATUS_LED_COLOR_SPEED_UPGRADING STATUS_LED_DEFAULT_COLOR_SPEED_PERCENT
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ACTIVATING_EFFECT_ID)
#define STATUS_LED_EFFECT_ACTIVATING(fallback) CONFIG_CUSTOM_LED_STATE_ACTIVATING_EFFECT_ID
#else
#define STATUS_LED_EFFECT_ACTIVATING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ACTIVATING_COLOR)
#define STATUS_LED_COLOR_ACTIVATING(fallback) CONFIG_CUSTOM_LED_STATE_ACTIVATING_COLOR
#else
#define STATUS_LED_COLOR_ACTIVATING(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ACTIVATING_COLOR2)
#define STATUS_LED_COLOR2_ACTIVATING CONFIG_CUSTOM_LED_STATE_ACTIVATING_COLOR2
#else
#define STATUS_LED_COLOR2_ACTIVATING "FFFFFF"
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ACTIVATING_COLOR_SOURCE_ID)
#define STATUS_LED_SOURCE_ACTIVATING CONFIG_CUSTOM_LED_STATE_ACTIVATING_COLOR_SOURCE_ID
#else
#define STATUS_LED_SOURCE_ACTIVATING 0
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ACTIVATING_RAINBOW_MODE_CHASE)
#define STATUS_LED_MODE_ACTIVATING StatusLedColorMode::kChase
#else
#define STATUS_LED_MODE_ACTIVATING StatusLedColorMode::kWhole
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ACTIVATING_RAINBOW_SPEED)
#define STATUS_LED_COLOR_SPEED_ACTIVATING CONFIG_CUSTOM_LED_STATE_ACTIVATING_RAINBOW_SPEED
#else
#define STATUS_LED_COLOR_SPEED_ACTIVATING STATUS_LED_DEFAULT_COLOR_SPEED_PERCENT
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ERROR_EFFECT_ID)
#define STATUS_LED_EFFECT_ERROR(fallback) CONFIG_CUSTOM_LED_STATE_ERROR_EFFECT_ID
#else
#define STATUS_LED_EFFECT_ERROR(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ERROR_COLOR)
#define STATUS_LED_COLOR_ERROR(fallback) CONFIG_CUSTOM_LED_STATE_ERROR_COLOR
#else
#define STATUS_LED_COLOR_ERROR(fallback) fallback
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ERROR_COLOR2)
#define STATUS_LED_COLOR2_ERROR CONFIG_CUSTOM_LED_STATE_ERROR_COLOR2
#else
#define STATUS_LED_COLOR2_ERROR "FFFFFF"
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ERROR_COLOR_SOURCE_ID)
#define STATUS_LED_SOURCE_ERROR CONFIG_CUSTOM_LED_STATE_ERROR_COLOR_SOURCE_ID
#else
#define STATUS_LED_SOURCE_ERROR 0
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ERROR_RAINBOW_MODE_CHASE)
#define STATUS_LED_MODE_ERROR StatusLedColorMode::kChase
#else
#define STATUS_LED_MODE_ERROR StatusLedColorMode::kWhole
#endif
#if defined(CONFIG_CUSTOM_LED_STATE_ERROR_RAINBOW_SPEED)
#define STATUS_LED_COLOR_SPEED_ERROR CONFIG_CUSTOM_LED_STATE_ERROR_RAINBOW_SPEED
#else
#define STATUS_LED_COLOR_SPEED_ERROR STATUS_LED_DEFAULT_COLOR_SPEED_PERCENT
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

inline StripColor StatusLedRgbToColor(uint32_t rgb) {
    return StripColor{static_cast<uint8_t>((rgb >> 16) & 0xFF), static_cast<uint8_t>((rgb >> 8) & 0xFF),
                      static_cast<uint8_t>(rgb & 0xFF)};
}

inline bool IsKnownStatusLedEffect(int effect_id) {
    return effect_id >= static_cast<int>(StatusLedEffect::kOff) && effect_id <= static_cast<int>(StatusLedEffect::kDualSpin);
}

inline bool IsKnownStatusLedColorSource(int source_id) {
    return source_id >= static_cast<int>(StatusLedColorSource::kFixed) &&
           source_id <= static_cast<int>(StatusLedColorSource::kParty);
}

inline StatusLedStyle MakeStatusLedStyle(const char* color, uint32_t fallback_color, const char* secondary_color,
                                         int effect_id, uint8_t level, int source_id, StatusLedColorMode mode,
                                         int color_speed_percent) {
    return StatusLedStyle{
        StatusLedRgbToColor(ParseStatusLedColor(color, fallback_color)),
        StatusLedRgbToColor(ParseStatusLedColor(secondary_color, 0xFFFFFF)),
        IsKnownStatusLedEffect(effect_id) ? static_cast<StatusLedEffect>(effect_id) : StatusLedEffect::kSteady,
        level,
        IsKnownStatusLedColorSource(source_id) ? static_cast<StatusLedColorSource>(source_id) : StatusLedColorSource::kFixed,
        mode,
        std::clamp(color_speed_percent, 1, STATUS_LED_MAX_COLOR_SPEED_PERCENT),
    };
}

#define STATUS_LED_STYLE_OF(state, color, rgb, effect, level)                                                      \
    MakeStatusLedStyle(STATUS_LED_COLOR_OF(state, color), rgb, STATUS_LED_COLOR2_OF(state),                         \
                       STATUS_LED_EFFECT_OF(state, effect), level, STATUS_LED_SOURCE_OF(state), STATUS_LED_MODE_OF(state), \
                       STATUS_LED_COLOR_SPEED_OF(state))

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

inline bool IsStatusLedPatternEffect(StatusLedEffect effect) {
    switch (effect) {
        case StatusLedEffect::kHeartbeat:
        case StatusLedEffect::kTheater:
        case StatusLedEffect::kStrobe:
        case StatusLedEffect::kFlicker:
        case StatusLedEffect::kComet:
        case StatusLedEffect::kWave:
        case StatusLedEffect::kTwinkle:
        case StatusLedEffect::kDualSpin:
            return true;
        default:
            return false;
    }
}

inline StatusLedEffect StatusLedSinglePixelEffect(StatusLedEffect effect) {
    switch (effect) {
        case StatusLedEffect::kScroll:
        case StatusLedEffect::kScanner:
        case StatusLedEffect::kWipe:
        case StatusLedEffect::kTheater:
        case StatusLedEffect::kComet:
        case StatusLedEffect::kDualSpin:
            return StatusLedEffect::kBlinkSlow;
        case StatusLedEffect::kWave:
            return StatusLedEffect::kBreathe;
        default:
            return effect;
    }
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

inline StripColor ScaleStripColor(StripColor color, uint32_t scale) {
    return ScaleStripColor(color.red, color.green, color.blue, scale);
}

inline uint8_t BlendStatusLedChannel(uint8_t from, uint8_t to, uint32_t amount) {
    return static_cast<uint8_t>((static_cast<uint32_t>(from) * (256 - amount) + static_cast<uint32_t>(to) * amount) / 256);
}

inline StripColor BlendStripColor(StripColor from, StripColor to, uint32_t amount) {
    return StripColor{BlendStatusLedChannel(from.red, to.red, amount), BlendStatusLedChannel(from.green, to.green, amount),
                      BlendStatusLedChannel(from.blue, to.blue, amount)};
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

inline constexpr StripColor kStatusLedPaletteFire[] = {{255, 0, 0}, {255, 70, 0}, {255, 170, 0}, {255, 40, 0}};
inline constexpr StripColor kStatusLedPaletteOcean[] = {{0, 20, 255}, {0, 110, 255}, {0, 230, 200}, {0, 60, 170}};
inline constexpr StripColor kStatusLedPaletteForest[] = {{0, 255, 0}, {130, 255, 0}, {0, 150, 40}, {60, 200, 0}};
inline constexpr StripColor kStatusLedPaletteSunset[] = {{120, 0, 255}, {255, 0, 140}, {255, 80, 0}, {255, 0, 60}};
inline constexpr StripColor kStatusLedPaletteIce[] = {{180, 210, 255}, {0, 140, 255}, {255, 255, 255}, {70, 200, 255}};
inline constexpr StripColor kStatusLedPalettePastel[] = {{255, 140, 180}, {255, 210, 140}, {140, 255, 190}, {150, 170, 255}};
inline constexpr StripColor kStatusLedPaletteParty[] = {{150, 0, 255}, {255, 0, 120}, {255, 200, 0}, {0, 200, 255}};

template <size_t N>
inline StripColor SampleStatusLedPalette(const StripColor (&stops)[N], uint8_t position) {
    const uint32_t scaled = static_cast<uint32_t>(position) * N;
    const size_t index = scaled / 256;
    return BlendStripColor(stops[index], stops[(index + 1) % N], scaled % 256);
}

inline StripColor StatusLedSourceColor(const StatusLedStyle& style, uint8_t position) {
    switch (style.source) {
        case StatusLedColorSource::kRainbow:
            return StatusLedWheel(position);
        case StatusLedColorSource::kGradient: {
            const StripColor stops[] = {style.primary, style.secondary};
            return SampleStatusLedPalette(stops, position);
        }
        case StatusLedColorSource::kFire:
            return SampleStatusLedPalette(kStatusLedPaletteFire, position);
        case StatusLedColorSource::kOcean:
            return SampleStatusLedPalette(kStatusLedPaletteOcean, position);
        case StatusLedColorSource::kForest:
            return SampleStatusLedPalette(kStatusLedPaletteForest, position);
        case StatusLedColorSource::kSunset:
            return SampleStatusLedPalette(kStatusLedPaletteSunset, position);
        case StatusLedColorSource::kIce:
            return SampleStatusLedPalette(kStatusLedPaletteIce, position);
        case StatusLedColorSource::kPastel:
            return SampleStatusLedPalette(kStatusLedPalettePastel, position);
        case StatusLedColorSource::kParty:
            return SampleStatusLedPalette(kStatusLedPaletteParty, position);
        default:
            return style.primary;
    }
}

inline uint8_t StatusLedColorPhase(int64_t elapsed_ms, int speed_percent) {
    static constexpr int64_t kPhaseUnitsPerSecondAtFullSpeed = 400;
    return static_cast<uint8_t>((elapsed_ms * speed_percent * kPhaseUnitsPerSecondAtFullSpeed / 100000) & 0xFF);
}

inline StripColor StatusLedBaseColor(const StatusLedStyle& style, int index, int count, uint8_t phase) {
    if (style.source == StatusLedColorSource::kFixed) {
        return style.primary;
    }
    if (style.mode == StatusLedColorMode::kChase) {
        const int cycle = count > 64 ? 48 : (count > 0 ? count : 1);
        return StatusLedSourceColor(style, static_cast<uint8_t>(phase + (index % cycle) * 256 / cycle));
    }
    return StatusLedSourceColor(style, phase);
}

inline uint8_t StatusLedNoise(uint32_t first, uint32_t second) {
    uint32_t value = first * 374761393u + second * 668265263u;
    value = (value ^ (value >> 13)) * 1274126177u;
    return static_cast<uint8_t>((value ^ (value >> 16)) >> 8);
}

inline uint8_t StatusLedTriangle(int64_t phase, int64_t width) {
    const int64_t half = width / 2;
    const int64_t distance = phase < half ? phase : width - phase;
    return static_cast<uint8_t>(std::clamp<int64_t>(distance * 255 / half, 0, 255));
}

inline int StatusLedRingDistance(int first, int second, int count) {
    const int direct = std::abs(first - second);
    return std::min(direct, count - direct);
}

inline uint8_t StatusLedEffectMask(StatusLedEffect effect, int index, int count, int64_t elapsed_ms, int step_ms) {
    static constexpr int64_t kBreathePeriodMs = 1800;
    static constexpr int64_t kHeartbeatPeriodMs = 1200;
    static constexpr int64_t kHeartbeatPulseMs = 160;
    static constexpr int64_t kHeartbeatSecondPulseStartMs = 260;
    static constexpr int64_t kStrobePeriodMs = 400;
    static constexpr int64_t kStrobeFlashMs = 40;
    static constexpr int64_t kFlickerFrameMs = 60;
    static constexpr int64_t kTwinklePeriodMs = 1400;
    static constexpr int64_t kTwinkleGlowMs = 600;
    static constexpr uint8_t kTwinkleFloor = 8;
    static constexpr uint8_t kWaveFloor = 16;
    const int64_t step = elapsed_ms / step_ms;
    switch (effect) {
        case StatusLedEffect::kOff:
            return 0;
        case StatusLedEffect::kBlinkFast:
        case StatusLedEffect::kBlinkSlow:
            return ((elapsed_ms / StatusLedBlinkInterval(effect)) % 2 == 0) ? 255 : 0;
        case StatusLedEffect::kBreathe:
            return StatusLedTriangle(elapsed_ms % kBreathePeriodMs, kBreathePeriodMs);
        case StatusLedEffect::kHeartbeat: {
            const int64_t phase = elapsed_ms % kHeartbeatPeriodMs;
            if (phase < kHeartbeatPulseMs) {
                return StatusLedTriangle(phase, kHeartbeatPulseMs);
            }
            const int64_t second = phase - kHeartbeatSecondPulseStartMs;
            if (second >= 0 && second < kHeartbeatPulseMs) {
                return static_cast<uint8_t>(StatusLedTriangle(second, kHeartbeatPulseMs) * 3 / 4);
            }
            return 0;
        }
        case StatusLedEffect::kStrobe:
            return (elapsed_ms % kStrobePeriodMs) < kStrobeFlashMs ? 255 : 0;
        case StatusLedEffect::kFlicker: {
            const uint8_t noise = StatusLedNoise(static_cast<uint32_t>(index), static_cast<uint32_t>(elapsed_ms / kFlickerFrameMs));
            return noise < 16 ? 48 : static_cast<uint8_t>(96 + noise * 159 / 255);
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
        case StatusLedEffect::kTheater: {
            const int offset = static_cast<int>((step / 2) % 3);
            return ((index + 3 - offset) % 3) == 0 ? 255 : 0;
        }
        case StatusLedEffect::kComet: {
            const int head = static_cast<int>(step % count);
            const int tail = std::max(2, count / 3);
            const int distance = (head - index + count) % count;
            if (distance >= tail) {
                return 0;
            }
            const int remaining = tail - distance;
            return static_cast<uint8_t>(remaining * remaining * 255 / (tail * tail));
        }
        case StatusLedEffect::kWave: {
            const uint8_t position = static_cast<uint8_t>(index * 256 / count - step * 8);
            const uint32_t triangle = position < 128 ? position * 2u : (255u - position) * 2u;
            return static_cast<uint8_t>(kWaveFloor + (triangle * triangle / 255) * (255 - kWaveFloor) / 255);
        }
        case StatusLedEffect::kTwinkle: {
            const int64_t local = elapsed_ms + static_cast<int64_t>(StatusLedNoise(static_cast<uint32_t>(index), 7)) * 8;
            const uint32_t cycle = static_cast<uint32_t>(local / kTwinklePeriodMs);
            const int64_t phase = local % kTwinklePeriodMs;
            if (StatusLedNoise(static_cast<uint32_t>(index), cycle) % 3 != 0 || phase >= kTwinkleGlowMs) {
                return kTwinkleFloor;
            }
            return std::max(kTwinkleFloor, StatusLedTriangle(phase, kTwinkleGlowMs));
        }
        case StatusLedEffect::kDualSpin: {
            const int first = static_cast<int>(step % count);
            const int second = (count - first) % count;
            const int distance = std::min(StatusLedRingDistance(index, first, count), StatusLedRingDistance(index, second, count));
            return distance == 0 ? 255 : (distance == 1 ? 96 : 0);
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
            .name = "led_pattern",
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
        SetColor(style.primary.red, style.primary.green, style.primary.blue);
        SetBrightness(StatusLedPercentTo255(style.level));
        if (IsStatusLedAudioEffect(style.effect)) {
            const uint8_t full = StatusLedPercentTo255(style.level);
            const int64_t start_ms = esp_timer_get_time() / 1000;
            follower_.Start([this, full, style, start_ms](uint8_t level) {
                if (IsCustomMode()) {
                    return;
                }
                if (style.source != StatusLedColorSource::kFixed) {
                    const uint8_t phase = StatusLedColorPhase(esp_timer_get_time() / 1000 - start_ms, style.color_speed_percent);
                    const StripColor color = StatusLedSourceColor(style, phase);
                    SetColor(color.red, color.green, color.blue);
                }
                SetBrightness(static_cast<uint8_t>(static_cast<uint32_t>(full) * level / 255));
                TurnOn();
            });
            return;
        }
        const StatusLedEffect effect = StatusLedSinglePixelEffect(style.effect);
        if (effect != StatusLedEffect::kOff &&
            (style.source != StatusLedColorSource::kFixed || IsStatusLedPatternEffect(effect))) {
            StartPatternEffect(style, effect);
            return;
        }
        switch (effect) {
            case StatusLedEffect::kOff:
                SingleLed::TurnOff();
                break;
            case StatusLedEffect::kBlinkFast:
            case StatusLedEffect::kBlinkSlow:
                StartContinuousBlink(StatusLedBlinkInterval(effect));
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

    void StartPatternEffect(const StatusLedStyle& style, StatusLedEffect effect) {
        ticker_.Start(StatusLedFrameMs(1), [this, style, effect](int64_t elapsed_ms) {
            if (IsCustomMode()) {
                return;
            }
            const uint8_t mask = StatusLedEffectMask(effect, 0, 1, elapsed_ms, STATUS_LED_SPEED_MS);
            if (mask == 0) {
                SingleLed::TurnOff();
                return;
            }
            const StripColor color = ScaleStripColor(
                StatusLedBaseColor(style, 0, 1, StatusLedColorPhase(elapsed_ms, style.color_speed_percent)), mask);
            SetColor(color.red, color.green, color.blue);
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
        if (style.source != StatusLedColorSource::kFixed || IsStatusLedPatternEffect(style.effect)) {
            StartPatternEffect(style, level);
            return;
        }
        const StripColor color = ScaleStripColor(style.primary, level);
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

    void StartPatternEffect(const StatusLedStyle& style, uint8_t full) {
        const int count = GetLedCount();
        frame_.assign(count, StripColor{});
        ticker_.Start(StatusLedFrameMs(count), [this, style, full, count](int64_t elapsed_ms) {
            if (IsCustomMode()) {
                return;
            }
            const uint8_t phase = StatusLedColorPhase(elapsed_ms, style.color_speed_percent);
            for (int i = 0; i < count; i++) {
                const uint8_t mask = StatusLedEffectMask(style.effect, i, count, elapsed_ms, STATUS_LED_SPEED_MS);
                frame_[i] = ScaleStripColor(StatusLedBaseColor(style, i, count, phase), static_cast<uint32_t>(full) * mask / 255);
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
            const bool colorful = style.source != StatusLedColorSource::kFixed;
            const uint8_t phase =
                colorful ? StatusLedColorPhase(esp_timer_get_time() / 1000 - start_ms, style.color_speed_percent) : 0;
            for (int i = 0; i < count; i++) {
                frame_[i] = StripColor{};
            }
            if (style.effect == StatusLedEffect::kAudioPulse) {
                const uint32_t scale = static_cast<uint32_t>(full) * level / 255;
                for (int i = 0; i < count; i++) {
                    frame_[i] = ScaleStripColor(StatusLedBaseColor(style, i, count, phase), scale);
                }
            } else if (style.effect == StatusLedEffect::kAudioBar) {
                const int lit = level * count / 255;
                for (int i = 0; i < lit; i++) {
                    if (colorful) {
                        frame_[i] = ScaleStripColor(StatusLedBaseColor(style, i, count, phase), full);
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
                    frame_[left] = ScaleStripColor(StatusLedBaseColor(style, left, count, phase), full);
                    frame_[right] = ScaleStripColor(StatusLedBaseColor(style, right, count, phase), full);
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
        StopAnimations();
        GpioLed::TurnOff();
    }

    void OnStateChanged() override {
        StatusLedStyle style;
        if (IsCustomMode() || !CurrentStatusLedStyle(style)) {
            return;
        }
        StopAnimations();
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
        const StatusLedEffect effect = StatusLedSinglePixelEffect(style.effect);
        if (IsStatusLedPatternEffect(effect)) {
            ticker_.Start(StatusLedFrameMs(1), [this, full, effect](int64_t elapsed_ms) {
                if (IsCustomMode()) {
                    return;
                }
                const uint8_t mask = StatusLedEffectMask(effect, 0, 1, elapsed_ms, STATUS_LED_SPEED_MS);
                GpioLed::SetBrightness(static_cast<uint8_t>(static_cast<uint32_t>(full) * mask / 255));
                TurnOn();
            });
            return;
        }
        switch (effect) {
            case StatusLedEffect::kOff:
                GpioLed::TurnOff();
                break;
            case StatusLedEffect::kBlinkFast:
            case StatusLedEffect::kBlinkSlow:
                GpioLed::StartBlink(StatusLedBlinkInterval(effect));
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
    StatusLedTicker ticker_;

    void StopAnimations() {
        follower_.Stop();
        ticker_.Stop();
    }
};
#endif
