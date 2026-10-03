#ifndef MCP_DEVICE_H
#define MCP_DEVICE_H
#include <esp_err.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
esp_err_t mcp_device_init(void);
esp_err_t mcp_device_execute_tool(const char *tool_name, const char *params_json, char *result_buf, size_t result_buf_sz);
#ifdef __cplusplus
}
#endif
#endif 
