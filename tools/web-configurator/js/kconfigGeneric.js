import { KCONFIG_FIELDS } from './kconfigFields.js';

const TARGET = 'IDF_TARGET_ESP32S3';
const PIN_RE = /(_PIN|_GPIO|_SDA|_SCL|_SCK|_MOSI|_MISO|_CS)$/;

export function kconfigDefaults() {
  const d = {};
  for (const f of KCONFIG_FIELDS) d[f.name] = f.default;
  return d;
}

function value(name, values) {
  if (name === 'y' || name === 'n') return name;
  if (name.startsWith('IDF_TARGET_')) return name === TARGET ? 'y' : 'n';
  return values.has(name) ? values.get(name) : (/^-?\d+$/.test(name) ? name : 'n');
}

export function evalDepends(expr, values) {
  if (typeof expr === 'string') return value(expr, values) !== 'n';
  const [op, a, b] = expr;
  if (op === 'and') return evalDepends(a, values) && evalDepends(b, values);
  if (op === 'or') return evalDepends(a, values) || evalDepends(b, values);
  if (op === 'not') return !evalDepends(a, values);
  const va = value(a, values), vb = value(b, values);
  if (op === '=') return va === vb;
  if (op === '!=') return va !== vb;
  return true;
}

function linesToValues(lines) {
  const values = new Map();
  for (const line of lines) {
    let m = /^# CONFIG_(\w+) is not set$/.exec(line);
    if (m) { values.set(m[1], 'n'); continue; }
    m = /^CONFIG_(\w+)=(.*)$/.exec(line);
    if (m) values.set(m[1], m[2].replace(/^"(.*)"$/, '$1'));
  }
  return values;
}

export function emitKconfigFields(lines, s) {
  const kc = s.kconfig || {};
  const values = linesToValues(lines);
  const out = ['# menuconfig (các mục không có ô viết tay)'];
  const en  = k => { out.push(`CONFIG_${k}=y`); values.set(k, 'y'); };
  const dis = k => { out.push(`# CONFIG_${k} is not set`); values.set(k, 'n'); };
  for (const f of KCONFIG_FIELDS) {
    if (!evalDepends(f.depends, values)) continue;
    const v = kc[f.name] ?? f.default;
    if (f.type === 'bool') v ? en(f.name) : dis(f.name);
    else if (f.type === 'choice') {
      const sel = f.members.includes(v) ? v : f.default;
      for (const m of f.members) m === sel ? en(m) : dis(m);
      values.set(f.name, 'y');
    } else if (f.type === 'int' || f.type === 'hex') {
      const n = Number.isFinite(Number(v)) ? Number(v) : f.default;
      out.push(`CONFIG_${f.name}=${n}`); values.set(f.name, String(n));
    } else {
      out.push(`CONFIG_${f.name}="${String(v ?? '').replace(/\\/g, '\\\\').replace(/"/g, '\\"')}"`);
      values.set(f.name, String(v ?? ''));
    }
  }
  if (out.length > 1) lines.push(...out, '');
}

export function parseKconfigFields(m, s, str) {
  s.kconfig = { ...kconfigDefaults(), ...(s.kconfig || {}) };
  for (const f of KCONFIG_FIELDS) {
    if (f.type === 'choice') {
      const hit = f.members.find(x => m[`CONFIG_${x}`] === 'y');
      if (hit) s.kconfig[f.name] = hit;
      continue;
    }
    const key = `CONFIG_${f.name}`;
    if (!(key in m)) continue;
    if (f.type === 'bool') s.kconfig[f.name] = m[key] === 'y';
    else if (f.type === 'int' || f.type === 'hex') {
      const n = parseInt(m[key], 10);
      if (!Number.isNaN(n)) s.kconfig[f.name] = n;
    } else s.kconfig[f.name] = str(key);
  }
}

export function kconfigPins(lines) {
  const values = linesToValues(lines);
  const pins = [];
  for (const f of KCONFIG_FIELDS) {
    if ((f.type !== 'int') || !PIN_RE.test(f.name) || !values.has(f.name)) continue;
    const pin = Number(values.get(f.name));
    if (pin < 0) continue;
    const bus = /_SDA$|_SCL$/.test(f.name) ? 'i2c' : /_SCK$|_MOSI$|_MISO$/.test(f.name) ? 'spi' : null;
    pins.push({ pin, name: f.name.replace(/^CUSTOM_/, ''), bus });
  }
  return pins;
}
