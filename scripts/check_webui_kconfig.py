#!/usr/bin/env python
import argparse
import collections
import glob
import json
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WEB = os.path.join(ROOT, 'tools', 'web-configurator')
PROJBUILD = os.path.normcase(os.path.join(ROOT, 'main', 'Kconfig.projbuild'))
ALLOWED = {'SEND_WAKE_WORD_DATA'}


def unquote(v):
    if len(v) >= 2 and v[0] == '"' and v[-1] == '"':
        return re.sub(r'\\(["\\])', r'\1', v[1:-1])
    return v


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
    ap = argparse.ArgumentParser(description='Cổng kiểm tra Web Configurator <-> menuconfig bằng kconfiglib của ESP-IDF.')
    ap.add_argument('--env', default=None)
    ap.add_argument('--out', default=os.path.join(ROOT, 'build_sim', 'wc_test'))
    args = ap.parse_args()
    sys.stdout.reconfigure(encoding='utf-8')

    subprocess.run(['node', os.path.join(WEB, 'gen_test_cases.mjs'), args.out], check=True)

    with open(find_env(args.env), encoding='utf-8') as f:
        for k, v in json.load(f).items():
            os.environ[k] = str(v)
    import esp_kconfiglib.core as kl
    kconf = kl.Kconfig(os.path.join(os.environ['IDF_PATH'], 'Kconfig'), warn=False)
    with open(os.path.join(WEB, 'ui_map.json'), encoding='utf-8') as f:
        excluded_choices = set(json.load(f).get('exclude', []))
    own = [s for s in kconf.unique_defined_syms
           if any(os.path.normcase(os.path.abspath(n.filename)) == PROJBUILD and n.prompt for n in s.nodes)
           and not (s.choice is not None and s.choice.name in excluded_choices)]

    mismatch, missing = collections.defaultdict(set), collections.defaultdict(set)
    files = sorted(glob.glob(os.path.join(args.out, '*.sdkconfig')))
    for path in files:
        case = os.path.basename(path)[:-len('.sdkconfig')]
        kconf.load_config(path, replace=True)
        written = set()
        with open(path, encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if line.startswith('CONFIG_') and '=' in line:
                    name, val = line[7:].split('=', 1)
                    val = unquote(val)
                elif line.startswith('# CONFIG_') and line.endswith(' is not set'):
                    name, val = line[9:-11], 'n'
                else:
                    continue
                written.add(name)
                sym = kconf.syms.get(name)
                if sym is None or not sym.nodes:
                    mismatch[name].add(f'không có trong Kconfig [{case}]')
                    continue
                got = sym.str_value
                if got == '' and sym.type != kl.STRING:
                    got = 'n'
                if got != val and name not in ALLOWED:
                    mismatch[name].add(f'ghi={val!r} hiệu lực={got!r} [{case}]')
        for sym in own:
            if sym.visibility and sym.name not in written:
                missing[sym.name].add(case)

    print(f'{len(files)} trường hợp; {len(mismatch)} symbol bị Kconfig sửa/bỏ qua; '
          f'{len(missing)} mục menuconfig đang hiện mà web không ghi')
    for name, why in sorted(mismatch.items()):
        print(f'  [LỆCH] {name}: {sorted(why)[0]} (+{len(why) - 1})')
    for name, cases in sorted(missing.items()):
        print(f'  [THIẾU] {name}: {sorted(cases)[0]} (+{len(cases) - 1})')
    return 1 if mismatch or missing else 0


if __name__ == '__main__':
    sys.exit(main())
