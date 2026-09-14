#include "actuator_controller.h"
#include "drivers/actuator/actuator_manager.h"
#include <esp_log.h>

#define TAG "ActuatorController"

ActuatorController::ActuatorController() {
    auto& mcp_server = McpServer::GetInstance();

    // 1. Tool điều khiển góc Servo
    PropertyList servo_props = {
        Property("angle", kPropertyTypeInteger, 90, 0, 180)
    };
    mcp_server.AddTool("self.actuator.set_servo",
                       "Set interactive servo angle between 0 and 180 degrees",
                       servo_props,
                       [](const PropertyList& properties) -> ReturnValue {
        int angle = properties["angle"].value<int>();
        actuator_set_servo_angle((uint8_t)angle);
        return true;
    });

    // 2. Tool điều khiển động cơ DC (TB6612FNG)
    PropertyList motor_props = {
        Property("motor_id", kPropertyTypeInteger, 1, 1, 2),
        Property("speed", kPropertyTypeInteger, 0, -100, 100)
    };
    mcp_server.AddTool("self.actuator.set_motor",
                       "Set DC motor speed (-100 to 100 percent) for motor 1 or 2",
                       motor_props,
                       [](const PropertyList& properties) -> ReturnValue {
        int motor_id = properties["motor_id"].value<int>();
        int speed = properties["speed"].value<int>();
        actuator_set_dc_motor(motor_id, speed);
        return true;
    });

    // 3. Tool phát âm thanh còi Buzzer
    PropertyList beep_props = {
        Property("duration_ms", kPropertyTypeInteger, 100, 10, 5000),
        Property("frequency_hz", kPropertyTypeInteger, 2000, 100, 10000)
    };
    mcp_server.AddTool("self.actuator.beep",
                       "Play a buzzer alarm or beep sound with specified frequency and duration",
                       beep_props,
                       [](const PropertyList& properties) -> ReturnValue {
        int duration = properties["duration_ms"].value<int>();
        int freq = properties["frequency_hz"].value<int>();
        actuator_beep((uint32_t)freq, (uint32_t)duration);
        return true;
    });

    // 4. Tool rung xúc giác (Haptic vibration)
    PropertyList vib_props = {
        Property("duration_ms", kPropertyTypeInteger, 200, 10, 3000)
    };
    mcp_server.AddTool("self.actuator.vibrate",
                       "Trigger haptic vibration motor feedback",
                       vib_props,
                       [](const PropertyList& properties) -> ReturnValue {
        int duration = properties["duration_ms"].value<int>();
        actuator_vibrate((uint32_t)duration);
        return true;
    });

    // 5. Tool bật/tắt rơ-le 220V
    PropertyList relay_props = {
        Property("state", kPropertyTypeBoolean, true)
    };
    mcp_server.AddTool("self.actuator.set_relay",
                       "Turn external 220V power relay ON or OFF",
                       relay_props,
                       [](const PropertyList& properties) -> ReturnValue {
        bool state = properties["state"].value<bool>();
        actuator_set_relay(state);
        return true;
    });

    mcp_server.AddTool("self.actuator.get_relay",
                       "Get the current power state of the 220V relay",
                       PropertyList(),
                       [](const PropertyList& properties) -> ReturnValue {
        return actuator_get_relay() ? "{\"state\": true}" : "{\"state\": false}";
    });

    ESP_LOGI(TAG, "ActuatorController registered 6 MCP tools: set_servo, set_motor, beep, vibrate, set_relay, get_relay");
}
