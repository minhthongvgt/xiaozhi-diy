const fs = require('fs');
const html = fs.readFileSync('tools/web-configurator/index.html', 'utf8');

// 1. Check no global save button
if (html.includes('id="btn-save-project"')) throw new Error('btn-save-project still in html');
console.log('[PASS] Global save button removed from header');

// 2. Check each tab has btn-save-tab and tab-status-pill
const tabs = ['panel-general', 'panel-audio', 'panel-display', 'panel-wakeword', 'panel-network', 'panel-mcp', 'panel-buttons', 'panel-peripherals', 'panel-communication', 'panel-flash'];
tabs.forEach(tab => {
  if (!html.includes(`data-tab="${tab}"`)) throw new Error(`Missing btn-save-tab for ${tab}`);
  if (!html.includes(`data-tab-status="${tab}"`)) throw new Error(`Missing tab-status-pill for ${tab}`);
});
console.log('[PASS] All 10 tabs have per-tab save button and status pill');

// 3. Check unsaved modal and toast container
if (!html.includes('id="unsaved-modal"')) throw new Error('Missing unsaved-modal');
if (!html.includes('id="toast-container"')) throw new Error('Missing toast-container');
console.log('[PASS] unsaved-modal and toast-container present');

// 4. Check display_ui_style removed
if (html.includes('id="display_ui_style"')) throw new Error('display_ui_style still in html');
console.log('[PASS] display_ui_style successfully removed from Tab 3');

// 5. Check flash assets in Tab 1
if (!html.includes('id="general_flash_assets"')) throw new Error('Missing general_flash_assets in Tab 1');
console.log('[PASS] general_flash_assets present as standard in Tab 1');

// 6. Check no numbering in sidebar tab-labels
const forbiddenNumbers = ['1. ', '2. ', '3. ', '4. ', '5. ', '6. ', '7. ', '8. ', '9. ', '10. '];
forbiddenNumbers.forEach(num => {
  if (html.includes(`<span class="tab-label">${num}`)) {
    throw new Error(`Sidebar still has numbered label: ${num}`);
  }
});
console.log('[PASS] All 10 sidebar tab-labels have no numbers');

// 7. Check no numbering in category h2 headers
forbiddenNumbers.forEach(num => {
  if (html.includes(`<h2>🌐 ${num}`) || html.includes(`<h2>🔊 ${num}`) || html.includes(`<h2>🖥️ ${num}`) ||
      html.includes(`<h2>🎙️ ${num}`) || html.includes(`<h2>📶 ${num}`) || html.includes(`<h2>🤖 ${num}`) ||
      html.includes(`<h2>🔘 ${num}`) || html.includes(`<h2>🔌 ${num}`) || html.includes(`<h2>📡 ${num}`) ||
      html.includes(`⚡ ${num}`)) {
    throw new Error(`Tab header still has numbered title: ${num}`);
  }
});
console.log('[PASS] All 10 category headers have no numbers');

// 8. Check Add Item Modal and Trigger Buttons
const modalRequiredIds = [
  'btn-add-button',
  'btn-add-peripheral',
  'btn-add-comm',
  'add-item-modal',
  'modal-dialog-title',
  'modal-item-type',
  'modal-dynamic-fields',
  'modal-btn-confirm',
  'modal-btn-cancel',
  'modal-btn-close'
];
modalRequiredIds.forEach(id => {
  if (!html.includes(`id="${id}"`)) {
    throw new Error(`Missing required modal or trigger element: id="${id}"`);
  }
});
console.log('[PASS] Add-item modal and trigger buttons present with correct IDs');

// 9. Check Flash and Partition dropdowns
if (!html.includes('<select class="form-control" id="flash_size">')) throw new Error('Missing select id="flash_size"');
if (!html.includes('<select class="form-control" id="partition_table">')) throw new Error('Missing select id="partition_table"');
if (!html.includes('<select class="form-control" id="psram_mode">')) throw new Error('Missing select id="psram_mode"');
console.log('[PASS] Flash and Partition dropdowns present');

console.log('=== ALL 9 DOM CHECKS PASSED ===');
