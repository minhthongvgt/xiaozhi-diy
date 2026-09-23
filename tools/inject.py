import os
import re
import sys

def main():
    kconfig_path = os.path.join(os.path.dirname(__file__), '..', 'main', 'Kconfig.projbuild')
    validator_path = os.path.join(os.path.dirname(__file__), '..', 'main', 'gpio_validator.c')

    with open(kconfig_path, 'r', encoding='utf-8') as f:
        kconfig_content = f.read()

    with open(validator_path, 'r', encoding='utf-8') as f:
        validator_content = f.read()

    kconfig_pins = set(re.findall(r'config\s+(CUSTOM_[A-Z0-9_]+(?:PIN|GPIO|SDA|SCL)[A-Z0-9_]*)', kconfig_content))
    
    missing_in_validator = []
    for pin in sorted(list(kconfig_pins)):
        if pin not in validator_content:
            missing_in_validator.append(pin)

    new_items = "        // --- AUTO-GENERATED MISSING PINS ---\n"
    for pin in missing_in_validator:
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
            
        new_items += f"#if defined(CONFIG_{pin})\n"
        new_items += f'        {{ CONFIG_{pin}, "{pin}", {bus_type}, true }},\n'
        new_items += f"#endif\n"

    # Inject into gpio_validator.c
    pattern = r'(configured_pin_t pins\[\] = \{.*?)(\n    \};\n)'
    match = re.search(pattern, validator_content, re.DOTALL)
    if match:
        updated = validator_content[:match.end(1)] + "\n" + new_items + validator_content[match.start(2):]
        with open(validator_path, 'w', encoding='utf-8') as f:
            f.write(updated)
        print("Injected successfully.")
    else:
        print("Could not find insertion point.")

if __name__ == "__main__":
    main()
