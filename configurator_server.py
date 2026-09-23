#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Xiaozhi-ESP32 Web Configurator Local Bridge Server
Target: ESP32-S3-N16R8
Zero-dependency server using Python standard library (http.server)
Automatically discovers dynamic project root path wherever it is placed.
"""

import os
import sys
import json
import mimetypes
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs

def sync_sdkconfig(sdkconfig_path, appends):
    ui_prefixes = [
        "CONFIG_BOARD_TYPE_",
        "CONFIG_LANGUAGE_",
        "CONFIG_FLASH_",
        "CONFIG_USE_",
        "CONFIG_ENABLE_CUSTOM_",
        "CONFIG_CUSTOM_",
        "CONFIG_ESPTOOLPY_FLASH",
        "CONFIG_PARTITION_TABLE_",
        "CONFIG_ESP_DEFAULT_CPU_FREQ_",
        "CONFIG_ESP32S3_INSTRUCTION_CACHE_",
        "CONFIG_ESP32S3_DATA_CACHE_",
        "CONFIG_DISPLAY_TYPE_",
        "CONFIG_TOUCH_",
        "CONFIG_AUDIO_",
        "CONFIG_SPIRAM_MODE_",
        "CONFIG_SPIRAM_TYPE_",
        "CONFIG_SPIRAM_SPEED_"
    ]
    ui_exact_keys = {
        "CONFIG_SPIRAM",
        "CONFIG_SPIRAM_MODE_OCT",
        "CONFIG_SPIRAM_MODE_QUAD",
        "CONFIG_SPIRAM_TYPE_ESP32S3_SPI_RAM_OCT",
        "CONFIG_SPIRAM_TYPE_ESP32S3_SPI_RAM_QUAD",
        "CONFIG_SPIRAM_SPEED_80M",
        "CONFIG_SPIRAM_SPEED_40M"
    }

    append_dict = {}
    for line in appends:
        line = line.strip()
        if not line or (line.startswith("# ") and not line.endswith(" is not set")): continue
        if line.startswith("# ") and line.endswith(" is not set"):
            key = line[2:-11]
            append_dict[key] = "n"
        elif "=" in line:
            k, v = line.split("=", 1)
            append_dict[k] = v

    lines = []
    if os.path.exists(sdkconfig_path):
        with open(sdkconfig_path, "r", encoding="utf-8") as f:
            lines = f.readlines()

    new_lines = []
    for line in lines:
        stripped = line.strip()
        if not stripped:
            new_lines.append(line)
            continue
            
        key = None
        if stripped.startswith("# ") and stripped.endswith(" is not set"):
            key = stripped[2:-11]
        elif "=" in stripped and not stripped.startswith("#"):
            key = stripped.split("=", 1)[0]
            
        if key:
            is_managed = (key in ui_exact_keys or key in append_dict)
            if not is_managed:
                for pref in ui_prefixes:
                    if key.startswith(pref):
                        is_managed = True
                        break
            if is_managed:
                continue
                
        new_lines.append(line)
        
    new_lines.append("\n# --- CẤU HÌNH TỪ WEB CONFIGURATOR ---\n")
    for k, v in append_dict.items():
        if v == "y":
            new_lines.append(f"{k}=y\n")
        elif v == "n":
            new_lines.append(f"# {k} is not set\n")
        else:
            new_lines.append(f"{k}={v}\n")
            
    with open(sdkconfig_path, "w", encoding="utf-8") as f:
        f.writelines(new_lines)

# Xác định đường dẫn thư mục gốc thực tế động theo vị trí file đang chạy
PROJECT_ROOT = os.path.abspath(os.path.dirname(__file__))

DEFAULT_PORT = 8080

def detect_project_info():
    """Kiểm tra cấu trúc thư mục mã nguồn tại vị trí thực tế."""
    has_main = os.path.isdir(os.path.join(PROJECT_ROOT, "main"))
    has_boards = os.path.isdir(os.path.join(PROJECT_ROOT, "main", "boards"))
    has_kconfig = os.path.isfile(os.path.join(PROJECT_ROOT, "main", "Kconfig.projbuild"))
    has_cmake = os.path.isfile(os.path.join(PROJECT_ROOT, "main", "CMakeLists.txt"))
    
    # Kiểm tra cấu hình hiện có
    custom_dir = os.path.join(PROJECT_ROOT, "main", "boards", "custom-s3-n16r8")
    sample_dir = os.path.join(PROJECT_ROOT, "main", "boards", "esp32s3-n16r8-custom")
    
    has_custom_config = os.path.isfile(os.path.join(custom_dir, "config.h")) or os.path.isfile(os.path.join(custom_dir, "config.json"))
    has_sample_config = os.path.isfile(os.path.join(sample_dir, "config.h")) or os.path.isfile(os.path.join(sample_dir, "config.json"))
    
    return {
        "success": True,
        "root_path": PROJECT_ROOT,
        "is_valid_project": has_main and has_boards and (has_kconfig or has_cmake),
        "has_custom_config": has_custom_config,
        "has_sample_config": has_sample_config,
        "board_type": "custom-s3-n16r8"
    }

def read_current_config():
    """Tự động đọc cấu hình hiện có từ thư mục mã nguồn thực tế."""
    custom_dir = os.path.join(PROJECT_ROOT, "main", "boards", "custom-s3-n16r8")
    sample_dir = os.path.join(PROJECT_ROOT, "main", "boards", "esp32s3-n16r8-custom")
    
    # Ưu tiên 1: custom-s3-n16r8/config.h (Cấu hình tùy chỉnh của người dùng)
    custom_h = os.path.join(custom_dir, "config.h")
    if os.path.isfile(custom_h):
        try:
            with open(custom_h, "r", encoding="utf-8") as f:
                return {"source": "custom_h", "content": f.read()}
        except Exception:
            pass

    # Ưu tiên 2: esp32s3-n16r8-custom/config.h (Cấu hình mẫu chuẩn của bo mạch)
    sample_h = os.path.join(sample_dir, "config.h")
    if os.path.isfile(sample_h):
        try:
            with open(sample_h, "r", encoding="utf-8") as f:
                return {"source": "sample_h", "content": f.read()}
        except Exception:
            pass

    # Ưu tiên 3: custom-s3-n16r8/config.json
    custom_json = os.path.join(custom_dir, "config.json")
    if os.path.isfile(custom_json):
        try:
            with open(custom_json, "r", encoding="utf-8") as f:
                return {"source": "custom_json", "data": json.load(f)}
        except Exception:
            pass

    # Ưu tiên 4: esp32s3-n16r8-custom/config.json
    sample_json = os.path.join(sample_dir, "config.json")
    if os.path.isfile(sample_json):
        try:
            with open(sample_json, "r", encoding="utf-8") as f:
                return {"source": "sample_json", "data": json.load(f)}
        except Exception:
            pass

    return {"source": "default", "data": None}

def save_configuration_to_disk(payload):
    """Ghi trực tiếp mã nguồn cấu hình vào thư mục mã nguồn thực tế."""
    try:
        custom_dir = os.path.join(PROJECT_ROOT, "main", "boards", "custom-s3-n16r8")
        os.makedirs(custom_dir, exist_ok=True)
        
        # 1. Ghi config.h
        if payload.get("config_h") and len(payload["config_h"].strip()) > 0:
            with open(os.path.join(custom_dir, "config.h"), "w", encoding="utf-8") as f:
                f.write(payload["config_h"])
                
        # 2. Ghi config.json
        if payload.get("config_json") and len(payload["config_json"].strip()) > 0:
            with open(os.path.join(custom_dir, "config.json"), "w", encoding="utf-8") as f:
                f.write(payload["config_json"])
            
            # 2.1 Trích xuất sdkconfig_append và ghi đè sdkconfig.defaults ở gốc
            try:
                cjson = json.loads(payload["config_json"])
                if "builds" in cjson and len(cjson["builds"]) > 0:
                    sdk_append = cjson["builds"][0].get("sdkconfig_append", [])
                    if sdk_append:
                        # Luôn đảm bảo Target được set cứng thành esp32s3 để tránh build nhầm chip
                        if not any("CONFIG_IDF_TARGET=" in line for line in sdk_append):
                            sdk_append.insert(0, 'CONFIG_IDF_TARGET="esp32s3"')
                            
                        sdk_path = os.path.join(PROJECT_ROOT, "sdkconfig.defaults")
                        with open(sdk_path, "w", encoding="utf-8") as sf:
                            sf.write("\n".join(sdk_append) + "\n")
                            
                        # 2.2 Đồng bộ hóa trực tiếp vào sdkconfig (để tương thích với menuconfig)
                        real_sdk_path = os.path.join(PROJECT_ROOT, "sdkconfig")
                        if os.path.isfile(real_sdk_path):
                            sync_sdkconfig(real_sdk_path, sdk_append)
                            
            except Exception as e:
                print(f"Lỗi khi ghi sdkconfig: {e}")

                
        # 3. Ghi custom_s3_n16r8_board.cc
        if payload.get("board_cc") and len(payload["board_cc"].strip()) > 0:
            with open(os.path.join(custom_dir, "custom_s3_n16r8_board.cc"), "w", encoding="utf-8") as f:
                f.write(payload["board_cc"])
                
        # 4. Tiêm vào Kconfig.projbuild nếu chưa có
        kconfig_path = os.path.join(PROJECT_ROOT, "main", "Kconfig.projbuild")
        if os.path.isfile(kconfig_path):
            with open(kconfig_path, "r", encoding="utf-8") as f:
                kcontent = f.read()
            if "BOARD_TYPE_CUSTOM_S3_N16R8" not in kcontent:
                entry = '\n    config BOARD_TYPE_CUSTOM_S3_N16R8\n        bool "Custom ESP32-S3-N16R8 Board"\n        depends on IDF_TARGET_ESP32S3\n'
                choice_idx = kcontent.find("choice BOARD_TYPE")
                if choice_idx != -1:
                    endchoice_idx = kcontent.find("endchoice", choice_idx)
                    if endchoice_idx != -1:
                        new_kcontent = kcontent[:endchoice_idx] + entry + kcontent[endchoice_idx:]
                        with open(kconfig_path, "w", encoding="utf-8") as f:
                            f.write(new_kcontent)
                            
        # 5. Tiêm vào main/CMakeLists.txt nếu chưa có
        cmake_path = os.path.join(PROJECT_ROOT, "main", "CMakeLists.txt")
        if os.path.isfile(cmake_path):
            with open(cmake_path, "r", encoding="utf-8") as f:
                cmake_content = f.read()
            if "CONFIG_BOARD_TYPE_CUSTOM_S3_N16R8" not in cmake_content:
                cmake_snippet = '\nif(CONFIG_BOARD_TYPE_CUSTOM_S3_N16R8)\n    set(BOARD_DIR "custom-s3-n16r8")\n    set(BUILTIN_TEXT_FONT font_noto_sans_basic_20_4)\n    set(BUILTIN_ICON_FONT font_material_symbols_20_4)\n    set(DEFAULT_EMOJI_COLLECTION noto-color-emoji_64)\n    set(EMOTE_RESOLUTION "320_240")\nelse'
                if "if(CONFIG_BOARD_TYPE_" in cmake_content:
                    cmake_content = cmake_content.replace("if(CONFIG_BOARD_TYPE_", cmake_snippet + "if(CONFIG_BOARD_TYPE_", 1)
                    with open(cmake_path, "w", encoding="utf-8") as f:
                        f.write(cmake_content)
                        
        return {"success": True, "message": "Đã lưu cấu hình và cập nhật mã nguồn thành công!"}
    except Exception as e:
        return {"success": False, "error": str(e)}

class ConfiguratorHandler(BaseHTTPRequestHandler):
    def end_headers(self):
        # CORS & Cache control
        self.send_header("Access-Control-Allow-Origin", "*")
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

        # 1. API: Thông tin dự án thực tế
        if path == "/api/project":
            info = detect_project_info()
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps(info, ensure_ascii=False).encode("utf-8"))
            return

        # 2. API: Tự nạp cấu hình hiện có
        if path == "/api/config":
            config_data = read_current_config()
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps(config_data, ensure_ascii=False).encode("utf-8"))
            return

        # 3. Phục vụ tĩnh giao diện Web Configurator
        file_path = None
        if path in ("/", "/index.html", "/configurator.html"):
            file_path = os.path.join(PROJECT_ROOT, "tools", "web-configurator", "index.html")
        elif path.startswith("/tools/web-configurator/"):
            subpath = path[len("/tools/web-configurator/"):]
            file_path = os.path.join(PROJECT_ROOT, "tools", "web-configurator", subpath)
        else:
            # Tìm file trong tools/web-configurator trước, sau đó tìm ở thư mục gốc
            cand1 = os.path.join(PROJECT_ROOT, "tools", "web-configurator", path.lstrip("/"))
            cand2 = os.path.join(PROJECT_ROOT, path.lstrip("/"))
            if os.path.isfile(cand1):
                file_path = cand1
            elif os.path.isfile(cand2):
                file_path = cand2

        if file_path and os.path.isfile(file_path):
            mime, _ = mimetypes.guess_type(file_path)
            if not mime:
                if file_path.endswith(".js"): mime = "application/javascript"
                elif file_path.endswith(".css"): mime = "text/css"
                elif file_path.endswith(".html"): mime = "text/html"
                else: mime = "text/plain"

            self.send_response(200)
            self.send_header("Content-Type", f"{mime}; charset=utf-8")
            self.end_headers()
            with open(file_path, "rb") as f:
                self.wfile.write(f.read())
            return

        self.send_response(404)
        self.send_header("Content-Type", "text/plain; charset=utf-8")
        self.end_headers()
        self.wfile.write(f"404 Not Found: {path}".encode("utf-8"))

    def do_POST(self):
        parsed = urlparse(self.path)
        path = parsed.path

        if path in ("/api/save", "/api/save-tab"):
            content_length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(content_length).decode("utf-8")
            try:
                payload = json.loads(body)
                res = save_configuration_to_disk(payload)
                self.send_response(200 if res.get("success") else 500)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.end_headers()
                self.wfile.write(json.dumps(res, ensure_ascii=False).encode("utf-8"))
            except Exception as e:
                self.send_response(400)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.end_headers()
                self.wfile.write(json.dumps({"success": False, "error": str(e)}).encode("utf-8"))
            return

        self.send_response(404)
        self.end_headers()

def run_server(port=DEFAULT_PORT):
    httpd = None
    target_port = port
    candidate_ports = [port] + [p for p in range(8080, 8100) if p != port] + [0]
    
    for p in candidate_ports:
        try:
            server_address = ("", p)
            httpd = HTTPServer(server_address, ConfiguratorHandler)
            target_port = httpd.server_port
            break
        except OSError:
            continue

    if not httpd:
        print("[LỖI] Không thể lắng nghe trên bất kỳ cổng mạng nào!")
        sys.exit(1)

    port = target_port

    print(f"=====================================================================")
    print(f"   XIAOZHI-ESP32 CONFIGURATOR SERVER (ESP32-S3-N16R8)")
    print(f"=====================================================================")
    print(f"[*] Vị trí thư mục gốc thực tế: {PROJECT_ROOT}")
    print(f"[*] Đang lắng nghe tại: http://localhost:{port}/")
    print(f"[*] Nhấn Ctrl+C để dừng máy chủ.")
    print(f"=====================================================================")
    
    # Mở trình duyệt
    try:
        import webbrowser
        webbrowser.open(f"http://localhost:{port}/")
    except Exception:
        pass

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n[*] Đang tắt máy chủ...")
        httpd.server_close()
        sys.exit(0)

if __name__ == "__main__":
    port = DEFAULT_PORT
    if len(sys.argv) > 1 and sys.argv[1].isdigit():
        port = int(sys.argv[1])
    run_server(port)
