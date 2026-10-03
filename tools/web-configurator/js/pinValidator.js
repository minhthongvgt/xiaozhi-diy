import { ESP32S3 } from './hardware.js';
export class PinValidator {
  static validate(s) {
    const errors = [];
    const usage  = new Map(); 
    function reg(pin, name, busType = null) {
      if (pin === undefined || pin === null || pin < 0 || isNaN(pin)) return;
      if (pin < ESP32S3.MIN || pin > ESP32S3.MAX) {
        errors.push({ pin, message: `${name}: GPIO ${pin} ngoài dải 0-48` });
        return;
      }
      if (ESP32S3.LOCKED.includes(pin)) {
        errors.push({ pin, message: `${name}: GPIO ${pin} BỊ KHÓA (Flash/PSRAM)` });
        return;
      }
      if (!usage.has(pin)) usage.set(pin, []);
      usage.get(pin).push({ name, busType });
    }
    if (s.display.enable) {
      reg(s.display.pin_mosi,'Display MOSI'); reg(s.display.pin_clk,'Display CLK');
      reg(s.display.pin_cs,'Display CS');     reg(s.display.pin_dc,'Display DC');
      reg(s.display.pin_rst,'Display RST');   reg(s.display.pin_blk,'Display BLK');
      reg(s.display.pin_i2c_sda,'Display SDA','i2c');
      reg(s.display.pin_i2c_scl,'Display SCL','i2c');
      if (s.display.uart_secondary || s.display.type==='CUSTOM_DISPLAY_UART') {
        reg(s.display.uart_tx,'UART Display TX');
        reg(s.display.uart_rx,'UART Display RX');
      }
    }
    if (s.touch.enable) {
      reg(s.touch.pin_sda,'Touch SDA','i2c'); reg(s.touch.pin_scl,'Touch SCL','i2c');
      reg(s.touch.pin_int,'Touch INT');       reg(s.touch.pin_rst,'Touch RST');
    }
    if (s.speaker.enable) {
      reg(s.speaker.pin_dout,'SPK DOUT'); reg(s.speaker.pin_bclk,'SPK BCLK');
      reg(s.speaker.pin_lrck,'SPK LRCK');
      reg(s.speaker.codec_sda,'SPK Codec SDA','i2c');
      reg(s.speaker.codec_scl,'SPK Codec SCL','i2c');
    }
    if (s.mic.enable) {
      reg(s.mic.pin_din,'MIC DIN'); reg(s.mic.pin_sck,'MIC SCK');
      reg(s.mic.pin_ws, 'MIC WS');
      reg(s.mic.codec_sda,'MIC Codec SDA','i2c');
      reg(s.mic.codec_scl,'MIC Codec SCL','i2c');
    }
    if (s.buttons.boot_enable)  reg(s.buttons.boot_gpio,  'Boot BTN');
    if (s.buttons.touch_enable) reg(s.buttons.touch_gpio, 'Touch BTN');
    if (s.buttons.vol_enable)   {
      reg(s.buttons.vol_up_gpio,'Vol Up'); reg(s.buttons.vol_down_gpio,'Vol Down');
    }
    if (s.buttons.rotary_enable) {
      reg(s.buttons.rotary_a,'Rotary A'); reg(s.buttons.rotary_b,'Rotary B');
      reg(s.buttons.rotary_key,'Rotary Key');
    }
    if (s.led.enable) reg(s.led.gpio,'LED');
    if (s.mcp.enable && s.mcp.lamp) reg(s.mcp.lamp_gpio,'MCP Lamp');
    if (s.uart.enable) {
      reg(s.uart.pin_tx,'UART TX'); reg(s.uart.pin_rx,'UART RX');
    }
    if (s.network.cellular_enable) {
      reg(s.network.cellular_tx, 'Cellular TX'); reg(s.network.cellular_rx, 'Cellular RX');
      reg(s.network.cellular_pwrkey, 'Cellular PWRKEY'); reg(s.network.cellular_rst, 'Cellular RST');
    }
    if (s.network.uart_ext_enable) {
      reg(s.network.uart_ext_tx, 'Ext UART TX'); reg(s.network.uart_ext_rx, 'Ext UART RX');
    }
    if (s.actuators.servo_enable)  reg(s.actuators.servo_gpio, 'Servo');
    if (s.actuators.buzzer_enable) reg(s.actuators.buzzer_gpio,'Buzzer');
    if (s.actuators.haptic_enable) reg(s.actuators.haptic_gpio,'Haptic');
    if (s.motor.relay_enable) reg(s.motor.relay_gpio,'Relay');
    if (s.motor.dc_enable) {
      reg(s.motor.pwma,'Motor PWMA'); reg(s.motor.dira,'Motor DIRA');
      reg(s.motor.pwmb,'Motor PWMB'); reg(s.motor.dirb,'Motor DIRB');
    }
    if (s.sensors.dht_enable)    reg(s.sensors.dht_gpio,     'DHT');
    if (s.sensors.pir)           reg(s.sensors.pir_gpio,     'PIR');
    if (s.sensors.vibration)     reg(s.sensors.vibration_gpio,'Vibration');
    if (s.sensors.flame)         reg(s.sensors.flame_gpio,   'Flame');
    if (s.sensors.gas_mq)        reg(s.sensors.gas_mq_gpio,  'MQ Gas');
    if (s.sensors.hcsr04) {
      reg(s.sensors.hcsr04_trig,'HCSR04 Trig');
      reg(s.sensors.hcsr04_echo,'HCSR04 Echo');
    }
    if (s.power.tp4056) reg(s.power.tp4056_gpio,'TP4056');
    for (const [pin, users] of usage.entries()) {
      if (users.length > 1) {
        const allI2c = users.every(u => u.busType === 'i2c');
        if (allI2c) continue; 
        const names = users.map(u => u.name).join(' & ');
        errors.push({ pin, message: `Xung đột GPIO ${pin}: ${names}` });
      }
    }
    return { errors, usage };
  }
}
