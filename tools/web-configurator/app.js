/**
 * Xiaozhi-ESP32 Web Configurator — app.js
 *
 * Architecture:  Web UI  ↔  configState (JS)  ↔  sdkconfig.defaults
 *
 * Luồng:
 *   sdkconfig.defaults  →  SdkconfigParser.parse()  →  configState
 *   configState         →  SdkconfigGenerator.build()  →  sdkconfig_lines[]
 *   POST /api/save  { sdkconfig_lines }  →  server ghi sdkconfig.defaults
 *
 * Board: esp32s3-n16r8-custom  (main/boards/esp32s3-n16r8-custom)
 * Không tạo config.h / board.cc — giống hệt menuconfig.
 */

'use strict';

// ============================================================================
// HARDWARE RULES  ESP32-S3-N16R8
// ============================================================================
const ESP32S3 = {
  MIN: 0, MAX: 48,
  // GPIO 26-37: dành riêng Octal Flash + Octal PSRAM, TUYỆT ĐỐI không dùng
  LOCKED: [26,27,28,29,30,31,32,33,34,35,36,37],
  STRAP:  [0, 3, 45, 46],
};

// ============================================================================
// DEFAULT STATE  — mirror Kconfig defaults
// ============================================================================
const DEFAULT_CONFIG = {

  // ── 0. System / Flash / PSRAM ──────────────────────────────────────────
  system: {
    cpu_freq:        "240",
    flash_size:      "16MB",
    flash_mode:      "QIO",
    flash_freq:      "80M",
    spiram:          true,
    spiram_mode:     "OCT",
    spiram_speed:    "80M",
    partition_table: "partitions/16m.csv",
  },

  // ── 1. Cơ bản & Ngôn ngữ ───────────────────────────────────────────────
  general: {
    language:          "LANGUAGE_VI_VN",
    ota_url:           "https://api.tenclass.net/xiaozhi/ota/",
    flash_assets:      "FLASH_DEFAULT_ASSETS",
    custom_assets_file:"https://github.com/78/xiaozhi-esp32/releases/download/v0.9/assets.bin",
    display_style:     "USE_DEFAULT_MESSAGE_STYLE",
    multiline_chat:    false,
  },

  // ── 2. Màn hình ────────────────────────────────────────────────────────
  display: {
    enable:           false,
    type:             "CUSTOM_DISPLAY_ST7789",
    width:            240,  height: 320,
    pin_mosi: -1, pin_clk: -1, pin_cs: -1, pin_dc: -1,
    use_rst:  true,  pin_rst: -1,
    use_blk:  true,  pin_blk: -1,
    pin_i2c_sda: -1, pin_i2c_scl: -1,
    offset_x: 0,  offset_y: 0,
    mirror_x: false, mirror_y: false, swap_xy: false, invert_color: false,
    // UART display
    uart_secondary:  false,
    uart_port:       1,
    uart_tx:        -1,   uart_rx: -1,
    uart_baud:       115200,
    uart_proto:      "CUSTOM_DISPLAY_UART_PROTO_NEXTION",
    // AMOLED QSPI
    amoled_chip:     "CUSTOM_DISPLAY_AMOLED_SH8601",
    qspi_cs:-1, qspi_clk:-1, qspi_d0:-1, qspi_d1:-1,
    qspi_d2:-1, qspi_d3:-1, qspi_rst:-1,
    // E-Paper
    epaper_busy: -1,
    // RGB ST7701
    rgb_pclk:-1, rgb_de:-1, rgb_vsync:-1, rgb_hsync:-1,
    rgb_d0:-1,  rgb_d1:-1,  rgb_d2:-1,  rgb_d3:-1,
    rgb_d4:-1,  rgb_d5:-1,  rgb_d6:-1,  rgb_d7:-1,
    rgb_d8:-1,  rgb_d9:-1,  rgb_d10:-1, rgb_d11:-1,
    rgb_d12:-1, rgb_d13:-1, rgb_d14:-1, rgb_d15:-1,
  },

  // ── 3. Cảm ứng ─────────────────────────────────────────────────────────
  touch: {
    enable:   false,
    type:     "CUSTOM_TOUCH_CST816S",
    pin_sda: -1, pin_scl: -1, pin_int: -1, pin_rst: -1,
  },

  // ── 4. Loa (Speaker) ───────────────────────────────────────────────────
  speaker: {
    enable:        false,
    type:          "CUSTOM_AUDIO_DAC_MAX98357A",
    pin_dout:     -1, pin_bclk: -1, pin_lrck: -1,
    codec_sda:    -1, codec_scl: -1,
  },

  // ── 5. Micro ───────────────────────────────────────────────────────────
  mic: {
    enable:        false,
    type:          "CUSTOM_AUDIO_MIC_INMP441",
    pin_din:      -1, pin_sck: -1, pin_ws: -1,
    codec_sda:    -1, codec_scl: -1,
  },

  // ── 6. I2S mode ────────────────────────────────────────────────────────
  audio: {
    i2s_mode: "CUSTOM_AUDIO_I2S_SIMPLEX",
  },

  // ── 7. Wake Word ───────────────────────────────────────────────────────
  wakeword: {
    type:                  "USE_AFE_WAKE_WORD",
    custom_word:           "xiao tu dou",
    custom_display:        "小土豆",
    threshold:             20,
    device_aec:            true,
    server_aec:            false,
    send_data:             true,
    detection_in_listening:false,
  },

  // ── 8. Wi-Fi ───────────────────────────────────────────────────────────
  wifi: {
    method: "USE_HOTSPOT_WIFI_PROVISIONING",
  },

  // ── 9. Phím bấm & Input ────────────────────────────────────────────────
  buttons: {
    boot_enable:   true, boot_gpio:     0,
    touch_enable:  false, touch_gpio:    -1,
    vol_enable:    false, vol_up_gpio:   -1, vol_down_gpio: -1,
    slider_enable: false,
    slider_pad1:  -1, slider_pad2: -1, slider_pad3: -1,
    rotary_enable: false,
    rotary_a:     -1, rotary_b: -1, rotary_key: -1,
    user_custom_sensors: false,
  },

  // ── 10. Camera ─────────────────────────────────────────────────────────
  camera: {
    enable:    false,
    sensor:    "CUSTOM_CAMERA_OV2640",
    pin_xclk: -1, pin_pclk: -1, pin_vsync: -1, pin_href: -1,
    pin_siod: -1, pin_sioc: -1,
    use_reset: false, pin_reset: -1,
    use_pwdn:  false, pin_pwdn:  -1,
    pin_d0:   -1, pin_d1: -1, pin_d2: -1, pin_d3: -1,
    pin_d4:   -1, pin_d5: -1, pin_d6: -1, pin_d7: -1,
    hmirror:  false, vflip: false,
  },

  // ── 11. LED ────────────────────────────────────────────────────────────
  led: {
    enable:  true,
    type:    "CUSTOM_LED_WS2812",
    gpio:   48,
    count:   1,
    rainbow: true,
  },

  // ── 12. MCP Server ─────────────────────────────────────────────────────
  mcp: {
    enable:   false,
    lamp:     true, lamp_gpio: -1,
    sensor:   true,
    actuator: true,
  },

  // ── 13. UART mở rộng ───────────────────────────────────────────────────
  uart: {
    enable:       false,
    port:         "CUSTOM_UART_PORT_1",
    pin_tx:      -1, pin_rx: -1,
    baudrate:     115200,
    flow_ctrl:    false,
    pin_rts:     -1, pin_cts: -1,
  },

  // ── 14. Servo / Buzzer / Haptic ────────────────────────────────────────
  actuators: {
    servo_enable:  false, servo_gpio:  -1,
    buzzer_enable: false, buzzer_gpio: -1,
    haptic_enable: false, haptic_gpio: -1,
  },

  // ── 15. Relay & Motor DC ───────────────────────────────────────────────
  motor: {
    relay_enable:   false, relay_gpio:  -1,
    dc_enable:      false,
    dc_driver:      "CUSTOM_MOTOR_DRIVER_TB6612",
    pwma: -1, dira: -1, pwmb: -1, dirb: -1,
  },

  // ── 16. Cảm biến môi trường ────────────────────────────────────────────
  sensors: {
    dht_enable:   false, dht_gpio:      -1,
    temp_enable:  false, aht20:         true,
    temp_sda:    -1,     temp_scl:      -1,
    bmp280:       false, bmp280_sda:   -1, bmp280_scl: -1,
    bh1750:       false, bh1750_sda:   -1, bh1750_scl: -1,
    ldr:          false, ldr_ch:        1,
    gas_co2:      false, gas_scd4x:     true,
    gas_sda:     -1,     gas_scl:      -1,
    gas_mq:       false, gas_mq_gpio:  -1,
    hcsr04:       false, hcsr04_trig:  -1, hcsr04_echo: -1,
    pir:          false, pir_gpio:     -1,
    vibration:    false, vibration_gpio:-1,
    flame:        false, flame_gpio:   -1,
  },

  // ── 17. Pin & Nguồn ────────────────────────────────────────────────────
  power: {
    battery_enable: false,
    battery_adc:    true,  battery_ch: 0,
    battery_r1:     100,   battery_r2: 100,
    ina2xx:         false, ina219: true,
    ina_sda:       -1,     ina_scl: -1,
    tp4056:         false, tp4056_gpio: -1,
  },
};

// ── Reactive state ────────────────────────────────────────────────────────────
let configState = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
let conflictErrors = [];
let isServerMode = false;
let serverProjectInfo = null;

function apiFetch(endpoint, options = {}) {
  let url = endpoint;
  if (typeof window !== 'undefined' && window.location.protocol === 'file:') {
    url = 'http://localhost:8080' + endpoint;
  }
  return fetch(url, options);
}

// ============================================================================
// SdkconfigParser  —  sdkconfig.defaults text  →  configState
// ============================================================================
class SdkconfigParser {

  /** Chuyển text thô thành dict { CONFIG_X → raw_value } */
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

  /** Helper — lấy bool (y) */
  static _y(m, k)         { return m[k] === 'y'; }
  /** Helper — lấy int với fallback */
  static _i(m, k, def=-1) {
    const value = parseInt(m[k], 10);
    return Number.isNaN(value) ? def : value;
  }
  /** Helper — lấy string, bỏ dấu ngoặc kép */
  static _s(m, k, def='') { return k in m ? m[k].replace(/^"|"$/g,'') : def; }
  /** Helper — một trong nhiều choice → trả key tên option đang =y */
  static _choice(m, keys) {
    for (const k of keys) if (m[k] === 'y') return k;
    return null;
  }

  /** Map → configState đầy đủ */
  static mapToState(m) {
    const s  = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
    const y  = (k)         => SdkconfigParser._y(m, k);
    const i  = (k, d=-1)   => SdkconfigParser._i(m, k, d);
    const st = (k, d='')   => SdkconfigParser._s(m, k, d);
    const ch = (keys)      => SdkconfigParser._choice(m, keys);

    // ── system ────────────────────────────────────────────────────────────
    s.system.spiram      = y('CONFIG_SPIRAM');
    s.system.spiram_mode = 'OCT';
    s.system.spiram_speed= y('CONFIG_SPIRAM_SPEED_80M') ? '80M' : '40M';
    s.system.flash_mode  = y('CONFIG_ESPTOOLPY_FLASHMODE_DIO') ? 'DIO' : 'QIO';
    s.system.flash_freq  = y('CONFIG_ESPTOOLPY_FLASHFREQ_40M') ? '40M' : '80M';
    s.system.cpu_freq    = y('CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_160') ? '160' : '240';
    if ('CONFIG_PARTITION_TABLE_CUSTOM_FILENAME' in m)
      s.system.partition_table = st('CONFIG_PARTITION_TABLE_CUSTOM_FILENAME');

    // ── general ───────────────────────────────────────────────────────────
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
    // Kconfig: USE_MULTILINE_CHAT_MESSAGE depends on USE_DEFAULT_MESSAGE_STYLE
    s.general.multiline_chat = (s.general.display_style === 'USE_DEFAULT_MESSAGE_STYLE') && y('CONFIG_USE_MULTILINE_CHAT_MESSAGE');

    // ── display ───────────────────────────────────────────────────────────
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
    // Kconfig: CUSTOM_DISPLAY_UART_BAUDRATE_CHOICE
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

    // ── touch ─────────────────────────────────────────────────────────────
    s.touch.enable = y('CONFIG_ENABLE_CUSTOM_TOUCH');
    const TTYPES = ['CST816S','GT911','FT5X06','USER_CUSTOM','NONE'];
    const ttype = ch(TTYPES.map(t=>`CONFIG_CUSTOM_TOUCH_${t}`));
    if (ttype) s.touch.type = ttype.replace('CONFIG_','');
    s.touch.pin_sda = i('CONFIG_CUSTOM_TOUCH_PIN_SDA');
    s.touch.pin_scl = i('CONFIG_CUSTOM_TOUCH_PIN_SCL');
    s.touch.pin_int = i('CONFIG_CUSTOM_TOUCH_PIN_INT');
    s.touch.pin_rst = i('CONFIG_CUSTOM_TOUCH_PIN_RST');

    // ── speaker ───────────────────────────────────────────────────────────
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

    // ── mic ───────────────────────────────────────────────────────────────
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

    // ── audio i2s ─────────────────────────────────────────────────────────
    s.audio.i2s_mode = y('CONFIG_CUSTOM_AUDIO_I2S_DUPLEX')
      ? 'CUSTOM_AUDIO_I2S_DUPLEX' : 'CUSTOM_AUDIO_I2S_SIMPLEX';

    // ── wakeword ──────────────────────────────────────────────────────────
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
    // Kconfig: WAKE_WORD_DETECTION_IN_LISTENING depends on USE_AFE_WAKE_WORD || USE_CUSTOM_WAKE_WORD
    s.wakeword.detection_in_listening = ['USE_AFE_WAKE_WORD','USE_CUSTOM_WAKE_WORD'].includes(s.wakeword.type) && y('CONFIG_WAKE_WORD_DETECTION_IN_LISTENING');

    // ── wifi ──────────────────────────────────────────────────────────────
    s.wifi.method = y('CONFIG_USE_ESP_BLUFI_WIFI_PROVISIONING')
      ? 'USE_ESP_BLUFI_WIFI_PROVISIONING' : 'USE_HOTSPOT_WIFI_PROVISIONING';

    // ── buttons ───────────────────────────────────────────────────────────
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

    // ── camera ────────────────────────────────────────────────────────────
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

    // ── led ───────────────────────────────────────────────────────────────
    s.led.enable = y('CONFIG_ENABLE_CUSTOM_LEDS');
    const LTYPES = ['WS2812','SINGLE_PWM','CIRCULAR_STRIP','USER_CUSTOM','NONE'];
    const ltype = ch(LTYPES.map(t=>`CONFIG_CUSTOM_LED_${t}`));
    if (ltype) s.led.type = ltype.replace('CONFIG_','');
    s.led.gpio    = i('CONFIG_CUSTOM_LED_GPIO');
    s.led.count   = i('CONFIG_CUSTOM_LED_COUNT', 1);
    s.led.rainbow = 'CONFIG_CUSTOM_LED_FAST_RAINBOW_EFFECT' in m
                    ? y('CONFIG_CUSTOM_LED_FAST_RAINBOW_EFFECT') : true;

    // ── mcp ───────────────────────────────────────────────────────────────
    s.mcp.enable    = y('CONFIG_ENABLE_CUSTOM_MCP_SERVER');
    s.mcp.lamp      = 'CONFIG_CUSTOM_MCP_TOOL_LAMP' in m
                      ? y('CONFIG_CUSTOM_MCP_TOOL_LAMP') : true;
    s.mcp.lamp_gpio = i('CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO');
    s.mcp.sensor    = 'CONFIG_CUSTOM_MCP_TOOL_SENSOR' in m
                      ? y('CONFIG_CUSTOM_MCP_TOOL_SENSOR') : true;
    s.mcp.actuator  = 'CONFIG_CUSTOM_MCP_TOOL_ACTUATOR' in m
                      ? y('CONFIG_CUSTOM_MCP_TOOL_ACTUATOR') : true;

    // ── uart ──────────────────────────────────────────────────────────────
    s.uart.enable     = y('CONFIG_ENABLE_CUSTOM_UART');
    s.uart.port       = y('CONFIG_CUSTOM_UART_PORT_2')
                        ? 'CUSTOM_UART_PORT_2' : 'CUSTOM_UART_PORT_1';
    s.uart.pin_tx     = i('CONFIG_CUSTOM_UART_PIN_TX');
    s.uart.pin_rx     = i('CONFIG_CUSTOM_UART_PIN_RX');
    s.uart.baudrate   = i('CONFIG_CUSTOM_UART_BAUDRATE', 115200);
    s.uart.flow_ctrl  = y('CONFIG_CUSTOM_UART_USE_FLOW_CONTROL');
    s.uart.pin_rts    = i('CONFIG_CUSTOM_UART_PIN_RTS');
    s.uart.pin_cts    = i('CONFIG_CUSTOM_UART_PIN_CTS');

    // ── actuators ─────────────────────────────────────────────────────────
    s.actuators.servo_enable  = y('CONFIG_CUSTOM_ENABLE_SERVO_DOG');
    s.actuators.servo_gpio    = i('CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO');
    s.actuators.buzzer_enable = y('CONFIG_ENABLE_BUZZER');
    s.actuators.buzzer_gpio   = i('CONFIG_BUZZER_PIN');
    s.actuators.haptic_enable = y('CONFIG_ENABLE_HAPTIC_MOTOR');
    s.actuators.haptic_gpio   = i('CONFIG_HAPTIC_PIN');

    // ── motor ─────────────────────────────────────────────────────────────
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

    // ── sensors ───────────────────────────────────────────────────────────
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

    // ── power ─────────────────────────────────────────────────────────────
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

    return s;
  }

  static parse(text) {
    return SdkconfigParser.mapToState(SdkconfigParser.textToMap(text));
  }
}

// ============================================================================
// SdkconfigGenerator  —  configState  →  sdkconfig_lines[]
// ============================================================================
class SdkconfigGenerator {

  static build(s) {
    const lines = [];
    const set   = (k, v) => lines.push(`${k}=${v}`);
    const en    = (k)    => lines.push(`${k}=y`);
    const dis   = (k)    => lines.push(`# ${k} is not set`);
    // choice helper: bật 1, tắt các option còn lại trong group
    const choice = (selected, allOptions) => {
      for (const opt of allOptions) {
        const key = `CONFIG_${opt}`;
        if (opt === selected) en(key); else dis(key);
      }
    };

    lines.push('# Generated by Xiaozhi Web Configurator');
    lines.push('# Equivalent to idf.py menuconfig output');
    lines.push('');

    // ── Board & Target (cố định) ──────────────────────────────────────────
    lines.push('# Board & Target');
    set('CONFIG_IDF_TARGET', '"esp32s3"');
    en('CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM');
    lines.push('');

    // ── ESP-IDF 6.1 Stability, Stack & Partition MD5 ──────────────────────
    lines.push('# ESP-IDF 6.1 System Settings');
    set('CONFIG_ESP_MAIN_TASK_STACK_SIZE', '12288');
    en('CONFIG_TOUCH_SUPPRESS_DEPRECATE_WARN');
    en('CONFIG_PARTITION_TABLE_MD5');
    lines.push('');

    // ── Partition table ───────────────────────────────────────────────────
    lines.push('# Partition table');
    en('CONFIG_PARTITION_TABLE_CUSTOM');
    set('CONFIG_PARTITION_TABLE_CUSTOM_FILENAME', `"${s.system.partition_table}"`);
    set('CONFIG_PARTITION_TABLE_FILENAME', `"${s.system.partition_table}"`);
    set('CONFIG_PARTITION_TABLE_OFFSET', '0x8000');
    lines.push('');

    // ── Flash & PSRAM ─────────────────────────────────────────────────────
    lines.push('# Flash & PSRAM');
    set('CONFIG_ESPTOOLPY_FLASHSIZE_16MB', 'y');
    choice(`ESPTOOLPY_FLASHMODE_${s.system.flash_mode}`,
           ['ESPTOOLPY_FLASHMODE_QIO','ESPTOOLPY_FLASHMODE_DIO']);
    choice(`ESPTOOLPY_FLASHFREQ_${s.system.flash_freq}`,
           ['ESPTOOLPY_FLASHFREQ_80M','ESPTOOLPY_FLASHFREQ_40M']);
    if (s.system.spiram) {
      en('CONFIG_SPIRAM');
      choice('SPIRAM_MODE_OCT', ['SPIRAM_MODE_OCT']);
      choice(`SPIRAM_SPEED_${s.system.spiram_speed}`,
             ['SPIRAM_SPEED_80M','SPIRAM_SPEED_40M']);
    } else {
      dis('CONFIG_SPIRAM');
    }
    lines.push('');

    // ── CPU Freq & Cache ──────────────────────────────────────────────────
    lines.push('# CPU & Cache');
    choice(`ESP_DEFAULT_CPU_FREQ_MHZ_${s.system.cpu_freq}`,
           ['ESP_DEFAULT_CPU_FREQ_MHZ_240','ESP_DEFAULT_CPU_FREQ_MHZ_160']);
    en('CONFIG_ESP32S3_INSTRUCTION_CACHE_32KB');
    en('CONFIG_ESP32S3_DATA_CACHE_32KB');
    en('CONFIG_ESP32S3_DATA_CACHE_LINE_64B');
    lines.push('');

    // ── Language ──────────────────────────────────────────────────────────
    lines.push('# Language');
    const LANGS = ['LANGUAGE_ZH_CN','LANGUAGE_ZH_TW','LANGUAGE_EN_US',
      'LANGUAGE_JA_JP','LANGUAGE_KO_KR','LANGUAGE_VI_VN','LANGUAGE_TH_TH',
      'LANGUAGE_DE_DE','LANGUAGE_FR_FR','LANGUAGE_ES_ES','LANGUAGE_IT_IT',
      'LANGUAGE_RU_RU','LANGUAGE_AR_SA','LANGUAGE_HI_IN','LANGUAGE_MR_IN',
      'LANGUAGE_PT_PT','LANGUAGE_PT_BR','LANGUAGE_PL_PL','LANGUAGE_CS_CZ',
      'LANGUAGE_FI_FI','LANGUAGE_TR_TR','LANGUAGE_ID_ID','LANGUAGE_UK_UA',
      'LANGUAGE_RO_RO','LANGUAGE_BG_BG','LANGUAGE_CA_ES','LANGUAGE_DA_DK',
      'LANGUAGE_EL_GR','LANGUAGE_FA_IR','LANGUAGE_FIL_PH','LANGUAGE_HE_IL',
      'LANGUAGE_HR_HR','LANGUAGE_HU_HU','LANGUAGE_MS_MY','LANGUAGE_NB_NO',
      'LANGUAGE_NL_NL','LANGUAGE_SK_SK','LANGUAGE_SL_SI','LANGUAGE_SV_SE',
      'LANGUAGE_SR_RS'];
    choice(s.general.language, LANGS);
    if (s.general.ota_url)
      set('CONFIG_OTA_URL', `"${s.general.ota_url}"`);
    lines.push('');

    // ── Flash Assets ─────────────────────────────────────────────────────
    lines.push('# Flash Assets');
    choice(s.general.flash_assets,
           ['FLASH_DEFAULT_ASSETS','FLASH_CUSTOM_ASSETS',
            'FLASH_EXPRESSION_ASSETS','FLASH_NONE_ASSETS']);
    if (s.general.flash_assets === 'FLASH_CUSTOM_ASSETS')
      set('CONFIG_CUSTOM_ASSETS_FILE', `"${s.general.custom_assets_file}"`);
    lines.push('');

    // ── Display Style ────────────────────────────────────────────────────
    lines.push('# Display Style');
    choice(s.general.display_style,
           ['USE_DEFAULT_MESSAGE_STYLE','USE_WECHAT_MESSAGE_STYLE',
            'USE_EMOTE_MESSAGE_STYLE']);
    // Kconfig: USE_MULTILINE_CHAT_MESSAGE depends on USE_DEFAULT_MESSAGE_STYLE
    if (s.general.display_style === 'USE_DEFAULT_MESSAGE_STYLE' && s.general.multiline_chat) {
      en('CONFIG_USE_MULTILINE_CHAT_MESSAGE');
    } else {
      dis('CONFIG_USE_MULTILINE_CHAT_MESSAGE');
    }
    lines.push('');

    // ── Display ──────────────────────────────────────────────────────────
    lines.push('# Display');
    if (s.display.enable) {
      en('CONFIG_ENABLE_CUSTOM_DISPLAY');
      const DTYPES = ['CUSTOM_DISPLAY_ST7789','CUSTOM_DISPLAY_ST7796',
        'CUSTOM_DISPLAY_ST7735','CUSTOM_DISPLAY_ST7701','CUSTOM_DISPLAY_ILI9341',
        'CUSTOM_DISPLAY_ILI9486','CUSTOM_DISPLAY_GC9A01','CUSTOM_DISPLAY_GC9107',
        'CUSTOM_DISPLAY_NV3023','CUSTOM_DISPLAY_JD9853','CUSTOM_DISPLAY_OLED_SSD1306',
        'CUSTOM_DISPLAY_OLED_SH1106','CUSTOM_DISPLAY_QSPI_AMOLED',
        'CUSTOM_DISPLAY_EPAPER_SSD1681','CUSTOM_DISPLAY_UART',
        'CUSTOM_DISPLAY_USER_CUSTOM','CUSTOM_DISPLAY_NONE'];
      choice(s.display.type, DTYPES);

      set('CONFIG_CUSTOM_DISPLAY_WIDTH',  s.display.width);
      set('CONFIG_CUSTOM_DISPLAY_HEIGHT', s.display.height);

      const isSpi = ['CUSTOM_DISPLAY_ST7789','CUSTOM_DISPLAY_ST7796',
        'CUSTOM_DISPLAY_ST7735','CUSTOM_DISPLAY_ILI9341','CUSTOM_DISPLAY_ILI9486',
        'CUSTOM_DISPLAY_GC9A01','CUSTOM_DISPLAY_GC9107','CUSTOM_DISPLAY_NV3023',
        'CUSTOM_DISPLAY_JD9853','CUSTOM_DISPLAY_ST7701'].includes(s.display.type);
      const isOled = ['CUSTOM_DISPLAY_OLED_SSD1306',
                      'CUSTOM_DISPLAY_OLED_SH1106'].includes(s.display.type);
      const isQspi = s.display.type === 'CUSTOM_DISPLAY_QSPI_AMOLED';
      const isEpaper = s.display.type === 'CUSTOM_DISPLAY_EPAPER_SSD1681';
      const isRgb = s.display.type === 'CUSTOM_DISPLAY_ST7701';

      if (isSpi) {
        set('CONFIG_CUSTOM_DISPLAY_PIN_MOSI', s.display.pin_mosi);
        set('CONFIG_CUSTOM_DISPLAY_PIN_CLK',  s.display.pin_clk);
      }
      set('CONFIG_CUSTOM_DISPLAY_PIN_CS', s.display.pin_cs);
      set('CONFIG_CUSTOM_DISPLAY_PIN_DC', s.display.pin_dc);

      if (s.display.use_rst) {
        en('CONFIG_CUSTOM_DISPLAY_USE_RST');
        set('CONFIG_CUSTOM_DISPLAY_PIN_RST', s.display.pin_rst);
      } else dis('CONFIG_CUSTOM_DISPLAY_USE_RST');

      if (s.display.use_blk) {
        en('CONFIG_CUSTOM_DISPLAY_USE_BLK');
        set('CONFIG_CUSTOM_DISPLAY_PIN_BLK', s.display.pin_blk);
      } else dis('CONFIG_CUSTOM_DISPLAY_USE_BLK');

      if (isOled) {
        set('CONFIG_CUSTOM_DISPLAY_PIN_I2C_SDA', s.display.pin_i2c_sda);
        set('CONFIG_CUSTOM_DISPLAY_PIN_I2C_SCL', s.display.pin_i2c_scl);
      }
      set('CONFIG_CUSTOM_DISPLAY_OFFSET_X', s.display.offset_x);
      set('CONFIG_CUSTOM_DISPLAY_OFFSET_Y', s.display.offset_y);
      if (s.display.mirror_x) en('CONFIG_CUSTOM_DISPLAY_MIRROR_X');
      else dis('CONFIG_CUSTOM_DISPLAY_MIRROR_X');
      if (s.display.mirror_y) en('CONFIG_CUSTOM_DISPLAY_MIRROR_Y');
      else dis('CONFIG_CUSTOM_DISPLAY_MIRROR_Y');
      if (s.display.swap_xy)  en('CONFIG_CUSTOM_DISPLAY_SWAP_XY');
      else dis('CONFIG_CUSTOM_DISPLAY_SWAP_XY');
      if (s.display.invert_color) en('CONFIG_CUSTOM_DISPLAY_INVERT_COLOR');
      else dis('CONFIG_CUSTOM_DISPLAY_INVERT_COLOR');

      if (isQspi) {
        choice(s.display.amoled_chip,
               ['CUSTOM_DISPLAY_AMOLED_SH8601','CUSTOM_DISPLAY_AMOLED_CO5300',
                'CUSTOM_DISPLAY_AMOLED_SPD2010']);
        set('CONFIG_CUSTOM_DISPLAY_QSPI_CS',  s.display.qspi_cs);
        set('CONFIG_CUSTOM_DISPLAY_QSPI_CLK', s.display.qspi_clk);
        for (let n=0;n<=3;n++)
          set(`CONFIG_CUSTOM_DISPLAY_QSPI_D${n}`, s.display[`qspi_d${n}`]);
        set('CONFIG_CUSTOM_DISPLAY_QSPI_RST', s.display.qspi_rst);
      }
      if (isEpaper) set('CONFIG_CUSTOM_DISPLAY_EPAPER_PIN_BUSY', s.display.epaper_busy);
      if (isRgb) {
        set('CONFIG_CUSTOM_DISPLAY_RGB_PCLK',  s.display.rgb_pclk);
        set('CONFIG_CUSTOM_DISPLAY_RGB_DE',    s.display.rgb_de);
        set('CONFIG_CUSTOM_DISPLAY_RGB_VSYNC', s.display.rgb_vsync);
        set('CONFIG_CUSTOM_DISPLAY_RGB_HSYNC', s.display.rgb_hsync);
        for (let n=0;n<=15;n++)
          set(`CONFIG_CUSTOM_DISPLAY_RGB_D${n}`, s.display[`rgb_d${n}`]);
      }

      // UART display
      const showUart = s.display.uart_secondary ||
                       s.display.type === 'CUSTOM_DISPLAY_UART';
      if (s.display.uart_secondary) en('CONFIG_ENABLE_CUSTOM_SECONDARY_UART_DISPLAY');
      else dis('CONFIG_ENABLE_CUSTOM_SECONDARY_UART_DISPLAY');
      if (showUart) {
        set('CONFIG_CUSTOM_DISPLAY_UART_PORT',    s.display.uart_port);
        set('CONFIG_CUSTOM_DISPLAY_UART_TX_PIN',  s.display.uart_tx);
        set('CONFIG_CUSTOM_DISPLAY_UART_RX_PIN',  s.display.uart_rx);
        const baudChoices = ['9600','19200','38400','57600','115200','230400','460800','921600'];
        const curBaud = String(s.display.uart_baud || 115200);
        const matchedBaud = baudChoices.includes(curBaud) ? curBaud : '115200';
        choice(`CUSTOM_DISPLAY_UART_BAUDRATE_${matchedBaud}`,
               baudChoices.map(b => `CUSTOM_DISPLAY_UART_BAUDRATE_${b}`));
        set('CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE', matchedBaud);
        choice(s.display.uart_proto,
               ['CUSTOM_DISPLAY_UART_PROTO_NEXTION','CUSTOM_DISPLAY_UART_PROTO_JSON',
                'CUSTOM_DISPLAY_UART_PROTO_RAW_TEXT','CUSTOM_DISPLAY_UART_PROTO_DWIN']);
      }
    } else {
      dis('CONFIG_ENABLE_CUSTOM_DISPLAY');
      dis('CONFIG_ENABLE_CUSTOM_SECONDARY_UART_DISPLAY');
    }
    lines.push('');

    // ── Touch ─────────────────────────────────────────────────────────────
    lines.push('# Touch');
    if (s.touch.enable) {
      en('CONFIG_ENABLE_CUSTOM_TOUCH');
      choice(s.touch.type,
             ['CUSTOM_TOUCH_CST816S','CUSTOM_TOUCH_GT911','CUSTOM_TOUCH_FT5X06',
              'CUSTOM_TOUCH_USER_CUSTOM','CUSTOM_TOUCH_NONE']);
      set('CONFIG_CUSTOM_TOUCH_PIN_SDA', s.touch.pin_sda);
      set('CONFIG_CUSTOM_TOUCH_PIN_SCL', s.touch.pin_scl);
      set('CONFIG_CUSTOM_TOUCH_PIN_INT', s.touch.pin_int);
      set('CONFIG_CUSTOM_TOUCH_PIN_RST', s.touch.pin_rst);
    } else {
      dis('CONFIG_ENABLE_CUSTOM_TOUCH');
    }
    lines.push('');

    // ── Speaker ───────────────────────────────────────────────────────────
    lines.push('# Speaker');
    if (s.speaker.enable) {
      en('CONFIG_ENABLE_CUSTOM_SPEAKER');
      const STYPES = ['CUSTOM_AUDIO_DAC_MAX98357A','CUSTOM_AUDIO_DAC_MAX98360A',
        'CUSTOM_AUDIO_DAC_PCM5102A','CUSTOM_AUDIO_SPK_CODEC_ES8311',
        'CUSTOM_AUDIO_SPK_CODEC_ES8388','CUSTOM_AUDIO_SPK_CODEC_ES8374',
        'CUSTOM_AUDIO_SPK_CODEC_ES8389','CUSTOM_AUDIO_SPK_CODEC_BOX',
        'CUSTOM_AUDIO_SPK_CODEC_USER_CUSTOM'];
      choice(s.speaker.type, STYPES);
      set('CONFIG_CUSTOM_AUDIO_SPK_GPIO_DOUT', s.speaker.pin_dout);
      set('CONFIG_CUSTOM_AUDIO_SPK_GPIO_BCLK', s.speaker.pin_bclk);
      set('CONFIG_CUSTOM_AUDIO_SPK_GPIO_LRCK', s.speaker.pin_lrck);
      const isCodecSpk = s.speaker.type.includes('CODEC');
      if (isCodecSpk) {
        set('CONFIG_CUSTOM_AUDIO_SPK_CODEC_I2C_SDA', s.speaker.codec_sda);
        set('CONFIG_CUSTOM_AUDIO_SPK_CODEC_I2C_SCL', s.speaker.codec_scl);
      }
    } else {
      dis('CONFIG_ENABLE_CUSTOM_SPEAKER');
    }
    lines.push('');

    // ── Mic ───────────────────────────────────────────────────────────────
    lines.push('# Microphone');
    if (s.mic.enable) {
      en('CONFIG_ENABLE_CUSTOM_MIC');
      const MTYPES = ['CUSTOM_AUDIO_MIC_INMP441','CUSTOM_AUDIO_MIC_PDM',
        'CUSTOM_AUDIO_MIC_CODEC_ES8311','CUSTOM_AUDIO_MIC_CODEC_ES8388',
        'CUSTOM_AUDIO_MIC_CODEC_ES8374','CUSTOM_AUDIO_MIC_CODEC_ES8389',
        'CUSTOM_AUDIO_MIC_CODEC_BOX','CUSTOM_AUDIO_MIC_CODEC_USER_CUSTOM'];
      choice(s.mic.type, MTYPES);
      set('CONFIG_CUSTOM_AUDIO_MIC_GPIO_DIN', s.mic.pin_din);
      set('CONFIG_CUSTOM_AUDIO_MIC_GPIO_SCK', s.mic.pin_sck);
      set('CONFIG_CUSTOM_AUDIO_MIC_GPIO_WS',  s.mic.pin_ws);
      const isCodecMic = s.mic.type.includes('CODEC');
      if (isCodecMic) {
        set('CONFIG_CUSTOM_AUDIO_MIC_CODEC_I2C_SDA', s.mic.codec_sda);
        set('CONFIG_CUSTOM_AUDIO_MIC_CODEC_I2C_SCL', s.mic.codec_scl);
      }
    } else {
      dis('CONFIG_ENABLE_CUSTOM_MIC');
    }
    lines.push('');

    // ── I2S mode ──────────────────────────────────────────────────────────
    lines.push('# I2S mode');
    choice(s.audio.i2s_mode,
           ['CUSTOM_AUDIO_I2S_SIMPLEX','CUSTOM_AUDIO_I2S_DUPLEX']);
    lines.push('');

    // ── Wake Word & Audio Processing ───────────────────────────────────────
    lines.push('# Wake Word & Audio Processing');
    choice(s.wakeword.type,
           ['USE_AFE_WAKE_WORD','USE_ESP_WAKE_WORD',
            'USE_CUSTOM_WAKE_WORD','WAKE_WORD_DISABLED']);
    if (s.wakeword.type === 'USE_CUSTOM_WAKE_WORD') {
      set('CONFIG_CUSTOM_WAKE_WORD',         `"${s.wakeword.custom_word}"`);
      set('CONFIG_CUSTOM_WAKE_WORD_DISPLAY', `"${s.wakeword.custom_display}"`);
      set('CONFIG_CUSTOM_WAKE_WORD_THRESHOLD', s.wakeword.threshold);
    }
    // Kconfig: USE_AUDIO_PROCESSOR depends on ESP32S3 && SPIRAM
    const hasAfe = s.wakeword.type === 'USE_AFE_WAKE_WORD' || s.wakeword.device_aec || s.wakeword.server_aec;
    if (s.system.spiram && hasAfe) {
      en('CONFIG_USE_AUDIO_PROCESSOR');
      if (s.wakeword.device_aec) en('CONFIG_USE_DEVICE_AEC');
      else dis('CONFIG_USE_DEVICE_AEC');
      if (s.wakeword.server_aec) en('CONFIG_USE_SERVER_AEC');
      else dis('CONFIG_USE_SERVER_AEC');
    } else {
      dis('CONFIG_USE_AUDIO_PROCESSOR');
      dis('CONFIG_USE_DEVICE_AEC');
      dis('CONFIG_USE_SERVER_AEC');
    }
    if (s.wakeword.send_data) en('CONFIG_SEND_WAKE_WORD_DATA');
    else dis('CONFIG_SEND_WAKE_WORD_DATA');
    // Kconfig: WAKE_WORD_DETECTION_IN_LISTENING depends on USE_AFE_WAKE_WORD || USE_CUSTOM_WAKE_WORD
    const canListen = ['USE_AFE_WAKE_WORD', 'USE_CUSTOM_WAKE_WORD'].includes(s.wakeword.type);
    if (canListen && s.wakeword.detection_in_listening) {
      en('CONFIG_WAKE_WORD_DETECTION_IN_LISTENING');
    } else {
      dis('CONFIG_WAKE_WORD_DETECTION_IN_LISTENING');
    }
    lines.push('');

    // ── Wi-Fi ─────────────────────────────────────────────────────────────
    lines.push('# Wi-Fi Provisioning');
    choice(s.wifi.method,
           ['USE_HOTSPOT_WIFI_PROVISIONING','USE_ESP_BLUFI_WIFI_PROVISIONING']);
    // Kconfig select: USE_ESP_BLUFI_WIFI_PROVISIONING selects BT_ENABLED, BT_BLE_42_FEATURES_SUPPORTED, BT_BLE_BLUFI_ENABLE
    if (s.wifi.method === 'USE_ESP_BLUFI_WIFI_PROVISIONING') {
      en('CONFIG_BT_ENABLED');
      en('CONFIG_BT_BLE_42_FEATURES_SUPPORTED');
      en('CONFIG_BT_BLE_BLUFI_ENABLE');
    } else {
      dis('CONFIG_BT_ENABLED');
      dis('CONFIG_BT_BLE_42_FEATURES_SUPPORTED');
      dis('CONFIG_BT_BLE_BLUFI_ENABLE');
    }
    lines.push('');

    // ── Buttons ───────────────────────────────────────────────────────────
    lines.push('# Buttons & Input');
    if (s.buttons.boot_enable) {
      en('CONFIG_CUSTOM_ENABLE_BUTTON_BOOT');
      set('CONFIG_CUSTOM_BUTTON_BOOT_GPIO', s.buttons.boot_gpio);
    } else dis('CONFIG_CUSTOM_ENABLE_BUTTON_BOOT');

    if (s.buttons.touch_enable) {
      en('CONFIG_CUSTOM_ENABLE_BUTTON_TOUCH');
      set('CONFIG_CUSTOM_BUTTON_TOUCH_GPIO', s.buttons.touch_gpio);
    } else dis('CONFIG_CUSTOM_ENABLE_BUTTON_TOUCH');

    if (s.buttons.vol_enable) {
      en('CONFIG_CUSTOM_ENABLE_BUTTON_VOLUME');
      set('CONFIG_CUSTOM_BUTTON_VOL_UP_GPIO',   s.buttons.vol_up_gpio);
      set('CONFIG_CUSTOM_BUTTON_VOL_DOWN_GPIO',  s.buttons.vol_down_gpio);
    } else dis('CONFIG_CUSTOM_ENABLE_BUTTON_VOLUME');

    if (s.buttons.slider_enable) {
      en('CONFIG_CUSTOM_ENABLE_TOUCH_SLIDER');
      set('CONFIG_CUSTOM_TOUCH_SLIDER_PAD1_GPIO', s.buttons.slider_pad1);
      set('CONFIG_CUSTOM_TOUCH_SLIDER_PAD2_GPIO', s.buttons.slider_pad2);
      set('CONFIG_CUSTOM_TOUCH_SLIDER_PAD3_GPIO', s.buttons.slider_pad3);
    } else dis('CONFIG_CUSTOM_ENABLE_TOUCH_SLIDER');

    if (s.buttons.rotary_enable) {
      en('CONFIG_CUSTOM_ENABLE_PERIPH_ROTARY_ENCODER');
      set('CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_A', s.buttons.rotary_a);
      set('CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_B', s.buttons.rotary_b);
      set('CONFIG_CUSTOM_PERIPH_ENCODER_KEY_PIN', s.buttons.rotary_key);
    } else dis('CONFIG_CUSTOM_ENABLE_PERIPH_ROTARY_ENCODER');

    if (s.buttons.user_custom_sensors) en('CONFIG_CUSTOM_ENABLE_USER_CUSTOM_SENSORS');
    else dis('CONFIG_CUSTOM_ENABLE_USER_CUSTOM_SENSORS');
    lines.push('');

    // ── Camera ────────────────────────────────────────────────────────────
    lines.push('# Camera');
    if (s.camera.enable) {
      en('CONFIG_ENABLE_CUSTOM_CAMERA');
      choice(s.camera.sensor,
             ['CUSTOM_CAMERA_OV2640','CUSTOM_CAMERA_OV3660','CUSTOM_CAMERA_OV5640',
              'CUSTOM_CAMERA_SC030IOT','CUSTOM_CAMERA_USB_UVC','CUSTOM_CAMERA_USER_CUSTOM']);
      if (s.camera.sensor !== 'CUSTOM_CAMERA_USB_UVC') {
        set('CONFIG_CUSTOM_CAM_PIN_XCLK',  s.camera.pin_xclk);
        set('CONFIG_CUSTOM_CAM_PIN_PCLK',  s.camera.pin_pclk);
        set('CONFIG_CUSTOM_CAM_PIN_VSYNC', s.camera.pin_vsync);
        set('CONFIG_CUSTOM_CAM_PIN_HREF',  s.camera.pin_href);
        set('CONFIG_CUSTOM_CAM_PIN_SIOD',  s.camera.pin_siod);
        set('CONFIG_CUSTOM_CAM_PIN_SIOC',  s.camera.pin_sioc);
        if (s.camera.use_reset) {
          en('CONFIG_CUSTOM_CAM_USE_RESET');
          set('CONFIG_CUSTOM_CAM_PIN_RESET', s.camera.pin_reset);
        } else dis('CONFIG_CUSTOM_CAM_USE_RESET');
        if (s.camera.use_pwdn) {
          en('CONFIG_CUSTOM_CAM_USE_PWDN');
          set('CONFIG_CUSTOM_CAM_PIN_PWDN', s.camera.pin_pwdn);
        } else dis('CONFIG_CUSTOM_CAM_USE_PWDN');
        for (let n=0;n<=7;n++)
          set(`CONFIG_CUSTOM_CAM_PIN_D${n}`, s.camera[`pin_d${n}`]);
        if (s.camera.hmirror) en('CONFIG_CUSTOM_CAM_HMIRROR');
        else dis('CONFIG_CUSTOM_CAM_HMIRROR');
        if (s.camera.vflip) en('CONFIG_CUSTOM_CAM_VFLIP');
        else dis('CONFIG_CUSTOM_CAM_VFLIP');
      }
    } else dis('CONFIG_ENABLE_CUSTOM_CAMERA');
    lines.push('');

    // ── LED ───────────────────────────────────────────────────────────────
    lines.push('# LED');
    if (s.led.enable) {
      en('CONFIG_ENABLE_CUSTOM_LEDS');
      choice(s.led.type,
             ['CUSTOM_LED_WS2812','CUSTOM_LED_SINGLE_PWM',
              'CUSTOM_LED_CIRCULAR_STRIP','CUSTOM_LED_USER_CUSTOM','CUSTOM_LED_NONE']);
      set('CONFIG_CUSTOM_LED_GPIO',  s.led.gpio);
      set('CONFIG_CUSTOM_LED_COUNT', s.led.count);
      if (['CUSTOM_LED_WS2812','CUSTOM_LED_CIRCULAR_STRIP'].includes(s.led.type)) {
        if (s.led.rainbow) en('CONFIG_CUSTOM_LED_FAST_RAINBOW_EFFECT');
        else dis('CONFIG_CUSTOM_LED_FAST_RAINBOW_EFFECT');
      }
    } else dis('CONFIG_ENABLE_CUSTOM_LEDS');
    lines.push('');

    // ── MCP Server ────────────────────────────────────────────────────────
    lines.push('# MCP Server');
    if (s.mcp.enable) {
      en('CONFIG_ENABLE_CUSTOM_MCP_SERVER');
      if (s.mcp.lamp) {
        en('CONFIG_CUSTOM_MCP_TOOL_LAMP');
        set('CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO', s.mcp.lamp_gpio);
      } else dis('CONFIG_CUSTOM_MCP_TOOL_LAMP');
      if (s.mcp.sensor)   en('CONFIG_CUSTOM_MCP_TOOL_SENSOR');
      else dis('CONFIG_CUSTOM_MCP_TOOL_SENSOR');
      if (s.mcp.actuator) en('CONFIG_CUSTOM_MCP_TOOL_ACTUATOR');
      else dis('CONFIG_CUSTOM_MCP_TOOL_ACTUATOR');
    } else dis('CONFIG_ENABLE_CUSTOM_MCP_SERVER');
    lines.push('');

    // ── UART ──────────────────────────────────────────────────────────────
    lines.push('# UART Extension');
    if (s.uart.enable) {
      en('CONFIG_ENABLE_CUSTOM_UART');
      choice(s.uart.port,
             ['CUSTOM_UART_PORT_1','CUSTOM_UART_PORT_2']);
      set('CONFIG_CUSTOM_UART_PORT', s.uart.port === 'CUSTOM_UART_PORT_2' ? 2 : 1);
      set('CONFIG_CUSTOM_UART_PIN_TX',   s.uart.pin_tx);
      set('CONFIG_CUSTOM_UART_PIN_RX',   s.uart.pin_rx);
      set('CONFIG_CUSTOM_UART_BAUDRATE', s.uart.baudrate);
      if (s.uart.flow_ctrl) {
        en('CONFIG_CUSTOM_UART_USE_FLOW_CONTROL');
        set('CONFIG_CUSTOM_UART_PIN_RTS', s.uart.pin_rts);
        set('CONFIG_CUSTOM_UART_PIN_CTS', s.uart.pin_cts);
      } else dis('CONFIG_CUSTOM_UART_USE_FLOW_CONTROL');
    } else dis('CONFIG_ENABLE_CUSTOM_UART');
    lines.push('');

    // ── Actuators ─────────────────────────────────────────────────────────
    lines.push('# Actuators (Servo / Buzzer / Haptic)');
    if (s.actuators.servo_enable) {
      en('CONFIG_CUSTOM_ENABLE_SERVO_DOG');
      set('CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO', s.actuators.servo_gpio);
    } else dis('CONFIG_CUSTOM_ENABLE_SERVO_DOG');
    if (s.actuators.buzzer_enable) {
      en('CONFIG_ENABLE_BUZZER');
      set('CONFIG_BUZZER_PIN', s.actuators.buzzer_gpio);
    } else dis('CONFIG_ENABLE_BUZZER');
    if (s.actuators.haptic_enable) {
      en('CONFIG_ENABLE_HAPTIC_MOTOR');
      set('CONFIG_HAPTIC_PIN', s.actuators.haptic_gpio);
    } else dis('CONFIG_ENABLE_HAPTIC_MOTOR');
    lines.push('');

    // ── Motor / Relay ─────────────────────────────────────────────────────
    lines.push('# Motor & Relay');
    if (s.motor.relay_enable) {
      en('CONFIG_CUSTOM_PERIPH_RELAY_ENABLE');
      set('CONFIG_CUSTOM_PERIPH_RELAY_GPIO', s.motor.relay_gpio);
    } else dis('CONFIG_CUSTOM_PERIPH_RELAY_ENABLE');
    if (s.motor.dc_enable) {
      en('CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE');
      choice(s.motor.dc_driver,
             ['CUSTOM_MOTOR_DRIVER_TB6612','CUSTOM_MOTOR_DRIVER_L9110S',
              'CUSTOM_MOTOR_DRIVER_DRV8833']);
      set('CONFIG_CUSTOM_PERIPH_MOTOR_PWMA_PIN', s.motor.pwma);
      set('CONFIG_CUSTOM_PERIPH_MOTOR_DIRA_PIN', s.motor.dira);
      set('CONFIG_CUSTOM_PERIPH_MOTOR_PWMB_PIN', s.motor.pwmb);
      set('CONFIG_CUSTOM_PERIPH_MOTOR_DIRB_PIN', s.motor.dirb);
    } else dis('CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE');
    lines.push('');

    // ── Sensors ───────────────────────────────────────────────────────────
    lines.push('# Sensors');
    if (s.sensors.dht_enable) {
      en('CONFIG_CUSTOM_ENABLE_SENSOR_DHT11_22');
      set('CONFIG_CUSTOM_SENSOR_DHT_GPIO', s.sensors.dht_gpio);
    } else dis('CONFIG_CUSTOM_ENABLE_SENSOR_DHT11_22');

    if (s.sensors.temp_enable) {
      en('CONFIG_CUSTOM_ENABLE_SENSOR_I2C_TEMP_HUMID');
      if (s.sensors.aht20) en('CONFIG_CUSTOM_ENABLE_SENSOR_AHT20');
      else dis('CONFIG_CUSTOM_ENABLE_SENSOR_AHT20');
      set('CONFIG_CUSTOM_SENSOR_I2C_SDA', s.sensors.temp_sda);
      set('CONFIG_CUSTOM_SENSOR_I2C_SCL', s.sensors.temp_scl);
    } else dis('CONFIG_CUSTOM_ENABLE_SENSOR_I2C_TEMP_HUMID');

    if (s.sensors.bmp280) {
      en('CONFIG_CUSTOM_ENABLE_SENSOR_BMP280');
      set('CONFIG_CUSTOM_SENSOR_BMP280_I2C_SDA', s.sensors.bmp280_sda);
      set('CONFIG_CUSTOM_SENSOR_BMP280_I2C_SCL', s.sensors.bmp280_scl);
    } else dis('CONFIG_CUSTOM_ENABLE_SENSOR_BMP280');

    if (s.sensors.bh1750) {
      en('CONFIG_CUSTOM_ENABLE_SENSOR_BH1750');
      set('CONFIG_CUSTOM_SENSOR_BH1750_I2C_SDA', s.sensors.bh1750_sda);
      set('CONFIG_CUSTOM_SENSOR_BH1750_I2C_SCL', s.sensors.bh1750_scl);
    } else dis('CONFIG_CUSTOM_ENABLE_SENSOR_BH1750');

    if (s.sensors.ldr) {
      en('CONFIG_CUSTOM_ENABLE_SENSOR_LDR');
      set('CONFIG_CUSTOM_SENSOR_LDR_ADC_CHANNEL', s.sensors.ldr_ch);
    } else dis('CONFIG_CUSTOM_ENABLE_SENSOR_LDR');

    if (s.sensors.gas_co2) {
      en('CONFIG_CUSTOM_ENABLE_SENSOR_GAS_CO2');
      if (s.sensors.gas_scd4x) en('CONFIG_CUSTOM_SENSOR_GAS_SCD4X');
      else dis('CONFIG_CUSTOM_SENSOR_GAS_SCD4X');
      set('CONFIG_CUSTOM_SENSOR_GAS_I2C_SDA', s.sensors.gas_sda);
      set('CONFIG_CUSTOM_SENSOR_GAS_I2C_SCL', s.sensors.gas_scl);
    } else dis('CONFIG_CUSTOM_ENABLE_SENSOR_GAS_CO2');

    if (s.sensors.gas_mq) {
      en('CONFIG_CUSTOM_ENABLE_SENSOR_GAS_ANALOG_MQ');
      set('CONFIG_CUSTOM_SENSOR_MQ_ANALOG_PIN', s.sensors.gas_mq_gpio);
    } else dis('CONFIG_CUSTOM_ENABLE_SENSOR_GAS_ANALOG_MQ');

    if (s.sensors.hcsr04) {
      en('CONFIG_CUSTOM_ENABLE_SENSOR_HCSR04');
      set('CONFIG_CUSTOM_SENSOR_HCSR04_TRIG_GPIO', s.sensors.hcsr04_trig);
      set('CONFIG_CUSTOM_SENSOR_HCSR04_ECHO_GPIO', s.sensors.hcsr04_echo);
    } else dis('CONFIG_CUSTOM_ENABLE_SENSOR_HCSR04');

    if (s.sensors.pir) {
      en('CONFIG_CUSTOM_ENABLE_SENSOR_PIR');
      set('CONFIG_CUSTOM_SENSOR_PIR_GPIO', s.sensors.pir_gpio);
    } else dis('CONFIG_CUSTOM_ENABLE_SENSOR_PIR');

    if (s.sensors.vibration) {
      en('CONFIG_CUSTOM_ENABLE_SENSOR_VIBRATION_SW420');
      set('CONFIG_CUSTOM_SENSOR_VIBRATION_PIN', s.sensors.vibration_gpio);
    } else dis('CONFIG_CUSTOM_ENABLE_SENSOR_VIBRATION_SW420');

    if (s.sensors.flame) {
      en('CONFIG_CUSTOM_ENABLE_SENSOR_FLAME');
      set('CONFIG_CUSTOM_SENSOR_FLAME_PIN', s.sensors.flame_gpio);
    } else dis('CONFIG_CUSTOM_ENABLE_SENSOR_FLAME');

    en('CONFIG_ENABLE_CUSTOM_SENSORS');  // compat flag
    lines.push('');

    // ── Power & Battery ───────────────────────────────────────────────────
    lines.push('# Power & Battery');
    if (s.power.battery_enable) {
      en('CONFIG_CUSTOM_ENABLE_BATTERY_MONITOR');
      if (s.power.battery_adc) {
        en('CONFIG_CUSTOM_BATTERY_MONITOR_ADC');
        set('CONFIG_CUSTOM_BATTERY_ADC_CHANNEL', s.power.battery_ch);
        set('CONFIG_CUSTOM_BATTERY_DIVIDER_R1',  s.power.battery_r1);
        set('CONFIG_CUSTOM_BATTERY_DIVIDER_R2',  s.power.battery_r2);
      } else dis('CONFIG_CUSTOM_BATTERY_MONITOR_ADC');
    } else {
      dis('CONFIG_CUSTOM_ENABLE_BATTERY_MONITOR');
      en('CONFIG_CUSTOM_BATTERY_MONITOR_NONE');
    }

    if (s.power.ina2xx) {
      en('CONFIG_CUSTOM_ENABLE_SENSOR_INA2XX');
      if (s.power.ina219) en('CONFIG_CUSTOM_SENSOR_INA219');
      else dis('CONFIG_CUSTOM_SENSOR_INA219');
      set('CONFIG_CUSTOM_SENSOR_INA2XX_I2C_SDA', s.power.ina_sda);
      set('CONFIG_CUSTOM_SENSOR_INA2XX_I2C_SCL', s.power.ina_scl);
    } else dis('CONFIG_CUSTOM_ENABLE_SENSOR_INA2XX');

    if (s.power.tp4056) {
      en('CONFIG_CUSTOM_ENABLE_PERIPH_BATTERY_CHARGING_DETECT');
      set('CONFIG_CUSTOM_PERIPH_BATTERY_CHRG_PIN', s.power.tp4056_gpio);
    } else dis('CONFIG_CUSTOM_ENABLE_PERIPH_BATTERY_CHARGING_DETECT');
    lines.push('');

    return lines;
  }
}

// ============================================================================
// PinValidator  —  kiểm tra xung đột GPIO
// ============================================================================
class PinValidator {
  static validate(s) {
    const errors = [];
    const usage  = new Map(); // pin → [fieldName]

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

    // display
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
    // touch
    if (s.touch.enable) {
      reg(s.touch.pin_sda,'Touch SDA','i2c'); reg(s.touch.pin_scl,'Touch SCL','i2c');
      reg(s.touch.pin_int,'Touch INT');       reg(s.touch.pin_rst,'Touch RST');
    }
    // speaker
    if (s.speaker.enable) {
      reg(s.speaker.pin_dout,'SPK DOUT'); reg(s.speaker.pin_bclk,'SPK BCLK');
      reg(s.speaker.pin_lrck,'SPK LRCK');
      reg(s.speaker.codec_sda,'SPK Codec SDA','i2c');
      reg(s.speaker.codec_scl,'SPK Codec SCL','i2c');
    }
    // mic
    if (s.mic.enable) {
      reg(s.mic.pin_din,'MIC DIN'); reg(s.mic.pin_sck,'MIC SCK');
      reg(s.mic.pin_ws, 'MIC WS');
      reg(s.mic.codec_sda,'MIC Codec SDA','i2c');
      reg(s.mic.codec_scl,'MIC Codec SCL','i2c');
    }
    // buttons
    if (s.buttons.boot_enable)  reg(s.buttons.boot_gpio,  'Boot BTN');
    if (s.buttons.touch_enable) reg(s.buttons.touch_gpio, 'Touch BTN');
    if (s.buttons.vol_enable)   {
      reg(s.buttons.vol_up_gpio,'Vol Up'); reg(s.buttons.vol_down_gpio,'Vol Down');
    }
    if (s.buttons.rotary_enable) {
      reg(s.buttons.rotary_a,'Rotary A'); reg(s.buttons.rotary_b,'Rotary B');
      reg(s.buttons.rotary_key,'Rotary Key');
    }
    // led
    if (s.led.enable) reg(s.led.gpio,'LED');
    // mcp
    if (s.mcp.enable && s.mcp.lamp) reg(s.mcp.lamp_gpio,'MCP Lamp');
    // uart
    if (s.uart.enable) {
      reg(s.uart.pin_tx,'UART TX'); reg(s.uart.pin_rx,'UART RX');
    }
    // actuators
    if (s.actuators.servo_enable)  reg(s.actuators.servo_gpio, 'Servo');
    if (s.actuators.buzzer_enable) reg(s.actuators.buzzer_gpio,'Buzzer');
    if (s.actuators.haptic_enable) reg(s.actuators.haptic_gpio,'Haptic');
    // motor
    if (s.motor.relay_enable) reg(s.motor.relay_gpio,'Relay');
    if (s.motor.dc_enable) {
      reg(s.motor.pwma,'Motor PWMA'); reg(s.motor.dira,'Motor DIRA');
      reg(s.motor.pwmb,'Motor PWMB'); reg(s.motor.dirb,'Motor DIRB');
    }
    // sensors with pins
    if (s.sensors.dht_enable)    reg(s.sensors.dht_gpio,     'DHT');
    if (s.sensors.pir)           reg(s.sensors.pir_gpio,     'PIR');
    if (s.sensors.vibration)     reg(s.sensors.vibration_gpio,'Vibration');
    if (s.sensors.flame)         reg(s.sensors.flame_gpio,   'Flame');
    if (s.sensors.gas_mq)        reg(s.sensors.gas_mq_gpio,  'MQ Gas');
    if (s.sensors.hcsr04) {
      reg(s.sensors.hcsr04_trig,'HCSR04 Trig');
      reg(s.sensors.hcsr04_echo,'HCSR04 Echo');
    }
    // power
    if (s.power.tp4056) reg(s.power.tp4056_gpio,'TP4056');

    // conflict detection
    for (const [pin, users] of usage.entries()) {
      if (users.length > 1) {
        const allI2c = users.every(u => u.busType === 'i2c');
        if (allI2c) continue; // dùng chung I2C bus hợp lệ
        const names = users.map(u => u.name).join(' & ');
        errors.push({ pin, message: `Xung đột GPIO ${pin}: ${names}` });
      }
    }
    return { errors, usage };
  }
}

// ============================================================================
// BuildController  —  Quản lý Build, Flash, Nhận diện IDF & Target chip
// ============================================================================
class BuildController {
  static logOffset = 0;
  static pollTimer = null;
  static isPolling = false;
  static currentStatus = null;

  static init() {
    BuildController.bindEvents();
    // Tự động kiểm tra trạng thái ngay khi khởi động
    setTimeout(() => BuildController.fetchStatus(), 600);
  }

  static onTabActivated() {
    BuildController.fetchStatus();
  }

  static bindEvents() {
    // 1. Quét cổng COM
    document.getElementById('btn-refresh-ports')?.addEventListener('click', async () => {
      await BuildController.refreshPorts();
      GUIController.showToast('Đã làm mới danh sách cổng COM', 'info');
    });

    // 2. Set Target esp32s3
    document.getElementById('btn-quick-set-target')?.addEventListener('click', async () => {
      await BuildController.triggerAction('set-target', { target: 'esp32s3' });
    });

    // 3. Biên dịch (Build)
    document.getElementById('btn-idf-build')?.addEventListener('click', async () => {
      await BuildController.triggerAction('build');
    });

    // 4. Nạp Chip (Flash)
    document.getElementById('btn-idf-flash')?.addEventListener('click', async () => {
      const port = document.getElementById('serial-port-select')?.value;
      const baud = document.getElementById('serial-baud-select')?.value || '460800';
      if (!port) {
        GUIController.showToast('Vui lòng chọn cổng COM trước khi nạp chip!', 'warning');
        return;
      }
      await BuildController.triggerAction('flash', { port, baud });
    });

    // 5. Biên dịch & Nạp ngay (Build & Flash)
    document.getElementById('btn-idf-build-flash')?.addEventListener('click', async () => {
      const port = document.getElementById('serial-port-select')?.value;
      const baud = document.getElementById('serial-baud-select')?.value || '460800';
      if (!port) {
        GUIController.showToast('Vui lòng chọn cổng COM trước khi nạp chip!', 'warning');
        return;
      }
      await BuildController.triggerAction('build-flash', { port, baud });
    });

    // 6. Clean build
    document.getElementById('btn-idf-clean')?.addEventListener('click', async () => {
      if (confirm('Bạn có chắc chắn muốn dọn dẹp thư mục build (clean)?')) {
        await BuildController.triggerAction('clean');
      }
    });

    // 7. Hủy tiến trình (Cancel)
    document.getElementById('btn-idf-cancel')?.addEventListener('click', async () => {
      try {
        const res = await apiFetch('/api/idf/cancel', { method: 'POST' });
        const data = await res.json();
        GUIController.showToast(data.message || 'Đã gửi lệnh dừng tiến trình', 'warning');
      } catch (e) {
        GUIController.showToast('Lỗi gửi lệnh dừng: ' + e.message, 'error');
      }
    });

    // 8. Sao chép nhật ký
    document.getElementById('btn-copy-logs')?.addEventListener('click', () => {
      const term = document.getElementById('terminal-body');
      if (term) {
        navigator.clipboard.writeText(term.textContent).then(() => {
          GUIController.showToast('Đã sao chép nhật ký vào bộ nhớ tạm', 'success');
        }).catch(() => {
          GUIController.showToast('Không thể sao chép nhật ký', 'error');
        });
      }
    });

    // 9. Xóa nhật ký
    document.getElementById('btn-clear-logs')?.addEventListener('click', async () => {
      const term = document.getElementById('terminal-body');
      if (term) {
        term.innerHTML = '<span class="term-dim">=== Nhật ký đã được xóa ===</span>\n';
      }
      BuildController.logOffset = 0;
      try {
        await apiFetch('/api/idf/clear-logs', { method: 'POST' });
      } catch (e) { }
    });

    // 10. Toggle panel tùy chỉnh / dò tìm IDF
    document.getElementById('btn-toggle-idf-custom')?.addEventListener('click', () => {
      const panel = document.getElementById('idf-custom-panel');
      if (panel) {
        const isHidden = panel.style.display === 'none' || !panel.style.display;
        panel.style.display = isHidden ? 'block' : 'none';
      }
    });

    // 11. Áp dụng đường dẫn IDF tùy chỉnh
    document.getElementById('btn-apply-idf-path')?.addEventListener('click', async () => {
      const input = document.getElementById('custom-idf-input');
      const val = input?.value?.trim();
      if (!val) {
        GUIController.showToast('Vui lòng nhập đường dẫn thư mục ESP-IDF!', 'warning');
        return;
      }
      await BuildController.applyCustomPath(val);
    });

    // 12. Quét toàn bộ ổ đĩa máy tính
    document.getElementById('btn-scan-all-idf')?.addEventListener('click', async () => {
      await BuildController.scanAllIdf();
    });

    // 13. Khôi phục mặc định tự động dò tìm
    document.getElementById('btn-reset-idf-path')?.addEventListener('click', async () => {
      await BuildController.resetCustomPath();
    });
  }

  static async applyCustomPath(customPath) {
    try {
      const res = await apiFetch('/api/idf/set-path', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ path: customPath })
      });
      const data = await res.json();
      if (data.success) {
        GUIController.showToast(data.message || 'Đã áp dụng đường dẫn ESP-IDF', 'success');
        await BuildController.fetchStatus();
      } else {
        GUIController.showToast(data.error || 'Đường dẫn không hợp lệ', 'error');
      }
    } catch (e) {
      GUIController.showToast('Lỗi gửi yêu cầu: ' + e.message, 'error');
    }
  }

  static async resetCustomPath() {
    try {
      const res = await apiFetch('/api/idf/reset-path', { method: 'POST' });
      const data = await res.json();
      if (data.success) {
        GUIController.showToast(data.message || 'Đã khôi phục chế độ tự động', 'info');
        const input = document.getElementById('custom-idf-input');
        if (input) input.value = '';
        await BuildController.fetchStatus();
      }
    } catch (e) {
      GUIController.showToast('Lỗi khôi phục mặc định: ' + e.message, 'error');
    }
  }

  static async scanAllIdf() {
    const loading = document.getElementById('idf-scan-loading');
    const container = document.getElementById('idf-scan-results');
    if (loading) loading.style.display = 'block';
    if (container) {
      container.style.display = 'none';
      container.innerHTML = '';
    }

    try {
      const res = await apiFetch('/api/idf/scan-all', { cache: 'no-store' });
      const data = await res.json();
      if (loading) loading.style.display = 'none';

      if (container && data.success && Array.isArray(data.installations)) {
        container.style.display = 'flex';
        if (data.installations.length === 0) {
          container.innerHTML = '<span style="font-size: 0.74rem; color: var(--text-muted); padding: 4px;">Không tìm thấy bản cài đặt ESP-IDF nào tự động trên máy tính. Hãy nhập đường dẫn thủ công phía trên.</span>';
          return;
        }

        data.installations.forEach(item => {
          const row = document.createElement('div');
          row.className = 'idf-scan-item';
          row.innerHTML = `
            <div class="idf-scan-item-info">
              <span class="idf-scan-path" title="${item.path}">${item.path}</span>
              <div class="idf-scan-meta">
                <span class="text-cyan font-bold">${item.version}</span>
                <span class="idf-scan-source-pill">${item.source}</span>
              </div>
            </div>
            <button type="button" class="btn btn-outline btn-xs btn-pick-idf" title="Chọn đường dẫn này làm ESP-IDF hoạt động">
              Chọn
            </button>
          `;
          row.querySelector('.btn-pick-idf')?.addEventListener('click', async () => {
            const input = document.getElementById('custom-idf-input');
            if (input) input.value = item.path;
            await BuildController.applyCustomPath(item.path);
          });
          container.appendChild(row);
        });
        GUIController.showToast(`Tìm thấy ${data.installations.length} phiên bản ESP-IDF`, 'success');
      }
    } catch (e) {
      if (loading) loading.style.display = 'none';
      GUIController.showToast('Lỗi khi dò tìm ESP-IDF: ' + e.message, 'error');
    }
  }

  static async fetchStatus() {
    try {
      const res = await apiFetch('/api/idf/status', { cache: 'no-store' });
      if (!res.ok) return;
      const data = await res.json();
      BuildController.currentStatus = data;
      BuildController.renderStatus(data);
    } catch (e) {
      console.log('Không thể lấy trạng thái IDF (chế độ offline):', e.message);
    }
  }

  static renderStatus(data) {
    const { idf, target, ports, job } = data;

    // 1. ESP-IDF Toolchain
    const idfBadge = document.getElementById('idf-status-badge');
    const idfVer = document.getElementById('idf-version-val');
    const idfPath = document.getElementById('idf-path-val');
    const idfPy = document.getElementById('idf-python-val');
    const idfCustomTag = document.getElementById('idf-custom-tag');
    const btnResetIdf = document.getElementById('btn-reset-idf-path');
    const customIdfInput = document.getElementById('custom-idf-input');

    if (idf && idf.installed) {
      if (idfBadge) {
        idfBadge.textContent = idf.is_custom ? 'Đã cài đặt (Tùy chỉnh)' : 'Đã cài đặt';
        idfBadge.className = 'status-badge success';
      }
      if (idfVer) idfVer.textContent = idf.version || 'v6.1';
      if (idfPath) idfPath.textContent = idf.idf_path || '--';
      if (idfPy) idfPy.textContent = idf.python_venv ? 'Python 3.11 (ESP-IDF venv)' : 'Python hệ thống';

      if (idfCustomTag) idfCustomTag.style.display = idf.is_custom ? 'inline-block' : 'none';
      if (btnResetIdf) btnResetIdf.style.display = idf.is_custom ? 'inline-flex' : 'none';
      if (customIdfInput && !customIdfInput.value && idf.idf_path) {
        customIdfInput.value = idf.idf_path;
      }
    } else {
      if (idfBadge) {
        idfBadge.textContent = 'Chưa cài đặt';
        idfBadge.className = 'status-badge danger';
      }
      if (idfVer) idfVer.textContent = 'Không tìm thấy ESP-IDF';
      if (idfPath) idfPath.textContent = 'Nhấn "Tùy chỉnh / Dò tìm IDF" để chỉ định thư mục';
      if (idfPy) idfPy.textContent = '--';

      if (idfCustomTag) idfCustomTag.style.display = 'none';
      if (btnResetIdf) btnResetIdf.style.display = 'none';
    }

    // 2. Target chip
    const targetBadge = document.getElementById('target-status-badge');
    const targetVal = document.getElementById('target-name-val');
    const btnSetTarget = document.getElementById('btn-quick-set-target');

    if (target && target.is_set && target.target === 'esp32s3') {
      if (targetBadge) {
        targetBadge.textContent = 'Đã thiết lập';
        targetBadge.className = 'status-badge success';
      }
      if (targetVal) targetVal.textContent = 'esp32s3 (Đã sẵn sàng build)';
      if (btnSetTarget) btnSetTarget.style.display = 'inline-flex';
    } else if (target && target.is_set) {
      if (targetBadge) {
        targetBadge.textContent = `Target: ${target.target}`;
        targetBadge.className = 'status-badge warning';
      }
      if (targetVal) targetVal.textContent = `${target.target} (Cần đổi sang esp32s3)`;
      if (btnSetTarget) btnSetTarget.style.display = 'inline-flex';
    } else {
      if (targetBadge) {
        targetBadge.textContent = 'Chưa set-target';
        targetBadge.className = 'status-badge warning';
      }
      if (targetVal) targetVal.textContent = 'Chưa xác định target';
      if (btnSetTarget) btnSetTarget.style.display = 'inline-flex';
    }

    // 3. COM Ports
    BuildController.renderPortOptions(ports);

    // 4. Job status
    BuildController.syncJobState(job);
  }

  static renderPortOptions(ports) {
    const sel = document.getElementById('serial-port-select');
    if (!sel) return;
    const currentVal = sel.value;
    sel.innerHTML = '';

    if (!ports || ports.length === 0) {
      const opt = document.createElement('option');
      opt.value = '';
      opt.textContent = '-- Không tìm thấy cổng COM nào --';
      sel.appendChild(opt);
      return;
    }

    let hasSelection = false;
    ports.forEach(p => {
      const opt = document.createElement('option');
      opt.value = p.port;
      opt.textContent = `${p.port} - ${p.desc || 'Thiết bị nối tiếp'}`;
      if (currentVal && p.port === currentVal) {
        opt.selected = true;
        hasSelection = true;
      } else if (!hasSelection && /CH343|CH340|CP210|ESP|USB-Enhanced/i.test(p.desc)) {
        opt.selected = true;
        hasSelection = true;
      }
      sel.appendChild(opt);
    });

    if (!hasSelection && ports.length > 0) {
      // Ưu tiên COM lớn hơn COM1 (thường COM1 là cổng COM ảo/motherboard)
      const nonCom1 = ports.find(p => p.port !== 'COM1');
      if (nonCom1) {
        sel.value = nonCom1.port;
      } else {
        sel.selectedIndex = 0;
      }
    }
  }

  static async refreshPorts() {
    try {
      const res = await apiFetch('/api/idf/ports', { cache: 'no-store' });
      if (!res.ok) return;
      const data = await res.json();
      BuildController.renderPortOptions(data.ports);
    } catch (e) {
      console.log('Lỗi refresh ports:', e.message);
    }
  }

  static async triggerAction(action, payload = {}) {
    const isRunning = BuildController.currentStatus?.job?.state === 'running';
    if (isRunning) {
      GUIController.showToast('Đang có tiến trình khác đang chạy!', 'warning');
      return;
    }

    // Lưu cấu hình trước khi build nếu có thay đổi chưa lưu
    const activePanel = document.querySelector('.tab-panel.active')?.id;
    if (activePanel && activePanel !== 'panel-build' && GUIController.dirty[activePanel]) {
      await GUIController.saveTab(activePanel);
    }

    // Chuyển sang panel-build để xem tiến trình
    GUIController.switchTab('panel-build');

    // Chuẩn bị terminal
    const term = document.getElementById('terminal-body');
    if (term) {
      term.innerHTML += `\n<span class="term-cyan">=== YÊU CẦU: ${action.toUpperCase()} ===</span>\n`;
    }
    BuildController.logOffset = 0;

    try {
      const res = await apiFetch(`/api/idf/${action}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
      });
      const data = await res.json();
      if (!data.success) {
        GUIController.showToast(data.error || 'Lỗi khởi động tác vụ', 'error');
        if (term) term.innerHTML += `<span class="term-error">[LỖI] ${data.error}</span>\n`;
        return;
      }

      GUIController.showToast(data.message || `Đang chạy ${action}...`, 'info');
      BuildController.startLogPolling();
    } catch (e) {
      GUIController.showToast('Lỗi kết nối server: ' + e.message, 'error');
    }
  }

  static syncJobState(job) {
    const isRunning = job && job.state === 'running';
    const state = job ? job.state : 'idle';

    // Buttons disable/enable
    const btnBuild = document.getElementById('btn-idf-build');
    const btnFlash = document.getElementById('btn-idf-flash');
    const btnBuildFlash = document.getElementById('btn-idf-build-flash');
    const btnClean = document.getElementById('btn-idf-clean');
    const btnCancel = document.getElementById('btn-idf-cancel');

    if (btnBuild) btnBuild.disabled = isRunning;
    if (btnFlash) btnFlash.disabled = isRunning;
    if (btnBuildFlash) btnBuildFlash.disabled = isRunning;
    if (btnClean) btnClean.disabled = isRunning;
    if (btnCancel) btnCancel.disabled = !isRunning;

    // Badges & status
    const stateTag = document.getElementById('terminal-state-tag');
    const liveDot = document.getElementById('terminal-live-dot');
    const navDot = document.getElementById('nav-build-dot');
    const overallStatus = document.getElementById('build-overall-status');
    const statusLine = document.getElementById('term-status-line');

    if (stateTag) {
      stateTag.textContent = state.toUpperCase();
      stateTag.style.color = isRunning ? '#fbbf24' : (state === 'success' ? '#34d399' : (state === 'failed' ? '#f87171' : '#38bdf8'));
    }

    if (liveDot) {
      liveDot.className = `terminal-dot ${isRunning ? 'running' : (state === 'failed' ? 'error' : '')}`;
    }
    if (navDot) {
      navDot.className = `build-status-dot ${isRunning ? 'running' : (state === 'success' ? 'success' : (state === 'failed' ? 'error' : ''))}`;
    }

    if (overallStatus) {
      if (isRunning) {
        overallStatus.textContent = `Đang chạy (${job.action || 'IDF'})...`;
        overallStatus.className = 'tab-status-pill unsaved';
      } else if (state === 'success') {
        overallStatus.textContent = 'Hoàn tất thành công';
        overallStatus.className = 'tab-status-pill saved';
      } else if (state === 'failed') {
        overallStatus.textContent = 'Thất bại';
        overallStatus.className = 'tab-status-pill unsaved';
      } else {
        overallStatus.textContent = 'Sẵn sàng';
        overallStatus.className = 'tab-status-pill saved';
      }
    }

    if (statusLine) {
      if (isRunning) {
        statusLine.textContent = `Đang chạy tác vụ: ${job.action || 'idf.py'}...`;
      } else if (state === 'success') {
        statusLine.textContent = 'Tiến trình hoàn tất thành công (Exit code: 0)';
      } else if (state === 'failed') {
        statusLine.textContent = `Tiến trình kết thúc với lỗi (Mã thoát: ${job.exit_code})`;
      } else if (state === 'cancelled') {
        statusLine.textContent = 'Tiến trình đã bị người dùng hủy';
      } else {
        statusLine.textContent = 'Trạng thái: Sẵn sàng';
      }
    }

    if (isRunning && !BuildController.isPolling) {
      BuildController.startLogPolling();
    }
  }

  static startLogPolling() {
    if (BuildController.isPolling) return;
    BuildController.isPolling = true;

    if (BuildController.pollTimer) clearInterval(BuildController.pollTimer);
    BuildController.pollTimer = setInterval(async () => {
      try {
        const res = await apiFetch(`/api/idf/logs?offset=${BuildController.logOffset}`, { cache: 'no-store' });
        if (!res.ok) return;
        const data = await res.json();

        if (data.logs && data.logs.length > 0) {
          BuildController.appendLogs(data.logs);
          BuildController.logOffset = data.next_offset;
        }

        BuildController.syncJobState({
          state: data.state,
          action: data.action,
          exit_code: data.exit_code
        });

        if (data.state !== 'running') {
          clearInterval(BuildController.pollTimer);
          BuildController.pollTimer = null;
          BuildController.isPolling = false;
          if (data.state === 'success') {
            GUIController.showToast('Tiến trình hoàn tất thành công!', 'success');
          } else if (data.state === 'failed') {
            GUIController.showToast(`Tiến trình thất bại (Mã: ${data.exit_code})`, 'error');
          }
        }
      } catch (e) {
        console.log('Lỗi poll logs:', e.message);
      }
    }, 600);
  }

  static appendLogs(lines) {
    const term = document.getElementById('terminal-body');
    if (!term) return;

    const frag = document.createDocumentFragment();
    lines.forEach(rawLine => {
      const line = rawLine;
      const span = document.createElement('span');

      if (/^===/.test(line)) {
        span.className = 'term-cyan';
      } else if (/error:|fatal:|\[LỖI\]|FAILED|Ninja build failed/i.test(line)) {
        span.className = 'term-error';
      } else if (/warning:/i.test(line)) {
        span.className = 'term-warn';
      } else if (/THÀNH CÔNG|Successfully|Hash of data verified|Done/i.test(line)) {
        span.className = 'term-success';
      } else if (/^-- /.test(line) || /^Executing/i.test(line)) {
        span.className = 'term-dim';
      }

      span.textContent = line + '\n';
      frag.appendChild(span);
    });

    term.appendChild(frag);

    const autoScroll = document.getElementById('terminal-autoscroll')?.checked;
    if (autoScroll) {
      term.scrollTop = term.scrollHeight;
    }
  }
}

// ============================================================================
// GUIController  —  liên kết HTML ↔ configState
// ============================================================================
class GUIController {
  static snapshots   = {};   // panelId → snapshot string
  static dirty       = {};   // panelId → bool
  static pendingTab  = null;

  static TAB_NAMES = {
    'panel-general':       'Cơ bản & Ngôn ngữ',
    'panel-audio':         'Âm thanh (Loa & Micro)',
    'panel-display':       'Màn hình & Cảm ứng',
    'panel-wakeword':      'Từ khóa & Giọng nói',
    'panel-network':       'Cấp mạng Wi-Fi',
    'panel-mcp':           'MCP & AI',
    'panel-buttons':       'Phím bấm & Input',
    'panel-camera':        'Camera',
    'panel-led':           'LED Trạng thái',
    'panel-uart':          'UART mở rộng',
    'panel-actuators':     'Servo / Buzzer / Haptic',
    'panel-motor':         'Relay & Motor DC',
    'panel-sensors':       'Cảm biến',
    'panel-power':         'Pin & Nguồn',
    'panel-flash':         'Flash & ESP32-S3',
    'panel-build':         'Biên dịch & Nạp',
  };

  // ── Khởi động ────────────────────────────────────────────────────────────
  static async init() {
    GUIController.bindNav();
    GUIController.bindInputs();
    GUIController.bindSaveBtns();
    GUIController.bindUnsavedModal();
    GUIController.bindOpenDir();
    BuildController.init();
    GUIController.syncStateToUI();
    GUIController.renderPinMatrix();
    GUIController.initSnapshots();
    await GUIController.autoConnect();
  }

  // ── Kết nối server và nạp cấu hình ──────────────────────────────────────
  static async autoConnect() {
    try {
      const res = await apiFetch('/api/project', { cache: 'no-store' });
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const info = await res.json();
      if (!info.success) throw new Error(info.error || 'Server không xác nhận');

      isServerMode = true;
      serverProjectInfo = info;
      GUIController._setStatus('connected', `Dự án: ${info.root_path}`);

      // Nạp sdkconfig.defaults
      const cfgRes = await apiFetch('/api/config', { cache: 'no-store' });
      if (cfgRes.ok) {
        const cfgData = await cfgRes.json();
        if (cfgData.content && cfgData.content.trim()) {
          configState = SdkconfigParser.parse(cfgData.content);
          GUIController.syncStateToUI();
          GUIController.initSnapshots();
          GUIController.updateValidation();
          GUIController.showToast(`Đã nạp cấu hình từ sdkconfig.defaults`, 'success');
        }
      }
    } catch (e) {
      isServerMode = false;
      console.warn('Không thể kết nối Configurator Server:', e.message);
      if (typeof window !== 'undefined' && window.location.protocol === 'file:') {
        GUIController._setStatus('error', 'Chưa chạy run_configurator.bat');
      } else {
        GUIController._setStatus('error', 'Chưa kết nối Server');
      }
    }
  }

  // ── Snapshot / Dirty ─────────────────────────────────────────────────────
  static snapshot(panelId) {
    const p = document.getElementById(panelId);
    if (!p) return '';
    const parts = [];
    p.querySelectorAll('input, select, textarea').forEach(el => {
      if (el.type === 'checkbox') parts.push(`${el.id}:${el.checked}`);
      else if (el.type === 'radio') { if (el.checked) parts.push(`${el.name}:${el.value}`); }
      else parts.push(`${el.id}:${el.value}`);
    });
    return parts.join('|');
  }

  static initSnapshots() {
    document.querySelectorAll('.tab-panel').forEach(p => {
      GUIController.snapshots[p.id] = GUIController.snapshot(p.id);
      GUIController.dirty[p.id]     = false;
      GUIController._updateTabBadge(p.id);
    });
  }

  static checkDirty(panelId) {
    GUIController.syncUIToState();
    const now   = GUIController.snapshot(panelId);
    const isDirty = now !== (GUIController.snapshots[panelId] || '');
    GUIController.dirty[panelId] = isDirty;
    GUIController._updateTabBadge(panelId);
    GUIController.updateValidation();
  }

  static _updateTabBadge(panelId) {
    const isDirty = !!GUIController.dirty[panelId];
    document.querySelector(`.nav-link[data-target="${panelId}"]`)
      ?.classList.toggle('has-unsaved', isDirty);
    const pill = document.querySelector(`.tab-status-pill[data-tab-status="${panelId}"]`);
    if (pill) {
      pill.className = `tab-status-pill ${isDirty ? 'unsaved' : 'saved'}`;
      pill.textContent = isDirty ? 'Chưa lưu' : 'Đã lưu';
    }
    document.querySelector(`.btn-save-tab[data-tab="${panelId}"]`)
      ?.classList.toggle('btn-highlight-save', isDirty);
  }

  // ── Tab switching ────────────────────────────────────────────────────────
  static switchTab(targetId) {
    document.querySelectorAll('.nav-link[data-target]').forEach(b =>
      b.classList.toggle('active', b.dataset.target === targetId));
    document.querySelectorAll('.tab-panel').forEach(p =>
      p.classList.toggle('active', p.id === targetId));
    if (targetId === 'panel-build') {
      BuildController.onTabActivated();
    }
  }

  static bindNav() {
    document.querySelectorAll('.nav-link[data-target]').forEach(btn => {
      btn.addEventListener('click', e => {
        const target  = btn.dataset.target;
        const active  = document.querySelector('.tab-panel.active');
        const current = active?.id;
        if (current === target) return;
        if (current && GUIController.dirty[current]) {
          e.preventDefault();
          GUIController.pendingTab = target;
          GUIController._openUnsavedModal(current);
          return;
        }
        GUIController.switchTab(target);
      });
    });
  }

  // ── Unsaved changes modal ────────────────────────────────────────────────
  static _openUnsavedModal(panelId) {
    const modal = document.getElementById('unsaved-modal');
    const desc  = document.getElementById('unsaved-modal-desc');
    const name  = GUIController.TAB_NAMES[panelId] || panelId;
    if (desc) desc.innerHTML =
      `Bạn có thay đổi chưa lưu trong mục <strong>"${name}"</strong>.`;
    modal?.classList.add('active');
  }

  static bindUnsavedModal() {
    const close = () => {
      document.getElementById('unsaved-modal')?.classList.remove('active');
      GUIController.pendingTab = null;
    };
    document.getElementById('unsaved-btn-stay')?.addEventListener('click', close);
    document.getElementById('unsaved-btn-close')?.addEventListener('click', close);
    document.getElementById('unsaved-btn-discard')?.addEventListener('click', () => {
      const active = document.querySelector('.tab-panel.active')?.id;
      if (active) {
        GUIController.syncStateToUI();
        GUIController.dirty[active] = false;
        GUIController.snapshots[active] = GUIController.snapshot(active);
        GUIController._updateTabBadge(active);
      }
      const t = GUIController.pendingTab;
      close();
      if (t) GUIController.switchTab(t);
    });
    document.getElementById('unsaved-btn-save')?.addEventListener('click', async () => {
      const active = document.querySelector('.tab-panel.active')?.id;
      if (active) await GUIController.saveTab(active);
      const t = GUIController.pendingTab;
      close();
      if (t) GUIController.switchTab(t);
    });
  }

  // ── Input listeners ──────────────────────────────────────────────────────
  static bindInputs() {
    document.querySelectorAll('.tab-panel').forEach(panel => {
      panel.addEventListener('input',  () => GUIController.checkDirty(panel.id));
      panel.addEventListener('change', () => GUIController.checkDirty(panel.id));
    });

    // Conditional show/hide triggers
    const on = (id, fn) => document.getElementById(id)?.addEventListener('change', fn);

    on('display_enable', () => GUIController._toggleDisplayGroups());
    on('display_type',   () => GUIController._toggleDisplayGroups());
    on('display_uart_secondary', () => GUIController._toggleDisplayGroups());
    on('general_display_style',  () => GUIController._toggleMultilineChat());
    on('touch_enable',   () => GUIController._toggleGroup('.touch-fields', 'touch_enable'));
    on('speaker_enable', () => GUIController._toggleGroup('.spk-fields',   'speaker_enable'));
    on('speaker_type',   () => GUIController._toggleCodecGroup('spk'));
    on('mic_enable',     () => GUIController._toggleGroup('.mic-fields',   'mic_enable'));
    on('mic_type',       () => GUIController._toggleCodecGroup('mic'));
    on('camera_enable',  () => GUIController._toggleCameraUsb());
    on('camera_sensor',  () => GUIController._toggleCameraUsb());
    on('led_enable',     () => GUIController._toggleGroup('.led-fields',   'led_enable'));
    on('led_type',       () => GUIController._toggleLedRainbow());
    on('mcp_enable',     () => GUIController._toggleGroup('.mcp-fields',   'mcp_enable'));
    on('uart_enable',    () => GUIController._toggleGroup('.uart-fields',  'uart_enable'));
    on('uart_flow',      () => GUIController._toggleGroup('.uart-flow-fields','uart_flow'));
    on('buttons_boot',   () => GUIController._toggleGroup('.boot-fields',  'buttons_boot'));
    on('buttons_touch',  () => GUIController._toggleGroup('.touch-btn-fields','buttons_touch'));
    on('buttons_vol',    () => GUIController._toggleGroup('.vol-fields',   'buttons_vol'));
    on('buttons_slider', () => GUIController._toggleGroup('.slider-fields','buttons_slider'));
    on('buttons_rotary', () => GUIController._toggleGroup('.rotary-fields','buttons_rotary'));
    on('servo_enable',   () => GUIController._toggleGroup('.servo-fields', 'servo_enable'));
    on('buzzer_enable',  () => GUIController._toggleGroup('.buzzer-fields','buzzer_enable'));
    on('haptic_enable',  () => GUIController._toggleGroup('.haptic-fields','haptic_enable'));
    on('relay_enable',   () => GUIController._toggleGroup('.relay-fields', 'relay_enable'));
    on('motor_dc_enable',() => GUIController._toggleGroup('.motor-fields', 'motor_dc_enable'));
    on('battery_enable', () => GUIController._toggleGroup('.battery-fields','battery_enable'));
    on('ina2xx_enable',  () => GUIController._toggleGroup('.ina-fields',   'ina2xx_enable'));
    on('tp4056_enable',  () => GUIController._toggleGroup('.tp4056-fields','tp4056_enable'));
    on('flash_size',     () => GUIController._syncFlashPartition());
    on('wakeword_type',  () => GUIController._toggleWakewordCustom());
    on('mcp_lamp',       () => GUIController._toggleGroup('.lamp-gpio-field','mcp_lamp'));
    // sensor enables
    ['dht','temp','bmp280','bh1750','ldr','gas_co2','gas_mq',
     'hcsr04','pir','vibration','flame'].forEach(sens => {
      on(`sensor_${sens}`, () =>
        GUIController._toggleGroup(`.sensor-${sens}-fields`, `sensor_${sens}`));
    });

    // Guard tab unload
    window.addEventListener('beforeunload', e => {
      if (Object.values(GUIController.dirty).some(Boolean)) {
        e.preventDefault(); e.returnValue = '';
      }
    });
  }

  // ── Save tab ─────────────────────────────────────────────────────────────
  static async saveTab(panelId) {
    GUIController.syncUIToState();
    const { errors } = PinValidator.validate(configState);
    if (errors.length) {
      GUIController.showToast('Có xung đột GPIO! Kiểm tra lại.', 'error');
      GUIController.updateValidation();
      return false;
    }

    if (isServerMode) {
      try {
        const lines = SdkconfigGenerator.build(configState);
        const res = await apiFetch('/api/save', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ sdkconfig_lines: lines }),
        });
        const data = await res.json();
        if (!data.success) throw new Error(data.error);
        GUIController.showToast(
          `Đã ghi "${GUIController.TAB_NAMES[panelId]||panelId}" vào sdkconfig.defaults`, 'success');
      } catch (e) {
        GUIController.showToast(`Lỗi khi lưu: ${e.message}`, 'error');
        return false;
      }
    } else {
      GUIController.showToast(
        `Đã lưu "${GUIController.TAB_NAMES[panelId]||panelId}" (chạy run_configurator.bat để ghi file)`,
        'success');
    }

    GUIController.snapshots[panelId] = GUIController.snapshot(panelId);
    GUIController.dirty[panelId]     = false;
    GUIController._updateTabBadge(panelId);
    return true;
  }

  static bindSaveBtns() {
    document.querySelectorAll('.btn-save-tab').forEach(btn => {
      btn.addEventListener('click', async () => {
        const tabId = btn.dataset.tab;
        if (tabId) {
          const ok = await GUIController.saveTab(tabId);
          if (ok) {
            btn.classList.add('btn-saved-success');
            const orig = btn.innerHTML;
            btn.innerHTML = '<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><polyline points="20 6 9 17 4 12"></polyline></svg><span>Đã lưu ✓</span>';
            setTimeout(() => { btn.innerHTML = orig; btn.classList.remove('btn-saved-success'); }, 1500);
          }
        }
      });
    });

    document.getElementById('btn-download-config')?.addEventListener('click', () => {
      GUIController.syncUIToState();
      const lines = SdkconfigGenerator.build(configState);
      const blob  = new Blob([lines.join('\n')], { type: 'text/plain' });
      const a     = Object.assign(document.createElement('a'), {
        href: URL.createObjectURL(blob),
        download: 'sdkconfig.defaults',
      });
      a.click();
      GUIController.showToast('Đã tải xuống sdkconfig.defaults', 'success');
    });
  }

  // ── Project Modal & Root Switching ───────────────────────────────────────
  static async openProjectModal() {
    const modal = document.getElementById('project-modal');
    if (!modal) return;
    modal.classList.add('active');
    modal.classList.add('visible');

    const input = document.getElementById('input-project-root');
    if (input && serverProjectInfo) {
      input.value = serverProjectInfo.root_path || '';
    }

    await GUIController.loadDetectedProjects();
  }

  static closeProjectModal() {
    const modal = document.getElementById('project-modal');
    modal?.classList.remove('active');
    modal?.classList.remove('visible');
  }

  static async loadDetectedProjects() {
    const container = document.getElementById('project-detected-list');
    if (!container) return;
    container.innerHTML = '<span style="font-size:0.75rem; color:var(--text-muted); padding:4px;">Đang quét dự án Xiaozhi trên máy tính...</span>';

    try {
      const res = await apiFetch('/api/project/scan-all', { cache: 'no-store' });
      const data = await res.json();
      if (data.success && Array.isArray(data.projects)) {
        container.innerHTML = '';
        if (data.projects.length === 0) {
          container.innerHTML = '<span style="font-size:0.75rem; color:var(--text-muted); padding:4px;">Không tìm thấy dự án Xiaozhi khác. Hãy nhập đường dẫn thư mục vào ô trên.</span>';
          return;
        }

        data.projects.forEach(item => {
          const row = document.createElement('div');
          row.className = `project-scan-item ${item.is_current ? 'is-current' : ''}`;
          row.innerHTML = `
            <div class="project-scan-item-info">
              <span class="project-scan-path" title="${item.path}">${item.path}</span>
              <div class="project-scan-meta">
                <span class="idf-scan-source-pill">${item.source}</span>
                ${item.is_current ? '<span class="text-emerald font-bold">● Đang hoạt động</span>' : ''}
              </div>
            </div>
            ${!item.is_current ? `
              <button type="button" class="btn btn-outline btn-xs btn-pick-project">
                Chọn
              </button>
            ` : '<span class="text-emerald" style="font-size:0.75rem;">✓ Hiện tại</span>'}
          `;
          row.querySelector('.btn-pick-project')?.addEventListener('click', async () => {
            const input = document.getElementById('input-project-root');
            if (input) input.value = item.path;
            await GUIController.applyProjectRoot(item.path);
          });
          container.appendChild(row);
        });
      }
    } catch (e) {
      container.innerHTML = `
        <div style="font-size:0.75rem; color:var(--accent-rose); padding:8px; background:rgba(244,63,94,0.1); border:1px solid rgba(244,63,94,0.25); border-radius:4px; line-height:1.45;">
          <strong>Chưa kết nối Backend Server (HTTP 8080):</strong><br>
          <span style="color:var(--text-secondary);">Vui lòng khởi động <code>run_configurator.bat</code> từ thư mục dự án để kích hoạt máy chủ cấu hình tại <code>http://localhost:8080/</code>.</span>
        </div>
      `;
    }
  }

  static async applyProjectRoot(newPath) {
    if (!newPath) {
      GUIController.showToast('Vui lòng nhập đường dẫn thư mục dự án!', 'warning');
      return;
    }
    try {
      const res = await apiFetch('/api/project/set-root', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ root_path: newPath })
      });
      const data = await res.json();
      if (data.success) {
        serverProjectInfo = data.project_info || { root_path: data.root_path };
        GUIController._setStatus('connected', `Dự án: ${data.root_path}`);
        GUIController.showToast(data.message || 'Đã chuyển sang dự án mới', 'success');
        GUIController.closeProjectModal();

        // Nạp lại cấu hình từ dự án mới
        const cfgRes = await apiFetch('/api/config', { cache: 'no-store' });
        if (cfgRes.ok) {
          const cfgData = await cfgRes.json();
          if (cfgData.content) {
            configState = SdkconfigParser.parse(cfgData.content);
            GUIController.syncStateToUI();
            GUIController.initSnapshots();
            GUIController.updateValidation();
          }
        }
        // Cập nhật lại status build panel
        BuildController.fetchStatus();
      } else {
        GUIController.showToast(data.error || 'Thư mục không hợp lệ!', 'error');
      }
    } catch (e) {
      GUIController.showToast('Lỗi gửi yêu cầu: ' + e.message, 'error');
    }
  }

  static async resetProjectRoot() {
    try {
      const res = await apiFetch('/api/project/reset-root', { method: 'POST' });
      const data = await res.json();
      if (data.success) {
        serverProjectInfo = data.project_info || { root_path: data.root_path };
        GUIController._setStatus('connected', `Dự án: ${data.root_path}`);
        GUIController.showToast(data.message || 'Đã khôi phục chế độ tự động', 'info');
        GUIController.closeProjectModal();

        const cfgRes = await apiFetch('/api/config', { cache: 'no-store' });
        if (cfgRes.ok) {
          const cfgData = await cfgRes.json();
          if (cfgData.content) {
            configState = SdkconfigParser.parse(cfgData.content);
            GUIController.syncStateToUI();
            GUIController.initSnapshots();
            GUIController.updateValidation();
          }
        }
        BuildController.fetchStatus();
      }
    } catch (e) {
      GUIController.showToast('Lỗi: ' + e.message, 'error');
    }
  }

  static bindOpenDir() {
    const triggerModal = async () => {
      if (!isServerMode) {
        await GUIController.autoConnect();
      }
      await GUIController.openProjectModal();
    };

    document.getElementById('btn-open-dir')?.addEventListener('click', triggerModal);
    document.getElementById('project-status-bar')?.addEventListener('click', triggerModal);

    // Đóng modal khi click vào backdrop
    document.getElementById('project-modal')?.addEventListener('click', (e) => {
      if (e.target.id === 'project-modal') {
        GUIController.closeProjectModal();
      }
    });

    // Modal events
    document.getElementById('btn-close-project-modal')?.addEventListener('click', () => GUIController.closeProjectModal());
    document.getElementById('btn-close-project-modal-footer')?.addEventListener('click', () => GUIController.closeProjectModal());
    document.getElementById('btn-scan-projects')?.addEventListener('click', () => GUIController.loadDetectedProjects());
    document.getElementById('btn-apply-project-root')?.addEventListener('click', () => {
      const val = document.getElementById('input-project-root')?.value?.trim();
      GUIController.applyProjectRoot(val);
    });
    document.getElementById('btn-reset-project-root')?.addEventListener('click', () => GUIController.resetProjectRoot());
  }

  // ── syncStateToUI ─────────────────────────────────────────────────────────
  static syncStateToUI() {
    const s = configState;
    const v  = (id, val) => { const el = document.getElementById(id); if (el) el.value = val; };
    const c  = (id, val) => { const el = document.getElementById(id); if (el) el.checked = !!val; };
    const sv = (id, val) => { const el = document.getElementById(id); if (el) el.value = String(val); };

    // ── General ──
    sv('general_language',      s.general.language);
    v('general_ota_url',        s.general.ota_url);
    sv('general_flash_assets',  s.general.flash_assets);
    v('general_custom_assets',  s.general.custom_assets_file);
    sv('general_display_style', s.general.display_style);
    c('general_multiline',      s.general.multiline_chat);

    // ── System / Flash ──
    sv('sys_cpu_freq',        s.system.cpu_freq);
    sv('sys_flash_mode',      s.system.flash_mode);
    sv('sys_flash_freq',      s.system.flash_freq);
    c('sys_spiram',           s.system.spiram);
    sv('sys_spiram_mode',     s.system.spiram_mode);
    sv('sys_spiram_speed',    s.system.spiram_speed);
    sv('sys_partition_table', s.system.partition_table);

    // ── Display ──
    c('display_enable',     s.display.enable);
    sv('display_type',      s.display.type);
    v('display_width',      s.display.width);
    v('display_height',     s.display.height);
    v('display_mosi',       s.display.pin_mosi);
    v('display_clk',        s.display.pin_clk);
    v('display_cs',         s.display.pin_cs);
    v('display_dc',         s.display.pin_dc);
    c('display_use_rst',    s.display.use_rst);
    v('display_rst',        s.display.pin_rst);
    c('display_use_blk',    s.display.use_blk);
    v('display_blk',        s.display.pin_blk);
    v('display_i2c_sda',    s.display.pin_i2c_sda);
    v('display_i2c_scl',    s.display.pin_i2c_scl);
    v('display_offset_x',   s.display.offset_x);
    v('display_offset_y',   s.display.offset_y);
    c('display_mirror_x',   s.display.mirror_x);
    c('display_mirror_y',   s.display.mirror_y);
    c('display_swap_xy',    s.display.swap_xy);
    c('display_invert',     s.display.invert_color);
    sv('display_amoled_chip', s.display.amoled_chip);
    v('display_qspi_cs',    s.display.qspi_cs);
    v('display_qspi_clk',   s.display.qspi_clk);
    v('display_qspi_d0',    s.display.qspi_d0); v('display_qspi_d1', s.display.qspi_d1);
    v('display_qspi_d2',    s.display.qspi_d2); v('display_qspi_d3', s.display.qspi_d3);
    v('display_qspi_rst',   s.display.qspi_rst);
    v('display_epaper_busy',s.display.epaper_busy);
    v('display_rgb_pclk',   s.display.rgb_pclk);  v('display_rgb_de',   s.display.rgb_de);
    v('display_rgb_vsync',  s.display.rgb_vsync);  v('display_rgb_hsync',s.display.rgb_hsync);
    for (let n=0;n<=15;n++) v(`display_rgb_d${n}`, s.display[`rgb_d${n}`]);
    c('display_uart_secondary', s.display.uart_secondary);
    sv('display_uart_port',     s.display.uart_port);
    v('display_uart_tx',        s.display.uart_tx);
    v('display_uart_rx',        s.display.uart_rx);
    sv('display_uart_baud',     s.display.uart_baud);
    sv('display_uart_proto',    s.display.uart_proto);

    // ── Touch ──
    c('touch_enable',   s.touch.enable);
    sv('touch_type',    s.touch.type);
    v('touch_sda',      s.touch.pin_sda); v('touch_scl', s.touch.pin_scl);
    v('touch_int',      s.touch.pin_int); v('touch_rst', s.touch.pin_rst);

    // ── Speaker ──
    c('speaker_enable',   s.speaker.enable);
    sv('speaker_type',    s.speaker.type);
    v('spk_dout',         s.speaker.pin_dout); v('spk_bclk', s.speaker.pin_bclk);
    v('spk_lrck',         s.speaker.pin_lrck);
    v('spk_codec_sda',    s.speaker.codec_sda); v('spk_codec_scl', s.speaker.codec_scl);

    // ── Mic ──
    c('mic_enable',   s.mic.enable);
    sv('mic_type',    s.mic.type);
    v('mic_din',      s.mic.pin_din); v('mic_sck', s.mic.pin_sck); v('mic_ws', s.mic.pin_ws);
    v('mic_codec_sda',s.mic.codec_sda); v('mic_codec_scl', s.mic.codec_scl);

    // ── I2S mode ──
    sv('audio_i2s_mode', s.audio.i2s_mode);

    // ── Wake word ──
    sv('wakeword_type',      s.wakeword.type);
    v('wakeword_word',       s.wakeword.custom_word);
    v('wakeword_display',    s.wakeword.custom_display);
    v('wakeword_threshold',  s.wakeword.threshold);
    c('wakeword_device_aec', s.wakeword.device_aec);
    c('wakeword_server_aec', s.wakeword.server_aec);
    c('wakeword_send_data',  s.wakeword.send_data);
    c('wakeword_in_listen',  s.wakeword.detection_in_listening);

    // ── Wi-Fi ──
    sv('wifi_method', s.wifi.method);

    // ── Buttons ──
    c('buttons_boot',    s.buttons.boot_enable);
    v('boot_gpio',       s.buttons.boot_gpio);
    c('buttons_touch',   s.buttons.touch_enable);
    v('touch_btn_gpio',  s.buttons.touch_gpio);
    c('buttons_vol',     s.buttons.vol_enable);
    v('vol_up_gpio',     s.buttons.vol_up_gpio);
    v('vol_down_gpio',   s.buttons.vol_down_gpio);
    c('buttons_slider',  s.buttons.slider_enable);
    v('slider_pad1',     s.buttons.slider_pad1);
    v('slider_pad2',     s.buttons.slider_pad2);
    v('slider_pad3',     s.buttons.slider_pad3);
    c('buttons_rotary',  s.buttons.rotary_enable);
    v('rotary_a',        s.buttons.rotary_a);
    v('rotary_b',        s.buttons.rotary_b);
    v('rotary_key',      s.buttons.rotary_key);
    c('user_custom_sensors', s.buttons.user_custom_sensors);

    // ── Camera ──
    c('camera_enable',    s.camera.enable);
    sv('camera_sensor',   s.camera.sensor);
    v('cam_xclk',s.camera.pin_xclk); v('cam_pclk',s.camera.pin_pclk);
    v('cam_vsync',s.camera.pin_vsync); v('cam_href',s.camera.pin_href);
    v('cam_siod',s.camera.pin_siod); v('cam_sioc',s.camera.pin_sioc);
    c('cam_use_reset',s.camera.use_reset); v('cam_reset',s.camera.pin_reset);
    c('cam_use_pwdn',s.camera.use_pwdn);   v('cam_pwdn',s.camera.pin_pwdn);
    for (let n=0;n<=7;n++) v(`cam_d${n}`, s.camera[`pin_d${n}`]);
    c('cam_hmirror', s.camera.hmirror);
    c('cam_vflip',   s.camera.vflip);

    // ── LED ──
    c('led_enable',    s.led.enable);
    sv('led_type',     s.led.type);
    v('led_gpio',      s.led.gpio);
    v('led_count',     s.led.count);
    c('led_rainbow',   s.led.rainbow);

    // ── MCP ──
    c('mcp_enable',    s.mcp.enable);
    c('mcp_lamp',      s.mcp.lamp);
    v('mcp_lamp_gpio', s.mcp.lamp_gpio);
    c('mcp_sensor',    s.mcp.sensor);
    c('mcp_actuator',  s.mcp.actuator);

    // ── UART ──
    c('uart_enable',   s.uart.enable);
    sv('uart_port',    s.uart.port);
    v('uart_tx',       s.uart.pin_tx); v('uart_rx', s.uart.pin_rx);
    v('uart_baud',     s.uart.baudrate);
    c('uart_flow',     s.uart.flow_ctrl);
    v('uart_rts',      s.uart.pin_rts); v('uart_cts', s.uart.pin_cts);

    // ── Actuators ──
    c('servo_enable',  s.actuators.servo_enable);
    v('servo_gpio',    s.actuators.servo_gpio);
    c('buzzer_enable', s.actuators.buzzer_enable);
    v('buzzer_gpio',   s.actuators.buzzer_gpio);
    c('haptic_enable', s.actuators.haptic_enable);
    v('haptic_gpio',   s.actuators.haptic_gpio);

    // ── Motor ──
    c('relay_enable',    s.motor.relay_enable);
    v('relay_gpio',      s.motor.relay_gpio);
    c('motor_dc_enable', s.motor.dc_enable);
    sv('motor_driver',   s.motor.dc_driver);
    v('motor_pwma',s.motor.pwma); v('motor_dira',s.motor.dira);
    v('motor_pwmb',s.motor.pwmb); v('motor_dirb',s.motor.dirb);

    // ── Sensors ──
    c('sensor_dht',  s.sensors.dht_enable);  v('dht_gpio',  s.sensors.dht_gpio);
    c('sensor_temp', s.sensors.temp_enable); c('sensor_aht20', s.sensors.aht20);
    v('temp_sda', s.sensors.temp_sda);       v('temp_scl', s.sensors.temp_scl);
    c('sensor_bmp280', s.sensors.bmp280);
    v('bmp280_sda', s.sensors.bmp280_sda);   v('bmp280_scl', s.sensors.bmp280_scl);
    c('sensor_bh1750', s.sensors.bh1750);
    v('bh1750_sda', s.sensors.bh1750_sda);   v('bh1750_scl', s.sensors.bh1750_scl);
    c('sensor_ldr', s.sensors.ldr);          sv('ldr_ch', s.sensors.ldr_ch);
    c('sensor_gas_co2', s.sensors.gas_co2);  c('gas_scd4x', s.sensors.gas_scd4x);
    v('gas_sda', s.sensors.gas_sda);         v('gas_scl', s.sensors.gas_scl);
    c('sensor_gas_mq', s.sensors.gas_mq);    v('gas_mq_gpio', s.sensors.gas_mq_gpio);
    c('sensor_hcsr04', s.sensors.hcsr04);
    v('hcsr04_trig', s.sensors.hcsr04_trig); v('hcsr04_echo', s.sensors.hcsr04_echo);
    c('sensor_pir', s.sensors.pir);          v('pir_gpio', s.sensors.pir_gpio);
    c('sensor_vibration', s.sensors.vibration); v('vibration_gpio', s.sensors.vibration_gpio);
    c('sensor_flame', s.sensors.flame);      v('flame_gpio', s.sensors.flame_gpio);

    // ── Power ──
    c('battery_enable', s.power.battery_enable);
    c('battery_adc',    s.power.battery_adc);
    sv('battery_ch',    s.power.battery_ch);
    v('battery_r1',     s.power.battery_r1); v('battery_r2', s.power.battery_r2);
    c('ina2xx_enable',  s.power.ina2xx);     c('ina219', s.power.ina219);
    v('ina_sda',        s.power.ina_sda);    v('ina_scl', s.power.ina_scl);
    c('tp4056_enable',  s.power.tp4056);     v('tp4056_gpio', s.power.tp4056_gpio);

    // Trigger conditional UI
    GUIController._toggleMultilineChat();
    GUIController._toggleDisplayGroups();
    GUIController._toggleWakewordCustom();
    GUIController._toggleCameraUsb();
    document.getElementById('touch_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('speaker_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('mic_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('camera_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('led_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('mcp_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('uart_enable')?.dispatchEvent(new Event('change'));

    // Buttons
    document.getElementById('buttons_boot')?.dispatchEvent(new Event('change'));
    document.getElementById('buttons_touch')?.dispatchEvent(new Event('change'));
    document.getElementById('buttons_vol')?.dispatchEvent(new Event('change'));
    document.getElementById('buttons_slider')?.dispatchEvent(new Event('change'));
    document.getElementById('buttons_rotary')?.dispatchEvent(new Event('change'));

    // Peripherals
    document.getElementById('servo_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('buzzer_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('haptic_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('relay_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('motor_dc_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('battery_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('ina2xx_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('tp4056_enable')?.dispatchEvent(new Event('change'));
  }

  // ── syncUIToState ─────────────────────────────────────────────────────────
  static syncUIToState() {
    const s = configState;
    const gv = id => document.getElementById(id)?.value ?? '';
    const gi = (id, def=-1) => { const v = parseInt(gv(id),10); return isNaN(v) ? def : v; };
    const gc = id => !!document.getElementById(id)?.checked;

    // general
    s.general.language         = gv('general_language');
    s.general.ota_url          = gv('general_ota_url');
    s.general.flash_assets     = gv('general_flash_assets');
    s.general.custom_assets_file = gv('general_custom_assets');
    s.general.display_style    = gv('general_display_style');
    s.general.multiline_chat   = gc('general_multiline');
    // system
    s.system.cpu_freq          = gv('sys_cpu_freq');
    s.system.flash_mode        = gv('sys_flash_mode');
    s.system.flash_freq        = gv('sys_flash_freq');
    s.system.spiram            = gc('sys_spiram');
    s.system.spiram_mode       = gv('sys_spiram_mode');
    s.system.spiram_speed      = gv('sys_spiram_speed');
    s.system.partition_table   = gv('sys_partition_table');
    // display
    s.display.enable           = gc('display_enable');
    s.display.type             = gv('display_type');
    s.display.width            = gi('display_width', 240);
    s.display.height           = gi('display_height', 320);
    s.display.pin_mosi         = gi('display_mosi');
    s.display.pin_clk          = gi('display_clk');
    s.display.pin_cs           = gi('display_cs');
    s.display.pin_dc           = gi('display_dc');
    s.display.use_rst          = gc('display_use_rst');
    s.display.pin_rst          = gi('display_rst');
    s.display.use_blk          = gc('display_use_blk');
    s.display.pin_blk          = gi('display_blk');
    s.display.pin_i2c_sda      = gi('display_i2c_sda');
    s.display.pin_i2c_scl      = gi('display_i2c_scl');
    s.display.offset_x         = gi('display_offset_x', 0);
    s.display.offset_y         = gi('display_offset_y', 0);
    s.display.mirror_x         = gc('display_mirror_x');
    s.display.mirror_y         = gc('display_mirror_y');
    s.display.swap_xy          = gc('display_swap_xy');
    s.display.invert_color     = gc('display_invert');
    s.display.amoled_chip      = gv('display_amoled_chip');
    s.display.qspi_cs          = gi('display_qspi_cs');
    s.display.qspi_clk         = gi('display_qspi_clk');
    for (let n=0;n<=3;n++) s.display[`qspi_d${n}`] = gi(`display_qspi_d${n}`);
    s.display.qspi_rst         = gi('display_qspi_rst');
    s.display.epaper_busy      = gi('display_epaper_busy');
    s.display.rgb_pclk         = gi('display_rgb_pclk');
    s.display.rgb_de           = gi('display_rgb_de');
    s.display.rgb_vsync        = gi('display_rgb_vsync');
    s.display.rgb_hsync        = gi('display_rgb_hsync');
    for (let n=0;n<=15;n++) s.display[`rgb_d${n}`] = gi(`display_rgb_d${n}`);
    s.display.uart_secondary   = gc('display_uart_secondary');
    s.display.uart_port        = gi('display_uart_port', 1);
    s.display.uart_tx          = gi('display_uart_tx');
    s.display.uart_rx          = gi('display_uart_rx');
    s.display.uart_baud        = gi('display_uart_baud', 115200);
    s.display.uart_proto       = gv('display_uart_proto');
    // touch
    s.touch.enable             = gc('touch_enable');
    s.touch.type               = gv('touch_type');
    s.touch.pin_sda            = gi('touch_sda'); s.touch.pin_scl = gi('touch_scl');
    s.touch.pin_int            = gi('touch_int'); s.touch.pin_rst = gi('touch_rst');
    // speaker
    s.speaker.enable           = gc('speaker_enable');
    s.speaker.type             = gv('speaker_type');
    s.speaker.pin_dout         = gi('spk_dout'); s.speaker.pin_bclk = gi('spk_bclk');
    s.speaker.pin_lrck         = gi('spk_lrck');
    s.speaker.codec_sda        = gi('spk_codec_sda'); s.speaker.codec_scl = gi('spk_codec_scl');
    // mic
    s.mic.enable               = gc('mic_enable');
    s.mic.type                 = gv('mic_type');
    s.mic.pin_din              = gi('mic_din'); s.mic.pin_sck = gi('mic_sck');
    s.mic.pin_ws               = gi('mic_ws');
    s.mic.codec_sda            = gi('mic_codec_sda'); s.mic.codec_scl = gi('mic_codec_scl');
    // audio
    s.audio.i2s_mode           = gv('audio_i2s_mode');
    // wakeword
    s.wakeword.type            = gv('wakeword_type');
    s.wakeword.custom_word     = gv('wakeword_word');
    s.wakeword.custom_display  = gv('wakeword_display');
    s.wakeword.threshold       = gi('wakeword_threshold', 20);
    s.wakeword.device_aec      = gc('wakeword_device_aec');
    s.wakeword.server_aec      = gc('wakeword_server_aec');
    s.wakeword.send_data       = gc('wakeword_send_data');
    s.wakeword.detection_in_listening = gc('wakeword_in_listen');
    // wifi
    s.wifi.method              = gv('wifi_method');
    // buttons
    s.buttons.boot_enable      = gc('buttons_boot');
    s.buttons.boot_gpio        = gi('boot_gpio');
    s.buttons.touch_enable     = gc('buttons_touch');
    s.buttons.touch_gpio       = gi('touch_btn_gpio');
    s.buttons.vol_enable       = gc('buttons_vol');
    s.buttons.vol_up_gpio      = gi('vol_up_gpio');
    s.buttons.vol_down_gpio    = gi('vol_down_gpio');
    s.buttons.slider_enable    = gc('buttons_slider');
    s.buttons.slider_pad1      = gi('slider_pad1');
    s.buttons.slider_pad2      = gi('slider_pad2');
    s.buttons.slider_pad3      = gi('slider_pad3');
    s.buttons.rotary_enable    = gc('buttons_rotary');
    s.buttons.rotary_a         = gi('rotary_a');
    s.buttons.rotary_b         = gi('rotary_b');
    s.buttons.rotary_key       = gi('rotary_key');
    s.buttons.user_custom_sensors = gc('user_custom_sensors');
    // camera
    s.camera.enable            = gc('camera_enable');
    s.camera.sensor            = gv('camera_sensor');
    s.camera.pin_xclk=gi('cam_xclk'); s.camera.pin_pclk=gi('cam_pclk');
    s.camera.pin_vsync=gi('cam_vsync'); s.camera.pin_href=gi('cam_href');
    s.camera.pin_siod=gi('cam_siod'); s.camera.pin_sioc=gi('cam_sioc');
    s.camera.use_reset=gc('cam_use_reset'); s.camera.pin_reset=gi('cam_reset');
    s.camera.use_pwdn=gc('cam_use_pwdn');   s.camera.pin_pwdn=gi('cam_pwdn');
    for (let n=0;n<=7;n++) s.camera[`pin_d${n}`]=gi(`cam_d${n}`);
    s.camera.hmirror=gc('cam_hmirror'); s.camera.vflip=gc('cam_vflip');
    // led
    s.led.enable=gc('led_enable'); s.led.type=gv('led_type');
    s.led.gpio=gi('led_gpio'); s.led.count=gi('led_count',1);
    s.led.rainbow=gc('led_rainbow');
    // mcp
    s.mcp.enable=gc('mcp_enable'); s.mcp.lamp=gc('mcp_lamp');
    s.mcp.lamp_gpio=gi('mcp_lamp_gpio');
    s.mcp.sensor=gc('mcp_sensor'); s.mcp.actuator=gc('mcp_actuator');
    // uart
    s.uart.enable=gc('uart_enable'); s.uart.port=gv('uart_port');
    s.uart.pin_tx=gi('uart_tx'); s.uart.pin_rx=gi('uart_rx');
    s.uart.baudrate=gi('uart_baud',115200); s.uart.flow_ctrl=gc('uart_flow');
    s.uart.pin_rts=gi('uart_rts'); s.uart.pin_cts=gi('uart_cts');
    // actuators
    s.actuators.servo_enable=gc('servo_enable');
    s.actuators.servo_gpio=gi('servo_gpio');
    s.actuators.buzzer_enable=gc('buzzer_enable');
    s.actuators.buzzer_gpio=gi('buzzer_gpio');
    s.actuators.haptic_enable=gc('haptic_enable');
    s.actuators.haptic_gpio=gi('haptic_gpio');
    // motor
    s.motor.relay_enable=gc('relay_enable'); s.motor.relay_gpio=gi('relay_gpio');
    s.motor.dc_enable=gc('motor_dc_enable'); s.motor.dc_driver=gv('motor_driver');
    s.motor.pwma=gi('motor_pwma'); s.motor.dira=gi('motor_dira');
    s.motor.pwmb=gi('motor_pwmb'); s.motor.dirb=gi('motor_dirb');
    // sensors
    s.sensors.dht_enable=gc('sensor_dht'); s.sensors.dht_gpio=gi('dht_gpio');
    s.sensors.temp_enable=gc('sensor_temp'); s.sensors.aht20=gc('sensor_aht20');
    s.sensors.temp_sda=gi('temp_sda'); s.sensors.temp_scl=gi('temp_scl');
    s.sensors.bmp280=gc('sensor_bmp280');
    s.sensors.bmp280_sda=gi('bmp280_sda'); s.sensors.bmp280_scl=gi('bmp280_scl');
    s.sensors.bh1750=gc('sensor_bh1750');
    s.sensors.bh1750_sda=gi('bh1750_sda'); s.sensors.bh1750_scl=gi('bh1750_scl');
    s.sensors.ldr=gc('sensor_ldr'); s.sensors.ldr_ch=gi('ldr_ch',1);
    s.sensors.gas_co2=gc('sensor_gas_co2'); s.sensors.gas_scd4x=gc('gas_scd4x');
    s.sensors.gas_sda=gi('gas_sda'); s.sensors.gas_scl=gi('gas_scl');
    s.sensors.gas_mq=gc('sensor_gas_mq'); s.sensors.gas_mq_gpio=gi('gas_mq_gpio');
    s.sensors.hcsr04=gc('sensor_hcsr04');
    s.sensors.hcsr04_trig=gi('hcsr04_trig'); s.sensors.hcsr04_echo=gi('hcsr04_echo');
    s.sensors.pir=gc('sensor_pir'); s.sensors.pir_gpio=gi('pir_gpio');
    s.sensors.vibration=gc('sensor_vibration');
    s.sensors.vibration_gpio=gi('vibration_gpio');
    s.sensors.flame=gc('sensor_flame'); s.sensors.flame_gpio=gi('flame_gpio');
    // power
    s.power.battery_enable=gc('battery_enable'); s.power.battery_adc=gc('battery_adc');
    s.power.battery_ch=gi('battery_ch',0);
    s.power.battery_r1=gi('battery_r1',100); s.power.battery_r2=gi('battery_r2',100);
    s.power.ina2xx=gc('ina2xx_enable'); s.power.ina219=gc('ina219');
    s.power.ina_sda=gi('ina_sda'); s.power.ina_scl=gi('ina_scl');
    s.power.tp4056=gc('tp4056_enable'); s.power.tp4056_gpio=gi('tp4056_gpio');
  }

  // ── Conditional UI helpers ────────────────────────────────────────────────
  static _show(sel, show) {
    document.querySelectorAll(sel).forEach(el =>
      el.style.display = show ? '' : 'none');
  }
  static _toggleGroup(sel, checkboxId) {
    const checked = !!document.getElementById(checkboxId)?.checked;
    GUIController._show(sel, checked);
  }

  static _toggleDisplayGroups() {
    const enabled = !!document.getElementById('display_enable')?.checked;
    GUIController._show('.display-common', enabled);
    const type = document.getElementById('display_type')?.value || '';
    const isSpi   = enabled && /ST7789|ST7796|ST7735|ILI9341|ILI9486|GC9A01|GC9107|NV3023|JD9853|ST7701/.test(type);
    const isOled  = enabled && /OLED_SSD1306|OLED_SH1106/.test(type);
    const isQspi  = enabled && type.includes('QSPI_AMOLED');
    const isEpaper= enabled && type.includes('EPAPER');
    const isRgb   = enabled && type.includes('ST7701');
    const isUart  = enabled && (type.includes('UART') || !!document.getElementById('display_uart_secondary')?.checked);
    GUIController._show('.display-spi',      isSpi);
    GUIController._show('.display-oled',     isOled);
    GUIController._show('.display-qspi',     isQspi);
    GUIController._show('.display-epaper',   isEpaper);
    GUIController._show('.display-rgb',      isRgb);
    GUIController._show('.display-uart-cfg', isUart);
    // auto-fill resolution
    const autoRes = {
      'CUSTOM_DISPLAY_ST7796':[320,480],'CUSTOM_DISPLAY_ILI9486':[320,480],
      'CUSTOM_DISPLAY_GC9A01':[240,240],'CUSTOM_DISPLAY_GC9107':[128,128],
      'CUSTOM_DISPLAY_OLED_SSD1306':[128,64],'CUSTOM_DISPLAY_OLED_SH1106':[128,64],
      'CUSTOM_DISPLAY_ST7735':[128,160],
    };
    if (enabled && type in autoRes && !GUIController.dirty['panel-display']) {
      const [w,h] = autoRes[type];
      const we = document.getElementById('display_width');
      const he = document.getElementById('display_height');
      if (we && we.value !== String(w)) we.value = w;
      if (he && he.value !== String(h)) he.value = h;
    }
  }

  static _toggleMultilineChat() {
    const style = document.getElementById('general_display_style')?.value;
    const isDefault = style === 'USE_DEFAULT_MESSAGE_STYLE';
    const el = document.getElementById('general_multiline');
    if (el) {
      el.disabled = !isDefault;
      if (!isDefault) el.checked = false;
      const parent = el.closest('.form-check') || el.parentElement;
      if (parent) parent.style.opacity = isDefault ? '1' : '0.4';
    }
  }

  static _toggleCodecGroup(prefix) {
    const type = document.getElementById(`${prefix === 'spk' ? 'speaker' : 'mic'}_type`)?.value || '';
    const isCodec = type.includes('CODEC');
    GUIController._show(`.${prefix}-codec-fields`, isCodec);
  }

  static _toggleCameraUsb() {
    const enabled = !!document.getElementById('camera_enable')?.checked;
    const isUsb = document.getElementById('camera_sensor')?.value === 'CUSTOM_CAMERA_USB_UVC';
    GUIController._show('.cam-fields', enabled);
    GUIController._show('.cam-dvp-fields', enabled && !isUsb);
  }

  static _toggleLedRainbow() {
    const type = document.getElementById('led_type')?.value || '';
    const hasRainbow = /WS2812|CIRCULAR/.test(type);
    GUIController._show('.led-rainbow-field', hasRainbow);
  }

  static _toggleWakewordCustom() {
    const type = document.getElementById('wakeword_type')?.value || '';
    const isCustom = type === 'USE_CUSTOM_WAKE_WORD';
    GUIController._show('.wakeword-custom-fields', isCustom);

    // Kconfig: WAKE_WORD_DETECTION_IN_LISTENING depends on USE_AFE_WAKE_WORD || USE_CUSTOM_WAKE_WORD
    const canListen = ['USE_AFE_WAKE_WORD', 'USE_CUSTOM_WAKE_WORD'].includes(type);
    const inListen = document.getElementById('wakeword_in_listen');
    if (inListen) {
      inListen.disabled = !canListen;
      if (!canListen) inListen.checked = false;
      const p = inListen.closest('.form-check') || inListen.parentElement;
      if (p) p.style.opacity = canListen ? '1' : '0.4';
    }
  }

  static _syncFlashPartition() {
    const size = document.getElementById('flash_size')?.value;
    const pt   = document.getElementById('sys_partition_table');
    if (pt && size === '16MB' && !pt.value.includes('16')) {
      pt.value = 'partitions/16m.csv';
    }
  }

  // ── PIN MATRIX ─────────────────────────────────────────────────────────────
  static renderPinMatrix() {
    const grid = document.getElementById('pin-matrix-grid');
    if (!grid) return;
    grid.innerHTML = '';
    for (let pin = 0; pin <= 48; pin++) {
      const cell = document.createElement('div');
      cell.className = 'pin-cell';
      cell.id = `pin-cell-${pin}`;
      cell.textContent = pin;
      if (ESP32S3.LOCKED.includes(pin)) {
        cell.classList.add('locked');
        cell.title = `GPIO ${pin}: KHÓA (Flash/PSRAM)`;
      } else {
        cell.title = `GPIO ${pin}: Trống`;
      }
      grid.appendChild(cell);
    }
  }

  // ── VALIDATION ────────────────────────────────────────────────────────────
  static updateValidation() {
    const { errors, usage } = PinValidator.validate(configState);
    conflictErrors = errors;

    // reset
    document.querySelectorAll('.form-control').forEach(el =>
      el.classList.remove('has-conflict','is-locked'));
    for (let p=0; p<=48; p++) {
      const cell = document.getElementById(`pin-cell-${p}`);
      if (cell && !ESP32S3.LOCKED.includes(p)) {
        cell.classList.remove('used','conflict');
        cell.title = `GPIO ${p}: Trống`;
      }
    }
    // mark used
    for (const [pin, users] of usage.entries()) {
      const cell = document.getElementById(`pin-cell-${pin}`);
      if (cell) {
        cell.classList.add('used');
        cell.title = `GPIO ${pin}: ${users.map(u=>u.name).join(', ')}`;
      }
    }
    // mark errors
    const bar = document.getElementById('conflict-alert-bar');
    const msg = document.getElementById('conflict-message');
    if (errors.length) {
      bar?.classList.add('visible');
      if (msg) msg.innerHTML = errors.map(e=>`• ${e.message}`).join('<br>');
      errors.forEach(e => {
        document.getElementById(`pin-cell-${e.pin}`)?.classList.add('conflict');
      });
      document.querySelectorAll('.btn-save-tab').forEach(b => b.disabled = true);
    } else {
      bar?.classList.remove('visible');
      document.querySelectorAll('.btn-save-tab').forEach(b => b.disabled = false);
    }
  }

  // ── TOAST ─────────────────────────────────────────────────────────────────
  static showToast(msg, type = 'success') {
    const c = document.getElementById('toast-container');
    if (!c) return;
    const t = document.createElement('div');
    t.className = `toast-item toast-${type}`;
    t.innerHTML = `<span>${{'success':'✓','error':'✕','warning':'⚠️'}[type]||'ℹ'}</span><span>${msg}</span>`;
    c.appendChild(t);
    setTimeout(() => { t.style.opacity='0'; setTimeout(()=>t.remove(),300); }, 3500);
  }

  // ── STATUS ────────────────────────────────────────────────────────────────
  static _setStatus(state, text) {
    const dot  = document.getElementById('fs-status-dot');
    const label= document.getElementById('fs-status-text');
    dot?.classList.toggle('connected', state === 'connected');
    dot?.classList.toggle('error',     state === 'error');
    if (label) label.textContent = text;
  }
}

// ============================================================================
// Bootstrap
// ============================================================================
if (typeof window !== 'undefined') {
  if (document.readyState === 'loading') {
    window.addEventListener('DOMContentLoaded', () => GUIController.init());
  } else {
    GUIController.init();
  }
}

// Node.js test export
if (typeof module !== 'undefined' && module.exports) {
  module.exports = { SdkconfigParser, SdkconfigGenerator, PinValidator, DEFAULT_CONFIG, GUIController, ESP32S3, BuildController };
}
