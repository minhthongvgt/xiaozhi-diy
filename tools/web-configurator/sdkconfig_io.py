"""
sdkconfig_io.py  —  phần "chuẩn ESP-IDF" cho Web Configurator (chỉ dùng thư viện chuẩn).

Ghép vào configurator_server.py:

    from sdkconfig_io import detect_target, load_effective_text, save_config

    GET  /api/idf/status  ->  {"target": detect_target(root), ...}
    GET  /api/config      ->  {"content": load_effective_text(root)}
    POST /api/save        ->  save_config(root, body["sdkconfig_lines"])

Quy tắc của ESP-IDF mà module này tuân theo
-------------------------------------------
1. Thứ tự ưu tiên giá trị:  Kconfig default < sdkconfig.defaults
                            < sdkconfig.defaults.<target> < sdkconfig (file sinh ra).
   => Nếu đã có `sdkconfig`, sửa mỗi sdkconfig.defaults KHÔNG có tác dụng khi build.
      Vì vậy save_config() cập nhật cả `sdkconfig` (để build dùng ngay) lẫn
      `sdkconfig.defaults` (để cấu hình sống sót qua set-target / xoá sdkconfig).
2. Target thật sự của dự án nằm ở CONFIG_IDF_TARGET trong `sdkconfig`
   (và IDF_TARGET trong build/CMakeCache.txt), do `idf.py set-target` ghi.
   Không đoán target từ tên thư mục hay từ UI.
3. Không đổi target ngầm: nếu target khác esp32s3, từ chối ghi vào `sdkconfig`
   (set-target sẽ xoá sdkconfig và build/, người dùng phải chủ động).
4. Sau khi ghi nên chạy `idf.py reconfigure` để kconfgen chuẩn hoá file
   (tự bỏ symbol mà `depends on` không thỏa) — hàm trả về cờ needs_reconfigure.
"""
from __future__ import annotations

import os
import re
import shutil
import tempfile
from typing import Dict, Iterable, List, Optional

WANTED_TARGET = "esp32s3"
DEFAULTS_FILE = f"sdkconfig.defaults.{WANTED_TARGET}"
P4_TARGET = "esp32p4"
P4_BUILD_DIR = "build_p4"
P4_DEFAULTS_FILE = f"sdkconfig.defaults.{P4_TARGET}"
P4_BOARD_DIR = "main/boards/esp32-p4"
P4_BOARDS = {
    "guition-m3-dev": "Guition JC-ESP32P4-M3-Dev",
    "function-ev": "Espressif ESP32-P4-Function-EV-Board",
    "waveshare-dev-kit": "Waveshare ESP32-P4-Module-DEV-KIT",
}

_SET_RE = re.compile(r"^(CONFIG_[A-Za-z0-9_]+)=(.*)$")
_UNSET_RE = re.compile(r"^# (CONFIG_[A-Za-z0-9_]+) is not set\s*$")


def _read(path: str) -> Optional[str]:
    try:
        with open(path, "r", encoding="utf-8", errors="replace", newline="") as f:
            return f.read()
    except OSError:
        return None


def parse_kconfig_text(text: str) -> Dict[str, str]:
    """text -> {CONFIG_X: 'y' | 'n' | '<giá trị thô>'} (giữ thứ tự xuất hiện)."""
    out: Dict[str, str] = {}
    for raw in text.splitlines():
        line = raw.strip()
        m = _UNSET_RE.match(line)
        if m:
            out[m.group(1)] = "n"
            continue
        m = _SET_RE.match(line)
        if m:
            out[m.group(1)] = m.group(2).strip()
    return out


def _unquote(v: str) -> str:
    v = v.strip()
    return v[1:-1] if len(v) >= 2 and v[0] == v[-1] == '"' else v


def detect_target(project_root: str, build_dir: str = "build", sdkconfig_rel: str = "sdkconfig",
                  wanted: str = WANTED_TARGET) -> dict:
    """
    Trả về dict khớp với front-end (buildController.renderStatus):
        is_set   : True nếu IDF đã thực sự set-target (có sdkconfig hoặc CMakeCache)
        target   : 'esp32s3' | 'esp32' | ... | None
        matches  : target == esp32s3
        source   : nguồn dùng để kết luận
        declared : target khai báo trong sdkconfig.defaults* (chưa chắc đã set-target)
        idf_version : phiên bản IDF ghi trong header sdkconfig (nếu có)
        conflicts: danh sách mâu thuẫn giữa các nguồn
    """
    found: Dict[str, str] = {}

    sdk = _read(os.path.join(project_root, sdkconfig_rel))
    idf_version = None
    if sdk is not None:
        m = re.search(r"^CONFIG_IDF_TARGET=\"?([a-z0-9]+)\"?\s*$", sdk, re.M)
        if m:
            found["sdkconfig"] = m.group(1)
        hv = re.search(r"Espressif IoT Development Framework \(ESP-IDF\)\s+(\S+)\s+Project Configuration", sdk)
        if hv:
            idf_version = hv.group(1)

    cache = _read(os.path.join(project_root, build_dir, "CMakeCache.txt"))
    if cache is not None:
        m = re.search(r"^IDF_TARGET:[A-Z]+=([a-z0-9]+)\s*$", cache, re.M)
        if m:
            found["build/CMakeCache.txt"] = m.group(1)

    declared = None
    defaults_path = os.path.join(project_root, "sdkconfig.defaults")
    defaults_text = _read(defaults_path)
    if defaults_text is not None:
        m = re.search(r'^CONFIG_IDF_TARGET="?([a-z0-9]+)"?\s*$', defaults_text, re.M)
        if m:
            declared = m.group(1)

    env = os.environ.get("IDF_TARGET")

    target, source = None, None
    for src in ("sdkconfig", "build/CMakeCache.txt"):   
        if src in found:
            target, source = found[src], src
            break
    is_set = target is not None
    if not is_set:
        target, source = (declared, "sdkconfig.defaults") if declared else ((env, "env:IDF_TARGET") if env else (None, None))

    conflicts: List[str] = []
    if len(set(found.values())) > 1:
        conflicts.append("sdkconfig và build/CMakeCache.txt khác target: "
                         + ", ".join(f"{k}={v}" for k, v in found.items())
                         + " — chạy 'idf.py set-target' để đồng bộ")
    if is_set and declared and declared != target:
        conflicts.append(f"sdkconfig.defaults khai báo {declared} nhưng sdkconfig đang là {target}")

    return {
        "is_set": is_set,
        "target": target,
        "matches": target == wanted,
        "source": source,
        "declared": declared,
        "idf_version": idf_version,
        "conflicts": conflicts,
    }


def load_effective(project_root: str) -> Dict[str, str]:
    merged: Dict[str, str] = {}
    defaults_path = os.path.join(project_root, "sdkconfig.defaults")
    defaults_text = _read(defaults_path)
    if defaults_text is not None:
        merged.update(parse_kconfig_text(defaults_text))
        target_defaults = _read(
            os.path.join(project_root, f"sdkconfig.defaults.{WANTED_TARGET}")
        )
        if target_defaults is not None:
            merged.update(parse_kconfig_text(target_defaults))
    sdkconfig_text = _read(os.path.join(project_root, "sdkconfig"))
    if sdkconfig_text is not None:
        merged.update(parse_kconfig_text(sdkconfig_text))
    return merged


def load_effective_text(project_root: str) -> str:
    """Text dạng sdkconfig để front-end SdkconfigParser.parse() đọc trực tiếp."""
    lines = []
    for k, v in load_effective(project_root).items():
        lines.append(f"# {k} is not set" if v == "n" else f"{k}={v}")
    return "\n".join(lines) + "\n"


def _atomic_write(path: str, text: str, newline: str) -> None:
    d = os.path.dirname(path) or "."
    fd, tmp = tempfile.mkstemp(prefix=".tmp_sdkconfig_", dir=d)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="") as f:
            f.write(text.replace("\r\n", "\n").replace("\n", newline))
        os.replace(tmp, path)
    except BaseException:
        if os.path.exists(tmp):
            os.unlink(tmp)
        raise


def _normalize_lines(lines: Iterable[str]) -> List[str]:
    """Chỉ giữ dòng CONFIG hợp lệ (bỏ comment trang trí / dòng rỗng), giữ thứ tự, bỏ trùng."""
    seen: Dict[str, str] = {}
    for raw in lines:
        s = raw.strip()
        m = _UNSET_RE.match(s) or _SET_RE.match(s)
        if m:
            key = m.group(1)
            seen[key] = "# %s is not set" % key if _UNSET_RE.match(s) else s
    return list(seen.values())


def apply_to_kconfig_file(path: str, lines: Iterable[str], skip_keys: Iterable[str] = ()) -> dict:
    """
    Cập nhật TẠI CHỖ một file kiểu sdkconfig: giữ nguyên mọi dòng/comment khác,
    thay dòng cũ của cùng key (cả dạng '=value' lẫn '# ... is not set'),
    key chưa có thì thêm vào cuối. Tạo <path>.bak trước khi ghi.
    """
    skip = set(skip_keys)
    wanted: Dict[str, str] = {}
    for ln in _normalize_lines(lines):
        m = _UNSET_RE.match(ln) or _SET_RE.match(ln)
        if m.group(1) not in skip:
            wanted[m.group(1)] = ln

    old = _read(path)
    newline = "\r\n" if (old and "\r\n" in old) else "\n"
    src = old.replace("\r\n", "\n").split("\n") if old else []
    if src and src[-1] == "":
        src.pop()

    out: List[str] = []
    done = set()
    changed = 0
    for ln in src:
        s = ln.strip()
        m = _UNSET_RE.match(s) or _SET_RE.match(s)
        if m and m.group(1) in wanted:
            key = m.group(1)
            if key in done:          
                changed += 1
                continue
            done.add(key)
            if wanted[key] != s:
                changed += 1
            out.append(wanted[key])
        else:
            out.append(ln)

    added = [v for k, v in wanted.items() if k not in done]
    if added:
        if out and out[-1].strip():
            out.append("")
        out.append("# --- Xiaozhi Web Configurator ---")
        out.extend(added)

    new_text = "\n".join(out) + "\n"
    if old is not None and old.replace("\r\n", "\n") == new_text:
        return {"path": path, "changed": 0, "added": 0, "written": False}
    if old is not None:
        shutil.copyfile(path, path + ".bak")
    _atomic_write(path, new_text, newline)
    return {"path": path, "changed": changed, "added": len(added), "written": True}


def save_config(project_root: str, sdkconfig_lines: Iterable[str]) -> dict:
    """
    Điểm vào cho POST /api/save.
      - luôn cập nhật sdkconfig.defaults  (nguồn cấu hình bền vững)
      - nếu đã có `sdkconfig`: cập nhật luôn để build dùng ngay (chỉ khi target = esp32s3)
    """
    info = detect_target(project_root)
    lines = list(sdkconfig_lines)

    if info["is_set"] and not info["matches"]:
        raise ValueError(
            f"Dự án đang set-target '{info['target']}', không phải {WANTED_TARGET}. "
            f"Chạy 'idf.py set-target {WANTED_TARGET}' trước (lệnh này xoá sdkconfig và build/)."
        )

    res = {"defaults": apply_to_kconfig_file(
        os.path.join(project_root, DEFAULTS_FILE), lines, skip_keys=())}

    sdk_path = os.path.join(project_root, "sdkconfig")
    if info["is_set"] and os.path.exists(sdk_path):
        res["sdkconfig"] = apply_to_kconfig_file(sdk_path, lines, skip_keys=("CONFIG_IDF_TARGET",))
        res["needs_reconfigure"] = bool(res["sdkconfig"]["written"])
    else:
        res["sdkconfig"] = None
        res["needs_reconfigure"] = False
        res["note"] = ("Chưa có sdkconfig: chạy 'idf.py set-target esp32s3' để IDF sinh sdkconfig "
                       "từ sdkconfig.defaults.")
    res["target"] = info
    return res


def p4_defaults_list(board: str) -> str:
    return f"sdkconfig.defaults;{P4_BOARD_DIR}/sdkconfig.{board}"


def p4_sdkconfig_rel() -> str:
    return f"{P4_BUILD_DIR}/sdkconfig"


P4_BOARD_SYMBOLS = {
    "guition-m3-dev": "CONFIG_BOARD_TYPE_GUITION_ESP32_P4_M3_DEV",
    "function-ev": "CONFIG_BOARD_TYPE_ESP32_P4_FUNCTION_EV_BOARD",
    "waveshare-dev-kit": "CONFIG_BOARD_TYPE_WAVESHARE_ESP32_P4_DEV_KIT",
}


def configured_p4_board(project_root: str) -> Optional[str]:
    text = _read(os.path.join(project_root, p4_sdkconfig_rel()))
    if text is None:
        return None
    values = parse_kconfig_text(text)
    for board, symbol in P4_BOARD_SYMBOLS.items():
        if values.get(symbol) == "y":
            return board
    return None


def reset_p4_sdkconfig(project_root: str) -> bool:
    sdk_path = os.path.join(project_root, p4_sdkconfig_rel())
    if not os.path.exists(sdk_path):
        return False
    os.replace(sdk_path, sdk_path + ".old")
    return True


def detect_target_p4(project_root: str, board: Optional[str] = None) -> dict:
    info = detect_target(project_root, P4_BUILD_DIR, p4_sdkconfig_rel(), P4_TARGET)
    if not info["is_set"]:
        info["target"], info["source"] = None, None
    info["conflicts"] = [c for c in info["conflicts"] if not c.startswith("sdkconfig.defaults khai báo")]
    configured = configured_p4_board(project_root)
    info["configured_board"] = configured
    if board and configured and configured != board:
        info["conflicts"].append(
            f"{p4_sdkconfig_rel()} đang là bo {P4_BOARDS.get(configured, configured)}, "
            f"khác bo đã chọn {P4_BOARDS.get(board, board)} — bấm set-target esp32p4 để sinh lại cấu hình")
    return info


def load_effective_p4(project_root: str, board: str) -> Dict[str, str]:
    merged: Dict[str, str] = {}
    for rel in ("sdkconfig.defaults", P4_DEFAULTS_FILE, f"{P4_BOARD_DIR}/sdkconfig.{board}", p4_sdkconfig_rel()):
        text = _read(os.path.join(project_root, rel))
        if text is not None:
            merged.update(parse_kconfig_text(text))
    return merged


def load_effective_text_p4(project_root: str, board: str) -> str:
    lines = []
    for k, v in load_effective_p4(project_root, board).items():
        lines.append(f"# {k} is not set" if v == "n" else f"{k}={v}")
    return "\n".join(lines) + "\n"


def save_config_p4(project_root: str, sdkconfig_lines: Iterable[str]) -> dict:
    info = detect_target_p4(project_root)
    lines = [ln for ln in _normalize_lines(sdkconfig_lines)
             if not re.search(r"CONFIG_(BOARD_TYPE_|IDF_TARGET)", ln)]
    res = {"defaults": apply_to_kconfig_file(os.path.join(project_root, P4_DEFAULTS_FILE), lines)}
    sdk_path = os.path.join(project_root, p4_sdkconfig_rel())
    if info["is_set"] and os.path.exists(sdk_path):
        res["sdkconfig"] = apply_to_kconfig_file(sdk_path, lines, skip_keys=("CONFIG_IDF_TARGET",))
        res["needs_reconfigure"] = bool(res["sdkconfig"]["written"])
    else:
        res["sdkconfig"] = None
        res["needs_reconfigure"] = False
        res["note"] = f"Chưa có {p4_sdkconfig_rel()}: bấm set-target esp32p4 để IDF sinh cấu hình cho bo ESP32-P4."
    res["target"] = info
    return res
