#ifndef __XPOWERS_H__
#define __XPOWERS_H__
#include "i2c_device.h"
#include <esp_log.h>
#if !defined(XPOWERS_CHIP_AXP192) && !defined(XPOWERS_CHIP_AXP202) && !defined(XPOWERS_CHIP_AXP2101)
#error "Define XPOWERS_CHIP_AXP192/AXP202/AXP2101 before including xpower.h, \
otherwise XPowersLib.h does not generate the XPowersPMU typedef"
#endif
#include "XPowersLib.h"
class Xpowers : public I2cDevice {
public:
    Xpowers(i2c_master_bus_handle_t i2c_bus, uint8_t addr)
        : I2cDevice(i2c_bus, addr), pmu() {
        s_instances_[addr & 0x7F] = this;
        if (!pmu.begin(addr, &Xpowers::ReadCallback, &Xpowers::WriteCallback)) {
            ESP_LOGE("Xpowers", "XPowers PMU init failed at 0x%02x", addr);
        }
    }
    XPowersPMU pmu;
    bool IsCharging() {
        return pmu.isCharging();
    }
    bool IsDischarging() {
        return pmu.isDischarge();
    }
    int GetBatteryLevel() {
        return pmu.getBatteryPercent();  
    }
    uint16_t GetBatteryVoltage() {
        return pmu.getBattVoltage();
    }
    bool IsBatteryConnect() {
        return pmu.isBatteryConnect();
    }
    float GetTemperature() {
        return pmu.getTemperature();
    }
    void PowerOff() {
        pmu.shutdown();
    }
private:
    static int ReadCallback(uint8_t devAddr, uint8_t regAddr, uint8_t* data, uint8_t len) {
        Xpowers* self = Find(devAddr);
        if (self == nullptr) {
            return -1;
        }
        self->ReadRegs(regAddr, data, len);
        return 0;
    }
    static int WriteCallback(uint8_t devAddr, uint8_t regAddr, uint8_t* data, uint8_t len) {
        Xpowers* self = Find(devAddr);
        if (self == nullptr) {
            return -1;
        }
        self->WriteRegs(regAddr, data, len);
        return 0;
    }
    static Xpowers* Find(uint8_t devAddr) {
        return s_instances_[devAddr & 0x7F];
    }
    static inline Xpowers* s_instances_[128] = {};  
};
#endif  
