import { ESP32S3, ESP32P4_WAVESHARE } from './hardware.js';
import { SdkconfigGenerator } from './generator.js';
import { kconfigPins } from './kconfigGeneric.js';

export class PinValidator {
  static getHardwareProfile(context = {}) {
    if (context.target === 'esp32p4' || context.board === 'waveshare_p4' || context.board_type === 'CONFIG_BOARD_TYPE_WAVESHARE_ESP32_P4_DEV_KIT') {
      return { profile: ESP32P4_WAVESHARE, isP4: true };
    }
    return { profile: ESP32S3, isP4: false };
  }

  static validatePinAssignment(role, pin, context = {}) {
    if (pin === undefined || pin === null || pin === -1 || pin === '-1' || isNaN(pin)) {
      return { valid: true, error: null, warning: null };
    }
    const num = Number(pin);
    const { profile, isP4 } = PinValidator.getHardwareProfile(context);
    if (num < profile.MIN || num > profile.MAX) {
      return { valid: false, error: `${role}: GPIO ${num} ngoài dải hợp lệ ${profile.MIN}-${profile.MAX}`, warning: null };
    }
    if (profile.LOCKED.includes(num)) {
      const reason = isP4 ? 'Kit Waveshare ESP32-P4 cố định' : 'Flash/PSRAM bus 26-37';
      return { valid: false, error: `${role}: GPIO ${num} BỊ KHÓA (${reason})`, warning: null };
    }
    let warning = null;
    if (!isP4 && (num === 19 || num === 20)) {
      warning = `${role}: GPIO ${num} là chân Native USB/JTAG; có thể xung đột với USB console`;
    } else if (profile.STRAP.includes(num)) {
      warning = `${role}: GPIO ${num} là chân Strapping pin; cần chú ý mức logic khi boot`;
    }
    return { valid: true, error: null, warning };
  }

  static validate(s) {
    const errors = [];
    const usage = new Map();
    const { profile, isP4 } = PinValidator.getHardwareProfile(s || {});
    function reg(pin, name, busType = null) {
      if (pin === undefined || pin === null || pin < 0 || isNaN(pin)) return;
      if (pin < profile.MIN || pin > profile.MAX) {
        errors.push({ pin, message: `${name}: GPIO ${pin} ngoài dải ${profile.MIN}-${profile.MAX}` });
        return;
      }
      if (profile.LOCKED.includes(pin)) {
        const reason = isP4 ? 'Waveshare P4 cố định' : 'Flash/PSRAM';
        errors.push({ pin, message: `${name}: GPIO ${pin} BỊ KHÓA (${reason})` });
        return;
      }
      let role = null;
      if (busType === 'i2c') role = /SCL|SIOC/i.test(name) ? 'i2c:scl' : 'i2c:sda';
      else if (busType === 'spi') role = /MISO/i.test(name) ? 'spi:miso' : /MOSI/i.test(name) ? 'spi:mosi' : 'spi:sck';
      if (!usage.has(pin)) usage.set(pin, []);
      usage.get(pin).push({ name, busType: role });
    }
    if (s.display && s.display.enable) {
      const isOled = /OLED_SSD1306|OLED_SH1106/.test(s.display.type);
      const hasSpiPins = !isOled && !['CUSTOM_DISPLAY_QSPI_AMOLED', 'CUSTOM_DISPLAY_UART',
        'CUSTOM_DISPLAY_NONE'].includes(s.display.type);
      if (hasSpiPins) {
        reg(s.display.pin_mosi,'Display MOSI','spi'); reg(s.display.pin_clk,'Display CLK','spi');
      }
      reg(s.display.pin_cs,'Display CS');     reg(s.display.pin_dc,'Display DC');
      if (s.display.use_rst) reg(s.display.pin_rst,'Display RST');
      if (s.display.use_blk) reg(s.display.pin_blk,'Display BLK');
      if (isOled) {
        reg(s.display.pin_i2c_sda,'Display SDA','i2c');
        reg(s.display.pin_i2c_scl,'Display SCL','i2c');
      }
      if (s.display.type === 'CUSTOM_DISPLAY_QSPI_AMOLED') {
        reg(s.display.qspi_cs,'AMOLED CS'); reg(s.display.qspi_clk,'AMOLED CLK');
        for (let n=0;n<=3;n++) reg(s.display[`qspi_d${n}`], `AMOLED D${n}`);
        reg(s.display.qspi_rst,'AMOLED RST');
      }
      if (s.display.type === 'CUSTOM_DISPLAY_EPAPER_SSD1681') reg(s.display.epaper_busy,'E-Paper BUSY');
      if (s.display.type === 'CUSTOM_DISPLAY_ST7701') {
        reg(s.display.rgb_pclk,'RGB PCLK'); reg(s.display.rgb_de,'RGB DE');
        reg(s.display.rgb_vsync,'RGB VSYNC'); reg(s.display.rgb_hsync,'RGB HSYNC');
        for (let n=0;n<=15;n++) reg(s.display[`rgb_d${n}`], `RGB D${n}`);
      }
      if (s.display.uart_secondary || s.display.type==='CUSTOM_DISPLAY_UART') {
        reg(s.display.uart_tx,'UART Display TX');
        reg(s.display.uart_rx,'UART Display RX');
      }
    }
    if (s.touch && s.touch.enable) {
      reg(s.touch.pin_sda,'Touch SDA','i2c'); reg(s.touch.pin_scl,'Touch SCL','i2c');
      reg(s.touch.pin_int,'Touch INT');       reg(s.touch.pin_rst,'Touch RST');
    }
    if (s.speaker && s.speaker.enable) {
      reg(s.speaker.pin_dout,'SPK DOUT'); reg(s.speaker.pin_bclk,'SPK BCLK');
      reg(s.speaker.pin_lrck,'SPK LRCK');
      if (/CODEC/.test(s.speaker.type) && !s.speaker.type.endsWith('USER_CUSTOM')) {
        reg(s.speaker.codec_sda,'SPK Codec SDA','i2c');
        reg(s.speaker.codec_scl,'SPK Codec SCL','i2c');
      }
    }
    if (s.mic && s.mic.enable) {
      reg(s.mic.pin_din,'MIC DIN'); reg(s.mic.pin_sck,'MIC SCK');
      reg(s.mic.pin_ws, 'MIC WS');
      if (/CODEC/.test(s.mic.type) && !s.mic.type.endsWith('USER_CUSTOM')) {
        reg(s.mic.codec_sda,'MIC Codec SDA','i2c');
        reg(s.mic.codec_scl,'MIC Codec SCL','i2c');
      }
    }
    if (s.buttons && s.buttons.boot_enable)  reg(s.buttons.boot_gpio,  'Boot BTN');
    if (s.buttons && s.buttons.touch_enable) reg(s.buttons.touch_gpio, 'Touch BTN');
    if (s.buttons && s.buttons.vol_enable)   {
      reg(s.buttons.vol_up_gpio,'Vol Up'); reg(s.buttons.vol_down_gpio,'Vol Down');
    }
    if (s.buttons && s.buttons.slider_enable) {
      reg(s.buttons.slider_pad1,'Slider Pad1'); reg(s.buttons.slider_pad2,'Slider Pad2');
      reg(s.buttons.slider_pad3,'Slider Pad3');
    }
    if (s.buttons && s.buttons.rotary_enable) {
      reg(s.buttons.rotary_a,'Rotary A'); reg(s.buttons.rotary_b,'Rotary B');
      reg(s.buttons.rotary_key,'Rotary Key');
    }
    if (s.camera && s.camera.enable && s.camera.sensor !== 'CUSTOM_CAMERA_USB_UVC') {
      reg(s.camera.pin_xclk,'CAM XCLK'); reg(s.camera.pin_pclk,'CAM PCLK');
      reg(s.camera.pin_vsync,'CAM VSYNC'); reg(s.camera.pin_href,'CAM HREF');
      reg(s.camera.pin_siod,'CAM SIOD','i2c'); reg(s.camera.pin_sioc,'CAM SIOC','i2c');
      if (s.camera.use_reset) reg(s.camera.pin_reset,'CAM RESET');
      if (s.camera.use_pwdn)  reg(s.camera.pin_pwdn,'CAM PWDN');
      for (let n=0;n<=7;n++) reg(s.camera[`pin_d${n}`], `CAM D${n}`);
    }
    if (s.led && s.led.enable) reg(s.led.gpio,'LED');
    if (s.mcp && s.mcp.enable && s.mcp.lamp) reg(s.mcp.lamp_gpio,'MCP Lamp');
    if (s.uart && s.uart.enable) {
      reg(s.uart.pin_tx,'UART TX'); reg(s.uart.pin_rx,'UART RX');
      if (s.uart.flow_ctrl) { reg(s.uart.pin_rts,'UART RTS'); reg(s.uart.pin_cts,'UART CTS'); }
    }
    if (s.actuators && s.actuators.servo_enable)  reg(s.actuators.servo_gpio, 'Servo');
    if (s.actuators && s.actuators.buzzer_enable) reg(s.actuators.buzzer_gpio,'Buzzer');
    if (s.actuators && s.actuators.haptic_enable) reg(s.actuators.haptic_gpio,'Haptic');
    if (s.motor && s.motor.relay_enable) reg(s.motor.relay_gpio,'Relay');
    if (s.motor && s.motor.dc_enable) {
      reg(s.motor.pwma,'Motor PWMA'); reg(s.motor.dira,'Motor DIRA');
      reg(s.motor.pwmb,'Motor PWMB'); reg(s.motor.dirb,'Motor DIRB');
    }
    if (s.sensors && s.sensors.dht_enable)    reg(s.sensors.dht_gpio,     'DHT');
    if (s.sensors && s.sensors.temp_enable) {
      reg(s.sensors.temp_sda,'Temp SDA','i2c'); reg(s.sensors.temp_scl,'Temp SCL','i2c');
    }
    if (s.sensors && s.sensors.bmp280) {
      reg(s.sensors.bmp280_sda,'BMP280 SDA','i2c'); reg(s.sensors.bmp280_scl,'BMP280 SCL','i2c');
    }
    if (s.sensors && s.sensors.bh1750) {
      reg(s.sensors.bh1750_sda,'BH1750 SDA','i2c'); reg(s.sensors.bh1750_scl,'BH1750 SCL','i2c');
    }
    if (s.sensors && s.sensors.gas_co2) {
      reg(s.sensors.gas_sda,'Gas SDA','i2c'); reg(s.sensors.gas_scl,'Gas SCL','i2c');
    }
    if (s.sensors && s.sensors.pir)           reg(s.sensors.pir_gpio,     'PIR');
    if (s.sensors && s.sensors.vibration)     reg(s.sensors.vibration_gpio,'Vibration');
    if (s.sensors && s.sensors.flame)         reg(s.sensors.flame_gpio,   'Flame');
    if (s.sensors && s.sensors.gas_mq)        reg(s.sensors.gas_mq_gpio,  'MQ Gas');
    if (s.sensors && s.sensors.hcsr04) {
      reg(s.sensors.hcsr04_trig,'HCSR04 Trig');
      reg(s.sensors.hcsr04_echo,'HCSR04 Echo');
    }
    if (s.power && s.power.tp4056) reg(s.power.tp4056_gpio,'TP4056');
    if (s.power && s.power.ina2xx) {
      reg(s.power.ina_sda,'INA SDA','i2c'); reg(s.power.ina_scl,'INA SCL','i2c');
    }
    if (!isP4 && s.kconfig) {
      for (const p of kconfigPins(SdkconfigGenerator.build(s))) reg(p.pin, p.name, p.bus);
    }
    for (const [pin, users] of usage.entries()) {
      if (users.length > 1) {
        const sharedBus = users[0].busType && users.every(u => u.busType === users[0].busType);
        if (sharedBus) continue;
        const names = users.map(u => u.name).join(' & ');
        errors.push({ pin, message: `Xung đột GPIO ${pin}: ${names}` });
      }
    }
    return { errors, usage };
  }
}
