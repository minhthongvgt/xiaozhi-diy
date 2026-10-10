#!/usr/bin/env python3

"""
Xiaozhi-ESP32 Web Configurator Local Bridge Server
Architecture: Web UI <-> sdkconfig.defaults  (= menuconfig output)
Board target: esp32s3-n16r8-custom
Zero-dependency — Python standard library only.
"""

import os
import sys
import json
import mimetypes
import subprocess
import threading
import time
import glob
import re
import string
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs, unquote

_HERE = os.path.dirname(os.path.abspath(__file__))
for _d in (_HERE, os.path.join(_HERE, "web-configurator"), os.path.join(_HERE, "tools", "web-configurator")):
    if os.path.isfile(os.path.join(_d, "sdkconfig_io.py")) and _d not in sys.path:
        sys.path.insert(0, _d)
import sdkconfig_io

os.environ["PYTHONUTF8"] = "1"
os.environ["PYTHONIOENCODING"] = "utf-8"
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")


def is_valid_xiaozhi_project(path: str) -> bool:
    """Kiểm tra một thư mục có phải là thư mục gốc của dự án Xiaozhi ESP-IDF hợp lệ không."""
    if not path or not isinstance(path, str) or not os.path.isdir(path):
        return False
    norm = os.path.normpath(os.path.abspath(path))
    has_cmake = os.path.isfile(os.path.join(norm, "CMakeLists.txt"))
    has_main = os.path.isdir(os.path.join(norm, "main"))
    has_sdkconfig = os.path.isfile(os.path.join(norm, "sdkconfig.defaults")) or os.path.isfile(os.path.join(norm, "sdkconfig"))
    has_boards = os.path.isdir(os.path.join(norm, "main", "boards")) or os.path.isfile(os.path.join(norm, "main", "Kconfig.projbuild"))
    return (has_cmake and (has_main or has_sdkconfig)) or (has_main and has_boards)


def scan_all_xiaozhi_projects(current_root=None):
    """Quét toàn diện máy tính để tìm các thư mục dự án Xiaozhi khả dụng."""
    results = []
    seen = set()

    def add_p(p, source):
        if not p or not isinstance(p, str):
            return
        norm = os.path.normpath(os.path.abspath(p.strip()))
        if norm.lower() in seen or not os.path.isdir(norm):
            return
        if is_valid_xiaozhi_project(norm):
            seen.add(norm.lower())
            results.append({
                "path": norm,
                "is_current": (norm.lower() == (current_root or "").lower()),
                "source": source
            })

    if current_root:
        add_p(current_root, "Thư mục hiện tại")

    script_dir = os.path.dirname(os.path.abspath(__file__))
    curr = script_dir
    for _ in range(5):
        add_p(curr, "Cấu trúc thư mục chứa file script")
        parent = os.path.dirname(curr)
        if parent == curr:
            break
        curr = parent

    cwd = os.getcwd()
    curr = cwd
    for _ in range(5):
        add_p(curr, "Thư mục làm việc hiện tại (CWD)")
        parent = os.path.dirname(curr)
        if parent == curr:
            break
        curr = parent

    known_candidates = [
        os.path.expanduser(r"~\esp\xiaozhi"),
        os.path.expanduser(r"~\xiaozhi"),
        os.path.expanduser(r"~\Desktop\xiaozhi"),
        os.path.expanduser(r"~\Documents\xiaozhi"),
        os.path.expanduser(r"~\projects\xiaozhi"),
    ]
    for k in known_candidates:
        add_p(k, "Thư mục người dùng")

    for env_k in ["XIAOZHI_PROJECT_ROOT", "ESP_PROJECT_ROOT", "PROJECT_ROOT"]:
        v = os.environ.get(env_k)
        if v:
            add_p(v, f"Biến môi trường ({env_k})")

    drives = [f"{d}:\\" for d in string.ascii_uppercase if os.path.exists(f"{d}:\\")]
    for drv in drives:
        for p in [
            os.path.join(drv, "esp", "xiaozhi"),
            os.path.join(drv, "xiaozhi"),
            os.path.join(drv, "projects", "xiaozhi"),
            os.path.join(drv, "src", "xiaozhi"),
        ]:
            add_p(p, f"Ổ đĩa {drv}")

    return results


class ProjectContext:
    """
    Quản lý ngữ cảnh đường dẫn thư mục gốc dự án và web-configurator động.
    Dù người dùng đặt thư mục ở bất kỳ đâu, hệ thống tự động tìm chính xác.
    """

    def __init__(self, cli_project=None, cli_web_dir=None):
        self.script_file = os.path.abspath(__file__)
        self.script_dir = os.path.dirname(self.script_file)
        self.root, self.source = self._resolve_initial_root(cli_project)
        self.web_dir = self._resolve_web_dir(cli_web_dir)
        self.project_config_file = os.path.join(self.web_dir, ".project_config.json")
        saved_root = self._load_saved_project_config()
        if saved_root and is_valid_xiaozhi_project(saved_root) and not cli_project:
            self.root = saved_root
            self.source = "Cấu hình tùy chỉnh đã lưu (.project_config.json)"
        self._update_paths()

    def _resolve_initial_root(self, cli_project=None):
        if cli_project and is_valid_xiaozhi_project(cli_project):
            return os.path.normpath(os.path.abspath(cli_project)), "Tham số dòng lệnh (--project)"

        for env_k in ["XIAOZHI_PROJECT_ROOT", "ESP_PROJECT_ROOT", "PROJECT_ROOT"]:
            v = os.environ.get(env_k)
            if v and is_valid_xiaozhi_project(v):
                return os.path.normpath(os.path.abspath(v)), f"Biến môi trường ({env_k})"

        curr = self.script_dir
        for _ in range(5):
            if is_valid_xiaozhi_project(curr):
                return curr, "Tự động nhận diện (Vị trí script)"
            parent = os.path.dirname(curr)
            if parent == curr:
                break
            curr = parent

        curr = os.getcwd()
        for _ in range(5):
            if is_valid_xiaozhi_project(curr):
                return curr, "Tự động nhận diện (CWD)"
            parent = os.path.dirname(curr)
            if parent == curr:
                break
            curr = parent

        for k in [os.path.expanduser(r"~\esp\xiaozhi"), os.path.expanduser(r"~\xiaozhi")]:
            if is_valid_xiaozhi_project(k):
                return os.path.normpath(os.path.abspath(k)), "Thư mục người dùng chuẩn"

        return self.script_dir, "Mặc định (Thư mục script)"

    def _resolve_web_dir(self, cli_web_dir=None):
        if cli_web_dir and os.path.isdir(cli_web_dir) and os.path.isfile(os.path.join(cli_web_dir, "index.html")):
            return os.path.normpath(os.path.abspath(cli_web_dir))

        candidates = [
            os.path.join(self.script_dir, "tools", "web-configurator"),
            self.script_dir,
            os.path.join(self.root, "tools", "web-configurator"),
            os.path.join(self.root, "web-configurator"),
            os.path.join(self.root, "src", "web-configurator"),
        ]
        for c in candidates:
            if os.path.isfile(os.path.join(c, "index.html")):
                return os.path.normpath(os.path.abspath(c))

        for root, dirs, files in os.walk(self.root):
            if "index.html" in files and "app.js" in files:
                return os.path.normpath(os.path.abspath(root))

        return os.path.normpath(os.path.abspath(candidates[0]))

    def _load_saved_project_config(self):
        if os.path.isfile(self.project_config_file):
            try:
                with open(self.project_config_file, "r", encoding="utf-8") as f:
                    cfg = json.load(f)
                    return cfg.get("project_root")
            except Exception:
                pass
        return None

    def _save_project_config(self, path):
        try:
            with open(self.project_config_file, "w", encoding="utf-8") as f:
                json.dump({
                    "project_root": path,
                    "updated_at": time.time()
                }, f, indent=2)
        except Exception:
            pass

    def set_root(self, new_root_path: str):
        if not new_root_path:
            return False, "Đường dẫn không được để trống"
        norm = os.path.normpath(os.path.abspath(new_root_path.strip()))
        if not is_valid_xiaozhi_project(norm):
            sub = [d for d in glob.glob(os.path.join(norm, "*")) if is_valid_xiaozhi_project(d)]
            if sub:
                norm = os.path.normpath(os.path.abspath(sub[0]))
            else:
                return False, f"Thư mục '{new_root_path}' không phải là dự án Xiaozhi ESP-IDF hợp lệ (thiếu CMakeLists.txt hoặc main/)."

        self.root = norm
        self.source = "Người dùng chỉ định trên Web UI"
        self._update_paths()
        self._save_project_config(self.root)
        self._sync_globals()
        return True, self.root

    def reset_root(self):
        if os.path.isfile(self.project_config_file):
            try:
                os.remove(self.project_config_file)
            except Exception:
                pass
        self.root, self.source = self._resolve_initial_root()
        self._update_paths()
        self._sync_globals()
        return True, self.root

    def _update_paths(self):
        self.sdkconfig_defaults = os.path.join(self.root, "sdkconfig.defaults.esp32s3")
        self.sdkconfig = os.path.join(self.root, "sdkconfig")
        self.idf_config_file = os.path.join(self.web_dir, ".idf_config.json")
        self.project_config_file = os.path.join(self.web_dir, ".project_config.json")

    def _sync_globals(self):
        global PROJECT_ROOT, SDKCONFIG_DEFAULTS, SDKCONFIG, WEB_DIR, IDF_CONFIG_FILE
        PROJECT_ROOT = self.root
        SDKCONFIG_DEFAULTS = self.sdkconfig_defaults
        SDKCONFIG = self.sdkconfig
        WEB_DIR = self.web_dir
        IDF_CONFIG_FILE = self.idf_config_file
        if 'build_job_manager' in globals():
            build_job_manager.project_root = self.root


project_ctx = ProjectContext()
PROJECT_ROOT          = project_ctx.root
SDKCONFIG_DEFAULTS    = project_ctx.sdkconfig_defaults
SDKCONFIG             = project_ctx.sdkconfig
WEB_DIR               = project_ctx.web_dir
IDF_CONFIG_FILE       = project_ctx.idf_config_file
DEFAULT_PORT          = 8080

UI_PREFIXES = (
    "CONFIG_BOARD_TYPE_",
    "CONFIG_ENABLE_CUSTOM_",
    "CONFIG_CUSTOM_",
    "CONFIG_LANGUAGE_",
    "CONFIG_FLASH_",
    "CONFIG_USE_AFE_WAKE_WORD",
    "CONFIG_USE_ESP_WAKE_WORD",
    "CONFIG_USE_CUSTOM_WAKE_WORD",
    "CONFIG_WAKE_WORD_DISABLED",
    "CONFIG_CUSTOM_WAKE_WORD",
    "CONFIG_SEND_WAKE_WORD_DATA",
    "CONFIG_WAKE_WORD_DETECTION_IN_LISTENING",
    "CONFIG_USE_AUDIO_PROCESSOR",
    "CONFIG_USE_DEVICE_AEC",
    "CONFIG_USE_SERVER_AEC",
    "CONFIG_USE_AUDIO_DEBUGGER",
    "CONFIG_AUDIO_DEBUG_UDP_SERVER",
    "CONFIG_USE_HOTSPOT_WIFI_PROVISIONING",
    "CONFIG_USE_ESP_BLUFI_WIFI_PROVISIONING",
    "CONFIG_BT_ENABLED",
    "CONFIG_BT_BLE_",
    "CONFIG_USE_DEFAULT_MESSAGE_STYLE",
    "CONFIG_USE_WECHAT_MESSAGE_STYLE",
    "CONFIG_USE_EMOTE_MESSAGE_STYLE",
    "CONFIG_USE_MULTILINE_CHAT_MESSAGE",
    "CONFIG_ENABLE_BUZZER",
    "CONFIG_BUZZER_PIN",
    "CONFIG_ENABLE_HAPTIC_MOTOR",
    "CONFIG_HAPTIC_PIN",
    "CONFIG_ENABLE_CUSTOM_SENSORS",
    "CONFIG_OTA_URL",
    "CONFIG_SPIRAM",
    "CONFIG_SPIRAM_MODE_",
    "CONFIG_SPIRAM_TYPE_",
    "CONFIG_SPIRAM_SPEED_",
    "CONFIG_ESPTOOLPY_FLASH",
    "CONFIG_ESP_DEFAULT_CPU_FREQ_",
    "CONFIG_ESP32S3_INSTRUCTION_CACHE_",
    "CONFIG_ESP32S3_DATA_CACHE_",
    "CONFIG_ESP_MAIN_TASK_STACK_SIZE",
    "CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE",
    "CONFIG_TOUCH_SUPPRESS_DEPRECATE_WARN",
    "CONFIG_PARTITION_TABLE_",
)



def read_active_config():
    """
    Cấu hình HIỆU LỰC đúng thứ tự ESP-IDF:
    sdkconfig.defaults < sdkconfig.defaults.esp32s3 < sdkconfig (file idf.py sinh ra).
    """
    content = sdkconfig_io.load_effective_text(PROJECT_ROOT)
    has = lambda n: os.path.isfile(os.path.join(PROJECT_ROOT, n))
    layers = [n for n in ("sdkconfig.defaults", sdkconfig_io.DEFAULTS_FILE, "sdkconfig") if has(n)]
    return {"source": " + ".join(layers) or "none", "content": content if layers else ""}


def read_sdkconfig_defaults():
    """Trả về nội dung thô của sdkconfig.defaults."""
    if os.path.isfile(SDKCONFIG_DEFAULTS):
        with open(SDKCONFIG_DEFAULTS, "r", encoding="utf-8") as f:
            return f.read()
    return ""


def write_sdkconfig_defaults(lines: list):
    """Ghi danh sách dòng ra sdkconfig.defaults."""
    content = "\n".join(lines)
    if not content.endswith("\n"):
        content += "\n"
    with open(SDKCONFIG_DEFAULTS, "w", encoding="utf-8") as f:
        f.write(content)


def sync_sdkconfig(entries: dict, lines: list = None):
    """
    Đồng bộ entries (dict CONFIG_->value) vào sdkconfig.
    Nếu sdkconfig chưa tồn tại, tự động sinh mới file sdkconfig từ cấu hình đã tạo.
    Nếu đã có, giữ nguyên các dòng hệ thống và cập nhật các dòng thuộc UI.
    """
    if not os.path.isfile(SDKCONFIG):
        if lines:
            content = "\n".join(lines)
            if not content.endswith("\n"):
                content += "\n"
            with open(SDKCONFIG, "w", encoding="utf-8") as f:
                f.write(content)
        return

    with open(SDKCONFIG, "r", encoding="utf-8") as f:
        original = f.readlines()

    kept = []
    for line in original:
        stripped = line.strip()
        key = None
        if stripped.startswith("# ") and stripped.endswith(" is not set"):
            key = stripped[2:-11].strip()
        elif "=" in stripped and not stripped.startswith("#"):
            key = stripped.split("=", 1)[0].strip()

        if key and any(key.startswith(p) for p in UI_PREFIXES):
            continue  
        kept.append(line)

    kept.append("\n# --- Cấu hình từ Web Configurator ---\n")
    for k, v in entries.items():
        if v == "n":
            kept.append(f"# {k} is not set\n")
        elif v == "y":
            kept.append(f"{k}=y\n")
        else:
            kept.append(f"{k}={v}\n")

    with open(SDKCONFIG, "w", encoding="utf-8") as f:
        f.writelines(kept)


def parse_lines_to_dict(lines: list) -> dict:
    """Chuyển danh sách dòng sdkconfig → dict {CONFIG_X: value}."""
    result = {}
    for line in lines:
        stripped = line.strip()
        if not stripped or stripped.startswith("##"):
            continue
        if stripped.startswith("# ") and stripped.endswith(" is not set"):
            result[stripped[2:-11].strip()] = "n"
        elif "=" in stripped and not stripped.startswith("#"):
            k, _, v = stripped.partition("=")
            result[k.strip()] = v.strip()
    return result



def detect_project_info():
    return {
        "success": True,
        "root_path": project_ctx.root,
        "web_dir": project_ctx.web_dir,
        "source": project_ctx.source,
        "is_valid_project": is_valid_xiaozhi_project(project_ctx.root),
        "board_type": "esp32s3-n16r8-custom",
        "has_sdkconfig_defaults": os.path.isfile(project_ctx.sdkconfig_defaults),
        "has_sdkconfig": os.path.isfile(project_ctx.sdkconfig),
        "all_detected": scan_all_xiaozhi_projects(project_ctx.root),
    }



def save_configuration(payload: dict) -> dict:
    """
    Nhận { "sdkconfig_lines": ["CONFIG_FOO=y", ...] }
      - cập nhật TẠI CHỖ sdkconfig.defaults.esp32s3 (giữ các dòng ngoài UI, không trùng key)
      - nếu đã set-target và có sdkconfig: cập nhật luôn sdkconfig để build dùng ngay
      - KHÔNG tự tạo sdkconfig giả: để `idf.py set-target esp32s3` / reconfigure sinh file chuẩn
      - từ chối nếu dự án đang ở target khác esp32s3
    """
    lines = payload.get("sdkconfig_lines")
    if not lines or not isinstance(lines, list) or not all(isinstance(x, str) for x in lines):
        return {"success": False, "error": "Thiếu hoặc sai định dạng sdkconfig_lines trong payload"}
    try:
        res = sdkconfig_io.save_config(PROJECT_ROOT, lines)
    except ValueError as e:
        return {"success": False, "error": str(e)}
    except Exception as e:
        return {"success": False, "error": str(e)}

    d, s = res["defaults"], res["sdkconfig"]
    msg = f"Đã ghi {sdkconfig_io.DEFAULTS_FILE} (đổi {d['changed']}, thêm {d['added']})"
    if s is not None:
        msg += " và đồng bộ vào sdkconfig"
    if res.get("needs_reconfigure"):
        msg += ". Chạy 'idf.py reconfigure' (hoặc build) để IDF chuẩn hoá sdkconfig"
    elif res.get("note"):
        msg += ". " + res["note"]
    return {"success": True, "message": msg, "needs_reconfigure": res.get("needs_reconfigure", False),
            "target": res["target"]}




def extract_idf_version(idf_dir: str) -> str:
    """Trích xuất phiên bản ESP-IDF từ esp_idf_version.h hoặc version.txt."""
    h_file = os.path.join(idf_dir, "components", "esp_common", "include", "esp_idf_version.h")
    if os.path.isfile(h_file):
        try:
            with open(h_file, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()
                major = re.search(r'#define\s+ESP_IDF_VERSION_MAJOR\s+(\d+)', content)
                minor = re.search(r'#define\s+ESP_IDF_VERSION_MINOR\s+(\d+)', content)
                patch = re.search(r'#define\s+ESP_IDF_VERSION_PATCH\s+(\d+)', content)
                if major and minor:
                    p = patch.group(1) if patch else "0"
                    return f"v{major.group(1)}.{minor.group(1)}.{p}"
        except Exception:
            pass

    v_file = os.path.join(idf_dir, "version.txt")
    if os.path.isfile(v_file):
        try:
            with open(v_file, "r", encoding="utf-8", errors="ignore") as f:
                v = f.read().strip()
                if v:
                    return v
        except Exception:
            pass

    return "ESP-IDF (chưa rõ phiên bản)"


def resolve_idf_path(raw_path: str):
    """
    Kiểm tra và chuẩn hóa một đường dẫn có phải là ESP-IDF không.
    Hỗ trợ cả trường hợp người dùng nhập C:\\idf mà thư mục con bên trong là esp-idf.
    Trả về (resolved_path, version) hoặc (None, error_msg).
    """
    if not raw_path or not isinstance(raw_path, str):
        return None, "Đường dẫn trống"

    clean = os.path.normpath(os.path.abspath(raw_path.strip().strip('"').strip("'")))
    if not os.path.isdir(clean):
        return None, f"Thư mục không tồn tại: {clean}"

    if os.path.isfile(os.path.join(clean, "tools", "idf.py")):
        ver = extract_idf_version(clean)
        return clean, ver

    sub = glob.glob(os.path.join(clean, "*", "tools", "idf.py"))
    if sub:
        found_root = os.path.dirname(os.path.dirname(sub[0]))
        ver = extract_idf_version(found_root)
        return found_root, ver

    return None, f"Không tìm thấy 'tools/idf.py' trong '{clean}' hoặc các thư mục con."


def get_custom_idf_config():
    """Đọc cấu hình đường dẫn ESP-IDF người dùng đã lưu thủ công."""
    if os.path.isfile(IDF_CONFIG_FILE):
        try:
            with open(IDF_CONFIG_FILE, "r", encoding="utf-8") as f:
                data = json.load(f)
                custom_path = data.get("idf_path")
                if custom_path:
                    res_path, ver = resolve_idf_path(custom_path)
                    if res_path:
                        return {"idf_path": res_path, "version": ver, "is_custom": True}
        except Exception:
            pass
    return None


def save_custom_idf_config(custom_path: str):
    """Lưu đường dẫn ESP-IDF tùy chỉnh vào file cấu hình."""
    res_path, ver = resolve_idf_path(custom_path)
    if not res_path:
        return {"success": False, "error": ver}
    try:
        with open(IDF_CONFIG_FILE, "w", encoding="utf-8") as f:
            json.dump({
                "idf_path": res_path,
                "version": ver,
                "updated_at": time.time()
            }, f, indent=2)
        return {"success": True, "idf_path": res_path, "version": ver, "message": f"Đã lưu đường dẫn ESP-IDF: {res_path} ({ver})"}
    except Exception as e:
        return {"success": False, "error": f"Lỗi ghi cấu hình: {str(e)}"}


def reset_custom_idf_config():
    """Xóa cấu hình tùy chỉnh để quay lại tự động dò tìm."""
    if os.path.isfile(IDF_CONFIG_FILE):
        try:
            os.remove(IDF_CONFIG_FILE)
        except Exception:
            pass
    return {"success": True, "message": "Đã khôi phục chế độ tự động dò tìm ESP-IDF"}


def scan_all_idf_installations():
    """
    Thuật toán dò tìm đa tầng toàn diện:
    1. Kiểm tra cấu hình tùy chỉnh đã lưu (.idf_config.json)
    2. Kiểm tra biến môi trường IDF_PATH, ESP_IDF, ESPRESSIF_IDF
    3. Đọc manifest từ trình cài đặt Espressif (%USERPROFILE%\\.espressif\\idf-env.json, espidf.json)
    4. Quét toàn bộ các ổ đĩa trên máy tính (C:\\, D:\\, E:\\...) với các mẫu thư mục chuẩn (C:\\idf, C:\\esp-idf, v.v.)
    5. Quét thư mục người dùng (%USERPROFILE%\\esp\\...)
    6. Dò tìm nông trên các thư mục gốc chứa 'idf' hoặc 'esp'
    """
    found = []
    seen_paths = set()

    def add_candidate(p, source):
        if not p:
            return
        res_p, ver = resolve_idf_path(p)
        if res_p and res_p.lower() not in seen_paths:
            seen_paths.add(res_p.lower())
            found.append({
                "path": res_p,
                "version": ver,
                "source": source
            })

    custom = get_custom_idf_config()
    if custom:
        add_candidate(custom["idf_path"], "Tùy chỉnh cá nhân (Saved Custom)")

    for env_k in ["IDF_PATH", "ESP_IDF", "ESPRESSIF_IDF"]:
        val = os.environ.get(env_k)
        if val:
            add_candidate(val, f"Biến môi trường ({env_k})")

    user_prof = os.environ.get("USERPROFILE", "")
    manifest_candidates = [
        os.path.join(user_prof, ".espressif", "idf-env.json"),
        os.path.join(user_prof, ".espressif", "espidf.json"),
        os.path.join(user_prof, "AppData", "Local", "Espressif", "idf-env.json"),
    ]
    for m in manifest_candidates:
        if os.path.isfile(m):
            try:
                with open(m, "r", encoding="utf-8") as f:
                    data = json.load(f)
                    installed = data.get("idfInstalled", {})
                    for item_k, item_v in installed.items():
                        if isinstance(item_v, dict) and "path" in item_v:
                            add_candidate(item_v["path"], f"Trình cài đặt Espressif ({os.path.basename(m)})")
                        elif isinstance(item_v, str):
                            add_candidate(item_v, f"Trình cài đặt Espressif ({os.path.basename(m)})")
            except Exception:
                pass

    drives = [f"{d}:\\" for d in string.ascii_uppercase if os.path.exists(f"{d}:\\")]
    for drv in drives:
        patterns = [
            os.path.join(drv, "idf"),
            os.path.join(drv, "idf", "*"),
            os.path.join(drv, "idf", "esp-idf"),
            os.path.join(drv, "idf", "v*", "esp-idf"),
            os.path.join(drv, "idf", "frameworks", "esp-idf*"),
            os.path.join(drv, "esp-idf"),
            os.path.join(drv, "esp-idf-*"),
            os.path.join(drv, "esp", "esp-idf"),
            os.path.join(drv, "esp", "*"),
            os.path.join(drv, "Espressif", "esp-idf"),
            os.path.join(drv, "Espressif", "v*", "esp-idf"),
            os.path.join(drv, "Espressif", "frameworks", "esp-idf*"),
            os.path.join(drv, "tools", "esp-idf"),
            os.path.join(drv, "tools", "idf"),
        ]
        for pat in patterns:
            for matched in glob.glob(pat):
                add_candidate(matched, f"Quét ổ đĩa ({drv})")

    if user_prof:
        u_patterns = [
            os.path.join(user_prof, "esp", "esp-idf"),
            os.path.join(user_prof, "esp", "*"),
            os.path.join(user_prof, "esp-idf"),
            os.path.join(user_prof, ".espressif", "esp-idf"),
            os.path.join(user_prof, "Desktop", "esp-idf*"),
            os.path.join(user_prof, "Documents", "esp-idf*"),
        ]
        for pat in u_patterns:
            for matched in glob.glob(pat):
                add_candidate(matched, "Thư mục người dùng (User Profile)")

    for drv in drives:
        try:
            for entry in os.scandir(drv):
                if entry.is_dir() and any(k in entry.name.lower() for k in ["idf", "espressif", "esp"]):
                    add_candidate(entry.path, f"Thư mục gốc ổ đĩa ({entry.name})")
        except Exception:
            pass

    return found


def find_profile_script_and_python(idf_path: str):
    """Tìm kịch bản kích hoạt PowerShell và Python venv tương thích với idf_path (dò tìm đa tầng trên mọi ổ đĩa)."""
    profile_script = None
    python_venv = None

    profile_candidates = []
    py_candidates = []

    idf_tools_env = os.environ.get("IDF_TOOLS_PATH")
    if idf_tools_env and os.path.isdir(idf_tools_env):
        profile_candidates.extend(glob.glob(os.path.join(idf_tools_env, "tools", "Microsoft.*.PowerShell_profile.ps1")))
        profile_candidates.extend(glob.glob(os.path.join(idf_tools_env, "Microsoft.*.PowerShell_profile.ps1")))
        py_candidates.extend(glob.glob(os.path.join(idf_tools_env, "tools", "python", "*", "venv", "Scripts", "python.exe")))
        py_candidates.extend(glob.glob(os.path.join(idf_tools_env, "python_env", "*", "Scripts", "python.exe")))

    drives = [f"{d}:\\" for d in string.ascii_uppercase if os.path.exists(f"{d}:\\")]
    for drv in drives:
        profile_candidates.extend(glob.glob(os.path.join(drv, "Espressif", "tools", "Microsoft.*.PowerShell_profile.ps1")))
        profile_candidates.extend(glob.glob(os.path.join(drv, "Espressif", "Microsoft.*.PowerShell_profile.ps1")))
        py_candidates.extend(glob.glob(os.path.join(drv, "Espressif", "tools", "python", "*", "venv", "Scripts", "python.exe")))
        
        profile_candidates.extend(glob.glob(os.path.join(drv, "idf", "tools", "Microsoft.*.PowerShell_profile.ps1")))
        profile_candidates.extend(glob.glob(os.path.join(drv, "idf", "Microsoft.*.PowerShell_profile.ps1")))
        py_candidates.extend(glob.glob(os.path.join(drv, "idf", "tools", "python", "*", "venv", "Scripts", "python.exe")))

    if idf_path:
        profile_candidates.extend(glob.glob(os.path.join(idf_path, "..", "tools", "Microsoft.*.PowerShell_profile.ps1")))
        profile_candidates.extend(glob.glob(os.path.join(idf_path, "..", "..", "tools", "Microsoft.*.PowerShell_profile.ps1")))
        py_candidates.extend(glob.glob(os.path.join(idf_path, ".venv", "Scripts", "python.exe")))
        py_candidates.extend(glob.glob(os.path.join(idf_path, "..", "python", "*", "venv", "Scripts", "python.exe")))

    user_prof = os.environ.get("USERPROFILE", "")
    if user_prof:
        profile_candidates.extend(glob.glob(os.path.join(user_prof, ".espressif", "tools", "Microsoft.*.PowerShell_profile.ps1")))
        profile_candidates.extend(glob.glob(os.path.join(user_prof, ".espressif", "Microsoft.*.PowerShell_profile.ps1")))
        py_candidates.extend(glob.glob(os.path.join(user_prof, ".espressif", "python_env", "*", "Scripts", "python.exe")))

    for p in profile_candidates:
        if p and os.path.isfile(p):
            profile_script = p
            break

    for py in py_candidates:
        if py and os.path.isfile(py):
            python_venv = py
            break

    return profile_script, python_venv


def detect_idf():
    """Tự động nhận diện ESP-IDF (kết hợp tùy chỉnh cá nhân và dò tìm đa tầng)."""
    custom = get_custom_idf_config()
    all_installations = scan_all_idf_installations()

    active_path = None
    active_version = "Chưa phát hiện"
    is_custom = False

    if custom:
        active_path = custom["idf_path"]
        active_version = custom["version"]
        is_custom = True
    elif all_installations:
        active_path = all_installations[0]["path"]
        active_version = all_installations[0]["version"]

    profile_script, python_venv = find_profile_script_and_python(active_path)

    return {
        "installed": active_path is not None,
        "idf_path": active_path,
        "version": active_version,
        "is_custom": is_custom,
        "profile_script": profile_script,
        "python_venv": python_venv,
        "all_found": all_installations
    }


def detect_target():
    """Nhận diện target thật sự đã set-target (sdkconfig > build/CMakeCache.txt > sdkconfig.defaults)."""
    return sdkconfig_io.detect_target(PROJECT_ROOT)


def detect_com_ports():
    """Tự động nhận diện danh sách cổng COM kết nối trên máy tính."""
    ports = []
    _, idf_py = find_profile_script_and_python(None)
    py_candidates = [idf_py, sys.executable]
    for exe in py_candidates:
        if exe and os.path.isfile(exe):
            try:
                cmd = [exe, "-c", "import serial.tools.list_ports as lp, json; print(json.dumps([[p.device, p.description] for p in lp.comports()]))"]
                out = subprocess.check_output(cmd, stderr=subprocess.DEVNULL, timeout=4).decode("utf-8").strip()
                items = json.loads(out)
                for dev, desc in items:
                    ports.append({"port": dev, "desc": desc})
                if ports:
                    return ports
            except Exception:
                pass

    try:
        ps_cmd = 'powershell -NoProfile -Command "[System.IO.Ports.SerialPort]::GetPortNames()"'
        out = subprocess.check_output(ps_cmd, shell=True, stderr=subprocess.DEVNULL, timeout=4).decode("utf-8")
        for line in out.strip().splitlines():
            p = line.strip()
            if p.startswith("COM"):
                ports.append({"port": p, "desc": p})
    except Exception:
        pass

    return ports



class BuildJobManager:
    """Quản lý thực thi các tác vụ Build & Flash trong tiến trình nền không gây đơ server."""

    def __init__(self, project_root):
        self.project_root = project_root
        self.lock = threading.Lock()
        self.process = None
        self.thread = None
        self.state = "idle"  
        self.current_action = None
        self.logs = []
        self.exit_code = None
        self.start_time = None
        self.end_time = None

    def get_status(self):
        with self.lock:
            return {
                "state": self.state,
                "action": self.current_action,
                "log_count": len(self.logs),
                "exit_code": self.exit_code,
                "start_time": self.start_time,
                "end_time": self.end_time,
            }

    def get_logs(self, offset=0):
        with self.lock:
            return {
                "state": self.state,
                "action": self.current_action,
                "exit_code": self.exit_code,
                "logs": self.logs[offset:],
                "next_offset": len(self.logs),
            }

    def clear_logs(self):
        with self.lock:
            if self.state == "running":
                return {"success": False, "error": "Không thể xóa log khi tiến trình đang chạy"}
            self.logs = []
            return {"success": True, "message": "Đã xóa log"}

    def cancel(self):
        with self.lock:
            if self.state != "running" or not self.process:
                return {"success": False, "error": "Không có tiến trình đang chạy"}
            pid = self.process.pid

        try:
            subprocess.run(f"taskkill /F /T /PID {pid}", shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except Exception:
            pass

        with self.lock:
            self.state = "cancelled"
            self.logs.append("[HỦY] Đã dừng tiến trình theo yêu cầu của người dùng.")
            self.end_time = time.time()

        return {"success": True, "message": "Đã gửi lệnh dừng tiến trình"}

    def start_job(self, action, idf_command, profile_script=None, idf_path=None):
        with self.lock:
            if self.state == "running":
                return {"success": False, "error": f"Đang có tiến trình '{self.current_action}' đang chạy. Vui lòng đợi hoặc bấm Hủy."}
            self.state = "running"
            self.current_action = action
            self.logs = [f"=== BẮT ĐẦU: {action.upper()} ===", f"Lệnh: {idf_command}"]
            if idf_path:
                self.logs.append(f"Đường dẫn ESP-IDF: {idf_path}")
            self.exit_code = None
            self.start_time = time.time()
            self.end_time = None

        def worker():
            if profile_script and os.path.isfile(profile_script):
                idf_override = f"$env:IDF_PATH = '{idf_path}'; " if idf_path else ""
                full_cmd = f"powershell -ExecutionPolicy Bypass -NoProfile -Command \". '{profile_script}'; {idf_override}{idf_command}\""
            else:
                full_cmd = idf_command

            try:
                proc = subprocess.Popen(
                    full_cmd,
                    cwd=self.project_root,
                    shell=True,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                    bufsize=1,
                    encoding="utf-8",
                    errors="replace"
                )
                with self.lock:
                    self.process = proc

                for line in iter(proc.stdout.readline, ""):
                    if not line:
                        break
                    clean_line = line.rstrip("\r\n")
                    with self.lock:
                        self.logs.append(clean_line)

                proc.stdout.close()
                code = proc.wait()

                with self.lock:
                    if self.state != "cancelled":
                        self.exit_code = code
                        self.state = "success" if code == 0 else "failed"
                        status_str = "THÀNH CÔNG" if code == 0 else f"THẤT BẠI (Mã thoát: {code})"
                        self.logs.append(f"=== TIẾN TRÌNH KẾT THÚC: {status_str} ===")
                    self.end_time = time.time()
                    self.process = None

            except Exception as e:
                with self.lock:
                    self.state = "failed"
                    self.logs.append(f"[LỖI TIẾN TRÌNH] {str(e)}")
                    self.end_time = time.time()
                    self.process = None

        self.thread = threading.Thread(target=worker, daemon=True)
        self.thread.start()
        return {"success": True, "message": f"Đã khởi động tiến trình '{action}'"}


build_job_manager = BuildJobManager(PROJECT_ROOT)


ALLOWED_TARGETS = {"esp32", "esp32s2", "esp32s3", "esp32c2", "esp32c3", "esp32c5", "esp32c6",
                   "esp32c61", "esp32h2", "esp32p4", "linux"}
_PORT_RE = re.compile(r"^(COM\d{1,3}|/dev/[A-Za-z0-9._/-]{1,40}|socket://[A-Za-z0-9.:_-]{1,60})$")
_BAUD_RE = re.compile(r"^\d{4,7}$")


def safe_port(v):
    v = (v or "").strip()
    return v if (not v or _PORT_RE.match(v)) else None


def safe_baud(v):
    v = str(v or "").strip()
    return v if (not v or _BAUD_RE.match(v)) else None


def origin_allowed(origin):
    if not origin:
        return True                      
    return re.match(r"^http://(localhost|127\.0\.0\.1)(:\d+)?$", origin) is not None



class ConfiguratorHandler(BaseHTTPRequestHandler):

    def log_message(self, fmt, *args):  
        pass

    def end_headers(self):
        origin = self.headers.get("Origin", "")
        if origin and origin_allowed(origin):
            self.send_header("Access-Control-Allow-Origin", origin)
            self.send_header("Vary", "Origin")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.send_header("Cache-Control", "no-cache, no-store, must-revalidate")
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200)
        self.end_headers()

    def do_GET(self):
        parsed = urlparse(self.path)
        path = parsed.path

        if path == "/api/project":
            return self._json(detect_project_info())

        if path == "/api/project/scan-all":
            return self._json({
                "success": True,
                "current_root": project_ctx.root,
                "projects": scan_all_xiaozhi_projects(project_ctx.root)
            })

        if path == "/api/config":
            return self._json(read_active_config())

        if path == "/api/idf/status":
            return self._json({
                "idf": detect_idf(),
                "target": detect_target(),
                "ports": detect_com_ports(),
                "job": build_job_manager.get_status()
            })

        if path == "/api/idf/scan-all":
            return self._json({
                "success": True,
                "installations": scan_all_idf_installations()
            })

        if path == "/api/idf/ports":
            return self._json({"ports": detect_com_ports()})

        if path == "/api/idf/logs":
            qs = parse_qs(parsed.query)
            try:
                offset = int(qs.get("offset", [0])[0])
            except ValueError:
                offset = 0
            return self._json(build_job_manager.get_logs(offset))

        file_path = self._resolve(path)
        if file_path:
            return self._serve_file(file_path)

        self.send_response(404)
        self.send_header("Content-Type", "text/plain; charset=utf-8")
        self.end_headers()
        self.wfile.write(f"404 Not Found: {path}".encode())

    def do_POST(self):
        path = urlparse(self.path).path
        if not origin_allowed(self.headers.get("Origin", "")):
            return self._json({"success": False, "error": "Origin không được phép"}, 403)
        if path in ("/api/save", "/api/save-tab"):
            length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(length).decode("utf-8")
            try:
                result = save_configuration(json.loads(body))
                return self._json(result, 200 if result["success"] else 500)
            except Exception as e:
                return self._json({"success": False, "error": str(e)}, 400)

        if path == "/api/project/set-root":
            length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(length).decode("utf-8") if length else "{}"
            try:
                data = json.loads(body)
            except Exception:
                data = {}
            target_path = data.get("root_path", "").strip()
            ok, res = project_ctx.set_root(target_path)
            if ok:
                build_job_manager.project_root = project_ctx.root
                return self._json({
                    "success": True,
                    "root_path": res,
                    "message": f"Đã kết nối thư mục dự án: {res}",
                    "project_info": detect_project_info()
                })
            else:
                return self._json({"success": False, "error": res}, 400)

        if path == "/api/project/reset-root":
            ok, res = project_ctx.reset_root()
            build_job_manager.project_root = project_ctx.root
            return self._json({
                "success": True,
                "root_path": res,
                "message": f"Đã khôi phục chế độ tự động dò tìm: {res}",
                "project_info": detect_project_info()
            })

        idf_info = detect_idf()
        profile_script = idf_info.get("profile_script")
        idf_path = idf_info.get("idf_path")

        if path == "/api/idf/set-path":
            length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(length).decode("utf-8") if length else "{}"
            try:
                data = json.loads(body)
            except Exception:
                data = {}
            target_path = data.get("path", "").strip()
            res = save_custom_idf_config(target_path)
            return self._json(res, 200 if res["success"] else 400)

        if path == "/api/idf/reset-path":
            res = reset_custom_idf_config()
            return self._json(res)

        if path == "/api/idf/build":
            result = build_job_manager.start_job("build", "idf.py build", profile_script, idf_path)
            return self._json(result, 200 if result["success"] else 400)

        if path == "/api/idf/flash":
            length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(length).decode("utf-8") if length else "{}"
            try:
                data = json.loads(body)
            except Exception:
                data = {}
            port = safe_port(data.get("port", ""))
            baud = safe_baud(data.get("baud", "460800"))
            if port is None or baud is None:
                return self._json({"success": False, "error": "Cổng COM hoặc baud không hợp lệ"}, 400)
            port_arg = f"-p {port}" if port else ""
            baud_arg = f"-b {baud}" if baud else ""
            cmd = f"idf.py {port_arg} {baud_arg} flash".strip()
            result = build_job_manager.start_job("flash", cmd, profile_script, idf_path)
            return self._json(result, 200 if result["success"] else 400)

        if path == "/api/idf/build-flash":
            length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(length).decode("utf-8") if length else "{}"
            try:
                data = json.loads(body)
            except Exception:
                data = {}
            port = safe_port(data.get("port", ""))
            baud = safe_baud(data.get("baud", "460800"))
            if port is None or baud is None:
                return self._json({"success": False, "error": "Cổng COM hoặc baud không hợp lệ"}, 400)
            port_arg = f"-p {port}" if port else ""
            baud_arg = f"-b {baud}" if baud else ""
            cmd = f"idf.py {port_arg} {baud_arg} build flash".strip()
            result = build_job_manager.start_job("build-flash", cmd, profile_script, idf_path)
            return self._json(result, 200 if result["success"] else 400)

        if path == "/api/idf/set-target":
            length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(length).decode("utf-8") if length else "{}"
            try:
                data = json.loads(body)
            except Exception:
                data = {}
            target = str(data.get("target", "esp32s3")).strip()
            if target not in ALLOWED_TARGETS:
                return self._json({"success": False, "error": f"Target '{target}' không hợp lệ"}, 400)
            cmd = f"idf.py set-target {target}"
            result = build_job_manager.start_job("set-target", cmd, profile_script, idf_path)
            return self._json(result, 200 if result["success"] else 400)

        if path == "/api/idf/clean":
            result = build_job_manager.start_job("clean", "idf.py clean", profile_script, idf_path)
            return self._json(result, 200 if result["success"] else 400)

        if path == "/api/idf/cancel":
            result = build_job_manager.cancel()
            return self._json(result)

        if path == "/api/idf/clear-logs":
            result = build_job_manager.clear_logs()
            return self._json(result)

        self.send_response(404)
        self.end_headers()


    def _json(self, data, status=200):
        body = json.dumps(data, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.end_headers()
        self.wfile.write(body)

    def _resolve(self, path):
        if path in ("/", "/index.html", "/configurator.html"):
            return os.path.join(project_ctx.web_dir, "index.html")
        path = unquote(path)
        if path.startswith("/tools/web-configurator/"):
            path = path[len("/tools/web-configurator/"):]
        base = os.path.realpath(project_ctx.web_dir)
        c = os.path.realpath(os.path.join(base, path.lstrip("/\\")))
        if (c == base or c.startswith(base + os.sep)) and os.path.isfile(c):
            return c
        return None

    def _serve_file(self, path):
        mime, _ = mimetypes.guess_type(path)
        if not mime:
            mime = {".js": "application/javascript", ".css": "text/css",
                    ".html": "text/html", ".json": "application/json"
                    }.get(os.path.splitext(path)[1].lower(), "text/plain")
        self.send_response(200)
        self.send_header("Content-Type", f"{mime}; charset=utf-8")
        self.end_headers()
        with open(path, "rb") as f:
            self.wfile.write(f.read())



def run_server(port=DEFAULT_PORT, project_path=None, web_dir_path=None):
    if project_path:
        project_ctx.set_root(project_path)
    if web_dir_path:
        project_ctx.web_dir = os.path.normpath(os.path.abspath(web_dir_path))
        project_ctx._update_paths()
        project_ctx._sync_globals()

    httpd = None
    for p in [port] + list(range(8080, 8100)) + [0]:
        try:
            httpd = HTTPServer(("127.0.0.1", p), ConfiguratorHandler)
            port = httpd.server_port
            break
        except OSError:
            continue

    if not httpd:
        print("[LỖI] Không khởi động được server!")
        sys.exit(1)

    print("=" * 65)
    print("   XIAOZHI WEB CONFIGURATOR  —  esp32s3-n16r8-custom")
    print("=" * 65)
    print(f"[*] Project : {project_ctx.root}")
    print(f"[*] Web UI  : {project_ctx.web_dir}")
    print(f"[*] Nguồn   : {project_ctx.source}")
    print(f"[*] URL     : http://localhost:{port}/")
    print(f"[*] Output  : {sdkconfig_io.DEFAULTS_FILE} + sdkconfig (nếu đã set-target)")
    print(f"[*] Ctrl+C  để dừng")
    print("=" * 65)

    try:
        import webbrowser
        webbrowser.open(f"http://localhost:{port}/")
    except Exception:
        pass

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n[*] Dừng server.")
        httpd.server_close()


if __name__ == "__main__":
    port = DEFAULT_PORT
    project_arg = None
    web_dir_arg = None

    args = sys.argv[1:]
    i = 0
    while i < len(args):
        arg = args[i]
        if arg in ("--project", "--dir", "--root") and i + 1 < len(args):
            project_arg = args[i + 1]
            i += 2
        elif arg == "--web-dir" and i + 1 < len(args):
            web_dir_arg = args[i + 1]
            i += 2
        elif arg in ("--port", "-p") and i + 1 < len(args):
            if args[i + 1].isdigit():
                port = int(args[i + 1])
            i += 2
        elif arg.isdigit():
            port = int(arg)
            i += 1
        else:
            i += 1

    run_server(port=port, project_path=project_arg, web_dir_path=web_dir_arg)

