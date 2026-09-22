import os
import json
import shutil
import sys

# Thêm đường dẫn để import configurator_server
sys.path.append(os.path.abspath("d:/Code/Antigravity/xiaozhi-v2"))
import configurator_server

def run_tests():
    project_root = "d:/Code/Antigravity/xiaozhi-v2"
    sdkconfig_path = os.path.join(project_root, "sdkconfig")
    sdkconfig_backup = os.path.join(project_root, "sdkconfig.bak")
    
    # 1. Backup sdkconfig if exists
    if os.path.exists(sdkconfig_path):
        shutil.copy2(sdkconfig_path, sdkconfig_backup)
    else:
        # Tạo file giả lập menuconfig
        with open(sdkconfig_path, "w", encoding="utf-8") as f:
            f.write("CONFIG_SPIRAM=y\n")
            f.write("CONFIG_SPIRAM_ALLOW_STACK_EXTERNAL_MEMORY=y\n")
            f.write("CONFIG_LWIP_MAX_SOCKETS=10\n")
            f.write("CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y\n")
            
    print("[*] Đã chuẩn bị môi trường test...")

    # 2. Tạo Payload giả lập từ app.js
    test_payload = {
        "config_json": json.dumps({
            "target": "esp32s3",
            "builds": [{
                "name": "custom-s3-n16r8",
                "sdkconfig_append": [
                    "CONFIG_BOARD_TYPE_CUSTOM_S3_N16R8=y",
                    "# CONFIG_SPIRAM is not set",
                    "CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y"
                ]
            }]
        }),
        "config_h": "#define TEST_MACRO 1\n",
        "board_cc": "// Board init test\n"
    }

    # 3. Ghi dữ liệu
    print("[*] Đang ghi cấu hình giả lập...")
    res = configurator_server.save_configuration_to_disk(test_payload)
    if not res.get("success"):
        print(f"[FAIL] Lỗi khi ghi cấu hình: {res.get('error')}")
        return
        
    print("[SUCCESS] Hàm save_configuration_to_disk() trả về True")

    # 4. Kiểm tra file được tạo ra
    custom_dir = os.path.join(project_root, "main", "boards", "custom-s3-n16r8")
    if not os.path.exists(os.path.join(custom_dir, "config.h")):
        print("[FAIL] config.h không được tạo")
    if not os.path.exists(os.path.join(custom_dir, "config.json")):
        print("[FAIL] config.json không được tạo")
    if not os.path.exists(os.path.join(custom_dir, "custom_s3_n16r8_board.cc")):
        print("[FAIL] custom_s3_n16r8_board.cc không được tạo")

    # 5. Kiểm tra sdkconfig (Rất quan trọng)
    print("[*] Kiểm tra tính nhất quán của sdkconfig...")
    with open(sdkconfig_path, "r", encoding="utf-8") as f:
        content = f.read()
        
        # Spirams UI-managed keys should be stripped/replaced
        if "CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y" in content:
            print("[FAIL] Cấu hình 4MB cũ chưa bị xóa khỏi sdkconfig!")
        elif "CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y" not in content:
            print("[FAIL] Cấu hình 16MB mới chưa được thêm vào sdkconfig!")
        else:
            print("[SUCCESS] Xóa flag cũ, chèn flag mới thành công.")
            
        if "# CONFIG_SPIRAM is not set" not in content:
            print("[FAIL] Missing negative config flag (# CONFIG_SPIRAM is not set)")
        else:
            print("[SUCCESS] Flag negative hoạt động tốt.")
            
        # Unmanaged keys should remain!
        if "CONFIG_LWIP_MAX_SOCKETS=10" not in content:
            print("[FAIL] Cấu hình của người dùng (LWIP) đã bị mất!")
        else:
            print("[SUCCESS] Cấu hình menuconfig thủ công được BẢO TOÀN.")
            
        if "CONFIG_SPIRAM_ALLOW_STACK_EXTERNAL_MEMORY=y" not in content:
            print("[FAIL] Cấu hình SPIRAM phụ (không thuộc UI quản lý) đã bị mất!")
        else:
            print("[SUCCESS] Cấu hình SPIRAM_ALLOW_STACK được bảo toàn nguyên vẹn.")

    # 6. Kiểm tra hàm Get Config
    print("[*] Kiểm tra hàm read_current_config()...")
    current_cfg = configurator_server.read_current_config()
    if current_cfg.get("source") == "custom_h":
        print("[SUCCESS] Phân giải và tải ngược (Parse) dữ liệu thành công.")
    else:
        print("[FAIL] Không thể load ngược dữ liệu từ config.h")

    # 7. Restore
    if os.path.exists(sdkconfig_backup):
        shutil.copy2(sdkconfig_backup, sdkconfig_path)
        os.remove(sdkconfig_backup)
        print("[*] Đã khôi phục sdkconfig gốc.")
        
    print("\n[KIỂM TRA HOÀN TẤT]")

if __name__ == "__main__":
    run_tests()
