import fs from 'node:fs';
import path from 'node:path';
import { SdkconfigGenerator as G } from './js/generator.js';
import { DEFAULT_CONFIG } from './js/configState.js';
import { BINDINGS } from './js/bindings.js';
import { KCONFIG_FIELDS } from './js/kconfigFields.js';

const out = process.argv[2];
if (!out) { console.error('Dùng: node gen_test_cases.mjs <thư mục ra>'); process.exit(2); }
fs.mkdirSync(out, { recursive: true });
for (const f of fs.readdirSync(out)) if (f.endsWith('.sdkconfig')) fs.rmSync(path.join(out, f));

const here = new URL('.', import.meta.url);
const index = fs.readFileSync(new URL('index.html', here), 'utf8');
const html = [...index.matchAll(/id="placeholder-(panel-[\w-]+)"/g)]
  .map(m => fs.readFileSync(new URL(`components/${m[1]}.html`, here), 'utf8')).join('\n');

const clone = o => JSON.parse(JSON.stringify(o));
const maximal = () => {
  const s = clone(DEFAULT_CONFIG);
  const walk = o => { for (const k in o) {
    if (typeof o[k] === 'boolean') o[k] = true;
    else if (o[k] && typeof o[k] === 'object') walk(o[k]);
  } };
  walk(s);
  return s;
};
const setPath = (s, p, v) => { const k = p.split('.'); const last = k.pop(); k.reduce((o, x) => o[x], s)[last] = v; };

const cases = { default: clone(DEFAULT_CONFIG), maximal: maximal() };
for (const [id, p, kind] of BINDINGS) {
  const m = new RegExp(`<select[^>]*\\sid="${id}"[^>]*>([^]*?)</select>`).exec(html);
  if (!m) continue;
  for (const o of m[1].matchAll(/value="([^"]*)"/g)) {
    const s = maximal();
    setPath(s, p, kind === 'int' || p === 'display.uart_baud' ? Number(o[1]) : o[1]);
    cases[`${id}__${o[1]}`] = s;
  }
}
{
  const s = maximal();
  let pin = 1;
  for (const f of KCONFIG_FIELDS) {
    if (f.type === 'int') s.kconfig[f.name] = /CHANNEL|CHS/.test(f.name) ? 3 : (pin = pin % 21 + 1);
    if (f.type === 'string') s.kconfig[f.name] = f.name.includes('UDP') ? '10.0.0.5:9000' : '4,5';
  }
  s.kconfig.XIAOZHI_CAMERA_ALLOW_JPEG_INPUT = false;
  s.kconfig.XIAOZHI_CAMERA_IMAGE_ROTATION_ANGLE = 'XIAOZHI_CAMERA_IMAGE_ROTATION_ANGLE_270';
  cases.generic_values = s;
}
for (const [name, s] of Object.entries(cases))
  fs.writeFileSync(path.join(out, name.replace(/[^\w.-]/g, '_') + '.sdkconfig'), G.build(s).join('\n'));
console.log(`${Object.keys(cases).length} trường hợp -> ${out}`);
