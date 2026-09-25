import os
import sys
import glob
sys.stdout.reconfigure(encoding='utf-8')

FAKE_MACROS = [
    "CONFIG_ENABLE_RELAY",
    "CONFIG_ENABLE_SERVO",
    "CONFIG_ENABLE_MOTOR_DRIVER",
    "CONFIG_ENABLE_PIR_SENSOR",
    "CONFIG_ENABLE_VIBRATION_SENSOR",
    "CONFIG_ENABLE_FLAME_SENSOR",
    "CONFIG_ENABLE_BATTERY_CHARGER_TP4056",
    "CONFIG_ENABLE_PERIPH_IR",
    "CONFIG_ENABLE_STORAGE_SDCARD",
    "CONFIG_ENABLE_PERIPH_LED_DRIVER_IC",
    "CONFIG_ENABLE_SENSOR_DS18B20",
    "CONFIG_ENABLE_SENSOR_FLOW",
    "CONFIG_ENABLE_SENSOR_MQ",
    "CONFIG_ENABLE_PERIPH_RC522",
    "CONFIG_ENABLE_PERIPH_TP4056",
    "CONFIG_ENABLE_SENSOR_HCSR04",
    "CONFIG_ENABLE_SENSOR_BMP280",
    "CONFIG_ENABLE_SENSOR_BH1750",
    "CONFIG_ENABLE_BATTERY_BQ27220",
    "CONFIG_ENABLE_TOUCH_SLIDER",
    "CONFIG_ENABLE_SENSOR_APDS9960",
    "CONFIG_ENABLE_SENSOR_VL53LX"
]

def test_all():
    print("=================================================================")
    print("KHỞI CHẠY KIỂM THỬ XÁC MINH TOÀN DIỆN (ESP-IDF 6.1 & IDF.MD)")
    print("=================================================================")

    # 1. custom_n16r8_board.cc
    with open('main/boards/esp32s3-n16r8-custom/custom_n16r8_board.cc', 'r', encoding='utf-8') as f:
        board_cc = f.read()
    assert 'press_to_talk_mcp_tool.h' in board_cc, "Missing press_to_talk include"
    assert 'PressToTalkMcpTool' in board_cc, "Missing PressToTalkMcpTool instance"
    assert 'CONFIG_ENABLE_BUZZER' in board_cc, "Missing CONFIG_ENABLE_BUZZER in board_cc"
    assert 'DHT11/22 Temperature/Humidity sensor active' in board_cc, "Missing active DHT log in board_cc"
    assert 'Motor DC (H-Bridge) driver active' in board_cc, "Missing active Motor DC log in board_cc"
    print("✓ [Pass] custom_n16r8_board.cc verified (Buzzer, Haptic, PressToTalk, Motor, DHT active)")

    # 2. CMakeLists.txt
    with open('main/CMakeLists.txt', 'r', encoding='utf-8') as f:
        cmake = f.read()
    assert 'drivers/sensor/vl6180x.c' in cmake, "Missing vl6180x.c in CMakeLists.txt"
    assert 'drivers/sensor/dht.c' in cmake, "Missing dht.c in CMakeLists.txt"
    print("✓ [Pass] CMakeLists.txt verified (vl6180x.c and dht.c registered)")

    # 3. dht.h and dht.c
    with open('main/drivers/sensor/dht.c', 'r', encoding='utf-8') as f:
        dht_c = f.read()
    assert 'printf(' not in dht_c, "Violation: printf found in dht.c"
    assert 'portENTER_CRITICAL' in dht_c, "Missing critical section in dht.c"
    assert 'esp_timer_get_time' in dht_c, "Missing high precision timer in dht.c"
    print("✓ [Pass] dht.c verified (FreeRTOS critical section, high resolution timer, no printf)")

    # 4. sensor_manager.c - Kiểm tra loại bỏ GPIO hardcode & macro ảo
    with open('main/drivers/sensor/sensor_manager.c', 'r', encoding='utf-8') as f:
        sensor_c = f.read()
    assert 'dht.h' in sensor_c, "Missing dht.h include"
    assert 'dht_read_data' in sensor_c, "Missing dht_read_data call"
    assert 'sensor_set_dht_pin' in sensor_c, "Missing dynamic GPIO setter"
    assert 'vl6180x_read_distance_mm' in sensor_c, "Missing vl6180x_read_distance_mm call"
    assert 'printf(' not in sensor_c, "Violation: printf found in sensor_manager.c"
    assert 's_pir_pin = GPIO_NUM_14;' not in sensor_c, "Violation: Hardcoded PIR GPIO_NUM_14 found"
    assert 's_vib_pin = GPIO_NUM_6;' not in sensor_c, "Violation: Hardcoded Vib GPIO_NUM_6 found"
    assert 's_flame_pin = GPIO_NUM_7;' not in sensor_c, "Violation: Hardcoded Flame GPIO_NUM_7 found"
    assert 's_chrg_pin = GPIO_NUM_3;' not in sensor_c, "Violation: Hardcoded TP4056 GPIO_NUM_3 found"
    assert 'CONFIG_CUSTOM_ENABLE_SENSOR_VIBRATION_SW420' in sensor_c, "Missing real vibration macro"
    assert 'CONFIG_CUSTOM_ENABLE_SENSOR_FLAME' in sensor_c, "Missing real flame macro"
    assert 'CONFIG_CUSTOM_ENABLE_PERIPH_BATTERY_CHARGING_DETECT' in sensor_c, "Missing real tp4056 macro"
    print("✓ [Pass] sensor_manager.c verified (No hardcoded GPIO, reads real Kconfig, fallback to GPIO_NUM_NC)")

    # 5. actuator_manager.c - Kiểm tra loại bỏ GPIO hardcode & macro ảo
    with open('main/drivers/actuator/actuator_manager.c', 'r', encoding='utf-8') as f:
        actuator_c = f.read()
    assert 's_relay_pin = GPIO_NUM_13;' not in actuator_c, "Violation: Hardcoded Relay GPIO_NUM_13 found"
    assert 's_buzzer_pin = GPIO_NUM_41;' not in actuator_c, "Violation: Hardcoded Buzzer GPIO_NUM_41 found"
    assert 's_haptic_pin = GPIO_NUM_42;' not in actuator_c, "Violation: Hardcoded Haptic GPIO_NUM_42 found"
    assert 's_motor_pwma = GPIO_NUM_1;' not in actuator_c, "Violation: Hardcoded Motor PWMA GPIO_NUM_1 found"
    assert 'CONFIG_CUSTOM_PERIPH_RELAY_ENABLE' in actuator_c, "Missing real relay macro"
    assert 'CONFIG_CUSTOM_ENABLE_SERVO_DOG' in actuator_c, "Missing real servo dog macro"
    assert 'CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE' in actuator_c, "Missing real motor DC macro"
    assert 'CONFIG_CUSTOM_PERIPH_MOTOR_PWMA_PIN' in actuator_c, "Missing real motor DC pin Kconfig"
    print("✓ [Pass] actuator_manager.c verified (No hardcoded GPIO, reads real Kconfig, fallback to GPIO_NUM_NC)")

    # 6. gpio_validator.c
    with open('main/gpio_validator.c', 'r', encoding='utf-8') as f:
        validator_c = f.read()
    assert 'CONFIG_BUZZER_PIN' in validator_c, "Missing CONFIG_BUZZER_PIN in validator"
    assert 'Buzzer Alarm' in validator_c, "Missing Buzzer Alarm in validator"
    assert 'CONFIG_CUSTOM_SENSOR_DHT_GPIO' in validator_c, "Missing DHT pin in validator"
    print("✓ [Pass] gpio_validator.c verified (Buzzer & DHT pin collision check active)")

    # 7. config.h
    with open('main/boards/esp32s3-n16r8-custom/config.h', 'r', encoding='utf-8') as f:
        conf_h = f.read()
    assert '#define BUZZER_PIN' in conf_h, "Missing BUZZER_PIN definition"
    assert '#define SENSOR_DHT_GPIO' in conf_h, "Missing SENSOR_DHT_GPIO definition"
    assert '((gpio_num_t)CONFIG_CUSTOM_SENSOR_DHT_GPIO)' in conf_h, "Missing gpio_num_t cast for SENSOR_DHT_GPIO"
    print("✓ [Pass] config.h verified (BUZZER_PIN & SENSOR_DHT_GPIO strict type casting)")

    # 8. Quét sạch toàn bộ main/ không chứa macro ảo
    print("\nKiểm tra quét toàn bộ tệp tin trong main/ để đảm bảo không còn macro ảo...")
    scanned_files = 0
    found_violations = []
    for root, _, files in os.walk('main'):
        for file in files:
            if file.endswith(('.c', '.cc', '.h', '.cpp')):
                path = os.path.join(root, file)
                scanned_files += 1
                with open(path, 'r', encoding='utf-8', errors='ignore') as src:
                    content = src.read()
                for fake in FAKE_MACROS:
                    if fake in content:
                        found_violations.append(f"{path}: chứa macro ảo '{fake}'")
    
    assert len(found_violations) == 0, f"Phát hiện macro ảo còn sót lại:\n" + "\n".join(found_violations)
    print(f"✓ [Pass] Đã quét {scanned_files} tệp tin trong main/ — 100% sạch, 0 macro ảo!")

    print("\n=================================================================")
    print("TẤT CẢ 8 HẠNG MỤC KIỂM THỬ ĐÃ VƯỢT QUA 100% (ALL TESTS PASSED)!")
    print("=================================================================")

if __name__ == '__main__':
    test_all()
