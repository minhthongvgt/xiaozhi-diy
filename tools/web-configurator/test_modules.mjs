
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
t('MCP tự động: không ghi công tắc MCP ẩn, chỉ ghi đèn khi chọn', () => {
  const off = text(clone(D));
  assert.doesNotMatch(off, /CONFIG_ENABLE_CUSTOM_MCP_SERVER|CONFIG_CUSTOM_MCP_TOOL_(SENSOR|ACTUATOR|ROBOT|LED|PERIPH|PRESS_TO_TALK)=/);
  assert.match(off, /^# CONFIG_CUSTOM_MCP_TOOL_LAMP is not set$/m);
  const s = clone(D); s.mcp.lamp = true; s.mcp.lamp_gpio = 10;
  const on = text(s);
  assert.match(on, /^CONFIG_CUSTOM_MCP_TOOL_LAMP=y$/m);
  assert.match(on, /^CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO=10$/m);
  assert.equal(P.parse(on).mcp.lamp, true);
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
t('PinValidator.validatePinAssignment từ chối dải khóa 26-37 cho N16R8', () => {
  for (let pin = 26; pin <= 37; pin++) {
    const res = PinValidator.validatePinAssignment('TestPin', pin);
    assert.equal(res.valid, false);
    assert.match(res.error, /BỊ KHÓA/);
  }
});
t('PinValidator.validatePinAssignment từ chối chân ngoài dải 0-48', () => {
  assert.equal(PinValidator.validatePinAssignment('TestPin', 49).valid, false);
  assert.equal(PinValidator.validatePinAssignment('TestPin', -2).valid, false);
});
t('PinValidator.validatePinAssignment chấp nhận -1 và bỏ qua', () => {
  assert.equal(PinValidator.validatePinAssignment('TestPin', -1).valid, true);
  assert.equal(PinValidator.validatePinAssignment('TestPin', null).valid, true);
});
t('PinValidator.validatePinAssignment cảnh báo USB JTAG và Strapping pins', () => {
  const usb = PinValidator.validatePinAssignment('USB', 19);
  assert.equal(usb.valid, true);
  assert.match(usb.warning, /USB/);
  const strap = PinValidator.validatePinAssignment('Boot', 0);
  assert.equal(strap.valid, true);
  assert.match(strap.warning, /Strapping/);
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

t('PinValidator.validatePinAssignment từ chối chân cố định của Waveshare ESP32-P4 Dev Kit', () => {
  const p4Context = { target: 'esp32p4' };
  const lockedPins = [0, 7, 8, 9, 10, 11, 12, 13, 20, 21, 26, 27, 28, 29, 30, 31, 34, 35, 37, 38, 39, 40, 41, 42, 43, 44, 49, 50, 51, 52, 53];
  for (const pin of lockedPins) {
    const res = PinValidator.validatePinAssignment('Test', pin, p4Context);
    assert.equal(res.valid, false);
    assert.match(res.error, /BỊ KHÓA/);
  }
});
t('PinValidator.validatePinAssignment cho phép chân tự do của Waveshare ESP32-P4 trong dải 0-54', () => {
  const p4Context = { target: 'esp32p4' };
  const freePins = [1, 2, 3, 4, 5, 6, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 32, 33, 36, 45, 46, 47, 48, 54];
  for (const pin of freePins) {
    const res = PinValidator.validatePinAssignment('Test', pin, p4Context);
    assert.equal(res.valid, true);
    assert.equal(res.error, null);
  }
});
t('PinValidator.validatePinAssignment từ chối GPIO ngoài dải 0-54 cho ESP32-P4', () => {
  const p4Context = { target: 'esp32p4' };
  const res = PinValidator.validatePinAssignment('Test', 55, p4Context);
  assert.equal(res.valid, false);
  assert.match(res.error, /ngoài dải hợp lệ 0-54/);
});

const AECS = ['USE_DEVICE_AEC', 'USE_SERVER_AEC', 'USE_NO_AEC'];
t('AEC mode được ghi đúng 1 lựa chọn và đọc lại đúng', () => {
  for (const mode of AECS) {
    const s = clone(D); s.wakeword.aec_mode = mode;
    const txt = text(s);
    assert.equal(yCount(txt, AECS), 1);
    assert.match(txt, new RegExp(`^CONFIG_${mode}=y$`, 'm'));
    assert.equal(P.parse(txt).wakeword.aec_mode, mode);
  }
});
t('AEC mode rỗng (thiếu phần tử DOM) rơi về USE_DEVICE_AEC, không sinh choice rỗng', () => {
  const s = clone(D); s.wakeword.aec_mode = '';
  assert.match(text(s), /^CONFIG_USE_DEVICE_AEC=y$/m);
});
t('không có PSRAM thì không ghi dòng choice AEC', () => {
  const s = clone(D); s.system.spiram = false; s.wakeword.aec_mode = 'USE_SERVER_AEC';
  const txt = text(s);
  assert.match(txt, /^# CONFIG_USE_AUDIO_PROCESSOR is not set$/m);
  for (const k of AECS) assert.doesNotMatch(txt, new RegExp(`CONFIG_${k}\\b`));
});
t('không có PSRAM thì Wake Word về WAKE_WORD_DISABLED; từ khóa tùy chỉnh được escape', () => {
  const s = clone(D); s.system.spiram = false; s.wakeword.type = 'USE_AFE_WAKE_WORD';
  assert.match(text(s), /^CONFIG_WAKE_WORD_DISABLED=y$/m);
  const c = clone(D); c.wakeword.type = 'USE_CUSTOM_WAKE_WORD'; c.wakeword.custom_word = 'ni "hao"';
  const txt = text(c);
  assert.match(txt, /^CONFIG_CUSTOM_WAKE_WORD="ni \\"hao\\""$/m);
  assert.equal(P.parse(txt).wakeword.custom_word, 'ni "hao"');
  assert.doesNotMatch(txt, /USE_ESP_WAKE_WORD/);
});
t('Adaptive UI ghi đủ symbol và đọc lại đúng, tên thành phố có dấu " được escape', () => {
  const s = clone(D);
  Object.assign(s.general, { display_style: 'CUSTOM_UI_STYLE_SMART_DASHBOARD', ui_accent: 'CUSTOM_UI_ACCENT_AMBER',
    ui_voice_wave: true, ui_auto_scale: true, ui_round_safe: true, ui_weather_enable: true,
    ui_weather_city: 'Ho Chi Minh "City" \\ VN' });
  const txt = text(s);
  assert.match(txt, /^CONFIG_CUSTOM_UI_WEATHER_CITY="Ho Chi Minh \\"City\\" \\\\ VN"$/m);
  const back = P.parse(txt).general;
  for (const k of ['display_style', 'ui_accent', 'ui_voice_wave', 'ui_auto_scale', 'ui_round_safe', 'ui_weather_enable', 'ui_weather_city'])
    assert.equal(back[k], s.general[k], k);
});
t('kiểu hiển thị không phải Adaptive thì tắt các tùy chọn Adaptive UI', () => {
  const s = clone(D); s.general.ui_weather_enable = true;
  const txt = text(s);
  assert.match(txt, /^# CONFIG_CUSTOM_UI_WEATHER_ENABLE is not set$/m);
  assert.doesNotMatch(txt, /CONFIG_CUSTOM_UI_ACCENT_\w+=y/);
});
t('mục menuconfig chung (state.kconfig) ghi ra rồi đọc lại đúng', () => {
  const s = clone(D);
  Object.assign(s.kconfig, { CUSTOM_ENABLE_PERIPH_RTC: true, CUSTOM_PERIPH_RTC_I2C_SDA: 8, CUSTOM_PERIPH_RTC_INT_PIN: 7,
    USE_AUDIO_DEBUGGER: true, AUDIO_DEBUG_UDP_SERVER: '10.0.0.5:9000', CUSTOM_LED_EXTRA_GPIOS: '4,5' });
  const txt = text(s);
  assert.match(txt, /^CONFIG_CUSTOM_ENABLE_PERIPH_RTC=y$/m);
  assert.match(txt, /^CONFIG_CUSTOM_PERIPH_RTC_I2C_SDA=8$/m);
  assert.match(txt, /^CONFIG_AUDIO_DEBUG_UDP_SERVER="10\.0\.0\.5:9000"$/m);
  const back = P.parse(txt).kconfig;
  for (const k of ['CUSTOM_ENABLE_PERIPH_RTC', 'CUSTOM_PERIPH_RTC_I2C_SDA', 'CUSTOM_PERIPH_RTC_INT_PIN',
    'USE_AUDIO_DEBUGGER', 'AUDIO_DEBUG_UDP_SERVER', 'CUSTOM_LED_EXTRA_GPIOS']) assert.equal(back[k], s.kconfig[k], k);
});
t('mục chung bị ẩn khi mục cha tắt (giống menuconfig)', () => {
  const s = clone(D); s.kconfig.CUSTOM_ENABLE_PERIPH_RTC = false; s.kconfig.CUSTOM_PERIPH_RTC_I2C_SDA = 8;
  s.kconfig.CUSTOM_MCP_TOOL_PRESS_TO_TALK = true;
  const txt = text(s);
  assert.doesNotMatch(txt, /CUSTOM_PERIPH_RTC_I2C_SDA/);
  assert.doesNotMatch(txt, /CUSTOM_MCP_TOOL_PRESS_TO_TALK/);
});
t('tắt AFE Audio Processing thì không ghi choice AEC', () => {
  const s = clone(D); s.wakeword.audio_processor = false;
  const txt = text(s);
  assert.match(txt, /^# CONFIG_USE_AUDIO_PROCESSOR is not set$/m);
  assert.doesNotMatch(txt, /CONFIG_USE_(DEVICE|SERVER|NO)_AEC/);
  assert.equal(P.parse(txt).wakeword.audio_processor, false);
});
t('PinValidator bắt trùng chân giữa mục chung và mục viết tay', () => {
  const s = clone(D); s.led.enable = true; s.led.gpio = 7;
  s.kconfig.CUSTOM_ENABLE_PERIPH_RTC = true; s.kconfig.CUSTOM_PERIPH_RTC_INT_PIN = 7;
  assert.ok(PinValidator.validate(s).errors.some(e => e.pin === 7 && /PERIPH_RTC_INT_PIN/.test(e.message)));
  s.kconfig.CUSTOM_ENABLE_PERIPH_RTC = false;
  assert.ok(!PinValidator.validate(s).errors.some(e => e.pin === 7));
});
t('PinValidator cho dùng chung bus I2C và SPI', () => {
  const s = clone(D);
  s.kconfig.CUSTOM_ENABLE_PERIPH_RTC = true; s.kconfig.CUSTOM_PERIPH_RTC_I2C_SDA = 8;
  s.kconfig.CUSTOM_ENABLE_PERIPH_NFC_PN532 = true; s.kconfig.CUSTOM_PERIPH_NFC_I2C_SDA = 8;
  s.display.enable = true; s.display.type = 'CUSTOM_DISPLAY_ST7789'; s.display.pin_clk = 12;
  s.kconfig.CUSTOM_ENABLE_PERIPH_SDCARD_SPI = true; s.kconfig.CUSTOM_PERIPH_SDCARD_SPI_SCK = 12;
  assert.deepEqual(PinValidator.validate(s).errors.filter(e => e.pin === 8 || e.pin === 12), []);
});
t('PinValidator: chân khác vai trò trên cùng bus vẫn bị báo trùng', () => {
  const s = clone(D); s.display.enable = true; s.display.type = 'CUSTOM_DISPLAY_ST7789';
  s.display.pin_mosi = 11; s.display.pin_clk = 11;
  assert.ok(PinValidator.validate(s).errors.some(e => e.pin === 11));
});
t('PinValidator bỏ qua chân codec I2C khi loa không dùng codec (ô đang ẩn)', () => {
  const s = clone(D); s.speaker.enable = true; s.speaker.type = 'CUSTOM_AUDIO_DAC_MAX98357A';
  s.speaker.codec_sda = 4; s.speaker.pin_dout = 4;
  assert.ok(!PinValidator.validate(s).errors.some(e => e.pin === 4));
});
t('chuỗi có dấu \\ (đường dẫn Windows) ghi ra rồi đọc lại không đổi', () => {
  const s = clone(D); s.general.flash_assets = 'FLASH_CUSTOM_ASSETS';
  s.general.custom_assets_file = 'C:\\a\\assets.bin';
  const txt = text(s);
  assert.match(txt, /^CONFIG_CUSTOM_ASSETS_FILE="C:\\\\a\\\\assets\.bin"$/m);
  assert.equal(P.parse(txt).general.custom_assets_file, 'C:\\a\\assets.bin');
});
t('console baud về 115200 thì bỏ Custom UART', () => {
  const s = clone(D); s.build.baud_rate = '115200';
  assert.match(text(s), /^# CONFIG_ESP_CONSOLE_UART_CUSTOM is not set$/m);
  s.build.baud_rate = '921600';
  assert.match(text(s), /^CONFIG_ESP_CONSOLE_UART_CUSTOM=y$/m);
});
t('kênh console USB chọn trong menuconfig được giữ nguyên', () => {
  const s = P.parse('CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y\n# CONFIG_ESP_CONSOLE_UART_DEFAULT is not set\n');
  assert.equal(s.build.console_channel, 'USB_SERIAL_JTAG');
  assert.doesNotMatch(text(s), /CONFIG_ESP_CONSOLE_UART/);
});
{
  const fs = await import('node:fs');
  const here = new URL('.', import.meta.url);
  const index = fs.readFileSync(new URL('index.html', here), 'utf8');
  const panels = [...index.matchAll(/id="placeholder-(panel-[\w-]+)"/g)].map(m => m[1]);
  const html = panels.map(p => fs.readFileSync(new URL(`components/${p}.html`, here), 'utf8')).join('\n') + index;
  t('panel được nạp chứa wakeword_aec_mode, không còn checkbox AEC cũ', () => {
    assert.match(html, /id="wakeword_aec_mode"/);
    assert.doesNotMatch(html, /id="wakeword_(device|server)_aec"/);
  });
  t('id của các phần tử trong các panel được nạp là duy nhất', () => {
    const ids = [...html.matchAll(/\sid="([^"]+)"/g)].map(m => m[1]);
    const dup = [...new Set(ids.filter((x, i) => ids.indexOf(x) !== i))];
    assert.deepEqual(dup, []);
  });
}

console.log(`\n=== TỔNG KẾT: ${pass} PASS, ${fail} FAIL ===`);
process.exit(fail ? 1 : 0);
