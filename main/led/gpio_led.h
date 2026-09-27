#ifndef _GPIO_LED_H_
#define _GPIO_LED_H_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "led.h"
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_timer.h>
#include <atomic>
#include <mutex>
#include <string>

class GpioLed : public Led {
 public:
    GpioLed(gpio_num_t gpio);
    GpioLed(gpio_num_t gpio, int output_invert);
    GpioLed(gpio_num_t gpio, int output_invert, ledc_timer_t timer_num, ledc_channel_t channel);
    virtual ~GpioLed();

    void OnStateChanged() override;
    void TurnOn() override;
    void TurnOff() override;
    void SetBrightness(uint8_t brightness) override;
    uint8_t GetBrightness() const override { return brightness_; }
    void SetColor(uint8_t r, uint8_t g, uint8_t b) override {
        uint8_t br = static_cast<uint8_t>((static_cast<uint32_t>(r) * 299 + static_cast<uint32_t>(g) * 587 + static_cast<uint32_t>(b) * 114) / 1000);
        SetBrightness(br);
    }
    void GetColor(uint8_t& r, uint8_t& g, uint8_t& b) const override {
        r = g = b = brightness_;
    }
    void StartBlink(int interval_ms = 200) override { StartContinuousBlink(interval_ms); }
    void StartBreathe(int interval_ms = 25) override { StartFadeTask(); }
    void StartScanner(int interval_ms = 30) override { StartFadeTask(); }
    void StartColorWipe(int interval_ms = 30) override { StartContinuousBlink(interval_ms); }
    void SetPixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b) override {
        if (index == 0) {
            SetColor(r, g, b);
            TurnOn();
        }
    }
    uint16_t GetLedCount() const override { return 1; }
    void SetCustomMode(bool custom) override { custom_mode_ = custom; }
    bool IsCustomMode() const override { return custom_mode_; }
    std::string GetType() const override { return "GpioLed"; }

 private:
    std::mutex mutex_;
    TaskHandle_t blink_task_ = nullptr;
    ledc_channel_config_t ledc_channel_ = {0};
    bool ledc_initialized_ = false;
    uint32_t duty_ = 0;
    uint8_t brightness_ = 0;
    bool is_on_ = false;
    int blink_counter_ = 0;
    int blink_interval_ms_ = 0;
    esp_timer_handle_t blink_timer_ = nullptr;
    bool fade_up_ = true;
    TaskHandle_t event_task_handle_ = nullptr;
    bool custom_mode_ = false;
    
    static void EventTask(void* arg);
    void StartBlinkTask(int times, int interval_ms);
    void OnBlinkTimer();

    void BlinkOnce();
    void Blink(int times, int interval_ms);
    void StartContinuousBlink(int interval_ms);
    void StartFadeTask();
    void OnFadeEnd();
    static bool IRAM_ATTR FadeCallback(const ledc_cb_param_t *param, void *user_arg);
};

#endif  // _GPIO_LED_H_
