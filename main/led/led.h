#ifndef _LED_H_
#define _LED_H_

#include <cstdint>
#include <string>

class Led {
public:
    virtual ~Led() = default;
    // Set the led state based on the device state
    virtual void OnStateChanged() = 0;

    virtual void SetCustomMode(bool custom) {}
    virtual bool IsCustomMode() const { return false; }
    virtual void TurnOn() {}
    virtual void TurnOff() {}
    virtual void SetBrightness(uint8_t brightness) {}
    virtual uint8_t GetBrightness() const { return 0; }
    virtual void SetColor(uint8_t r, uint8_t g, uint8_t b) {}
    virtual void GetColor(uint8_t& r, uint8_t& g, uint8_t& b) const { r = g = b = 0; }
    virtual void StartRainbow(int interval_ms = 25) {}
    virtual void StartChase(int interval_ms = 30) {}
    virtual void StartBreathe(int interval_ms = 25) {}
    virtual void StartBlink(int interval_ms = 200) {}
    virtual void StartScanner(int interval_ms = 30) {}
    virtual void StartColorWipe(int interval_ms = 30) {}
    virtual void SetPixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b) {}
    virtual uint16_t GetLedCount() const { return 1; }
    virtual std::string GetType() const { return "Led"; }
};


class NoLed : public Led {
public:
    virtual void OnStateChanged() override {}
    std::string GetType() const override { return "NoLed"; }
};

#endif // _LED_H_
