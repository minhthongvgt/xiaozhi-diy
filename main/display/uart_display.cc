#include "uart_display.h"
#include "application.h"
#include "audio_codec.h"
#include "board.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cstring>
#include <ctime>
#include <vector>

#define TAG "UartDisplay"

#define SYSINFO_INTERVAL_MS 30000
#define RX_LINE_MAX 256

UartDisplay::UartDisplay(uart_port_t uart_num, gpio_num_t tx_pin, gpio_num_t rx_pin, int baud_rate,
                         UartDisplayProtocol protocol)
    : uart_num_(uart_num), protocol_(protocol) {
    
    mutex_ = xSemaphoreCreateMutex();

    if (!GPIO_IS_VALID_GPIO(tx_pin)) {
        ESP_LOGE(TAG, "UartDisplay TX pin (%d) is invalid or not connected. Init aborted.", static_cast<int>(tx_pin));
        return;
    }

    if (uart_is_driver_installed(static_cast<uart_port_t>(uart_num_))) {
        ESP_LOGW(TAG, "UART %d driver already installed; reusing existing driver for UartDisplay", static_cast<int>(uart_num_));
        is_initialized_ = true;
        SetStatus("Connected");
        return;
    }

    uart_config_t uart_config = {
        .baud_rate = (baud_rate > 0) ? baud_rate : 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
        .flags = {}
    };

    esp_err_t ret = uart_param_config(static_cast<uart_port_t>(uart_num_), &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "uart_param_config failed on UART %d: %s", static_cast<int>(uart_num_), esp_err_to_name(ret));
        return;
    }

    int tx = static_cast<int>(tx_pin);
    int rx = GPIO_IS_VALID_GPIO(rx_pin) ? static_cast<int>(rx_pin) : UART_PIN_NO_CHANGE;
    ret = uart_set_pin(static_cast<uart_port_t>(uart_num_), tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "uart_set_pin failed on UART %d (TX=%d, RX=%d): %s", static_cast<int>(uart_num_), tx, rx, esp_err_to_name(ret));
        return;
    }

    const int rx_buffer_size = 512;
    ret = uart_driver_install(static_cast<uart_port_t>(uart_num_), rx_buffer_size, 0, 0, nullptr, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "uart_driver_install failed on UART %d: %s", static_cast<int>(uart_num_), esp_err_to_name(ret));
        return;
    }

    is_initialized_ = true;
    ESP_LOGI(TAG, "UartDisplay initialized on UART %d (TX=%d, RX=%d, baud=%d, proto=%d)",
             static_cast<int>(uart_num_), static_cast<int>(tx_pin), static_cast<int>(rx_pin), baud_rate, static_cast<int>(protocol_));

    SetStatus("Connected");

    if (protocol_ == UartDisplayProtocol::JsonStream && rx != UART_PIN_NO_CHANGE) {
        StartRxTask();
    }
}

UartDisplay::~UartDisplay() {
    if (rx_task_ != nullptr) {
        rx_running_ = false;
        for (int i = 0; i < 20 && rx_task_ != nullptr; ++i) {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
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

static std::string escape_nextion_string(const char* str) {
    if (!str) return "";
    std::string escaped;
    escaped.reserve(strlen(str) + 8);
    for (const char* p = str; *p; ++p) {
        if (*p == '"') {
            escaped += "\\\"";
        } else if (*p == '\r') {
            continue;
        } else {
            escaped += *p;
        }
    }
    return escaped;
}

void UartDisplay::SendDwinText(uint16_t vp_addr, const std::string& text) {
    if (!is_initialized_) return;
    std::string safe_text = text;
    if (safe_text.length() > 240) {
        safe_text = safe_text.substr(0, 240);
    }
    size_t data_len = safe_text.length() + 2;
    size_t frame_len = 3 + data_len;
    std::vector<uint8_t> frame;
    frame.reserve(3 + frame_len);
    frame.push_back(0x5A);
    frame.push_back(0xA5);
    frame.push_back(static_cast<uint8_t>(frame_len));
    frame.push_back(0x82);
    frame.push_back(static_cast<uint8_t>((vp_addr >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(vp_addr & 0xFF));
    for (char c : safe_text) {
        frame.push_back(static_cast<uint8_t>(c));
    }
    frame.push_back(0x00);
    frame.push_back(0x00);
    uart_write_bytes(uart_num_, (const char*)frame.data(), frame.size());
}

void UartDisplay::SetStatus(const char* status) {
    DisplayLockGuard lock(this);
    if (!status) return;
    last_status_ = status;

    switch (protocol_) {
        case UartDisplayProtocol::NextionTjc:
            SendNextionCmd(std::string("t_status.txt=\"") + escape_nextion_string(status) + "\"");
            break;
        case UartDisplayProtocol::JsonStream: {
            nlohmann::json j;
            j["type"] = "status";
            j["val"] = status;
            SendRawString(j.dump() + "\n");
            break;
        }
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
            SendNextionCmd(std::string("t_notify.txt=\"") + escape_nextion_string(notification) + "\"");
            break;
        case UartDisplayProtocol::JsonStream: {
            nlohmann::json j;
            j["type"] = "notify";
            j["val"] = notification;
            SendRawString(j.dump() + "\n");
            break;
        }
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
    last_emotion_ = emotion;

    switch (protocol_) {
        case UartDisplayProtocol::NextionTjc:
            SendNextionCmd(std::string("t_emotion.txt=\"") + escape_nextion_string(emotion) + "\"");
            break;
        case UartDisplayProtocol::JsonStream: {
            nlohmann::json j;
            j["type"] = "emotion";
            j["val"] = emotion;
            SendRawString(j.dump() + "\n");
            break;
        }
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
    if (strcmp(role, "user") == 0) {
        last_user_message_ = content;
    } else {
        last_assistant_role_ = role;
        last_assistant_message_ = content;
    }

    switch (protocol_) {
        case UartDisplayProtocol::NextionTjc: {
            std::string escaped = escape_nextion_string(content);
            if (strcmp(role, "user") == 0) {
                SendNextionCmd(std::string("t_user.txt=\"") + escaped + "\"");
            } else {
                SendNextionCmd(std::string("t_chat.txt=\"") + escaped + "\"");
            }
            break;
        }
        case UartDisplayProtocol::JsonStream: {
            nlohmann::json j;
            j["type"] = "chat";
            j["role"] = role;
            j["content"] = content;
            SendRawString(j.dump() + "\n");
            break;
        }
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
    last_user_message_.clear();
    last_assistant_message_.clear();
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
    if (protocol_ != UartDisplayProtocol::JsonStream || !is_initialized_) return;
    TickType_t now = xTaskGetTickCount();
    if (!update_all && last_sysinfo_tick_ != 0 &&
        now - last_sysinfo_tick_ < pdMS_TO_TICKS(SYSINFO_INTERVAL_MS)) {
        return;
    }
    DisplayLockGuard lock(this);
    SendSysInfo();
}

void UartDisplay::SendSysInfo() {
    last_sysinfo_tick_ = xTaskGetTickCount();
    auto& board = Board::GetInstance();
    nlohmann::json j;
    j["type"] = "sysinfo";

    time_t now = time(nullptr);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    if (tm_now.tm_year >= 2025 - 1900) {
        char buf[16];
        strftime(buf, sizeof(buf), "%H:%M:%S", &tm_now);
        j["time"] = buf;
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tm_now);
        j["date"] = buf;
    }

    auto board_json = nlohmann::json::parse(board.GetBoardJson(), nullptr, false);
    if (board_json.is_object() && board_json.contains("rssi") && board_json["rssi"].is_number()) {
        j["wifi"] = board_json["rssi"];
    }

    int battery_level = 0;
    bool charging = false, discharging = false;
    if (board.GetBatteryLevel(battery_level, charging, discharging)) {
        j["battery"] = battery_level;
        j["charging"] = charging;
    }

    auto codec = board.GetAudioCodec();
    if (codec != nullptr) {
        j["volume"] = codec->output_volume();
    }
    SendRawString(j.dump() + "\n");
}

void UartDisplay::ReplayState() {
    if (!last_status_.empty()) {
        SendRawString(nlohmann::json{{"type", "status"}, {"val", last_status_}}.dump() + "\n");
    }
    if (!last_emotion_.empty()) {
        SendRawString(nlohmann::json{{"type", "emotion"}, {"val", last_emotion_}}.dump() + "\n");
    }
    if (!last_user_message_.empty()) {
        SendRawString(nlohmann::json{{"type", "chat"}, {"role", "user"},
                                     {"content", last_user_message_}}.dump() + "\n");
    }
    if (!last_assistant_message_.empty()) {
        SendRawString(nlohmann::json{{"type", "chat"}, {"role", last_assistant_role_},
                                     {"content", last_assistant_message_}}.dump() + "\n");
    }
    SendSysInfo();
}

void UartDisplay::StartRxTask() {
    rx_running_ = true;
    if (xTaskCreate(RxTask, "uart_disp_rx", 4096, this, 3, &rx_task_) != pdPASS) {
        rx_running_ = false;
        rx_task_ = nullptr;
        ESP_LOGE(TAG, "Failed to start UART display RX task");
    }
}

void UartDisplay::RxTask(void* arg) {
    auto self = static_cast<UartDisplay*>(arg);
    std::string line;
    bool overflow = false;
    uint8_t buf[64];
    while (self->rx_running_) {
        int len = uart_read_bytes(self->uart_num_, buf, sizeof(buf), pdMS_TO_TICKS(100));
        for (int i = 0; i < len; ++i) {
            char c = static_cast<char>(buf[i]);
            if (c == '\n') {
                if (!overflow && !line.empty()) {
                    self->HandleDisplayEvent(line);
                }
                line.clear();
                overflow = false;
            } else if (c != '\r') {
                if (line.size() < RX_LINE_MAX) {
                    line += c;
                } else {
                    overflow = true;
                }
            }
        }
    }
    self->rx_task_ = nullptr;
    vTaskDelete(nullptr);
}

void UartDisplay::HandleDisplayEvent(const std::string& line) {
    auto j = nlohmann::json::parse(line, nullptr, false);
    if (!j.is_object() || !j.contains("event") || !j["event"].is_string()) {
        ESP_LOGW(TAG, "Ignoring display line: %.64s", line.c_str());
        return;
    }
    std::string event = j["event"].get<std::string>();
    auto& app = Application::GetInstance();

    if (event == "hello") {
        ESP_LOGI(TAG, "Display connected, replaying state");
        app.Schedule([this]() {
            DisplayLockGuard lock(this);
            ReplayState();
        });
    } else if (event == "toggle_chat") {
        app.Schedule([&app]() { app.ToggleChatState(); });
    } else if (event == "abort") {
        app.Schedule([&app]() {
            if (app.GetDeviceState() == kDeviceStateSpeaking) {
                app.AbortSpeaking(kAbortReasonNone);
            }
        });
    } else if (event == "volume" && j.contains("val") && j["val"].is_number()) {
        int volume = std::clamp(j["val"].get<int>(), 0, 100);
        app.Schedule([this, volume]() {
            auto codec = Board::GetInstance().GetAudioCodec();
            if (codec != nullptr) {
                codec->SetOutputVolume(volume);
            }
            DisplayLockGuard lock(this);
            SendSysInfo();
        });
    }
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
