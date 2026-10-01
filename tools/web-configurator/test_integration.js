/**
 * Integration Test: Test against real Xiaozhi Repository files and Sdkconfig Bridge
 */

const fs = require('fs');
const path = require('path');
const { SdkconfigParser, SdkconfigGenerator } = require('./app.js');

console.log("=== KIỂM THỬ TÍCH HỢP VỚI TỆP THỰC TẾ CỦA XIAOZHI-ESP32 ===");

const projectRoot = path.resolve(__dirname, '../../');
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

// 1. Kiểm tra đọc và phân tích cú pháp tệp sdkconfig.defaults và sdkconfig.defaults.esp32s3
const sdkDefaultsPath = path.join(projectRoot, 'sdkconfig.defaults');
const sdkS3DefaultsPath = path.join(projectRoot, 'sdkconfig.defaults.esp32s3');

console.log("\n1. Đọc và phân tích cú pháp tệp sdkconfig.defaults:");
if (fs.existsSync(sdkDefaultsPath)) {
  const content = fs.readFileSync(sdkDefaultsPath, 'utf8');
  const state = SdkconfigParser.parse(content);
  assert(state && typeof state === 'object', "Phân tích cú pháp tệp sdkconfig.defaults thành công");
  assert(state.system.flash_size === "16MB", `Trích xuất dung lượng Flash: ${state.system.flash_size}`);
  assert(state.system.partition_table.includes("partitions/16m.csv"), `Trích xuất phân vùng: ${state.system.partition_table}`);
} else {
  assert(false, "Không tìm thấy tệp sdkconfig.defaults");
}

console.log("\n2. Đọc và phân tích cú pháp tệp sdkconfig.defaults.esp32s3:");
if (fs.existsSync(sdkS3DefaultsPath)) {
  const contentS3 = fs.readFileSync(sdkS3DefaultsPath, 'utf8');
  const stateS3 = SdkconfigParser.parse(contentS3);
  assert(stateS3.system.spiram === true, "Trích xuất kích hoạt PSRAM = true (CONFIG_SPIRAM=y)");
  assert(stateS3.system.spiram_mode === "OCT", "Trích xuất chế độ Octal PSRAM (CONFIG_SPIRAM_MODE_OCT=y)");
  assert(stateS3.system.cpu_freq === "240", "Trích xuất xung nhịp CPU 240MHz");
} else {
  assert(false, "Không tìm thấy tệp sdkconfig.defaults.esp32s3");
}

// 3. Kiểm tra menu Kconfig.projbuild trong main
const kconfigProjPath = path.join(projectRoot, 'main/Kconfig.projbuild');
console.log("\n3. Kiểm tra menu Kconfig.projbuild trong main:");
if (fs.existsSync(kconfigProjPath)) {
  const kconfigContent = fs.readFileSync(kconfigProjPath, 'utf8');
  assert(kconfigContent.includes('menu "Xiaozhi Assistant"'), "Tìm thấy menu chính 'Xiaozhi Assistant'");
  assert(kconfigContent.includes('choice BOARD_TYPE'), "Tìm thấy menu chọn loại bo mạch 'choice BOARD_TYPE'");
  assert(kconfigContent.includes('config BOARD_TYPE_ESP32_S3_N16R8_CUSTOM'), "Tìm thấy cấu hình bo mạch 'BOARD_TYPE_ESP32_S3_N16R8_CUSTOM'");
  assert(kconfigContent.includes('config LANGUAGE_VI_VN'), "Tìm thấy tùy chọn ngôn ngữ Tiếng Việt 'LANGUAGE_VI_VN'");
} else {
  assert(false, "Không tìm thấy tệp main/Kconfig.projbuild");
}

// 4. Kiểm tra tệp bảng phân vùng Flash (partitions/16m.csv)
const partitionPath = path.join(projectRoot, 'partitions', '16m.csv');
console.log("\n4. Kiểm tra tệp bảng phân vùng Flash (partitions/16m.csv):");
if (fs.existsSync(partitionPath)) {
  const partContent = fs.readFileSync(partitionPath, 'utf8');
  assert(partContent.includes('app0') && partContent.includes('app1'), "Bảng phân vùng hỗ trợ 2 phân vùng OTA (app0, app1)");
  assert(partContent.includes('nvs'), "Bảng phân vùng có vùng lưu trữ NVS");
} else {
  assert(false, "Không tìm thấy tệp partitions/16m.csv");
}

// 5. Kiểm tra khả năng tạo lại cấu hình tương thích Kconfig
console.log("\n5. Kiểm tra khả năng tạo cấu hình tương thích Kconfig từ SdkconfigGenerator:");
{
  const stateS3 = SdkconfigParser.parse(fs.readFileSync(sdkS3DefaultsPath, 'utf8'));
  const lines = SdkconfigGenerator.build(stateS3);
  assert(lines.length > 50, `Sinh thành công ${lines.length} dòng cấu hình`);
  assert(lines.some(l => l.includes('CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM=y')), "Đã cấu hình bo mạch ESP32-S3-N16R8 Custom");
  assert(lines.some(l => l.includes('CONFIG_SPIRAM=y')), "Đã kích hoạt hỗ trợ PSRAM (CONFIG_SPIRAM=y)");
  assert(lines.some(l => l.includes('CONFIG_SPIRAM_MODE_OCT=y')), "Đã cấu hình Octal PSRAM (CONFIG_SPIRAM_MODE_OCT=y)");
}

console.log(`\n=== TỔNG KẾT TÍCH HỢP: ${passed} PASSED, ${failed} FAILED ===`);
if (failed > 0) process.exit(1);
