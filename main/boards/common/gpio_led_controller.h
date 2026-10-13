#ifndef GPIO_LED_CONTROLLER_H
#define GPIO_LED_CONTROLLER_H
#include "mcp_server.h"
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class GpioUserLed {
public:
    enum class Effect { kSteady, kBlink, kBreathe };

    GpioUserLed(gpio_num_t gpio, int ledc_channel) : gpio_(gpio), ledc_channel_(ledc_channel) {
        if (ledc_channel_ >= 0 && !ConfigurePwm()) {
            ledc_channel_ = -1;
        }
        if (ledc_channel_ < 0) {
            ConfigureDigital();
        }
        const esp_timer_create_args_t timer_args = {
            .callback = [](void* arg) { static_cast<GpioUserLed*>(arg)->OnTick(); },
            .arg = this,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "gpio_led",
            .skip_unhandled_events = true,
        };
        if (esp_timer_create(&timer_args, &timer_) != ESP_OK) {
            timer_ = nullptr;
        }
        Output(0);
    }

    gpio_num_t gpio() const { return gpio_; }
    bool dimmable() const { return ledc_channel_ >= 0; }

    void SetPower(bool on) {
        std::lock_guard<std::mutex> lock(mutex_);
        power_ = on;
        RestartLocked();
    }

    void SetBrightness(int percent) {
        std::lock_guard<std::mutex> lock(mutex_);
        brightness_ = ClampPercent(percent);
        power_ = brightness_ > 0;
        RestartLocked();
    }

    void SetEffect(Effect effect, int period_ms) {
        std::lock_guard<std::mutex> lock(mutex_);
        effect_ = (effect == Effect::kBreathe && !dimmable()) ? Effect::kBlink : effect;
        period_ms_ = period_ms;
        power_ = true;
        if (brightness_ == 0) {
            brightness_ = kDefaultBrightness;
        }
        RestartLocked();
    }

    nlohmann::json GetState() {
        std::lock_guard<std::mutex> lock(mutex_);
        nlohmann::json j;
        j["gpio"] = static_cast<int>(gpio_);
        j["power"] = power_;
        j["brightness"] = brightness_;
        j["effect"] = EffectName(effect_);
        j["period_ms"] = period_ms_;
        j["dimmable"] = dimmable();
        return j;
    }

    static const char* EffectName(Effect effect) {
        switch (effect) {
            case Effect::kBlink: return "blink";
            case Effect::kBreathe: return "breathe";
            default: return "steady";
        }
    }

    static int ClampPercent(int value) { return value < 0 ? 0 : (value > 100 ? 100 : value); }

private:
    static constexpr const char* kTag = "GpioUserLed";
    static constexpr ledc_timer_t kLedcTimer = LEDC_TIMER_1;
    static constexpr ledc_timer_bit_t kLedcResolution = LEDC_TIMER_13_BIT;
    static constexpr uint32_t kLedcMaxDuty = (1U << 13) - 1;
    static constexpr uint32_t kLedcFrequencyHz = 4000;
    static constexpr int kBreatheStepMs = 20;
    static constexpr int kDefaultBrightness = 100;

    gpio_num_t gpio_;
    int ledc_channel_;
    esp_timer_handle_t timer_ = nullptr;
    std::mutex mutex_;
    bool power_ = false;
    int brightness_ = kDefaultBrightness;
    Effect effect_ = Effect::kSteady;
    int period_ms_ = 1000;
    int64_t effect_start_us_ = 0;

    bool ConfigurePwm() {
        const ledc_timer_config_t timer_config = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = kLedcResolution,
            .timer_num = kLedcTimer,
            .freq_hz = kLedcFrequencyHz,
            .clk_cfg = LEDC_AUTO_CLK,
            .deconfigure = false,
        };
        esp_err_t err = ledc_timer_config(&timer_config);
        if (err != ESP_OK) {
            ESP_LOGW(kTag, "LEDC timer config failed for GPIO %d: %s", static_cast<int>(gpio_), esp_err_to_name(err));
            return false;
        }
        const ledc_channel_config_t channel_config = {
            .gpio_num = static_cast<int>(gpio_),
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = static_cast<ledc_channel_t>(ledc_channel_),
            .timer_sel = kLedcTimer,
            .duty = 0,
            .hpoint = 0,
            .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
            .flags = {.output_invert = 0},
            .deconfigure = false,
        };
        err = ledc_channel_config(&channel_config);
        if (err != ESP_OK) {
            ESP_LOGW(kTag, "LEDC channel %d config failed for GPIO %d: %s", ledc_channel_, static_cast<int>(gpio_),
                     esp_err_to_name(err));
            return false;
        }
        return true;
    }

    void ConfigureDigital() {
        const gpio_config_t config = {
            .pin_bit_mask = (1ULL << gpio_),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        esp_err_t err = gpio_config(&config);
        if (err != ESP_OK) {
            ESP_LOGW(kTag, "GPIO %d config failed: %s", static_cast<int>(gpio_), esp_err_to_name(err));
        }
    }

    void Output(int percent) {
        if (dimmable()) {
            const uint32_t duty = static_cast<uint32_t>(percent) * kLedcMaxDuty / 100;
            const ledc_channel_t channel = static_cast<ledc_channel_t>(ledc_channel_);
            ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
        } else {
            gpio_set_level(gpio_, percent > 0 ? 1 : 0);
        }
    }

    void RestartLocked() {
        if (timer_ != nullptr) {
            esp_timer_stop(timer_);
        }
        if (!power_) {
            Output(0);
            return;
        }
        if (effect_ == Effect::kSteady || timer_ == nullptr) {
            Output(brightness_);
            return;
        }
        effect_start_us_ = esp_timer_get_time();
        const int step_ms = effect_ == Effect::kBreathe ? kBreatheStepMs : period_ms_ / 2;
        Output(effect_ == Effect::kBlink ? brightness_ : 0);
        esp_timer_start_periodic(timer_, static_cast<uint64_t>(step_ms) * 1000);
    }

    void OnTick() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!power_ || effect_ == Effect::kSteady) {
            return;
        }
        const int64_t elapsed_ms = (esp_timer_get_time() - effect_start_us_) / 1000;
        const int phase_ms = static_cast<int>(elapsed_ms % period_ms_);
        if (effect_ == Effect::kBlink) {
            Output(phase_ms < period_ms_ / 2 ? brightness_ : 0);
            return;
        }
        const int half = period_ms_ / 2;
        const int ramp = phase_ms < half ? phase_ms : period_ms_ - phase_ms;
        const int level = ramp * ramp * brightness_ / (half * half);
        Output(level);
    }
};

class GpioLedController {
public:
    GpioLedController(const std::vector<gpio_num_t>& pins, const std::vector<int>& ledc_channels) {
        for (size_t i = 0; i < pins.size(); i++) {
            const int channel = i < ledc_channels.size() ? ledc_channels[i] : -1;
            leds_.push_back(std::make_unique<GpioUserLed>(pins[i], channel));
            ESP_LOGI(kTag, "LED %d on GPIO %d (%s)", static_cast<int>(i + 1), static_cast<int>(pins[i]),
                     leds_.back()->dimmable() ? "PWM" : "on/off");
        }
        RegisterTools();
    }

private:
    static constexpr const char* kTag = "GpioLedController";
    std::vector<std::unique_ptr<GpioUserLed>> leds_;

    std::string Describe() const {
        std::string text;
        for (size_t i = 0; i < leds_.size(); i++) {
            text += (i ? ", " : "") + std::string("LED ") + std::to_string(i + 1) + " = GPIO " +
                    std::to_string(static_cast<int>(leds_[i]->gpio())) +
                    (leds_[i]->dimmable() ? " (dimmable)" : " (on/off only)");
        }
        return text;
    }

    template <typename Action>
    ReturnValue ForEach(int index, Action action) {
        if (index < 0 || index > static_cast<int>(leds_.size())) {
            nlohmann::json j;
            j["error"] = "LED index out of range";
            return j;
        }
        for (size_t i = 0; i < leds_.size(); i++) {
            if (index == 0 || static_cast<size_t>(index) == i + 1) {
                action(*leds_[i]);
            }
        }
        return true;
    }

    void RegisterTools() {
        auto& mcp_server = McpServer::GetInstance();
        const int count = static_cast<int>(leds_.size());
        const std::string index_hint = " index: 1.." + std::to_string(count) + " selects one LED, 0 selects all. " + Describe() + ".";

        mcp_server.AddTool("self.gpio_led.get_state", "Get power, brightness and effect of the single-color LEDs. " + Describe() + ".",
                           PropertyList(), [this](const PropertyList&) -> ReturnValue {
            nlohmann::json list = nlohmann::json::array();
            for (size_t i = 0; i < leds_.size(); i++) {
                nlohmann::json item = leds_[i]->GetState();
                item["index"] = static_cast<int>(i + 1);
                list.push_back(item);
            }
            return list;
        });

        mcp_server.AddTool("self.gpio_led.turn_on", "Turn on single-color LEDs." + index_hint,
                           PropertyList({Property("index", kPropertyTypeInteger, 0, 0, count)}),
                           [this](const PropertyList& properties) -> ReturnValue {
            return ForEach(properties["index"].value<int>(), [](GpioUserLed& led) { led.SetPower(true); });
        });

        mcp_server.AddTool("self.gpio_led.turn_off", "Turn off single-color LEDs." + index_hint,
                           PropertyList({Property("index", kPropertyTypeInteger, 0, 0, count)}),
                           [this](const PropertyList& properties) -> ReturnValue {
            return ForEach(properties["index"].value<int>(), [](GpioUserLed& led) { led.SetPower(false); });
        });

        mcp_server.AddTool("self.gpio_led.set_brightness",
                           "Set brightness 0-100 percent of single-color LEDs; 0 turns off. On/off-only LEDs light at any value above 0." + index_hint,
                           PropertyList({Property("index", kPropertyTypeInteger, 0, 0, count),
                                         Property("brightness", kPropertyTypeInteger, 0, 100)}),
                           [this](const PropertyList& properties) -> ReturnValue {
            const int brightness = properties["brightness"].value<int>();
            return ForEach(properties["index"].value<int>(), [brightness](GpioUserLed& led) { led.SetBrightness(brightness); });
        });

        mcp_server.AddTool("self.gpio_led.set_effect",
                           "Set effect of single-color LEDs: 'steady', 'blink' or 'breathe' (breathe falls back to blink on on/off-only LEDs). "
                           "period_ms is the length of one cycle." + index_hint,
                           PropertyList({Property("index", kPropertyTypeInteger, 0, 0, count),
                                         Property("effect", kPropertyTypeString),
                                         Property("period_ms", kPropertyTypeInteger, 1000, 200, 10000)}),
                           [this](const PropertyList& properties) -> ReturnValue {
            const std::string name = properties["effect"].value<std::string>();
            GpioUserLed::Effect effect;
            if (name == "steady") {
                effect = GpioUserLed::Effect::kSteady;
            } else if (name == "blink") {
                effect = GpioUserLed::Effect::kBlink;
            } else if (name == "breathe") {
                effect = GpioUserLed::Effect::kBreathe;
            } else {
                nlohmann::json j;
                j["error"] = "Unknown effect, use steady, blink or breathe";
                return j;
            }
            const int period_ms = properties["period_ms"].value<int>();
            return ForEach(properties["index"].value<int>(), [effect, period_ms](GpioUserLed& led) { led.SetEffect(effect, period_ms); });
        });
    }
};
#endif
