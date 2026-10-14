#ifndef UART_CODE_BRIDGE_H
#define UART_CODE_BRIDGE_H
#include "mcp_server.h"
#include <driver/uart.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <deque>
#include <mutex>
#include <string>

class UartCodeBridge {
public:
    explicit UartCodeBridge(uart_port_t port) : port_(port) {
        if (!uart_is_driver_installed(port_)) {
            ESP_LOGW(kTag, "UART%d driver not installed, MCP UART tools not registered", static_cast<int>(port_));
            return;
        }
        if (xTaskCreatePinnedToCore(RxTask, "uart_code_rx", kRxTaskStack, this, kRxTaskPriority, &rx_task_, kRxTaskCore) != pdPASS) {
            rx_task_ = nullptr;
            ESP_LOGW(kTag, "UART%d receive task not started, replies will not be read", static_cast<int>(port_));
        }
        RegisterTools();
        ESP_LOGI(kTag, "MCP UART tools ready on UART%d (4-byte frames)", static_cast<int>(port_));
    }

    UartCodeBridge(const UartCodeBridge&) = delete;
    UartCodeBridge& operator=(const UartCodeBridge&) = delete;

private:
    static constexpr const char* kTag = "UartCodeBridge";
    static constexpr uint8_t kFrameStart = 0xAA;
    static constexpr size_t kInboxMax = 16;
    static constexpr uint32_t kRxTaskStack = 4096;
    static constexpr UBaseType_t kRxTaskPriority = 3;
    static constexpr BaseType_t kRxTaskCore = 1;
    static constexpr int kReadTimeoutMs = 100;

    uart_port_t port_;
    TaskHandle_t rx_task_ = nullptr;
    std::mutex inbox_mutex_;
    std::deque<int> inbox_;
    int dropped_ = 0;

    void PushCode(int code) {
        std::lock_guard<std::mutex> lock(inbox_mutex_);
        if (inbox_.size() >= kInboxMax) {
            inbox_.pop_front();
            ++dropped_;
        }
        inbox_.push_back(code);
    }

    static void RxTask(void* arg) {
        static_cast<UartCodeBridge*>(arg)->RunRxLoop();
    }

    void RunRxLoop() {
        uint8_t buf[32];
        uint8_t frame[4];
        size_t frame_len = 0;
        while (true) {
            const int len = uart_read_bytes(port_, buf, sizeof(buf), pdMS_TO_TICKS(kReadTimeoutMs));
            for (int i = 0; i < len; ++i) {
                const uint8_t b = buf[i];
                if (frame_len == 0 && b != kFrameStart) {
                    continue;
                }
                frame[frame_len++] = b;
                if (frame_len < sizeof(frame)) {
                    continue;
                }
                frame_len = 0;
                if ((frame[0] ^ frame[1] ^ frame[2]) == frame[3] && frame[1] <= 9 && frame[2] <= 9) {
                    PushCode(frame[1] * 10 + frame[2]);
                } else {
                    ESP_LOGW(kTag, "Dropped bad frame %02X %02X %02X %02X", frame[0], frame[1], frame[2], frame[3]);
                }
            }
        }
    }

    void RegisterTools() {
        auto& mcp_server = McpServer::GetInstance();
        mcp_server.AddTool("self.uart.send_code",
                           "Send a command code (0-99) over the wired UART to the connected microcontroller as a 4-byte "
                           "frame: 0xAA, tens, units, XOR checksum. Replies arrive later; read them with self.uart.read_messages.",
                           PropertyList({Property("code", kPropertyTypeInteger, 0, 99)}),
                           [this](const PropertyList& properties) -> ReturnValue {
            const int code = properties["code"].value<int>();
            const uint8_t tens = static_cast<uint8_t>(code / 10);
            const uint8_t units = static_cast<uint8_t>(code % 10);
            const uint8_t bytes[4] = {kFrameStart, tens, units, static_cast<uint8_t>(kFrameStart ^ tens ^ units)};
            const int written = uart_write_bytes(port_, bytes, sizeof(bytes));
            if (written != static_cast<int>(sizeof(bytes))) {
                nlohmann::json j;
                j["success"] = false;
                j["error"] = "UART write failed";
                return j;
            }
            return true;
        });

        mcp_server.AddTool("self.uart.read_messages",
                           "Read and clear the reply codes (0-99) received over the wired UART from the connected microcontroller",
                           PropertyList(), [this](const PropertyList&) -> ReturnValue {
            nlohmann::json j;
            nlohmann::json messages = nlohmann::json::array();
            std::lock_guard<std::mutex> lock(inbox_mutex_);
            for (int code : inbox_) {
                messages.push_back(code);
            }
            inbox_.clear();
            j["messages"] = messages;
            j["dropped"] = dropped_;
            dropped_ = 0;
            return j;
        });
    }
};
#endif
