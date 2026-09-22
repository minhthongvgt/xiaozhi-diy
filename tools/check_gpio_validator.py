import os
import re
import sys

def main():
    kconfig_path = os.path.join(os.path.dirname(__file__), '..', 'main', 'Kconfig.projbuild')
    validator_path = os.path.join(os.path.dirname(__file__), '..', 'main', 'gpio_validator.c')

    if not os.path.exists(kconfig_path):
        print(f"Error: {kconfig_path} not found.")
        sys.exit(1)
        
    if not os.path.exists(validator_path):
        print(f"Error: {validator_path} not found.")
        sys.exit(1)

    with open(kconfig_path, 'r', encoding='utf-8') as f:
        kconfig_content = f.read()

    with open(validator_path, 'r', encoding='utf-8') as f:
        validator_content = f.read()

    kconfig_pins = set(re.findall(r'config\s+(CUSTOM_[A-Z0-9_]+(?:PIN|GPIO|SDA|SCL)[A-Z0-9_]*)', kconfig_content))
    
    missing_in_validator = []
    for pin in sorted(list(kconfig_pins)):
        if pin not in validator_content:
            missing_in_validator.append(pin)

    if missing_in_validator:
        print("ERROR: The following pins from Kconfig.projbuild are MISSING in gpio_validator.c:")
        for pin in missing_in_validator:
            print(f"  - {pin}")
        sys.exit(1)
    else:
        print("SUCCESS: All GPIO configurations in Kconfig.projbuild are covered in gpio_validator.c!")
        sys.exit(0)

if __name__ == "__main__":
    main()
