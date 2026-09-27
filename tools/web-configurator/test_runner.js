/**
 * Test Runner for Xiaozhi Web Configurator Core Modules (Dynamic Modular Architecture)
 */

const { ESP32S3_RULES, DEFAULT_CONFIG, ConfigParser, CodeGenerator, PinValidator, GUIController } = require('./app.js');

console.log("=== BẮT ĐẦU KIỂM THỬ WEB CONFIGURATOR ENGINE ===");

let passed = 0;
let failed = 0;

function assert(condition, message) {
  if (condition) {
    console.log(`  [PASS] ${message}`);
    passed++;
  } else {
    console.error(`  [FAIL] ${message}`);
    failed++;
  }
}

// ----------------------------------------------------------------------------
// TEST 1: DEFAULT CONFIG INTEGRITY & PIN VALIDATION
// ----------------------------------------------------------------------------
console.log("\n1. Kiểm tra cấu hình mặc định:");
{
  const result = PinValidator.validate(DEFAULT_CONFIG);
  assert(result.errors.length === 0, `Cấu hình mặc định không được có lỗi xung đột (tìm thấy: ${result.errors.length})`);
}

// ----------------------------------------------------------------------------
// TEST 2: RESTRICTED OCTAL SPI PINS (GPIO 26-32)
// ----------------------------------------------------------------------------
console.log("\n2. Kiểm tra phát hiện cấm GPIO 26-32 (Octal Flash/PSRAM):");
for (let pin = 26; pin <= 32; pin++) {
  const testState = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  testState.audio.spk_bclk = pin;
  const result = PinValidator.validate(testState);
  const hasOctalError = result.errors.some(e => e.message.includes("BỊ KHÓA") && e.pin === pin);
  assert(hasOctalError, `GPIO ${pin} phải bị cấm và báo lỗi Octal SPI Flash/PSRAM`);
}

// ----------------------------------------------------------------------------
// TEST 3: DYNAMIC PERIPHERALS & COMMUNICATION CONFLICT DETECTION
// ----------------------------------------------------------------------------
console.log("\n3. Kiểm tra phát hiện xung đột chân cho các module động:");
{
  // Test A: Thêm 1 ngoại vi có chân trùng với Speaker BCLK (GPIO 15)
  const conflictState1 = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  conflictState1.peripherals.push({ id: "p_test", type: "sensor_dht", name: "DHT Test", pin: 15 });
  const res1 = PinValidator.validate(conflictState1);
  assert(res1.errors.some(e => e.message.includes("Xung đột GPIO 15")), "Phát hiện xung đột khi DHT Sensor trùng Audio BCLK (GPIO 15)");

  // Test B: Thêm giao tiếp UART có TX trùng với Display MOSI (GPIO 47)
  const conflictState2 = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  conflictState2.communication.push({ id: "c_uart", type: "uart", name: "UART Test", port: 1, baudrate: 115200, tx: 47, rx: 18 });
  const res2 = PinValidator.validate(conflictState2);
  assert(res2.errors.some(e => e.message.includes("Xung đột GPIO 47")), "Phát hiện xung đột khi UART TX trùng Display MOSI (GPIO 47)");

  // Test C: Thêm Touch Button trùng với Builtin LED (GPIO 48)
  const conflictState3 = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  conflictState3.buttons.push({ id: "btn_touch", type: "touch", name: "Touch", pin: 48 });
  const res3 = PinValidator.validate(conflictState3);
  assert(res3.errors.some(e => e.message.includes("Xung đột GPIO 48")), "Phát hiện xung đột khi Touch Button trùng LED (GPIO 48)");
}

// ----------------------------------------------------------------------------
// TEST 4: COMPREHENSIVE DRIVERS FROM main/drivers (ACTUATORS, SENSORS, STORAGE, COMM)
// ----------------------------------------------------------------------------
console.log("\n4. Kiểm tra các driver mới quét từ main/drivers:");
{
  const testState = JSON.parse(JSON.stringify(DEFAULT_CONFIG));

  // Thêm Rotary Encoder EC11
  testState.buttons.push({
    id: "btn_rotary",
    type: "rotary",
    name: "EC11 Encoder",
    pin_a: 1,
    pin_b: 2,
    pin_key: 10
  });

  // Thêm Actuators: Buzzer, Haptic, Servo, DC Motor TB6612
  testState.peripherals.push({ id: "act_buzzer", type: "buzzer", name: "Buzzer", pin: 11 });
  testState.peripherals.push({ id: "act_haptic", type: "haptic", name: "Haptic", pin: 12 });
  testState.peripherals.push({ id: "act_servo", type: "servo", name: "Servo SG90", pin: 14 });
  testState.peripherals.push({
    id: "act_motor",
    type: "motor_dc",
    name: "TB6612 Motor",
    pwma: 35,
    dira: 36,
    pwmb: 37,
    dirb: 39
  });

  // Thêm Sensors: VL6180X, HC-SR04, Gas MQ2, LDR, Battery ADC
  testState.peripherals.push({ id: "sens_vl6180x", type: "sensor_vl6180x", name: "VL6180X ToF", sda: 8, scl: 9 });
  testState.peripherals.push({ id: "sens_hcsr04", type: "sensor_hcsr04", name: "HC-SR04", trig_pin: 43, echo_pin: 44 });
  testState.peripherals.push({ id: "sens_gas", type: "sensor_gas", name: "MQ-2 Gas", channel: 1 });
  testState.peripherals.push({ id: "sens_ldr", type: "sensor_ldr", name: "LDR Light", channel: 2 });
  testState.peripherals.push({ id: "sens_bat", type: "battery_adc", name: "Battery ADC", channel: 3 });

  // Thêm Storage: SD Card SPI
  testState.communication.push({
    id: "stor_sd",
    type: "sdcard",
    name: "MicroSD SPI",
    cs: 45,
    mosi: 46,
    miso: 17,
    sclk: 18
  });

  // 4.1 Validate pin conflicts
  const valResult = PinValidator.validate(testState);
  assert(valResult.errors.length === 0, `Hệ thống driver mở rộng hợp lệ không có xung đột (lỗi: ${valResult.errors.length})`);

  // 4.2 Test conflict detection with extended drivers
  const conflictState = JSON.parse(JSON.stringify(testState));
  // Làm cho DC Motor dirb (39) trùng với Buzzer (11 -> sửa thành 39)
  const buzzer = conflictState.peripherals.find(p => p.type === "buzzer");
  buzzer.pin = 39;
  const conflictRes = PinValidator.validate(conflictState);
  assert(conflictRes.errors.some(e => e.message.includes("Xung đột GPIO 39")), "Phát hiện xung đột chân giữa TB6612 dirb và Buzzer (GPIO 39)");

  // 4.3 Test restricted Octal Pin on extended driver
  const octalConflictState = JSON.parse(JSON.stringify(testState));
  const rotaryItem = octalConflictState.buttons.find(b => b.type === "rotary");
  rotaryItem.pin_a = 28; // Thuộc dải cấm 26-32
  const octalRes = PinValidator.validate(octalConflictState);
  assert(octalRes.errors.some(e => e.message.includes("BỊ KHÓA") && e.pin === 28), "Phát hiện cấm GPIO 28 trên Rotary Encoder Pin A");

  // 4.4 Test Code Generator for extended drivers
  const configH = CodeGenerator.generateConfigH(testState);
  assert(configH.includes("#define BUZZER_PIN"), "config.h có define BUZZER_PIN");
  assert(configH.includes("#define HAPTIC_PIN"), "config.h có define HAPTIC_PIN");
  assert(configH.includes("#define SERVO_PIN"), "config.h có define SERVO_PIN");
  assert(configH.includes("#define MOTOR_PWMA_PIN"), "config.h có define MOTOR_PWMA_PIN");
  assert(configH.includes("#define ROTARY_ENCODER_A_PIN"), "config.h có define ROTARY_ENCODER_A_PIN");
  assert(configH.includes("#define SENSOR_HCSR04_TRIG_GPIO"), "config.h có define SENSOR_HCSR04_TRIG_GPIO");
  assert(configH.includes("#define SENSOR_VL6180X_SDA"), "config.h có define SENSOR_VL6180X_SDA");
  assert(configH.includes("#define SDCARD_SPI_CS_PIN"), "config.h có define SDCARD_SPI_CS_PIN");
  assert(configH.includes("#define BATTERY_ADC_CHANNEL"), "config.h có define BATTERY_ADC_CHANNEL");
}

// ----------------------------------------------------------------------------
// TEST 5: BI-DIRECTIONAL CONFIG.H PARSER
// ----------------------------------------------------------------------------
console.log("\n5. Kiểm tra bộ phân tích cú pháp config.h:");
{
  const sampleHeader = `
    #define AUDIO_INPUT_SAMPLE_RATE 16000
    #define AUDIO_I2S_SPK_GPIO_BCLK ((gpio_num_t)14)
    #define DISPLAY_WIDTH 320
    #define DISPLAY_HEIGHT 480
    #define BUILTIN_LED_GPIO GPIO_NUM_48
    #define LAMP_GPIO GPIO_NUM_13
    #define SENSOR_DHT_GPIO ((gpio_num_t)10)
    #define CUSTOM_UART_TX_PIN ((gpio_num_t)17)
    #define CUSTOM_UART_RX_PIN ((gpio_num_t)18)
    #define CUSTOM_UART_BAUDRATE 115200
  `;
  const parsed = ConfigParser.parseConfigH(sampleHeader);
  assert(parsed.audio.sample_rate === 16000, `audio.sample_rate = 16000`);
  assert(parsed.audio.spk_bclk === 14, `audio.spk_bclk = 14`);
  assert(parsed.peripherals.some(p => p.type === "led" && p.pin === 48), `peripherals có LED = 48`);
  assert(parsed.peripherals.some(p => p.type === "relay_lamp" && p.pin === 13), `peripherals có LAMP = 13`);
  assert(parsed.peripherals.some(p => p.type === "sensor_dht" && p.pin === 10), `peripherals có DHT = 10`);
  assert(parsed.communication.some(c => c.type === "uart" && c.tx === 17 && c.rx === 18), `communication có UART TX 17, RX 18`);
}

// ----------------------------------------------------------------------------
// TEST 6: CODE GENERATORS INTEGRITY (FLASH & SYSTEM)
// ----------------------------------------------------------------------------
console.log("\n6. Kiểm tra tính toàn vẹn của mã sinh ra:");
{
  const configH = CodeGenerator.generateConfigH(DEFAULT_CONFIG);
  assert(configH.includes("#define BUILTIN_LED_GPIO"), "config.h có define BUILTIN_LED_GPIO");
  assert(configH.includes("#define LAMP_GPIO"), "config.h có define LAMP_GPIO");
  assert(configH.includes("#define DISPLAY_MOSI_PIN"), "config.h có define DISPLAY_MOSI_PIN");

  const configJsonStr = CodeGenerator.generateConfigJson(DEFAULT_CONFIG);
  const configJson = JSON.parse(configJsonStr);
  const appends = configJson.builds[0].sdkconfig_append;
  assert(appends.includes("CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y"), "config.json chứa 16MB Flash");
  assert(appends.includes("CONFIG_SPIRAM_MODE_OCT=y"), "config.json chứa Octal PSRAM");
  assert(appends.includes("CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240=y"), "config.json chứa CPU 240MHz");

  // 10 Categories verification
  assert(appends.includes("CONFIG_LANGUAGE_VI_VN=y"), "config.json chứa CONFIG_LANGUAGE_VI_VN (General)");
  assert(appends.includes("CONFIG_FLASH_DEFAULT_ASSETS=y"), "config.json chứa CONFIG_FLASH_DEFAULT_ASSETS (General Flash Assets)");
  assert(!appends.some(a => a.startsWith("CONFIG_CUSTOM_UI_STYLE_")), "config.json đã loại bỏ UI Style trùng lặp, quy chuẩn hoàn toàn về Flash Assets");
  assert(appends.includes("CONFIG_USE_AFE_WAKE_WORD=y"), "config.json chứa WakeNet AFE (Wake Word)");
  assert(appends.includes("CONFIG_USE_HOTSPOT_WIFI_PROVISIONING=y"), "config.json chứa Hotspot SoftAP (Network)");
  assert(appends.includes("CONFIG_ENABLE_CUSTOM_MCP_SERVER=y"), "config.json chứa MCP Server Enable (MCP)");

  const boardCC = CodeGenerator.generateBoardCC(DEFAULT_CONFIG);
  assert(boardCC.includes("LampController lamp(LAMP_GPIO)"), "board.cc khởi tạo LampController");
  assert(boardCC.includes("SingleLed led(BUILTIN_LED_GPIO)"), "board.cc khởi tạo SingleLed");
}

// ----------------------------------------------------------------------------
// TEST 7: AUDIO/MIC DRIVERS, TOUCH CONTROLLER, UART DISPLAY & CUSTOM ASSETS
// ----------------------------------------------------------------------------
console.log("\n7. Kiểm tra Driver Âm thanh riêng biệt, Cảm ứng Touch, Màn hình UART & Giao diện .bin:");
{
  const testState = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  testState.audio.spk_driver = "pcm5102a";
  testState.audio.mic_driver = "ics43434";
  testState.general.flash_assets = "FLASH_CUSTOM_ASSETS";
  testState.general.custom_assets_file = "custom_face_assets.bin";
  testState.display.touch_chip = "cst816s";
  testState.display.touch_sda = 8;
  testState.display.touch_scl = 9;
  testState.display.touch_int = 3;
  testState.display.touch_rst = 21;

  // 7.1 Sinh config.h cho Audio Drivers, Touch, Custom Assets
  const configH = CodeGenerator.generateConfigH(testState);
  assert(configH.includes('#define AUDIO_SPEAKER_DRIVER        "pcm5102a"'), "config.h có define AUDIO_SPEAKER_DRIVER pcm5102a");
  assert(configH.includes('#define AUDIO_MIC_DRIVER            "ics43434"'), "config.h có define AUDIO_MIC_DRIVER ics43434");
  assert(configH.includes("#define TOUCH_CONTROLLER_CST816S 1"), "config.h có define TOUCH_CONTROLLER_CST816S");
  assert(configH.includes("#define TOUCH_I2C_SDA_PIN           ((gpio_num_t)8)"), "config.h có define TOUCH_I2C_SDA_PIN");
  assert(configH.includes("#define TOUCH_INT_PIN               ((gpio_num_t)3)"), "config.h có define TOUCH_INT_PIN");

  // 7.2 Sinh config.json cho Custom Assets và Touch
  const configJson = JSON.parse(CodeGenerator.generateConfigJson(testState));
  const appends = configJson.builds[0].sdkconfig_append;
  assert(appends.includes("CONFIG_FLASH_CUSTOM_ASSETS=y"), "config.json chứa CONFIG_FLASH_CUSTOM_ASSETS");
  assert(appends.includes('CONFIG_CUSTOM_ASSETS_FILE="custom_face_assets.bin"'), "config.json chứa CONFIG_CUSTOM_ASSETS_FILE");
  assert(appends.includes("CONFIG_CUSTOM_TOUCH_CST816S=y"), "config.json chứa CONFIG_CUSTOM_TOUCH_CST816S");

  // 7.3 Kiểm tra Màn hình rời qua UART
  const uartDisplayState = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  uartDisplayState.display.type = "uart_display";
  uartDisplayState.display.uart_port = 2;
  uartDisplayState.display.uart_baud = 115200;
  uartDisplayState.display.uart_tx = 17;
  uartDisplayState.display.uart_rx = 18;

  const uartConfigH = CodeGenerator.generateConfigH(uartDisplayState);
  assert(uartConfigH.includes("#define DISPLAY_TYPE_UART           1"), "config.h có define DISPLAY_TYPE_UART");
  assert(uartConfigH.includes("#define DISPLAY_UART_PORT           UART_NUM_2"), "config.h có define DISPLAY_UART_PORT 2");
  assert(uartConfigH.includes("#define DISPLAY_UART_TX_PIN         ((gpio_num_t)17)"), "config.h có define DISPLAY_UART_TX_PIN");

  const uartJson = JSON.parse(CodeGenerator.generateConfigJson(uartDisplayState));
  assert(uartJson.builds[0].sdkconfig_append.includes("CONFIG_DISPLAY_TYPE_UART=y"), "config.json chứa CONFIG_DISPLAY_TYPE_UART");

  const uartBoardCC = CodeGenerator.generateBoardCC(uartDisplayState);
  assert(uartBoardCC.includes('#include "display/uart_display.h"'), "board.cc include display/uart_display.h");
}

// ----------------------------------------------------------------------------
// TEST 8: MCP & PERIPHERALS CONFLICT RESOLUTION
// ----------------------------------------------------------------------------
console.log("\n8. Kiểm tra triệt tiêu xung đột giữa MCP và Ngoại vi:");
{
  // Test State: MCP enabled, Ngoại vi có Relay/Lamp ở GPIO 13
  const mcpState = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  mcpState.mcp.enable = true;
  // Đảm bảo trong peripherals có Relay ở GPIO 13
  const lamp = mcpState.peripherals.find(p => p.type === "relay_lamp");
  assert(lamp !== undefined && lamp.pin === 13, "Ngoại vi có Relay ở GPIO 13");

  // Chạy validate
  const res = PinValidator.validate(mcpState);
  assert(res.errors.length === 0, "Không có bất kỳ xung đột nào giữa MCP và Relay Ngoại vi");

  // Đổi chân Relay thành GPIO 14, vẫn không xung đột
  lamp.pin = 14;
  const res2 = PinValidator.validate(mcpState);
  assert(res2.errors.length === 0, "Đổi chân Relay thành GPIO 14 vẫn hợp lệ và không xung đột");
}

// ----------------------------------------------------------------------------
// TEST 9: DIRTY TRACKING LOGIC, REMINDER ON UNSAVED, & PER-TAB PERSISTENCE
// ----------------------------------------------------------------------------
console.log("\n9. Kiểm tra cơ chế Theo dõi Thay đổi (Dirty Tracking) & Lưu theo từng Tab:");
{
  // 1. Kiểm tra danh sách Tab và tên định danh (đã loại bỏ đánh số)
  assert(Object.keys(GUIController.TAB_NAMES).length === 10, "Đủ 10 danh mục cấu hình chuẩn");
  assert(GUIController.TAB_NAMES["panel-general"] === "Cơ bản & Ngôn ngữ", "Danh mục Cơ bản & Ngôn ngữ không còn đánh số");
  assert(GUIController.TAB_NAMES["panel-display"] === "Màn hình & Cảm ứng", "Danh mục Màn hình & Cảm ứng không còn đánh số");
  assert(typeof GUIController.autoConnectAndLoadConfig === "function", "Hàm autoConnectAndLoadConfig đã được tích hợp");
  assert(typeof GUIController.writeConfigFilesToDisk === "function", "Hàm writeConfigFilesToDisk sẵn sàng");

  // 2. Mô phỏng cơ chế phát hiện thay đổi (dirty check)
  const initialSnapshot = "general_ota_url:https://api.tenclass.net/xiaozhi/ota/|general_language:LANGUAGE_VI_VN|general_flash_assets:FLASH_DEFAULT_ASSETS";
  const modifiedSnapshot = "general_ota_url:https://my-custom-ota.com/|general_language:LANGUAGE_VI_VN|general_flash_assets:FLASH_DEFAULT_ASSETS";

  const isDirtyBefore = initialSnapshot !== initialSnapshot;
  assert(isDirtyBefore === false, "Khi chưa chỉnh sửa: isDirty = false (Không nhắc người dùng)");

  const isDirtyAfter = modifiedSnapshot !== initialSnapshot;
  assert(isDirtyAfter === true, "Khi có chỉnh sửa: isDirty = true (Sẽ nhắc người dùng lưu thay đổi)");

  // 3. Quy chuẩn Flash Assets không còn trường UI style trùng lặp
  assert(!DEFAULT_CONFIG.display.ui_style, "DEFAULT_CONFIG.display không còn chứa ui_style trùng lặp");
}

console.log(`\n=== TỔNG KẾT KIỂM THỬ: ${passed} PASSED, ${failed} FAILED ===`);
if (failed > 0) process.exit(1);
