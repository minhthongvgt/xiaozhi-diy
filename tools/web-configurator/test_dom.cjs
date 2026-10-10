const fs = require('fs');
const path = require('path');
const htmlPath = path.resolve(__dirname, 'index.html');
let html = fs.readFileSync(htmlPath, 'utf8');
const componentsDir = path.resolve(__dirname, 'components');
if (fs.existsSync(componentsDir)) {
  const files = fs.readdirSync(componentsDir);
  for (const file of files) {
    if (file.endsWith('.html')) {
      html += fs.readFileSync(path.join(componentsDir, file), 'utf8');
    }
  }
}
console.log("=== BẮT ĐẦU KIỂM THỬ DOM & CẤU TRÚC GIAO DIỆN ===");
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
assert(!html.includes('id="btn-save-project"'), 'Global save button đã được loại bỏ khỏi header');
const tabs = [
  'panel-assistant',
  'panel-display',
  'panel-audio',
  'panel-network',
  'panel-peripherals',
  'panel-system',
  'panel-build',
  'panel-power'
];
tabs.forEach(tab => {
  assert(html.includes(`id="${tab}"`), `Tab [${tab}] có phần tử section id tương ứng`);
  assert(html.includes(`data-target="${tab}"`), `Tab [${tab}] có nút chuyển tab trên sidebar`);
});
assert(html.includes('id="project-status-bar"'), 'Thanh trạng thái dự án (project-status-bar) hiện diện');
assert(html.includes('id="fs-status-dot"'), 'Đèn trạng thái dự án (fs-status-dot) hiện diện');
assert(html.includes('id="fs-status-text"'), 'Nhãn trạng thái dự án (fs-status-text) hiện diện');
assert(html.includes('id="btn-open-dir"'), 'Nút mở dự án (btn-open-dir) hiện diện');
assert(html.includes('id="project-modal"'), 'Modal quản lý thư mục dự án (project-modal) hiện diện');
assert(html.includes('id="input-project-root"'), 'Ô nhập đường dẫn dự án (input-project-root) hiện diện');
assert(html.includes('id="btn-apply-project-root"'), 'Nút Áp dụng đường dẫn dự án (btn-apply-project-root) hiện diện');
assert(html.includes('id="btn-reset-project-root"'), 'Nút Tự động nhận diện dự án (btn-reset-project-root) hiện diện');
assert(html.includes('id="pin-matrix-grid"'), 'Widget ma trận chân GPIO (pin-matrix-grid) hiện diện');
assert(html.includes('id="conflict-alert-bar"'), 'Thanh cảnh báo xung đột chân (conflict-alert-bar) hiện diện');
assert(html.includes('id="conflict-message"'), 'Khung thông báo lỗi xung đột (conflict-message) hiện diện');
console.log(`\n=== TỔNG KẾT: ${passed} PASS, ${failed} FAIL ===`);
if (failed > 0) process.exit(1);
