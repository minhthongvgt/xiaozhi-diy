/**
 * Test Runner for Xiaozhi Web Configurator Core Modules (Sdkconfig & GPIO Engine)
 */

const { SdkconfigParser, SdkconfigGenerator, PinValidator, DEFAULT_CONFIG, ESP32S3 } = require('./app.js');

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
  testState.speaker.enable = true;
  testState.speaker.pin_bclk = pin;
  const result = PinValidator.validate(testState);
  const hasOctalError = result.errors.some(e => e.message.includes("BỊ KHÓA") && e.pin === pin);
  assert(hasOctalError, `GPIO ${pin} phải bị cấm và báo lỗi Octal SPI Flash/PSRAM`);
}

// ----------------------------------------------------------------------------
// TEST 3: CONFLICT DETECTION BETWEEN PERIPHERALS
// ----------------------------------------------------------------------------
console.log("\n3. Kiểm tra phát hiện xung đột chân cho các module:");
{
  // Test A: DHT Sensor trùng chân Speaker BCLK (GPIO 15)
  const conflictState1 = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  conflictState1.speaker.enable = true;
  conflictState1.speaker.pin_bclk = 15;
  conflictState1.sensors.dht_enable = true;
  conflictState1.sensors.dht_gpio = 15;
  const res1 = PinValidator.validate(conflictState1);
  assert(res1.errors.some(e => e.message.includes("Xung đột GPIO 15")), "Phát hiện xung đột khi DHT Sensor trùng Speaker BCLK (GPIO 15)");

  // Test B: Nút nhấn BOOT (GPIO 0) trùng với LED đơn
  const conflictState2 = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  conflictState2.buttons.boot_enable = true;
  conflictState2.buttons.boot_gpio = 0;
  conflictState2.led.enable = true;
  conflictState2.led.gpio = 0;
  const res2 = PinValidator.validate(conflictState2);
  assert(res2.errors.some(e => e.message.includes("Xung đột GPIO 0")), "Phát hiện xung đột khi Nút BOOT trùng chân LED (GPIO 0)");

  // Test C: Servo PWM trùng với Relay GPIO
  const conflictState3 = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  conflictState3.actuators.servo_enable = true;
  conflictState3.actuators.servo_gpio = 48;
  conflictState3.motor.relay_enable = true;
  conflictState3.motor.relay_gpio = 48;
  const res3 = PinValidator.validate(conflictState3);
  assert(res3.errors.some(e => e.message.includes("Xung đột GPIO 48")), "Phát hiện xung đột khi Servo trùng chân Relay (GPIO 48)");
}

// ----------------------------------------------------------------------------
// TEST 4: I2C BUS SHARING IS ALLOWED
// ----------------------------------------------------------------------------
console.log("\n4. Kiểm tra chia sẻ I2C Bus giữa nhiều thiết bị (hợp lệ):");
{
  const i2cState = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  i2cState.display.enable = true;
  i2cState.display.pin_i2c_sda = 8;
  i2cState.display.pin_i2c_scl = 9;

  i2cState.touch.enable = true;
  i2cState.touch.pin_sda = 8;
  i2cState.touch.pin_scl = 9;

  i2cState.speaker.enable = true;
  i2cState.speaker.codec_sda = 8;
  i2cState.speaker.codec_scl = 9;

  const res = PinValidator.validate(i2cState);
  const hasI2cConflict = res.errors.some(e => e.pin === 8 || e.pin === 9);
  assert(!hasI2cConflict, "Các thiết bị cùng bus I2C (SDA=8, SCL=9) không bị báo lỗi xung đột");
}

// ----------------------------------------------------------------------------
// TEST 5: SDKCONFIG GENERATOR & PARSER ROUNDTRIP
// ----------------------------------------------------------------------------
console.log("\n5. Kiểm tra tính toàn vẹn SdkconfigGenerator và SdkconfigParser:");
{
  const testState = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  testState.general.language = "LANGUAGE_VI_VN";
  testState.system.flash_size = "16MB";
  testState.system.spiram_mode = "OCT";
  testState.speaker.enable = true;
  testState.speaker.pin_dout = 7;
  testState.speaker.pin_bclk = 15;
  testState.speaker.pin_lrck = 16;
  testState.sensors.dht_enable = true;
  testState.sensors.dht_gpio = 12;

  // Generate lines
  const lines = SdkconfigGenerator.build(testState);
  assert(lines.length > 50, `Sinh được ${lines.length} dòng cấu hình Kconfig`);
  assert(lines.includes("CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM=y"), "Chứa CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM=y");
  assert(lines.includes("CONFIG_SPIRAM_MODE_OCT=y"), "Chứa CONFIG_SPIRAM_MODE_OCT=y");
  assert(lines.includes("CONFIG_LANGUAGE_VI_VN=y"), "Chứa CONFIG_LANGUAGE_VI_VN=y");
  assert(lines.includes("CONFIG_CUSTOM_AUDIO_SPK_GPIO_DOUT=7"), "Chứa CONFIG_CUSTOM_AUDIO_SPK_GPIO_DOUT=7");
  assert(lines.includes("CONFIG_CUSTOM_SENSOR_DHT_GPIO=12"), "Chứa CONFIG_CUSTOM_SENSOR_DHT_GPIO=12");

  // Parse text back to state
  const rawText = lines.join("\n");
  const parsedState = SdkconfigParser.parse(rawText);
  assert(parsedState.general.language === "LANGUAGE_VI_VN", "Parse lại ngôn ngữ chính xác (LANGUAGE_VI_VN)");
  assert(parsedState.system.spiram_mode === "OCT", "Parse lại PSRAM mode chính xác (OCT)");
  assert(parsedState.speaker.pin_dout === 7, "Parse lại Speaker DOUT chính xác (7)");
  assert(parsedState.sensors.dht_gpio === 12, "Parse lại DHT GPIO chính xác (12)");
}

// ----------------------------------------------------------------------------
// TEST 6: INVALID NUMERIC VALUES FALL BACK TO DEFAULTS
// ----------------------------------------------------------------------------
console.log("\n6. Kiểm tra fallback khi giá trị số trong sdkconfig không hợp lệ:");
{
  const parsed = SdkconfigParser.parse("CONFIG_CUSTOM_DISPLAY_WIDTH=not-a-number");
  assert(parsed.display.width === 240, "Giá trị số không hợp lệ phải quay về mặc định 240");
}

console.log(`\n=== TỔNG KẾT ENGINE TEST: ${passed} PASS, ${failed} FAIL ===`);
if (failed > 0) process.exit(1);
