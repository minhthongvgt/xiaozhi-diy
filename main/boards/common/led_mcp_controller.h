#ifndef __LED_MCP_CONTROLLER_H__
#define __LED_MCP_CONTROLLER_H__

#include "mcp_server.h"
#include "board.h"
#include "led/led.h"
#include <esp_log.h>
#include <cstdio>
#include <string>

#define TAG_LED_MCP "LedMcpController"

class LedMcpController {
public:
    LedMcpController() {
        auto& mcp_server = McpServer::GetInstance();

        // 1. Tool điều khiển hiệu ứng ánh sáng LED RGB
        mcp_server.AddTool(
            "self.led.set_effect",
            "Set dynamic lighting effect on status LED (supported effects: 'rainbow', 'chase', 'breathe', 'blink', 'scanner', 'wipe', 'off', 'auto')",
            PropertyList({
                Property("effect", kPropertyTypeString),
                Property("speed_ms", kPropertyTypeInteger, 25, 10, 1000)
            }),
            [](const PropertyList& properties) -> ReturnValue {
                std::string effect = properties["effect"].value<std::string>();
                int speed = properties["speed_ms"].value<int>();
                if (speed < 10) speed = 10;
                if (speed > 1000) speed = 1000;

                auto led = Board::GetInstance().GetLed();
                if (!led) {
                    nlohmann::json j;
                    j["error"] = "No status LED configured on this board";
                    return j;
                }

                // Handling auto / restore status mode
                if (effect == "auto" || effect == "status") {
                    led->SetCustomMode(false);
                    led->OnStateChanged();
                    return true;
                }

                if (effect == "rainbow") {
                    led->SetCustomMode(true);
                    led->StartRainbow(speed);
                } else if (effect == "chase") {
                    led->SetCustomMode(true);
                    led->StartChase(speed);
                } else if (effect == "breathe") {
                    led->SetCustomMode(true);
                    led->StartBreathe(speed);
                } else if (effect == "blink") {
                    led->SetCustomMode(true);
                    led->StartBlink(speed);
                } else if (effect == "scanner") {
                    led->SetCustomMode(true);
                    led->StartScanner(speed);
                } else if (effect == "wipe") {
                    led->SetCustomMode(true);
                    led->StartColorWipe(speed);
                } else if (effect == "off") {
                    led->SetCustomMode(false);
                    led->TurnOff();
                } else {
                    nlohmann::json j;
                    j["error"] = "Unsupported effect: " + effect;
                    return j;
                }

                return true;
            }
        );

        // 2. Tool đặt màu sắc tĩnh cho toàn bộ dải LED RGB
        mcp_server.AddTool(
            "self.led.set_color",
            "Set static RGB color for all LEDs (red: 0..255, green: 0..255, blue: 0..255)",
            PropertyList({
                Property("red", kPropertyTypeInteger, 0, 0, 255),
                Property("green", kPropertyTypeInteger, 0, 0, 255),
                Property("blue", kPropertyTypeInteger, 0, 0, 255)
            }),
            [](const PropertyList& properties) -> ReturnValue {
                uint8_t r = (uint8_t)properties["red"].value<int>();
                uint8_t g = (uint8_t)properties["green"].value<int>();
                uint8_t b = (uint8_t)properties["blue"].value<int>();

                auto led = Board::GetInstance().GetLed();
                if (!led) {
                    nlohmann::json j;
                    j["error"] = "No status LED configured on this board";
                    return j;
                }

                led->SetCustomMode(true);
                led->SetColor(r, g, b);
                led->TurnOn();
                return true;
            }
        );

        // 3. Tool đặt màu sắc cho một mắt LED cụ thể trên dải LED (0..count-1)
        mcp_server.AddTool(
            "self.led.set_pixel",
            "Set static RGB color for a specific pixel on the LED strip (index: 0..count-1, red: 0..255, green: 0..255, blue: 0..255)",
            PropertyList({
                Property("index", kPropertyTypeInteger, 0, 0, 1023),
                Property("red", kPropertyTypeInteger, 0, 0, 255),
                Property("green", kPropertyTypeInteger, 0, 0, 255),
                Property("blue", kPropertyTypeInteger, 0, 0, 255)
            }),
            [](const PropertyList& properties) -> ReturnValue {
                int index = properties["index"].value<int>();
                uint8_t r = (uint8_t)properties["red"].value<int>();
                uint8_t g = (uint8_t)properties["green"].value<int>();
                uint8_t b = (uint8_t)properties["blue"].value<int>();

                auto led = Board::GetInstance().GetLed();
                if (!led) {
                    nlohmann::json j;
                    j["error"] = "No status LED configured on this board";
                    return j;
                }

                if (index < 0 || index >= led->GetLedCount()) {
                    nlohmann::json j;
                    j["error"] = "Pixel index " + std::to_string(index) + " out of range (0.." + std::to_string(led->GetLedCount() - 1) + ")";
                    return j;
                }

                led->SetCustomMode(true);
                led->SetPixel(static_cast<uint16_t>(index), r, g, b);
                return true;
            }
        );

        // 4. Tool điều chỉnh độ sáng đèn LED (0-100%)
        mcp_server.AddTool(
            "self.led.set_brightness",
            "Set brightness level of status LED (brightness: 0..100 percent)",
            PropertyList({
                Property("brightness", kPropertyTypeInteger, 50, 0, 100)
            }),
            [](const PropertyList& properties) -> ReturnValue {
                int pct = properties["brightness"].value<int>();
                if (pct < 0) pct = 0;
                if (pct > 100) pct = 100;
                uint8_t br = (uint8_t)(pct * 255 / 100);

                auto led = Board::GetInstance().GetLed();
                if (!led) {
                    nlohmann::json j;
                    j["error"] = "No status LED configured on this board";
                    return j;
                }

                led->SetBrightness(br);
                return true;
            }
        );

        // 5. Tool đọc trạng thái đèn LED hiện tại
        mcp_server.AddTool(
            "self.led.get_state",
            "Get current operational status, pixel count, custom mode, brightness and RGB color of the LED",
            PropertyList(),
            [](const PropertyList& properties) -> ReturnValue {
                auto led = Board::GetInstance().GetLed();
                if (!led) {
                    nlohmann::json j;
                    j["configured"] = false;
                    return j;
                }

                uint8_t r = 0, g = 0, b = 0;
                led->GetColor(r, g, b);
                nlohmann::json j;
                j["configured"] = true;
                j["type"] = led->GetType();
                j["count"] = led->GetLedCount();
                j["custom_mode"] = led->IsCustomMode();
                j["brightness"] = (static_cast<int>(led->GetBrightness()) * 100 + 127) / 255;
                j["brightness_raw"] = led->GetBrightness();
                j["color"]["r"] = r;
                j["color"]["g"] = g;
                j["color"]["b"] = b;
                return j;
            }
        );

        ESP_LOGI(TAG_LED_MCP, "LedMcpController registered 5 tools: set_effect, set_color, set_pixel, set_brightness, get_state");
    }
};

#endif // __LED_MCP_CONTROLLER_H__
