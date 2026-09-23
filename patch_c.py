import sys, re
sys.stdout.reconfigure(encoding='utf-8')

# 1. Patch Kconfig.projbuild
txt = open('main/Kconfig.projbuild', encoding='utf-8').read()

def set_default(config, val):
    global txt
    # Replaces default <number> for the given config
    pattern = r'(config\s+' + config + r'\b.*?default\s+)-?\d+'
    txt = re.sub(pattern, r'\g<1>' + str(val), txt, flags=re.DOTALL)

# UART is already 17/18 in core pins, but let's make sure
set_default('CUSTOM_DISPLAY_UART_TX_PIN', 17)
set_default('CUSTOM_DISPLAY_UART_RX_PIN', 18)
set_default('CUSTOM_UART_PIN_TX', 17)
set_default('CUSTOM_UART_PIN_RX', 18)
set_default('CUSTOM_MODEM_UART_TX_PIN', 17)
set_default('CUSTOM_MODEM_UART_RX_PIN', 18)

# Encoder to 43/44
set_default('CUSTOM_PERIPH_ENCODER_PHASE_A', 43)
set_default('CUSTOM_PERIPH_ENCODER_PHASE_B', 44)

# IR TX to 45
set_default('CUSTOM_PERIPH_IR_TX_PIN', 45)

# Motor PWMA to 41, DIRA to 42, PWMB to 1, DIRB to 2 (swap them or just put PWMA to 41)
set_default('CUSTOM_PERIPH_MOTOR_PWMA_PIN', 41)
set_default('CUSTOM_PERIPH_MOTOR_DIRA_PIN', 42)
set_default('CUSTOM_PERIPH_MOTOR_PWMB_PIN', 1)
set_default('CUSTOM_PERIPH_MOTOR_DIRB_PIN', 2)

# SENSOR MQ to 3
set_default('CUSTOM_SENSOR_MQ_ANALOG_PIN', 3)

with open('main/Kconfig.projbuild', 'w', encoding='utf-8') as f:
    f.write(txt)

# 2. Patch app.js
app = open('tools/web-configurator/app.js', encoding='utf-8').read()
app = app.replace('id="modal_field_rot_a" min="0" max="48" value="17"', 'id="modal_field_rot_a" min="0" max="48" value="43"')
app = app.replace('id="modal_field_rot_b" min="0" max="48" value="18"', 'id="modal_field_rot_b" min="0" max="48" value="44"')
app = app.replace('<option value="0">ADC1 Channel 0 (GPIO 1)</option>', '<option value="0" selected>ADC1 Channel 0 (GPIO 1)</option>')
app = app.replace('<option value="2">ADC1 Channel 2 (GPIO 3)</option>', '<option value="2" selected>ADC1 Channel 2 (GPIO 3)</option>')
# The HTML selects the LAST selected option, so the last one wins. Or I can just regex it.
import re
app = re.sub(r'<option value="0"( selected)?>ADC1 Channel 0', r'<option value="0">ADC1 Channel 0', app)
app = re.sub(r'<option value="2"( selected)?>ADC1 Channel 2', r'<option value="2" selected>ADC1 Channel 2', app)

# Motor PWMA/DIRA
app = app.replace('id="modal_field_pwma" value="1"', 'id="modal_field_pwma" value="41"')
app = app.replace('id="modal_field_dira" value="2"', 'id="modal_field_dira" value="42"')
app = app.replace('id="modal_field_pwmb" value="41"', 'id="modal_field_pwmb" value="1"')
app = app.replace('id="modal_field_dirb" value="42"', 'id="modal_field_dirb" value="2"')

with open('tools/web-configurator/app.js', 'w', encoding='utf-8') as f:
    f.write(app)

print("Patched Task C")
