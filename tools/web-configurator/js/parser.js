import { DEFAULT_CONFIG } from './configState.js';
export class SdkconfigParser {
  static textToMap(text) {
    const map = {};
    for (const raw of text.split(/\r?\n/)) {
      const line = raw.trim();
      if (!line || line.startsWith('##')) continue;
      if (line.startsWith('# ') && line.endsWith(' is not set')) {
        map[line.slice(2, -11).trim()] = 'n';
        continue;
      }
      if (!line.startsWith('#') && line.includes('=')) {
        const eq = line.indexOf('=');
        map[line.slice(0, eq).trim()] = line.slice(eq + 1).trim();
      }
    }
    return map;
  }
  static _y(m, k)         { return k in m ? m[k] === 'y' : undefined; }
  static _i(m, k, def) {
    if (!(k in m)) return def;            
    const value = parseInt(m[k], 10);
    return Number.isNaN(value) ? def : value;
  }
  static _restoreMissing(s, d) {
    for (const k of Object.keys(d)) {
      if (d[k] && typeof d[k] === 'object') SdkconfigParser._restoreMissing(s[k], d[k]);
      else if (s[k] === undefined) s[k] = d[k];
    }
  }
  static _s(m, k, def='') { return k in m ? m[k].replace(/^"|"$/g,'') : def; }
  static _choice(m, keys) {
    for (const k of keys) if (m[k] === 'y') return k;
    return null;
  }
  static mapToState(m) {
    const s  = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
    const y  = (k)         => SdkconfigParser._y(m, k);
    const i  = (k, d)      => SdkconfigParser._i(m, k, d);
    const st = (k, d='')   => SdkconfigParser._s(m, k, d);
    const ch = (keys)      => SdkconfigParser._choice(m, keys);
    if ('CONFIG_SPIRAM' in m) s.system.spiram = y('CONFIG_SPIRAM');
    const pick = (opts, fmt) => opts.find(o => m[fmt(o)] === 'y');
    const pm = pick(['OCT','QUAD'], o => `CONFIG_SPIRAM_MODE_${o}`);
    if (pm) s.system.spiram_mode = pm === 'QUAD' ? 'QIO' : 'OCT';   
    const ps = pick(['120M','80M','40M'], o => `CONFIG_SPIRAM_SPEED_${o}`);
    if (ps) s.system.spiram_speed = ps;
    const fm = pick(['QIO','QOUT','DIO','DOUT'], o => `CONFIG_ESPTOOLPY_FLASHMODE_${o}`);
    if (fm) s.system.flash_mode = fm;
    const ff = pick(['80M','40M'], o => `CONFIG_ESPTOOLPY_FLASHFREQ_${o}`);
    if (ff) s.system.flash_freq = ff;
    const fs_ = pick(['4MB','8MB','16MB','32MB'], o => `CONFIG_ESPTOOLPY_FLASHSIZE_${o}`);
    if (fs_) s.system.flash_size = fs_;
    const cf = pick(['240','160'], o => `CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_${o}`);
    if (cf) s.system.cpu_freq = cf;
    if ('CONFIG_PARTITION_TABLE_CUSTOM_FILENAME' in m)
      s.system.partition_table = st('CONFIG_PARTITION_TABLE_CUSTOM_FILENAME');
    const lv = pick(['NONE','ERROR','WARN','INFO','DEBUG','VERBOSE'], o => `CONFIG_LOG_DEFAULT_LEVEL_${o}`);
    if (lv) s.build.log_level = lv;
    if ('CONFIG_ESP_CONSOLE_UART_BAUDRATE' in m) s.build.baud_rate = String(i('CONFIG_ESP_CONSOLE_UART_BAUDRATE', 115200));
    if ('CONFIG_TOUCH_SUPPRESS_DEPRECATE_WARN' in m) s.build.touch_suppress = y('CONFIG_TOUCH_SUPPRESS_DEPRECATE_WARN');
    if ('CONFIG_PARTITION_TABLE_MD5' in m) s.build.md5_partition = y('CONFIG_PARTITION_TABLE_MD5');
    s.network.cellular_enable = false;
    const LANGS = ['ZH_CN','ZH_TW','EN_US','JA_JP','KO_KR','VI_VN','TH_TH',
      'DE_DE','FR_FR','ES_ES','IT_IT','RU_RU','AR_SA','HI_IN','MR_IN',
      'PT_PT','PT_BR','PL_PL','CS_CZ','FI_FI','TR_TR','ID_ID','UK_UA',
      'RO_RO','BG_BG','CA_ES','DA_DK','EL_GR','FA_IR','FIL_PH','HE_IL',
      'HR_HR','HU_HU','MS_MY','NB_NO','NL_NL','SK_SK','SL_SI','SV_SE','SR_RS'];
    const lang = ch(LANGS.map(l=>`CONFIG_LANGUAGE_${l}`));
    if (lang) s.general.language = lang.replace('CONFIG_','');
    if ('CONFIG_OTA_URL' in m) s.general.ota_url = st('CONFIG_OTA_URL');
    const ASSETS = ['FLASH_DEFAULT_ASSETS','FLASH_CUSTOM_ASSETS',
                    'FLASH_EXPRESSION_ASSETS','FLASH_NONE_ASSETS'];
    const asset = ch(ASSETS.map(a=>`CONFIG_${a}`));
    if (asset) s.general.flash_assets = asset.replace('CONFIG_','');
    if ('CONFIG_CUSTOM_ASSETS_FILE' in m)
      s.general.custom_assets_file = st('CONFIG_CUSTOM_ASSETS_FILE');
    const STYLES = ['USE_DEFAULT_MESSAGE_STYLE','USE_WECHAT_MESSAGE_STYLE',
                    'USE_EMOTE_MESSAGE_STYLE'];
    const style = ch(STYLES.map(s2=>`CONFIG_${s2}`));
    if (style) s.general.display_style = style.replace('CONFIG_','');
    s.general.multiline_chat = (s.general.display_style === 'USE_DEFAULT_MESSAGE_STYLE') && y('CONFIG_USE_MULTILINE_CHAT_MESSAGE');
    s.display.enable = y('CONFIG_ENABLE_CUSTOM_DISPLAY');
    const DTYPES = ['ST7789','ST7796','ST7735','ST7701','ILI9341','ILI9486',
      'GC9A01','GC9107','NV3023','JD9853','OLED_SSD1306','OLED_SH1106',
      'QSPI_AMOLED','EPAPER_SSD1681','UART','USER_CUSTOM','NONE'];
    const dtype = ch(DTYPES.map(t=>`CONFIG_CUSTOM_DISPLAY_${t}`));
    if (dtype) s.display.type = dtype.replace('CONFIG_','');
    s.display.width    = i('CONFIG_CUSTOM_DISPLAY_WIDTH', 240);
    s.display.height   = i('CONFIG_CUSTOM_DISPLAY_HEIGHT', 320);
    s.display.pin_mosi = i('CONFIG_CUSTOM_DISPLAY_PIN_MOSI');
    s.display.pin_clk  = i('CONFIG_CUSTOM_DISPLAY_PIN_CLK');
    s.display.pin_cs   = i('CONFIG_CUSTOM_DISPLAY_PIN_CS');
    s.display.pin_dc   = i('CONFIG_CUSTOM_DISPLAY_PIN_DC');
    s.display.use_rst  = y('CONFIG_CUSTOM_DISPLAY_USE_RST');
    s.display.pin_rst  = i('CONFIG_CUSTOM_DISPLAY_PIN_RST');
    s.display.use_blk  = y('CONFIG_CUSTOM_DISPLAY_USE_BLK');
    s.display.pin_blk  = i('CONFIG_CUSTOM_DISPLAY_PIN_BLK');
    s.display.pin_i2c_sda = i('CONFIG_CUSTOM_DISPLAY_PIN_I2C_SDA');
    s.display.pin_i2c_scl = i('CONFIG_CUSTOM_DISPLAY_PIN_I2C_SCL');
    s.display.offset_x = i('CONFIG_CUSTOM_DISPLAY_OFFSET_X', 0);
    s.display.offset_y = i('CONFIG_CUSTOM_DISPLAY_OFFSET_Y', 0);
    s.display.mirror_x = y('CONFIG_CUSTOM_DISPLAY_MIRROR_X');
    s.display.mirror_y = y('CONFIG_CUSTOM_DISPLAY_MIRROR_Y');
    s.display.swap_xy  = y('CONFIG_CUSTOM_DISPLAY_SWAP_XY');
    s.display.invert_color = y('CONFIG_CUSTOM_DISPLAY_INVERT_COLOR');
    const ACHIPS = ['SH8601','CO5300','SPD2010'];
    const achip = ch(ACHIPS.map(c=>`CONFIG_CUSTOM_DISPLAY_AMOLED_${c}`));
    if (achip) s.display.amoled_chip = achip.replace('CONFIG_','');
    s.display.qspi_cs = i('CONFIG_CUSTOM_DISPLAY_QSPI_CS');
    s.display.qspi_clk= i('CONFIG_CUSTOM_DISPLAY_QSPI_CLK');
    for (let n=0;n<=3;n++) s.display[`qspi_d${n}`]=i(`CONFIG_CUSTOM_DISPLAY_QSPI_D${n}`);
    s.display.qspi_rst= i('CONFIG_CUSTOM_DISPLAY_QSPI_RST');
    s.display.epaper_busy = i('CONFIG_CUSTOM_DISPLAY_EPAPER_PIN_BUSY');
    s.display.rgb_pclk  = i('CONFIG_CUSTOM_DISPLAY_RGB_PCLK');
    s.display.rgb_de    = i('CONFIG_CUSTOM_DISPLAY_RGB_DE');
    s.display.rgb_vsync = i('CONFIG_CUSTOM_DISPLAY_RGB_VSYNC');
    s.display.rgb_hsync = i('CONFIG_CUSTOM_DISPLAY_RGB_HSYNC');
    for (let n=0;n<=15;n++) s.display[`rgb_d${n}`]=i(`CONFIG_CUSTOM_DISPLAY_RGB_D${n}`);
    s.display.uart_secondary = y('CONFIG_ENABLE_CUSTOM_SECONDARY_UART_DISPLAY');
    s.display.uart_port  = i('CONFIG_CUSTOM_DISPLAY_UART_PORT', 1);
    s.display.uart_tx    = i('CONFIG_CUSTOM_DISPLAY_UART_TX_PIN');
    s.display.uart_rx    = i('CONFIG_CUSTOM_DISPLAY_UART_RX_PIN');
    const BAUDS = ['9600','19200','38400','57600','115200','230400','460800','921600'];
    const baudChoice = ch(BAUDS.map(b => `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_${b}`));
    if (baudChoice) {
      s.display.uart_baud = parseInt(baudChoice.replace('CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_', ''), 10);
    } else {
      s.display.uart_baud = i('CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE', 115200);
    }
    const PROTOS = ['NEXTION','JSON','RAW_TEXT','DWIN'];
    const proto = ch(PROTOS.map(p=>`CONFIG_CUSTOM_DISPLAY_UART_PROTO_${p}`));
    if (proto) s.display.uart_proto = proto.replace('CONFIG_','');
    s.touch.enable = y('CONFIG_ENABLE_CUSTOM_TOUCH');
    const TTYPES = ['CST816S','GT911','FT5X06','USER_CUSTOM','NONE'];
    const ttype = ch(TTYPES.map(t=>`CONFIG_CUSTOM_TOUCH_${t}`));
    if (ttype) s.touch.type = ttype.replace('CONFIG_','');
    s.touch.pin_sda = i('CONFIG_CUSTOM_TOUCH_PIN_SDA');
    s.touch.pin_scl = i('CONFIG_CUSTOM_TOUCH_PIN_SCL');
    s.touch.pin_int = i('CONFIG_CUSTOM_TOUCH_PIN_INT');
    s.touch.pin_rst = i('CONFIG_CUSTOM_TOUCH_PIN_RST');
    s.speaker.enable = y('CONFIG_ENABLE_CUSTOM_SPEAKER');
    const STYPES = ['DAC_MAX98357A','DAC_MAX98360A','DAC_PCM5102A',
      'SPK_CODEC_ES8311','SPK_CODEC_ES8388','SPK_CODEC_ES8374',
      'SPK_CODEC_ES8389','SPK_CODEC_BOX','SPK_CODEC_USER_CUSTOM'];
    const stype = ch(STYPES.map(t=>`CONFIG_CUSTOM_AUDIO_${t}`));
    if (stype) s.speaker.type = stype.replace('CONFIG_','');
    s.speaker.pin_dout  = i('CONFIG_CUSTOM_AUDIO_SPK_GPIO_DOUT');
    s.speaker.pin_bclk  = i('CONFIG_CUSTOM_AUDIO_SPK_GPIO_BCLK');
    s.speaker.pin_lrck  = i('CONFIG_CUSTOM_AUDIO_SPK_GPIO_LRCK');
    s.speaker.codec_sda = i('CONFIG_CUSTOM_AUDIO_SPK_CODEC_I2C_SDA');
    s.speaker.codec_scl = i('CONFIG_CUSTOM_AUDIO_SPK_CODEC_I2C_SCL');
    s.mic.enable = y('CONFIG_ENABLE_CUSTOM_MIC');
    const MTYPES = ['MIC_INMP441','MIC_PDM','MIC_CODEC_ES8311','MIC_CODEC_ES8388',
      'MIC_CODEC_ES8374','MIC_CODEC_ES8389','MIC_CODEC_BOX','MIC_CODEC_USER_CUSTOM'];
    const mtype = ch(MTYPES.map(t=>`CONFIG_CUSTOM_AUDIO_${t}`));
    if (mtype) s.mic.type = mtype.replace('CONFIG_','');
    s.mic.pin_din  = i('CONFIG_CUSTOM_AUDIO_MIC_GPIO_DIN');
    s.mic.pin_sck  = i('CONFIG_CUSTOM_AUDIO_MIC_GPIO_SCK');
    s.mic.pin_ws   = i('CONFIG_CUSTOM_AUDIO_MIC_GPIO_WS');
    s.mic.codec_sda= i('CONFIG_CUSTOM_AUDIO_MIC_CODEC_I2C_SDA');
    s.mic.codec_scl= i('CONFIG_CUSTOM_AUDIO_MIC_CODEC_I2C_SCL');
    s.audio.i2s_mode = y('CONFIG_CUSTOM_AUDIO_I2S_DUPLEX')
      ? 'CUSTOM_AUDIO_I2S_DUPLEX' : 'CUSTOM_AUDIO_I2S_SIMPLEX';
    const WWTYPES = ['USE_AFE_WAKE_WORD','USE_ESP_WAKE_WORD',
                     'USE_CUSTOM_WAKE_WORD','WAKE_WORD_DISABLED'];
    const wwt = ch(WWTYPES.map(t=>`CONFIG_${t}`));
    if (wwt) s.wakeword.type = wwt.replace('CONFIG_','');
    s.wakeword.custom_word    = st('CONFIG_CUSTOM_WAKE_WORD','xiao tu dou');
    s.wakeword.custom_display = st('CONFIG_CUSTOM_WAKE_WORD_DISPLAY','小土豆');
    s.wakeword.threshold      = i('CONFIG_CUSTOM_WAKE_WORD_THRESHOLD', 20);
    s.wakeword.device_aec     = y('CONFIG_USE_DEVICE_AEC');
    s.wakeword.server_aec     = y('CONFIG_USE_SERVER_AEC');
    s.wakeword.send_data      = 'CONFIG_SEND_WAKE_WORD_DATA' in m
                                  ? y('CONFIG_SEND_WAKE_WORD_DATA') : true;
    s.wakeword.detection_in_listening = ['USE_AFE_WAKE_WORD','USE_CUSTOM_WAKE_WORD'].includes(s.wakeword.type) && y('CONFIG_WAKE_WORD_DETECTION_IN_LISTENING');
    s.wifi.method = y('CONFIG_USE_ESP_BLUFI_WIFI_PROVISIONING')
      ? 'USE_ESP_BLUFI_WIFI_PROVISIONING' : 'USE_HOTSPOT_WIFI_PROVISIONING';
    s.buttons.boot_enable   = y('CONFIG_CUSTOM_ENABLE_BUTTON_BOOT');
    s.buttons.boot_gpio     = i('CONFIG_CUSTOM_BUTTON_BOOT_GPIO');
    s.buttons.touch_enable  = y('CONFIG_CUSTOM_ENABLE_BUTTON_TOUCH');
    s.buttons.touch_gpio    = i('CONFIG_CUSTOM_BUTTON_TOUCH_GPIO');
    s.buttons.vol_enable    = y('CONFIG_CUSTOM_ENABLE_BUTTON_VOLUME');
    s.buttons.vol_up_gpio   = i('CONFIG_CUSTOM_BUTTON_VOL_UP_GPIO');
    s.buttons.vol_down_gpio = i('CONFIG_CUSTOM_BUTTON_VOL_DOWN_GPIO');
    s.buttons.slider_enable = y('CONFIG_CUSTOM_ENABLE_TOUCH_SLIDER');
    s.buttons.slider_pad1   = i('CONFIG_CUSTOM_TOUCH_SLIDER_PAD1_GPIO');
    s.buttons.slider_pad2   = i('CONFIG_CUSTOM_TOUCH_SLIDER_PAD2_GPIO');
    s.buttons.slider_pad3   = i('CONFIG_CUSTOM_TOUCH_SLIDER_PAD3_GPIO');
    s.buttons.rotary_enable = y('CONFIG_CUSTOM_ENABLE_PERIPH_ROTARY_ENCODER');
    s.buttons.rotary_a      = i('CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_A');
    s.buttons.rotary_b      = i('CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_B');
    s.buttons.rotary_key    = i('CONFIG_CUSTOM_PERIPH_ENCODER_KEY_PIN');
    s.buttons.user_custom_sensors = y('CONFIG_CUSTOM_ENABLE_USER_CUSTOM_SENSORS');
    s.camera.enable = y('CONFIG_ENABLE_CUSTOM_CAMERA');
    const CSENS = ['OV2640','OV3660','OV5640','SC030IOT','USB_UVC','USER_CUSTOM'];
    const csens = ch(CSENS.map(c=>`CONFIG_CUSTOM_CAMERA_${c}`));
    if (csens) s.camera.sensor = csens.replace('CONFIG_','');
    s.camera.pin_xclk  = i('CONFIG_CUSTOM_CAM_PIN_XCLK');
    s.camera.pin_pclk  = i('CONFIG_CUSTOM_CAM_PIN_PCLK');
    s.camera.pin_vsync = i('CONFIG_CUSTOM_CAM_PIN_VSYNC');
    s.camera.pin_href  = i('CONFIG_CUSTOM_CAM_PIN_HREF');
    s.camera.pin_siod  = i('CONFIG_CUSTOM_CAM_PIN_SIOD');
    s.camera.pin_sioc  = i('CONFIG_CUSTOM_CAM_PIN_SIOC');
    s.camera.use_reset = y('CONFIG_CUSTOM_CAM_USE_RESET');
    s.camera.pin_reset = i('CONFIG_CUSTOM_CAM_PIN_RESET');
    s.camera.use_pwdn  = y('CONFIG_CUSTOM_CAM_USE_PWDN');
    s.camera.pin_pwdn  = i('CONFIG_CUSTOM_CAM_PIN_PWDN');
    for (let n=0;n<=7;n++) s.camera[`pin_d${n}`]=i(`CONFIG_CUSTOM_CAM_PIN_D${n}`);
    s.camera.hmirror = y('CONFIG_CUSTOM_CAM_HMIRROR');
    s.camera.vflip   = y('CONFIG_CUSTOM_CAM_VFLIP');
    s.led.enable = y('CONFIG_ENABLE_CUSTOM_LEDS');
    const LTYPES = ['WS2812','SINGLE_PWM','CIRCULAR_STRIP','USER_CUSTOM','NONE'];
    const ltype = ch(LTYPES.map(t=>`CONFIG_CUSTOM_LED_${t}`));
    if (ltype) s.led.type = ltype.replace('CONFIG_','');
    s.led.gpio    = i('CONFIG_CUSTOM_LED_GPIO');
    s.led.count   = i('CONFIG_CUSTOM_LED_COUNT', 1);
    s.led.rainbow = 'CONFIG_CUSTOM_LED_FAST_RAINBOW_EFFECT' in m
                    ? y('CONFIG_CUSTOM_LED_FAST_RAINBOW_EFFECT') : true;
    s.mcp.enable    = y('CONFIG_ENABLE_CUSTOM_MCP_SERVER');
    s.mcp.lamp      = 'CONFIG_CUSTOM_MCP_TOOL_LAMP' in m
                      ? y('CONFIG_CUSTOM_MCP_TOOL_LAMP') : true;
    s.mcp.lamp_gpio = i('CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO');
    s.mcp.sensor    = 'CONFIG_CUSTOM_MCP_TOOL_SENSOR' in m
                      ? y('CONFIG_CUSTOM_MCP_TOOL_SENSOR') : true;
    s.mcp.actuator  = 'CONFIG_CUSTOM_MCP_TOOL_ACTUATOR' in m
                      ? y('CONFIG_CUSTOM_MCP_TOOL_ACTUATOR') : true;
    s.uart.enable     = y('CONFIG_ENABLE_CUSTOM_UART');
    s.uart.port       = y('CONFIG_CUSTOM_UART_PORT_2')
                        ? 'CUSTOM_UART_PORT_2' : 'CUSTOM_UART_PORT_1';
    s.uart.pin_tx     = i('CONFIG_CUSTOM_UART_PIN_TX');
    s.uart.pin_rx     = i('CONFIG_CUSTOM_UART_PIN_RX');
    s.uart.baudrate   = i('CONFIG_CUSTOM_UART_BAUDRATE', 115200);
    s.uart.flow_ctrl  = y('CONFIG_CUSTOM_UART_USE_FLOW_CONTROL');
    s.uart.pin_rts    = i('CONFIG_CUSTOM_UART_PIN_RTS');
    s.uart.pin_cts    = i('CONFIG_CUSTOM_UART_PIN_CTS');
    s.actuators.servo_enable  = y('CONFIG_CUSTOM_ENABLE_SERVO_DOG');
    s.actuators.servo_gpio    = i('CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO');
    s.actuators.buzzer_enable = y('CONFIG_ENABLE_BUZZER');
    s.actuators.buzzer_gpio   = i('CONFIG_BUZZER_PIN');
    s.actuators.haptic_enable = y('CONFIG_ENABLE_HAPTIC_MOTOR');
    s.actuators.haptic_gpio   = i('CONFIG_HAPTIC_PIN');
    s.motor.relay_enable = y('CONFIG_CUSTOM_PERIPH_RELAY_ENABLE');
    s.motor.relay_gpio   = i('CONFIG_CUSTOM_PERIPH_RELAY_GPIO');
    s.motor.dc_enable    = y('CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE');
    const DRIV = ['TB6612','L9110S','DRV8833'];
    const driv = ch(DRIV.map(d=>`CONFIG_CUSTOM_MOTOR_DRIVER_${d}`));
    if (driv) s.motor.dc_driver = driv.replace('CONFIG_','');
    s.motor.pwma = i('CONFIG_CUSTOM_PERIPH_MOTOR_PWMA_PIN');
    s.motor.dira = i('CONFIG_CUSTOM_PERIPH_MOTOR_DIRA_PIN');
    s.motor.pwmb = i('CONFIG_CUSTOM_PERIPH_MOTOR_PWMB_PIN');
    s.motor.dirb = i('CONFIG_CUSTOM_PERIPH_MOTOR_DIRB_PIN');
    s.sensors.dht_enable   = y('CONFIG_CUSTOM_ENABLE_SENSOR_DHT11_22');
    s.sensors.dht_gpio     = i('CONFIG_CUSTOM_SENSOR_DHT_GPIO');
    s.sensors.temp_enable  = y('CONFIG_CUSTOM_ENABLE_SENSOR_I2C_TEMP_HUMID');
    s.sensors.aht20        = 'CONFIG_CUSTOM_ENABLE_SENSOR_AHT20' in m
                             ? y('CONFIG_CUSTOM_ENABLE_SENSOR_AHT20') : true;
    s.sensors.temp_sda     = i('CONFIG_CUSTOM_SENSOR_I2C_SDA');
    s.sensors.temp_scl     = i('CONFIG_CUSTOM_SENSOR_I2C_SCL');
    s.sensors.bmp280       = y('CONFIG_CUSTOM_ENABLE_SENSOR_BMP280');
    s.sensors.bmp280_sda   = i('CONFIG_CUSTOM_SENSOR_BMP280_I2C_SDA');
    s.sensors.bmp280_scl   = i('CONFIG_CUSTOM_SENSOR_BMP280_I2C_SCL');
    s.sensors.bh1750       = y('CONFIG_CUSTOM_ENABLE_SENSOR_BH1750');
    s.sensors.bh1750_sda   = i('CONFIG_CUSTOM_SENSOR_BH1750_I2C_SDA');
    s.sensors.bh1750_scl   = i('CONFIG_CUSTOM_SENSOR_BH1750_I2C_SCL');
    s.sensors.ldr          = y('CONFIG_CUSTOM_ENABLE_SENSOR_LDR');
    s.sensors.ldr_ch       = i('CONFIG_CUSTOM_SENSOR_LDR_ADC_CHANNEL', 1);
    s.sensors.gas_co2      = y('CONFIG_CUSTOM_ENABLE_SENSOR_GAS_CO2');
    s.sensors.gas_scd4x    = 'CONFIG_CUSTOM_SENSOR_GAS_SCD4X' in m
                             ? y('CONFIG_CUSTOM_SENSOR_GAS_SCD4X') : true;
    s.sensors.gas_sda      = i('CONFIG_CUSTOM_SENSOR_GAS_I2C_SDA');
    s.sensors.gas_scl      = i('CONFIG_CUSTOM_SENSOR_GAS_I2C_SCL');
    s.sensors.gas_mq       = y('CONFIG_CUSTOM_ENABLE_SENSOR_GAS_ANALOG_MQ');
    s.sensors.gas_mq_gpio  = i('CONFIG_CUSTOM_SENSOR_MQ_ANALOG_PIN');
    s.sensors.hcsr04       = y('CONFIG_CUSTOM_ENABLE_SENSOR_HCSR04');
    s.sensors.hcsr04_trig  = i('CONFIG_CUSTOM_SENSOR_HCSR04_TRIG_GPIO');
    s.sensors.hcsr04_echo  = i('CONFIG_CUSTOM_SENSOR_HCSR04_ECHO_GPIO');
    s.sensors.pir          = y('CONFIG_CUSTOM_ENABLE_SENSOR_PIR');
    s.sensors.pir_gpio     = i('CONFIG_CUSTOM_SENSOR_PIR_GPIO');
    s.sensors.vibration    = y('CONFIG_CUSTOM_ENABLE_SENSOR_VIBRATION_SW420');
    s.sensors.vibration_gpio = i('CONFIG_CUSTOM_SENSOR_VIBRATION_PIN');
    s.sensors.flame        = y('CONFIG_CUSTOM_ENABLE_SENSOR_FLAME');
    s.sensors.flame_gpio   = i('CONFIG_CUSTOM_SENSOR_FLAME_PIN');
    s.power.battery_enable = y('CONFIG_CUSTOM_ENABLE_BATTERY_MONITOR');
    s.power.battery_adc    = 'CONFIG_CUSTOM_BATTERY_MONITOR_ADC' in m
                             ? y('CONFIG_CUSTOM_BATTERY_MONITOR_ADC') : true;
    s.power.battery_ch     = i('CONFIG_CUSTOM_BATTERY_ADC_CHANNEL', 0);
    s.power.battery_r1     = i('CONFIG_CUSTOM_BATTERY_DIVIDER_R1', 100);
    s.power.battery_r2     = i('CONFIG_CUSTOM_BATTERY_DIVIDER_R2', 100);
    s.power.ina2xx         = y('CONFIG_CUSTOM_ENABLE_SENSOR_INA2XX');
    s.power.ina219         = 'CONFIG_CUSTOM_SENSOR_INA219' in m
                             ? y('CONFIG_CUSTOM_SENSOR_INA219') : true;
    s.power.ina_sda        = i('CONFIG_CUSTOM_SENSOR_INA2XX_I2C_SDA');
    s.power.ina_scl        = i('CONFIG_CUSTOM_SENSOR_INA2XX_I2C_SCL');
    s.power.tp4056         = y('CONFIG_CUSTOM_ENABLE_PERIPH_BATTERY_CHARGING_DETECT');
    s.power.tp4056_gpio    = i('CONFIG_CUSTOM_PERIPH_BATTERY_CHRG_PIN');
    SdkconfigParser._restoreMissing(s, DEFAULT_CONFIG);
    return s;
  }
  static parse(text) {
    return SdkconfigParser.mapToState(SdkconfigParser.textToMap(text));
  }
}
