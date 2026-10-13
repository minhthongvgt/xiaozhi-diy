#ifndef USER_LED_GROUP_H
#define USER_LED_GROUP_H
#include "mcp_server.h"
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

class UserLedOutput {
public:
    virtual ~UserLedOutput() = default;
    virtual void Write(int percent) = 0;
    virtual bool dimmable() const = 0;
    virtual std::string label() const = 0;
    virtual int breathe_step_ms() const { return 20; }
};

class GpioLedOutput : public UserLedOutput {
public:
    GpioLedOutput(gpio_num_t gpio, int ledc_channel) : gpio_(gpio), ledc_channel_(ledc_channel) {
        if (ledc_channel_ >= 0 && !ConfigurePwm()) {
            ledc_channel_ = -1;
        }
        if (ledc_channel_ < 0) {
            ConfigureDigital();
        }
    }

    void Write(int percent) override {
        if (dimmable()) {
            const uint32_t duty = static_cast<uint32_t>(percent) * kLedcMaxDuty / 100;
            const ledc_channel_t channel = static_cast<ledc_channel_t>(ledc_channel_);
            ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
        } else {
            gpio_set_level(gpio_, percent > 0 ? 1 : 0);
        }
    }

    bool dimmable() const override { return ledc_channel_ >= 0; }
    std::string label() const override { return "GPIO " + std::to_string(static_cast<int>(gpio_)); }

private:
    static constexpr const char* kTag = "GpioLedOutput";
    static constexpr ledc_timer_t kLedcTimer = LEDC_TIMER_1;
    static constexpr uint32_t kLedcMaxDuty = (1U << 13) - 1;
    gpio_num_t gpio_;
    int ledc_channel_;

    bool ConfigurePwm() {
        const ledc_timer_config_t timer_config = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_13_BIT,
            .timer_num = kLedcTimer,
            .freq_hz = 4000,
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
};

enum class UserLedEffect : int { kOff = 0, kSteady = 1, kBlink = 2, kBreathe = 3 };

typedef struct UserLedDefaults {
    int brightness;
    UserLedEffect boot_effect;
    int period_ms;
} UserLedDefaults;

class UserLed {
public:
    UserLed(std::unique_ptr<UserLedOutput> output, const UserLedDefaults& defaults)
        : output_(std::move(output)), brightness_(ClampPercent(defaults.brightness)), period_ms_(ClampPeriod(defaults.period_ms)) {
        const esp_timer_create_args_t timer_args = {
            .callback = [](void* arg) { static_cast<UserLed*>(arg)->OnTick(); },
            .arg = this,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "user_led",
            .skip_unhandled_events = true,
        };
        if (esp_timer_create(&timer_args, &timer_) != ESP_OK) {
            timer_ = nullptr;
        }
        if (brightness_ == 0) {
            brightness_ = 100;
        }
        if (defaults.boot_effect == UserLedEffect::kOff) {
            output_->Write(0);
        } else {
            SetEffect(defaults.boot_effect, period_ms_);
        }
    }

    const UserLedOutput& output() const { return *output_; }

    void SetPower(bool on) {
        std::lock_guard<std::mutex> lock(mutex_);
        power_ = on;
        if (on && effect_ == UserLedEffect::kOff) {
            effect_ = UserLedEffect::kSteady;
        }
        RestartLocked();
    }

    void SetBrightness(int percent) {
        std::lock_guard<std::mutex> lock(mutex_);
        const int value = ClampPercent(percent);
        power_ = value > 0;
        if (value > 0) {
            brightness_ = value;
            if (effect_ == UserLedEffect::kOff) {
                effect_ = UserLedEffect::kSteady;
            }
        }
        RestartLocked();
    }

    void SetEffect(UserLedEffect effect, int period_ms) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (effect == UserLedEffect::kOff) {
            power_ = false;
            RestartLocked();
            return;
        }
        effect_ = (effect == UserLedEffect::kBreathe && !output_->dimmable()) ? UserLedEffect::kBlink : effect;
        period_ms_ = ClampPeriod(period_ms);
        power_ = true;
        RestartLocked();
    }

    nlohmann::json GetState() {
        std::lock_guard<std::mutex> lock(mutex_);
        nlohmann::json j;
        j["output"] = output_->label();
        j["power"] = power_;
        j["brightness"] = brightness_;
        j["effect"] = EffectName(power_ ? effect_ : UserLedEffect::kOff);
        j["period_ms"] = period_ms_;
        j["dimmable"] = output_->dimmable();
        return j;
    }

    static const char* EffectName(UserLedEffect effect) {
        switch (effect) {
            case UserLedEffect::kSteady: return "steady";
            case UserLedEffect::kBlink: return "blink";
            case UserLedEffect::kBreathe: return "breathe";
            default: return "off";
        }
    }

    static bool ParseEffect(const std::string& name, UserLedEffect& effect) {
        if (name == "off") {
            effect = UserLedEffect::kOff;
        } else if (name == "steady") {
            effect = UserLedEffect::kSteady;
        } else if (name == "blink") {
            effect = UserLedEffect::kBlink;
        } else if (name == "breathe") {
            effect = UserLedEffect::kBreathe;
        } else {
            return false;
        }
        return true;
    }

    static int ClampPercent(int value) { return value < 0 ? 0 : (value > 100 ? 100 : value); }
    static int ClampPeriod(int value) { return value < 200 ? 200 : (value > 10000 ? 10000 : value); }

private:
    std::unique_ptr<UserLedOutput> output_;
    esp_timer_handle_t timer_ = nullptr;
    std::mutex mutex_;
    bool power_ = false;
    int brightness_;
    UserLedEffect effect_ = UserLedEffect::kSteady;
    int period_ms_;
    int64_t effect_start_us_ = 0;

    void RestartLocked() {
        if (timer_ != nullptr) {
            esp_timer_stop(timer_);
        }
        if (!power_) {
            output_->Write(0);
            return;
        }
        if (effect_ == UserLedEffect::kSteady || timer_ == nullptr) {
            output_->Write(brightness_);
            return;
        }
        effect_start_us_ = esp_timer_get_time();
        const int step_ms = effect_ == UserLedEffect::kBreathe ? output_->breathe_step_ms() : period_ms_ / 2;
        output_->Write(effect_ == UserLedEffect::kBlink ? brightness_ : 0);
        esp_timer_start_periodic(timer_, static_cast<uint64_t>(step_ms) * 1000);
    }

    void OnTick() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!power_ || effect_ == UserLedEffect::kSteady) {
            return;
        }
        const int64_t elapsed_ms = (esp_timer_get_time() - effect_start_us_) / 1000;
        const int phase_ms = static_cast<int>(elapsed_ms % period_ms_);
        const int half = period_ms_ / 2;
        if (effect_ == UserLedEffect::kBlink) {
            output_->Write(phase_ms < half ? brightness_ : 0);
            return;
        }
        const int ramp = phase_ms < half ? phase_ms : period_ms_ - phase_ms;
        output_->Write(ramp * ramp * brightness_ / (half * half));
    }
};

inline std::vector<int> ParseUserLedList(const char* text) {
    std::vector<int> values;
    if (text == nullptr) {
        return values;
    }
    std::stringstream stream(text);
    std::string token;
    while (std::getline(stream, token, ',')) {
        char* end = nullptr;
        const long value = std::strtol(token.c_str(), &end, 10);
        if (end != token.c_str() && value >= 0) {
            values.push_back(static_cast<int>(value));
        }
    }
    return values;
}

class UserLedGroup {
public:
    UserLedGroup(const std::string& tool_prefix, const std::string& title, std::vector<std::unique_ptr<UserLedOutput>> outputs,
                 const UserLedDefaults& defaults)
        : tool_prefix_(tool_prefix), title_(title) {
        for (auto& output : outputs) {
            ESP_LOGI(kTag, "%s: LED %d on %s (%s)", title_.c_str(), static_cast<int>(leds_.size() + 1), output->label().c_str(),
                     output->dimmable() ? "dimmable" : "on/off");
            leds_.push_back(std::make_unique<UserLed>(std::move(output), defaults));
        }
        if (!leds_.empty()) {
            RegisterTools(defaults.period_ms);
        }
    }

    size_t size() const { return leds_.size(); }

private:
    static constexpr const char* kTag = "UserLedGroup";
    std::string tool_prefix_;
    std::string title_;
    std::vector<std::unique_ptr<UserLed>> leds_;

    std::string Describe() const {
        std::string text = title_ + ": ";
        for (size_t i = 0; i < leds_.size(); i++) {
            text += (i ? ", " : "") + std::string("LED ") + std::to_string(i + 1) + " = " + leds_[i]->output().label() +
                    (leds_[i]->output().dimmable() ? "" : " (on/off only)");
        }
        return text + ".";
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

    void RegisterTools(int default_period_ms) {
        auto& mcp_server = McpServer::GetInstance();
        const int count = static_cast<int>(leds_.size());
        const std::string hint = " index 1-" + std::to_string(count) + " selects one LED, 0 selects all. " + Describe();
        const int period = UserLed::ClampPeriod(default_period_ms);

        mcp_server.AddTool(tool_prefix_ + ".get_state", "Get power, brightness and effect of each LED. " + Describe(), PropertyList(),
                           [this](const PropertyList&) -> ReturnValue {
            nlohmann::json list = nlohmann::json::array();
            for (size_t i = 0; i < leds_.size(); i++) {
                nlohmann::json item = leds_[i]->GetState();
                item["index"] = static_cast<int>(i + 1);
                list.push_back(item);
            }
            return list;
        });

        mcp_server.AddTool(tool_prefix_ + ".turn_on", "Turn on LEDs with their last brightness and effect." + hint,
                           PropertyList({Property("index", kPropertyTypeInteger, 0, 0, count)}),
                           [this](const PropertyList& properties) -> ReturnValue {
            return ForEach(properties["index"].value<int>(), [](UserLed& led) { led.SetPower(true); });
        });

        mcp_server.AddTool(tool_prefix_ + ".turn_off", "Turn off LEDs." + hint,
                           PropertyList({Property("index", kPropertyTypeInteger, 0, 0, count)}),
                           [this](const PropertyList& properties) -> ReturnValue {
            return ForEach(properties["index"].value<int>(), [](UserLed& led) { led.SetPower(false); });
        });

        mcp_server.AddTool(tool_prefix_ + ".set_brightness",
                           "Set brightness 0-100 percent; 0 turns the LED off. On/off-only LEDs light at any value above 0." + hint,
                           PropertyList({Property("index", kPropertyTypeInteger, 0, 0, count),
                                         Property("brightness", kPropertyTypeInteger, 0, 100)}),
                           [this](const PropertyList& properties) -> ReturnValue {
            const int brightness = properties["brightness"].value<int>();
            return ForEach(properties["index"].value<int>(), [brightness](UserLed& led) { led.SetBrightness(brightness); });
        });

        mcp_server.AddTool(tool_prefix_ + ".set_effect",
                           "Set LED effect: 'steady', 'blink', 'breathe' or 'off'. Breathe becomes blink on on/off-only LEDs. "
                           "period_ms is the length of one cycle." + hint,
                           PropertyList({Property("index", kPropertyTypeInteger, 0, 0, count),
                                         Property("effect", kPropertyTypeString),
                                         Property("period_ms", kPropertyTypeInteger, period, 200, 10000)}),
                           [this](const PropertyList& properties) -> ReturnValue {
            UserLedEffect effect;
            if (!UserLed::ParseEffect(properties["effect"].value<std::string>(), effect)) {
                nlohmann::json j;
                j["error"] = "Unknown effect, use steady, blink, breathe or off";
                return j;
            }
            const int period_ms = properties["period_ms"].value<int>();
            return ForEach(properties["index"].value<int>(), [effect, period_ms](UserLed& led) { led.SetEffect(effect, period_ms); });
        });
    }
};

#define USER_LED_DEFAULTS(prefix) \
    UserLedDefaults { prefix##_BRIGHTNESS, static_cast<UserLedEffect>(prefix##_BOOT_EFFECT_ID), prefix##_PERIOD_MS }
#endif
