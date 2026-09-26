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
#include <string>

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
    void SetBrightness(uint8_t brightness) override { SetBrightness(brightness, brightness / 8); }
    uint8_t GetBrightness() const override { return default_brightness_; }
    void SetAllColor(StripColor color);
    void SetSingleColor(uint16_t index, StripColor color);
    void SetMultiColors(const std::vector<StripColor>& colors);
    void SetColor(uint8_t r, uint8_t g, uint8_t b) override { SetAllColor({r, g, b}); }
    void SetPixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b) override { SetSingleColor(index, {r, g, b}); }
    uint16_t GetLedCount() const override { return static_cast<uint16_t>(max_leds_); }
    void GetColor(uint8_t& r, uint8_t& g, uint8_t& b) const override {
        if (!colors_.empty()) {
            r = colors_[0].red; g = colors_[0].green; b = colors_[0].blue;
        } else {
            r = g = b = 0;
        }
    }
    void TurnOn() override;
    void TurnOff() override;
    void Blink(StripColor color, int interval_ms);
    void Breathe(StripColor low, StripColor high, int interval_ms);
    void Scroll(StripColor low, StripColor high, int length, int interval_ms);
    void Scanner(StripColor color, int length, int interval_ms);
    void ColorWipe(StripColor color, int interval_ms);
    void Rainbow(int interval_ms = 25);
    void RainbowChase(int interval_ms = 30);
    void Rainbow(StripColor low, StripColor high, int interval_ms);
    void FadeOut(int interval_ms);

    void StartRainbow(int interval_ms = 25) override { Rainbow(interval_ms); }
    void StartChase(int interval_ms = 30) override { RainbowChase(interval_ms); }
    void StartBreathe(int interval_ms = 25) override;
    void StartBlink(int interval_ms = 200) override;
    void StartScanner(int interval_ms = 30) override;
    void StartColorWipe(int interval_ms = 30) override;
    void SetCustomMode(bool custom) override { custom_mode_ = custom; }
    bool IsCustomMode() const override { return custom_mode_; }
    std::string GetType() const override { return "CircularStrip"; }

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
    bool custom_mode_ = false;

    // Per-effect state
    bool       blink_on_        = true;
    bool       breathe_up_      = true;
    StripColor breathe_color_   = {};
    int        scroll_offset_   = 0;
    uint8_t    rainbow_offset_  = 0;
    int        scanner_pos_     = 0;
    bool       scanner_forward_ = true;
    int        wipe_index_      = 0;
    bool       wipe_clear_      = false;

    void StartStripTask(int interval_ms, std::function<void()> cb);
};

#endif // _CIRCULAR_STRIP_H_
