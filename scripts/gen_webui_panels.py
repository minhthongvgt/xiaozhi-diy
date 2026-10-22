#!/usr/bin/env python
import html
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WEB = os.path.join(ROOT, 'tools', 'web-configurator')

SAVE_ICON = ('<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" '
             'stroke-width="2"><path d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2z">'
             '</path><polyline points="17 21 17 13 7 13 7 21"></polyline><polyline points="7 3 7 8 15 8">'
             '</polyline></svg>')
ALWAYS_TRUE = {'y', 'IDF_TARGET_ESP32S3'}
EXTERNAL = {'SPIRAM': 'sys_spiram'}
S3_ONLY_TAG = '[ESP32-S3]'


def esc(text):
    return html.escape(str(text), quote=True)


def hint_of(help_text):
    text = ' '.join(help_text.split())
    if not text:
        return ''
    m = re.match(r'(.+?[.!?])(\s|$)', text)
    first = m.group(1) if m else text
    return first if len(first) <= 200 else first[:197] + '...'


class Builder:
    def __init__(self, schema, ui):
        self.syms = {s['name']: s for s in schema['symbols']}
        self.choices = {c['name']: c for c in schema['choices']}
        self.fields = ui['fields']
        self.bind_of_sym = {}
        self.select_of = {}
        for bid, ref in self.fields.items():
            kind, _, name = ref.partition(':') if ':' in ref else ('sym', '', ref)
            if kind == 'sym':
                self.bind_of_sym[name] = bid
            elif kind == 'choice':
                for m in self.choices[name]['members']:
                    self.select_of[m] = (bid, self.option_value(bid, m))
            elif kind == 'pseudo':
                for m in name.split(','):
                    self.select_of[m] = (bid, m)
        self.unbound = []
        self.generic = []
        self.exclude = set(ui.get('exclude', []))

    def add_generic(self, sections):
        def inside(path, sec):
            m = sec['menu']
            return path == m if sec.get('exact') else path[:len(m)] == m
        bound_choices = {r.split(':', 1)[1] for r in self.fields.values() if r.startswith('choice:')}
        items = [('choice', c) for c in self.choices.values()] + \
                [('sym', s) for s in self.syms.values() if s['choice'] is None]
        for kind, it in sorted(items, key=lambda x: x[1]['order']):
            name = it['name']
            if not it['prompt'] or name in self.exclude or not any(inside(it['menu'], s) for s in sections):
                continue
            if any(c != 'y' for _, c in it['default']) or it.get('type') == 'hex':
                if not (name in self.bind_of_sym or f'choice:{name}' in self.fields.values()):
                    sys.exit(f'{name}: default có điều kiện hoặc kiểu hex chưa được hỗ trợ cho mục chung; '
                             'hãy thêm ô viết tay trong ui_map.json hoặc mở rộng kconfigGeneric.js')
            if kind == 'choice':
                if name in bound_choices:
                    continue
                bid = f'kc_{name}'
                self.fields[bid] = f'choice:{name}'
                for m in it['members']:
                    self.select_of[m] = (bid, m)
                self.generic.append({'name': name, 'type': 'choice', 'members': it['members'],
                                     'default': it['default'][0][0] if it['default'] else it['members'][0],
                                     'depends': it['depends']})
            elif name not in self.bind_of_sym and name not in self.select_of:
                bid = f'kc_{name}'
                self.fields[bid] = name
                self.bind_of_sym[name] = bid
                d = it['default'][0][0] if it['default'] else {'bool': 'n', 'string': ''}.get(it['type'], '0')
                if it['type'] == 'bool':
                    d = d == 'y'
                elif it['type'] in ('int', 'hex'):
                    d = int(d, 0)
                self.generic.append({'name': name, 'type': it['type'], 'default': d, 'depends': it['depends'],
                                     'prompt': it['prompt']})

    def write_generic_module(self, path):
        body = ',\n'.join('  ' + json.dumps(f, ensure_ascii=False) for f in self.generic)
        with open(path, 'w', encoding='utf-8', newline='\n') as f:
            f.write(f'export const KCONFIG_FIELDS = [\n{body}\n];\n')

    @staticmethod
    def option_value(bid, member):
        if bid == 'display_uart_baud':
            return member.rsplit('_', 1)[1]
        return member

    def leaf(self, name, neg=False):
        if name in ALWAYS_TRUE:
            return [] if neg else [[]]
        if name == 'n':
            return [[]] if neg else []
        if name in EXTERNAL:
            return [[('!' if neg else '') + EXTERNAL[name]]]
        if name in self.bind_of_sym:
            return [[('!' if neg else '') + self.bind_of_sym[name]]]
        if name in self.select_of:
            sid, val = self.select_of[name]
            return [[f'{sid}{"!=" if neg else "="}{val}']]
        return [[]]

    def dnf(self, e, neg=False):
        if isinstance(e, str):
            return self.leaf(e, neg)
        op = e[0]
        if op == 'not':
            return self.dnf(e[1], not neg)
        if op in ('=', '!='):
            a, b = e[1], e[2]
            if b in ('y', 'n') and isinstance(a, str):
                want_y = (b == 'y') == (op == '=')
                return self.leaf(a, neg == want_y)
            return [[]]
        if op in ('and', 'or'):
            left, right = self.dnf(e[1], neg), self.dnf(e[2], neg)
            if (op == 'and') != neg:
                return [a + b for a in left for b in right]
            return left + right
        return [[]]

    def show_if(self, dep):
        terms = []
        for conj in self.dnf(dep):
            conj = list(dict.fromkeys(conj))
            if not conj:
                return ''
            if conj not in terms:
                terms.append(conj)
        merged = {}
        for conj in terms:
            sel = [c for c in conj if re.match(r'^[\w-]+=', c)]
            rest = tuple(c for c in conj if c not in sel)
            if len(sel) == 1:
                sid, val = sel[0].split('=', 1)
                merged.setdefault((rest, sid), []).append(val)
            else:
                merged.setdefault((tuple(conj), None), [])
        out = []
        for (rest, sid), vals in merged.items():
            parts = list(rest) + ([f'{sid}={"|".join(vals)}'] if sid else [])
            out.append('&'.join(parts))
        return '||'.join(out) if out else 'false'

    @staticmethod
    def attr_show(cond):
        return f' data-show-if="{esc(cond)}"' if cond else ''

    def render_symbol(self, s):
        bid = self.bind_of_sym.get(s['name'])
        if bid is None:
            if s['prompt']:
                self.unbound.append(s['name'])
            return ''
        cond = self.attr_show(self.show_if(s['depends']))
        hint = hint_of(s['help'])
        hint_html = f'\n              <div class="form-hint">{esc(hint)}</div>' if hint else ''
        label = esc(s['prompt'])
        if s['type'] == 'bool':
            cls = 'form-item span-2' + (' form-item-master' if s['menuconfig'] else '')
            return (f'            <div class="{cls}"{cond}>\n'
                    f'              <label class="form-checkbox"><input type="checkbox" id="{bid}">'
                    f'<span>{"<strong>" + label + "</strong>" if s["menuconfig"] else label}</span></label>'
                    f'{hint_html}\n            </div>\n')
        attrs = ''
        if s['type'] in ('int', 'hex'):
            for lo, hi, c in s['range']:
                if c == 'y' and re.fullmatch(r'-?\d+', str(lo)) and re.fullmatch(r'-?\d+', str(hi)):
                    attrs = f' min="{lo}" max="{hi}"'
                    break
            inp = f'<input type="number" class="form-control" id="{bid}"{attrs}>'
        else:
            inp = f'<input type="text" class="form-control" id="{bid}">'
        return (f'            <div class="form-item"{cond}>\n'
                f'              <label class="form-label" for="{bid}">{label}</label>\n'
                f'              {inp}{hint_html}\n            </div>\n')

    def render_select(self, bid, label, members, dep, help_text=''):
        cond = self.attr_show(self.show_if(dep))
        opts = ''.join(
            f'\n                <option value="{esc(self.option_value(bid, m))}">'
            f'{esc(self.syms[m]["prompt"] or m)}</option>' for m in members)
        hint = hint_of(help_text)
        hint_html = f'\n              <div class="form-hint">{esc(hint)}</div>' if hint else ''
        return (f'            <div class="form-item"{cond}>\n'
                f'              <label class="form-label" for="{bid}">{esc(label)}</label>\n'
                f'              <select class="form-control" id="{bid}">{opts}\n              </select>'
                f'{hint_html}\n            </div>\n')

    def items_in(self, section):
        menu = section['menu']
        def inside(path):
            return path == menu if section.get('exact') else path[:len(menu)] == menu
        items = [('sym', s) for s in self.syms.values() if inside(s['menu']) and s['choice'] is None]
        items += [('choice', c) for c in self.choices.values() if inside(c['menu'])]
        return sorted(items, key=lambda it: it[1]['order']), menu

    def render_section(self, section, panel_id, first):
        items, menu = self.items_in(section)
        title = menu[-1]
        body, last_sub, pseudo_done = '', None, set()
        for kind, it in items:
            sub = tuple(it['menu'][len(menu):])
            chunk = ''
            if kind == 'choice':
                bid = next((b for b, r in self.fields.items() if r == f'choice:{it["name"]}'), None)
                if bid is None:
                    self.unbound.append(it['name'])
                    continue
                chunk = self.render_select(bid, it['prompt'], it['members'], it['depends'])
            elif it['name'] in self.select_of and it['name'] not in self.bind_of_sym:
                bid, _ = self.select_of[it['name']]
                if bid in pseudo_done:
                    continue
                pseudo_done.add(bid)
                members = self.fields[bid].split(':', 1)[1].split(',')
                chunk = self.render_select(bid, title, members, it['depends'])
            else:
                chunk = self.render_symbol(it)
            if not chunk:
                continue
            if sub and sub != last_sub and not (kind == 'sym' and it.get('menuconfig')):
                body += f'            <div class="span-2 form-subheading"><span>{esc(sub[-1])}</span></div>\n'
            last_sub = sub + (it['prompt'],) if kind == 'sym' and it.get('menuconfig') else sub
            body += chunk
        actions = ''
        if first:
            actions = (f'\n          <div class="tab-header-actions">\n'
                       f'            <span class="tab-status-pill saved" data-tab-status="{panel_id}">Đã lưu</span>\n'
                       f'            <button type="button" class="btn btn-primary btn-save-tab" data-tab="{panel_id}" '
                       f'title="Lưu thông tin mục này">\n              {SAVE_ICON}\n'
                       f'              <span>Lưu mục này</span>\n            </button>\n          </div>')
        s3_only = ' data-s3-only' if any(S3_ONLY_TAG in name for name in menu) else ''
        return (f'        <div class="config-group"{s3_only}>\n'
                f'          <div class="group-header">\n'
                f'            <div class="group-title">\n'
                f'              <h2>{section.get("icon", "")} {esc(section.get("title", title))}</h2>\n'
                f'              <span class="group-desc">menuconfig: {esc(" → ".join(menu))}</span>\n'
                f'            </div>{actions}\n'
                f'          </div>\n'
                f'          <div class="form-grid">\n{body}          </div>\n'
                f'        </div>\n')

    def render_panel(self, panel):
        groups = ''.join(self.render_section(s, panel['id'], i == 0)
                         for i, s in enumerate(panel['sections']))
        return f'<section class="tab-panel" id="{panel["id"]}">\n{groups}</section>\n'


def main():
    sys.stdout.reconfigure(encoding='utf-8')
    with open(os.path.join(WEB, 'kconfig_schema.json'), encoding='utf-8') as f:
        schema = json.load(f)
    with open(os.path.join(WEB, 'ui_map.json'), encoding='utf-8') as f:
        ui = json.load(f)
    b = Builder(schema, ui)
    b.add_generic([s for p in ui['panels'] for s in p['sections']])
    b.write_generic_module(os.path.join(WEB, 'js', 'kconfigFields.js'))
    for panel in ui['panels']:
        path = os.path.join(WEB, 'components', panel['id'] + '.html')
        with open(path, 'w', encoding='utf-8', newline='\n') as f:
            f.write(b.render_panel(panel))
        print(f'Đã ghi {os.path.relpath(path, ROOT)}')
    print(f'{len(b.unbound)} mục menuconfig chưa có ô trên giao diện')
    if '--list-unbound' in sys.argv:
        for name in b.unbound:
            print('  ', name)


if __name__ == '__main__':
    main()
