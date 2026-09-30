/**
 * Test: 100% Logic Parity between Web Configurator and Kconfig / menuconfig
 */

const { SdkconfigParser, SdkconfigGenerator } = require('./app.js');

console.log("=== KIỂM THỬ ĐỘNG VÀ TƯƠNG ĐỒNG 100% VỚI MENUCONFIG (KCONFIG) ===");

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

// 1. Kiểm tra Cascading Suppression khi module cha bị tắt (depends on)
console.log("\n1. Kiểm tra Cascading Suppression (Triệt tiêu biến con khi cha tắt):");
{
  const state = SdkconfigParser.parse("");
  state.display.enable = false;
  state.display.pin_mosi = 4;
  state.display.pin_clk = 5;
  const lines = SdkconfigGenerator.build(state);

  assert(lines.includes('# CONFIG_ENABLE_CUSTOM_DISPLAY is not set'), "Ghi nhận '# CONFIG_ENABLE_CUSTOM_DISPLAY is not set'");
  assert(!lines.some(l => l.startsWith('CONFIG_CUSTOM_DISPLAY_PIN_MOSI=')), "Không rò rỉ CONFIG_CUSTOM_DISPLAY_PIN_MOSI khi màn hình tắt");
  assert(!lines.some(l => l.startsWith('CONFIG_CUSTOM_DISPLAY_WIDTH=')), "Không rò rỉ CONFIG_CUSTOM_DISPLAY_WIDTH khi màn hình tắt");
}

// 2. Kiểm tra ràng buộc depends on cho Multiline Chat
console.log("\n2. Kiểm tra ràng buộc depends on cho Multiline Chat:");
{
  const state = SdkconfigParser.parse("");
  state.general.display_style = "USE_WECHAT_MESSAGE_STYLE";
  state.general.multiline_chat = true; // cố tình set true khi ở WeChat style
  const lines = SdkconfigGenerator.build(state);

  assert(lines.includes('# CONFIG_USE_MULTILINE_CHAT_MESSAGE is not set'), "Cưỡng chế '# CONFIG_USE_MULTILINE_CHAT_MESSAGE is not set' khi không ở default style");

  state.general.display_style = "USE_DEFAULT_MESSAGE_STYLE";
  state.general.multiline_chat = true;
  const lines2 = SdkconfigGenerator.build(state);
  assert(lines2.includes('CONFIG_USE_MULTILINE_CHAT_MESSAGE=y'), "Bật 'CONFIG_USE_MULTILINE_CHAT_MESSAGE=y' khi ở default style");
}

// 3. Kiểm tra cặp Choice Flag + Derived Value (Baudrate và UART Port)
console.log("\n3. Kiểm tra cặp Choice Flag + Derived Value:");
{
  const state = SdkconfigParser.parse("");
  state.display.enable = true;
  state.display.type = "CUSTOM_DISPLAY_UART";
  state.display.uart_baud = 115200;
  state.uart.enable = true;
  state.uart.port = "CUSTOM_UART_PORT_2";

  const lines = SdkconfigGenerator.build(state);

  assert(lines.includes('CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_115200=y'), "Sinh cờ choice: CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_115200=y");
  assert(lines.includes('CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE=115200'), "Sinh biến giá trị: CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE=115200");
  assert(lines.includes('CONFIG_CUSTOM_UART_PORT_2=y'), "Sinh cờ choice port: CONFIG_CUSTOM_UART_PORT_2=y");
  assert(lines.includes('CONFIG_CUSTOM_UART_PORT=2'), "Sinh biến giá trị port: CONFIG_CUSTOM_UART_PORT=2");
}

// 4. Kiểm tra cơ chế tự động kích hoạt phụ thuộc chéo (Kconfig select)
console.log("\n4. Kiểm tra cơ chế Kconfig select (Wi-Fi BluFi tự kích hoạt Bluetooth):");
{
  const state = SdkconfigParser.parse("");
  state.wifi.method = "USE_ESP_BLUFI_WIFI_PROVISIONING";
  const lines = SdkconfigGenerator.build(state);

  assert(lines.includes('CONFIG_BT_ENABLED=y'), "BluFi tự động kích hoạt CONFIG_BT_ENABLED=y");
  assert(lines.includes('CONFIG_BT_BLE_42_FEATURES_SUPPORTED=y'), "BluFi tự động kích hoạt CONFIG_BT_BLE_42_FEATURES_SUPPORTED=y");
  assert(lines.includes('CONFIG_BT_BLE_BLUFI_ENABLE=y'), "BluFi tự động kích hoạt CONFIG_BT_BLE_BLUFI_ENABLE=y");

  state.wifi.method = "USE_HOTSPOT_WIFI_PROVISIONING";
  const linesHotspot = SdkconfigGenerator.build(state);
  assert(linesHotspot.includes('# CONFIG_BT_ENABLED is not set'), "Hotspot tắt CONFIG_BT_ENABLED");
}

// 5. Kiểm tra ràng buộc AFE và Audio Processor
console.log("\n5. Kiểm tra ràng buộc AFE và Audio Processor:");
{
  const state = SdkconfigParser.parse("");
  state.system.spiram = true;
  state.wakeword.type = "USE_AFE_WAKE_WORD";
  state.wakeword.detection_in_listening = true;

  const lines = SdkconfigGenerator.build(state);
  assert(lines.includes('CONFIG_USE_AUDIO_PROCESSOR=y'), "Kích hoạt CONFIG_USE_AUDIO_PROCESSOR=y cho AFE");
  assert(lines.includes('CONFIG_WAKE_WORD_DETECTION_IN_LISTENING=y'), "Kích hoạt detection in listening khi có AFE");

  state.wakeword.type = "WAKE_WORD_DISABLED";
  const linesDisabled = SdkconfigGenerator.build(state);
  assert(linesDisabled.includes('# CONFIG_WAKE_WORD_DETECTION_IN_LISTENING is not set'), "Vô hiệu hóa detection in listening khi wakeword disabled");
}

// 6. Kiểm tra các tham số bắt buộc của ESP-IDF 6.1
console.log("\n6. Kiểm tra các tham số bắt buộc của ESP-IDF 6.1:");
{
  const state = SdkconfigParser.parse("");
  const lines = SdkconfigGenerator.build(state);

  assert(lines.includes('CONFIG_ESP_MAIN_TASK_STACK_SIZE=12288'), "Chứa CONFIG_ESP_MAIN_TASK_STACK_SIZE=12288 chống tràn stack");
  assert(lines.includes('CONFIG_TOUCH_SUPPRESS_DEPRECATE_WARN=y'), "Chứa CONFIG_TOUCH_SUPPRESS_DEPRECATE_WARN=y chống lỗi -Werror=cpp");
  assert(lines.includes('CONFIG_PARTITION_TABLE_MD5=y'), "Chứa CONFIG_PARTITION_TABLE_MD5=y kiểm tra phân vùng");
}

console.log(`\n=== TỔNG KẾT KCONFIG PARITY: ${passed} PASSED, ${failed} FAILED ===`);
if (failed > 0) process.exit(1);
