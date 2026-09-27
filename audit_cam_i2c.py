import sys, re, os
sys.stdout.reconfigure(encoding='utf-8')

txt = open('main/gpio_validator.c', encoding='utf-8').read()
print('=== Camera SIOD/SIOC registered as which BUS_TYPE? ===')
for i, l in enumerate(txt.splitlines(), 1):
    if 'SIOD' in l or 'SIOC' in l:
        print(f'{i}: {l.strip()}')

print()
txt_cfg = open('main/boards/esp32s3-n16r8-custom/config.h', encoding='utf-8').read()
cam_siod = re.search(r'define\s+CAM_PIN_SIOD\s+(\w+)', txt_cfg)
cam_sioc = re.search(r'define\s+CAM_PIN_SIOC\s+(\w+)', txt_cfg)
i2c_sda  = re.search(r'define\s+DISPLAY_I2C_SDA_PIN\s+(\w+)', txt_cfg)
i2c_scl  = re.search(r'define\s+DISPLAY_I2C_SCL_PIN\s+(\w+)', txt_cfg)
print('=== Matching defaults: CAM vs I2C ===')
print('CAM_PIN_SIOD default:  ' + (cam_siod.group(1) if cam_siod else '?'))
print('CAM_PIN_SIOC default:  ' + (cam_sioc.group(1) if cam_sioc else '?'))
print('DISPLAY_I2C_SDA_PIN:   ' + (i2c_sda.group(1) if i2c_sda else '?'))
print('DISPLAY_I2C_SCL_PIN:   ' + (i2c_scl.group(1) if i2c_scl else '?'))

print()
# Check if gpio_validator conflicts would detect this collision
# CAM_PIN_SIOD -> BUS_TYPE_EXCLUSIVE, I2C SDA -> BUS_TYPE_I2C_SDA -> collision!
print('CONCLUSION: If camera is enabled with default SIOD=8, SIOC=9, and I2C SDA=8, SCL=9,')
print('            gpio_validator.c will REPORT CONFLICT (EXCLUSIVE vs I2C_SDA), halting boot.')
print()
print('BUG: Camera SIOD/SIOC should be registered as BUS_TYPE_I2C_SDA/I2C_SCL in gpio_validator')
print('to allow SCCB (I2C-compatible) to share the bus with other I2C devices.')
