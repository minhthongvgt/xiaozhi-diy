import os
import re
import sys

def main():
    kconfig_path = os.path.join(os.path.dirname(__file__), '..', 'main', 'Kconfig.projbuild')
    
    with open(kconfig_path, 'r', encoding='utf-8') as f:
        kconfig_content = f.read()

    kconfig_pins = set(re.findall(r'config\s+(CUSTOM_[A-Z0-9_]+(?:PIN|GPIO|SDA|SCL)[A-Z0-9_]*)', kconfig_content))
    
    print("        // --- AUTO-GENERATED MISSING PINS ---")
    for pin in sorted(list(kconfig_pins)):
        if "I2C_SDA" in pin or pin.endswith("_SDA"):
            bus_type = "BUS_TYPE_I2C_SDA"
        elif "I2C_SCL" in pin or pin.endswith("_SCL"):
            bus_type = "BUS_TYPE_I2C_SCL"
        elif "I2S_BCLK" in pin or pin.endswith("_BCLK") or pin.endswith("_SCK"):
            bus_type = "BUS_TYPE_I2S_BCLK"
        elif "I2S_WS" in pin or pin.endswith("_LRCK") or pin.endswith("_WS"):
            bus_type = "BUS_TYPE_I2S_WS"
        else:
            bus_type = "BUS_TYPE_EXCLUSIVE"
            
        print(f"#if defined(CONFIG_{pin})")
        print(f'        {{ CONFIG_{pin}, "{pin}", {bus_type}, true }},')
        print(f"#endif")
        
if __name__ == "__main__":
    main()
