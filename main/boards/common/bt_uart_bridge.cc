#include "bt_uart_bridge.h"
#include "mcp_server.h"
#include <esp_log.h>
#include <cstdio>

#define TAG "BtUartBridge"

namespace {

constexpr size_t kInboxMax = 16;
constexpr size_t kMessageMax = 256;
constexpr uint32_t kReadTimeoutMs = 50;
constexpr uint8_t kFrameStart = 0xAA;
constexpr uint32_t kRxTaskStack = 4096;
constexpr UBaseType_t kRxTaskPriority = 3;
constexpr BaseType_t kRxTaskCore = 1;

bool IsValidUtf8(const std::string& text) {
    size_t i = 0;
    while (i < text.size()) {
        unsigned char lead = static_cast<unsigned char>(text[i]);
        size_t extra = 0;
        unsigned char second_min = 0x80;
        unsigned char second_max = 0xBF;
        if (lead < 0x80) {
            extra = 0;
        } else if (lead >= 0xC2 && lead <= 0xDF) {
            extra = 1;
        } else if (lead >= 0xE0 && lead <= 0xEF) {
            extra = 2;
            second_min = lead == 0xE0 ? 0xA0 : 0x80;
            second_max = lead == 0xED ? 0x9F : 0xBF;
        } else if (lead >= 0xF0 && lead <= 0xF4) {
            extra = 3;
            second_min = lead == 0xF0 ? 0x90 : 0x80;
            second_max = lead == 0xF4 ? 0x8F : 0xBF;
        } else {
            return false;
        }
        if (text.size() - i <= extra) {
            return false;
        }
        for (size_t k = 1; k <= extra; ++k) {
            unsigned char next = static_cast<unsigned char>(text[i + k]);
            unsigned char low = k == 1 ? second_min : 0x80;
            unsigned char high = k == 1 ? second_max : 0xBF;
            if (next < low || next > high) {
                return false;
            }
        }
        i += extra + 1;
    }
    return true;
}

std::string ToSafeText(const std::string& bytes) {
    bool printable = IsValidUtf8(bytes);
    for (unsigned char c : bytes) {
        if (c < 0x20 && c != '\t') {
            printable = false;
            break;
        }
    }
    if (printable) {
        return bytes;
    }
    std::string out;
    char hex[5];
    for (unsigned char c : bytes) {
        if (c >= 0x20 && c < 0x7F) {
            out += static_cast<char>(c);
        } else {
            snprintf(hex, sizeof(hex), "\\x%02X", c);
            out += hex;
        }
    }
    return out;
}

const char* LinkStateName(bt_uart_link_state_t state) {
    switch (state) {
        case BT_UART_LINK_CONNECTED:
            return "connected";
        case BT_UART_LINK_DISCONNECTED:
            return "disconnected";
        default:
            return "unknown";
    }
}

const char* FrameName(BtUartFrame frame) {
    switch (frame) {
        case BtUartFrame::Raw:
            return "raw";
        case BtUartFrame::Line:
            return "line";
        default:
            return "binary4";
    }
}

}

BtUartBridge::BtUartBridge(bt_uart_type_t module_type, BtUartFrame frame)
    : module_type_(module_type), frame_(frame) {
    if (!bt_uart_module_is_ready()) {
        ESP_LOGW(TAG, "Bluetooth module is not ready; MCP tools not registered");
        return;
    }
    if (xTaskCreatePinnedToCore(RxTask, "bt_uart_rx", kRxTaskStack, this, kRxTaskPriority, &rx_task_,
                                kRxTaskCore) != pdPASS) {
        rx_task_ = nullptr;
        ESP_LOGE(TAG, "Failed to start Bluetooth RX task");
        return;
    }
    RegisterTools();
}

void BtUartBridge::PushText(const std::string& text) {
    std::lock_guard<std::mutex> lock(inbox_mutex_);
    if (text_inbox_.size() >= kInboxMax) {
        text_inbox_.pop_front();
        ++dropped_;
    }
    text_inbox_.push_back(text);
}

void BtUartBridge::PushCode(int code) {
    std::lock_guard<std::mutex> lock(inbox_mutex_);
    if (code_inbox_.size() >= kInboxMax) {
        code_inbox_.pop_front();
        ++dropped_;
    }
    code_inbox_.push_back(code);
}

bool BtUartBridge::SendBytes(const std::string& bytes) {
    int written = bt_uart_module_write(reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size());
    return written == static_cast<int>(bytes.size());
}

void BtUartBridge::RxTask(void* arg) {
    static_cast<BtUartBridge*>(arg)->RunRxLoop();
}

void BtUartBridge::RunRxLoop() {
    uint8_t buf[64];
    std::string pending;
    bool overflow = false;
    uint8_t frame[4];
    size_t frame_len = 0;
    while (true) {
        int len = bt_uart_module_read(buf, sizeof(buf), kReadTimeoutMs);
        if (len <= 0) {
            if (frame_ == BtUartFrame::Raw && !pending.empty()) {
                PushText(pending);
                pending.clear();
            }
            continue;
        }
        for (int i = 0; i < len; ++i) {
            uint8_t b = buf[i];
            if (frame_ == BtUartFrame::Binary4) {
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
                    ESP_LOGW(TAG, "Dropped bad frame %02X %02X %02X %02X", frame[0], frame[1], frame[2], frame[3]);
                }
            } else if (frame_ == BtUartFrame::Line) {
                if (b == '\n') {
                    if (!overflow && !pending.empty()) {
                        PushText(pending);
                    }
                    pending.clear();
                    overflow = false;
                } else if (b != '\r') {
                    if (pending.size() < kMessageMax) {
                        pending += static_cast<char>(b);
                    } else {
                        overflow = true;
                    }
                }
            } else {
                pending += static_cast<char>(b);
                if (pending.size() >= kMessageMax) {
                    PushText(pending);
                    pending.clear();
                }
            }
        }
    }
}

void BtUartBridge::RegisterTools() {
    auto& mcp_server = McpServer::GetInstance();
    const std::string module_name = bt_uart_module_type_name(module_type_);

    mcp_server.AddTool("self.bluetooth.get_status",
                       "Get the state of the wireless Bluetooth UART link (" + module_name +
                           ") to the remote microcontroller: link state, frame format and unread messages",
                       PropertyList(), [this, module_name](const PropertyList&) -> ToolResult {
        nlohmann::json j;
        j["module"] = module_name;
        j["link"] = LinkStateName(bt_uart_module_link_state());
        j["frame"] = FrameName(frame_);
        std::lock_guard<std::mutex> lock(inbox_mutex_);
        j["unread"] = frame_ == BtUartFrame::Binary4 ? code_inbox_.size() : text_inbox_.size();
        return j;
    });

    if (frame_ == BtUartFrame::Binary4) {
        mcp_server.AddTool("self.bluetooth.send_code",
                           "Send a command code (0-99) to the remote microcontroller over Bluetooth "
                           "as a 4-byte frame: 0xAA, tens, units, XOR checksum",
                           PropertyList({Property("code", kPropertyTypeInteger, 0, 99)}),
                           [this](const PropertyList& properties) -> ToolResult {
            int code = properties["code"].value<int>();
            uint8_t tens = static_cast<uint8_t>(code / 10);
            uint8_t units = static_cast<uint8_t>(code % 10);
            std::string bytes = {static_cast<char>(kFrameStart), static_cast<char>(tens), static_cast<char>(units),
                                 static_cast<char>(kFrameStart ^ tens ^ units)};
            if (!SendBytes(bytes)) {
                return std::unexpected(std::string("Bluetooth UART write failed"));
            }
            return true;
        });
    } else {
        const bool line = frame_ == BtUartFrame::Line;
        mcp_server.AddTool("self.bluetooth.send_text",
                           line ? "Send one line of text to the remote microcontroller over Bluetooth; "
                                  "a newline is appended automatically"
                                : "Send text to the remote microcontroller over Bluetooth exactly as given",
                           PropertyList({Property("text", kPropertyTypeString).SetMaxLength(kMessageMax)}),
                           [this, line](const PropertyList& properties) -> ToolResult {
            std::string bytes = properties["text"].value<std::string>();
            if (line) {
                bytes += '\n';
            }
            if (!SendBytes(bytes)) {
                return std::unexpected(std::string("Bluetooth UART write failed"));
            }
            return true;
        });
    }

    mcp_server.AddTool("self.bluetooth.read_messages",
                       "Read and clear the messages received over Bluetooth from the remote microcontroller",
                       PropertyList(), [this](const PropertyList&) -> ToolResult {
        nlohmann::json j;
        nlohmann::json messages = nlohmann::json::array();
        std::lock_guard<std::mutex> lock(inbox_mutex_);
        if (frame_ == BtUartFrame::Binary4) {
            for (int code : code_inbox_) {
                messages.push_back(code);
            }
            code_inbox_.clear();
        } else {
            for (const auto& text : text_inbox_) {
                messages.push_back(ToSafeText(text));
            }
            text_inbox_.clear();
        }
        j["messages"] = messages;
        j["dropped"] = dropped_;
        dropped_ = 0;
        return j;
    });

    ESP_LOGI(TAG, "Bluetooth UART bridge ready (%s, frame=%s)", module_name.c_str(), FrameName(frame_));
}
