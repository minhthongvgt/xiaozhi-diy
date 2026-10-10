import fs from 'node:fs';
import assert from 'node:assert/strict';
import { JSDOM } from 'jsdom';
const U = new URL('.', import.meta.url).href;
const W = new URL('.', import.meta.url);
let html = fs.readFileSync(new URL('index.html', W), 'utf8');
for (const m of html.matchAll(/<div id="placeholder-(panel-[\w-]+)"><\/div>/g))
  html = html.replace(m[0], fs.readFileSync(new URL(`components/${m[1]}.html`, W), 'utf8'));
html = html.replace(/<script[\s\S]*?<\/script>/g, '');
const dom = new JSDOM(html, { url: 'http://localhost/' });
Object.assign(globalThis, { window: dom.window, document: dom.window.document, Event: dom.window.Event, HTMLElement: dom.window.HTMLElement });
window.apiFetch = async () => { throw new Error('offline'); };
const { DEFAULT_CONFIG } = await import(U + 'js/configState.js');
const { GUIController: G } = await import(U + 'js/guiController.js');
const { SdkconfigGenerator } = await import(U + 'js/generator.js');
const { SdkconfigParser } = await import(U + 'js/parser.js');
console.log('=== KIỂM THỬ GIAO DIỆN TRÊN DOM (jsdom) ===');
let pass = 0, fail = 0;
const t = async (n, f) => { try { await f(); console.log('  [PASS]', n); pass++; } catch (e) { console.log('  [FAIL]', n, '\n   ', e.message.split('\n').slice(0, 12).join('\n    ')); fail++; } };
const clone = o => JSON.parse(JSON.stringify(o));
window.configState = clone(DEFAULT_CONFIG);
await G.init();
const vis = id => { let el = document.getElementById(id); while (el) { if (el.style && el.style.display === 'none') return false; el = el.parentElement; } return true; };
const setv = (id, v) => { const el = document.getElementById(id); if (el.type === 'checkbox') el.checked = v; else el.value = v; el.dispatchEvent(new Event('change', { bubbles: true })); };

await t('init chạy không lỗi, đổ state lên giao diện rồi đọc lại không đổi', () => {
  const before = clone(window.configState);
  G.syncUIToState();
  assert.deepEqual(window.configState, before);
});
await t('state "bật mọi thứ" đi qua giao diện không mất giá trị', () => {
  const s = clone(DEFAULT_CONFIG);
  const walk = o => { for (const k in o) { if (typeof o[k] === 'boolean') o[k] = true; else if (o[k] && typeof o[k] === 'object') walk(o[k]); } };
  walk(s);
  s.display.type = 'CUSTOM_DISPLAY_QSPI_AMOLED'; s.display.amoled_chip = 'CUSTOM_DISPLAY_AMOLED_CO5300';
  s.camera.pin_d3 = 21; s.display.qspi_d2 = 7; s.display.uart_baud = 921600; s.uart.baudrate = 230400;
  s.general.ui_weather_city = 'Đà Nẵng'; s.wakeword.aec_mode = 'USE_SERVER_AEC'; s.wifi.method = 'USE_ESP_BLUFI_WIFI_PROVISIONING';
  window.configState = clone(s);
  G.syncStateToUI(); G.syncUIToState();
  const diff = [];
  const cmp = (a, b, p) => { for (const k in a) { const pp = p ? p + '.' + k : k; if (a[k] && typeof a[k] === 'object') cmp(a[k], b[k], pp); else if (a[k] !== b[k]) diff.push(`${pp}: ${JSON.stringify(a[k])} -> ${JSON.stringify(b[k])}`); } };
  cmp(s, window.configState, '');
  assert.deepEqual(diff, []);
});
await t('phần tử không có trên giao diện không bị ghi đè khi lưu', () => {
  window.configState = clone(DEFAULT_CONFIG);
  window.configState.build.console_channel = 'USB_SERIAL_JTAG';
  G.syncStateToUI(); G.syncUIToState();
  assert.equal(window.configState.build.console_channel, 'USB_SERIAL_JTAG');
});
await t('ẩn/hiện theo depends on: camera, UART, MCP, AMOLED, Adaptive UI', () => {
  window.configState = clone(DEFAULT_CONFIG); G.syncStateToUI();
  setv('camera_enable', false); assert.equal(vis('cam_xclk'), false);
  setv('camera_enable', true);  assert.equal(vis('cam_xclk'), true);
  setv('camera_sensor', 'CUSTOM_CAMERA_USB_UVC'); assert.equal(vis('cam_xclk'), false, 'UVC ẩn chân DVP');
  setv('mcp_enable', false); assert.equal(vis('mcp_lamp'), false);
  setv('mcp_enable', true);  assert.equal(vis('mcp_lamp'), true);
  setv('mcp_lamp', false); assert.equal(vis('mcp_lamp_gpio'), false);
  setv('uart_enable', true); setv('uart_flow', false); assert.equal(vis('uart_rts'), false);
  setv('uart_flow', true); assert.equal(vis('uart_rts'), true);
  setv('display_enable', true); setv('display_type', 'CUSTOM_DISPLAY_QSPI_AMOLED');
  assert.equal(vis('display_qspi_d0'), true); assert.equal(vis('display_mosi'), false);
  setv('display_type', 'CUSTOM_DISPLAY_ST7789'); assert.equal(vis('display_qspi_d0'), false); assert.equal(vis('display_mosi'), true);
  setv('general_display_style', 'USE_DEFAULT_MESSAGE_STYLE'); assert.equal(vis('ui_accent'), false);
  setv('general_display_style', 'CUSTOM_UI_STYLE_MINIMAL_ZEN'); assert.equal(vis('ui_accent'), true);
  setv('ui_weather_enable', false); assert.equal(vis('ui_weather_city'), false);
  setv('sys_spiram', false); assert.equal(vis('wakeword_aec_mode'), false, 'AEC cần SPIRAM');
});
await t('đổi ô trên giao diện -> generator ghi đúng symbol', () => {
  window.configState = clone(DEFAULT_CONFIG); G.syncStateToUI();
  setv('camera_enable', true); setv('camera_sensor', 'CUSTOM_CAMERA_OV5640'); setv('cam_d0', 33);
  setv('mcp_enable', true); setv('uart_enable', true); setv('uart_tx', 17); setv('uart_baud', 57600);
  setv('spk_bclk', 15); setv('speaker_enable', true);
  G.syncUIToState();
  const txt = SdkconfigGenerator.build(window.configState).join('\n');
  for (const re of [/^CONFIG_ENABLE_CUSTOM_CAMERA=y$/m, /^CONFIG_CUSTOM_CAMERA_OV5640=y$/m, /^CONFIG_CUSTOM_CAM_PIN_D0=33$/m,
    /^CONFIG_ENABLE_CUSTOM_MCP_SERVER=y$/m, /^CONFIG_ENABLE_CUSTOM_UART=y$/m, /^CONFIG_CUSTOM_UART_PIN_TX=17$/m,
    /^CONFIG_CUSTOM_UART_BAUDRATE=57600$/m, /^CONFIG_CUSTOM_AUDIO_SPK_GPIO_BCLK=15$/m]) assert.match(txt, re);
  const back = SdkconfigParser.parse(txt);
  assert.equal(back.camera.pin_d0, 33); assert.equal(back.uart.baudrate, 57600);
});
await t('mở trang có tab hiển thị; Bỏ thay đổi khôi phục giá trị', () => {
  assert.ok(document.querySelector('.tab-panel.active'), 'không có tab nào active');
  window.configState = clone(DEFAULT_CONFIG); G.syncStateToUI(); G.initSnapshots();
  G.switchTab('panel-display');
  setv('display_enable', true);
  assert.equal(window.configState.display.enable, true);
  G.pendingTab = null;
  document.getElementById('unsaved-btn-discard').click();
  assert.equal(window.configState.display.enable, DEFAULT_CONFIG.display.enable);
  assert.equal(document.getElementById('display_enable').checked, DEFAULT_CONFIG.display.enable);
});
await t('giá trị lạ trong ô chọn không bị xoá khi lưu', () => {
  window.configState = clone(DEFAULT_CONFIG); window.configState.build.baud_rate = '460800';
  window.configState.system.partition_table = 'partitions/custom.csv';
  G.syncStateToUI(); G.syncUIToState();
  assert.equal(window.configState.build.baud_rate, '460800');
  assert.equal(window.configState.system.partition_table, 'partitions/custom.csv');
});
await t('chọn chip màn hình tự điền độ phân giải', () => {
  window.configState = clone(DEFAULT_CONFIG); G.syncStateToUI(); G.initSnapshots();
  setv('display_enable', true);
  const el = document.getElementById('display_type'); el.value = 'CUSTOM_DISPLAY_OLED_SSD1306';
  el.dispatchEvent(new Event('input', { bubbles: true })); el.dispatchEvent(new Event('change', { bubbles: true }));
  assert.equal(window.configState.display.width, 128); assert.equal(window.configState.display.height, 64);
});
console.log(`\n=== TỔNG KẾT: ${pass} PASS, ${fail} FAIL ===`);
process.exit(fail ? 1 : 0);
