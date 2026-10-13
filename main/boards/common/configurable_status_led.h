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

enum class StatusLedEffect : int { kOff = 0, kSteady = 1, kBlinkFast = 2, kBlinkSlow = 3, kBreathe = 4, kRainbow = 5 };

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
    const bool known = effect_id >= static_cast<int>(StatusLedEffect::kOff) && effect_id <= static_cast<int>(StatusLedEffect::kRainbow);
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

class ConfigurableSingleLed : public SingleLed {
public:
    explicit ConfigurableSingleLed(gpio_num_t gpio) : SingleLed(gpio) {}

    void OnStateChanged() override {
        StatusLedStyle style;
        if (IsCustomMode() || !CurrentStatusLedStyle(style)) {
            return;
        }
        SetColor(style.red, style.green, style.blue);
        SetBrightness(StatusLedPercentTo255(style.level));
        switch (style.effect) {
            case StatusLedEffect::kOff:
                TurnOff();
                break;
            case StatusLedEffect::kSteady:
                TurnOn();
                break;
            case StatusLedEffect::kBlinkFast:
            case StatusLedEffect::kBlinkSlow:
                StartContinuousBlink(StatusLedBlinkInterval(style.effect));
                break;
            case StatusLedEffect::kBreathe:
                SingleLed::StartBreathe(20);
                break;
            case StatusLedEffect::kRainbow:
                SingleLed::StartRainbow(25, StatusLedPercentTo255(style.level));
                break;
        }
    }
};

class ConfigurableCircularStrip : public CircularStrip {
public:
    ConfigurableCircularStrip(gpio_num_t gpio, uint16_t max_leds) : CircularStrip(gpio, max_leds) {}

    void OnStateChanged() override {
        StatusLedStyle style;
        if (IsCustomMode() || !CurrentStatusLedStyle(style)) {
            return;
        }
        const uint8_t level = StatusLedPercentTo255(style.level);
        const StripColor color = {
            static_cast<uint8_t>(static_cast<uint32_t>(style.red) * level / 255),
            static_cast<uint8_t>(static_cast<uint32_t>(style.green) * level / 255),
            static_cast<uint8_t>(static_cast<uint32_t>(style.blue) * level / 255),
        };
        switch (style.effect) {
            case StatusLedEffect::kOff:
                FadeOut(50);
                break;
            case StatusLedEffect::kSteady:
                SetAllColor(color);
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
                if (GetLedCount() > 1) {
                    RainbowChase(25);
                } else {
                    Rainbow(25);
                }
                break;
        }
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
        GpioLed::SetBrightness(StatusLedPercentTo255(style.level));
        switch (style.effect) {
            case StatusLedEffect::kOff:
                TurnOff();
                break;
            case StatusLedEffect::kSteady:
            case StatusLedEffect::kRainbow:
                TurnOn();
                break;
            case StatusLedEffect::kBlinkFast:
            case StatusLedEffect::kBlinkSlow:
                GpioLed::StartBlink(StatusLedBlinkInterval(style.effect));
                break;
            case StatusLedEffect::kBreathe:
                GpioLed::StartBreathe();
                break;
        }
    }
};
#endif
