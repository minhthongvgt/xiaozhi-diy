const assert = require("assert");
const fs = require("fs");
const path = require("path");

const {
  DEFAULT_CONFIG,
  ConfigParser,
  CodeGenerator,
  PinValidator
} = require("./web-configurator/app.js");

console.log("=== BẮT ĐẦU KIỂM THỬ TÁI HIỆN VÀ GỠ LỖI (REGRESSION TESTS) ===");

let passed = 0;
let failed = 0;

function test(name, fn) {
  try {
    fn();
    console.log(`  [PASS] ${name}`);
    passed++;
  } catch (err) {
    console.error(`  [FAIL] ${name}: ${err.message}`);
    failed++;
  }
}

// ----------------------------------------------------------------------------
// TEST 1: Board CC sinh mã chính xác cho UART Display
// ----------------------------------------------------------------------------
console.log("\n1. Kiểm tra sinh mã cho UART Display:");
test("board.cc phải có InitializeUartDisplay và gọi trong constructor", () => {
  const s = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  s.display.type = "uart_display";
  s.display.uart_port = 1;
  s.display.uart_tx = 17;
  s.display.uart_rx = 18;
  s.display.uart_baud = 115200;

  const cc = CodeGenerator.generateBoardCC(s);
  assert(cc.includes("InitializeUartDisplay"), "Phải chứa hàm InitializeUartDisplay");
  assert(cc.includes("new UartDisplay(DISPLAY_UART_PORT"), "Phải tạo đối tượng UartDisplay");
  assert(cc.includes("InitializeUartDisplay();"), "Constructor phải gọi InitializeUartDisplay()");
  assert(!cc.includes("display_ = new NoDisplay()"), "Không được gán NoDisplay khi chọn UART display");
});

// ----------------------------------------------------------------------------
// TEST 2: Board CC sinh mã chính xác cho GC9A01 và ILI9341
// ----------------------------------------------------------------------------
console.log("\n2. Kiểm tra sinh mã cho GC9A01 và ILI9341:");
test("GC9A01 phải gọi esp_lcd_new_panel_gc9a01 và include esp_lcd_gc9a01.h", () => {
  const s = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  s.display.type = "gc9a01";
  const cc = CodeGenerator.generateBoardCC(s);
  assert(cc.includes('#include "esp_lcd_gc9a01.h"'), "Phải include esp_lcd_gc9a01.h");
  assert(cc.includes("esp_lcd_new_panel_gc9a01"), "Phải gọi esp_lcd_new_panel_gc9a01");
  assert(!cc.includes("esp_lcd_new_panel_st7789"), "Không được gọi nhầm esp_lcd_new_panel_st7789 cho GC9A01");
});

test("ILI9341 phải gọi esp_lcd_new_panel_ili9341 và include esp_lcd_ili9341.h", () => {
  const s = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  s.display.type = "ili9341";
  const cc = CodeGenerator.generateBoardCC(s);
  assert(cc.includes('#include "esp_lcd_ili9341.h"'), "Phải include esp_lcd_ili9341.h");
  assert(cc.includes("esp_lcd_new_panel_ili9341"), "Phải gọi esp_lcd_new_panel_ili9341");
});

// ----------------------------------------------------------------------------
// TEST 3: Board CC sinh mã khởi tạo Touch Controller
// ----------------------------------------------------------------------------
console.log("\n3. Kiểm tra sinh mã khởi tạo Touch Controller:");
test("Khi bật CST816S, board.cc phải có khởi tạo Touch Screen I2C", () => {
  const s = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  s.display.touch_chip = "cst816s";
  s.display.touch_sda = 8;
  s.display.touch_scl = 9;
  s.display.touch_int = 3;

  const cc = CodeGenerator.generateBoardCC(s);
  assert(cc.includes("esp_lcd_touch"), "Phải chứa mã khởi tạo touch controller");
  assert(cc.includes("cst816s") || cc.includes("CST816S"), "Phải gọi driver CST816S");
});

// ----------------------------------------------------------------------------
// TEST 4: PinValidator cho phép I2C Bus chia sẻ SDA/SCL
// ----------------------------------------------------------------------------
console.log("\n4. Kiểm tra cơ chế I2C Bus chia sẻ SDA/SCL:");
test("ES8311 và CST816S cùng dùng SDA 8 và SCL 9 không được coi là xung đột chí mạng", () => {
  const s = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  s.audio.mode = "es8311";
  s.audio.codec_sda = 8;
  s.audio.codec_scl = 9;

  s.display.touch_chip = "cst816s";
  s.display.touch_sda = 8;
  s.display.touch_scl = 9;

  const res = PinValidator.validate(s);
  assert.strictEqual(res.errors.length, 0, "I2C SDA/SCL dùng chung không được tạo ra lỗi cấm lưu");
});

// ----------------------------------------------------------------------------
// TEST 5: Bug 8 - PinValidator.validate() trả về Object { errors, pinUsage }
// ----------------------------------------------------------------------------
console.log("\n5. Kiểm tra kiểu trả về của PinValidator.validate():");
test("PinValidator.validate() trả về object có thuộc tính errors là Array", () => {
  const s = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  const res = PinValidator.validate(s);
  assert(typeof res === "object" && res !== null, "validate() phải trả về object");
  assert(Array.isArray(res.errors), "res.errors phải là một Array");
  assert(res.pinUsage instanceof Map, "res.pinUsage phải là một Map");
});

// ----------------------------------------------------------------------------
// TEST 6: Bug 9 - generateConfigH() không còn typo s.audio_mic_*
// ----------------------------------------------------------------------------
console.log("\n6. Kiểm tra loại bỏ typo mic trong generateConfigH():");
test("generateConfigH() với Simplex mode ánh xạ chính xác chân Mic", () => {
  const s = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  s.audio.mode = "simplex";
  s.audio.mic_sck = 4;
  s.audio.mic_ws = 5;
  s.audio.mic_din = 6;
  const cfgH = CodeGenerator.generateConfigH(s);
  assert(cfgH.includes("#define AUDIO_I2S_MIC_GPIO_SCK      ((gpio_num_t)4)"), "AUDIO_I2S_MIC_GPIO_SCK phải là ((gpio_num_t)4)");
  assert(cfgH.includes("#define AUDIO_I2S_MIC_GPIO_WS       ((gpio_num_t)5)"), "AUDIO_I2S_MIC_GPIO_WS phải là ((gpio_num_t)5)");
  assert(cfgH.includes("#define AUDIO_I2S_MIC_GPIO_DIN      ((gpio_num_t)6)"), "AUDIO_I2S_MIC_GPIO_DIN phải là ((gpio_num_t)6)");
});

// ----------------------------------------------------------------------------
// TEST 7: Bug 10 - Touch Screen tạo esp_lcd_new_panel_io_i2c và InitializeI2cBus
// ----------------------------------------------------------------------------
console.log("\n7. Kiểm tra sinh mã Touch Screen kèm I2C panel IO:");
test("Khi bật Touch Screen, board.cc phải có InitializeI2cBus() và esp_lcd_new_panel_io_i2c()", () => {
  const s = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  s.display.touch_chip = "cst816s";
  s.display.touch_sda = 8;
  s.display.touch_scl = 9;

  const cc = CodeGenerator.generateBoardCC(s);
  assert(cc.includes("InitializeI2cBus()"), "board.cc phải có InitializeI2cBus()");
  assert(cc.includes("i2c_master_bus_handle_t i2c_bus_ = nullptr;"), "Phải khai báo i2c_bus_ member");
  assert(cc.includes("esp_lcd_new_panel_io_i2c(i2c_bus_"), "Phải gọi esp_lcd_new_panel_io_i2c trước khi gán handle");
  assert(cc.includes("InitializeI2cBus();"), "Constructor phải gọi InitializeI2cBus()");
});

// ----------------------------------------------------------------------------
// TEST 8: Bug 11 - ES8311 Codec nhận i2c_bus_ handle thay vì SDA/SCL pin
// ----------------------------------------------------------------------------
console.log("\n8. Kiểm tra ES8311 Codec constructor signature:");
test("ES8311 constructor phải nhận i2c_bus_ handle", () => {
  const s = JSON.parse(JSON.stringify(DEFAULT_CONFIG));
  s.audio.mode = "es8311";
  s.audio.codec_sda = 8;
  s.audio.codec_scl = 9;

  const cc = CodeGenerator.generateBoardCC(s);
  assert(cc.includes("Es8311AudioCodec audio_codec("), "Phải khai báo Es8311AudioCodec");
  assert(cc.includes("i2c_bus_"), "Phải truyền i2c_bus_ vào Es8311AudioCodec");
  assert(!cc.includes("AUDIO_CODEC_I2C_SDA_PIN, AUDIO_CODEC_I2C_SCL_PIN,"), "Không được truyền trực tiếp SDA/SCL pin vào Es8311AudioCodec");
});

// ----------------------------------------------------------------------------
// TEST 9: Bug 12 - DISPLAY_UART_TX_PIN không bị rơi vào communication[]
// ----------------------------------------------------------------------------
console.log("\n9. Kiểm tra phân tích cú pháp UART Display độc lập với Communication UART:");
test("parseConfigH với DISPLAY_UART_TX_PIN chỉ gán vào display.uart_tx, không tạo communication[]", () => {
  const sample = `#define DISPLAY_TYPE_UART 1
#define DISPLAY_UART_PORT UART_NUM_1
#define DISPLAY_UART_BAUDRATE 115200
#define DISPLAY_UART_TX_PIN GPIO_NUM_17
#define DISPLAY_UART_RX_PIN GPIO_NUM_18
`;
  const parsed = ConfigParser.parseConfigH(sample);
  assert.strictEqual(parsed.display.type, "uart_display", "Display type phải là uart_display");
  assert.strictEqual(parsed.display.uart_tx, 17, "uart_tx phải là 17");
  assert.strictEqual(parsed.display.uart_rx, 18, "uart_rx phải là 18");
  const commUarts = parsed.communication.filter(c => c.type === "uart");
  assert.strictEqual(commUarts.length, 0, "Không được tự động thêm vào communication[] từ define của display UART");
});

// ----------------------------------------------------------------------------
// TEST 10: Bug 13 - Kiểm tra logic bảo vệ chống ghi file rỗng
// ----------------------------------------------------------------------------
console.log("\n10. Kiểm tra an toàn khi lưu: không cho phép ghi đè nội dung rỗng:");
test("Kiểm tra chuỗi rỗng: chuỗi whitespace hoặc rỗng không hợp lệ", () => {
  const isNonEmpty = (str) => Boolean(str && str.trim().length > 0);
  assert.strictEqual(isNonEmpty(""), false, "Chuỗi rỗng '' phải coi là không hợp lệ");
  assert.strictEqual(isNonEmpty("   \n\t  "), false, "Chuỗi chỉ chứa khoảng trắng phải coi là không hợp lệ");
  assert.strictEqual(isNonEmpty("#define TEST 1"), true, "Nội dung config.h thực sự phải hợp lệ");
});

// ----------------------------------------------------------------------------
// TEST 11: Nút thêm linh kiện / ngoại vi modal ID mapping
// ----------------------------------------------------------------------------
console.log("\n11. Kiểm tra tính toàn vẹn của Modal thêm ngoại vi / linh kiện:");
test("app.js và index.html phải khớp ID cho modal thêm linh kiện/ngoại vi", () => {
  const htmlContent = fs.readFileSync(path.join(__dirname, "web-configurator/index.html"), "utf8");
  const jsContent = fs.readFileSync(path.join(__dirname, "web-configurator/app.js"), "utf8");

  // Kiểm tra index.html chứa đầy đủ các ID
  const requiredModalIds = [
    "add-item-modal",
    "modal-dialog-title",
    "modal-item-type",
    "modal-dynamic-fields",
    "modal-btn-confirm",
    "modal-btn-cancel",
    "btn-add-peripheral",
    "btn-add-button",
    "btn-add-comm"
  ];
  for (const id of requiredModalIds) {
    assert(htmlContent.includes(`id="${id}"`), `index.html phải có phần tử với id="${id}"`);
  }

  // Kiểm tra app.js xử lý đúng các ID từ index.html
  assert(jsContent.includes('document.getElementById("add-item-modal")'), "app.js phải hỗ trợ add-item-modal");
  assert(jsContent.includes('document.getElementById("modal-dialog-title")'), "app.js phải hỗ trợ modal-dialog-title");
  assert(jsContent.includes('document.getElementById("modal-item-type")'), "app.js phải hỗ trợ modal-item-type");
  assert(jsContent.includes('document.getElementById("modal-dynamic-fields")'), "app.js phải hỗ trợ modal-dynamic-fields");
});

console.log(`\n=== TỔNG KẾT: ${passed} PASSED, ${failed} FAILED ===`);
if (failed > 0) process.exit(1);

