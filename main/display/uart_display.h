#ifndef UART_DISPLAY_H
#define UART_DISPLAY_H
#include "display.h"
#include <driver/gpio.h>
#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <string>
enum class UartDisplayProtocol {
    NextionTjc = 0,
    JsonStream = 1,
    RawText = 2,
    DwinDgus = 3
};
class UartDisplay : public Display {
public:
    UartDisplay(uart_port_t uart_num, gpio_num_t tx_pin, gpio_num_t rx_pin, int baud_rate,
                UartDisplayProtocol protocol = UartDisplayProtocol::NextionTjc);
    virtual ~UartDisplay();
    virtual void SetStatus(const char* status) override;
    virtual void ShowNotification(const char* notification, int duration_ms = 3000) override;
    virtual void ShowNotification(const std::string& notification, int duration_ms = 3000) override;
    virtual void SetEmotion(const char* emotion) override;
    virtual void SetChatMessage(const char* role, const char* content) override;
    virtual void ClearChatMessages() override;
    virtual void UpdateStatusBar(bool update_all = false) override;
    virtual void SetPowerSaveMode(bool on) override;
    virtual bool IsMonochrome() const override { return false; }
    virtual bool SupportsGuiOperations() const override { return false; }
    bool IsInitialized() const { return is_initialized_; }
protected:
    virtual bool Lock(int timeout_ms = 0) override;
    virtual void Unlock() override;
private:
    uart_port_t uart_num_;
    UartDisplayProtocol protocol_;
    SemaphoreHandle_t mutex_ = nullptr;
    bool is_initialized_ = false;
    bool owns_driver_ = false;
    std::string last_status_;
    std::string last_emotion_;
    std::string last_user_message_;
    std::string last_assistant_role_ = "assistant";
    std::string last_assistant_message_;
    TickType_t last_sysinfo_tick_ = 0;
    TaskHandle_t rx_task_ = nullptr;
    volatile bool rx_running_ = false;
    void SendRawString(const std::string& str);
    void SendNextionCmd(const std::string& cmd);
    void SendDwinText(uint16_t vp_addr, const std::string& text);
    void SendSysInfo();
    void ReplayState();
    void StartRxTask();
    void HandleDisplayEvent(const std::string& line);
    static void RxTask(void* arg);
};
#endif 
