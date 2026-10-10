#!/usr/bin/env python3
"""Dọn comment trong mã nguồn để chuẩn bị xuất bản.

Hỗ trợ: C/C++ (.c .h .cc .cpp .hpp), JavaScript (.js .mjs .cjs), CSS, HTML, Python.

Mặc định chỉ chạy thử (dry-run) và in báo cáo. Thêm --apply để ghi file;
trước khi ghi, các file bị thay đổi được sao lưu vào một file .zip.

Luôn giữ lại:
  - Header bản quyền / giấy phép (Copyright, SPDX, License, ...)
  - Comment chỉ thị công cụ (NOLINT, clang-format, eslint, noqa, type:, pylint, ...)
  - Comment /*! ... */, shebang và dòng khai báo encoding của Python
  - Docstring Python (là chuỗi, không phải comment)

Giữ nguyên dòng trống có sẵn, kiểu xuống dòng (CRLF/LF) và BOM. Chỉ xoá các dòng
trở nên trống do comment bị gỡ.

Ví dụ:
  python tools/clean_comments.py                    # xem trước toàn dự án
  python tools/clean_comments.py main --diff        # xem diff cho thư mục main
  python tools/clean_comments.py --apply            # ghi thay đổi (có sao lưu)
  python tools/clean_comments.py --apply --keep-doc # giữ comment tài liệu /** */ và ///
"""

import argparse
import ast
import difflib
import fnmatch
import io
import os
import re
import sys
import time
import tokenize
import zipfile

C_EXTS = {'.c', '.h', '.cc', '.cpp', '.cxx', '.hpp', '.hh', '.hxx', '.ino'}
JS_EXTS = {'.js', '.mjs', '.cjs'}
CSS_EXTS = {'.css'}
HTML_EXTS = {'.html', '.htm'}
PY_EXTS = {'.py'}
ALL_EXTS = C_EXTS | JS_EXTS | CSS_EXTS | HTML_EXTS | PY_EXTS

DEFAULT_EXCLUDES = [
    '.git', '.svn', 'build', 'build_*', 'managed_components', 'node_modules',
    'graphify-out', '.agents', '.claude', '.vscode', '.idea', '__pycache__',
    '.comment_backup', 'dist', '*.min.js', '*.min.css',
    'main/display/lvgl_display/gif/gifdec*',
]

LICENSE_RE = re.compile(
    r'copyright|spdx-|licen[sc]e[ds]?\b|all rights reserved|permission is hereby granted'
    r'|public domain|\(c\)\s*\d{4}|©', re.IGNORECASE)

DIRECTIVE_RE = re.compile(
    r'^\s*(?:'
    r'NOLINT|NOLINTNEXTLINE|NOLINTBEGIN|NOLINTEND|clang-format\s+(?:on|off)|clang-tidy'
    r'|cppcheck-suppress|coverity|LCOV_EXCL|IWYU\s+pragma|fallthrough|FALLTHROUGH|falls?\s+through'
    r'|eslint|jshint|jslint|globals?\s|prettier-ignore|@ts-|@jsx|@flow|istanbul|c8\s|webpackChunkName'
    r'|@license|@preserve|#\s*(?:sourceMappingURL|sourceURL)'
    r'|noqa|type:\s|pylint:|fmt:\s*(?:on|off|skip)|pragma:|isort:|mypy:|pyright:|nosec|pragma\s+no\s+cover'
    r')', re.IGNORECASE)

PY_CODING_RE = re.compile(r'^[ \t\f]*#.*?coding[:=][ \t]*[-\w.]+')

JS_REGEX_PRECEDERS = set('(,=:[!&|?{};+-*%<>~^')
JS_REGEX_KEYWORDS = {
    'return', 'typeof', 'instanceof', 'in', 'of', 'new', 'delete', 'void',
    'throw', 'case', 'do', 'else', 'yield', 'await',
}
IDENT_CHARS = re.compile(r'[A-Za-z0-9_$]')
RAW_PREFIXES = {'R', 'u8R', 'uR', 'UR', 'LR'}
RAW_OPEN_RE = re.compile(r'"([^()\\ \t\r\n]{0,16})\(')
HTML_RAW_RE = re.compile(r'<(script|style|pre|textarea)\b.*?</\1\s*>', re.IGNORECASE | re.DOTALL)

SENTINEL = '\x00'
NEWLINE_RE = re.compile(r'\r\n|\n|\r')


class Comment:
    __slots__ = ('start', 'end', 'block')

    def __init__(self, start, end, block):
        self.start = start
        self.end = end
        self.block = block


def scan_c_like(text, lang):
    """Trả về danh sách comment trong mã C/C++/JS/CSS. lang: 'c' | 'js' | 'css'."""
    comments = []
    n = len(text)
    i = 0
    line_comments = lang != 'css'
    # Ngăn xếp cho template literal JS: mỗi phần tử là độ sâu ngoặc { } trong ${...}
    tpl_stack = []
    last_sig = ''      # ký tự có nghĩa cuối cùng (bỏ qua khoảng trắng và comment)
    last_word = ''     # từ định danh cuối cùng, để nhận diện regex sau từ khoá

    def skip_quoted(j, quote):
        # Chuỗi kết thúc tại dấu nháy đóng hoặc xuống dòng không được escape
        while j < n:
            ch = text[j]
            if ch == '\\':
                j += 2
                continue
            if ch == quote:
                return j + 1
            if ch == '\n' and quote != '`':
                return j
            j += 1
        return n

    def skip_template(j):
        # j trỏ ngay sau dấu ` mở hoặc sau } đóng ${...}; trả về (vị trí, gặp_${)
        while j < n:
            ch = text[j]
            if ch == '\\':
                j += 2
                continue
            if ch == '`':
                return j + 1, False
            if ch == '$' and j + 1 < n and text[j + 1] == '{':
                return j + 2, True
            j += 1
        return n, False

    def skip_regex(j):
        in_class = False
        while j < n:
            ch = text[j]
            if ch == '\\':
                j += 2
                continue
            if ch == '\n':
                return j
            if in_class:
                if ch == ']':
                    in_class = False
            elif ch == '[':
                in_class = True
            elif ch == '/':
                j += 1
                while j < n and IDENT_CHARS.match(text[j]):
                    j += 1
                return j
            j += 1
        return n

    while i < n:
        ch = text[i]
        nxt = text[i + 1] if i + 1 < n else ''

        if ch == '/' and nxt == '/' and line_comments:
            j = i + 2
            while j < n:
                if text[j] == '\n':
                    # Trong C, dấu \ cuối dòng nối comment sang dòng sau
                    k = j - 1
                    if k >= i and text[k] == '\r':
                        k -= 1
                    if lang == 'c' and text[k] == '\\':
                        j += 1
                        continue
                    break
                j += 1
            if j < n and j > 0 and text[j - 1] == '\r':
                j -= 1
            comments.append(Comment(i, j, False))
            i = j
            continue

        if ch == '/' and nxt == '*':
            j = text.find('*/', i + 2)
            j = n if j < 0 else j + 2
            comments.append(Comment(i, j, True))
            i = j
            continue

        if ch in ' \t\r\n\f\v':
            i += 1
            continue

        if lang == 'c' and ch == '"':
            k = i - 1
            while k >= 0 and IDENT_CHARS.match(text[k]):
                k -= 1
            if text[k + 1:i] in RAW_PREFIXES:
                m = RAW_OPEN_RE.match(text, i)
                if m:
                    close = ')' + m.group(1) + '"'
                    j = text.find(close, m.end())
                    i = n if j < 0 else j + len(close)
                    last_sig, last_word = '"', ''
                    continue

        if ch == '"' or (ch == "'" and lang != 'c'):
            i = skip_quoted(i + 1, ch)
            last_sig, last_word = ch, ''
            continue

        if ch == "'" and lang == 'c':
            # Phân biệt dấu phân cách chữ số C++14 (1'000'000) với ký tự literal
            k = i - 1
            while k >= 0 and IDENT_CHARS.match(text[k]):
                k -= 1
            word = text[k + 1:i]
            if word and word[0].isdigit():
                i += 1
                continue
            i = skip_quoted(i + 1, ch)
            last_sig, last_word = ch, ''
            continue

        if lang == 'js':
            if ch == '`':
                i, opened = skip_template(i + 1)
                if opened:
                    tpl_stack.append(0)
                last_sig, last_word = ('{' if opened else '`'), ''
                continue
            if ch == '{' and tpl_stack:
                tpl_stack[-1] += 1
            elif ch == '}' and tpl_stack:
                if tpl_stack[-1] == 0:
                    tpl_stack.pop()
                    i, opened = skip_template(i + 1)
                    if opened:
                        tpl_stack.append(0)
                    last_sig, last_word = ('{' if opened else '`'), ''
                    continue
                tpl_stack[-1] -= 1
            if ch == '/':
                if last_sig == '' or last_sig in JS_REGEX_PRECEDERS or last_word in JS_REGEX_KEYWORDS:
                    i = skip_regex(i + 1)
                    last_sig, last_word = '/', ''
                    continue

        if IDENT_CHARS.match(ch):
            j = i
            while j < n and IDENT_CHARS.match(text[j]):
                j += 1
            last_word = text[i:j]
            last_sig = text[j - 1]
            i = j
            continue

        last_sig, last_word = ch, ''
        i += 1

    return comments


def scan_html(text):
    raw = [(m.start(), m.end()) for m in HTML_RAW_RE.finditer(text)]
    comments = []
    i = 0
    while True:
        j = text.find('<!--', i)
        if j < 0:
            break
        inside = next((e for s, e in raw if s < j < e), None)
        if inside is not None:
            i = inside
            continue
        e = text.find('-->', j + 4)
        e = len(text) if e < 0 else e + 3
        body = text[j + 4:e - 3]
        if not body.startswith('[if') and not body.startswith('<![endif'):
            comments.append(Comment(j, e, True))
        i = e
    return comments


def scan_python(text):
    comments = []
    offsets = [0]
    for m in NEWLINE_RE.finditer(text):
        offsets.append(m.end())
    tokens = tokenize.generate_tokens(io.StringIO(text).readline)
    for tok in tokens:
        if tok.type != tokenize.COMMENT:
            continue
        (sl, sc), (el, ec) = tok.start, tok.end
        start = offsets[sl - 1] + sc
        end = offsets[el - 1] + ec
        if sl == 1 and tok.string.startswith('#!'):
            continue
        if sl <= 2 and PY_CODING_RE.match(tok.string):
            continue
        comments.append(Comment(start, end, False))
    return comments


def comment_body(raw):
    if raw.startswith('<!--'):
        return raw[4:-3]
    if raw.startswith('/*'):
        return raw[2:-2] if raw.endswith('*/') else raw[2:]
    if raw.startswith('//'):
        return raw[2:]
    if raw.startswith('#'):
        return raw[1:]
    return raw


def leading_header(text, comments):
    """Các comment liền nhau ở đầu file (trước dòng code đầu tiên)."""
    header = []
    pos = 0
    for c in comments:
        gap = text[pos:c.start]
        if gap.strip() and not gap.strip().startswith('#!'):
            break
        header.append(c)
        pos = c.end
    return header


def select_removable(text, comments, keep_doc):
    keep = set()
    header = leading_header(text, comments)
    if header and any(LICENSE_RE.search(text[c.start:c.end]) for c in header):
        keep.update(id(c) for c in header)
    removable = []
    for c in comments:
        if id(c) in keep:
            continue
        raw = text[c.start:c.end]
        body = comment_body(raw)
        if raw.startswith('/*!'):
            continue
        if keep_doc and (raw.startswith('/**') or raw.startswith('///') or raw.startswith('//!')):
            continue
        if c.block and LICENSE_RE.search(body):
            continue
        if DIRECTIVE_RE.search(body.lstrip('/*!<# \t')):
            continue
        removable.append(c)
    return removable


def remove_spans(text, spans):
    out = []
    pos = 0
    for c in spans:
        out.append(text[pos:c.start])
        out.append(SENTINEL)
        pos = c.end
    out.append(text[pos:])
    joined = ''.join(out)

    parts = NEWLINE_RE.split(joined)
    seps = NEWLINE_RE.findall(joined) + ['']
    result = []
    dropped_since_kept = False
    removed_lines = 0
    for line, sep in zip(parts, seps):
        if SENTINEL in line:
            if not line.replace(SENTINEL, '').strip():
                removed_lines += 1
                dropped_since_kept = True
                continue
            line = clean_sentinel_line(line)
        elif not line.strip():
            # Không để lại nhiều dòng trống liên tiếp ở chỗ vừa xoá comment
            prev_blank = not result or not result[-1][0].strip()
            if dropped_since_kept and prev_blank:
                removed_lines += 1
                continue
        else:
            dropped_since_kept = False
        result.append((line, sep))

    # Bỏ dòng trống ở đầu file nếu header comment bị xoá
    if not text[:spans[0].start].strip():
        while result and not result[0][0].strip():
            result.pop(0)
            removed_lines += 1

    return ''.join(l + s for l, s in result), removed_lines


def clean_sentinel_line(line):
    pieces = line.split(SENTINEL)
    out = pieces[0]
    for piece in pieces[1:]:
        if not out.strip():
            # Comment ở đầu dòng: giữ thụt lề gốc
            out += piece.lstrip(' 	')
        elif not piece.strip():
            out = out.rstrip(' 	')
        else:
            # Giữ khoảng trắng để hai token không dính nhau (a/**/b -> a b)
            left = out.rstrip(' 	')
            right = piece.lstrip(' 	')
            gap = out[len(left):] or piece[:len(piece) - len(right)] or ' '
            out = left + gap + right
    return out.rstrip(' 	')


def code_signature(text, comments):
    """Chuỗi code đã bỏ comment và chuẩn hoá khoảng trắng, dùng để tự kiểm tra."""
    out = []
    pos = 0
    for c in comments:
        out.append(text[pos:c.start])
        out.append(' ')
        pos = c.end
    out.append(text[pos:])
    return ' '.join(''.join(out).split())


def language_of(path):
    ext = os.path.splitext(path)[1].lower()
    if ext in C_EXTS:
        return 'c'
    if ext in JS_EXTS:
        return 'js'
    if ext in CSS_EXTS:
        return 'css'
    if ext in HTML_EXTS:
        return 'html'
    if ext in PY_EXTS:
        return 'py'
    return None


def scan(text, lang):
    if lang == 'py':
        return scan_python(text)
    if lang == 'html':
        return scan_html(text)
    return scan_c_like(text, lang)


def clean_text(text, lang, keep_doc):
    comments = scan(text, lang)
    removable = select_removable(text, comments, keep_doc)
    if not removable:
        return text, 0, 0
    cleaned, removed_lines = remove_spans(text, removable)
    verify(text, cleaned, lang, comments, removable, keep_doc)
    return cleaned, len(removable), removed_lines


class VerifyError(Exception):
    pass


def verify(original, cleaned, lang, comments, removable, keep_doc):
    if lang == 'py':
        try:
            if ast.dump(ast.parse(original)) != ast.dump(ast.parse(cleaned)):
                raise VerifyError('AST Python thay đổi sau khi dọn')
        except SyntaxError as e:
            raise VerifyError(f'lỗi cú pháp Python: {e}')
        return
    if lang == 'html':
        return
    removed_ids = {id(c) for c in removable}
    expected = code_signature(original, [c for c in comments if id(c) in removed_ids])
    if ' '.join(cleaned.split()) != expected:
        raise VerifyError('nội dung code thay đổi ngoài phần comment')
    again = select_removable(cleaned, scan(cleaned, lang), keep_doc)
    if again:
        raise VerifyError('quét lại vẫn còn comment (lexer có thể đã hiểu sai)')


def read_source(path):
    with open(path, 'rb') as f:
        data = f.read()
    bom = data.startswith(b'\xef\xbb\xbf')
    if bom:
        data = data[3:]
    return data.decode('utf-8'), bom


def write_source(path, text, bom):
    data = text.encode('utf-8')
    if bom:
        data = b'\xef\xbb\xbf' + data
    with open(path, 'wb') as f:
        f.write(data)


def is_excluded(rel, patterns):
    rel = rel.replace('\\', '/')
    parts = rel.split('/')
    for pat in patterns:
        pat = pat.replace('\\', '/').strip('/')
        if '/' in pat:
            if fnmatch.fnmatch(rel, pat) or fnmatch.fnmatch(rel, pat + '/*') or rel.startswith(pat + '/'):
                return True
        elif any(fnmatch.fnmatch(p, pat) for p in parts):
            return True
    return False


def iter_files(paths, root, excludes, exts, self_path):
    for target in paths:
        if os.path.isfile(target):
            yield os.path.abspath(target)
            continue
        for dirpath, dirnames, filenames in os.walk(target):
            rel_dir = os.path.relpath(dirpath, root)
            dirnames[:] = [d for d in dirnames
                           if not is_excluded(os.path.normpath(os.path.join(rel_dir, d)), excludes)]
            for name in sorted(filenames):
                full = os.path.abspath(os.path.join(dirpath, name))
                if full == self_path:
                    continue
                if os.path.splitext(name)[1].lower() not in exts:
                    continue
                if is_excluded(os.path.relpath(full, root), excludes):
                    continue
                yield full


def load_ignore_file(root):
    path = os.path.join(root, '.cleancommentsignore')
    if not os.path.isfile(path):
        return []
    with open(path, encoding='utf-8') as f:
        return [l.strip() for l in f if l.strip() and not l.lstrip().startswith('#')]


def main():
    ap = argparse.ArgumentParser(
        description='Dọn comment trong mã nguồn (mặc định chỉ xem trước).',
        formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__.split('Ví dụ:')[1])
    ap.add_argument('paths', nargs='*', help='file/thư mục cần xử lý (mặc định: gốc dự án)')
    ap.add_argument('--apply', action='store_true', help='ghi thay đổi vào file')
    ap.add_argument('--no-backup', action='store_true', help='không tạo file sao lưu khi --apply')
    ap.add_argument('--diff', action='store_true', help='in unified diff')
    ap.add_argument('--keep-doc', action='store_true', help='giữ comment tài liệu /** */, ///, //!')
    ap.add_argument('--lang', action='append', choices=['c', 'js', 'css', 'html', 'py'],
                    help='chỉ xử lý ngôn ngữ này (lặp lại được)')
    ap.add_argument('--exclude', action='append', default=[], metavar='PATTERN',
                    help='loại trừ thêm (glob theo tên hoặc đường dẫn tương đối)')
    args = ap.parse_args()
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding='utf-8', errors='replace')
        except AttributeError:
            pass

    root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    paths = args.paths or [root]
    excludes = DEFAULT_EXCLUDES + load_ignore_file(root) + args.exclude
    exts = ALL_EXTS
    if args.lang:
        exts = {e for e in ALL_EXTS if language_of('x' + e) in args.lang}

    changes = []
    errors = []
    total_comments = total_lines = 0
    for path in iter_files(paths, root, excludes, exts, os.path.abspath(__file__)):
        rel = os.path.relpath(path, root)
        try:
            text, bom = read_source(path)
        except UnicodeDecodeError:
            errors.append((rel, 'không phải UTF-8, bỏ qua'))
            continue
        try:
            cleaned, n_comments, n_lines = clean_text(text, language_of(path), args.keep_doc)
        except (VerifyError, tokenize.TokenError, SyntaxError, IndentationError) as e:
            errors.append((rel, str(e)))
            continue
        if cleaned == text:
            continue
        changes.append((path, rel, text, cleaned, bom))
        total_comments += n_comments
        total_lines += n_lines
        print(f'  {rel}: {n_comments} comment, -{n_lines} dòng')
        if args.diff:
            sys.stdout.writelines(difflib.unified_diff(
                text.splitlines(True), cleaned.splitlines(True), 'a/' + rel, 'b/' + rel))

    print(f'\n{len(changes)} file, {total_comments} comment, {total_lines} dòng sẽ bị xoá.')
    for rel, msg in errors:
        print(f'  BỎ QUA {rel}: {msg}')

    if not changes:
        return 0
    if not args.apply:
        print('\nChế độ xem trước. Chạy lại với --apply để ghi thay đổi.')
        return 0

    if not args.no_backup:
        backup_dir = os.path.join(root, '.comment_backup')
        os.makedirs(backup_dir, exist_ok=True)
        backup = os.path.join(backup_dir, time.strftime('comments_%Y%m%d_%H%M%S.zip'))
        with zipfile.ZipFile(backup, 'w', zipfile.ZIP_DEFLATED) as z:
            for path, rel, *_ in changes:
                z.write(path, rel)
        print(f'Đã sao lưu {len(changes)} file vào {os.path.relpath(backup, root)}')

    for path, rel, text, cleaned, bom in changes:
        write_source(path, cleaned, bom)
    print(f'Đã ghi {len(changes)} file.')
    return 1 if errors else 0


if __name__ == '__main__':
    sys.exit(main())
