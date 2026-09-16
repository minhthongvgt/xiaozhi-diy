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
    void SetColor(uint8_t r, uint8_t g, uint8_t b) override {
        uint8_t br = (uint8_t)(((uint32_t)r * 299 + (uint32_t)g * 587 + (uint32_t)b * 114) / 1000);
        SetBrightness(br);
    }
    void StartRainbow(int interval_ms = 25) override { TurnOn(); }
    void StartChase(int interval_ms = 15) override { StartContinuousBlink(interval_ms); }
    void StartBreathe(int interval_ms = 30) override { StartFadeTask(); }
    void StartBlink(int interval_ms = 200) override { StartContinuousBlink(interval_ms); }
    std::string GetType() const override { return "GpioLed"; }
    void GetColor(uint8_t& r, uint8_t& g, uint8_t& b) const override {
        r = (uint8_t)duty_; g = (uint8_t)duty_; b = (uint8_t)duty_;
    }
    uint8_t GetBrightness() const override { return (uint8_t)duty_; }

 private:
    std::mutex mutex_;
    TaskHandle_t blink_task_ = nullptr;
    ledc_channel_config_t ledc_channel_ = {0};
    bool ledc_initialized_ = false;
    uint32_t duty_ = 0;
    int blink_counter_ = 0;
    int blink_interval_ms_ = 0;
    esp_timer_handle_t blink_timer_ = nullptr;
    bool fade_up_ = true;
    TaskHandle_t event_task_handle_;
    
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
