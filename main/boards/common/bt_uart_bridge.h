#ifndef BT_UART_BRIDGE_H
#define BT_UART_BRIDGE_H
#include <deque>
#include <mutex>
#include <string>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "drivers/periph/bt_uart_module.h"

enum class BtUartFrame {
    Raw,
    Line,
    Binary4,
};

class BtUartBridge {
public:
    BtUartBridge(bt_uart_type_t module_type, BtUartFrame frame);
    BtUartBridge(const BtUartBridge&) = delete;
    BtUartBridge& operator=(const BtUartBridge&) = delete;

private:
    bt_uart_type_t module_type_;
    BtUartFrame frame_;
    std::mutex inbox_mutex_;
    std::deque<std::string> text_inbox_;
    std::deque<int> code_inbox_;
    int dropped_ = 0;
    TaskHandle_t rx_task_ = nullptr;

    void RegisterTools();
    void PushText(const std::string& text);
    void PushCode(int code);
    bool SendBytes(const std::string& bytes);
    static void RxTask(void* arg);
    void RunRxLoop();
};

#endif
