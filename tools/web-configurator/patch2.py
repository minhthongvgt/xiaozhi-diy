import re
with open('app.js', 'r', encoding='utf-8') as f:
    app_js = f.read()

# 1. Update CodeGenerator
cg_inj = '''
      } else if (p.type === "sensor_nfc") {
        cfg.push(`#define CUSTOM_PERIPH_NFC_I2C_SDA             ${gpioMacro(p.sda)}`);
        cfg.push(`#define CUSTOM_PERIPH_NFC_I2C_SCL             ${gpioMacro(p.scl)}`);
        cfg.push(`#define CUSTOM_PERIPH_NFC_IRQ_PIN             ${gpioMacro(p.irq)}`);
        cfg.push(`#define CUSTOM_PERIPH_NFC_RST_PIN             ${gpioMacro(p.rst)}`);
      } else if (p.type === "sensor_rtc") {
        cfg.push(`#define CUSTOM_PERIPH_RTC_I2C_SDA             ${gpioMacro(p.sda)}`);
        cfg.push(`#define CUSTOM_PERIPH_RTC_I2C_SCL             ${gpioMacro(p.scl)}`);
        cfg.push(`#define CUSTOM_PERIPH_RTC_INT_PIN             ${gpioMacro(p.int)}`);
      } else if (p.type === "actuator_pca9685") {
        cfg.push(`#define CUSTOM_PERIPH_PCA9685_I2C_SDA         ${gpioMacro(p.sda)}`);
        cfg.push(`#define CUSTOM_PERIPH_PCA9685_I2C_SCL         ${gpioMacro(p.scl)}`);
        cfg.push(`#define CUSTOM_PERIPH_PCA9685_OE_PIN          ${gpioMacro(p.oe)}`);
'''
cg_target = '} else if (p.type === "sensor_vl6180x") {'
app_js = app_js.replace(cg_target, cg_inj.strip('\n') + '\n      ' + cg_target)

cg_comm_inj = '''
      } else if (c.type === "comm_twai") {
        cfg.push(`#define CUSTOM_PERIPH_TWAI_TX_PIN             ${gpioMacro(c.tx)}`);
        cfg.push(`#define CUSTOM_PERIPH_TWAI_RX_PIN             ${gpioMacro(c.rx)}`);
      } else if (c.type === "comm_4g") {
        cfg.push(`#define CUSTOM_PERIPH_4G_UART_TX_PIN          ${gpioMacro(c.tx)}`);
        cfg.push(`#define CUSTOM_PERIPH_4G_UART_RX_PIN          ${gpioMacro(c.rx)}`);
        cfg.push(`#define CUSTOM_PERIPH_4G_PWRKEY_PIN           ${gpioMacro(c.pwr)}`);
'''
cg_comm_target = '} else if (c.type === "sdcard") {'
app_js = app_js.replace(cg_comm_target, cg_comm_inj.strip('\n') + '\n      ' + cg_comm_target)

# 2. Update PinValidator
pv_inj = '''
      } else if (p.type === "sensor_nfc") {
        registerPin(p.sda, `${p.name} SDA`, `p_${p.id}_sda`, "i2c_sda");
        registerPin(p.scl, `${p.name} SCL`, `p_${p.id}_scl`, "i2c_scl");
        registerPin(p.irq, `${p.name} IRQ`, `p_${p.id}_irq`);
        registerPin(p.rst, `${p.name} RST`, `p_${p.id}_rst`);
      } else if (p.type === "sensor_rtc") {
        registerPin(p.sda, `${p.name} SDA`, `p_${p.id}_sda`, "i2c_sda");
        registerPin(p.scl, `${p.name} SCL`, `p_${p.id}_scl`, "i2c_scl");
        registerPin(p.int, `${p.name} INT`, `p_${p.id}_int`);
      } else if (p.type === "actuator_pca9685") {
        registerPin(p.sda, `${p.name} SDA`, `p_${p.id}_sda`, "i2c_sda");
        registerPin(p.scl, `${p.name} SCL`, `p_${p.id}_scl`, "i2c_scl");
        registerPin(p.oe, `${p.name} OE`, `p_${p.id}_oe`);
'''
pv_target = '} else if (p.type === "sensor_vl6180x") {'
app_js = app_js.replace(pv_target, pv_inj.strip('\n') + '\n      ' + pv_target)

pv_comm_inj = '''
      } else if (c.type === "comm_twai") {
        registerPin(c.tx, `${c.name} TX`, `comm_${c.id}_tx`);
        registerPin(c.rx, `${c.name} RX`, `comm_${c.id}_rx`);
      } else if (c.type === "comm_4g") {
        registerPin(c.tx, `${c.name} TX`, `comm_${c.id}_tx`);
        registerPin(c.rx, `${c.name} RX`, `comm_${c.id}_rx`);
        registerPin(c.pwr, `${c.name} PWR`, `comm_${c.id}_pwr`);
'''
pv_comm_target = '} else if (c.type === "sdcard") {'
app_js = app_js.replace(pv_comm_target, pv_comm_inj.strip('\n') + '\n      ' + pv_comm_target)

# 3. Update UI save/load for dynamic fields
load_inj = '''
        } else if (type === "sensor_nfc") {
          obj.sda = parseInt(document.getElementById("modal_field_nfc_sda").value, 10);
          obj.scl = parseInt(document.getElementById("modal_field_nfc_scl").value, 10);
          obj.irq = parseInt(document.getElementById("modal_field_nfc_irq").value, 10);
          obj.rst = parseInt(document.getElementById("modal_field_nfc_rst").value, 10);
        } else if (type === "sensor_rtc") {
          obj.sda = parseInt(document.getElementById("modal_field_rtc_sda").value, 10);
          obj.scl = parseInt(document.getElementById("modal_field_rtc_scl").value, 10);
          obj.int = parseInt(document.getElementById("modal_field_rtc_int").value, 10);
        } else if (type === "actuator_pca9685") {
          obj.sda = parseInt(document.getElementById("modal_field_pca_sda").value, 10);
          obj.scl = parseInt(document.getElementById("modal_field_pca_scl").value, 10);
          obj.oe = parseInt(document.getElementById("modal_field_pca_oe").value, 10);
        } else if (type === "comm_twai") {
          obj.tx = parseInt(document.getElementById("modal_field_twai_tx").value, 10);
          obj.rx = parseInt(document.getElementById("modal_field_twai_rx").value, 10);
        } else if (type === "comm_4g") {
          obj.tx = parseInt(document.getElementById("modal_field_4g_tx").value, 10);
          obj.rx = parseInt(document.getElementById("modal_field_4g_rx").value, 10);
          obj.pwr = parseInt(document.getElementById("modal_field_4g_pwr").value, 10);
'''
load_target = '} else if (type === "sensor_dht") {'
app_js = app_js.replace(load_target, load_inj.strip('\n') + '\n        ' + load_target)

with open('app.js', 'w', encoding='utf-8') as f:
    f.write(app_js)
print("Patch2 applied")
