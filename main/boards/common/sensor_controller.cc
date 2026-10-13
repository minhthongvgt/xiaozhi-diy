#include "sensor_controller.h"
#include "drivers/sensor/sensor_manager.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <string>

#define TAG "SensorController"

namespace {

[[maybe_unused]] std::string JoinFields(std::initializer_list<const char*> fields) {
    std::string out;
    for (const char* field : fields) {
        if (field[0] == '\0') {
            continue;
        }
        if (!out.empty()) {
            out += ", ";
        }
        out += field;
    }
    return out;
}

[[maybe_unused]] std::string ToolDescription(const char* purpose, const std::string& fields) {
    return std::string(purpose) + " Possible fields: " + fields +
           ". The result only contains fields that the installed sensors actually measured this time. A missing field "
           "means that quantity is not measured on this device: never guess or invent a value for it. 'valid' is false "
           "when no sensor returned a reading.";
}

[[maybe_unused]] nlohmann::json NoReading() {
    nlohmann::json j;
    j["valid"] = false;
    return j;
}

[[maybe_unused]] void RegisterEnvironmentTool(McpServer& mcp_server) {
    const std::string fields = JoinFields({
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_DHT11_22) || defined(CONFIG_CUSTOM_ENABLE_SENSOR_AHT20) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_BMP280) || defined(CONFIG_CUSTOM_SENSOR_GAS_SCD4X)
        "temperature_c",
#endif
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_DHT11_22) || defined(CONFIG_CUSTOM_ENABLE_SENSOR_AHT20) || \
    defined(CONFIG_CUSTOM_SENSOR_GAS_SCD4X)
        "humidity_pct",
#endif
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_BMP280)
        "pressure_hpa",
#endif
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_BH1750) || defined(CONFIG_CUSTOM_ENABLE_SENSOR_LDR) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_APDS9960)
        "light_lux",
#endif
#if defined(CONFIG_CUSTOM_SENSOR_GAS_SCD4X)
        "co2_ppm",
#endif
        ""});
    mcp_server.AddTool("self.sensor.get_environment",
                       ToolDescription("Read the environmental sensors installed on this device.", fields),
                       PropertyList(), [](const PropertyList& properties) -> ReturnValue {
        sensor_environment_t env;
        sensor_read_environment(&env);
        if (!env.valid) {
            return NoReading();
        }
        nlohmann::json j;
        j["valid"] = true;
        if (env.has_temperature) j["temperature_c"] = env.temperature_c;
        if (env.has_humidity) j["humidity_pct"] = env.humidity_pct;
        if (env.has_pressure) j["pressure_hpa"] = env.pressure_hpa;
        if (env.has_light) j["light_lux"] = env.light_lux;
        if (env.has_co2) j["co2_ppm"] = env.co2_ppm;
        return j;
    });
}

[[maybe_unused]] void RegisterDistanceTool(McpServer& mcp_server) {
    const std::string fields = JoinFields({
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_VL53LX)
        "laser_distance_mm (VL53L0X/VL53L1X)",
#endif
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_HCSR04)
        "ultrasonic_distance_cm (HC-SR04)",
#endif
        ""});
    mcp_server.AddTool("self.sensor.get_distance",
                       ToolDescription("Measure the distance to the nearest obstacle.", fields),
                       PropertyList(), [](const PropertyList& properties) -> ReturnValue {
        sensor_distance_t dist;
        sensor_read_distance(&dist);
        if (!dist.valid) {
            return NoReading();
        }
        nlohmann::json j;
        j["valid"] = true;
        if (dist.has_laser) j["laser_distance_mm"] = dist.distance_laser_mm;
        if (dist.has_ultrasonic) j["ultrasonic_distance_cm"] = dist.distance_ultrasonic_cm;
        return j;
    });
}

[[maybe_unused]] void RegisterSecurityTool(McpServer& mcp_server) {
    const std::string fields = JoinFields({
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_PIR)
        "motion_detected (PIR)",
#endif
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_VIBRATION_SW420)
        "vibration_detected (SW-420)",
#endif
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_FLAME)
        "flame_detected (flame sensor)",
#endif
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_GAS_ANALOG_MQ)
        "gas_ppm, gas_leak_alert (MQ gas sensor)",
#endif
        ""});
    mcp_server.AddTool("self.sensor.get_security",
                       ToolDescription("Check the safety and security sensors installed on this device.", fields),
                       PropertyList(), [](const PropertyList& properties) -> ReturnValue {
        sensor_security_t sec;
        sensor_read_security(&sec);
        if (!sec.valid) {
            return NoReading();
        }
        nlohmann::json j;
        j["valid"] = true;
        if (sec.has_motion) j["motion_detected"] = sec.motion_detected;
        if (sec.has_vibration) j["vibration_detected"] = sec.vibration_detected;
        if (sec.has_flame) j["flame_detected"] = sec.flame_detected;
        if (sec.has_gas) {
            j["gas_ppm"] = sec.gas_level_ppm;
            j["gas_leak_alert"] = sec.gas_leak_alert;
        }
        return j;
    });
}

[[maybe_unused]] void RegisterPowerTool(McpServer& mcp_server) {
    const std::string fields = JoinFields({
#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_BATTERY_CHARGING_DETECT) || defined(CONFIG_CUSTOM_BATTERY_MONITOR_BQ27220)
        "is_charging",
#endif
#if defined(CONFIG_CUSTOM_BATTERY_MONITOR_BQ27220) || defined(CONFIG_CUSTOM_ENABLE_SENSOR_INA2XX)
        "battery_voltage (V), current_ma, power_mw",
#endif
#if defined(CONFIG_CUSTOM_BATTERY_MONITOR_BQ27220)
        "battery_percentage",
#endif
        ""});
    mcp_server.AddTool("self.sensor.get_power",
                       ToolDescription("Read the battery and power sensors installed on this device.", fields),
                       PropertyList(), [](const PropertyList& properties) -> ReturnValue {
        sensor_power_t pwr;
        sensor_read_power(&pwr);
        if (!pwr.valid) {
            return NoReading();
        }
        nlohmann::json j;
        j["valid"] = true;
        if (pwr.has_charging) j["is_charging"] = pwr.is_charging;
        if (pwr.has_voltage) j["battery_voltage"] = pwr.battery_voltage;
        if (pwr.has_current) {
            j["current_ma"] = pwr.bus_current_ma;
            j["power_mw"] = pwr.bus_power_mw;
        }
        if (pwr.has_percentage) j["battery_percentage"] = pwr.battery_percentage;
        return j;
    });
}

[[maybe_unused]] void RegisterApds9960Tools(McpServer& mcp_server) {
    mcp_server.AddTool("self.sensor.get_proximity_color",
                       "Read the APDS-9960 sensor: proximity (0 = nothing near, 255 = object very close), raw "
                       "clear/red/green/blue light counts and estimated light_lux. Use it to tell whether something is in "
                       "front of the device or to describe the dominant light color. 'valid' is false when the sensor did "
                       "not return a reading.",
                       PropertyList(), [](const PropertyList& properties) -> ReturnValue {
        sensor_color_t c;
        if (sensor_read_proximity_color(&c) != ESP_OK) {
            return NoReading();
        }
        nlohmann::json j;
        j["valid"] = true;
        j["proximity"] = c.proximity;
        j["clear"] = c.clear;
        j["red"] = c.red;
        j["green"] = c.green;
        j["blue"] = c.blue;
        j["light_lux"] = c.light_lux;
        return j;
    });
    mcp_server.AddTool("self.sensor.read_gesture",
                       "Wait for a hand swipe over the APDS-9960 sensor and return its direction: up, down, left, right, "
                       "or none if no gesture happened before the timeout. Tell the user to swipe a hand about 5-10 cm "
                       "above the sensor before calling this tool.",
                       PropertyList({Property("timeout_ms", kPropertyTypeInteger, 5000, 500, 10000)}),
                       [](const PropertyList& properties) -> ReturnValue {
        int timeout_ms = properties["timeout_ms"].value<int>();
        const char* gesture = "none";
        esp_err_t ret = sensor_read_gesture(static_cast<uint32_t>(timeout_ms), &gesture);
        nlohmann::json j;
        j["valid"] = (ret == ESP_OK);
        j["gesture"] = gesture;
        return j;
    });
}

}

SensorController::SensorController() {
    [[maybe_unused]] auto& mcp_server = McpServer::GetInstance();

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_DHT11_22) || defined(CONFIG_CUSTOM_ENABLE_SENSOR_AHT20) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_BMP280) || defined(CONFIG_CUSTOM_ENABLE_SENSOR_BH1750) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_LDR) || defined(CONFIG_CUSTOM_SENSOR_GAS_SCD4X) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_APDS9960)
    if (sensor_environment_available()) {
        RegisterEnvironmentTool(mcp_server);
    } else {
        ESP_LOGW(TAG, "Environment sensors enabled but none detected, MCP tool not registered");
    }
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_HCSR04) || defined(CONFIG_CUSTOM_ENABLE_SENSOR_VL53LX)
    if (sensor_distance_available()) {
        RegisterDistanceTool(mcp_server);
    } else {
        ESP_LOGW(TAG, "Distance sensors enabled but none detected, MCP tool not registered");
    }
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_PIR) || defined(CONFIG_CUSTOM_ENABLE_SENSOR_VIBRATION_SW420) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_FLAME) || defined(CONFIG_CUSTOM_ENABLE_SENSOR_GAS_ANALOG_MQ)
    if (sensor_security_available()) {
        RegisterSecurityTool(mcp_server);
    } else {
        ESP_LOGW(TAG, "Security sensors enabled but no pin configured, MCP tool not registered");
    }
#endif

#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_BATTERY_CHARGING_DETECT) || defined(CONFIG_CUSTOM_ENABLE_SENSOR_INA2XX) || \
    defined(CONFIG_CUSTOM_BATTERY_MONITOR_BQ27220)
    if (sensor_power_available()) {
        RegisterPowerTool(mcp_server);
    } else {
        ESP_LOGW(TAG, "Power sensors enabled but none detected, MCP tool not registered");
    }
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_APDS9960)
    if (sensor_color_available()) {
        RegisterApds9960Tools(mcp_server);
    } else {
        ESP_LOGW(TAG, "APDS-9960 enabled but not detected, MCP tools not registered");
    }
#endif
}
