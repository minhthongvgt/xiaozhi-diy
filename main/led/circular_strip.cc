#include "circular_strip.h"
#include "application.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <algorithm>

#define TAG "CircularStrip"

// Fast 256-step integer color wheel for rainbow effects (zero float, zero heap allocation)
static inline StripColor Wheel(uint8_t wheel_pos, uint8_t brightness) {
    wheel_pos = 255 - wheel_pos;
    if (wheel_pos < 85) {
        return {
            static_cast<uint8_t>((255 - wheel_pos * 3) * brightness / 255),
            0,
            static_cast<uint8_t>((wheel_pos * 3) * brightness / 255)
        };
    } else if (wheel_pos < 170) {
        wheel_pos -= 85;
        return {
            0,
            static_cast<uint8_t>((wheel_pos * 3) * brightness / 255),
            static_cast<uint8_t>((255 - wheel_pos * 3) * brightness / 255)
        };
    } else {
        wheel_pos -= 170;
        return {
            static_cast<uint8_t>((wheel_pos * 3) * brightness / 255),
            static_cast<uint8_t>((255 - wheel_pos * 3) * brightness / 255),
            0
        };
    }
}

CircularStrip::CircularStrip(gpio_num_t gpio, uint16_t max_leds) : max_leds_(max_leds) {
    if (max_leds_ <= 0) {
        max_leds_ = 1;
    }
    colors_.resize(max_leds_);

    if (gpio == GPIO_NUM_NC) {
        ESP_LOGW(TAG, "CircularStrip initialized with GPIO_NUM_NC, LED will not function");
        return;
    }

    led_strip_config_t strip_config = {};
    strip_config.strip_gpio_num = gpio;
    strip_config.max_leds = max_leds_;
    strip_config.color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB;
    strip_config.led_model = LED_MODEL_WS2812;

    led_strip_rmt_config_t rmt_config = {};
    rmt_config.resolution_hz = 10 * 1000 * 1000; // 10MHz
    if (max_leds_ > 24) {
        rmt_config.flags.with_dma = 1;
    }

    esp_err_t ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip_);
    if (ret != ESP_OK && rmt_config.flags.with_dma) {
        ESP_LOGW(TAG, "RMT with DMA failed (%s), falling back to standard RMT channel", esp_err_to_name(ret));
        rmt_config.flags.with_dma = 0;
        ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip_);
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create CircularStrip RMT device on GPIO %d: %s", gpio, esp_err_to_name(ret));
        led_strip_ = nullptr;
        return;
    }
    led_strip_clear(led_strip_);

    esp_timer_create_args_t strip_timer_args = {
        .callback = [](void *arg) {
            auto strip = static_cast<CircularStrip*>(arg);
            std::lock_guard<std::mutex> lock(strip->mutex_);
            if (strip->strip_callback_ != nullptr) {
                strip->strip_callback_();
            }
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "strip_timer",
        .skip_unhandled_events = false,
    };
    ret = esp_timer_create(&strip_timer_args, &strip_timer_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create strip_timer: %s", esp_err_to_name(ret));
        strip_timer_ = nullptr;
    }
}

CircularStrip::~CircularStrip() {
    if (strip_timer_ != nullptr) {
        esp_timer_stop(strip_timer_);
        esp_timer_delete(strip_timer_);
        strip_timer_ = nullptr;
    }
    if (led_strip_ != nullptr) {
        led_strip_clear(led_strip_);
        led_strip_del(led_strip_);
        led_strip_ = nullptr;
    }
}

void CircularStrip::SetAllColor(StripColor color) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (strip_timer_ != nullptr) {
        esp_timer_stop(strip_timer_);
    }
    strip_callback_ = nullptr;
    if (led_strip_ == nullptr) return;

    for (int i = 0; i < max_leds_; i++) {
        colors_[i] = color;
        led_strip_set_pixel(led_strip_, i, color.red, color.green, color.blue);
    }
    led_strip_refresh(led_strip_);
}

void CircularStrip::SetSingleColor(uint16_t index, StripColor color) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (strip_timer_ != nullptr) {
        esp_timer_stop(strip_timer_);
    }
    strip_callback_ = nullptr;
    if (led_strip_ == nullptr || index >= max_leds_ || index >= colors_.size()) return;

    colors_[index] = color;
    led_strip_set_pixel(led_strip_, index, color.red, color.green, color.blue);
    led_strip_refresh(led_strip_);
}

void CircularStrip::SetMultiColors(const std::vector<StripColor>& colors) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (strip_timer_ != nullptr) {
        esp_timer_stop(strip_timer_);
    }
    strip_callback_ = nullptr;
    if (led_strip_ == nullptr) return;

    int count = std::min(max_leds_, static_cast<int>(colors.size()));
    for (int i = 0; i < count; i++) {
        colors_[i] = colors[i];
        led_strip_set_pixel(led_strip_, i, colors[i].red, colors[i].green, colors[i].blue);
    }
    led_strip_refresh(led_strip_);
}

void CircularStrip::TurnOn() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (strip_timer_ != nullptr) {
        esp_timer_stop(strip_timer_);
    }
    strip_callback_ = nullptr;
    if (led_strip_ == nullptr) return;

    bool all_zero = true;
    for (const auto& c : colors_) {
        if (c.red != 0 || c.green != 0 || c.blue != 0) {
            all_zero = false;
            break;
        }
    }
    if (all_zero) {
        for (int i = 0; i < max_leds_ && i < static_cast<int>(colors_.size()); i++) {
            colors_[i] = { default_brightness_, default_brightness_, default_brightness_ };
            led_strip_set_pixel(led_strip_, i, default_brightness_, default_brightness_, default_brightness_);
        }
    } else {
        for (int i = 0; i < max_leds_ && i < static_cast<int>(colors_.size()); i++) {
            led_strip_set_pixel(led_strip_, i, colors_[i].red, colors_[i].green, colors_[i].blue);
        }
    }
    led_strip_refresh(led_strip_);
}

void CircularStrip::TurnOff() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (strip_timer_ != nullptr) {
        esp_timer_stop(strip_timer_);
    }
    strip_callback_ = nullptr;
    if (led_strip_ != nullptr) {
        led_strip_clear(led_strip_);
    }
}

void CircularStrip::Blink(StripColor color, int interval_ms) {
    if (led_strip_ == nullptr) return;
    for (int i = 0; i < max_leds_ && i < static_cast<int>(colors_.size()); i++) {
        colors_[i] = color;
    }
    blink_on_ = true;
    StartStripTask(interval_ms, [this]() {
        if (led_strip_ == nullptr) return;
        if (blink_on_) {
            for (int i = 0; i < max_leds_ && i < static_cast<int>(colors_.size()); i++) {
                led_strip_set_pixel(led_strip_, i, colors_[i].red, colors_[i].green, colors_[i].blue);
            }
            led_strip_refresh(led_strip_);
        } else {
            led_strip_clear(led_strip_);
        }
        blink_on_ = !blink_on_;
    });
}

void CircularStrip::FadeOut(int interval_ms) {
    if (led_strip_ == nullptr) return;
    StartStripTask(interval_ms, [this]() {
        if (led_strip_ == nullptr) return;
        bool all_off = true;
        for (int i = 0; i < max_leds_ && i < static_cast<int>(colors_.size()); i++) {
            colors_[i].red /= 2;
            colors_[i].green /= 2;
            colors_[i].blue /= 2;
            if (colors_[i].red != 0 || colors_[i].green != 0 || colors_[i].blue != 0) {
                all_off = false;
            }
            led_strip_set_pixel(led_strip_, i, colors_[i].red, colors_[i].green, colors_[i].blue);
        }
        if (all_off) {
            led_strip_clear(led_strip_);
            if (strip_timer_ != nullptr) {
                esp_timer_stop(strip_timer_);
            }
        } else {
            led_strip_refresh(led_strip_);
        }
    });
}

void CircularStrip::Breathe(StripColor low, StripColor high, int interval_ms) {
    if (led_strip_ == nullptr) return;
    breathe_up_    = true;
    breathe_color_ = low;
    int delta = std::max({std::abs(high.red - low.red), std::abs(high.green - low.green), std::abs(high.blue - low.blue)});
    int step = std::max(1, delta / 30);

    StartStripTask(interval_ms, [this, low, high, step]() {
        if (led_strip_ == nullptr) return;
        if (breathe_up_) {
            breathe_color_.red   = (breathe_color_.red + step < high.red) ? breathe_color_.red + step : high.red;
            breathe_color_.green = (breathe_color_.green + step < high.green) ? breathe_color_.green + step : high.green;
            breathe_color_.blue  = (breathe_color_.blue + step < high.blue) ? breathe_color_.blue + step : high.blue;
            if (breathe_color_.red >= high.red && breathe_color_.green >= high.green && breathe_color_.blue >= high.blue)
                breathe_up_ = false;
        } else {
            breathe_color_.red   = (breathe_color_.red > low.red + step) ? breathe_color_.red - step : low.red;
            breathe_color_.green = (breathe_color_.green > low.green + step) ? breathe_color_.green - step : low.green;
            breathe_color_.blue  = (breathe_color_.blue > low.blue + step) ? breathe_color_.blue - step : low.blue;
            if (breathe_color_.red <= low.red && breathe_color_.green <= low.green && breathe_color_.blue <= low.blue)
                breathe_up_ = true;
        }
        for (int i = 0; i < max_leds_ && i < static_cast<int>(colors_.size()); i++)
            led_strip_set_pixel(led_strip_, i, breathe_color_.red, breathe_color_.green, breathe_color_.blue);
        led_strip_refresh(led_strip_);
    });
}

void CircularStrip::StartBreathe(int interval_ms) {
    StripColor target = { default_brightness_, default_brightness_, default_brightness_ };
    if (!colors_.empty() && (colors_[0].red != 0 || colors_[0].green != 0 || colors_[0].blue != 0)) {
        target = colors_[0];
    }
    Breathe({0, 0, 0}, target, interval_ms);
}

void CircularStrip::StartBlink(int interval_ms) {
    StripColor target = { default_brightness_, default_brightness_, default_brightness_ };
    if (!colors_.empty() && (colors_[0].red != 0 || colors_[0].green != 0 || colors_[0].blue != 0)) {
        target = colors_[0];
    }
    Blink(target, interval_ms);
}

void CircularStrip::Scroll(StripColor low, StripColor high, int length, int interval_ms) {
    if (led_strip_ == nullptr) return;
    for (int i = 0; i < max_leds_ && i < static_cast<int>(colors_.size()); i++) {
        colors_[i] = low;
    }
    scroll_offset_ = 0;
    StartStripTask(interval_ms, [this, low, high, length]() {
        if (led_strip_ == nullptr) return;
        int offset = scroll_offset_;
        for (int i = 0; i < max_leds_ && i < static_cast<int>(colors_.size()); i++) {
            colors_[i] = low;
        }
        int effective_len = length;
        if (effective_len >= max_leds_) {
            effective_len = std::max(1, max_leds_ / 2);
        }
        for (int j = 0; j < effective_len; j++) {
            int i = (offset + j) % max_leds_;
            if (i < static_cast<int>(colors_.size())) {
                colors_[i] = high;
            }
        }
        for (int i = 0; i < max_leds_ && i < static_cast<int>(colors_.size()); i++) {
            led_strip_set_pixel(led_strip_, i, colors_[i].red, colors_[i].green, colors_[i].blue);
        }
        led_strip_refresh(led_strip_);
        scroll_offset_ = (offset + 1) % max_leds_;
    });
}

void CircularStrip::Scanner(StripColor color, int length, int interval_ms) {
    if (led_strip_ == nullptr) return;
    scanner_pos_ = 0;
    scanner_forward_ = true;
    StartStripTask(interval_ms, [this, color, length]() {
        if (led_strip_ == nullptr) return;
        // Smooth decaying tail
        for (int i = 0; i < max_leds_ && i < static_cast<int>(colors_.size()); i++) {
            colors_[i].red   /= 2;
            colors_[i].green /= 2;
            colors_[i].blue  /= 2;
        }

        int effective_len = length;
        if (effective_len >= max_leds_) {
            effective_len = std::max(1, max_leds_ / 4);
        }

        for (int j = 0; j < effective_len; j++) {
            int idx = scanner_pos_ + (scanner_forward_ ? j : -j);
            if (idx >= 0 && idx < max_leds_ && idx < static_cast<int>(colors_.size())) {
                colors_[idx] = color;
            }
        }

        for (int i = 0; i < max_leds_ && i < static_cast<int>(colors_.size()); i++) {
            led_strip_set_pixel(led_strip_, i, colors_[i].red, colors_[i].green, colors_[i].blue);
        }
        led_strip_refresh(led_strip_);

        if (scanner_forward_) {
            scanner_pos_++;
            if (scanner_pos_ >= max_leds_ - effective_len) {
                scanner_forward_ = false;
            }
        } else {
            scanner_pos_--;
            if (scanner_pos_ <= 0) {
                scanner_forward_ = true;
            }
        }
    });
}

void CircularStrip::ColorWipe(StripColor color, int interval_ms) {
    if (led_strip_ == nullptr) return;
    wipe_index_ = 0;
    wipe_clear_ = false;
    StartStripTask(interval_ms, [this, color]() {
        if (led_strip_ == nullptr) return;
        if (!wipe_clear_) {
            if (wipe_index_ < max_leds_ && wipe_index_ < static_cast<int>(colors_.size())) {
                colors_[wipe_index_] = color;
                led_strip_set_pixel(led_strip_, wipe_index_, color.red, color.green, color.blue);
                led_strip_refresh(led_strip_);
                wipe_index_++;
            } else {
                wipe_clear_ = true;
                wipe_index_ = 0;
            }
        } else {
            if (wipe_index_ < max_leds_ && wipe_index_ < static_cast<int>(colors_.size())) {
                colors_[wipe_index_] = {0, 0, 0};
                led_strip_set_pixel(led_strip_, wipe_index_, 0, 0, 0);
                led_strip_refresh(led_strip_);
                wipe_index_++;
            } else {
                wipe_clear_ = false;
                wipe_index_ = 0;
            }
        }
    });
}

void CircularStrip::StartScanner(int interval_ms) {
    StripColor target = { default_brightness_, default_brightness_, default_brightness_ };
    if (!colors_.empty() && (colors_[0].red != 0 || colors_[0].green != 0 || colors_[0].blue != 0)) {
        target = colors_[0];
    } else {
        target = { default_brightness_, 0, 0 };
    }
    int len = std::max(1, max_leds_ / 6);
    Scanner(target, len, interval_ms);
}

void CircularStrip::StartColorWipe(int interval_ms) {
    StripColor target = { default_brightness_, default_brightness_, default_brightness_ };
    if (!colors_.empty() && (colors_[0].red != 0 || colors_[0].green != 0 || colors_[0].blue != 0)) {
        target = colors_[0];
    }
    ColorWipe(target, interval_ms);
}

void CircularStrip::Rainbow(int interval_ms) {
    if (led_strip_ == nullptr) return;
    rainbow_offset_ = 0;
    StartStripTask(interval_ms, [this]() {
        if (led_strip_ == nullptr) return;
        rainbow_offset_ += 4;
        StripColor c = Wheel(rainbow_offset_, default_brightness_);
        for (int i = 0; i < max_leds_ && i < static_cast<int>(colors_.size()); i++) {
            colors_[i] = c;
            led_strip_set_pixel(led_strip_, i, c.red, c.green, c.blue);
        }
        led_strip_refresh(led_strip_);
    });
}

void CircularStrip::RainbowChase(int interval_ms) {
    if (led_strip_ == nullptr) return;
    rainbow_offset_ = 0;
    StartStripTask(interval_ms, [this]() {
        if (led_strip_ == nullptr) return;
        rainbow_offset_ += 4;
        // For strips longer than 64 LEDs, repeat rainbow cycles every 48 LEDs
        int cycle_len = (max_leds_ > 64) ? 48 : (max_leds_ > 0 ? max_leds_ : 1);
        for (int i = 0; i < max_leds_ && i < static_cast<int>(colors_.size()); i++) {
            uint8_t pixel_hue = rainbow_offset_ + ((i % cycle_len) * 256 / cycle_len);
            StripColor c = Wheel(pixel_hue, default_brightness_);
            colors_[i] = c;
            led_strip_set_pixel(led_strip_, i, c.red, c.green, c.blue);
        }
        led_strip_refresh(led_strip_);
    });
}

void CircularStrip::Rainbow(StripColor low, StripColor high, int interval_ms) {
    RainbowChase(interval_ms);
}

void CircularStrip::StartStripTask(int interval_ms, std::function<void()> cb) {
    if (led_strip_ == nullptr || strip_timer_ == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    esp_timer_stop(strip_timer_);
    
    strip_callback_ = cb;
    esp_timer_start_periodic(strip_timer_, interval_ms * 1000);
}

void CircularStrip::SetBrightness(uint8_t default_brightness, uint8_t low_brightness) {
    default_brightness_ = default_brightness;
    low_brightness_ = low_brightness;
    if (!custom_mode_) {
        OnStateChanged();
    } else {
        std::lock_guard<std::mutex> lock(mutex_);
        if (led_strip_ != nullptr) {
            for (int i = 0; i < max_leds_ && i < static_cast<int>(colors_.size()); i++) {
                uint8_t r = (uint16_t)colors_[i].red * default_brightness_ / 255;
                uint8_t g = (uint16_t)colors_[i].green * default_brightness_ / 255;
                uint8_t b = (uint16_t)colors_[i].blue * default_brightness_ / 255;
                led_strip_set_pixel(led_strip_, i, r, g, b);
            }
            led_strip_refresh(led_strip_);
        }
    }
}

void CircularStrip::OnStateChanged() {
    if (custom_mode_) {
        return;
    }
    auto& app = Application::GetInstance();
    auto device_state = app.GetDeviceState();
    switch (device_state) {
        case kDeviceStateStarting: {
#if defined(CONFIG_CUSTOM_LED_FAST_RAINBOW_EFFECT)
            RainbowChase(20);
#else
            StripColor low = { 0, 0, 0 };
            StripColor high = { low_brightness_, low_brightness_, default_brightness_ };
            Scroll(low, high, 3, 100);
#endif
            break;
        }
        case kDeviceStateWifiConfiguring: {
            StripColor color = { low_brightness_, low_brightness_, default_brightness_ };
            Blink(color, 500);
            break;
        }
        case kDeviceStateIdle:
            FadeOut(50);
            break;
        case kDeviceStateConnecting: {
            StripColor color = { low_brightness_, low_brightness_, default_brightness_ };
            SetAllColor(color);
            break;
        }
        case kDeviceStateListening:
        case kDeviceStateAudioTesting: {
            StripColor color = { default_brightness_, low_brightness_, low_brightness_ };
            SetAllColor(color);
            break;
        }
        case kDeviceStateSpeaking: {
#if defined(CONFIG_CUSTOM_LED_FAST_RAINBOW_EFFECT)
            if (max_leds_ > 1) {
                RainbowChase(25);
            } else {
                Rainbow(25);
            }
#else
            StripColor color = { low_brightness_, default_brightness_, low_brightness_ };
            SetAllColor(color);
#endif
            break;
        }
        case kDeviceStateNotifying: {
            StripColor color = { low_brightness_, default_brightness_, low_brightness_ };
            SetAllColor(color);
            break;
        }
        case kDeviceStateUpgrading: {
            StripColor color = { low_brightness_, default_brightness_, low_brightness_ };
            Blink(color, 100);
            break;
        }
        case kDeviceStateActivating: {
            StripColor color = { low_brightness_, default_brightness_, low_brightness_ };
            Blink(color, 500);
            break;
        }
        case kDeviceStateFatalError: {
            StripColor color = { default_brightness_, 0, 0 };
            Blink(color, 100);
            break;
        }
        default:
            ESP_LOGW(TAG, "Unknown device state for led strip: %d", device_state);
            return;
    }
}
