#ifndef _LED_H_
#define _LED_H_

class Led {
protected:
    bool custom_mode_ = false;

public:
    virtual ~Led() = default;
    // Set the led state based on the device state
    virtual void OnStateChanged() = 0;

    virtual void SetCustomMode(bool enable) { custom_mode_ = enable; }
    virtual bool IsCustomMode() const { return custom_mode_; }
};


class NoLed : public Led {
public:
    virtual void OnStateChanged() override {}
};

#endif // _LED_H_
