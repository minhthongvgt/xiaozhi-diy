import sys, re, os
sys.stdout.reconfigure(encoding='utf-8')

# Check input_manager.c for phantom macros
txt = open('main/drivers/input/input_manager.c', encoding='utf-8').read()
lines = txt.splitlines()
print('=== input_manager.c encoder references ===')
for i, l in enumerate(lines, 1):
    if 'ROTARY' in l or 'ENCODER' in l:
        print(f'input_manager.c:{i}: {l.strip()}')

print()
# Check mcp_server.cc for MCP tools registered
txt_mcp = open('main/mcp_server.cc', encoding='utf-8').read()
print('=== MCP tool names (AddTool / GetName) ===')
for i, l in enumerate(txt_mcp.splitlines(), 1):
    if 'AddTool' in l or '.name =' in l:
        print(f'mcp_server.cc:{i}: {l.strip()}')

print()
# Check sensor_manager.c for phantom macros
txt_sm = open('main/drivers/sensor/sensor_manager.c', encoding='utf-8').read()
print('=== sensor_manager.c phantom macros ===')
for i, l in enumerate(txt_sm.splitlines(), 1):
    if 'CONFIG_CUSTOM' in l:
        print(f'sensor_manager.c:{i}: {l.strip()}')
