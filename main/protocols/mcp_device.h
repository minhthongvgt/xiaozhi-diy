/**
 * @file mcp_device.h
 * @brief Device-side Model Context Protocol (MCP) Execution Interface
 */

#ifndef MCP_DEVICE_H
#define MCP_DEVICE_H

#include <esp_err.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Device-side MCP tool handlers
 */
esp_err_t mcp_device_init(void);

/**
 * @brief Execute an MCP tool invocation on hardware
 *
 * @param tool_name Name of the tool (e.g., "turn_on_lamp", "control_relay", "get_sensor_data")
 * @param params_json JSON string of tool parameters
 * @param[out] result_buf Buffer to store execution result JSON
 * @param result_buf_sz Size of result_buf
 * @return ESP_OK on success
 */
esp_err_t mcp_device_execute_tool(const char *tool_name, const char *params_json, char *result_buf, size_t result_buf_sz);

#ifdef __cplusplus
}
#endif

#endif // MCP_DEVICE_H
