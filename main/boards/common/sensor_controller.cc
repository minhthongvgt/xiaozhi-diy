#include "sensor_controller.h"
#include "drivers/sensor/sensor_manager.h"
#include <esp_log.h>
#include <cstdio>

#define TAG "SensorController"

SensorController::SensorController() {
    auto& mcp_server = McpServer::GetInstance();

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_DHT11_22) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_I2C_TEMP_HUMID) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_BMP280) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_BH1750) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_LDR) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_GAS_CO2)
    // 1. Tool đọc thông số môi trường (Nhiệt độ, độ ẩm, áp suất, ánh sáng, CO2)
    mcp_server.AddTool("self.sensor.get_environment",
                       "Get current environmental sensor readings (temperature, humidity, light lux, barometric pressure, CO2). Field 'valid' indicates real sensor data vs no sensor connected.",
                       PropertyList(),
                       [](const PropertyList& properties) -> ReturnValue {
        sensor_environment_t env;
        sensor_read_environment(&env);
        char buf[340];
        snprintf(buf, sizeof(buf),
                 "{\"temperature\": %.1f, \"humidity\": %.1f, \"light_lux\": %.1f, \"pressure_hpa\": %.1f, \"co2_ppm\": %.0f, \"tvoc_ppb\": %.0f, \"valid\": %s}",
                 env.temperature_c, env.humidity_pct, env.light_lux, env.pressure_hpa, env.co2_ppm, env.tvoc_ppb,
                 env.valid ? "true" : "false");
        return std::string(buf);
    });
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_HCSR04)
    // 2. Tool đọc khoảng cách vật cản (ToF Laser / Ultrasonic)
    mcp_server.AddTool("self.sensor.get_distance",
                       "Get distance to nearest obstacle using Laser ToF (mm). Field 'valid' is false when no sensor is connected.",
                       PropertyList(),
                       [](const PropertyList& properties) -> ReturnValue {
        sensor_distance_t dist;
        sensor_read_distance(&dist);
        char buf[220];
        snprintf(buf, sizeof(buf),
                 "{\"laser_distance_mm\": %.1f, \"ultrasonic_distance_cm\": %.1f, \"valid\": %s}",
                 dist.distance_laser_mm, dist.distance_ultrasonic_cm,
                 dist.valid ? "true" : "false");
        return std::string(buf);
    });
#endif

#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_PIR) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_VIBRATION_SW420) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_FLAME) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_GAS_ANALOG_MQ)
    // 3. Tool kiểm tra an ninh & cảnh báo (PIR chuyển động, Rung, Lửa, Khí gas)
    mcp_server.AddTool("self.sensor.get_security",
                       "Check safety and security sensors (PIR motion, vibration, flame, gas leak). Field 'valid' is false when no sensor is connected.",
                       PropertyList(),
                       [](const PropertyList& properties) -> ReturnValue {
        sensor_security_t sec;
        sensor_read_security(&sec);
        char buf[300];
        snprintf(buf, sizeof(buf),
                 "{\"motion_detected\": %s, \"vibration_detected\": %s, \"flame_detected\": %s, \"gas_ppm\": %.1f, \"gas_leak_alert\": %s, \"valid\": %s}",
                 sec.motion_detected ? "true" : "false",
                 sec.vibration_detected ? "true" : "false",
                 sec.flame_detected ? "true" : "false",
                 sec.gas_level_ppm,
                 sec.gas_leak_alert ? "true" : "false",
                 sec.valid ? "true" : "false");
        return std::string(buf);
    });
#endif

#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_BATTERY_CHARGING_DETECT) || \
    defined(CONFIG_CUSTOM_ENABLE_SENSOR_INA2XX)
    // 4. Tool đọc thông số nguồn điện & dung lượng Pin
    mcp_server.AddTool("self.sensor.get_power",
                       "Get battery charging status (TP4056 CHRG pin). Note: battery_voltage/percentage/current require dedicated ADC monitor hardware. Field 'valid' is false when no power sensor is connected.",
                       PropertyList(),
                       [](const PropertyList& properties) -> ReturnValue {
        sensor_power_t pwr;
        sensor_read_power(&pwr);
        char buf[260];
        snprintf(buf, sizeof(buf),
                 "{\"battery_voltage\": %.2f, \"battery_percentage\": %d, \"is_charging\": %s, \"current_ma\": %.1f, \"power_mw\": %.1f, \"valid\": %s}",
                 pwr.battery_voltage, pwr.battery_percentage,
                 pwr.is_charging ? "true" : "false",
                 pwr.bus_current_ma, pwr.bus_power_mw,
                 pwr.valid ? "true" : "false");
        return std::string(buf);
    });
#endif

    ESP_LOGI(TAG, "SensorController registered active MCP tools based on user config");
}
