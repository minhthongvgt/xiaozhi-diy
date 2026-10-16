#include "actuator_controller.h"
#include "drivers/actuator/actuator_manager.h"
#include <esp_log.h>

#define TAG "ActuatorController"

ActuatorController::ActuatorController() {
  [[maybe_unused]] auto &mcp_server = McpServer::GetInstance();

#if defined(CONFIG_CUSTOM_ENABLE_SERVO_DOG)
  if (actuator_has_servo()) {
    mcp_server.AddTool(
        "self.actuator.set_servo",
        "Set interactive servo angle between 0 and 180 degrees",
        PropertyList({Property("angle", kPropertyTypeInteger, 90, 0, 180)}),
        [](const PropertyList &properties) -> ReturnValue {
          int angle = properties["angle"].value<int>();
          actuator_set_servo_angle((uint8_t)angle);
          return true;
        });

    mcp_server.AddTool("self.actuator.get_servo",
                       "Get the current angle (0 to 180 degrees) and active "
                       "state of the interactive servo",
                       PropertyList(),
                       [](const PropertyList &properties) -> ReturnValue {
                         nlohmann::json j;
                         j["angle"] = actuator_get_servo_angle();
                         j["attached"] = actuator_is_servo_attached();
                         return j;
                       });

    mcp_server.AddTool("self.actuator.detach_servo",
                       "Detach servo PWM signal to release holding torque and "
                       "prevent jitter/overheating",
                       PropertyList(),
                       [](const PropertyList &properties) -> ReturnValue {
                         actuator_servo_detach();
                         return true;
                       });
  } else {
    ESP_LOGW(TAG, "Servo enabled but PWM pin not configured, MCP tools not registered");
  }
#endif

#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE)
  if (actuator_has_motor(1) || actuator_has_motor(2)) {
    const int first_motor = actuator_has_motor(1) ? 1 : 2;
    const int last_motor = actuator_has_motor(2) ? 2 : 1;
    mcp_server.AddTool(
        "self.actuator.set_motor",
        last_motor == first_motor
            ? "Set DC motor speed (-100 to 100 percent). Only motor " + std::to_string(first_motor) + " is connected."
            : std::string("Set DC motor speed (-100 to 100 percent) for motor 1 or 2"),
        PropertyList({Property("motor_id", kPropertyTypeInteger, first_motor, first_motor, last_motor),
                      Property("speed", kPropertyTypeInteger, 0, -100, 100)}),
        [](const PropertyList &properties) -> ReturnValue {
          int motor_id = properties["motor_id"].value<int>();
          int speed = properties["speed"].value<int>();
          actuator_set_dc_motor(motor_id, speed);
          return true;
        });
  } else {
    ESP_LOGW(TAG, "DC motor enabled but PWM/DIR pins not configured, MCP tool not registered");
  }
#endif

#if defined(CONFIG_ENABLE_BUZZER)
  if (actuator_has_buzzer()) {
#if defined(CONFIG_BUZZER_TYPE_PASSIVE)
    constexpr const char *kBeepDescription =
        "Play a buzzer tone with the specified frequency and duration";
#else
    constexpr const char *kBeepDescription =
        "Play a buzzer beep for the specified duration; the buzzer has a fixed pitch, so frequency_hz has no effect";
#endif
    mcp_server.AddTool(
        "self.actuator.beep",
        kBeepDescription,
        PropertyList(
            {Property("duration_ms", kPropertyTypeInteger, 100, 10, 5000),
             Property("frequency_hz", kPropertyTypeInteger, 2000, 100, 10000)}),
        [](const PropertyList &properties) -> ReturnValue {
          int duration = properties["duration_ms"].value<int>();
          int freq = properties["frequency_hz"].value<int>();
          actuator_beep((uint32_t)freq, (uint32_t)duration);
          return true;
        });
  } else {
    ESP_LOGW(TAG, "Buzzer enabled but pin not configured, MCP tool not registered");
  }
#endif

#if defined(CONFIG_ENABLE_HAPTIC_MOTOR)
  if (actuator_has_haptic()) {
    mcp_server.AddTool("self.actuator.vibrate",
                       "Trigger haptic vibration motor feedback",
                       PropertyList({Property("duration_ms", kPropertyTypeInteger,
                                              200, 10, 3000)}),
                       [](const PropertyList &properties) -> ReturnValue {
                         int duration = properties["duration_ms"].value<int>();
                         actuator_vibrate((uint32_t)duration);
                         return true;
                       });
  } else {
    ESP_LOGW(TAG, "Haptic motor enabled but pin not configured, MCP tool not registered");
  }
#endif

#if defined(CONFIG_CUSTOM_PERIPH_RELAY_ENABLE)
  if (actuator_has_relay()) {
    mcp_server.AddTool(
        "self.actuator.set_relay", "Turn external 220V power relay ON or OFF",
        PropertyList({Property("state", kPropertyTypeBoolean, true)}),
        [](const PropertyList &properties) -> ReturnValue {
          bool state = properties["state"].value<bool>();
          actuator_set_relay(state);
          return true;
        });

    mcp_server.AddTool("self.actuator.get_relay",
                       "Get the current power state of the 220V relay",
                       PropertyList(),
                       [](const PropertyList &properties) -> ReturnValue {
                         nlohmann::json j;
                         j["state"] = actuator_get_relay();
                         return j;
                       });
  } else {
    ESP_LOGW(TAG, "Relay enabled but pin not configured, MCP tools not registered");
  }
#endif
}
