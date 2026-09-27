import sys, re
sys.stdout.reconfigure(encoding='utf-8')
txt = open('gpio_validator.c', encoding='utf-8').read()

replacements = {
    'CONFIG_CUSTOM_I2C_SDA_PIN': 'CONFIG_CUSTOM_SENSOR_I2C_SDA',
    'CONFIG_CUSTOM_I2C_SCL_PIN': 'CONFIG_CUSTOM_SENSOR_I2C_SCL',
    'CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_BCLK': 'CONFIG_CUSTOM_AUDIO_SPK_GPIO_BCLK',
    'CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_LRCK': 'CONFIG_CUSTOM_AUDIO_SPK_GPIO_LRCK',
    'CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_DOUT': 'CONFIG_CUSTOM_AUDIO_SPK_GPIO_DOUT',
    'CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_SCK': 'CONFIG_CUSTOM_AUDIO_MIC_GPIO_SCK',
    'CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_WS': 'CONFIG_CUSTOM_AUDIO_MIC_GPIO_WS',
    'CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_DIN': 'CONFIG_CUSTOM_AUDIO_MIC_GPIO_DIN',
    'CONFIG_CUSTOM_BUTTON_VOLUME_UP_GPIO': 'CONFIG_CUSTOM_BUTTON_VOL_UP_GPIO',
    'CONFIG_CUSTOM_BUTTON_VOLUME_DOWN_GPIO': 'CONFIG_CUSTOM_BUTTON_VOL_DOWN_GPIO',
    'CONFIG_CUSTOM_LED_WS2812_GPIO': 'CONFIG_CUSTOM_LED_GPIO',
    'CONFIG_CUSTOM_LED_SINGLE_PWM_GPIO': 'CONFIG_CUSTOM_LED_GPIO'
}

for k, v in replacements.items():
    if k != v:
        txt = txt.replace(k, v)
        old_str = '"' + k.replace('CONFIG_', '') + '"'
        new_str = '"' + v.replace('CONFIG_', '') + '"'
        txt = txt.replace(old_str, new_str)

blocks = txt.split('#if defined(')
new_blocks = []
seen = set()
for b in blocks:
    if ')' in b and b.startswith('CONFIG_'):
        macro = b.split(')')[0]
        if macro in seen:
            continue
        seen.add(macro)
    new_blocks.append(b)

txt = '#if defined('.join(new_blocks)

additions = '''
#if defined(CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_A)
        { CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_A, "CUSTOM_PERIPH_ENCODER_PHASE_A", BUS_TYPE_EXCLUSIVE, true },
#endif
#if defined(CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_B)
        { CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_B, "CUSTOM_PERIPH_ENCODER_PHASE_B", BUS_TYPE_EXCLUSIVE, true },
#endif
#if defined(CONFIG_CUSTOM_PERIPH_RC522_SPI_MISO)
        { CONFIG_CUSTOM_PERIPH_RC522_SPI_MISO, "CUSTOM_PERIPH_RC522_SPI_MISO", BUS_TYPE_SPI_MISO, true },
#endif
#if defined(CONFIG_CUSTOM_PERIPH_RC522_SPI_MOSI)
        { CONFIG_CUSTOM_PERIPH_RC522_SPI_MOSI, "CUSTOM_PERIPH_RC522_SPI_MOSI", BUS_TYPE_SPI_MOSI, true },
#endif
#if defined(CONFIG_CUSTOM_PERIPH_RC522_SPI_SCK)
        { CONFIG_CUSTOM_PERIPH_RC522_SPI_SCK, "CUSTOM_PERIPH_RC522_SPI_SCK", BUS_TYPE_SPI_SCK, true },
#endif
#if defined(CONFIG_CUSTOM_PERIPH_RC522_SPI_CS)
        { CONFIG_CUSTOM_PERIPH_RC522_SPI_CS, "CUSTOM_PERIPH_RC522_SPI_CS", BUS_TYPE_EXCLUSIVE, true },
#endif
#if defined(CONFIG_CUSTOM_PERIPH_SDCARD_SPI_MISO)
        { CONFIG_CUSTOM_PERIPH_SDCARD_SPI_MISO, "CUSTOM_PERIPH_SDCARD_SPI_MISO", BUS_TYPE_SPI_MISO, true },
#endif
#if defined(CONFIG_CUSTOM_PERIPH_SDCARD_SPI_MOSI)
        { CONFIG_CUSTOM_PERIPH_SDCARD_SPI_MOSI, "CUSTOM_PERIPH_SDCARD_SPI_MOSI", BUS_TYPE_SPI_MOSI, true },
#endif
#if defined(CONFIG_CUSTOM_PERIPH_SDCARD_SPI_SCK)
        { CONFIG_CUSTOM_PERIPH_SDCARD_SPI_SCK, "CUSTOM_PERIPH_SDCARD_SPI_SCK", BUS_TYPE_SPI_SCK, true },
#endif
#if defined(CONFIG_CUSTOM_PERIPH_SDCARD_SPI_CS)
        { CONFIG_CUSTOM_PERIPH_SDCARD_SPI_CS, "CUSTOM_PERIPH_SDCARD_SPI_CS", BUS_TYPE_EXCLUSIVE, true },
#endif
'''

idx = txt.find('// 5. Kết thúc mảng')
if idx != -1:
    txt = txt[:idx] + additions.strip() + '\n        ' + txt[idx:]

with open('gpio_validator.c', 'w', encoding='utf-8') as f:
    f.write(txt)
print("Updated gpio_validator.c")
