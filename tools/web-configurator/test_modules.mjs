
import assert from 'node:assert/strict';
import { SdkconfigGenerator as G } from './js/generator.js';
import { SdkconfigParser as P } from './js/parser.js';
import { DEFAULT_CONFIG as D } from './js/configState.js';
import { PinValidator } from './js/pinValidator.js';

let pass = 0, fail = 0;
const clone = o => JSON.parse(JSON.stringify(o));
const t = (name, fn) => { try { fn(); console.log('  [PASS]', name); pass++; } catch (e) { console.error('  [FAIL]', name, '\n        ', e.message.split('\n')[0]); fail++; } };
const text = s => G.build(s).join('\n');
const yCount = (txt, keys) => keys.filter(k => new RegExp(`^CONFIG_${k}=y$`, 'm').test(txt)).length;

console.log('=== TEST MODULE THẬT (js/) ===');

t('generator không ném lỗi với cấu hình mặc định', () => G.build(clone(D)));
t('parser không ném lỗi với text rỗng', () => P.parse(''));
t('parse("") trả về đúng DEFAULT_CONFIG (không ghi đè mặc định bằng false/-1)', () => assert.deepEqual(P.parse(''), D));
t('round-trip: build → parse → build giữ nguyên', () => {
  const a = text(clone(D));
  assert.equal(text(P.parse(a)), a);
});
t('file thiếu CONFIG_SPIRAM (nằm ở sdkconfig.defaults.esp32s3) vẫn giữ PSRAM=bật', () => {
  assert.equal(P.parse('CONFIG_LANGUAGE_VI_VN=y\n').system.spiram, true);
});
t('"# CONFIG_SPIRAM is not set" tắt PSRAM thật sự', () => {
  assert.equal(P.parse('# CONFIG_SPIRAM is not set\n').system.spiram, false);
});
t('LED GPIO48 mặc định không bị biến thành -1 khi file không có key', () => assert.equal(P.parse('').led.gpio, 48));
t('PSRAM Quad (UI "QIO") → CONFIG_SPIRAM_MODE_QUAD và đọc lại đúng', () => {
  const s = clone(D); s.system.spiram_mode = 'QIO';
  const txt = text(s);
  assert.match(txt, /^CONFIG_SPIRAM_MODE_QUAD=y$/m);
  assert.equal(P.parse(txt).system.spiram_mode, 'QIO');
});
t('PSRAM Octal đọc lại đúng (không hard-code OCT)', () => {
  assert.equal(P.parse('CONFIG_SPIRAM_MODE_QUAD=y\n').system.spiram_mode, 'QIO');
  assert.equal(P.parse('CONFIG_SPIRAM_MODE_OCT=y\n').system.spiram_mode, 'OCT');
});
t('flash_size được đọc từ file (8MB không bị ghi đè thành 16MB)', () => {
  assert.equal(P.parse('CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y\n').system.flash_size, '8MB');
});
t('mỗi nhóm choice có đúng 1 dòng =y', () => {
  const txt = text(clone(D));
  assert.equal(yCount(txt, ['ESPTOOLPY_FLASHSIZE_4MB','ESPTOOLPY_FLASHSIZE_8MB','ESPTOOLPY_FLASHSIZE_16MB','ESPTOOLPY_FLASHSIZE_32MB']), 1);
  assert.equal(yCount(txt, ['SPIRAM_MODE_OCT','SPIRAM_MODE_QUAD']), 1);
  assert.equal(yCount(txt, ['LOG_DEFAULT_LEVEL_INFO','LOG_DEFAULT_LEVEL_DEBUG','LOG_DEFAULT_LEVEL_ERROR']), 1);
});
t('tab UART (s.uart) thực sự được ghi ra sdkconfig', () => {
  const s = clone(D); Object.assign(s.uart, { enable: true, port: 'CUSTOM_UART_PORT_2', pin_tx: 17, pin_rx: 18, baudrate: 9600 });
  const txt = text(s);
  assert.match(txt, /^CONFIG_ENABLE_CUSTOM_UART=y$/m);
  assert.match(txt, /^CONFIG_CUSTOM_UART_PORT_2=y$/m);
  assert.match(txt, /^CONFIG_CUSTOM_UART_PIN_TX=17$/m);
  const r = P.parse(txt).uart;
  assert.equal(r.enable, true); assert.equal(r.port, 'CUSTOM_UART_PORT_2'); assert.equal(r.baudrate, 9600);
});
t('Wi-Fi BluFi (s.wifi.method của GUI) được ghi ra', () => {
  const s = clone(D); s.wifi.method = 'USE_ESP_BLUFI_WIFI_PROVISIONING';
  assert.match(text(s), /^CONFIG_USE_ESP_BLUFI_WIFI_PROVISIONING=y$/m);
});
t('MCP server (s.mcp.enable của GUI) được ghi ra', () => {
  const s = clone(D); s.mcp.enable = true;
  assert.match(text(s), /^CONFIG_ENABLE_CUSTOM_MCP_SERVER=y$/m);
});
t('log level / baud console đang dùng trong file được giữ nguyên', () => {
  const r = P.parse('CONFIG_LOG_DEFAULT_LEVEL_DEBUG=y\nCONFIG_ESP_CONSOLE_UART_BAUDRATE=921600\n');
  assert.equal(r.build.log_level, 'DEBUG'); assert.equal(r.build.baud_rate, '921600');
});
t('luôn ghi CONFIG_IDF_TARGET="esp32s3"', () => assert.match(text(clone(D)), /^CONFIG_IDF_TARGET="esp32s3"$/m));


t('PinValidator mặc định không có xung đột', () => {
  assert.equal(PinValidator.validate(clone(D)).errors.length, 0);
});
t('PinValidator khóa GPIO 26..32 khi dùng Octal PSRAM', () => {
  for (let pin = 26; pin <= 32; pin++) {
    const s = clone(D);
    s.speaker.enable = true;
    s.speaker.pin_bclk = pin;
    const result = PinValidator.validate(s);
    assert.ok(result.errors.some(e => e.pin === pin && e.message.includes('BỊ KHÓA')));
  }
});
t('PinValidator phát hiện GPIO dùng trùng giữa module', () => {
  const s = clone(D);
  s.speaker.enable = true;
  s.speaker.pin_bclk = 15;
  s.sensors.dht_enable = true;
  s.sensors.dht_gpio = 15;
  assert.ok(PinValidator.validate(s).errors.some(e => e.message.includes('Xung đột GPIO 15')));
});
t('I2C cùng bus không bị báo xung đột', () => {
  const s = clone(D);
  s.display.enable = true;
  s.display.pin_i2c_sda = 8;
  s.display.pin_i2c_scl = 9;
  s.touch.enable = true;
  s.touch.pin_sda = 8;
  s.touch.pin_scl = 9;
  s.speaker.enable = true;
  s.speaker.type = 'CUSTOM_AUDIO_SPK_CODEC_ES8311';
  s.speaker.codec_sda = 8;
  s.speaker.codec_scl = 9;
  assert.equal(PinValidator.validate(s).errors.filter(e => e.pin === 8 || e.pin === 9).length, 0);
});
t('generator sinh đúng các symbol ESP-IDF 6.1 bắt buộc', () => {
  const txt = text(clone(D));
  assert.match(txt, /^CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE=10096$/m);
  assert.match(txt, /^CONFIG_ESP_MAIN_TASK_STACK_SIZE=6584$/m);
  assert.match(txt, /^CONFIG_TOUCH_SUPPRESS_DEPRECATE_WARN=y$/m);
  assert.match(txt, /^CONFIG_PARTITION_TABLE_MD5=y$/m);
});
t('depends-on Multiline Chat được áp dụng', () => {
  const s = clone(D);
  s.general.display_style = 'USE_WECHAT_MESSAGE_STYLE';
  s.general.multiline_chat = true;
  assert.match(text(s), /^# CONFIG_USE_MULTILINE_CHAT_MESSAGE is not set$/m);
  s.general.display_style = 'USE_DEFAULT_MESSAGE_STYLE';
  assert.match(text(s), /^CONFIG_USE_MULTILINE_CHAT_MESSAGE=y$/m);
});
t('BluFi bật các symbol Bluetooth cần thiết', () => {
  const s = clone(D);
  s.wifi.method = 'USE_ESP_BLUFI_WIFI_PROVISIONING';
  const txt = text(s);
  assert.match(txt, /^CONFIG_BT_ENABLED=y$/m);
  assert.match(txt, /^CONFIG_BT_BLE_42_FEATURES_SUPPORTED=y$/m);
  assert.match(txt, /^CONFIG_BT_BLE_BLUFI_ENABLE=y$/m);
});

console.log(`\n=== TỔNG KẾT: ${pass} PASS, ${fail} FAIL ===`);
process.exit(fail ? 1 : 0);
