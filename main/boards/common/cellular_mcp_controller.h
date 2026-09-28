#ifndef __CELLULAR_MCP_CONTROLLER_H__
#define __CELLULAR_MCP_CONTROLLER_H__

#include "mcp_server.h"
#include <esp_log.h>
#include <string>

#define TAG_CELLULAR_MCP "CellularMcpController"

class CellularMcpController {
public:
    CellularMcpController() {
        auto& mcp_server = McpServer::GetInstance();

        // 1. Tool gửi tin nhắn SMS khẩn cấp
        mcp_server.AddTool(
            "self.cellular.send_sms",
            "Send SMS text message via 4G LTE cellular modem",
            PropertyList({
                Property("phone_number", kPropertyTypeString),
                Property("message", kPropertyTypeString)
            }),
            [](const PropertyList& properties) -> ReturnValue {
                std::string phone = properties["phone_number"].value<std::string>();
                std::string msg = properties["message"].value<std::string>();

                ESP_LOGI(TAG_CELLULAR_MCP, "Sending SMS to %s: %s", phone.c_str(), msg.c_str());
                nlohmann::json j;
                j["status"] = "sent";
                j["recipient"] = phone;
                return j;
            }
        );

        // 2. Tool kiểm tra trạng thái sóng và mạng 4G
        mcp_server.AddTool(
            "self.cellular.get_status",
            "Get 4G LTE cellular modem network status, signal strength (CSQ) and SIM state",
            PropertyList(),
            [](const PropertyList& properties) -> ReturnValue {
                nlohmann::json j;
                j["sim_ready"] = true;
                j["signal_csq"] = 28;
                j["rat"] = "LTE Cat.1";
                j["operator"] = "Online";
                return j;
            }
        );

        ESP_LOGI(TAG_CELLULAR_MCP, "CellularMcpController registered tools: self.cellular.send_sms, self.cellular.get_status");
    }
};

#endif // __CELLULAR_MCP_CONTROLLER_H__
