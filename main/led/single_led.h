#ifndef _SINGLE_LED_H_
#define _SINGLE_LED_H_

#include "led.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <led_strip.h>
#include <esp_timer.h>
#include <atomic>
#include <mutex>
#include <string>

class SingleLed : public Led {
public:
    SingleLed(gpio_num_t gpio);
    virtual ~SingleLed();

    void OnStateChanged() override;

    void TurnOn() override;
    void TurnOff() override;
    void SetColor(uint8_t r, uint8_t g, uint8_t b) override;
    void GetColor(uint8_t& r, uint8_t& g, uint8_t& b) const override { r = r_; g = g_; b = b_; }
    void SetBrightness(uint8_t brightness) override;
    uint8_t GetBrightness() const override { return rainbow_brightness_; }
    void StartRainbow(int interval_ms = 25) override { StartRainbow(interval_ms, rainbow_brightness_); }
    void StartRainbow(int interval_ms, uint8_t brightness);
    void StartChase(int interval_ms = 30) override { StartRainbow(interval_ms, rainbow_brightness_); }
    void StartBreathe(int interval_ms = 25) override;
    void StartBlink(int interval_ms = 200) override { StartContinuousBlink(interval_ms); }
    void StartScanner(int interval_ms = 30) override { StartContinuousBlink(interval_ms); }
    void StartColorWipe(int interval_ms = 30) override { StartRainbow(interval_ms, rainbow_brightness_); }
    void SetPixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b) override {
        if (index == 0) {
            SetColor(r, g, b);
            TurnOn();
        }
    }
    uint16_t GetLedCount() const override { return 1; }
    void SetCustomMode(bool custom) override { custom_mode_ = custom; }
    bool IsCustomMode() const override { return custom_mode_; }
    std::string GetType() const override { return "SingleLed"; }

    void BlinkOnce();
    void Blink(int times, int interval_ms);
    void StartContinuousBlink(int interval_ms);
    void StopEffect();

private:
    enum class EffectMode {
        kNone,
        kBlink,
        kRainbow,
        kBreathe
    };

    std::mutex mutex_;
    led_strip_handle_t led_strip_ = nullptr;
    uint8_t r_ = 0, g_ = 0, b_ = 0;
    int blink_counter_ = 0;
    int blink_interval_ms_ = 0;
    esp_timer_handle_t timer_ = nullptr;

    EffectMode mode_ = EffectMode::kNone;
    uint8_t rainbow_pos_ = 0;
    uint8_t rainbow_brightness_ = 255;
    bool custom_mode_ = false;
    bool breathe_up_ = true;
    uint8_t breathe_val_ = 0;

    void StartBlinkTask(int times, int interval_ms);
    void OnTimer();
};

#endif // _SINGLE_LED_H_
