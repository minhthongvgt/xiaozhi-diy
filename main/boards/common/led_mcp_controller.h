#ifndef __LED_MCP_CONTROLLER_H__
#define __LED_MCP_CONTROLLER_H__

#include "mcp_server.h"
#include "board.h"
#include "led/single_led.h"
#include "led/circular_strip.h"
#include "led/gpio_led.h"
#include <esp_log.h>
#include <string>

#define TAG_LED_MCP "LedMcpController"

class LedMcpController {
public:
    LedMcpController() {
        auto& mcp_server = McpServer::GetInstance();

        // 1. Tool điều khiển hiệu ứng ánh sáng LED RGB
        mcp_server.AddTool(
            "self.led.set_effect",
            "Set lighting dynamic effect on status LED strip (supported effects: 'rainbow', 'chase', 'breathe', 'blink', 'off')",
            PropertyList({
                Property("effect", kPropertyTypeString),
                Property("speed_ms", kPropertyTypeInteger, 25, 10, 1000)
            }),
            [](const PropertyList& properties) -> ReturnValue {
                std::string effect = properties["effect"].value<std::string>();
                int speed = properties["speed_ms"].value<int>();

                auto led = Board::GetInstance().GetLed();
                if (!led) {
                    return "{\"error\": \"No status LED configured on this board\"}";
                }

                auto strip = dynamic_cast<CircularStrip*>(led);
                if (strip) {
                    if (effect == "rainbow") {
                        strip->Rainbow(speed);
                    } else if (effect == "chase") {
                        strip->RainbowChase(speed);
                    } else if (effect == "breathe") {
                        strip->Breathe({0, 0, 0}, {32, 32, 32}, speed);
                    } else if (effect == "blink") {
                        strip->Blink({32, 32, 32}, speed);
                    } else if (effect == "off") {
                        strip->TurnOff();
                    } else {
                        return "{\"error\": \"Unsupported effect for CircularStrip\"}";
                    }
                    return true;
                }

                auto single = dynamic_cast<SingleLed*>(led);
                if (single) {
                    if (effect == "rainbow") {
                        single->StartRainbow(speed);
                    } else if (effect == "blink") {
                        single->StartContinuousBlink(speed);
                    } else if (effect == "off") {
                        single->TurnOff();
                    } else {
                        return "{\"error\": \"Unsupported effect for SingleLed\"}";
                    }
                    return true;
                }

                auto gpio_led = dynamic_cast<GpioLed*>(led);
                if (gpio_led) {
                    if (effect == "off") {
                        gpio_led->TurnOff();
                    } else {
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

                return true;
            }
        );

        ESP_LOGI(TAG_LED_MCP, "LedMcpController registered tools: self.led.set_effect, self.led.set_color");
    }
};

#endif // __LED_MCP_CONTROLLER_H__
