/**
 * @file mcp_device.c
 * @brief Device-side Model Context Protocol (MCP) Execution Implementation
 */

#include "mcp_device.h"
#include "drivers/actuator/actuator_manager.h"
#include "drivers/sensor/sensor_manager.h"
#include <esp_log.h>
#include <string.h>
#include <stdio.h>

#define TAG "McpDevice"

esp_err_t mcp_device_init(void)
{
    ESP_LOGI(TAG, "Initializing Device-side MCP Execution Subsystem...");
    return ESP_OK;
}

esp_err_t mcp_device_execute_tool(const char *tool_name, const char *params_json, char *result_buf, size_t result_buf_sz)
{
    if (!tool_name || !result_buf || result_buf_sz == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Executing MCP Tool: '%s' with params: %s", tool_name, params_json ? params_json : "{}");

    if (strcmp(tool_name, "control_relay") == 0 || strcmp(tool_name, "turn_on_lamp") == 0) {
        bool turn_on = (params_json && strstr(params_json, "\"state\": true")) ||
                       (params_json && strstr(params_json, "\"state\":true")) ||
                       (strcmp(tool_name, "turn_on_lamp") == 0);
        actuator_set_relay(turn_on);
        snprintf(result_buf, result_buf_sz, "{\"status\":\"success\",\"state\":%s}", turn_on ? "true" : "false");
        return ESP_OK;
    } else if (strcmp(tool_name, "get_sensor_data") == 0) {
        sensor_data_t data;
        sensor_manager_read_all(&data);
        snprintf(result_buf, result_buf_sz,
                 "{\"temp\":%.1f,\"humidity\":%.1f,\"light\":%.1f,\"co2\":%.1f,\"battery\":%.2f}",
                 data.temperature_c, data.humidity_pct, data.light_lux, data.co2_ppm, data.battery_voltage);
        return ESP_OK;
    } else if (strcmp(tool_name, "beep") == 0) {
        actuator_beep(2000, 200);
        snprintf(result_buf, result_buf_sz, "{\"status\":\"beep_success\"}");
        return ESP_OK;
    }

    snprintf(result_buf, result_buf_sz, "{\"status\":\"error\",\"message\":\"Unknown tool\"}");
    return ESP_ERR_NOT_FOUND;
}
