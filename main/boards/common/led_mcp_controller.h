#ifndef __LED_MCP_CONTROLLER_H__
#define __LED_MCP_CONTROLLER_H__

#include "mcp_server.h"
#include "board.h"
#include "led/single_led.h"
#include "led/circular_strip.h"
#include "led/gpio_led.h"
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
            "Set dynamic lighting effect on status LED strip (supported effects: 'rainbow', 'chase', 'breathe', 'blink', 'off', 'auto')",
            PropertyList({
                Property("effect", kPropertyTypeString),
                Property("speed_ms", kPropertyTypeInteger, 25, 10, 1000)
            }),
            [](const PropertyList& properties) -> ReturnValue {
                std::string effect = properties["effect"].value<std::string>();
                int speed = 25;
                if (properties.find("speed_ms") != properties.end()) {
                    speed = properties["speed_ms"].value<int>();
                }
                if (speed < 10) speed = 10;
                if (speed > 1000) speed = 1000;

                auto led = Board::GetInstance().GetLed();
                if (!led) {
                    return "{\"error\": \"No status LED configured on this board\"}";
                }

                // Handling auto / restore status mode
                if (effect == "auto" || effect == "status") {
                    led->SetCustomMode(false);
                    led->OnStateChanged();
                    return true;
                }

                auto strip = dynamic_cast<CircularStrip*>(led);
                if (strip) {
                    if (effect == "rainbow") {
                        led->SetCustomMode(true);
                        strip->Rainbow(speed);
                    } else if (effect == "chase") {
                        led->SetCustomMode(true);
                        strip->RainbowChase(speed);
                    } else if (effect == "breathe") {
                        led->SetCustomMode(true);
                        StripColor c = strip->GetColor();
                        if (c.red == 0 && c.green == 0 && c.blue == 0) {
                            c = { 32, 32, 32 };
                        }
                        strip->Breathe({0, 0, 0}, c, speed);
                    } else if (effect == "blink") {
                        led->SetCustomMode(true);
                        StripColor c = strip->GetColor();
                        if (c.red == 0 && c.green == 0 && c.blue == 0) {
                            c = { 32, 32, 32 };
                        }
                        strip->Blink(c, speed);
                    } else if (effect == "off") {
                        led->SetCustomMode(false);
                        strip->TurnOff();
                    } else {
                        return "{\"error\": \"Unsupported effect for CircularStrip: " + effect + "\"}";
                    }
                    return true;
                }

                auto single = dynamic_cast<SingleLed*>(led);
                if (single) {
                    if (effect == "rainbow") {
                        led->SetCustomMode(true);
                        single->StartRainbow(speed);
                    } else if (effect == "chase") {
                        led->SetCustomMode(true);
                        single->StartChase(speed);
                    } else if (effect == "breathe") {
                        led->SetCustomMode(true);
                        single->StartBreathe(speed, 32);
                    } else if (effect == "blink") {
                        led->SetCustomMode(true);
                        single->StartContinuousBlink(speed);
                    } else if (effect == "off") {
                        led->SetCustomMode(false);
                        single->TurnOff();
                    } else {
                        return "{\"error\": \"Unsupported effect for SingleLed: " + effect + "\"}";
                    }
                    return true;
                }

                auto gpio_led = dynamic_cast<GpioLed*>(led);
                if (gpio_led) {
                    if (effect == "off") {
                        led->SetCustomMode(false);
                        gpio_led->TurnOff();
                    } else if (effect == "blink") {
                        led->SetCustomMode(true);
                        gpio_led->StartContinuousBlink(speed);
                    } else if (effect == "breathe") {
                        led->SetCustomMode(true);
                        gpio_led->StartFadeTask();
                    } else {
                        led->SetCustomMode(true);
                        gpio_led->TurnOn();
                    }
                    return true;
                }

                return true;
            }
        );

        // 2. Tool đặt màu sắc tĩnh cho LED RGB
        mcp_server.AddTool(
            "self.led.set_color",
            "Set static RGB color for status LED (red: 0..255, green: 0..255, blue: 0..255)",
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
                    return "{\"error\": \"No status LED configured on this board\"}";
                }

                led->SetCustomMode(true);

                auto strip = dynamic_cast<CircularStrip*>(led);
                if (strip) {
                    strip->SetAllColor({r, g, b});
                    return true;
                }

                auto single = dynamic_cast<SingleLed*>(led);
                if (single) {
                    single->SetColor(r, g, b);
                    single->TurnOn();
                    return true;
                }

                auto gpio_led = dynamic_cast<GpioLed*>(led);
                if (gpio_led) {
                    uint8_t br = (uint8_t)(((uint32_t)r * 299 + (uint32_t)g * 587 + (uint32_t)b * 114) / 1000);
                    gpio_led->SetBrightness(br);
                    gpio_led->TurnOn();
                    return true;
                }

                return true;
            }
        );

        // 3. Tool điều chỉnh độ sáng đèn LED (0-100%)
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
                    return "{\"error\": \"No status LED configured on this board\"}";
                }

                auto strip = dynamic_cast<CircularStrip*>(led);
                if (strip) {
                    strip->SetBrightness(br, br > 4 ? 4 : br / 2);
                    return true;
                }

                auto single = dynamic_cast<SingleLed*>(led);
                if (single) {
                    single->SetBrightness(br);
                    return true;
                }

                auto gpio_led = dynamic_cast<GpioLed*>(led);
                if (gpio_led) {
                    gpio_led->SetBrightness(br);
                    return true;
                }

                return true;
            }
        );

        // 4. Tool đọc trạng thái đèn LED hiện tại
        mcp_server.AddTool(
            "self.led.get_state",
            "Get current operational status, custom mode, brightness and RGB color of the LED",
            PropertyList(),
            [](const PropertyList& properties) -> ReturnValue {
                auto led = Board::GetInstance().GetLed();
                if (!led) {
                    return "{\"configured\": false}";
                }

                char buf[220];
                auto strip = dynamic_cast<CircularStrip*>(led);
                if (strip) {
                    StripColor c = strip->GetColor();
                    snprintf(buf, sizeof(buf),
                             "{\"configured\": true, \"type\": \"CircularStrip\", \"custom_mode\": %s, \"brightness\": %d, \"color\": {\"r\": %d, \"g\": %d, \"b\": %d}}",
                             strip->IsCustomMode() ? "true" : "false",
                             strip->GetBrightness(), c.red, c.green, c.blue);
                    return std::string(buf);
                }

                auto single = dynamic_cast<SingleLed*>(led);
                if (single) {
                    snprintf(buf, sizeof(buf),
                             "{\"configured\": true, \"type\": \"SingleLed\", \"custom_mode\": %s}",
                             single->IsCustomMode() ? "true" : "false");
                    return std::string(buf);
                }

                auto gpio_led = dynamic_cast<GpioLed*>(led);
                if (gpio_led) {
                    snprintf(buf, sizeof(buf),
                             "{\"configured\": true, \"type\": \"GpioLed\", \"custom_mode\": %s}",
                             gpio_led->IsCustomMode() ? "true" : "false");
                    return std::string(buf);
                }

                return "{\"configured\": true, \"type\": \"GenericLed\"}";
            }
        );

        ESP_LOGI(TAG_LED_MCP, "LedMcpController registered 4 tools: set_effect, set_color, set_brightness, get_state");
    }
};

#endif // __LED_MCP_CONTROLLER_H__
