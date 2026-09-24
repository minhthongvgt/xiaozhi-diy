/**
 * Xiaozhi-ESP32 Web Configurator Engine
 * Target: ESP32-S3-N16R8 (Octal Flash 16MB, Octal PSRAM 8MB)
 * Scanned from: main/drivers (actuator, audio, display, input, sensor, storage)
 */

// ============================================================================
// CONSTANTS & ESP32-S3 HARDWARE RULES
// ============================================================================
const ESP32S3_RULES = {
  MIN_GPIO: 0,
  MAX_GPIO: 48,
  // GPIO 26-32 are strictly wired to Octal SPI Flash & Octal PSRAM on ESP32-S3-N16R8
  LOCKED_OCTAL_PINS: [26, 27, 28, 29, 30, 31, 32],
  STRAPPING_PINS: [0, 3, 45, 46],
};

// Default Hardware Configuration State (10 Categories matching Menuconfig)
const DEFAULT_CONFIG = {
  // 1. General & Language (Kconfig General)
  general: {
    ota_url: "https://api.tenclass.net/xiaozhi/ota/",
    language: "LANGUAGE_VI_VN",
    flash_assets: "FLASH_DEFAULT_ASSETS",
    custom_assets_file: "assets.bin"
  },

  // 2. Audio (main/drivers/audio)
  audio: {
    mode: "simplex",
    spk_driver: "max98357a",
    mic_driver: "inmp441",
    sample_rate: 16000,
    spk_bclk: 15,
    spk_ws: 16,
    spk_dout: 7,
    mic_sck: 5,
    mic_ws: 4,
    mic_din: 6,
    codec_sda: 8,
    codec_scl: 9,
    mclk: -1,
    pa_pin: -1
  },

  // 3. Display & UI Hardware (main/drivers/display & main/display)
  display: {
    type: "st7789",
    weather_city: "TP. Hồ Chí Minh",
    voice_wave: true,
    mosi: 47,
    clk: 21,
    cs: 41,
    dc: 40,
    rst: 42,
    blk: 38,
    i2c_sda: 8,
    i2c_scl: 9,
    uart_port: 1,
    uart_baud: 115200,
    uart_tx: 17,
    uart_rx: 18,
    touch_chip: "none",
    touch_sda: 8,
    touch_scl: 9,
    touch_int: 3,
    touch_rst: -1,
    width: 240,
    height: 320,
    offset_x: 0,
    offset_y: 0,
    invert: true,
    swap_xy: false,
    mirror_x: false,
    mirror_y: false
  },

  // 4. Wake Word & AI Speech (Kconfig Wake Word)
  wake_word: {
    type: "USE_AFE_WAKE_WORD",
    threshold: 20,
    pinyin: "xiao tu dou",
    display: "小土豆",
    device_aec: false,
    server_aec: true,
    debugger_server: ""
  },

  // 5. Network & Provisioning (Kconfig Network)
  network: {
    wifi_method: "USE_HOTSPOT_WIFI_PROVISIONING",
    secondary: "none"
  },

  // 6. MCP Server Tools (Kconfig MCP Server - Software Protocol Layer)
  mcp: {
    enable: true,
    tool_peripherals: true,
    tool_info: true,
    tool_reboot: true,
    tool_theme: true
  },

  // 7. Input Drivers (main/drivers/input)
  buttons: [
    { id: "btn_boot", type: "boot", name: "BOOT Button (Wi-Fi config)", pin: 0 }
  ],

  // 8. Actuator & Sensor Drivers (main/drivers/actuator & main/drivers/sensor)
  peripherals: [
    { id: "p_led", type: "led", name: "Builtin Status LED", pin: 48 },
    { id: "p_lamp", type: "relay_lamp", name: "Relay 220V / Đèn bàn", pin: 13 }
  ],

  // 9. Storage & Communication Drivers (main/drivers/storage)
  communication: [],

  // 10. Flash & System ESP32-S3-N16R8 (Always at bottom)
  flash_system: {
    flash_size: "16MB",
    flash_mode: "QIO",
    flash_freq: "80M",
    psram_mode: "OCT",
    psram_speed: "80M",
    cpu_freq: "240",
    partition_table: "partitions/v2/16m.csv"
  }
};

// Reactive State
let configState = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
let directoryHandle = null;
let conflictErrors = [];
let currentModalCategory = null;

// ============================================================================
// MODULE A: FILE SYSTEM ACCESS API ENGINE
// ============================================================================
class FileSystemEngine {
  static async openProjectDirectory() {
    if (!window.showDirectoryPicker) {
      alert("Trình duyệt hiện tại không hỗ trợ Web File System Access API. Vui lòng dùng Google Chrome hoặc Microsoft Edge trên máy tính.");
      return null;
    }

    try {
      const handle = await window.showDirectoryPicker({
        mode: "readwrite"
      });

      const isXiaozhi = await FileSystemEngine.validateIntegrity(handle);
      if (!isXiaozhi) {
        const proceed = confirm(
          "Cảnh báo: Không tìm thấy tệp 'main/Kconfig.projbuild' hoặc 'main/boards/'. Bạn có chắc đây là thư mục gốc của dự án xiaozhi-esp32 không?"
        );
        if (!proceed) return null;
      }

      return handle;
    } catch (err) {
      if (err.name !== "AbortError") {
        console.error("Lỗi khi mở thư mục:", err);
        alert("Không thể mở thư mục: " + err.message);
      }
      return null;
    }
  }

  static async validateIntegrity(rootHandle) {
    try {
      const mainHandle = await rootHandle.getDirectoryHandle("main");
      await mainHandle.getFileHandle("Kconfig.projbuild");
      await mainHandle.getDirectoryHandle("boards");
      return true;
    } catch (e) {
      return false;
    }
  }

  static async readFile(rootHandle, relativePath) {
    const parts = relativePath.split(/[/\\]+/).filter(Boolean);
    let currentDir = rootHandle;

    for (let i = 0; i < parts.length - 1; i++) {
      currentDir = await currentDir.getDirectoryHandle(parts[i]);
    }

    const fileHandle = await currentDir.getFileHandle(parts[parts.length - 1]);
    const file = await fileHandle.getFile();
    return await file.text();
  }

  static async writeFile(rootHandle, relativePath, content) {
    const parts = relativePath.split(/[/\\]+/).filter(Boolean);
    let currentDir = rootHandle;

    for (let i = 0; i < parts.length - 1; i++) {
      currentDir = await currentDir.getDirectoryHandle(parts[i], { create: true });
    }

    const fileHandle = await currentDir.getFileHandle(parts[parts.length - 1], { create: true });
    const writable = await fileHandle.createWritable();
    await writable.write(content);
    await writable.close();
    return true;
  }
}

// ============================================================================
// MODULE B: BI-DIRECTIONAL PARSER & CODE GENERATOR
// ============================================================================
class ConfigParser {
  static parseConfigH(content) {
    const s = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
    const lines = content.split(/\r?\n/);
    const defineRegex = /^\s*#define\s+([A-Za-z0-9_]+)\s+(.+)$/;

    for (const line of lines) {
      const match = line.match(defineRegex);
      if (!match) continue;

      const key = match[1];
      let val = match[2].trim();

      val = val.replace(/\/\/.*$/, "").trim();
      val = val.replace(/\/\*.*?\*\//, "").trim();
      val = val.replace(/static_cast<gpio_num_t>\\(([0-9\\-]+)\\)/, "$1");
      val = val.replace(/\(\(adc_channel_t\)([0-9\-]+)\)/, "$1");
      val = val.replace(/GPIO_NUM_([0-9]+)/, "$1");
      val = val.replace(/ADC_CHANNEL_([0-9]+)/, "$1");
      val = val.replace(/UART_NUM_([0-9]+)/, "$1");
      val = val.replace(/GPIO_NUM_NC/, "-1");

      const numVal = parseInt(val, 10);

      switch (key) {
        // Audio Drivers & Chips
        case "AUDIO_SPEAKER_DRIVER":
          s.audio.spk_driver = val.toLowerCase().replace(/["']/g, "");
          break;
        case "AUDIO_MIC_DRIVER":
          s.audio.mic_driver = val.toLowerCase().replace(/["']/g, "");
          break;
        case "AUDIO_INPUT_SAMPLE_RATE":
        case "AUDIO_OUTPUT_SAMPLE_RATE":
          if (!isNaN(numVal)) s.audio.sample_rate = numVal;
          break;
        case "AUDIO_I2S_SPK_GPIO_BCLK":
        case "AUDIO_I2S_GPIO_BCLK":
          if (!isNaN(numVal)) s.audio.spk_bclk = numVal;
          break;
        case "AUDIO_I2S_SPK_GPIO_LRCK":
        case "AUDIO_I2S_GPIO_WS":
          if (!isNaN(numVal)) s.audio.spk_ws = numVal;
          break;
        case "AUDIO_I2S_SPK_GPIO_DOUT":
        case "AUDIO_I2S_GPIO_DOUT":
          if (!isNaN(numVal)) s.audio.spk_dout = numVal;
          break;
        case "AUDIO_I2S_MIC_GPIO_SCK":
          if (!isNaN(numVal)) s.audio.mic_sck = numVal;
          break;
        case "AUDIO_I2S_MIC_GPIO_WS":
          if (!isNaN(numVal)) s.audio.mic_ws = numVal;
          break;
        case "AUDIO_I2S_MIC_GPIO_DIN":
        case "AUDIO_I2S_GPIO_DIN":
          if (!isNaN(numVal)) s.audio.mic_din = numVal;
          break;
        case "AUDIO_CODEC_I2C_SDA_PIN":
          if (!isNaN(numVal)) s.audio.codec_sda = numVal;
          break;
        case "AUDIO_CODEC_I2C_SCL_PIN":
          if (!isNaN(numVal)) s.audio.codec_scl = numVal;
          break;
        case "AUDIO_I2S_GPIO_MCLK":
          if (!isNaN(numVal)) s.audio.mclk = numVal;
          break;
        case "AUDIO_PA_PIN":
          if (!isNaN(numVal)) s.audio.pa_pin = numVal;
          break;

        // Display & Touch Drivers
        case "DISPLAY_TYPE_UART":
          s.display.type = "uart_display";
          break;
        case "DISPLAY_UART_PORT":
          if (!isNaN(numVal)) s.display.uart_port = numVal;
          break;
        case "DISPLAY_UART_BAUDRATE":
          if (!isNaN(numVal)) s.display.uart_baud = numVal;
          break;
        case "DISPLAY_UART_TX_PIN":
          if (!isNaN(numVal)) {
            s.display.uart_tx = numVal;
            s.display.type = "uart_display";
          }
          break;
        case "DISPLAY_UART_RX_PIN":
          if (!isNaN(numVal)) {
            s.display.uart_rx = numVal;
            s.display.type = "uart_display";
          }
          break;
        case "TOUCH_CONTROLLER_CST816S":
          s.display.touch_chip = "cst816s";
          break;
        case "TOUCH_CONTROLLER_GT911":
          s.display.touch_chip = "gt911";
          break;
        case "TOUCH_CONTROLLER_FT6236":
          s.display.touch_chip = "ft6236";
          break;
        case "TOUCH_I2C_SDA_PIN":
          if (!isNaN(numVal)) s.display.touch_sda = numVal;
          break;
        case "TOUCH_I2C_SCL_PIN":
          if (!isNaN(numVal)) s.display.touch_scl = numVal;
          break;
        case "TOUCH_INT_PIN":
          if (!isNaN(numVal)) s.display.touch_int = numVal;
          break;
        case "TOUCH_RST_PIN":
          if (!isNaN(numVal)) s.display.touch_rst = numVal;
          break;
        case "CUSTOM_ASSETS_FILE":
          s.general.custom_assets_file = val.replace(/["']/g, "");
          s.general.flash_assets = "FLASH_CUSTOM_ASSETS";
          break;
        case "DISPLAY_WIDTH":
          if (!isNaN(numVal)) s.display.width = numVal;
          break;
        case "DISPLAY_HEIGHT":
          if (!isNaN(numVal)) s.display.height = numVal;
          break;
        case "DISPLAY_OFFSET_X":
          if (!isNaN(numVal)) s.display.offset_x = numVal;
          break;
        case "DISPLAY_OFFSET_Y":
          if (!isNaN(numVal)) s.display.offset_y = numVal;
          break;
        case "DISPLAY_MOSI_PIN":
          if (!isNaN(numVal)) s.display.mosi = numVal;
          break;
        case "DISPLAY_CLK_PIN":
          if (!isNaN(numVal)) s.display.clk = numVal;
          break;
        case "DISPLAY_CS_PIN":
          if (!isNaN(numVal)) s.display.cs = numVal;
          break;
        case "DISPLAY_DC_PIN":
          if (!isNaN(numVal)) s.display.dc = numVal;
          break;
        case "DISPLAY_RST_PIN":
          if (!isNaN(numVal)) s.display.rst = numVal;
          break;
        case "DISPLAY_BACKLIGHT_PIN":
          if (!isNaN(numVal)) s.display.blk = numVal;
          break;
        case "DISPLAY_I2C_SDA_PIN":
        case "DISPLAY_SDA_PIN":
          if (!isNaN(numVal)) s.display.i2c_sda = numVal;
          break;
        case "DISPLAY_I2C_SCL_PIN":
        case "DISPLAY_SCL_PIN":
          if (!isNaN(numVal)) s.display.i2c_scl = numVal;
          break;
        case "DISPLAY_INVERT_COLOR":
          s.display.invert = val === "true" || val === "1";
          break;
        case "DISPLAY_SWAP_XY":
          s.display.swap_xy = val === "true" || val === "1";
          break;
        case "DISPLAY_MIRROR_X":
          s.display.mirror_x = val === "true" || val === "1";
          break;
        case "DISPLAY_MIRROR_Y":
          s.display.mirror_y = val === "true" || val === "1";
          break;

        // Input Drivers (main/drivers/input)
        case "BOOT_BUTTON_GPIO":
          if (!isNaN(numVal)) {
            const b = s.buttons.find(x => x.type === "boot");
            if (b) b.pin = numVal;
          }
          break;
        case "TOUCH_BUTTON_GPIO":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.buttons.some(x => x.type === "touch")) {
              s.buttons.push({ id: "btn_touch", type: "touch", name: "Touch Button", pin: numVal });
            }
          }
          break;
        case "WAKE_BUTTON_GPIO":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.buttons.some(x => x.type === "wake")) {
              s.buttons.push({ id: "btn_wake", type: "wake", name: "WAKE Button", pin: numVal });
            }
          }
          break;
        case "VOLUME_UP_BUTTON_GPIO":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.buttons.some(x => x.type === "vol_up")) {
              s.buttons.push({ id: "btn_vol_up", type: "vol_up", name: "Volume Up Button", pin: numVal });
            }
          }
          break;
        case "VOLUME_DOWN_BUTTON_GPIO":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.buttons.some(x => x.type === "vol_down")) {
              s.buttons.push({ id: "btn_vol_down", type: "vol_down", name: "Volume Down Button", pin: numVal });
            }
          }
          break;
        case "ROTARY_ENCODER_A_PIN":
          if (!isNaN(numVal) && numVal >= 0) {
            let rot = s.buttons.find(x => x.type === "rotary");
            if (!rot) {
              rot = { id: "btn_rotary", type: "rotary", name: "Rotary Encoder EC11", pin_a: numVal, pin_b: 18, pin_key: 21 };
              s.buttons.push(rot);
            } else {
              rot.pin_a = numVal;
            }
          }
          break;

        // Actuator Drivers (main/drivers/actuator)
        case "BUILTIN_LED_GPIO":
          if (!isNaN(numVal)) {
            const led = s.peripherals.find(x => x.type === "led");
            if (led) led.pin = numVal;
            else if (numVal >= 0) s.peripherals.push({ id: "p_led", type: "led", name: "Builtin Status LED", pin: numVal });
          }
          break;
        case "LAMP_GPIO":
          if (!isNaN(numVal)) {
            const lamp = s.peripherals.find(x => x.type === "relay_lamp");
            if (lamp) lamp.pin = numVal;
            else if (numVal >= 0) s.peripherals.push({ id: "p_lamp", type: "relay_lamp", name: "Relay 220V / Lamp MCP", pin: numVal });
          }
          break;
        case "BUZZER_PIN":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.peripherals.some(x => x.type === "buzzer")) {
              s.peripherals.push({ id: "p_buzzer", type: "buzzer", name: "Buzzer Alarm", pin: numVal });
            }
          }
          break;
        case "HAPTIC_PIN":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.peripherals.some(x => x.type === "haptic")) {
              s.peripherals.push({ id: "p_haptic", type: "haptic", name: "Haptic Vibration Motor", pin: numVal });
            }
          }
          break;
        case "SERVO_PIN":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.peripherals.some(x => x.type === "servo")) {
              s.peripherals.push({ id: "p_servo", type: "servo", name: "Servo Motor PWM 50Hz", pin: numVal });
            }
          }
          break;

        // Sensor Drivers (main/drivers/sensor)
        case "SENSOR_DHT_GPIO":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.peripherals.some(x => x.type === "sensor_dht")) {
              s.peripherals.push({ id: "p_dht", type: "sensor_dht", name: "Cảm biến nhiệt độ DHT11/22", pin: numVal });
            }
          }
          break;
        case "SENSOR_PIR_GPIO":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.peripherals.some(x => x.type === "sensor_pir")) {
              s.peripherals.push({ id: "p_pir", type: "sensor_pir", name: "Cảm biến chuyển động PIR", pin: numVal });
            }
          }
          break;
        case "SENSOR_VIBRATION_GPIO":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.peripherals.some(x => x.type === "sensor_vibration")) {
              s.peripherals.push({ id: "p_vib", type: "sensor_vibration", name: "Cảm biến rung SW-420", pin: numVal });
            }
          }
          break;
        case "SENSOR_FLAME_GPIO":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.peripherals.some(x => x.type === "sensor_flame")) {
              s.peripherals.push({ id: "p_flame", type: "sensor_flame", name: "Cảm biến ngọn lửa Flame", pin: numVal });
            }
          }
          break;
        case "SENSOR_TP4056_CHRG_GPIO":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.peripherals.some(x => x.type === "sensor_tp4056")) {
              s.peripherals.push({ id: "p_tp4056", type: "sensor_tp4056", name: "Giám sát sạc TP4056", pin: numVal });
            }
          }
          break;
        case "SENSOR_VL6180X_SDA":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.peripherals.some(x => x.type === "sensor_vl6180x")) {
              s.peripherals.push({ id: "p_vl6180x", type: "sensor_vl6180x", name: "Laser ToF VL6180X", sda: numVal, scl: 9 });
            }
          }
          break;
        case "SENSOR_AHT20_SDA":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.peripherals.some(x => x.type === "sensor_aht20")) {
              s.peripherals.push({ id: "p_aht20", type: "sensor_aht20", name: "Nhiệt/Ẩm AHT20/21", sda: numVal, scl: 9 });
            }
          }
          break;
        case "SENSOR_MPU6050_SDA":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.peripherals.some(x => x.type === "sensor_mpu6050")) {
              s.peripherals.push({ id: "p_mpu6050", type: "sensor_mpu6050", name: "Gia tốc MPU6050", sda: numVal, scl: 9 });
            }
          }
          break;
        case "SENSOR_MAX30102_SDA":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.peripherals.some(x => x.type === "sensor_max30102")) {
              s.peripherals.push({ id: "p_max30102", type: "sensor_max30102", name: "Nhịp tim MAX30102", sda: numVal, scl: 9 });
            }
          }
          break;
        case "PERIPH_WS2812B_PIN":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.peripherals.some(x => x.type === "led_ws2812b")) {
              s.peripherals.push({ id: "p_ws2812b", type: "led_ws2812b", name: "LED RGB WS2812B", pin: numVal, count: 12 });
            }
          }
          break;
        // Storage Drivers (main/drivers/storage)
        case "SDCARD_SPI_CS_PIN":
          if (!isNaN(numVal) && numVal >= 0) {
            if (!s.communication.some(x => x.type === "sdcard")) {
              s.communication.push({ id: "c_sdcard", type: "sdcard", name: "MicroSD Card (SPI)", cs: numVal, mosi: 11, miso: 13, sclk: 12 });
            }
          }
          break;

        // Communication
        case "CUSTOM_UART_TX_PIN":
          if (!isNaN(numVal) && numVal >= 0) {
            let u = s.communication.find(x => x.type === "uart");
            if (!u) {
              u = { id: "comm_uart", type: "uart", name: "UART mở rộng", port: 1, baudrate: 115200, tx: numVal, rx: -1 };
              s.communication.push(u);
            } else {
              u.tx = numVal;
            }
          }
          break;
        case "CUSTOM_UART_RX_PIN":
          if (!isNaN(numVal) && numVal >= 0) {
            let u = s.communication.find(x => x.type === "uart");
            if (!u) {
              u = { id: "comm_uart", type: "uart", name: "UART mở rộng", port: 1, baudrate: 115200, tx: -1, rx: numVal };
              s.communication.push(u);
            } else {
              u.rx = numVal;
            }
          }
          break;
      }
    }
    return s;
  }
}

// Code Generator for Target Board
class CodeGenerator {
  static generateConfigH(s) {
    const isSimplex = s.audio.mode === "simplex";
    const isEs8311 = s.audio.mode === "es8311";
    const isSpiDisplay = s.display.type === "st7789" || s.display.type === "gc9a01" || s.display.type === "ili9341";
    const isOled = s.display.type === "ssd1306";

    const gpioMacro = (pin) => (pin !== undefined && pin !== null && pin >= 0 ? `static_cast<gpio_num_t>(${pin})` : "GPIO_NUM_NC");

    // Extract buttons
    const bootBtn = s.buttons.find(b => b.type === "boot")?.pin ?? 0;
    const touchBtn = s.buttons.find(b => b.type === "touch")?.pin ?? -1;
    const wakeBtn = s.buttons.find(b => b.type === "wake")?.pin ?? -1;
    const volUpBtn = s.buttons.find(b => b.type === "vol_up")?.pin ?? -1;
    const volDownBtn = s.buttons.find(b => b.type === "vol_down")?.pin ?? -1;
    const rotary = s.buttons.find(b => b.type === "rotary");

    // Extract peripherals & actuators
    const ledPin = s.peripherals.find(p => p.type === "led")?.pin ?? -1;
    const lampPin = s.peripherals.find(p => p.type === "relay_lamp")?.pin ?? -1;
    const buzzerPin = s.peripherals.find(p => p.type === "buzzer")?.pin ?? -1;
    const hapticPin = s.peripherals.find(p => p.type === "haptic")?.pin ?? -1;
    const servoPin = s.peripherals.find(p => p.type === "servo")?.pin ?? -1;
    const dcMotor = s.peripherals.find(p => p.type === "motor_dc");

    // Extract sensors
    const dhtPin = s.peripherals.find(p => p.type === "sensor_dht")?.pin ?? -1;
    const pirPin = s.peripherals.find(p => p.type === "sensor_pir")?.pin ?? -1;
    const vibPin = s.peripherals.find(p => p.type === "sensor_vibration")?.pin ?? -1;
    const flamePin = s.peripherals.find(p => p.type === "sensor_flame")?.pin ?? -1;
    const tp4056Pin = s.peripherals.find(p => p.type === "sensor_tp4056")?.pin ?? -1;
    const vl6180x = s.peripherals.find(p => p.type === "sensor_vl6180x");
    const aht20 = s.peripherals.find(p => p.type === "sensor_aht20");
    const mpu6050 = s.peripherals.find(p => p.type === "sensor_mpu6050");
    const max30102 = s.peripherals.find(p => p.type === "sensor_max30102");
    const ws2812b = s.peripherals.find(p => p.type === "led_ws2812b");
    const hcsr04 = s.peripherals.find(p => p.type === "sensor_hcsr04");
    const battery = s.peripherals.find(p => p.type === "battery_adc");
    const gasSensor = s.peripherals.find(p => p.type === "sensor_gas");
    const ldrSensor = s.peripherals.find(p => p.type === "sensor_ldr");

    // Extract communication & storage
    const uart = s.communication.find(c => c.type === "uart");
    const i2c = s.communication.find(c => c.type === "i2c");
    const sdcard = s.communication.find(c => c.type === "sdcard");

    const isUartDisplay = s.display.type === "uart_display";
    const hasTouchScreen = s.display.touch_chip && s.display.touch_chip !== "none";

    return `// ==============================================================================
// Hardware Pinout Configuration for Custom ESP32-S3-N16R8 Board
// Generated by Xiaozhi Web Configurator (Zero-Install)
// Modules Scanned From: main/drivers
// ==============================================================================

#ifndef _CUSTOM_S3_N16R8_BOARD_CONFIG_H_
#define _CUSTOM_S3_N16R8_BOARD_CONFIG_H_

#include <sdkconfig.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <driver/uart.h>

// -----------------------------------------------------------------------------
// 1. Audio Drivers (main/drivers/audio)
// -----------------------------------------------------------------------------
#define AUDIO_INPUT_SAMPLE_RATE     ${s.audio.sample_rate}
#define AUDIO_OUTPUT_SAMPLE_RATE    ${s.audio.sample_rate}
#define AUDIO_SPEAKER_DRIVER        "${s.audio.spk_driver ?? 'max98357a'}"
#define AUDIO_MIC_DRIVER            "${s.audio.mic_driver ?? 'inmp441'}"

${isSimplex ? `// Simplex I2S Mode (INMP441 + MAX98357A)
#define AUDIO_I2S_METHOD_SIMPLEX    1
#define AUDIO_I2S_SPK_GPIO_BCLK     ${gpioMacro(s.audio.spk_bclk)}
#define AUDIO_I2S_SPK_GPIO_LRCK     ${gpioMacro(s.audio.spk_ws)}
#define AUDIO_I2S_SPK_GPIO_DOUT     ${gpioMacro(s.audio.spk_dout)}
#define AUDIO_I2S_MIC_GPIO_SCK      ${gpioMacro(s.audio.mic_sck)}
#define AUDIO_I2S_MIC_GPIO_WS       ${gpioMacro(s.audio.mic_ws)}
#define AUDIO_I2S_MIC_GPIO_DIN      ${gpioMacro(s.audio.mic_din)}
` : isEs8311 ? `// Integrated Codec ES8311 Mode
#define AUDIO_CODEC_ES8311          1
#define AUDIO_I2S_GPIO_BCLK         ${gpioMacro(s.audio.spk_bclk)}
#define AUDIO_I2S_GPIO_WS           ${gpioMacro(s.audio.spk_ws)}
#define AUDIO_I2S_GPIO_DOUT         ${gpioMacro(s.audio.spk_dout)}
#define AUDIO_I2S_GPIO_DIN          ${gpioMacro(s.audio.mic_din)}
#define AUDIO_I2S_GPIO_MCLK         ${gpioMacro(s.audio.mclk)}
#define AUDIO_CODEC_I2C_SDA_PIN     ${gpioMacro(s.audio.codec_sda)}
#define AUDIO_CODEC_I2C_SCL_PIN     ${gpioMacro(s.audio.codec_scl)}
` : `// Duplex I2S Mode
#define AUDIO_I2S_GPIO_BCLK         ${gpioMacro(s.audio.spk_bclk)}
#define AUDIO_I2S_GPIO_WS           ${gpioMacro(s.audio.spk_ws)}
#define AUDIO_I2S_GPIO_DOUT         ${gpioMacro(s.audio.spk_dout)}
#define AUDIO_I2S_GPIO_DIN          ${gpioMacro(s.audio.mic_din)}
`}
#define AUDIO_PA_PIN                ${gpioMacro(s.audio.pa_pin)}

// -----------------------------------------------------------------------------
// 2. Display & Touch Drivers (main/drivers/display)
// -----------------------------------------------------------------------------
#define DISPLAY_WIDTH               ${s.display.width}
#define DISPLAY_HEIGHT              ${s.display.height}
#define DISPLAY_OFFSET_X            ${s.display.offset_x}
#define DISPLAY_OFFSET_Y            ${s.display.offset_y}

#define DISPLAY_INVERT_COLOR        ${s.display.invert ? "true" : "false"}
#define DISPLAY_SWAP_XY             ${s.display.swap_xy ? "true" : "false"}
#define DISPLAY_MIRROR_X            ${s.display.mirror_x ? "true" : "false"}
#define DISPLAY_MIRROR_Y            ${s.display.mirror_y ? "true" : "false"}

${isSpiDisplay ? `// SPI LCD Panel (ST7789, GC9A01, ILI9341)
#define DISPLAY_SPI_MODE            0
#define DISPLAY_RGB_ORDER           LCD_RGB_ELEMENT_ORDER_RGB
#define DISPLAY_MOSI_PIN            ${gpioMacro(s.display.mosi)}
#define DISPLAY_CLK_PIN             ${gpioMacro(s.display.clk)}
#define DISPLAY_CS_PIN              ${gpioMacro(s.display.cs)}
#define DISPLAY_DC_PIN              ${gpioMacro(s.display.dc)}
#define DISPLAY_RST_PIN             ${gpioMacro(s.display.rst)}
#define DISPLAY_BACKLIGHT_PIN       ${gpioMacro(s.display.blk)}
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false
` : isUartDisplay ? `// UART Display Panel (Nextion / D-UART)
#define DISPLAY_TYPE_UART           1
#define DISPLAY_UART_PORT           UART_NUM_${s.display.uart_port ?? 1}
#define DISPLAY_UART_BAUDRATE       ${s.display.uart_baud ?? 115200}
#define DISPLAY_UART_TX_PIN         ${gpioMacro(s.display.uart_tx)}
#define DISPLAY_UART_RX_PIN         ${gpioMacro(s.display.uart_rx)}
#define DISPLAY_BACKLIGHT_PIN       GPIO_NUM_NC
` : isOled ? `// I2C OLED Panel (SSD1306)
#define DISPLAY_SDA_PIN             ${gpioMacro(s.display.i2c_sda)}
#define DISPLAY_SCL_PIN             ${gpioMacro(s.display.i2c_scl)}
#define DISPLAY_RST_PIN             ${gpioMacro(s.display.rst)}
#define DISPLAY_BACKLIGHT_PIN       GPIO_NUM_NC
` : `// No Display
#define DISPLAY_BACKLIGHT_PIN       GPIO_NUM_NC
`}

${hasTouchScreen ? `// Capacitive Touch Driver (${s.display.touch_chip.toUpperCase()})
#define TOUCH_CONTROLLER_${s.display.touch_chip.toUpperCase()} 1
#define TOUCH_I2C_SDA_PIN           ${gpioMacro(s.display.touch_sda)}
#define TOUCH_I2C_SCL_PIN           ${gpioMacro(s.display.touch_scl)}
#define TOUCH_INT_PIN               ${gpioMacro(s.display.touch_int)}
#define TOUCH_RST_PIN               ${gpioMacro(s.display.touch_rst)}
` : ``}
// -----------------------------------------------------------------------------
// 3. Input Drivers (main/drivers/input)
// -----------------------------------------------------------------------------
#define BOOT_BUTTON_GPIO            ${gpioMacro(bootBtn)}
#define TOUCH_BUTTON_GPIO           ${gpioMacro(touchBtn)}
#define WAKE_BUTTON_GPIO            ${gpioMacro(wakeBtn)}
#define VOLUME_UP_BUTTON_GPIO       ${gpioMacro(volUpBtn)}
#define VOLUME_DOWN_BUTTON_GPIO     ${gpioMacro(volDownBtn)}

${rotary ? `// Rotary Encoder EC11
#define ROTARY_ENCODER_A_PIN        ${gpioMacro(rotary.pin_a)}
#define ROTARY_ENCODER_B_PIN        ${gpioMacro(rotary.pin_b)}
#define ROTARY_ENCODER_KEY_PIN      ${gpioMacro(rotary.pin_key)}
` : ``}

// -----------------------------------------------------------------------------
// 4. Actuator Drivers (main/drivers/actuator)
// -----------------------------------------------------------------------------
#define BUILTIN_LED_GPIO            ${gpioMacro(ledPin)}
#define LAMP_GPIO                   ${gpioMacro(lampPin)}
#define BUZZER_PIN                  ${gpioMacro(buzzerPin)}
#define HAPTIC_PIN                  ${gpioMacro(hapticPin)}
#define SERVO_PIN                   ${gpioMacro(servoPin)}

${dcMotor ? `// DC Motor H-Bridge Driver (TB6612)
#define MOTOR_PWMA_PIN              ${gpioMacro(dcMotor.pwma)}
#define MOTOR_DIRA_PIN              ${gpioMacro(dcMotor.dira)}
#define MOTOR_PWMB_PIN              ${gpioMacro(dcMotor.pwmb)}
#define MOTOR_DIRB_PIN              ${gpioMacro(dcMotor.dirb)}
` : ``}

// -----------------------------------------------------------------------------
// 5. Sensor Drivers (main/drivers/sensor)
// -----------------------------------------------------------------------------
#define SENSOR_DHT_GPIO             ${gpioMacro(dhtPin)}
#define SENSOR_PIR_GPIO             ${gpioMacro(pirPin)}
#define SENSOR_VIBRATION_GPIO       ${gpioMacro(vibPin)}
#define SENSOR_FLAME_GPIO           ${gpioMacro(flamePin)}
#define SENSOR_TP4056_CHRG_GPIO     ${gpioMacro(tp4056Pin)}

${hcsr04 ? `#define SENSOR_HCSR04_TRIG_GPIO     ${gpioMacro(hcsr04.trig_pin)}
#define SENSOR_HCSR04_ECHO_GPIO     ${gpioMacro(hcsr04.echo_pin)}
` : ``}

${vl6180x ? `// Laser ToF Distance Sensor VL6180X
#define SENSOR_VL6180X_SDA          ${gpioMacro(vl6180x.sda)}
#define SENSOR_VL6180X_SCL          ${gpioMacro(vl6180x.scl)}
${vl6180x.addr ? `#define SENSOR_VL6180X_I2C_ADDR     ${vl6180x.addr}\n` : ``}` : ``}

${aht20 ? `// AHT20/21 Temperature & Humidity Sensor
#define SENSOR_AHT20_SDA            ${gpioMacro(aht20.sda)}
#define SENSOR_AHT20_SCL            ${gpioMacro(aht20.scl)}
${aht20.addr ? `#define SENSOR_AHT20_I2C_ADDR       ${aht20.addr}\n` : ``}` : ``}

${mpu6050 ? `// MPU6050 IMU Accelerometer/Gyroscope
#define SENSOR_MPU6050_SDA          ${gpioMacro(mpu6050.sda)}
#define SENSOR_MPU6050_SCL          ${gpioMacro(mpu6050.scl)}
${mpu6050.addr ? `#define SENSOR_MPU6050_I2C_ADDR     ${mpu6050.addr}\n` : ``}` : ``}

${max30102 ? `// MAX30102 Heart Rate Sensor
#define SENSOR_MAX30102_SDA         ${gpioMacro(max30102.sda)}
#define SENSOR_MAX30102_SCL         ${gpioMacro(max30102.scl)}
${max30102.addr ? `#define SENSOR_MAX30102_I2C_ADDR    ${max30102.addr}\n` : ``}` : ``}

${ws2812b ? `// WS2812B RGB LED Strip (RMT)
#define PERIPH_WS2812B_PIN          ${gpioMacro(ws2812b.pin)}
#define PERIPH_WS2812B_COUNT        ${ws2812b.count}
` : ``}

${gasSensor ? `#define SENSOR_GAS_ADC_CHANNEL      ((adc_channel_t)${gasSensor.channel})
` : ``}
${ldrSensor ? `#define SENSOR_LDR_ADC_CHANNEL      ((adc_channel_t)${ldrSensor.channel})
` : ``}
${battery ? `#define BATTERY_ADC_CHANNEL         ((adc_channel_t)${battery.channel})
#define BATTERY_DIVIDER_R1          100
#define BATTERY_DIVIDER_R2          100
` : ``}

// -----------------------------------------------------------------------------
// 6. Storage & Comm Drivers (main/drivers/storage)
// -----------------------------------------------------------------------------
${sdcard ? `// MicroSD Card via SPI Bus
#define SDCARD_SPI_CS_PIN           ${gpioMacro(sdcard.cs)}
#define SDCARD_SPI_MOSI_PIN         ${gpioMacro(sdcard.mosi)}
#define SDCARD_SPI_MISO_PIN         ${gpioMacro(sdcard.miso)}
#define SDCARD_SPI_CLK_PIN          ${gpioMacro(sdcard.sclk)}
` : ``}

${uart ? `// Custom UART Expansion
#define CUSTOM_UART_PORT            UART_NUM_${uart.port}
#define CUSTOM_UART_BAUDRATE        ${uart.baudrate}
#define CUSTOM_UART_TX_PIN          ${gpioMacro(uart.tx)}
#define CUSTOM_UART_RX_PIN          ${gpioMacro(uart.rx)}
` : ``}

${i2c ? `// Shared I2C Bus Master
#define SHARED_I2C_PORT             ${i2c.port}
#define SHARED_I2C_SDA_PIN          ${gpioMacro(i2c.sda)}
#define SHARED_I2C_SCL_PIN          ${gpioMacro(i2c.scl)}
` : ``}

#endif // _CUSTOM_S3_N16R8_BOARD_CONFIG_H_
`;
  }

  static generateConfigJson(s) {
    const f = s.flash_system;
    const sdkAppend = [
      "CONFIG_BOARD_TYPE_CUSTOM_S3_N16R8=y",
      `CONFIG_${s.general ? s.general.language : "LANGUAGE_VI_VN"}=y`,
      `CONFIG_${s.general ? s.general.flash_assets : "FLASH_DEFAULT_ASSETS"}=y`,
      `CONFIG_${s.wake_word ? s.wake_word.type : "USE_AFE_WAKE_WORD"}=y`,
      `CONFIG_${s.network ? s.network.wifi_method : "USE_HOTSPOT_WIFI_PROVISIONING"}=y`,
      `CONFIG_ENABLE_CUSTOM_MCP_SERVER=${s.mcp && s.mcp.enable ? "y" : "n"}`,
      `CONFIG_ESPTOOLPY_FLASHSIZE_${f.flash_size}=y`,
      `CONFIG_ESPTOOLPY_FLASHMODE_${f.flash_mode}=y`,
      `CONFIG_ESPTOOLPY_FLASHFREQ_${f.flash_freq}=y`,
      "CONFIG_PARTITION_TABLE_CUSTOM=y",
      `CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="${f.partition_table}"`,
      'CONFIG_PARTITION_TABLE_FILENAME="partitions.csv"',
      "CONFIG_PARTITION_TABLE_OFFSET=0x8000",
      `CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_${f.cpu_freq}=y`,
      "CONFIG_ESP32S3_INSTRUCTION_CACHE_32KB=y",
      "CONFIG_ESP32S3_DATA_CACHE_32KB=y",
      "CONFIG_ESP32S3_DATA_CACHE_LINE_64B=y"
    ];

    if (f.psram_mode !== "none") {
      sdkAppend.push(
        "CONFIG_SPIRAM=y",
        `CONFIG_SPIRAM_MODE_${f.psram_mode}=y`,
        `CONFIG_SPIRAM_TYPE_ESP32S3_${f.psram_mode}=y`,
        `CONFIG_SPIRAM_SPEED_${f.psram_speed}=y`
      );
    } else {
      sdkAppend.push("# CONFIG_SPIRAM is not set");
    }

    if (s.general && s.general.flash_assets === "FLASH_CUSTOM_ASSETS") {
      sdkAppend.push(`CONFIG_CUSTOM_ASSETS_FILE="${s.general.custom_assets_file || "assets.bin"}"`);
    }

    if (s.display.type === "user_custom") {
      sdkAppend.push("CONFIG_CUSTOM_DISPLAY_USER_CUSTOM=y");
    } else if (s.display.type === "uart_display") {
      sdkAppend.push("CONFIG_DISPLAY_TYPE_UART=y");
    } else {
      sdkAppend.push("# CONFIG_DISPLAY_TYPE_UART is not set");
      sdkAppend.push("# CONFIG_CUSTOM_DISPLAY_USER_CUSTOM is not set");
    }

    const touchChips = ["CST816", "FT6X36", "GT911", "CHSC5816"];
    if (s.display.touch_chip && s.display.touch_chip !== "none") {
      const selected = s.display.touch_chip.toUpperCase();
      if (selected === "USER_CUSTOM") {
        sdkAppend.push("CONFIG_CUSTOM_TOUCH_USER_CUSTOM=y");
        touchChips.forEach(chip => sdkAppend.push(`# CONFIG_CUSTOM_TOUCH_${chip} is not set`));
      } else {
        sdkAppend.push(`CONFIG_CUSTOM_TOUCH_${selected}=y`);
        touchChips.forEach(chip => {
          if (chip !== selected) sdkAppend.push(`# CONFIG_CUSTOM_TOUCH_${chip} is not set`);
        });
        sdkAppend.push("# CONFIG_CUSTOM_TOUCH_USER_CUSTOM is not set");
      }
    } else {
      touchChips.forEach(chip => sdkAppend.push(`# CONFIG_CUSTOM_TOUCH_${chip} is not set`));
      sdkAppend.push("# CONFIG_CUSTOM_TOUCH_USER_CUSTOM is not set");
    }

    if (s.audio && s.audio.spk_driver === "user_custom") {
      sdkAppend.push("CONFIG_CUSTOM_AUDIO_SPK_CODEC_USER_CUSTOM=y");
    }
    if (s.audio && s.audio.mic_driver === "user_custom") {
      sdkAppend.push("CONFIG_CUSTOM_AUDIO_MIC_CODEC_USER_CUSTOM=y");
    }

    if (s.peripherals && s.peripherals.some(p => p.type === "user_custom_led")) {
      sdkAppend.push("CONFIG_CUSTOM_LED_USER_CUSTOM=y");
    }
    if (s.peripherals && s.peripherals.some(p => p.type === "user_custom_sensors")) {
      sdkAppend.push("CONFIG_CUSTOM_ENABLE_USER_CUSTOM_SENSORS=y");
    }

    const configObj = {
      target: "esp32s3",
      builds: [
        {
          name: "custom-s3-n16r8",
          sdkconfig_append: sdkAppend
        }
      ]
    };
    return JSON.stringify(configObj, null, 4);
  }

  static generateBoardCC(s) {
    const isSimplex = s.audio.mode === "simplex";
    const isEs8311 = s.audio.mode === "es8311";
    const isOled = s.display.type === "ssd1306";
    const isSpiDisplay = s.display.type === "st7789" || s.display.type === "gc9a01" || s.display.type === "ili9341";
    const isUartDisplay = s.display.type === "uart_display";
    const hasBacklight = isSpiDisplay && s.display.blk >= 0;

    const hasLed = s.peripherals.some(p => p.type === "led" && p.pin >= 0);
    const hasLamp = s.peripherals.some(p => p.type === "relay_lamp" && p.pin >= 0);
    const hasTouch = s.buttons.some(b => b.type === "touch" && b.pin >= 0);
    const hasTouchScreen = Boolean(s.display.touch_chip && s.display.touch_chip !== "none");
    const needI2cBus = hasTouchScreen || isEs8311;

    return `// ==============================================================================
// Board Implementation for Custom ESP32-S3-N16R8
// Generated by Xiaozhi Web Configurator (Zero-Install)
// Integrates drivers from main/drivers
// ==============================================================================

#include "wifi_board.h"
#include "user_driver_registry.h"
#include "audio/codecs/no_audio_codec.h"
${isEs8311 ? '#include "audio/codecs/es8311_audio_codec.h"\n' : ''}
${isOled ? '#include "display/oled_display.h"\n' : ''}
${isSpiDisplay ? '#include "display/lcd_display.h"\n' : ''}
${isUartDisplay ? '#include "display/uart_display.h"\n' : ''}
${s.display.type === "gc9a01" ? '#include "esp_lcd_gc9a01.h"\n' : ''}
${s.display.type === "ili9341" ? '#include "esp_lcd_ili9341.h"\n' : ''}
${hasTouchScreen && s.display.touch_chip === "cst816s" ? '#include <esp_lcd_touch_cst816s.h>\n' : ''}
${hasTouchScreen && s.display.touch_chip === "gt911" ? '#include <esp_lcd_touch_gt911.h>\n' : ''}
${hasTouchScreen && s.display.touch_chip === "ft6236" ? '#include <esp_lcd_touch_ft5x06.h>\n' : ''}
${s.display.type === "none" ? '#include "display/no_display.h"\n' : ''}
#include "application.h"
#include "button.h"
#include "config.h"
#include "backlight.h"
${hasLamp ? '#include "lamp_controller.h"\n' : ''}
${hasLed ? '#include "led/single_led.h"\n' : ''}

#include <esp_log.h>
#include <driver/i2c_master.h>
#include <driver/spi_common.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>

#define TAG "CustomS3N16R8Board"

class CustomS3N16R8Board : public WifiBoard {
private:
    Button boot_button_;
${hasTouch ? '    Button touch_button_;\n' : ''}
    Display* display_ = nullptr;
${needI2cBus ? '    i2c_master_bus_handle_t i2c_bus_ = nullptr;\n' : ''}${isOled ? '    i2c_master_bus_handle_t display_i2c_bus_;\n' : ''}
${hasTouchScreen ? '    esp_lcd_touch_handle_t touch_handle_ = nullptr;\n' : ''}

${isSpiDisplay ? `    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_MOSI_PIN;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_CLK_PIN;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeLcdDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = static_cast<int>(DISPLAY_CS_PIN);
        io_config.dc_gpio_num = static_cast<int>(DISPLAY_DC_PIN);
        io_config.spi_mode = DISPLAY_SPI_MODE;
        io_config.pclk_hz = 40 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &panel_io));

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = static_cast<int>(DISPLAY_RST_PIN);
        panel_config.rgb_ele_order = DISPLAY_RGB_ORDER;
        panel_config.bits_per_pixel = 16;
${s.display.type === "gc9a01" ?
`        ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(panel_io, &panel_config, &panel));` :
s.display.type === "ili9341" ?
`        ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(panel_io, &panel_config, &panel));` :
`        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_config, &panel));`}

        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, DISPLAY_INVERT_COLOR);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);

        display_ = new SpiLcdDisplay(panel_io, panel,
                                    DISPLAY_WIDTH, DISPLAY_HEIGHT,
                                    DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,
                                    DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y,
                                    DISPLAY_SWAP_XY);
    }` : isUartDisplay ? `    void InitializeUartDisplay() {
        ESP_LOGI(TAG, "Initializing External UART Display (TX: %d, RX: %d, Baud: %d)",
                 DISPLAY_UART_TX_PIN, DISPLAY_UART_RX_PIN, DISPLAY_UART_BAUDRATE);
        UartDisplayProtocol proto = UartDisplayProtocol::NextionTjc;
        display_ = new UartDisplay(DISPLAY_UART_PORT, DISPLAY_UART_TX_PIN, DISPLAY_UART_RX_PIN,
                                   DISPLAY_UART_BAUDRATE, proto);
    }` : isOled ? `    void InitializeOledDisplay() {
        i2c_master_bus_config_t bus_config = {
            .i2c_port = (i2c_port_t)0,
            .sda_io_num = DISPLAY_SDA_PIN,
            .scl_io_num = DISPLAY_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = { .enable_internal_pullup = 1 },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &display_i2c_bus_));

        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;
        esp_lcd_panel_io_i2c_config_t io_config = {
            .dev_addr = 0x3C,
            .scl_speed_hz = 400 * 1000,
            .control_phase_bytes = 1,
            .dc_bit_offset = 6,
            .lcd_cmd_bits = 8,
            .lcd_param_bits = 8,
        };
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(display_i2c_bus_, &io_config, &panel_io));

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_RST_PIN;
        panel_config.bits_per_pixel = 1;
        ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(panel_io, &panel_config, &panel));

        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_disp_on_off(panel, true);

        display_ = new OledDisplay(panel_io, panel, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
    }` : `    void InitializeNoDisplay() {
        display_ = new NoDisplay();
    }`}

${needI2cBus ? `    void InitializeI2cBus() {
        if (i2c_bus_ != nullptr) return;
        i2c_master_bus_config_t bus_config = {
            .i2c_port = (i2c_port_t)0,
            .sda_io_num = ${hasTouchScreen ? 'static_cast<gpio_num_t>(TOUCH_I2C_SDA_PIN)' : 'static_cast<gpio_num_t>(AUDIO_CODEC_I2C_SDA_PIN)'},
            .scl_io_num = ${hasTouchScreen ? 'static_cast<gpio_num_t>(TOUCH_I2C_SCL_PIN)' : 'static_cast<gpio_num_t>(AUDIO_CODEC_I2C_SCL_PIN)'},
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = { .enable_internal_pullup = 1 },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_));
    }\n` : ''}
${hasTouchScreen ? `    void InitializeTouchScreen() {
        ESP_LOGI(TAG, "Initializing Touch Screen Driver (${s.display.touch_chip.toUpperCase()})");
        InitializeI2cBus();
        esp_lcd_panel_io_handle_t tp_io_handle = nullptr;
        esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_DEFAULT_CONFIG();
        esp_lcd_touch_config_t tp_cfg = {
            .x_max = DISPLAY_WIDTH,
            .y_max = DISPLAY_HEIGHT,
            .rst_gpio_num = static_cast<gpio_num_t>(TOUCH_RST_PIN),
            .int_gpio_num = static_cast<gpio_num_t>(TOUCH_INT_PIN),
            .levels = { .reset = 0, .interrupt = 0 },
            .flags = { .swap_xy = 0, .mirror_x = 0, .mirror_y = 0 },
        };
${s.display.touch_chip === "cst816s" ?
`        tp_io_config.dev_addr = ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus_, &tp_io_config, &tp_io_handle));
        ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst816s(tp_io_handle, &tp_cfg, &touch_handle_));
        ` :
s.display.touch_chip === "gt911" ?
`        tp_io_config.dev_addr = ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus_, &tp_io_config, &tp_io_handle));
        ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &touch_handle_));
        ` :
`        tp_io_config.dev_addr = ESP_LCD_TOUCH_IO_I2C_FT5x06_ADDRESS;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus_, &tp_io_config, &tp_io_handle));
        ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &touch_handle_));
        `}
    }` : ''}

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting) {
                EnterWifiConfigMode();
                return;
            }
            app.ToggleChatState();
        });
    }

${hasLamp || (s.peripherals && s.peripherals.some(p => p.type === "user_custom_sensors")) ? `    void InitializeTools() {
${hasLamp ? '        static LampController lamp(LAMP_GPIO);\n' : ''}${s.peripherals && s.peripherals.some(p => p.type === "user_custom_sensors") ? '        InitializeUserCustomSensors(this);\n' : ''}    }` : ''}

public:
    CustomS3N16R8Board() :
        boot_button_(BOOT_BUTTON_GPIO)
${hasTouch ? '        , touch_button_(TOUCH_BUTTON_GPIO)\n' : ''}
    {
${needI2cBus ? '        InitializeI2cBus();\n' : ''}${isSpiDisplay ? '        InitializeSpi();\n        InitializeLcdDisplay();\n' : ''}
${isOled ? '        InitializeOledDisplay();\n' : ''}
${isUartDisplay ? '        InitializeUartDisplay();\n' : ''}
${s.display.type === "none" ? '        InitializeNoDisplay();\n' : ''}
${hasTouchScreen ? '        InitializeTouchScreen();\n' : ''}
        InitializeButtons();
${hasLamp ? '        InitializeTools();\n' : ''}

${hasBacklight ? `        if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
            GetBacklight()->RestoreBrightness();
        }` : ''}
    }

    virtual Led* GetLed() override {
${s.peripherals && s.peripherals.some(p => p.type === "user_custom_led") ? `        Led* user_led = CreateUserCustomLedDriver();
        if (user_led != nullptr) return user_led;\n` : ''}${hasLed ? `        if (BUILTIN_LED_GPIO != GPIO_NUM_NC) {
            static SingleLed led(BUILTIN_LED_GPIO);
            return &led;
        }` : ''}
        return nullptr;
    }

    virtual AudioCodec* GetAudioCodec() override {
${(s.audio && (s.audio.spk_driver === "user_custom" || s.audio.mic_driver === "user_custom")) ? `        AudioCodec* user_codec = CreateUserCustomAudioCodecDriver();
        if (user_codec != nullptr) {
            return user_codec;
        }\n` : ''}${isSimplex ? `        static NoAudioCodecSimplex audio_codec(
            AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT,
            AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_WS, AUDIO_I2S_MIC_GPIO_DIN
        );
        return &audio_codec;` : isEs8311 ? `        if (i2c_bus_ == nullptr) {
            InitializeI2cBus();
        }
        static Es8311AudioCodec audio_codec(
            i2c_bus_, I2C_NUM_0,
            AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            GPIO_NUM_NC, AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS,
            AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN,
            AUDIO_PA_PIN, 0x18, false
        );
        return &audio_codec;` : `        static NoAudioCodecDuplex audio_codec(
            AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN
        );
        return &audio_codec;`}
    }

    virtual Display* GetDisplay() override {
        return display_;
    }

    virtual Backlight* GetBacklight() override {
${hasBacklight ? `        if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
            static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
            return &backlight;
        }` : ''}
        return nullptr;
    }
};

DECLARE_BOARD(CustomS3N16R8Board);
`;
  }
}

// ============================================================================
// MODULE C: PIN VALIDATION & CONFLICT ENGINE (ESP32-S3 RULES)
// ============================================================================
class PinValidator {
  static validate(s) {
    const errors = [];
    const pinUsage = new Map();

    function registerPin(pin, fieldName, fieldId, busType = null) {
      if (pin === undefined || pin === null || pin === -1 || isNaN(pin)) return;

      if (pin < ESP32S3_RULES.MIN_GPIO || pin > ESP32S3_RULES.MAX_GPIO) {
        errors.push({
          pin,
          fieldId,
          message: `${fieldName}: GPIO ${pin} nằm ngoài dải hợp lệ (0-48) của ESP32-S3!`
        });
        return;
      }

      if (ESP32S3_RULES.LOCKED_OCTAL_PINS.includes(pin)) {
        errors.push({
          pin,
          fieldId,
          message: `${fieldName}: GPIO ${pin} BỊ KHÓA! Dành riêng cho Octal SPI Flash & PSRAM trên ESP32-S3-N16R8.`
        });
        return;
      }

      if (!pinUsage.has(pin)) {
        pinUsage.set(pin, []);
      }
      pinUsage.get(pin).push({ fieldName, fieldId, busType });
    }

    // 1. Audio Pins
    registerPin(s.audio.spk_bclk, "Speaker BCLK", "audio_spk_bclk");
    registerPin(s.audio.spk_ws, "Speaker WS", "audio_spk_ws");
    registerPin(s.audio.spk_dout, "Speaker DOUT", "audio_spk_dout");

    if (s.audio.mode === "simplex") {
      registerPin(s.audio.mic_sck, "Mic SCK", "audio_mic_sck");
      registerPin(s.audio.mic_ws, "Mic WS", "audio_mic_ws");
      registerPin(s.audio.mic_din, "Mic DIN", "audio_mic_din");
    } else if (s.audio.mode === "es8311") {
      registerPin(s.audio.mic_din, "Mic DIN", "audio_mic_din");
      registerPin(s.audio.codec_sda, "Codec I2C SDA", "audio_codec_sda", "i2c_sda");
      registerPin(s.audio.codec_scl, "Codec I2C SCL", "audio_codec_scl", "i2c_scl");
      if (s.audio.mclk >= 0) registerPin(s.audio.mclk, "Audio MCLK", "audio_mclk");
      if (s.audio.pa_pin >= 0) registerPin(s.audio.pa_pin, "Audio PA Pin", "audio_pa_pin");
    } else {
      registerPin(s.audio.mic_din, "Mic DIN", "audio_mic_din");
      if (s.audio.pa_pin >= 0) registerPin(s.audio.pa_pin, "Audio PA Pin", "audio_pa_pin");
    }

    // 2. Display & Touch Pins
    if (s.display.type === "st7789" || s.display.type === "gc9a01" || s.display.type === "ili9341") {
      registerPin(s.display.mosi, "Display MOSI", "display_mosi");
      registerPin(s.display.clk, "Display SCK", "display_clk");
      registerPin(s.display.cs, "Display CS", "display_cs");
      registerPin(s.display.dc, "Display DC", "display_dc");
      if (s.display.rst >= 0) registerPin(s.display.rst, "Display RST", "display_rst");
      if (s.display.blk >= 0) registerPin(s.display.blk, "Display Backlight", "display_blk");
    } else if (s.display.type === "ssd1306") {
      registerPin(s.display.i2c_sda, "OLED I2C SDA", "display_i2c_sda", "i2c_sda");
      registerPin(s.display.i2c_scl, "OLED I2C SCL", "display_i2c_scl", "i2c_scl");
      if (s.display.rst >= 0) registerPin(s.display.rst, "Display RST", "display_rst");
    } else if (s.display.type === "uart_display") {
      registerPin(s.display.uart_tx, "UART Display TX", "display_uart_tx");
      registerPin(s.display.uart_rx, "UART Display RX", "display_uart_rx");
    }

    // Touch Controller Pins
    if (s.display.touch_chip && s.display.touch_chip !== "none") {
      registerPin(s.display.touch_sda, "Touch I2C SDA", "touch_sda", "i2c_sda");
      registerPin(s.display.touch_scl, "Touch I2C SCL", "touch_scl", "i2c_scl");
      if (s.display.touch_int !== undefined && s.display.touch_int >= 0) {
        registerPin(s.display.touch_int, "Touch INT", "touch_int");
      }
      if (s.display.touch_rst !== undefined && s.display.touch_rst >= 0) {
        registerPin(s.display.touch_rst, "Touch RST", "touch_rst");
      }
    }

    // 3. Input Drivers
    s.buttons.forEach(b => {
      if (b.type === "rotary") {
        registerPin(b.pin_a, `${b.name} Phase A`, `btn_${b.id}_a`);
        registerPin(b.pin_b, `${b.name} Phase B`, `btn_${b.id}_b`);
        registerPin(b.pin_key, `${b.name} Key`, `btn_${b.id}_key`);
      } else {
        registerPin(b.pin, b.name, `btn_${b.id}`);
      }
    });

    // 4. Actuators & Sensors Drivers (Physical Hardware Layer)
    s.peripherals.forEach(p => {
      if (p.type === "sensor_hcsr04") {
        registerPin(p.trig_pin, `${p.name} Trig`, `p_${p.id}_trig`);
        registerPin(p.echo_pin, `${p.name} Echo`, `p_${p.id}_echo`);
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
      } else if (p.type === "sensor_vl6180x") {
        registerPin(p.sda, `${p.name} SDA`, `p_${p.id}_sda`, "i2c_sda");
        registerPin(p.scl, `${p.name} SCL`, `p_${p.id}_scl`, "i2c_scl");
      } else if (p.type === "motor_dc") {
        registerPin(p.pwma, `${p.name} PWMA`, `p_${p.id}_pwma`);
        registerPin(p.dira, `${p.name} DIRA`, `p_${p.id}_dira`);
        registerPin(p.pwmb, `${p.name} PWMB`, `p_${p.id}_pwmb`);
        registerPin(p.dirb, `${p.name} DIRB`, `p_${p.id}_dirb`);
      } else if (p.type === "battery_adc" || p.type === "sensor_gas" || p.type === "sensor_ldr") {
        // Dedicated ADC channel
      } else {
        registerPin(p.pin, p.name, `p_${p.id}`);
      }
    });

    // 5. Storage & Communication Drivers
    s.communication.forEach(c => {
      if (c.type === "uart") {
        registerPin(c.tx, `${c.name} TX`, `comm_${c.id}_tx`);
        registerPin(c.rx, `${c.name} RX`, `comm_${c.id}_rx`);
      } else if (c.type === "i2c") {
        registerPin(c.sda, `${c.name} SDA`, `comm_${c.id}_sda`, "i2c_sda");
        registerPin(c.scl, `${c.name} SCL`, `comm_${c.id}_scl`, "i2c_scl");
      } else if (c.type === "spi") {
        registerPin(c.mosi, `${c.name} MOSI`, `comm_${c.id}_mosi`);
        registerPin(c.miso, `${c.name} MISO`, `comm_${c.id}_miso`);
        registerPin(c.sclk, `${c.name} SCLK`, `comm_${c.id}_sclk`);
            } else if (c.type === "comm_twai") {
        cfg.push(`#define CUSTOM_PERIPH_TWAI_TX_PIN             ${gpioMacro(c.tx)}`);
        cfg.push(`#define CUSTOM_PERIPH_TWAI_RX_PIN             ${gpioMacro(c.rx)}`);
      } else if (c.type === "comm_4g") {
        cfg.push(`#define CUSTOM_PERIPH_4G_UART_TX_PIN          ${gpioMacro(c.tx)}`);
        cfg.push(`#define CUSTOM_PERIPH_4G_UART_RX_PIN          ${gpioMacro(c.rx)}`);
        cfg.push(`#define CUSTOM_PERIPH_4G_PWRKEY_PIN           ${gpioMacro(c.pwr)}`);
            } else if (c.type === "comm_twai") {
        registerPin(c.tx, `${c.name} TX`, `comm_${c.id}_tx`);
        registerPin(c.rx, `${c.name} RX`, `comm_${c.id}_rx`);
      } else if (c.type === "comm_4g") {
        registerPin(c.tx, `${c.name} TX`, `comm_${c.id}_tx`);
        registerPin(c.rx, `${c.name} RX`, `comm_${c.id}_rx`);
        registerPin(c.pwr, `${c.name} PWR`, `comm_${c.id}_pwr`);
      } else if (c.type === "sdcard") {
        registerPin(c.cs, `${c.name} CS`, `comm_${c.id}_cs`);
        registerPin(c.mosi, `${c.name} MOSI`, `comm_${c.id}_mosi`);
        registerPin(c.miso, `${c.name} MISO`, `comm_${c.id}_miso`);
        registerPin(c.sclk, `${c.name} SCK`, `comm_${c.id}_sclk`);
      }
    });

    // Conflict detection
    for (const [pin, users] of pinUsage.entries()) {
      if (users.length > 1) {
        // Cho phép chia sẻ bus I2C chuẩn (tất cả là i2c_sda hoặc tất cả là i2c_scl)
        const isSharedI2cBus = (users.every(u => u.busType === "i2c_sda") || users.every(u => u.busType === "i2c_scl"));
        if (isSharedI2cBus) {
          continue;
        }

        const names = users.map(u => u.fieldName).join(" và ");
        users.forEach(u => {
          errors.push({
            pin,
            fieldId: u.fieldId,
            message: `Xung đột GPIO ${pin}: đang được dùng đồng thời bởi ${names}!`
          });
        });
      }
    }

    return {
      errors,
      pinUsage
    };
  }
}

// ============================================================================
// MODULE D & E: GUI CONTROLLER & AUTO-INJECTION ENGINE
// ============================================================================
class GUIController {
  static tabSnapshots = {};
  static tabDirtyState = {};
  static pendingTargetTabId = null;
  static isServerMode = false;
  static serverProjectInfo = null;

  static TAB_NAMES = {
    "panel-general": "Cơ bản & Ngôn ngữ",
    "panel-audio": "Âm thanh (Audio & Mic)",
    "panel-display": "Màn hình & Cảm ứng",
    "panel-wakeword": "Từ khóa & Giọng nói",
    "panel-network": "Cấp mạng Wi-Fi",
    "panel-mcp": "Điều khiển MCP & AI",
    "panel-buttons": "Phím bấm & Điều khiển",
    "panel-peripherals": "Ngoại vi & Cảm biến",
    "panel-communication": "Giao tiếp & Thẻ nhớ",
    "panel-flash": "Flash & ESP32-S3"
  };

  static async init() {
    GUIController.bindEvents();
    GUIController.renderPinMatrix();
    GUIController.syncStateToUI();
    GUIController.renderAllLists();
    GUIController.updateValidation();
    GUIController.initTabSnapshots();

    // Tự động kiểm tra và nhận diện thư mục dự án qua local server
    await GUIController.autoConnectAndLoadConfig();
  }

  static async autoConnectAndLoadConfig() {
    if (typeof fetch === "undefined") return;
    try {
      const res = await fetch("/api/project", { cache: "no-store" });
      if (res.ok) {
        const info = await res.json();
        if (info && info.success) {
          GUIController.isServerMode = true;
          GUIController.serverProjectInfo = info;

          const dot = document.getElementById("fs-status-dot");
          const text = document.getElementById("fs-status-text");
          if (dot) {
            dot.classList.remove("error");
            dot.classList.add("connected");
          }
          if (text) {
            text.textContent = `Dự án: ${info.root_path}`;
            text.title = `Thư mục dự án: ${info.root_path}`;
          }

          // Tự động nạp cấu hình hiện hành từ dự án
          try {
            const cfgRes = await fetch("/api/config", { cache: "no-store" });
            if (cfgRes.ok) {
              const cfgData = await cfgRes.json();
              if (cfgData && cfgData.content) {
                const parsed = ConfigParser.parseConfigH(cfgData.content);
                configState = parsed;
                GUIController.syncStateToUI();
                GUIController.renderAllLists();
                GUIController.initTabSnapshots();
                GUIController.updateValidation();
                GUIController.showToast(`Đã tự động nhận diện dự án tại "${info.root_path}" và nạp cấu hình!`, "success");
              } else if (cfgData && cfgData.source !== "default") {
                GUIController.initTabSnapshots();
                GUIController.showToast(`Đã kết nối dự án tại "${info.root_path}"!`, "success");
              }
            }
          } catch (cfgErr) {
            console.warn("Không thể tải /api/config:", cfgErr);
          }
        }
      }
    } catch (e) {
      console.log("Môi trường Standalone (chưa chạy qua Configurator Server):", e);
    }
  }

  static getPanelSnapshot(panelId) {
    const panel = document.getElementById(panelId);
    if (!panel) return "";
    const formElements = panel.querySelectorAll("input, select, textarea");
    const data = [];
    formElements.forEach(el => {
      if (el.type === "checkbox") {
        data.push(`${el.id || el.name}:${el.checked}`);
      } else if (el.type === "radio") {
        if (el.checked) data.push(`${el.name}:${el.value}`);
      } else {
        data.push(`${el.id || el.name}:${el.value}`);
      }
    });

    if (panelId === "panel-buttons") {
      data.push("buttons:" + JSON.stringify(configState.buttons || []));
    } else if (panelId === "panel-peripherals") {
      data.push("peripherals:" + JSON.stringify(configState.peripherals || []));
    } else if (panelId === "panel-communication") {
      data.push("communication:" + JSON.stringify(configState.communication || []));
    }

    return data.join("|");
  }

  static initTabSnapshots() {
    document.querySelectorAll(".tab-panel").forEach(panel => {
      GUIController.tabSnapshots[panel.id] = GUIController.getPanelSnapshot(panel.id);
      GUIController.tabDirtyState[panel.id] = false;
      GUIController.updateTabStatusUI(panel.id);
    });
  }

  static checkPanelDirty(panelId) {
    if (!panelId) return;
    const current = GUIController.getPanelSnapshot(panelId);
    const isDirty = current !== (GUIController.tabSnapshots[panelId] || "");
    GUIController.tabDirtyState[panelId] = isDirty;
    GUIController.updateTabStatusUI(panelId);
    GUIController.syncUIToState();
    GUIController.updateValidation();
  }

  static updateTabStatusUI(panelId) {
    const isDirty = !!GUIController.tabDirtyState[panelId];

    // 1. Sidebar indicator
    const navBtn = document.querySelector(`.nav-link[data-target="${panelId}"]`);
    if (navBtn) {
      if (isDirty) {
        navBtn.classList.add("has-unsaved");
      } else {
        navBtn.classList.remove("has-unsaved");
      }
    }

    // 2. Tab header status pill
    const pill = document.querySelector(`.tab-status-pill[data-tab-status="${panelId}"]`);
    if (pill) {
      if (isDirty) {
        pill.className = "tab-status-pill unsaved";
        pill.textContent = "Chưa lưu";
      } else {
        pill.className = "tab-status-pill saved";
        pill.textContent = "Đã lưu";
      }
    }

    // 3. Tab save button
    const saveBtn = document.querySelector(`.btn-save-tab[data-tab="${panelId}"]`);
    if (saveBtn) {
      if (isDirty) {
        saveBtn.classList.add("btn-highlight-save");
      } else {
        saveBtn.classList.remove("btn-highlight-save");
      }
    }
  }

  static switchTab(targetId) {
    const tabButtons = document.querySelectorAll(".nav-link[data-target]");
    tabButtons.forEach(b => {
      if (b.getAttribute("data-target") === targetId) {
        b.classList.add("active");
      } else {
        b.classList.remove("active");
      }
    });

    document.querySelectorAll(".tab-panel").forEach(panel => {
      if (panel.id === targetId) {
        panel.classList.add("active");
      } else {
        panel.classList.remove("active");
      }
    });
  }

  static openUnsavedModal(currentPanelId, targetId) {
    const modal = document.getElementById("unsaved-modal");
    const desc = document.getElementById("unsaved-modal-desc");
    if (!modal) return;

    const tabTitle = GUIController.TAB_NAMES[currentPanelId] || currentPanelId;
    if (desc) {
      desc.innerHTML = `Bạn đang có một số thay đổi trong mục <strong>"${tabTitle}"</strong> chưa được lưu lại. Bạn có muốn lưu thay đổi này trước khi chuyển sang mục khác không?`;
    }

    modal.classList.add("active");
  }

  static closeUnsavedModal() {
    const modal = document.getElementById("unsaved-modal");
    if (modal) modal.classList.remove("active");
    GUIController.pendingTargetTabId = null;
  }

  static discardTabChanges(panelId) {
    GUIController.syncStateToUI();
    GUIController.renderAllLists();
    GUIController.tabDirtyState[panelId] = false;
    GUIController.tabSnapshots[panelId] = GUIController.getPanelSnapshot(panelId);
    GUIController.updateTabStatusUI(panelId);
    GUIController.updateValidation();
  }

  static showToast(message, type = "success") {
    const container = document.getElementById("toast-container");
    if (!container) return;

    const toast = document.createElement("div");
    toast.className = `toast-item toast-${type}`;
    
    let icon = "✓";
    if (type === "error") icon = "✕";
    if (type === "warning") icon = "⚠️";

    toast.innerHTML = `
      <span style="font-weight: 700;">${icon}</span>
      <span>${message}</span>
    `;

    container.appendChild(toast);

    setTimeout(() => {
      toast.style.opacity = "0";
      toast.style.transform = "translateY(10px)";
      setTimeout(() => toast.remove(), 300);
    }, 3500);
  }

  static async writeConfigFilesToDisk() {
    const configH = CodeGenerator.generateConfigH(configState);
    const configJson = CodeGenerator.generateConfigJson(configState);
    const boardCC = CodeGenerator.generateBoardCC(configState);

    // 1. Ưu tiên ghi trực tiếp qua Configurator Server cục bộ
    if (GUIController.isServerMode) {
      const resp = await fetch("/api/save", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          config_h: configH,
          config_json: configJson,
          board_cc: boardCC
        })
      });
      if (!resp.ok) {
        const errJson = await resp.json().catch(() => ({}));
        throw new Error(errJson.error || "Lỗi khi lưu qua máy chủ cục bộ");
      }
      return true;
    }

    // 2. Ghi qua Web File System Access API nếu mở trực tiếp bằng browser
    if (!directoryHandle) return false;
    await FileSystemEngine.writeFile(directoryHandle, "main/boards/custom-s3-n16r8/config.h", configH);
    await FileSystemEngine.writeFile(directoryHandle, "main/boards/custom-s3-n16r8/config.json", configJson);
    await FileSystemEngine.writeFile(directoryHandle, "main/boards/custom-s3-n16r8/custom_s3_n16r8_board.cc", boardCC);

    try {
      let kconfig = await FileSystemEngine.readFile(directoryHandle, "main/Kconfig.projbuild");
      if (!kconfig.includes("BOARD_TYPE_CUSTOM_S3_N16R8")) {
        const entry = `\n    config BOARD_TYPE_CUSTOM_S3_N16R8\n        bool "Custom ESP32-S3-N16R8 Board"\n        depends on IDF_TARGET_ESP32S3\n`;
        const boardChoiceIdx = kconfig.indexOf("choice BOARD_TYPE");
        if (boardChoiceIdx !== -1) {
          const endchoiceIdx = kconfig.indexOf("endchoice", boardChoiceIdx);
          if (endchoiceIdx !== -1) {
            kconfig = kconfig.slice(0, endchoiceIdx) + entry + kconfig.slice(endchoiceIdx);
            await FileSystemEngine.writeFile(directoryHandle, "main/Kconfig.projbuild", kconfig);
          }
        }
      }
    } catch (kErr) {
      console.warn("Không thể tự động cập nhật Kconfig.projbuild:", kErr);
    }

    try {
      let cmake = await FileSystemEngine.readFile(directoryHandle, "main/CMakeLists.txt");
      if (!cmake.includes("CONFIG_BOARD_TYPE_CUSTOM_S3_N16R8")) {
        const cmakeSnippet = `\nif(CONFIG_BOARD_TYPE_CUSTOM_S3_N16R8)\n    set(BOARD_DIR "custom-s3-n16r8")\n    set(BUILTIN_TEXT_FONT font_noto_sans_basic_20_4)\n    set(BUILTIN_ICON_FONT font_material_symbols_20_4)\n    set(DEFAULT_EMOJI_COLLECTION noto-color-emoji_64)\n    set(EMOTE_RESOLUTION "320_240")\nelse`;
        if (cmake.includes("if(CONFIG_BOARD_TYPE_")) {
          cmake = cmake.replace("if(CONFIG_BOARD_TYPE_", cmakeSnippet + "if(CONFIG_BOARD_TYPE_");
          await FileSystemEngine.writeFile(directoryHandle, "main/CMakeLists.txt", cmake);
        }
      }
    } catch (cErr) {
      console.warn("Không thể tự động cập nhật main/CMakeLists.txt:", cErr);
    }
    return true;
  }

  static async saveTab(panelId) {
    GUIController.syncUIToState();

    const result = PinValidator.validate(configState);
    if (result.errors.length > 0) {
      GUIController.showToast("Cảnh báo: Đang có xung đột chân GPIO! Vui lòng kiểm tra lại bảng cảnh báo.", "error");
      GUIController.updateValidation();
      return false;
    }

    GUIController.tabSnapshots[panelId] = GUIController.getPanelSnapshot(panelId);
    GUIController.tabDirtyState[panelId] = false;
    GUIController.updateTabStatusUI(panelId);

    const tabTitle = GUIController.TAB_NAMES[panelId] || panelId;

    if (GUIController.isServerMode || directoryHandle) {
      try {
        await GUIController.writeConfigFilesToDisk();
        GUIController.showToast(`Đã lưu và đồng bộ "${tabTitle}" vào dự án!`, "success");
      } catch (e) {
        GUIController.showToast(`Lỗi khi ghi tệp dự án: ${e.message}`, "error");
        return false;
      }
    } else {
      GUIController.showToast(`Đã lưu cấu hình "${tabTitle}"! (Chạy run_configurator.bat để tự động ghi vào mã nguồn)`, "success");
    }

    const saveBtn = document.querySelector(`.btn-save-tab[data-tab="${panelId}"]`);
    if (saveBtn) {
      const origHtml = saveBtn.innerHTML;
      saveBtn.classList.add("btn-saved-success");
      saveBtn.innerHTML = `
        <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><polyline points="20 6 9 17 4 12"></polyline></svg>
        <span>Đã lưu ✓</span>
      `;
      setTimeout(() => {
        saveBtn.innerHTML = origHtml;
        saveBtn.classList.remove("btn-saved-success");
      }, 1500);
    }

    return true;
  }

  static bindEvents() {
    // 1. Tab switching with unsaved changes verification
    const tabButtons = document.querySelectorAll(".nav-link[data-target]");
    tabButtons.forEach(btn => {
      btn.addEventListener("click", (e) => {
        const targetId = btn.getAttribute("data-target");
        const activePanel = document.querySelector(".tab-panel.active");
        const currentPanelId = activePanel ? activePanel.id : null;

        if (currentPanelId === targetId) return;

        // CHỈ NHẮC NGƯỜI DÙNG KHI CÓ SỰ THAY ĐỔI MÀ CHƯA ĐƯỢC LƯU
        if (currentPanelId && GUIController.tabDirtyState[currentPanelId]) {
          e.preventDefault();
          e.stopPropagation();
          GUIController.pendingTargetTabId = targetId;
          GUIController.openUnsavedModal(currentPanelId, targetId);
          return;
        }

        // NẾU KHÔNG CÓ THAY ĐỔI CHƯA LƯU: CHUYỂN TAB TRỰC TIẾP KHÔNG HỎI
        GUIController.switchTab(targetId);
      });
    });

    // 2. Unsaved Changes Modal Actions
    const btnStay = document.getElementById("unsaved-btn-stay");
    const btnClose = document.getElementById("unsaved-btn-close");
    const btnDiscard = document.getElementById("unsaved-btn-discard");
    const btnSaveUnsaved = document.getElementById("unsaved-btn-save");

    if (btnStay) btnStay.addEventListener("click", () => GUIController.closeUnsavedModal());
    if (btnClose) btnClose.addEventListener("click", () => GUIController.closeUnsavedModal());

    if (btnDiscard) {
      btnDiscard.addEventListener("click", () => {
        const activePanel = document.querySelector(".tab-panel.active");
        const currentPanelId = activePanel ? activePanel.id : null;
        if (currentPanelId) {
          GUIController.discardTabChanges(currentPanelId);
        }
        const targetId = GUIController.pendingTargetTabId;
        GUIController.closeUnsavedModal();
        if (targetId) GUIController.switchTab(targetId);
      });
    }

    if (btnSaveUnsaved) {
      btnSaveUnsaved.addEventListener("click", async () => {
        const activePanel = document.querySelector(".tab-panel.active");
        const currentPanelId = activePanel ? activePanel.id : null;
        if (currentPanelId) {
          const ok = await GUIController.saveTab(currentPanelId);
          if (!ok) return;
        }
        const targetId = GUIController.pendingTargetTabId;
        GUIController.closeUnsavedModal();
        if (targetId) GUIController.switchTab(targetId);
      });
    }

    // 3. Per-Tab Save Buttons
    document.querySelectorAll(".btn-save-tab").forEach(btn => {
      btn.addEventListener("click", async () => {
        const tabId = btn.getAttribute("data-tab");
        if (tabId) await GUIController.saveTab(tabId);
      });
    });

    // 4. Panel Input & Change Listeners (Dirty Detection)
    document.querySelectorAll(".tab-panel").forEach(panel => {
      panel.addEventListener("input", () => {
        GUIController.checkPanelDirty(panel.id);
      });
      panel.addEventListener("change", () => {
        GUIController.checkPanelDirty(panel.id);
      });
    });

    // 5. Browser Unload Guard
    window.addEventListener("beforeunload", (e) => {
      const isAnyDirty = Object.values(GUIController.tabDirtyState).some(d => !!d);
      if (isAnyDirty) {
        e.preventDefault();
        e.returnValue = "";
      }
    });

    // Open Directory
    document.getElementById("btn-open-dir").addEventListener("click", async () => {
      const handle = await FileSystemEngine.openProjectDirectory();
      if (handle) {
        directoryHandle = handle;
        document.getElementById("fs-status-dot").classList.remove("error");
        document.getElementById("fs-status-dot").classList.add("connected");
        document.getElementById("fs-status-text").textContent = `Dự án: ${handle.name}`;

        let loadedConfig = false;
        try {
          const configContent = await FileSystemEngine.readFile(handle, "main/boards/custom-s3-n16r8/config.h");
          const parsed = ConfigParser.parseConfigH(configContent);
          configState = parsed;
          loadedConfig = true;
          GUIController.showToast("Đã tự động nạp cấu hình từ 'main/boards/custom-s3-n16r8/config.h'!", "success");
        } catch (e1) {
          try {
            const sampleContent = await FileSystemEngine.readFile(handle, "main/boards/esp32s3-n16r8-custom/config.h");
            const parsed = ConfigParser.parseConfigH(sampleContent);
            configState = parsed;
            loadedConfig = true;
            GUIController.showToast("Đã tự động nạp cấu hình mẫu từ 'esp32s3-n16r8-custom/config.h'!", "success");
          } catch (e2) {
            console.log("Chưa có cấu hình cũ, sử dụng cấu hình mặc định.");
          }
        }

        GUIController.syncStateToUI();
        GUIController.renderAllLists();
        GUIController.initTabSnapshots();
        GUIController.updateValidation();
      }
    });

    // Audio Mode switch
    document.getElementById("audio_mode").addEventListener("change", (e) => {
      const mode = e.target.value;
      configState.audio.mode = mode;

      const micGroup = document.querySelectorAll(".mic-pin-group");
      const codecGroup = document.querySelectorAll(".codec-pin-group");

      micGroup.forEach(el => el.style.display = (mode === "simplex" ? "flex" : "none"));
      codecGroup.forEach(el => el.style.display = (mode === "es8311" ? "flex" : "none"));

      GUIController.updateValidation();
    });

    // Audio Speaker Driver switch
    const spkDriverSelect = document.getElementById("audio_spk_driver");
    if (spkDriverSelect) {
      spkDriverSelect.addEventListener("change", (e) => {
        configState.audio.spk_driver = e.target.value;
        if (e.target.value === "es8311" || e.target.value === "es8388") {
          document.querySelectorAll(".codec-pin-group").forEach(el => el.style.display = "flex");
        }
      });
    }

    // Audio Mic Driver switch
    const micDriverSelect = document.getElementById("audio_mic_driver");
    if (micDriverSelect) {
      micDriverSelect.addEventListener("change", (e) => {
        configState.audio.mic_driver = e.target.value;
        if (e.target.value === "es8311") {
          document.querySelectorAll(".codec-pin-group").forEach(el => el.style.display = "flex");
        }
      });
    }

    // General Flash Assets switch
    const flashAssetsSelect = document.getElementById("general_flash_assets");
    if (flashAssetsSelect) {
      flashAssetsSelect.addEventListener("change", (e) => {
        configState.general.flash_assets = e.target.value;
        const customGroup = document.getElementById("custom-assets-group");
        if (customGroup) customGroup.style.display = (e.target.value === "FLASH_CUSTOM_ASSETS") ? "flex" : "none";
      });
    }

    // Display Type switch
    document.getElementById("display_type").addEventListener("change", (e) => {
      const type = e.target.value;
      configState.display.type = type;

      const isSpi = type === "st7789" || type === "gc9a01" || type === "ili9341";
      const isOled = type === "ssd1306";
      const isUart = type === "uart_display";

      document.querySelectorAll(".display-spi-group").forEach(el => el.style.display = isSpi ? "flex" : "none");
      document.querySelectorAll(".display-i2c-group").forEach(el => el.style.display = isOled ? "flex" : "none");
      document.querySelectorAll(".display-uart-group").forEach(el => el.style.display = isUart ? "flex" : "none");
      document.querySelectorAll(".display-common-group").forEach(el => el.style.display = (type !== "none" && !isUart) ? "flex" : "none");

      if (type === "ssd1306") {
        document.getElementById("display_width").value = 128;
        document.getElementById("display_height").value = 64;
        configState.display.width = 128;
        configState.display.height = 64;
      } else if (type === "gc9a01") {
        document.getElementById("display_width").value = 240;
        document.getElementById("display_height").value = 240;
        configState.display.width = 240;
        configState.display.height = 240;
      } else if (type === "ili9341") {
        document.getElementById("display_width").value = 240;
        document.getElementById("display_height").value = 320;
        configState.display.width = 240;
        configState.display.height = 320;
      }

      GUIController.updateValidation();
    });

    // Touch Chip switch
    const touchSelect = document.getElementById("touch_chip");
    if (touchSelect) {
      touchSelect.addEventListener("change", (e) => {
        const hasTouch = e.target.value !== "none";
        configState.display.touch_chip = e.target.value;
        document.querySelectorAll(".touch-fields-group").forEach(el => el.style.display = hasTouch ? "flex" : "none");
        GUIController.updateValidation();
      });
    }

    // Link Flash Size to Partition Table
    const flashSelect = document.getElementById("flash_size");
    const partitionSelect = document.getElementById("partition_table");
    if (flashSelect && partitionSelect) {
      flashSelect.addEventListener("change", (e) => {
        const size = e.target.value;
        if (size === "8MB") partitionSelect.value = "partitions/v2/8m.csv";
        else if (size === "16MB") partitionSelect.value = "partitions/v2/16m.csv";
        else if (size === "32MB") partitionSelect.value = "partitions/v2/32m.csv";
        GUIController.syncUIToState();
      });
    }

    // Static Form Inputs Listeners
    const staticInputs = document.querySelectorAll("input, select");
    staticInputs.forEach(input => {
      input.addEventListener("input", () => {
        GUIController.syncUIToState();
        GUIController.updateValidation();
      });
      input.addEventListener("change", () => {
        GUIController.syncUIToState();
        GUIController.updateValidation();
      });
    });

    // Add Item Buttons
    const btnAddBtn = document.getElementById("btn-add-button");
    if (btnAddBtn) btnAddBtn.addEventListener("click", () => GUIController.openAddItemModal("buttons"));

    const btnAddPeriph = document.getElementById("btn-add-peripheral");
    if (btnAddPeriph) btnAddPeriph.addEventListener("click", () => GUIController.openAddItemModal("peripherals"));

    const btnAddComm = document.getElementById("btn-add-comm");
    if (btnAddComm) btnAddComm.addEventListener("click", () => GUIController.openAddItemModal("communication"));

    // Modal Events
    const btnCloseItemModal = document.getElementById("modal-btn-close") || document.getElementById("btn-close-item-modal");
    if (btnCloseItemModal) btnCloseItemModal.addEventListener("click", () => {
      const m = document.getElementById("add-item-modal") || document.getElementById("modal-add-item");
      if (m) m.classList.remove("active");
    });

    const btnCancelItemModal = document.getElementById("modal-btn-cancel") || document.getElementById("btn-cancel-item-modal");
    if (btnCancelItemModal) btnCancelItemModal.addEventListener("click", () => {
      const m = document.getElementById("add-item-modal") || document.getElementById("modal-add-item");
      if (m) m.classList.remove("active");
    });

    const modalTypeSelect = document.getElementById("modal-item-type") || document.getElementById("modal-item-type-select");
    if (modalTypeSelect) modalTypeSelect.addEventListener("change", (e) => {
      GUIController.renderModalFields(currentModalCategory, e.target.value);
    });

    const btnConfirmItemModal = document.getElementById("modal-btn-confirm") || document.getElementById("btn-confirm-item-modal");
    if (btnConfirmItemModal) btnConfirmItemModal.addEventListener("click", () => {
      GUIController.saveItemFromModal();
    });

    // Success Modal Close
    const btnCloseModal = document.getElementById("btn-close-modal");
    if (btnCloseModal) btnCloseModal.addEventListener("click", () => {
      const sm = document.getElementById("success-modal");
      if (sm) sm.classList.remove("active");
    });

    const btnModalOk = document.getElementById("btn-modal-ok");
    if (btnModalOk) btnModalOk.addEventListener("click", () => {
      const sm = document.getElementById("success-modal");
      if (sm) sm.classList.remove("active");
    });

    const btnCopyCmd = document.getElementById("btn-copy-cmd");
    if (btnCopyCmd) btnCopyCmd.addEventListener("click", () => {
      const cmdEl = document.getElementById("build-command");
      const text = cmdEl ? cmdEl.textContent : "";
      navigator.clipboard.writeText(text);
      btnCopyCmd.textContent = "Copied!";
      setTimeout(() => {
        btnCopyCmd.textContent = "Copy";
      }, 2000);
    });
  }

  // Render Pin Matrix (GPIO 0 - 48)
  static renderPinMatrix() {
    const grid = document.getElementById("pin-matrix-grid");
    if (!grid) return;
    grid.innerHTML = "";

    for (let pin = ESP32S3_RULES.MIN_GPIO; pin <= ESP32S3_RULES.MAX_GPIO; pin++) {
      const cell = document.createElement("div");
      cell.className = "pin-cell";
      cell.id = `pin-cell-${pin}`;
      cell.textContent = pin;

      if (ESP32S3_RULES.LOCKED_OCTAL_PINS.includes(pin)) {
        cell.classList.add("locked");
        cell.title = `GPIO ${pin}: Dành riêng cho Octal SPI Flash & PSRAM (KHÓA)`;
      } else {
        cell.title = `GPIO ${pin}: Còn trống`;
      }

      grid.appendChild(cell);
    }
  }

  // Sync state to static fields
  static syncStateToUI() {
    // 1. General & Language
    if (document.getElementById("general_ota_url")) document.getElementById("general_ota_url").value = configState.general?.ota_url || "https://api.tenclass.net/xiaozhi/ota/";
    if (document.getElementById("general_language")) document.getElementById("general_language").value = configState.general?.language || "LANGUAGE_VI_VN";
    if (document.getElementById("general_flash_assets")) document.getElementById("general_flash_assets").value = configState.general?.flash_assets || "FLASH_DEFAULT_ASSETS";
    if (document.getElementById("general_custom_assets_file")) document.getElementById("general_custom_assets_file").value = configState.general?.custom_assets_file || "assets.bin";
    if (document.getElementById("custom-assets-group")) {
      document.getElementById("custom-assets-group").style.display = (configState.general?.flash_assets === "FLASH_CUSTOM_ASSETS") ? "flex" : "none";
    }

    // 2. Audio
    if (document.getElementById("audio_mode")) document.getElementById("audio_mode").value = configState.audio.mode;
    if (document.getElementById("audio_spk_driver")) document.getElementById("audio_spk_driver").value = configState.audio.spk_driver || "max98357a";
    if (document.getElementById("audio_mic_driver")) document.getElementById("audio_mic_driver").value = configState.audio.mic_driver || "inmp441";
    if (document.getElementById("audio_sample_rate")) document.getElementById("audio_sample_rate").value = configState.audio.sample_rate;
    if (document.getElementById("audio_spk_bclk")) document.getElementById("audio_spk_bclk").value = configState.audio.spk_bclk;
    if (document.getElementById("audio_spk_ws")) document.getElementById("audio_spk_ws").value = configState.audio.spk_ws;
    if (document.getElementById("audio_spk_dout")) document.getElementById("audio_spk_dout").value = configState.audio.spk_dout;
    if (document.getElementById("audio_mic_sck")) document.getElementById("audio_mic_sck").value = configState.audio.mic_sck;
    if (document.getElementById("audio_mic_ws")) document.getElementById("audio_mic_ws").value = configState.audio.mic_ws;
    if (document.getElementById("audio_mic_din")) document.getElementById("audio_mic_din").value = configState.audio.mic_din;
    if (document.getElementById("audio_codec_sda")) document.getElementById("audio_codec_sda").value = configState.audio.codec_sda;
    if (document.getElementById("audio_codec_scl")) document.getElementById("audio_codec_scl").value = configState.audio.codec_scl;
    if (document.getElementById("audio_mclk")) document.getElementById("audio_mclk").value = configState.audio.mclk;
    if (document.getElementById("audio_pa_pin")) document.getElementById("audio_pa_pin").value = configState.audio.pa_pin;

    // 3. Display & UI Hardware
    if (document.getElementById("display_type")) document.getElementById("display_type").value = configState.display.type;
    if (document.getElementById("ui_weather_city")) document.getElementById("ui_weather_city").value = configState.display.weather_city || "TP. Hồ Chí Minh";
    if (document.getElementById("ui_voice_wave")) document.getElementById("ui_voice_wave").checked = configState.display.voice_wave !== false;

    document.getElementById("display_mosi").value = configState.display.mosi;
    document.getElementById("display_clk").value = configState.display.clk;
    document.getElementById("display_cs").value = configState.display.cs;
    document.getElementById("display_dc").value = configState.display.dc;
    document.getElementById("display_rst").value = configState.display.rst;
    document.getElementById("display_blk").value = configState.display.blk;
    if (document.getElementById("display_i2c_sda")) document.getElementById("display_i2c_sda").value = configState.display.i2c_sda;
    if (document.getElementById("display_i2c_scl")) document.getElementById("display_i2c_scl").value = configState.display.i2c_scl;
    document.getElementById("display_width").value = configState.display.width;
    document.getElementById("display_height").value = configState.display.height;
    if (document.getElementById("display_offset_x")) document.getElementById("display_offset_x").value = configState.display.offset_x;
    if (document.getElementById("display_offset_y")) document.getElementById("display_offset_y").value = configState.display.offset_y;
    document.getElementById("display_invert").checked = !!configState.display.invert;
    if (document.getElementById("display_swap_xy")) document.getElementById("display_swap_xy").checked = !!configState.display.swap_xy;
    if (document.getElementById("display_mirror_x")) document.getElementById("display_mirror_x").checked = !!configState.display.mirror_x;
    if (document.getElementById("display_mirror_y")) document.getElementById("display_mirror_y").checked = !!configState.display.mirror_y;

    // Touch Controller
    if (document.getElementById("touch_chip")) document.getElementById("touch_chip").value = configState.display.touch_chip || "none";
    if (document.getElementById("touch_sda")) document.getElementById("touch_sda").value = configState.display.touch_sda ?? 8;
    if (document.getElementById("touch_scl")) document.getElementById("touch_scl").value = configState.display.touch_scl ?? 9;
    if (document.getElementById("touch_int")) document.getElementById("touch_int").value = configState.display.touch_int ?? 3;
    if (document.getElementById("touch_rst")) document.getElementById("touch_rst").value = configState.display.touch_rst ?? -1;

    // UART Display
    if (document.getElementById("display_uart_port")) document.getElementById("display_uart_port").value = configState.display.uart_port ?? 1;
    if (document.getElementById("display_uart_baud")) document.getElementById("display_uart_baud").value = configState.display.uart_baud ?? 115200;
    if (document.getElementById("display_uart_tx")) document.getElementById("display_uart_tx").value = configState.display.uart_tx ?? 17;
    if (document.getElementById("display_uart_rx")) document.getElementById("display_uart_rx").value = configState.display.uart_rx ?? 18;

    // 4. Wake Word & AFE
    if (document.getElementById("wakeword_type")) document.getElementById("wakeword_type").value = configState.wake_word?.type || "USE_AFE_WAKE_WORD";
    if (document.getElementById("wakeword_threshold")) document.getElementById("wakeword_threshold").value = configState.wake_word?.threshold || 20;
    if (document.getElementById("wakeword_pinyin")) document.getElementById("wakeword_pinyin").value = configState.wake_word?.pinyin || "xiao tu dou";
    if (document.getElementById("wakeword_display")) document.getElementById("wakeword_display").value = configState.wake_word?.display || "小土豆";
    if (document.getElementById("audio_device_aec")) document.getElementById("audio_device_aec").checked = !!configState.wake_word?.device_aec;
    if (document.getElementById("audio_server_aec")) document.getElementById("audio_server_aec").checked = !!configState.wake_word?.server_aec;
    if (document.getElementById("audio_debugger_server")) document.getElementById("audio_debugger_server").value = configState.wake_word?.debugger_server || "";

    // 5. Network
    if (document.getElementById("network_wifi_method")) document.getElementById("network_wifi_method").value = configState.network?.wifi_method || "USE_HOTSPOT_WIFI_PROVISIONING";
    if (document.getElementById("network_secondary")) document.getElementById("network_secondary").value = configState.network?.secondary || "none";

    // 6. MCP Server Tools (Software Protocol Layer - No hardware pin conflict)
    if (document.getElementById("mcp_server_enable")) document.getElementById("mcp_server_enable").checked = configState.mcp?.enable !== false;
    if (document.getElementById("mcp_tool_peripherals")) document.getElementById("mcp_tool_peripherals").checked = configState.mcp?.tool_peripherals !== false;
    if (document.getElementById("mcp_tool_info")) document.getElementById("mcp_tool_info").checked = configState.mcp?.tool_info !== false;
    if (document.getElementById("mcp_tool_reboot")) document.getElementById("mcp_tool_reboot").checked = configState.mcp?.tool_reboot !== false;
    if (document.getElementById("mcp_tool_theme")) document.getElementById("mcp_tool_theme").checked = configState.mcp?.tool_theme !== false;

    // 10. Flash & System
    if (document.getElementById("flash_size")) document.getElementById("flash_size").value = configState.flash_system.flash_size;
    if (document.getElementById("flash_mode")) document.getElementById("flash_mode").value = configState.flash_system.flash_mode;
    if (document.getElementById("flash_freq")) document.getElementById("flash_freq").value = configState.flash_system.flash_freq;
    if (document.getElementById("psram_mode")) document.getElementById("psram_mode").value = configState.flash_system.psram_mode;
    if (document.getElementById("psram_speed")) document.getElementById("psram_speed").value = configState.flash_system.psram_speed;
    if (document.getElementById("cpu_freq")) document.getElementById("cpu_freq").value = configState.flash_system.cpu_freq;
    if (document.getElementById("partition_table")) document.getElementById("partition_table").value = configState.flash_system.partition_table;

    if (document.getElementById("audio_mode")) document.getElementById("audio_mode").dispatchEvent(new Event("change"));
    if (document.getElementById("display_type")) document.getElementById("display_type").dispatchEvent(new Event("change"));
    if (document.getElementById("touch_chip")) document.getElementById("touch_chip").dispatchEvent(new Event("change"));
  }

  // Sync static fields back to state
  static syncUIToState() {
    // 1. General & Language
    if (!configState.general) configState.general = {};
    if (document.getElementById("general_ota_url")) configState.general.ota_url = document.getElementById("general_ota_url").value;
    if (document.getElementById("general_language")) configState.general.language = document.getElementById("general_language").value;
    if (document.getElementById("general_flash_assets")) configState.general.flash_assets = document.getElementById("general_flash_assets").value;
    if (document.getElementById("general_custom_assets_file")) configState.general.custom_assets_file = document.getElementById("general_custom_assets_file").value;

    // 2. Audio
    if (document.getElementById("audio_mode")) configState.audio.mode = document.getElementById("audio_mode").value;
    if (document.getElementById("audio_spk_driver")) configState.audio.spk_driver = document.getElementById("audio_spk_driver").value;
    if (document.getElementById("audio_mic_driver")) configState.audio.mic_driver = document.getElementById("audio_mic_driver").value;
    if (document.getElementById("audio_sample_rate")) configState.audio.sample_rate = parseInt(document.getElementById("audio_sample_rate").value, 10);
    if (document.getElementById("audio_spk_bclk")) configState.audio.spk_bclk = parseInt(document.getElementById("audio_spk_bclk").value, 10);
    if (document.getElementById("audio_spk_ws")) configState.audio.spk_ws = parseInt(document.getElementById("audio_spk_ws").value, 10);
    if (document.getElementById("audio_spk_dout")) configState.audio.spk_dout = parseInt(document.getElementById("audio_spk_dout").value, 10);
    if (document.getElementById("audio_mic_sck")) configState.audio.mic_sck = parseInt(document.getElementById("audio_mic_sck").value, 10);
    if (document.getElementById("audio_mic_ws")) configState.audio.mic_ws = parseInt(document.getElementById("audio_mic_ws").value, 10);
    if (document.getElementById("audio_mic_din")) configState.audio.mic_din = parseInt(document.getElementById("audio_mic_din").value, 10);
    if (document.getElementById("audio_codec_sda")) configState.audio.codec_sda = parseInt(document.getElementById("audio_codec_sda").value, 10);
    if (document.getElementById("audio_codec_scl")) configState.audio.codec_scl = parseInt(document.getElementById("audio_codec_scl").value, 10);
    if (document.getElementById("audio_mclk")) configState.audio.mclk = parseInt(document.getElementById("audio_mclk").value, 10);
    if (document.getElementById("audio_pa_pin")) configState.audio.pa_pin = parseInt(document.getElementById("audio_pa_pin").value, 10);

    // 3. Display & UI Hardware
    if (document.getElementById("display_type")) configState.display.type = document.getElementById("display_type").value;
    if (document.getElementById("ui_weather_city")) configState.display.weather_city = document.getElementById("ui_weather_city").value;
    if (document.getElementById("ui_voice_wave")) configState.display.voice_wave = document.getElementById("ui_voice_wave").checked;

    if (document.getElementById("display_mosi")) configState.display.mosi = parseInt(document.getElementById("display_mosi").value, 10);
    if (document.getElementById("display_clk")) configState.display.clk = parseInt(document.getElementById("display_clk").value, 10);
    if (document.getElementById("display_cs")) configState.display.cs = parseInt(document.getElementById("display_cs").value, 10);
    if (document.getElementById("display_dc")) configState.display.dc = parseInt(document.getElementById("display_dc").value, 10);
    if (document.getElementById("display_rst")) configState.display.rst = parseInt(document.getElementById("display_rst").value, 10);
    if (document.getElementById("display_blk")) configState.display.blk = parseInt(document.getElementById("display_blk").value, 10);
    if (document.getElementById("display_uart_port")) configState.display.uart_port = parseInt(document.getElementById("display_uart_port").value, 10);
    if (document.getElementById("display_uart_baud")) configState.display.uart_baud = parseInt(document.getElementById("display_uart_baud").value, 10);
    if (document.getElementById("display_uart_tx")) configState.display.uart_tx = parseInt(document.getElementById("display_uart_tx").value, 10);
    if (document.getElementById("display_uart_rx")) configState.display.uart_rx = parseInt(document.getElementById("display_uart_rx").value, 10);
    if (document.getElementById("display_i2c_sda")) configState.display.i2c_sda = parseInt(document.getElementById("display_i2c_sda").value, 10);
    if (document.getElementById("display_i2c_scl")) configState.display.i2c_scl = parseInt(document.getElementById("display_i2c_scl").value, 10);
    if (document.getElementById("display_width")) configState.display.width = parseInt(document.getElementById("display_width").value, 10);
    if (document.getElementById("display_height")) configState.display.height = parseInt(document.getElementById("display_height").value, 10);
    if (document.getElementById("display_offset_x")) configState.display.offset_x = parseInt(document.getElementById("display_offset_x").value, 10);
    if (document.getElementById("display_offset_y")) configState.display.offset_y = parseInt(document.getElementById("display_offset_y").value, 10);
    if (document.getElementById("display_invert")) configState.display.invert = document.getElementById("display_invert").checked;
    if (document.getElementById("display_swap_xy")) configState.display.swap_xy = document.getElementById("display_swap_xy").checked;
    if (document.getElementById("display_mirror_x")) configState.display.mirror_x = document.getElementById("display_mirror_x").checked;
    if (document.getElementById("display_mirror_y")) configState.display.mirror_y = document.getElementById("display_mirror_y").checked;

    // Touch Controller
    if (document.getElementById("touch_chip")) configState.display.touch_chip = document.getElementById("touch_chip").value;
    if (document.getElementById("touch_sda")) configState.display.touch_sda = parseInt(document.getElementById("touch_sda").value, 10);
    if (document.getElementById("touch_scl")) configState.display.touch_scl = parseInt(document.getElementById("touch_scl").value, 10);
    if (document.getElementById("touch_int")) configState.display.touch_int = parseInt(document.getElementById("touch_int").value, 10);
    if (document.getElementById("touch_rst")) configState.display.touch_rst = parseInt(document.getElementById("touch_rst").value, 10);

    // 4. Wake Word & AFE
    if (!configState.wake_word) configState.wake_word = {};
    if (document.getElementById("wakeword_type")) configState.wake_word.type = document.getElementById("wakeword_type").value;
    if (document.getElementById("wakeword_threshold")) configState.wake_word.threshold = parseInt(document.getElementById("wakeword_threshold").value, 10);
    if (document.getElementById("wakeword_pinyin")) configState.wake_word.pinyin = document.getElementById("wakeword_pinyin").value;
    if (document.getElementById("wakeword_display")) configState.wake_word.display = document.getElementById("wakeword_display").value;
    if (document.getElementById("audio_device_aec")) configState.wake_word.device_aec = document.getElementById("audio_device_aec").checked;
    if (document.getElementById("audio_server_aec")) configState.wake_word.server_aec = document.getElementById("audio_server_aec").checked;
    if (document.getElementById("audio_debugger_server")) configState.wake_word.debugger_server = document.getElementById("audio_debugger_server").value;

    // 5. Network
    if (!configState.network) configState.network = {};
    if (document.getElementById("network_wifi_method")) configState.network.wifi_method = document.getElementById("network_wifi_method").value;
    if (document.getElementById("network_secondary")) configState.network.secondary = document.getElementById("network_secondary").value;

    // 6. MCP Server Tools (Software Protocol Layer - No hardware pin conflict)
    if (!configState.mcp) configState.mcp = {};
    if (document.getElementById("mcp_server_enable")) configState.mcp.enable = document.getElementById("mcp_server_enable").checked;
    if (document.getElementById("mcp_tool_peripherals")) configState.mcp.tool_peripherals = document.getElementById("mcp_tool_peripherals").checked;
    if (document.getElementById("mcp_tool_info")) configState.mcp.tool_info = document.getElementById("mcp_tool_info").checked;
    if (document.getElementById("mcp_tool_reboot")) configState.mcp.tool_reboot = document.getElementById("mcp_tool_reboot").checked;
    if (document.getElementById("mcp_tool_theme")) configState.mcp.tool_theme = document.getElementById("mcp_tool_theme").checked;

    // 10. Flash & System
    if (document.getElementById("flash_size")) configState.flash_system.flash_size = document.getElementById("flash_size").value;
    if (document.getElementById("flash_mode")) configState.flash_system.flash_mode = document.getElementById("flash_mode").value;
    if (document.getElementById("flash_freq")) configState.flash_system.flash_freq = document.getElementById("flash_freq").value;
    if (document.getElementById("psram_mode")) configState.flash_system.psram_mode = document.getElementById("psram_mode").value;
    if (document.getElementById("psram_speed")) configState.flash_system.psram_speed = document.getElementById("psram_speed").value;
    if (document.getElementById("cpu_freq")) configState.flash_system.cpu_freq = document.getElementById("cpu_freq").value;
    if (document.getElementById("partition_table")) configState.flash_system.partition_table = document.getElementById("partition_table").value;
  }

  // Render items lists
  static renderAllLists() {
    GUIController.renderButtonsList();
    GUIController.renderPeripheralsList();
    GUIController.renderCommunicationList();
  }

  static renderButtonsList() {
    const container = document.getElementById("buttons-list");
    if (!container) return;

    if (configState.buttons.length === 0) {
      container.innerHTML = `
        <div class="empty-state" style="grid-column: 1 / -1;">
          <div class="empty-icon">🔘</div>
          <div class="empty-title">Chưa cấu hình nút bấm nào</div>
          <div class="empty-desc">Nhấn nút "+ Thêm phím bấm / Input" ở trên để bổ sung BOOT, Touch, Wake, hoặc Rotary Encoder (main/drivers/input).</div>
        </div>
      `;
      return;
    }

    container.innerHTML = configState.buttons.map(b => {
      let pinDisplay = "";
      if (b.type === "rotary") {
        pinDisplay = `<span class="pin-tag">Phase A: GPIO ${b.pin_a}</span> <span class="pin-tag">Phase B: GPIO ${b.pin_b}</span> <span class="pin-tag">Key: GPIO ${b.pin_key}</span>`;
      } else {
        pinDisplay = `<span class="pin-tag">GPIO ${b.pin}</span>`;
      }

      return `
        <div class="item-card">
          <div class="item-card-header">
            <div class="item-title">
              <span>🔘</span> ${b.name}
            </div>
            <span class="item-type-badge">${b.type.toUpperCase()}</span>
          </div>
          <div class="item-pins-list">
            ${pinDisplay}
          </div>
          <div class="item-card-actions">
            ${b.type !== "boot" ? `<button class="btn-item-action delete" onclick="GUIController.deleteItem('buttons', '${b.id}')">Gỡ bỏ</button>` : `<span style="font-size:0.75rem; color:var(--text-muted);">Mặc định</span>`}
          </div>
        </div>
      `;
    }).join("");
  }

  static renderPeripheralsList() {
    const container = document.getElementById("peripherals-list");
    if (!container) return;

    if (configState.peripherals.length === 0) {
      container.innerHTML = `
        <div class="empty-state" style="grid-column: 1 / -1;">
          <div class="empty-icon">🔌</div>
          <div class="empty-title">Chưa có ngoại vi hoặc cảm biến nào</div>
          <div class="empty-desc">Nhấn "+ Thêm ngoại vi / cảm biến" ở trên để thêm các driver từ main/drivers/actuator và main/drivers/sensor.</div>
        </div>
      `;
      return;
    }

    container.innerHTML = configState.peripherals.map(p => {
      let pinDisplay = "";
      if (p.type === "sensor_hcsr04") {
        pinDisplay = `<span class="pin-tag">Trig: GPIO ${p.trig_pin}</span> <span class="pin-tag">Echo: GPIO ${p.echo_pin}</span>`;
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
      } else if (p.type === "sensor_vl6180x") {
        pinDisplay = `<span class="pin-tag">SDA: GPIO ${p.sda}</span> <span class="pin-tag">SCL: GPIO ${p.scl}</span>`;
      } else if (p.type === "motor_dc") {
        pinDisplay = `<span class="pin-tag">PWMA: ${p.pwma}</span> <span class="pin-tag">DIRA: ${p.dira}</span> <span class="pin-tag">PWMB: ${p.pwmb}</span> <span class="pin-tag">DIRB: ${p.dirb}</span>`;
      } else if (p.type === "battery_adc" || p.type === "sensor_gas" || p.type === "sensor_ldr") {
        pinDisplay = `<span class="pin-tag">ADC Channel ${p.channel}</span>`;
      } else {
        pinDisplay = `<span class="pin-tag">GPIO ${p.pin}</span>`;
      }

      return `
        <div class="item-card">
          <div class="item-card-header">
            <div class="item-title">
              <span>${p.type.startsWith('sensor') ? '🌡️' : '💡'}</span> ${p.name}
            </div>
            <span class="item-type-badge">${p.type.toUpperCase()}</span>
          </div>
          <div class="item-pins-list">
            ${pinDisplay}
          </div>
          <div class="item-card-actions">
            <button class="btn-item-action delete" onclick="GUIController.deleteItem('peripherals', '${p.id}')">Gỡ bỏ</button>
          </div>
        </div>
      `;
    }).join("");
  }

  static renderCommunicationList() {
    const container = document.getElementById("communication-list");
    if (!container) return;

    if (configState.communication.length === 0) {
      container.innerHTML = `
        <div class="empty-state" style="grid-column: 1 / -1;">
          <div class="empty-icon">📡</div>
          <div class="empty-title">Chưa có giao tiếp hoặc thẻ nhớ nào</div>
          <div class="empty-desc">Nhấn "+ Thêm giao tiếp / Thẻ nhớ" ở trên để bổ sung thẻ nhớ MicroSD (main/drivers/storage) hoặc cổng UART/I2C/SPI.</div>
        </div>
      `;
      return;
    }

    container.innerHTML = configState.communication.map(c => {
      let pinDisplay = "";
      if (c.type === "uart") {
        pinDisplay = `<span class="pin-tag">Port ${c.port}</span> <span class="pin-tag">TX: GPIO ${c.tx}</span> <span class="pin-tag">RX: GPIO ${c.rx}</span> <span class="pin-tag">${c.baudrate} bps</span>`;
      } else if (c.type === "i2c") {
        pinDisplay = `<span class="pin-tag">Port ${c.port}</span> <span class="pin-tag">SDA: GPIO ${c.sda}</span> <span class="pin-tag">SCL: GPIO ${c.scl}</span>`;
      } else if (c.type === "spi") {
        pinDisplay = `<span class="pin-tag">MOSI: GPIO ${c.mosi}</span> <span class="pin-tag">MISO: GPIO ${c.miso}</span> <span class="pin-tag">SCLK: GPIO ${c.sclk}</span>`;
            } else if (c.type === "comm_twai") {
        cfg.push(`#define CUSTOM_PERIPH_TWAI_TX_PIN             ${gpioMacro(c.tx)}`);
        cfg.push(`#define CUSTOM_PERIPH_TWAI_RX_PIN             ${gpioMacro(c.rx)}`);
      } else if (c.type === "comm_4g") {
        cfg.push(`#define CUSTOM_PERIPH_4G_UART_TX_PIN          ${gpioMacro(c.tx)}`);
        cfg.push(`#define CUSTOM_PERIPH_4G_UART_RX_PIN          ${gpioMacro(c.rx)}`);
        cfg.push(`#define CUSTOM_PERIPH_4G_PWRKEY_PIN           ${gpioMacro(c.pwr)}`);
            } else if (c.type === "comm_twai") {
        registerPin(c.tx, `${c.name} TX`, `comm_${c.id}_tx`);
        registerPin(c.rx, `${c.name} RX`, `comm_${c.id}_rx`);
      } else if (c.type === "comm_4g") {
        registerPin(c.tx, `${c.name} TX`, `comm_${c.id}_tx`);
        registerPin(c.rx, `${c.name} RX`, `comm_${c.id}_rx`);
        registerPin(c.pwr, `${c.name} PWR`, `comm_${c.id}_pwr`);
      } else if (c.type === "sdcard") {
        pinDisplay = `<span class="pin-tag">CS: GPIO ${c.cs}</span> <span class="pin-tag">MOSI: GPIO ${c.mosi}</span> <span class="pin-tag">MISO: GPIO ${c.miso}</span> <span class="pin-tag">SCK: GPIO ${c.sclk}</span>`;
      }

      return `
        <div class="item-card">
          <div class="item-card-header">
            <div class="item-title">
              <span>${c.type === 'sdcard' ? '💾' : '📡'}</span> ${c.name}
            </div>
            <span class="item-type-badge">${c.type.toUpperCase()}</span>
          </div>
          <div class="item-pins-list">
            ${pinDisplay}
          </div>
          <div class="item-card-actions">
            <button class="btn-item-action delete" onclick="GUIController.deleteItem('communication', '${c.id}')">Gỡ bỏ</button>
          </div>
        </div>
      `;
    }).join("");
  }

  // Delete item handler
  static deleteItem(category, id) {
    if (category === "buttons") {
      configState.buttons = configState.buttons.filter(b => b.id !== id);
      GUIController.renderButtonsList();
      GUIController.checkPanelDirty("panel-buttons");
    } else if (category === "peripherals") {
      configState.peripherals = configState.peripherals.filter(p => p.id !== id);
      GUIController.renderPeripheralsList();
      GUIController.checkPanelDirty("panel-peripherals");
    } else if (category === "communication") {
      configState.communication = configState.communication.filter(c => c.id !== id);
      GUIController.renderCommunicationList();
      GUIController.checkPanelDirty("panel-communication");
    }
    GUIController.updateValidation();
  }

  // Open Add Item Modal
  static openAddItemModal(category) {
    currentModalCategory = category;
    const modal = document.getElementById("add-item-modal") || document.getElementById("modal-add-item");
    const titleEl = document.getElementById("modal-dialog-title") || document.getElementById("modal-item-title");
    const selectEl = document.getElementById("modal-item-type") || document.getElementById("modal-item-type-select");

    if (!modal || !titleEl || !selectEl) return;
    selectEl.innerHTML = "";

    if (category === "buttons") {
      titleEl.innerHTML = "<span>🔘</span> Thêm Driver Input (main/drivers/input)";
      selectEl.innerHTML = `
        <option value="touch">Touch Button (Nút cảm ứng chạm)</option>
        <option value="wake">WAKE Button (Nút đánh thức hệ thống)</option>
        <option value="vol_up">Volume Up (Nút tăng âm lượng)</option>
        <option value="vol_down">Volume Down (Nút giảm âm lượng)</option>
        <option value="rotary">Rotary Encoder EC11 (Chiết áp vô tận A/B/Key)</option>
      `;
    } else if (category === "peripherals") {
      titleEl.innerHTML = "<span>🔌</span> Thêm Actuator & Sensor (main/drivers/actuator & sensor)";
      selectEl.innerHTML = `
        <optgroup label="✨ Driver Tùy Chỉnh (User Custom Drivers)">
          <option value="user_custom_led">✨ Driver LED Tùy Chỉnh (User Custom LED Driver)</option>
          <option value="user_custom_sensors">✨ Driver Cảm Biến / Mạch Mở Rộng Tùy Chỉnh (User Custom Sensors Driver)</option>
        </optgroup>
        <optgroup label="Actuators (Thiết bị chấp hành)">
          <option value="actuator_pca9685">Mở rộng PWM PCA9685 (I2C)</option>
          <option value="led">Built-in Status LED (Đèn LED trạng thái)</option>
          <option value="relay_lamp">Relay 220V / Đèn bàn MCP Tool</option>
          <option value="buzzer">Buzzer Alarm (Còi chíp báo động)</option>
          <option value="haptic">Haptic Vibration Motor (Động cơ rung phản hồi)</option>
          <option value="servo">Servo Motor PWM 50Hz (Động cơ servo góc)</option>
          <option value="motor_dc">DC Motor H-Bridge Driver TB6612 (Động cơ DC)</option>
        </optgroup>
        <optgroup label="Sensors (Cảm biến)">
          <option value="sensor_nfc">Đầu đọc thẻ RFID/NFC (I2C)</option>
          <option value="sensor_rtc">Đồng hồ thời gian thực RTC (I2C)</option>
          <option value="sensor_vl6180x">Cảm biến khoảng cách Laser ToF VL6180X (I2C)</option>
          <option value="sensor_dht">Cảm biến nhiệt độ & độ ẩm DHT11/22</option>
          <option value="sensor_aht20">Cảm biến nhiệt độ & độ ẩm AHT20/21 (I2C)</option>
          <option value="sensor_pir">Cảm biến chuyển động hồng ngoại PIR</option>
          <option value="sensor_vibration">Cảm biến rung chấn SW-420</option>
          <option value="sensor_flame">Cảm biến phát hiện lửa Flame Sensor</option>
          <option value="sensor_hcsr04">Cảm biến khoảng cách siêu âm HC-SR04</option>
          <option value="sensor_tp4056">Giám sát sạc Pin TP4056 CHRG</option>
          <option value="sensor_gas">Cảm biến khí Gas MQ-2 (ADC1)</option>
          <option value="sensor_ldr">Cảm biến ánh sáng quang trở LDR (ADC1)</option>
          <option value="sensor_mpu6050">Cảm biến gia tốc MPU6050 (I2C)</option>
          <option value="sensor_max30102">Cảm biến nhịp tim MAX30102 (I2C)</option>
          <option value="led_ws2812b">Dải LED RGB WS2812B (RMT)</option>
          <option value="battery_adc">Giám sát dung lượng Pin qua ADC</option>
        </optgroup>
      `;
    } else if (category === "communication") {
      titleEl.innerHTML = "<span>📡</span> Thêm Giao tiếp & Thẻ nhớ (main/drivers/storage)";
      selectEl.innerHTML = `
        <option value="sdcard">Thẻ nhớ MicroSD Card qua SPI (Storage)</option>
        <option value="uart">Cổng UART mở rộng (D-UART / Nextion LCD)</option>
        <option value="i2c">Cổng I2C Bus dùng chung</option>
        <option value="spi">Cổng SPI Bus dùng chung</option>
        <option value="comm_twai">Mạng CAN/TWAI Bus</option>
        <option value="comm_4g">Module 4G LTE (UART)</option>
      `;
    }

    GUIController.renderModalFields(category, selectEl.value);
    modal.classList.add("active");
  }

  // Render dynamic form fields inside modal
  static renderModalFields(category, type) {
    const fieldsContainer = document.getElementById("modal-dynamic-fields") || document.getElementById("modal-item-fields");
    if (!fieldsContainer) return;
    fieldsContainer.innerHTML = "";

    if (category === "buttons") {
      if (type === "rotary") {
        fieldsContainer.innerHTML = `
          <div class="form-item">
            <label class="form-label" for="modal_field_rot_a">Phase A Pin</label>
            <input type="number" class="form-control" id="modal_field_rot_a" min="0" max="48" value="43">
          </div>
          <div class="form-item">
            <label class="form-label" for="modal_field_rot_b">Phase B Pin</label>
            <input type="number" class="form-control" id="modal_field_rot_b" min="0" max="48" value="44">
          </div>
          <div class="form-item">
            <label class="form-label" for="modal_field_rot_key">Key Push Pin</label>
            <input type="number" class="form-control" id="modal_field_rot_key" min="0" max="48" value="21">
          </div>
        `;
      } else {
        fieldsContainer.innerHTML = `
          <div class="form-item">
            <label class="form-label" for="modal_field_pin">Chân GPIO <span class="pin-type">GPIO (0-48)</span></label>
            <input type="number" class="form-control" id="modal_field_pin" min="0" max="48" value="1">
          </div>
        `;
      }
    } else if (category === "peripherals") {
      if (type === "led") {
        fieldsContainer.innerHTML = `
          <div class="form-item">
            <label class="form-label" for="modal_field_pin">Chân LED GPIO</label>
            <input type="number" class="form-control" id="modal_field_pin" min="0" max="48" value="48">
          </div>
        `;
      } else if (type === "relay_lamp") {
        fieldsContainer.innerHTML = `
          <div class="form-item">
            <label class="form-label" for="modal_field_pin">Chân Relay / Lamp GPIO</label>
            <input type="number" class="form-control" id="modal_field_pin" min="0" max="48" value="13">
          </div>
        `;
      } else if (type === "buzzer") {
        fieldsContainer.innerHTML = `
          <div class="form-item">
            <label class="form-label" for="modal_field_pin">Chân Buzzer Alarm GPIO</label>
            <input type="number" class="form-control" id="modal_field_pin" min="0" max="48" value="41">
          </div>
        `;
      } else if (type === "haptic") {
        fieldsContainer.innerHTML = `
          <div class="form-item">
            <label class="form-label" for="modal_field_pin">Chân Haptic Motor GPIO</label>
            <input type="number" class="form-control" id="modal_field_pin" min="0" max="48" value="42">
          </div>
        `;
      } else if (type === "servo") {
        fieldsContainer.innerHTML = `
          <div class="form-item">
            <label class="form-label" for="modal_field_pin">Chân Servo PWM 50Hz GPIO</label>
            <input type="number" class="form-control" id="modal_field_pin" min="0" max="48" value="48">
          </div>
        `;
      } else if (type === "motor_dc") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label">PWMA Pin</label><input type="number" class="form-control" id="modal_field_pwma" value="41"></div>
          <div class="form-item"><label class="form-label">DIRA Pin</label><input type="number" class="form-control" id="modal_field_dira" value="42"></div>
          <div class="form-item"><label class="form-label">PWMB Pin</label><input type="number" class="form-control" id="modal_field_pwmb" value="1"></div>
          <div class="form-item"><label class="form-label">DIRB Pin</label><input type="number" class="form-control" id="modal_field_dirb" value="2"></div>
        `;
      } else if (type === "sensor_vl6180x" || type === "sensor_aht20" || type === "sensor_mpu6050" || type === "sensor_max30102") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label">I2C SDA Pin</label><input type="number" class="form-control" id="modal_field_sda" value="8"></div>
          <div class="form-item"><label class="form-label">I2C SCL Pin</label><input type="number" class="form-control" id="modal_field_scl" value="9"></div>
          <div class="form-item"><label class="form-label">Địa chỉ I2C tùy biến (Hex, VD: 0x41)</label><input type="text" class="form-control" id="modal_field_addr" value="" placeholder="Để trống = Mặc định"></div>
        `;
      } else if (type === "led_ws2812b") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label">Chân Data (RMT)</label><input type="number" class="form-control" id="modal_field_pin" value="21"></div>
          <div class="form-item"><label class="form-label">Số lượng LED</label><input type="number" class="form-control" id="modal_field_count" value="12"></div>
          <div class="form-item"><label class="form-label text-warning"><i class="bi bi-exclamation-triangle"></i> LƯU Ý</label><span>Không đấu dây Data >20cm trực tiếp 3.3V. Cấp nguồn 5V ngoài cho LED.</span></div>
        `;
            } else if (type === "sensor_nfc") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_nfc_sda">I2C SDA (hoặc SPI MOSI)</label><input type="number" class="form-control" id="modal_field_nfc_sda" value="8"></div>
          <div class="form-item"><label class="form-label" for="modal_field_nfc_scl">I2C SCL (hoặc SPI SCK)</label><input type="number" class="form-control" id="modal_field_nfc_scl" value="9"></div>
          <div class="form-item"><label class="form-label" for="modal_field_nfc_irq">Ngắt IRQ</label><input type="number" class="form-control" id="modal_field_nfc_irq" value="17"></div>
          <div class="form-item"><label class="form-label" for="modal_field_nfc_rst">Reset RST</label><input type="number" class="form-control" id="modal_field_nfc_rst" value="18"></div>
        `;
      } else if (type === "sensor_rtc") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_rtc_sda">I2C SDA</label><input type="number" class="form-control" id="modal_field_rtc_sda" value="8"></div>
          <div class="form-item"><label class="form-label" for="modal_field_rtc_scl">I2C SCL</label><input type="number" class="form-control" id="modal_field_rtc_scl" value="9"></div>
          <div class="form-item"><label class="form-label" for="modal_field_rtc_int">Ngắt INT</label><input type="number" class="form-control" id="modal_field_rtc_int" value="17"></div>
        `;
      } else if (type === "actuator_pca9685") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_pca_sda">I2C SDA</label><input type="number" class="form-control" id="modal_field_pca_sda" value="8"></div>
          <div class="form-item"><label class="form-label" for="modal_field_pca_scl">I2C SCL</label><input type="number" class="form-control" id="modal_field_pca_scl" value="9"></div>
          <div class="form-item"><label class="form-label" for="modal_field_pca_oe">Chân OE</label><input type="number" class="form-control" id="modal_field_pca_oe" value="-1"></div>
        `;
      } else if (type === "comm_twai") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_twai_tx">TWAI TX</label><input type="number" class="form-control" id="modal_field_twai_tx" value="15"></div>
          <div class="form-item"><label class="form-label" for="modal_field_twai_rx">TWAI RX</label><input type="number" class="form-control" id="modal_field_twai_rx" value="16"></div>
        `;
      } else if (type === "comm_4g") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_4g_tx">UART TX</label><input type="number" class="form-control" id="modal_field_4g_tx" value="17"></div>
          <div class="form-item"><label class="form-label" for="modal_field_4g_rx">UART RX</label><input type="number" class="form-control" id="modal_field_4g_rx" value="18"></div>
          <div class="form-item"><label class="form-label" for="modal_field_4g_pwr">PWRKEY</label><input type="number" class="form-control" id="modal_field_4g_pwr" value="2"></div>
        `;
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
        } else if (type === "sensor_dht") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_pin">Chân Data DHT11/22</label><input type="number" class="form-control" id="modal_field_pin" value="14"></div>
          <div class="form-item"><label class="form-label text-warning"><i class="bi bi-exclamation-triangle"></i> Yêu cầu phần cứng</label><span>Bắt buộc nối điện trở Pull-up 4.7k hoặc 10k giữa VCC và chân Data</span></div>
        `;
      } else if (type === "sensor_pir") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_pin">Chân PIR Signal</label><input type="number" class="form-control" id="modal_field_pin" value="10"></div>
        `;
      } else if (type === "sensor_vibration") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_pin">Chân Cảm biến rung SW-420</label><input type="number" class="form-control" id="modal_field_pin" value="6"></div>
        `;
      } else if (type === "sensor_flame") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_pin">Chân Cảm biến ngọn lửa</label><input type="number" class="form-control" id="modal_field_pin" value="7"></div>
        `;
      } else if (type === "sensor_tp4056") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_pin">Chân TP4056 CHRG Pin</label><input type="number" class="form-control" id="modal_field_pin" value="3"></div>
        `;
      } else if (type === "sensor_hcsr04") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_trig">Trigger Pin</label><input type="number" class="form-control" id="modal_field_trig" value="11"></div>
          <div class="form-item"><label class="form-label" for="modal_field_echo">Echo Pin</label><input type="number" class="form-control" id="modal_field_echo" value="12"></div>
        `;
      } else if (type === "sensor_gas" || type === "sensor_ldr" || type === "battery_adc") {
        fieldsContainer.innerHTML = `
          <div class="form-item">
            <label class="form-label" for="modal_field_adc">Kênh ADC1 Channel (0-4)</label>
            <select class="form-control" id="modal_field_adc">
              <option value="0">ADC1 Channel 0 (GPIO 1)</option>
              <option value="1">ADC1 Channel 1 (GPIO 2)</option>
              <option value="2" selected>ADC1 Channel 2 (GPIO 3)</option>
              <option value="3">ADC1 Channel 3 (GPIO 4)</option>
            </select>
          </div>
        `;
      }
    } else if (category === "communication") {
      if (type === "sdcard") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label">Chip Select (CS)</label><input type="number" class="form-control" id="modal_field_cs" value="10"></div>
          <div class="form-item"><label class="form-label">SPI MOSI</label><input type="number" class="form-control" id="modal_field_mosi" value="11"></div>
          <div class="form-item"><label class="form-label">SPI MISO</label><input type="number" class="form-control" id="modal_field_miso" value="13"></div>
          <div class="form-item"><label class="form-label">SPI SCK</label><input type="number" class="form-control" id="modal_field_sclk" value="12"></div>
        `;
      } else if (type === "uart") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label">Cổng UART</label><select class="form-control" id="modal_field_uart_port"><option value="1">UART Port 1</option><option value="2">UART Port 2</option></select></div>
          <div class="form-item"><label class="form-label">Baudrate</label><select class="form-control" id="modal_field_baud"><option value="115200">115200 bps</option><option value="9600">9600 bps</option><option value="921600">921600 bps</option></select></div>
          <div class="form-item"><label class="form-label">TX Pin</label><input type="number" class="form-control" id="modal_field_tx" value="17"></div>
          <div class="form-item"><label class="form-label">RX Pin</label><input type="number" class="form-control" id="modal_field_rx" value="18"></div>
        `;
      } else if (type === "i2c") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label">Cổng I2C</label><select class="form-control" id="modal_field_i2c_port"><option value="0">I2C Port 0</option><option value="1">I2C Port 1</option></select></div>
          <div class="form-item"><label class="form-label">SDA Pin</label><input type="number" class="form-control" id="modal_field_sda" value="8"></div>
          <div class="form-item"><label class="form-label">SCL Pin</label><input type="number" class="form-control" id="modal_field_scl" value="9"></div>
        `;
      } else if (type === "spi") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label">MOSI Pin</label><input type="number" class="form-control" id="modal_field_mosi" value="11"></div>
          <div class="form-item"><label class="form-label">MISO Pin</label><input type="number" class="form-control" id="modal_field_miso" value="13"></div>
          <div class="form-item"><label class="form-label">SCLK Pin</label><input type="number" class="form-control" id="modal_field_sclk" value="12"></div>
        `;
      }
    }
  }

  // Save Item from Modal
  static saveItemFromModal() {
    const typeSelect = document.getElementById("modal-item-type") || document.getElementById("modal-item-type-select");
    if (!typeSelect) return;
    const type = typeSelect.value;
    const id = `${type}_${Date.now()}`;

    if (currentModalCategory === "buttons") {
      if (type === "rotary") {
        const a = parseInt(document.getElementById("modal_field_rot_a").value, 10);
        const b = parseInt(document.getElementById("modal_field_rot_b").value, 10);
        const k = parseInt(document.getElementById("modal_field_rot_key").value, 10);
        configState.buttons.push({ id, type, name: "Rotary Encoder EC11", pin_a: a, pin_b: b, pin_key: k });
      } else {
        const pin = parseInt(document.getElementById("modal_field_pin").value, 10);
        const nameMap = {
          touch: "Touch Button",
          wake: "WAKE Button",
          vol_up: "Volume Up Button",
          vol_down: "Volume Down Button"
        };
        configState.buttons.push({ id, type, name: nameMap[type] || "Phím bấm", pin });
      }
      GUIController.renderButtonsList();
      GUIController.checkPanelDirty("panel-buttons");
    } else if (currentModalCategory === "peripherals") {
      if (type === "sensor_hcsr04") {
        const trig = parseInt(document.getElementById("modal_field_trig").value, 10);
        const echo = parseInt(document.getElementById("modal_field_echo").value, 10);
        configState.peripherals.push({ id, type, name: "Cảm biến siêu âm HC-SR04", trig_pin: trig, echo_pin: echo });
      } else if (type === "sensor_vl6180x" || type === "sensor_aht20" || type === "sensor_mpu6050" || type === "sensor_max30102") {
        const sda = parseInt(document.getElementById("modal_field_sda").value, 10);
        const scl = parseInt(document.getElementById("modal_field_scl").value, 10);
        const addr = document.getElementById("modal_field_addr").value.trim();
        const nameMap = {
          sensor_vl6180x: "Laser ToF VL6180X",
          sensor_aht20: "Nhiệt/Ẩm AHT20/21",
          sensor_mpu6050: "Gia tốc MPU6050",
          sensor_max30102: "Nhịp tim MAX30102"
        };
        configState.peripherals.push({ id, type, name: nameMap[type], sda, scl, addr });
      } else if (type === "led_ws2812b") {
        const pin = parseInt(document.getElementById("modal_field_pin").value, 10);
        const count = parseInt(document.getElementById("modal_field_count").value, 10) || 12;
        configState.peripherals.push({ id, type, name: "LED RGB WS2812B", pin, count });
      } else if (type === "motor_dc") {
        const pwma = parseInt(document.getElementById("modal_field_pwma").value, 10);
        const dira = parseInt(document.getElementById("modal_field_dira").value, 10);
        const pwmb = parseInt(document.getElementById("modal_field_pwmb").value, 10);
        const dirb = parseInt(document.getElementById("modal_field_dirb").value, 10);
        configState.peripherals.push({ id, type, name: "DC Motor H-Bridge TB6612", pwma, dira, pwmb, dirb });
      } else if (type === "battery_adc" || type === "sensor_gas" || type === "sensor_ldr") {
        const ch = parseInt(document.getElementById("modal_field_adc").value, 10);
        const nameMap = {
          battery_adc: "Giám sát Pin ADC",
          sensor_gas: "Cảm biến Gas MQ-2",
          sensor_ldr: "Cảm biến ánh sáng LDR"
        };
        configState.peripherals.push({ id, type, name: nameMap[type], channel: ch });
      } else {
        const pin = parseInt(document.getElementById("modal_field_pin").value, 10);
        const nameMap = {
          led: "Builtin Status LED",
          relay_lamp: "Relay 220V / Lamp MCP",
          buzzer: "Buzzer Alarm",
          haptic: "Haptic Vibration Motor",
          sensor_vl6180x: "Khoảng cách ToF VL6180X",
                    sensor_nfc: "RFID/NFC RC522/PN532",
          sensor_rtc: "Đồng hồ RTC DS3231",
          actuator_pca9685: "Mở rộng PWM PCA9685",
          sensor_dht: "Nhiệt/Ẩm DHT11/22",
          sensor_aht20: "Nhiệt/Ẩm AHT20 (I2C)",
          sensor_mpu6050: "Gia tốc MPU6050 (I2C)",
          sensor_max30102: "Nhịp tim MAX30102",
          led_ws2812b: "LED RGB WS2812B",
          sensor_pir: "Chuyển động PIR",
          sensor_vibration: "Cảm biến rung SW-420",
          sensor_flame: "Cảm biến ngọn lửa Flame",
          sensor_tp4056: "Giám sát sạc TP4056"
        };
        configState.peripherals.push({ id, type, name: nameMap[type] || "Thiết bị ngoại vi", pin });
      }
      GUIController.renderPeripheralsList();
      GUIController.checkPanelDirty("panel-peripherals");
    } else if (currentModalCategory === "communication") {
      if (type === "sdcard") {
        const cs = parseInt(document.getElementById("modal_field_cs").value, 10);
        const mosi = parseInt(document.getElementById("modal_field_mosi").value, 10);
        const miso = parseInt(document.getElementById("modal_field_miso").value, 10);
        const sclk = parseInt(document.getElementById("modal_field_sclk").value, 10);
        configState.communication.push({ id, type, name: "Thẻ nhớ MicroSD Card (SPI)", cs, mosi, miso, sclk });
      } else if (type === "uart") {
        const port = parseInt(document.getElementById("modal_field_uart_port").value, 10);
        const baud = parseInt(document.getElementById("modal_field_baud").value, 10);
        const tx = parseInt(document.getElementById("modal_field_tx").value, 10);
        const rx = parseInt(document.getElementById("modal_field_rx").value, 10);
        configState.communication.push({ id, type, name: `Cổng UART ${port}`, port, baudrate: baud, tx, rx });
      } else if (type === "i2c") {
        const port = parseInt(document.getElementById("modal_field_i2c_port").value, 10);
        const sda = parseInt(document.getElementById("modal_field_sda").value, 10);
        const scl = parseInt(document.getElementById("modal_field_scl").value, 10);
        configState.communication.push({ id, type, name: `I2C Bus ${port}`, port, sda, scl });
      } else if (type === "spi") {
        const mosi = parseInt(document.getElementById("modal_field_mosi").value, 10);
        const miso = parseInt(document.getElementById("modal_field_miso").value, 10);
        const sclk = parseInt(document.getElementById("modal_field_sclk").value, 10);
        configState.communication.push({ id, type, name: `SPI Bus`, mosi, miso, sclk });
      }
      GUIController.renderCommunicationList();
      GUIController.checkPanelDirty("panel-communication");
    }

    const modal = document.getElementById("add-item-modal") || document.getElementById("modal-add-item");
    if (modal) modal.classList.remove("active");
    GUIController.updateValidation();
  }

  // Update validation and UI highlighting
  static updateValidation() {
    const result = PinValidator.validate(configState);
    conflictErrors = result.errors;

    document.querySelectorAll(".form-control").forEach(el => {
      el.classList.remove("has-conflict", "is-locked");
    });

    for (let pin = ESP32S3_RULES.MIN_GPIO; pin <= ESP32S3_RULES.MAX_GPIO; pin++) {
      const cell = document.getElementById(`pin-cell-${pin}`);
      if (!cell) continue;
      cell.classList.remove("used", "conflict");
      if (!ESP32S3_RULES.LOCKED_OCTAL_PINS.includes(pin)) {
        cell.title = `GPIO ${pin}: Còn trống`;
      }
    }

    for (const [pin, users] of result.pinUsage.entries()) {
      const cell = document.getElementById(`pin-cell-${pin}`);
      if (cell) {
        cell.classList.add("used");
        cell.title = `GPIO ${pin}: ${users.map(u => u.fieldName).join(", ")}`;
      }
    }

    const alertBar = document.getElementById("conflict-alert-bar");
    const alertMsg = document.getElementById("conflict-message");

    const btnSaveGlobal = document.getElementById("btn-save") || document.getElementById("btn-save-project");

    if (conflictErrors.length > 0) {
      alertBar.classList.add("visible");
      alertMsg.innerHTML = conflictErrors.map(e => `• ${e.message}`).join("<br>");

      conflictErrors.forEach(err => {
        const input = document.getElementById(err.fieldId);
        if (input) input.classList.add("has-conflict");

        const cell = document.getElementById(`pin-cell-${err.pin}`);
        if (cell) cell.classList.add("conflict");
      });

      if (btnSaveGlobal) btnSaveGlobal.disabled = true;
      document.querySelectorAll(".btn-save-tab").forEach(btn => btn.disabled = true);
    } else {
      alertBar.classList.remove("visible");
      if (btnSaveGlobal) btnSaveGlobal.disabled = !(GUIController.isServerMode || directoryHandle);
      document.querySelectorAll(".btn-save-tab").forEach(btn => btn.disabled = false);
    }
  }

  static async saveAndApplyConfiguration() {
    if (!GUIController.isServerMode && !directoryHandle) {
      alert("Vui lòng chạy kịch bản 'run_configurator.bat' hoặc nhấn 'Mở thư mục xiaozhi' trước khi lưu.");
      return;
    }

    if (conflictErrors.length > 0) {
      alert("Vui lòng giải quyết toàn bộ xung đột chân trước khi lưu cấu hình!");
      return;
    }

    const btnSave = document.getElementById("btn-save");
    if (btnSave) {
      btnSave.disabled = true;
      btnSave.textContent = "Đang lưu cấu hình...";
    }

    try {
      await GUIController.writeConfigFilesToDisk();

      // Show success modal
      document.getElementById("success-modal").classList.add("active");
    } catch (err) {
      console.error("Lỗi khi lưu cấu hình:", err);
      alert("Đã xảy ra lỗi khi ghi file: " + err.message);
    } finally {
      if (btnSave) {
        btnSave.disabled = false;
        btnSave.innerHTML = `
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2z"></path><polyline points="17 21 17 13 7 13 7 21"></polyline><polyline points="7 3 7 8 15 8"></polyline></svg>
          Save & Apply Configuration
        `;
      }
    }
  }

}

// Start application when DOM is loaded in browser
if (typeof window !== "undefined" && typeof window.addEventListener === "function") {
  window.addEventListener("DOMContentLoaded", () => {
    GUIController.init();
  });
}

// Export for Node.js unit testing if applicable
if (typeof module !== "undefined" && module.exports) {
  module.exports = {
    ESP32S3_RULES,
    DEFAULT_CONFIG,
    FileSystemEngine,
    ConfigParser,
    CodeGenerator,
    PinValidator,
    GUIController
  };
}
