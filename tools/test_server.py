#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Kiểm thử chức năng máy chủ cục bộ configurator_server.py
Xác minh:
1. Nhận diện đường dẫn thực tế động (dynamic real path)
2. Kiểm tra tính hợp lệ của dự án xiaozhi
3. Tự động đọc cấu hình hiện có
4. Khởi chạy HTTP server và phản hồi API /api/project, /api/config
"""

import os
import sys
import json
import time
import threading
import urllib.request

# Cấu hình stdout hiển thị UTF-8 trên Windows console
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

# Import trực tiếp configurator_server từ thư mục gốc
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
import configurator_server

def test_unit_server_functions():
    print("1. Kiểm tra nhận diện thư mục thực tế động:")
    info = configurator_server.detect_project_info()
    assert info["success"] is True, "detect_project_info phải trả về success"
    assert os.path.isabs(info["root_path"]), "root_path phải là đường dẫn tuyệt đối thực tế"
    assert os.path.isdir(info["root_path"]), "root_path phải là thư mục tồn tại"
    assert info["is_valid_project"] is True, "Phải nhận diện đúng dự án xiaozhi-esp32"
    print(f"  [PASS] Nhận diện thành công thư mục thực tế: {info['root_path']}")
    print(f"  [PASS] Xác thực tính hợp lệ dự án: {info['is_valid_project']}")

    print("\n2. Kiểm tra tự động đọc cấu hình hiện hành:")
    cfg = configurator_server.read_current_config()
    assert cfg is not None, "read_current_config không được trả về None"
    assert "source" in cfg, "Cấu hình phải có nguồn đọc (source)"
    print(f"  [PASS] Nguồn cấu hình nạp được: {cfg['source']}")
    if "content" in cfg:
        assert len(cfg["content"]) > 0, "Nội dung config.h không được rỗng"
        print(f"  [PASS] Kích thước file cấu hình: {len(cfg['content'])} bytes")

def test_http_api_endpoints():
    print("\n3. Kiểm tra HTTP server và các API Endpoints:")
    test_port = 8199
    
    # Chạy server trên thread riêng
    server_address = ("", test_port)
    httpd = configurator_server.HTTPServer(server_address, configurator_server.ConfiguratorHandler)
    server_thread = threading.Thread(target=httpd.serve_forever, daemon=True)
    server_thread.start()
    time.sleep(0.5)

    base_url = f"http://localhost:{test_port}"

    try:
        # Test GET /api/project
        req = urllib.request.urlopen(f"{base_url}/api/project", timeout=5)
        assert req.status == 200, f"API /api/project trả về {req.status}"
        data = json.loads(req.read().decode("utf-8"))
        assert data["success"] is True, "Response /api/project success"
        assert os.path.isabs(data["root_path"]), "Response root_path hợp lệ"
        print(f"  [PASS] GET /api/project -> HTTP 200 (root: {data['root_path']})")

        # Test GET /api/config
        req_cfg = urllib.request.urlopen(f"{base_url}/api/config", timeout=5)
        assert req_cfg.status == 200, f"API /api/config trả về {req_cfg.status}"
        cfg_res = json.loads(req_cfg.read().decode("utf-8"))
        assert "source" in cfg_res, "Response /api/config có trường source"
        print(f"  [PASS] GET /api/config -> HTTP 200 (source: {cfg_res['source']})")

        # Test GET / (trang chủ giao diện)
        req_home = urllib.request.urlopen(f"{base_url}/", timeout=5)
        assert req_home.status == 200, f"GET / trả về {req_home.status}"
        content = req_home.read().decode("utf-8")
        assert "<title>Xiaozhi-ESP32" in content or "Web Configurator" in content, "Trang chủ tải đúng HTML Web Configurator"
        print(f"  [PASS] GET / -> HTTP 200 (HTML Web Configurator)")

    finally:
        httpd.shutdown()
        httpd.server_close()
        print("  [PASS] Đã dừng server thử nghiệm an toàn.")

if __name__ == "__main__":
    test_unit_server_functions()
    test_http_api_endpoints()
    print("\n=== TOÀN BỘ KIỂM THỬ SERVER & API TỰ NẠP ĐÃ PASS 100% ===")
