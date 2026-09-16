#ifndef _CIRCULAR_STRIP_H_
#define _CIRCULAR_STRIP_H_

#include "led.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <led_strip.h>
#include <esp_timer.h>
#include <atomic>
#include <mutex>
#include <vector>
#include <functional>

#define DEFAULT_BRIGHTNESS 32
#define LOW_BRIGHTNESS 4

struct StripColor {
    uint8_t red = 0, green = 0, blue = 0;
};

class CircularStrip : public Led {
public:
    CircularStrip(gpio_num_t gpio, uint16_t max_leds);
    virtual ~CircularStrip();

    void OnStateChanged() override;
    void SetBrightness(uint8_t default_brightness, uint8_t low_brightness);
    void SetAllColor(StripColor color);
    void SetSingleColor(uint8_t index, StripColor color);
    void SetMultiColors(const std::vector<StripColor>& colors);
    void Blink(StripColor color, int interval_ms);
    void Breathe(StripColor low, StripColor high, int interval_ms);
    void Scroll(StripColor low, StripColor high, int length, int interval_ms);
    void Rainbow(int interval_ms = 25);
    void RainbowChase(int interval_ms = 30);
    void Rainbow(StripColor low, StripColor high, int interval_ms);
    void FadeOut(int interval_ms);
    void TurnOff() override;
    StripColor GetColor(uint8_t index = 0) const;

    void TurnOn() override {
        StripColor c = GetColor();
        if (c.red == 0 && c.green == 0 && c.blue == 0) {
            c = { default_brightness_, default_brightness_, default_brightness_ };
        }
        SetAllColor(c);
    }
    void SetColor(uint8_t r, uint8_t g, uint8_t b) override { SetAllColor({r, g, b}); }
    void SetBrightness(uint8_t brightness) override { SetBrightness(brightness, brightness > 4 ? 4 : brightness / 2); }
    void StartRainbow(int interval_ms = 25) override { Rainbow(interval_ms); }
    void StartChase(int interval_ms = 15) override { RainbowChase(interval_ms); }
    void StartBreathe(int interval_ms = 30) override {
        StripColor c = GetColor();
        if (c.red == 0 && c.green == 0 && c.blue == 0) {
            c = { default_brightness_, default_brightness_, default_brightness_ };
        }
        Breathe({0, 0, 0}, c, interval_ms);
    }
    void StartBlink(int interval_ms = 200) override {
        StripColor c = GetColor();
        if (c.red == 0 && c.green == 0 && c.blue == 0) {
            c = { default_brightness_, default_brightness_, default_brightness_ };
        }
        Blink(c, interval_ms);
    }
    std::string GetType() const override { return "CircularStrip"; }
    void GetColor(uint8_t& r, uint8_t& g, uint8_t& b) const override {
        StripColor c = GetColor();
        r = c.red; g = c.green; b = c.blue;
    }
    uint8_t GetBrightness() const override { return default_brightness_; }

private:
    std::mutex mutex_;
    led_strip_handle_t led_strip_ = nullptr;
    int max_leds_ = 0;
    std::vector<StripColor> colors_;
    int blink_counter_ = 0;
    int blink_interval_ms_ = 0;
    esp_timer_handle_t strip_timer_ = nullptr;
    std::function<void()> strip_callback_ = nullptr;

    uint8_t default_brightness_ = DEFAULT_BRIGHTNESS;
    uint8_t low_brightness_ = LOW_BRIGHTNESS;

    // Per-effect state
    bool       blink_on_        = true;
    bool       breathe_up_      = true;
    StripColor breathe_color_   = {};
    int        scroll_offset_   = 0;
    uint8_t    rainbow_offset_  = 0;

    void StartStripTask(int interval_ms, std::function<void()> cb);
};

#endif // _CIRCULAR_STRIP_H_
