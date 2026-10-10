#!/usr/bin/env python
import argparse
import glob
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DOC = os.path.join(ROOT, 'Danh_Sach_Kconfig_WebUI.md')
PROJBUILD = os.path.normcase(os.path.join(ROOT, 'main', 'Kconfig.projbuild'))

IDF_ROWS = [
    ('ESP_MAIN_TASK_STACK_SIZE', 'int', 'Main Task Stack Size', '6584'),
    ('ESP_SYSTEM_EVENT_TASK_STACK_SIZE', 'int', 'Event Task Stack Size', '10096'),
    ('ESP_DEFAULT_CPU_FREQ_MHZ_240', 'bool', 'CPU Freq 240MHz', 'y'),
    ('ESPTOOLPY_FLASHMODE_QIO', 'bool', 'Flash Mode QIO', 'y'),
    ('ESPTOOLPY_FLASHFREQ_80M', 'bool', 'Flash Freq 80MHz', 'y'),
]


def cell(text):
    return str(text).replace('|', '\\|').replace('\n', ' ').strip()


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
    ap = argparse.ArgumentParser(description='Sinh lại Danh_Sach_Kconfig_WebUI.md từ cây menu của main/Kconfig.projbuild.')
    ap.add_argument('--env', default=None)
    args = ap.parse_args()
    sys.stderr.reconfigure(encoding="utf-8")

    with open(find_env(args.env), encoding='utf-8') as f:
        for k, v in json.load(f).items():
            os.environ[k] = str(v)
    import esp_kconfiglib.core as kl

    kconf = kl.Kconfig(os.path.join(os.environ['IDF_PATH'], 'Kconfig'), warn=False)

    def menu_of(node):
        p = node.parent
        while p is not None and not (p.item is kl.MENU or (isinstance(p.item, kl.Symbol) and p.is_menuconfig)):
            p = p.parent
        return p.prompt[0] if p is not None and p.prompt else ''

    def default_of(node):
        sym = node.item
        for val, cond in node.orig_defaults:
            text = kl.expr_str(val)
            if cond is not kconf.y:
                text += ' if ' + kl.expr_str(cond)
            return text
        if sym.choice is not None and sym.choice.defaults and sym.choice.defaults[0][0] is sym:
            return 'y (mặc định của choice)'
        return ''

    rows, seen = [], set()
    for node in kconf.node_iter():
        sym = node.item
        if not isinstance(sym, kl.Symbol) or sym.name in seen:
            continue
        if os.path.normcase(os.path.abspath(node.filename)) != PROJBUILD:
            continue
        seen.add(sym.name)
        rows.append((sym.name, kl.TYPE_TO_STR[sym.orig_type], menu_of(node),
                     node.prompt[0] if node.prompt else '', default_of(node)))

    out = ['# Kế hoạch Đồng bộ Menuconfig sang Web UI', '',
           'Danh sách này chứa tất cả các cấu hình Kconfig sẽ được ánh xạ sang Web UI. '
           'Các mục này sẽ ghi thẳng vào file `sdkconfig` để build.', '',
           '> Tệp tự sinh bằng `scripts/gen_kconfig_webui_doc.py` từ cây menu của '
           '`main/Kconfig.projbuild`. Không sửa tay.', '',
           '| Tên Kconfig (sdkconfig) | Kiểu dữ liệu | Danh mục (Menu) | Mô tả hiển thị (Prompt) | Giá trị mặc định |',
           '| --- | --- | --- | --- | --- |']
    for name, typ, menu, prompt, default in rows:
        out.append(f'| `CONFIG_{name}` | {typ} | {cell(menu)} | {cell(prompt)} | {cell(default)} |')
    for name, typ, prompt, default in IDF_ROWS:
        out.append(f'| `CONFIG_{name}` | {typ} | ESP-IDF System | {prompt} | {default} |')

    with open(DOC, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(out) + '\n')
    print(f'Đã ghi {len(rows) + len(IDF_ROWS)} dòng vào {os.path.relpath(DOC, ROOT)}', file=sys.stderr)


if __name__ == '__main__':
    main()
