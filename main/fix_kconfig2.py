import re

def fix_kconfig():
    with open('Kconfig.projbuild', 'r', encoding='utf-8') as f:
        txt = f.read()

    # Core pins that we want to keep their actual default values
    # because they are either enabled by default or primary features
    core_pins = {
        'CUSTOM_DISPLAY_PIN_MOSI': 47,
        'CUSTOM_DISPLAY_PIN_CLK': 21,
        'CUSTOM_DISPLAY_PIN_CS': 41,
        'CUSTOM_DISPLAY_PIN_DC': 40,
        'CUSTOM_DISPLAY_PIN_RST': 42,
        'CUSTOM_DISPLAY_PIN_BLK': 38,
        
        'CUSTOM_AUDIO_SPK_GPIO_BCLK': 15,
        'CUSTOM_AUDIO_SPK_GPIO_LRCK': 16,
        'CUSTOM_AUDIO_SPK_GPIO_DOUT': 7,
        'CUSTOM_AUDIO_MIC_GPIO_SCK': 5,
        'CUSTOM_AUDIO_MIC_GPIO_WS': 4,
        'CUSTOM_AUDIO_MIC_GPIO_DIN': 6,
        
        'CUSTOM_I2C_SDA_PIN': 8,
        'CUSTOM_I2C_SCL_PIN': 9,
        
        'CUSTOM_UART_PIN_TX': 17,
        'CUSTOM_UART_PIN_RX': 18,
        
        'CUSTOM_BUTTON_BOOT_GPIO': 0,
        'CUSTOM_LED_GPIO': 48,
    }
    
    # We will replace `default <number>` with `default -1` for any pin config 
    # that is NOT in core_pins and NOT already -1, IF it conflicts with another pin.
    # Actually, to be absolutely safe and prevent ANY overlap, let's set ALL non-core 
    # optional peripheral default pins to -1.
    
    # Match blocks like:
    # config CUSTOM_CAM_PIN_SIOD
    #     int "Camera SIOD"
    #     depends on !CUSTOM_CAMERA_USB_UVC
    #     range 0 48
    #     default 4
    
    def replacer(match):
        config_name = match.group(1)
        body = match.group(2)
        default_val = match.group(3)
        
        # Only touch it if it's a pin config
        if any(x in config_name for x in ['_PIN', '_GPIO', 'CAM_PIN']):
            if config_name not in core_pins and int(default_val) != -1:
                print(f"Changing {config_name} from {default_val} to -1")
                # Need to replace ONLY the default value line inside the body
                # The match.group(0) contains the whole config block.
                # It's safer to just replace the specific default line.
                return match.group(0) # handled differently below
        return match.group(0)

    # Let's do it line by line to preserve exact formatting
    lines = txt.splitlines(True)
    current_config = None
    
    for i, line in enumerate(lines):
        m = re.match(r'^\s*config\s+(CUSTOM_[A-Za-z0-9_]+)', line)
        if m:
            current_config = m.group(1)
        
        m_def = re.match(r'^(\s*(?:depends on .*)?default\s+)(-?\d+)\b(.*)', line)
        if m_def and current_config:
            if any(x in current_config for x in ['_PIN', '_GPIO', 'CAM_PIN']):
                if current_config not in core_pins:
                    val = int(m_def.group(2))
                    if val != -1:
                        # Some shared pins like SPI buses and I2C buses can be kept.
                        if '_I2C_SDA' in current_config:
                            lines[i] = m_def.group(1) + "8" + m_def.group(3) + "\n"
                        elif '_I2C_SCL' in current_config:
                            lines[i] = m_def.group(1) + "9" + m_def.group(3) + "\n"
                        elif '_SPI_MOSI' in current_config:
                            lines[i] = m_def.group(1) + "11" + m_def.group(3) + "\n"
                        elif '_SPI_MISO' in current_config:
                            lines[i] = m_def.group(1) + "13" + m_def.group(3) + "\n"
                        elif '_SPI_SCK' in current_config:
                            lines[i] = m_def.group(1) + "12" + m_def.group(3) + "\n"
                        else:
                            # Not a shared bus, set to -1
                            lines[i] = m_def.group(1) + "-1" + m_def.group(3) + "\n"
                            print(f"Changed {current_config} default {val} -> -1")

    with open('Kconfig.projbuild', 'w', encoding='utf-8') as f:
        f.writelines(lines)

fix_kconfig()
