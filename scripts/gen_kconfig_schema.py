#!/usr/bin/env python
import argparse
import glob
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, 'tools', 'web-configurator', 'kconfig_schema.json')
PROJBUILD = os.path.normcase(os.path.join(ROOT, 'main', 'Kconfig.projbuild'))


def find_env(arg):
    if arg:
        return arg
    found = sorted(glob.glob(os.path.join(ROOT, 'build*', 'config.env')), key=os.path.getmtime, reverse=True)
    preferred = os.path.join(ROOT, 'build_sim', 'config.env')
    if os.path.exists(preferred):
        return preferred
    if found:
        return found[0]
    sys.exit('Không tìm thấy build*/config.env. Chạy "idf.py -B build_sim reconfigure" một lần trước.')


def main():
    ap = argparse.ArgumentParser(description='Xuất cây menu của main/Kconfig.projbuild ra tools/web-configurator/kconfig_schema.json.')
    ap.add_argument('--env', default=None)
    args = ap.parse_args()
    sys.stderr.reconfigure(encoding='utf-8')

    with open(find_env(args.env), encoding='utf-8') as f:
        for k, v in json.load(f).items():
            os.environ[k] = str(v)
    import esp_kconfiglib.core as kl

    kconf = kl.Kconfig(os.path.join(os.environ['IDF_PATH'], 'Kconfig'), warn=False)
    own = {}

    def is_own(node):
        return os.path.normcase(os.path.abspath(node.filename)) == PROJBUILD

    def expr(e):
        if isinstance(e, tuple):
            op = e[0]
            if op is kl.AND:
                return ['and', expr(e[1]), expr(e[2])]
            if op is kl.OR:
                return ['or', expr(e[1]), expr(e[2])]
            if op is kl.NOT:
                return ['not', expr(e[1])]
            return [kl.REL_TO_STR[op], expr(e[1]), expr(e[2])]
        if e is kconf.y:
            return 'y'
        if e is kconf.n:
            return 'n'
        if isinstance(e, kl.Choice):
            return e.name or 'CHOICE_' + e.syms[0].name
        return e.name if e.is_constant is False else e.str_value

    def menu_path(node):
        path, p = [], node.parent
        while p is not None and p is not kconf.top_node:
            if p.item is kl.MENU or (isinstance(p.item, kl.Symbol) and p.is_menuconfig):
                path.append(p.prompt[0] if p.prompt else '')
            p = p.parent
        return list(reversed(path))

    symbols, choices = [], []
    seen = set()
    order = 0
    for node in kconf.node_iter():
        item = node.item
        if not is_own(node):
            continue
        if isinstance(item, kl.Choice):
            if id(item) in seen:
                continue
            seen.add(id(item))
            order += 1
            choices.append({
                'name': item.name or 'CHOICE_' + item.syms[0].name,
                'order': order,
                'prompt': node.prompt[0] if node.prompt else '',
                'menu': menu_path(node),
                'depends': expr(node.dep),
                'default': [[s.name, expr(c)] for s, c in node.orig_defaults],
                'members': [s.name for s in item.syms],
            })
            continue
        if not isinstance(item, kl.Symbol) or item.name in seen:
            continue
        seen.add(item.name)
        order += 1
        symbols.append({
            'name': item.name,
            'order': order,
            'type': kl.TYPE_TO_STR[item.orig_type],
            'prompt': node.prompt[0] if node.prompt else '',
            'menu': menu_path(node),
            'menuconfig': bool(node.is_menuconfig),
            'choice': (item.choice.name or 'CHOICE_' + item.choice.syms[0].name)
                      if item.choice is not None else None,
            'depends': expr(node.dep),
            'default': [[expr(v), expr(c)] for v, c in node.orig_defaults],
            'range': [[expr(lo), expr(hi), expr(c)] for lo, hi, c in node.orig_ranges],
            'help': (node.help or '').strip(),
        })

    data = {'source': 'main/Kconfig.projbuild', 'choices': choices, 'symbols': symbols}
    with open(OUT, 'w', encoding='utf-8', newline='\n') as f:
        json.dump(data, f, ensure_ascii=False, indent=1)
        f.write('\n')
    print(f'Đã ghi {len(symbols)} symbol, {len(choices)} choice vào '
          f'{os.path.relpath(OUT, ROOT)}', file=sys.stderr)


if __name__ == '__main__':
    main()
