#include "uart_display.h"
#include <esp_log.h>
#include <cstring>
#include <vector>

#define TAG "UartDisplay"

UartDisplay::UartDisplay(uart_port_t uart_num, int tx_pin, int rx_pin, int baud_rate,
                         UartDisplayProtocol protocol)
    : uart_num_(uart_num), protocol_(protocol) {
    
    mutex_ = xSemaphoreCreateMutex();

    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
        .flags = {}
    };

    esp_err_t ret = uart_param_config(uart_num_, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to config UART %d: %s", (int)uart_num_, esp_err_to_name(ret));
        return;
    }

    ret = uart_set_pin(uart_num_, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART pins (TX=%d, RX=%d): %s", tx_pin, rx_pin, esp_err_to_name(ret));
        return;
    }

    const int rx_buffer_size = 512;
    ret = uart_driver_install(uart_num_, rx_buffer_size, 0, 0, nullptr, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install UART driver: %s", esp_err_to_name(ret));
        return;
    }

    is_initialized_ = true;
    ESP_LOGI(TAG, "UartDisplay initialized on UART %d (TX=%d, RX=%d, baud=%d, proto=%d)",
             (int)uart_num_, tx_pin, rx_pin, baud_rate, (int)protocol_);

    // Send initial test/welcome packet
    SetStatus("Connected");
}

UartDisplay::~UartDisplay() {
    if (is_initialized_) {
        uart_driver_delete(uart_num_);
        is_initialized_ = false;
    }
    if (mutex_ != nullptr) {
        vSemaphoreDelete(mutex_);
        mutex_ = nullptr;
    }
}

bool UartDisplay::Lock(int timeout_ms) {
    if (mutex_ == nullptr) return false;
    TickType_t ticks = (timeout_ms <= 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(mutex_, ticks) == pdTRUE;
}

void UartDisplay::Unlock() {
    if (mutex_ != nullptr) {
        xSemaphoreGive(mutex_);
    }
}

void UartDisplay::SendRawString(const std::string& str) {
    if (!is_initialized_) return;
    uart_write_bytes(uart_num_, str.c_str(), str.length());
}

void UartDisplay::SendNextionCmd(const std::string& cmd) {
    if (!is_initialized_) return;
    uart_write_bytes(uart_num_, cmd.c_str(), cmd.length());
    const uint8_t tail[3] = { 0xFF, 0xFF, 0xFF };
    uart_write_bytes(uart_num_, (const char*)tail, 3);
}

void UartDisplay::SendDwinText(uint16_t vp_addr, const std::string& text) {
    if (!is_initialized_) return;
    // DWIN DGUS frame: 5A A5 [Length] 82 [VP_H] [VP_L] [Data...] [00 00]
    size_t data_len = text.length() + 2; // text + string terminator
    size_t frame_len = 3 + data_len;     // 82 + VP_H + VP_L + data
    std::vector<uint8_t> frame;
    frame.reserve(3 + frame_len);
    frame.push_back(0x5A);
    frame.push_back(0xA5);
    frame.push_back(static_cast<uint8_t>(frame_len));
    frame.push_back(0x82); // Write command
    frame.push_back(static_cast<uint8_t>((vp_addr >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(vp_addr & 0xFF));
    for (char c : text) {
        frame.push_back(static_cast<uint8_t>(c));
    }
    frame.push_back(0x00);
    frame.push_back(0x00);
    uart_write_bytes(uart_num_, (const char*)frame.data(), frame.size());
}

void UartDisplay::SetStatus(const char* status) {
    DisplayLockGuard lock(this);
    if (!status) return;

    switch (protocol_) {
        case UartDisplayProtocol::NextionTjc:
            SendNextionCmd(std::string("t_status.txt=\"") + status + "\"");
            break;
        case UartDisplayProtocol::JsonStream:
            SendRawString(std::string("{\"type\":\"status\",\"val\":\"") + status + "\"}\n");
            break;
        case UartDisplayProtocol::RawText:
            SendRawString(std::string("STATUS:") + status + "\r\n");
            break;
        case UartDisplayProtocol::DwinDgus:
            SendDwinText(0x1000, status);
            break;
    }
}

void UartDisplay::ShowNotification(const char* notification, int duration_ms) {
    DisplayLockGuard lock(this);
    if (!notification) return;

    switch (protocol_) {
        case UartDisplayProtocol::NextionTjc:
            SendNextionCmd(std::string("t_notify.txt=\"") + notification + "\"");
            break;
        case UartDisplayProtocol::JsonStream:
            SendRawString(std::string("{\"type\":\"notify\",\"val\":\"") + notification + "\"}\n");
            break;
        case UartDisplayProtocol::RawText:
            SendRawString(std::string("NOTIFY:") + notification + "\r\n");
            break;
        case UartDisplayProtocol::DwinDgus:
            SendDwinText(0x1100, notification);
            break;
    }
}

void UartDisplay::ShowNotification(const std::string& notification, int duration_ms) {
    ShowNotification(notification.c_str(), duration_ms);
}

void UartDisplay::SetEmotion(const char* emotion) {
    DisplayLockGuard lock(this);
    if (!emotion) return;

    switch (protocol_) {
        case UartDisplayProtocol::NextionTjc:
            SendNextionCmd(std::string("t_emotion.txt=\"") + emotion + "\"");
            break;
        case UartDisplayProtocol::JsonStream:
            SendRawString(std::string("{\"type\":\"emotion\",\"val\":\"") + emotion + "\"}\n");
            break;
        case UartDisplayProtocol::RawText:
            SendRawString(std::string("EMOTION:") + emotion + "\r\n");
            break;
        case UartDisplayProtocol::DwinDgus:
            SendDwinText(0x1200, emotion);
            break;
    }
}

void UartDisplay::SetChatMessage(const char* role, const char* content) {
    DisplayLockGuard lock(this);
    if (!role || !content) return;

    switch (protocol_) {
        case UartDisplayProtocol::NextionTjc:
            if (strcmp(role, "user") == 0) {
                SendNextionCmd(std::string("t_user.txt=\"") + content + "\"");
            } else {
                SendNextionCmd(std::string("t_chat.txt=\"") + content + "\"");
            }
            break;
        case UartDisplayProtocol::JsonStream:
            SendRawString(std::string("{\"type\":\"chat\",\"role\":\"") + role + "\",\"content\":\"" + content + "\"}\n");
            break;
        case UartDisplayProtocol::RawText:
            SendRawString(std::string("CHAT:") + role + ":" + content + "\r\n");
            break;
        case UartDisplayProtocol::DwinDgus:
            if (strcmp(role, "user") == 0) {
                SendDwinText(0x1300, content);
            } else {
                SendDwinText(0x1400, content);
            }
            break;
    }
}

void UartDisplay::ClearChatMessages() {
    DisplayLockGuard lock(this);
    switch (protocol_) {
        case UartDisplayProtocol::NextionTjc:
            SendNextionCmd("t_user.txt=\"\"");
            SendNextionCmd("t_chat.txt=\"\"");
            break;
        case UartDisplayProtocol::JsonStream:
            SendRawString("{\"type\":\"clear_chat\"}\n");
            break;
        case UartDisplayProtocol::RawText:
            SendRawString("CLEAR_CHAT\r\n");
            break;
        case UartDisplayProtocol::DwinDgus:
            SendDwinText(0x1300, "");
            SendDwinText(0x1400, "");
            break;
    }
}

void UartDisplay::UpdateStatusBar(bool update_all) {
    // Optional status bar ping
}

void UartDisplay::SetPowerSaveMode(bool on) {
    DisplayLockGuard lock(this);
    switch (protocol_) {
        case UartDisplayProtocol::NextionTjc:
            SendNextionCmd(on ? "sleep=1" : "sleep=0");
            break;
        case UartDisplayProtocol::JsonStream:
            SendRawString(on ? "{\"type\":\"power\",\"state\":\"sleep\"}\n" : "{\"type\":\"power\",\"state\":\"wake\"}\n");
            break;
        case UartDisplayProtocol::RawText:
            SendRawString(on ? "SLEEP\r\n" : "WAKE\r\n");
            break;
        case UartDisplayProtocol::DwinDgus:
            break;
    }
}
