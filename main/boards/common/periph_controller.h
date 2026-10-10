#ifndef __PERIPH_CONTROLLER_H__
#define __PERIPH_CONTROLLER_H__
#include "mcp_server.h"
#include <esp_timer.h>
#include <driver/i2c_master.h>
class PeripheralController {
public:
    explicit PeripheralController(i2c_master_bus_handle_t i2c_bus);
    ~PeripheralController() = default;
private:
    void InitRtc(i2c_master_bus_handle_t i2c_bus);
    void InitNfc(i2c_master_bus_handle_t i2c_bus);
    void InitPwm(i2c_master_bus_handle_t i2c_bus);
    void InitLedIc(i2c_master_bus_handle_t i2c_bus);
    void InitRc522();
    void InitImu();
    void InitExpander();
    void InitPmic();
    void InitEthernet();
    static void RtcSyncTimerCallback(void* arg);
    void SyncRtcFromSystem(bool force);
    esp_timer_handle_t rtc_sync_timer_ = nullptr;
    int64_t last_rtc_write_us_ = 0;
};
#endif
