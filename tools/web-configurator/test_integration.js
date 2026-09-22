/**
 * Integration Test: Test against real Xiaozhi Repository files
 */

const fs = require('fs');
const path = require('path');
const { ConfigParser, CodeGenerator } = require('./app.js');

console.log("=== KIỂM THỬ TÍCH HỢP VỚI TỆP THỰC TẾ CỦA XIAOZHI-ESP32 ===");

const projectPath = fs.existsSync(path.resolve(__dirname, '../../main/Kconfig.projbuild'))
  ? path.resolve(__dirname, '../../')
  : path.resolve(__dirname, '../../../Xiaozhi/Oginal');
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

// 1. Test parsing real config.h from esp32s3-n16r8-custom
const realConfigH = path.join(projectPath, 'main/boards/esp32s3-n16r8-custom/config.h');
if (fs.existsSync(realConfigH)) {
  const content = fs.readFileSync(realConfigH, 'utf8');
  const parsed = ConfigParser.parseConfigH(content);
  console.log("\n1. Đọc tệp config.h thực tế từ esp32s3-n16r8-custom:");
  assert(parsed.display.mosi === 47, `Trích xuất DISPLAY_MOSI_PIN = 47 (nhận được: ${parsed.display.mosi})`);
  assert(parsed.display.clk === 21, `Trích xuất DISPLAY_CLK_PIN = 21 (nhận được: ${parsed.display.clk})`);
  assert(parsed.display.cs === 41, `Trích xuất DISPLAY_CS_PIN = 41 (nhận được: ${parsed.display.cs})`);
  assert(parsed.audio.spk_bclk === 15, `Trích xuất AUDIO_I2S_SPK_GPIO_BCLK = 15 (nhận được: ${parsed.audio.spk_bclk})`);
  assert(parsed.audio.spk_ws === 16, `Trích xuất AUDIO_I2S_SPK_GPIO_LRCK = 16 (nhận được: ${parsed.audio.spk_ws})`);
  assert(parsed.audio.spk_dout === 7, `Trích xuất AUDIO_I2S_SPK_GPIO_DOUT = 7 (nhận được: ${parsed.audio.spk_dout})`);
  const lamp = parsed.peripherals.find(p => p.type === "relay_lamp");
  assert(lamp && lamp.pin === 13, `Trích xuất LAMP_GPIO = 13 (nhận được: ${lamp ? lamp.pin : undefined})`);
} else {
  console.log("\n[SKIP] Không tìm thấy đường dẫn project để test trực tiếp.");
}

// 2. Test Kconfig injection simulation
const realKconfig = path.join(projectPath, 'main/Kconfig.projbuild');
if (fs.existsSync(realKconfig)) {
  console.log("\n2. Kiểm tra mô phỏng chèn Kconfig.projbuild:");
  let kconfig = fs.readFileSync(realKconfig, 'utf8');
  const entry = `\n    config BOARD_TYPE_CUSTOM_S3_N16R8\n        bool "Custom ESP32-S3-N16R8 Board"\n        depends on IDF_TARGET_ESP32S3\n`;
  const boardChoiceIdx = kconfig.indexOf("choice BOARD_TYPE");
  assert(boardChoiceIdx !== -1, "Tìm thấy 'choice BOARD_TYPE' trong Kconfig.projbuild");

  const endchoiceIdx = kconfig.indexOf("endchoice", boardChoiceIdx);
  assert(endchoiceIdx !== -1, "Tìm thấy 'endchoice' của choice BOARD_TYPE");

  const updatedKconfig = kconfig.slice(0, endchoiceIdx) + entry + kconfig.slice(endchoiceIdx);
  assert(updatedKconfig.includes("config BOARD_TYPE_CUSTOM_S3_N16R8"), "Đã chèn thành công entry vào Kconfig");
}

// 3. Test CMakeLists.txt injection simulation
const realCMake = path.join(projectPath, 'main/CMakeLists.txt');
if (fs.existsSync(realCMake)) {
  console.log("\n3. Kiểm tra mô phỏng chèn CMakeLists.txt:");
  let cmake = fs.readFileSync(realCMake, 'utf8');
  const cmakeSnippet = `\nif(CONFIG_BOARD_TYPE_CUSTOM_S3_N16R8)\n    set(BOARD_DIR "custom-s3-n16r8")\n    set(BUILTIN_TEXT_FONT font_noto_sans_basic_20_4)\n    set(BUILTIN_ICON_FONT font_material_symbols_20_4)\n    set(DEFAULT_EMOJI_COLLECTION noto-color-emoji_64)\n    set(EMOTE_RESOLUTION "320_240")\nelse`;
  assert(cmake.includes("if(CONFIG_BOARD_TYPE_"), "Tìm thấy 'if(CONFIG_BOARD_TYPE_' trong CMakeLists.txt");

  const updatedCMake = cmake.replace("if(CONFIG_BOARD_TYPE_", cmakeSnippet + "if(CONFIG_BOARD_TYPE_");
  assert(updatedCMake.includes("if(CONFIG_BOARD_TYPE_CUSTOM_S3_N16R8)"), "Đã chèn thành công nhánh build vào CMakeLists.txt");
}

console.log(`\n=== TỔNG KẾT TÍCH HỢP: ${passed} PASSED, ${failed} FAILED ===`);
if (failed > 0) process.exit(1);
