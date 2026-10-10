import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { SdkconfigParser as P } from './js/parser.js';
import { SdkconfigGenerator as G } from './js/generator.js';

const here = path.dirname(fileURLToPath(import.meta.url));
const projectRoot = path.resolve(here, '../..');
let pass = 0;
let fail = 0;

function test(name, fn) {
  try {
    fn();
    console.log('  [PASS]', name);
    pass++;
  } catch (error) {
    console.error('  [FAIL]', name, '\n       ', error.message.split('\n')[0]);
    fail++;
  }
}

console.log('=== KIỂM THỬ TÍCH HỢP WEB CONFIGURATOR VỚI XIAOZHI ===');

const defaults = fs.readFileSync(path.join(projectRoot, 'sdkconfig.defaults'), 'utf8');
const targetDefaults = fs.readFileSync(path.join(projectRoot, 'sdkconfig.defaults.esp32s3'), 'utf8');
const kconfig = fs.readFileSync(path.join(projectRoot, 'main/Kconfig.projbuild'), 'utf8');
const partitions = fs.readFileSync(path.join(projectRoot, 'partitions/16m.csv'), 'utf8');

test('sdkconfig.defaults được parse', () => {
  const state = P.parse(defaults);
  assert.equal(state.system.flash_size, '16MB');
  assert.equal(state.system.partition_table, 'partitions/16m.csv');
});

test('sdkconfig.defaults.esp32s3 được parse', () => {
  const state = P.parse(targetDefaults);
  assert.equal(state.system.spiram, true);
  assert.equal(state.system.spiram_mode, 'OCT');
  assert.equal(state.system.cpu_freq, '240');
});

test('Kconfig.projbuild chứa các symbol nền tảng', () => {
  assert.match(kconfig, /menu "Xiaozhi Assistant"/);
  assert.match(kconfig, /choice BOARD_TYPE/);
  assert.match(kconfig, /config BOARD_TYPE_ESP32_S3_N16R8_CUSTOM/);
  assert.match(kconfig, /config LANGUAGE_VI_VN/);
});

test('partition 16MB có OTA và NVS', () => {
  assert.match(partitions, /\bapp0\b/);
  assert.match(partitions, /\bapp1\b/);
  assert.match(partitions, /\bnvs\b/);
});

test('generator xử lý state đọc từ target defaults', () => {
  const lines = G.build(P.parse(targetDefaults));
  assert.ok(lines.some(line => line === 'CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM=y'));
  assert.ok(lines.some(line => line === 'CONFIG_SPIRAM=y'));
  assert.ok(lines.some(line => line === 'CONFIG_SPIRAM_MODE_OCT=y'));
});

console.log(`\n=== TỔNG KẾT: ${pass} PASS, ${fail} FAIL ===`);
process.exit(fail ? 1 : 0);
