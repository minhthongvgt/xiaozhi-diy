#ifndef __ROBOT_MCP_CONTROLLER_H__
#define __ROBOT_MCP_CONTROLLER_H__

#include "mcp_server.h"
#include "drivers/actuator/actuator_manager.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>

#define TAG_ROBOT_MCP "RobotMcpController"

class RobotMcpController {
public:
    RobotMcpController() {
        auto& mcp_server = McpServer::GetInstance();

#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE)
        // 1. Tool điều khiển xe robot di chuyển
        mcp_server.AddTool(
            "self.robot.move",
            "Control robot car movement (directions: 'forward', 'backward', 'left', 'right', 'stop')",
            PropertyList({
                Property("direction", kPropertyTypeString),
                Property("speed", kPropertyTypeInteger, 60, 0, 100),
                Property("duration_ms", kPropertyTypeInteger, 1000, 100, 5000)
            }),
            [](const PropertyList& properties) -> ReturnValue {
                std::string dir = properties["direction"].value<std::string>();
                int speed = properties["speed"].value<int>();
                int duration = properties["duration_ms"].value<int>();

                if (dir == "forward") {
                    actuator_set_dc_motor(1, speed);
                    actuator_set_dc_motor(2, speed);
                } else if (dir == "backward") {
                    actuator_set_dc_motor(1, -speed);
                    actuator_set_dc_motor(2, -speed);
                } else if (dir == "left") {
                    actuator_set_dc_motor(1, -speed);
                    actuator_set_dc_motor(2, speed);
                } else if (dir == "right") {
                    actuator_set_dc_motor(1, speed);
                    actuator_set_dc_motor(2, -speed);
                } else {
                    actuator_set_dc_motor(1, 0);
                    actuator_set_dc_motor(2, 0);
                    return true;
                }

                vTaskDelay(pdMS_TO_TICKS(duration));
                actuator_set_dc_motor(1, 0);
                actuator_set_dc_motor(2, 0);
                return true;
            }
        );
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SERVO_DOG)
        // 2. Tool thực hiện cử chỉ cảm xúc của robot (gật đầu, lắc đầu, vẫy, thả lỏng)
        mcp_server.AddTool(
            "self.robot.gesture",
            "Perform robot interactive physical gesture (gestures: 'nod', 'shake', 'center', 'relax')",
            PropertyList({
                Property("action", kPropertyTypeString)
            }),
            [](const PropertyList& properties) -> ReturnValue {
                std::string action = properties["action"].value<std::string>();

                if (action == "nod") {
                    actuator_set_servo_angle(60);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    actuator_set_servo_angle(120);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    actuator_set_servo_angle(90);
                } else if (action == "shake") {
                    actuator_set_servo_angle(45);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    actuator_set_servo_angle(135);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    actuator_set_servo_angle(90);
                } else if (action == "relax" || action == "detach") {
                    actuator_servo_detach();
                } else {
                    actuator_set_servo_angle(90);
                }
                return true;
            }
        );
#endif

        ESP_LOGI(TAG_ROBOT_MCP, "RobotMcpController registered active MCP tools based on user config");
    }
};

#endif // __ROBOT_MCP_CONTROLLER_H__
