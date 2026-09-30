/**
 * DOM Validation Test Suite for Xiaozhi Web Configurator (15 Panels Modular Architecture)
 */

const fs = require('fs');
const path = require('path');

const htmlPath = path.resolve(__dirname, 'index.html');
const html = fs.readFileSync(htmlPath, 'utf8');

console.log("=== BẮT ĐẦU KIỂM THỬ DOM & CẤU TRÚC GIAO DIỆN (15 TABS) ===");

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

// 1. Kiểm tra không còn nút lưu toàn cục trên header
assert(!html.includes('id="btn-save-project"'), 'Global save button đã được loại bỏ khỏi header');

// 2. Danh sách 16 panels chuẩn mực
const tabs = [
  'panel-general',
  'panel-audio',
  'panel-display',
  'panel-wakeword',
  'panel-network',
  'panel-mcp',
  'panel-buttons',
  'panel-camera',
  'panel-led',
  'panel-uart',
  'panel-actuators',
  'panel-motor',
  'panel-sensors',
  'panel-power',
  'panel-flash',
  'panel-build'
];

// Kiểm tra mỗi panel đều có button lưu riêng và pill trạng thái
tabs.forEach(tab => {
  assert(html.includes(`data-tab="${tab}"`), `Tab [${tab}] có nút lưu độc lập data-tab`);
  assert(html.includes(`data-tab-status="${tab}"`), `Tab [${tab}] có pill trạng thái data-tab-status`);
  assert(html.includes(`id="${tab}"`), `Tab [${tab}] có phần tử section id tương ứng`);
  assert(html.includes(`data-target="${tab}"`), `Tab [${tab}] có nút chuyển tab trên sidebar`);
});

// 3. Kiểm tra Modal cảnh báo chưa lưu, Project Modal và Toast Container
assert(html.includes('id="unsaved-modal"'), 'Modal xác nhận thay đổi chưa lưu (unsaved-modal) hiện diện');
assert(html.includes('id="toast-container"'), 'Container thông báo nổi (toast-container) hiện diện');
assert(html.includes('id="success-modal"'), 'Modal thông báo lưu thành công và lệnh nạp (success-modal) hiện diện');
assert(html.includes('id="project-modal"'), 'Modal quản lý thư mục dự án (project-modal) hiện diện');
assert(html.includes('id="input-project-root"'), 'Ô nhập đường dẫn dự án (input-project-root) hiện diện');
assert(html.includes('id="btn-apply-project-root"'), 'Nút Áp dụng đường dẫn dự án (btn-apply-project-root) hiện diện');
assert(html.includes('id="btn-reset-project-root"'), 'Nút Tự động nhận diện dự án (btn-reset-project-root) hiện diện');
assert(html.includes('id="project-detected-list"'), 'Khung danh sách dự án tìm thấy (project-detected-list) hiện diện');

// 4. Kiểm tra không có đánh số cứng trong sidebar và tiêu đề H2
const forbiddenNumbers = ['1. ', '2. ', '3. ', '4. ', '5. ', '6. ', '7. ', '8. ', '9. ', '10. '];
let hasNumberedSidebar = false;
forbiddenNumbers.forEach(num => {
  if (html.includes(`<span class="tab-label">${num}`)) hasNumberedSidebar = true;
});
assert(!hasNumberedSidebar, 'Sidebar tab-labels không bị đánh số thứ tự cứng');

let hasNumberedH2 = false;
forbiddenNumbers.forEach(num => {
  if (html.includes(`<h2>🌐 ${num}`) || html.includes(`<h2>🔊 ${num}`) || html.includes(`<h2>🖥️ ${num}`) ||
      html.includes(`<h2>🎙️ ${num}`) || html.includes(`<h2>📶 ${num}`) || html.includes(`<h2>🤖 ${num}`) ||
      html.includes(`<h2>🔘 ${num}`)) {
    hasNumberedH2 = true;
  }
});
assert(!hasNumberedH2, 'Tiêu đề h2 các danh mục không bị đánh số thứ tự cứng');

// 5. Kiểm tra các dropdown Flash & Partition
assert(html.includes('id="flash_size"'), 'Dropdown id="flash_size" hiện diện');
assert(html.includes('id="sys_spiram_mode"'), 'Dropdown id="sys_spiram_mode" hiện diện');
assert(html.includes('id="sys_partition_table"'), 'Dropdown id="sys_partition_table" hiện diện');
assert(html.includes('id="sys_cpu_freq"'), 'Dropdown id="sys_cpu_freq" hiện diện');

// 6. Kiểm tra Widget Sơ đồ chân GPIO Matrix
assert(html.includes('id="pin-matrix-grid"'), 'Widget ma trận chân GPIO (pin-matrix-grid) hiện diện');
assert(html.includes('id="conflict-alert-bar"'), 'Thanh cảnh báo xung đột chân (conflict-alert-bar) hiện diện');
assert(html.includes('id="conflict-message"'), 'Khung thông báo lỗi xung đột (conflict-message) hiện diện');

// 7. Kiểm tra Panel 16: Build & Flash Controls
assert(html.includes('id="btn-idf-build"'), 'Nút Biên dịch (btn-idf-build) hiện diện');
assert(html.includes('id="btn-idf-flash"'), 'Nút Nạp chip (btn-idf-flash) hiện diện');
assert(html.includes('id="btn-idf-build-flash"'), 'Nút Biên dịch & Nạp ngay (btn-idf-build-flash) hiện diện');
assert(html.includes('id="serial-port-select"'), 'Dropdown chọn cổng COM (serial-port-select) hiện diện');
assert(html.includes('id="serial-baud-select"'), 'Dropdown chọn tốc độ Baudrate (serial-baud-select) hiện diện');
assert(html.includes('id="terminal-body"'), 'Cửa sổ dòng lệnh Terminal trực tiếp (terminal-body) hiện diện');
assert(html.includes('id="idf-status-badge"'), 'Huy hiệu trạng thái ESP-IDF (idf-status-badge) hiện diện');
assert(html.includes('id="target-status-badge"'), 'Huy hiệu trạng thái Target (target-status-badge) hiện diện');
assert(html.includes('id="btn-toggle-idf-custom"'), 'Nút Tùy chỉnh / Dò tìm IDF (btn-toggle-idf-custom) hiện diện');
assert(html.includes('id="custom-idf-input"'), 'Ô nhập đường dẫn IDF tùy chỉnh (custom-idf-input) hiện diện');
assert(html.includes('id="btn-apply-idf-path"'), 'Nút Áp dụng đường dẫn IDF (btn-apply-idf-path) hiện diện');
assert(html.includes('id="btn-scan-all-idf"'), 'Nút Quét toàn bộ ổ đĩa (btn-scan-all-idf) hiện diện');
assert(html.includes('id="btn-reset-idf-path"'), 'Nút Khôi phục mặc định IDF (btn-reset-idf-path) hiện diện');
assert(html.includes('id="idf-scan-results"'), 'Khung kết quả quét ổ đĩa (idf-scan-results) hiện diện');
assert(html.includes('id="idf-custom-tag"'), 'Nhãn trạng thái Tùy chỉnh (idf-custom-tag) hiện diện');

console.log(`\n=== TỔNG KẾT: ${passed} PASS, ${failed} FAIL ===`);
if (failed > 0) process.exit(1);

