import assert from 'node:assert/strict';
import fs from 'node:fs';
import { BINDINGS } from './js/bindings.js';

let pass = 0, fail = 0;
const t = (name, fn) => {
  try { fn(); console.log('  [PASS]', name); pass++; }
  catch (e) { console.error('  [FAIL]', name, '\n        ', e.message.split('\n').join('\n         ')); fail++; }
};
const here = new URL('.', import.meta.url);
const read = p => fs.readFileSync(new URL(p, here), 'utf8');

const schema = JSON.parse(read('kconfig_schema.json'));
const choiceOf = new Map();
for (const c of schema.choices) for (const m of c.members) choiceOf.set(m, c);
const symbols = new Map(schema.symbols.map(s => [s.name, s]));

const index = read('index.html');
const loaded = [...index.matchAll(/id="placeholder-(panel-[\w-]+)"/g)].map(m => m[1]);
const html = loaded.map(p => read(`components/${p}.html`)).join('\n') + index;

const fields = [...html.matchAll(/<(input|select|textarea)\b([^>]*)>/g)].map(m => ({
  tag: m[1], attrs: m[2],
  id: /\sid="([^"]+)"/.exec(m[2])?.[1],
  key: /data-config-key="([^"]+)"/.exec(m[2])?.[1],
  index: m.index,
}));
const idCount = new Map();
for (const m of html.matchAll(/\sid="([^"]+)"/g)) idCount.set(m[1], (idCount.get(m[1]) || 0) + 1);
const bindById = new Map(BINDINGS.map(b => [b[0], b]));

console.log('=== KIỂM THỬ ĐỒNG BỘ GIAO DIỆN VỚI MENUCONFIG ===');

t('mỗi binding có đúng 1 phần tử trong các panel được nạp', () => {
  const missing = BINDINGS.filter(b => !idCount.has(b[0])).map(b => b[0]);
  const dup = BINDINGS.filter(b => idCount.get(b[0]) > 1).map(b => b[0]);
  assert.deepEqual({ missing, dup }, { missing: [], dup: [] });
});

t('id phần tử trong các panel được nạp là duy nhất', () => {
  assert.deepEqual([...idCount].filter(([, n]) => n > 1).map(([k]) => k), []);
});

t('mọi ô có data-config-key đều có binding cùng đường dẫn', () => {
  const bad = fields.filter(f => f.key).filter(f => !f.id || bindById.get(f.id)?.[1] !== f.key)
    .map(f => `${f.id ?? '(không id)'} key=${f.key} binding=${bindById.get(f.id)?.[1] ?? '-'}`);
  assert.deepEqual(bad, []);
});

t('mọi ô nhập liệu có binding (không có ô "chết")', () => {
  const dead = fields.filter(f => !/type="(hidden|button|submit|file)"/.test(f.attrs))
    .filter(f => !f.id || !bindById.has(f.id))
    .filter(f => !/data-no-config/.test(f.attrs))
    .filter(f => !['input-project-root'].includes(f.id))
    .map(f => f.id ?? `(không id) ${f.attrs.trim().slice(0, 60)}`);
  assert.deepEqual(dead, []);
});

const selects = [...html.matchAll(/<select\b([^>]*)>([\s\S]*?)<\/select>/g)].map(m => ({
  id: /\sid="([^"]+)"/.exec(m[1])?.[1],
  values: [...m[2].matchAll(/<option\b[^>]*value="([^"]*)"/g)].map(o => o[1]),
}));
t('option của các ô chọn khớp đúng và đủ choice trong Kconfig', () => {
  const problems = [];
  for (const sel of selects) {
    const known = sel.values.filter(v => choiceOf.has(v));
    if (!known.length) continue;
    const choice = choiceOf.get(known[0]);
    const extra = sel.values.filter(v => !choice.members.includes(v));
    const lacking = choice.members.filter(m => !sel.values.includes(m));
    if (extra.length || lacking.length)
      problems.push(`${sel.id} (${choice.name}): thừa [${extra}] thiếu [${lacking}]`);
  }
  assert.deepEqual(problems, []);
});

t('mọi id mà JS tìm bằng getElementById đều có trong trang', () => {
  const missing = [];
  for (const f of fs.readdirSync(new URL('js/', here))) {
    const src = read(`js/${f}`);
    for (const m of src.matchAll(/getElementById\('([\w-]+)'\)/g))
      if (!idCount.has(m[1])) missing.push(`${f}: ${m[1]}`);
  }
  assert.deepEqual([...new Set(missing)], []);
});

t('kconfig_schema.json còn mới so với Kconfig.projbuild', () => {
  const kconfig = fs.readFileSync(new URL('../../main/Kconfig.projbuild', here), 'utf8');
  const names = new Set([...kconfig.matchAll(/^\s*(?:menu)?config\s+(\w+)/gm)].map(m => m[1]));
  const notInSchema = [...names].filter(n => !symbols.has(n));
  const stale = [...symbols.keys()].filter(n => !names.has(n));
  assert.deepEqual({ notInSchema, stale }, { notInSchema: [], stale: [] },
    'chạy lại: python scripts/gen_kconfig_schema.py');
});

console.log(`\n=== TỔNG KẾT: ${pass} PASS, ${fail} FAIL ===`);
process.exit(fail ? 1 : 0);
