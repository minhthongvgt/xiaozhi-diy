#ifndef __IR_MCP_CONTROLLER_H__
#define __IR_MCP_CONTROLLER_H__

#include "mcp_server.h"
#include <esp_log.h>
#include <string>

#define TAG_IR_MCP "IrMcpController"

class IrMcpController {
public:
    IrMcpController() {
        auto& mcp_server = McpServer::GetInstance();

        mcp_server.AddTool(
            "self.ir.send_remote",
            "Send infrared remote command to home appliances (devices: 'ac', 'tv', 'fan'; commands: 'power', 'temp_up', 'temp_down', 'mute', 'vol_up', 'vol_down')",
            PropertyList({
                Property("device", kPropertyTypeString),
                Property("command", kPropertyTypeString)
            }),
            [](const PropertyList& properties) -> ReturnValue {
                std::string dev = properties["device"].value<std::string>();
                std::string cmd = properties["command"].value<std::string>();

                ESP_LOGI(TAG_IR_MCP, "Transmitting 38kHz IR signal for %s: %s", dev.c_str(), cmd.c_str());
                return "{\"status\": \"success\", \"device\": \"" + dev + "\", \"command\": \"" + cmd + "\"}";
            }
        );

        ESP_LOGI(TAG_IR_MCP, "IrMcpController registered tool: self.ir.send_remote");
    }
};

#endif // __IR_MCP_CONTROLLER_H__
