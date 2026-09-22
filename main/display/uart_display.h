#ifndef UART_DISPLAY_H
#define UART_DISPLAY_H

#include "display.h"
#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <string>

enum class UartDisplayProtocol {
    NextionTjc = 0,
    JsonStream = 1,
    RawText = 2,
    DwinDgus = 3
};

class UartDisplay : public Display {
public:
    UartDisplay(uart_port_t uart_num, int tx_pin, int rx_pin, int baud_rate,
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

protected:
    virtual bool Lock(int timeout_ms = 0) override;
    virtual void Unlock() override;

private:
    uart_port_t uart_num_;
    UartDisplayProtocol protocol_;
    SemaphoreHandle_t mutex_ = nullptr;
    bool is_initialized_ = false;

    void SendRawString(const std::string& str);
    void SendNextionCmd(const std::string& cmd);
    void SendDwinText(uint16_t vp_addr, const std::string& text);
};

#endif // UART_DISPLAY_H
