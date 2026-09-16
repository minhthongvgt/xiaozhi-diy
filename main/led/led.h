#ifndef _LED_H_
#define _LED_H_

#include <cstdint>
#include <string>

class Led {
protected:
    bool custom_mode_ = false;

public:
    virtual ~Led() = default;
    // Set the led state based on the device state
    virtual void OnStateChanged() = 0;

    virtual void SetCustomMode(bool enable) { custom_mode_ = enable; }
    virtual bool IsCustomMode() const { return custom_mode_; }

    // Unified virtual methods for LED control across all LED types (RTTI-free)
    virtual void TurnOn() {}
    virtual void TurnOff() {}
    virtual void SetColor(uint8_t r, uint8_t g, uint8_t b) {}
    virtual void SetBrightness(uint8_t brightness) {}
    virtual void StartRainbow(int interval_ms = 25) {}
    virtual void StartChase(int interval_ms = 15) {}
    virtual void StartBreathe(int interval_ms = 30) {}
    virtual void StartBlink(int interval_ms = 200) {}
    virtual std::string GetType() const { return "GenericLed"; }
    virtual void GetColor(uint8_t& r, uint8_t& g, uint8_t& b) const { r = 0; g = 0; b = 0; }
    virtual uint8_t GetBrightness() const { return 0; }
};

class NoLed : public Led {
public:
    virtual void OnStateChanged() override {}
    virtual std::string GetType() const override { return "NoLed"; }
};

#endif // _LED_H_
