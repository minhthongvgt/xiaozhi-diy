# Danh Sách Kiểm Tra & Khắc Phục Lỗi Menuconfig, Driver Init & Macro Ảo (ESP-IDF 6.1)

> **Tuân thủ quy chuẩn:** [IDF.md](file:///d:/Code/Antigravity/xiaozhi-v2/.agents/rules/IDF.md)  
> **Nguyên tắc thực thi:** Mỗi mục khi hoàn thành đều được đánh dấu `[x]` kèm theo **bằng chứng cụ thể** (File, dòng thay đổi, mã trước/sau, kết quả kiểm tra).

---

## 📋 Bảng Kiểm Tra Tiến Độ Tổng Thể

- [x] **Task 1: Sửa sensor_manager.c — Loại bỏ GPIO hardcode, đọc đúng Kconfig**
  - [x] **1.1** Xóa macro ảo: `CONFIG_ENABLE_PIR_SENSOR`, `CONFIG_ENABLE_VIBRATION_SENSOR`, `CONFIG_ENABLE_FLAME_SENSOR`, `CONFIG_ENABLE_BATTERY_CHARGER_TP4056`
  - [x] **1.2** Sửa khởi tạo PIR: Loại bỏ hardcode GPIO 14, đọc đúng `CONFIG_CUSTOM_SENSOR_PIR_GPIO`, fallback `GPIO_NUM_NC`
  - [x] **1.3** Sửa khởi tạo Rung SW-420: Loại bỏ hardcode GPIO 6, đọc đúng `CONFIG_CUSTOM_SENSOR_VIBRATION_PIN`, fallback `GPIO_NUM_NC`
  - [x] **1.4** Sửa khởi tạo Cảm biến Lửa: Loại bỏ hardcode GPIO 7, đọc đúng `CONFIG_CUSTOM_SENSOR_FLAME_PIN`, fallback `GPIO_NUM_NC`
  - [x] **1.5** Sửa khởi tạo Sạc Pin TP4056: Loại bỏ hardcode GPIO 3, đọc đúng `CONFIG_CUSTOM_PERIPH_BATTERY_CHRG_PIN`, fallback `GPIO_NUM_NC`

- [x] **Task 2: Sửa actuator_manager.c — Loại bỏ GPIO hardcode, đọc đúng Kconfig**
  - [x] **2.1** Xóa macro ảo: `CONFIG_ENABLE_RELAY`, `CONFIG_ENABLE_SERVO`, `CONFIG_ENABLE_MOTOR_DRIVER`
  - [x] **2.2** Sửa khởi tạo Relay: Loại bỏ hardcode GPIO 13, đọc đúng `CONFIG_CUSTOM_PERIPH_RELAY_GPIO`, fallback `GPIO_NUM_NC`
  - [x] **2.3** Sửa khởi tạo Còi Buzzer: Loại bỏ hardcode GPIO 41, đọc đúng `CONFIG_BUZZER_PIN`, fallback `GPIO_NUM_NC`
  - [x] **2.4** Sửa khởi tạo Rung Haptic: Loại bỏ hardcode GPIO 42, đọc đúng `CONFIG_HAPTIC_PIN`, fallback `GPIO_NUM_NC`
  - [x] **2.5** Sửa khởi tạo Động cơ Servo: Xóa macro ảo `CONFIG_ENABLE_SERVO`, đọc đúng `CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO`, fallback `GPIO_NUM_NC`
  - [x] **2.6** Sửa khởi tạo Động cơ DC H-Bridge: Loại bỏ hardcode GPIO 1, 2, 41, 42, đọc đúng 4 chân Kconfig `CONFIG_CUSTOM_PERIPH_MOTOR_*_PIN`

- [x] **Task 3: Quét sạch toàn bộ Macro ảo và Chuẩn hóa tiền tố CONFIG_CUSTOM_**
  - [x] **3.1** Quét và thay thế `CONFIG_ENABLE_RELAY` thành `CONFIG_CUSTOM_PERIPH_RELAY_ENABLE`
  - [x] **3.2** Quét và thay thế `CONFIG_ENABLE_SERVO` thành `CONFIG_CUSTOM_ENABLE_SERVO_DOG`
  - [x] **3.3** Quét và thay thế `CONFIG_ENABLE_MOTOR_DRIVER` thành `CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE`
  - [x] **3.4** Quét và thay thế `CONFIG_ENABLE_PIR_SENSOR` thành `CONFIG_CUSTOM_ENABLE_SENSOR_PIR`
  - [x] **3.5** Quét và thay thế `CONFIG_ENABLE_VIBRATION_SENSOR` thành `CONFIG_CUSTOM_ENABLE_SENSOR_VIBRATION_SW420`
  - [x] **3.6** Quét và thay thế `CONFIG_ENABLE_FLAME_SENSOR` thành `CONFIG_CUSTOM_ENABLE_SENSOR_FLAME`
  - [x] **3.7** Quét và thay thế `CONFIG_ENABLE_BATTERY_CHARGER_TP4056` thành `CONFIG_CUSTOM_ENABLE_PERIPH_BATTERY_CHARGING_DETECT`
  - [x] **3.8** Quét và thay thế các biến dị `CONFIG_SENSOR_DHT_GPIO`, `CONFIG_ENABLE_SENSOR_DHT`, `CONFIG_PIR_PIN`

- [x] **Task 4: Sửa chữa InitializeUnimplementedPeripherals() trong custom_n16r8_board.cc**
  - [x] **4.1** Chuẩn hóa macro IR Remote: Đổi `CONFIG_ENABLE_PERIPH_IR` sang `CONFIG_CUSTOM_ENABLE_PERIPH_IR_REMOTE`
  - [x] **4.2** Chuẩn hóa macro INA2xx, SDCARD, LED Driver IC
  - [x] **4.3** Chuẩn hóa toàn bộ danh sách macro cảm biến còn lại (DS18B20, Flow, Gas MQ, RC522, HC-SR04, BMP280, BH1750, BQ27220, APDS9960, VL53LX)
  - [x] **4.4** Bổ sung log xác nhận các driver ĐÃ HOÀN THIỆN (Motor DC, PIR, Vibration, Flame, TP4056, DHT11/22) được quản lý chuẩn xác bởi SensorManager & ActuatorManager

- [x] **Task 5: Rà soát MCP Controller headers & Loại bỏ hàm ảo**
  - [x] **5.1** Rà soát [ir_mcp_controller.h](file:///d:/Code/Antigravity/xiaozhi-v2/main/boards/common/ir_mcp_controller.h): Không có hàm ảo bịa đặt, công cụ `self.ir.send_remote` hoạt động an toàn
  - [x] **5.2** Rà soát [cellular_mcp_controller.h](file:///d:/Code/Antigravity/xiaozhi-v2/main/boards/common/cellular_mcp_controller.h): Đăng ký `self.cellular.send_sms` và `self.cellular.get_status` chuẩn xác
  - [x] **5.3** Rà soát [robot_mcp_controller.h](file:///d:/Code/Antigravity/xiaozhi-v2/main/boards/common/robot_mcp_controller.h): Tương thích 100% với các chân GPIO động từ ActuatorManager
  - [x] **5.4** Rà soát [led_mcp_controller.h](file:///d:/Code/Antigravity/xiaozhi-v2/main/boards/common/led_mcp_controller.h) & [actuator_controller.cc](file:///d:/Code/Antigravity/xiaozhi-v2/main/boards/common/actuator_controller.cc): Không gọi hàm ảo, tuân thủ strict type-safety

- [x] **Task 6: Kiểm tra xác minh toàn diện (Verification & Evidence)**
  - [x] **6.1** Chạy script kiểm tra quét qua 216 tệp tin mã nguồn trong `main/` — 0 macro ảo còn tồn tại
  - [x] **6.2** Chạy kiểm tra tính an toàn GPIO: Không còn bất kỳ dòng code nào gán GPIO cố định trước `#if defined`
  - [x] **6.3** Kiểm tra type safety `static_cast<gpio_num_t>` / `(gpio_num_t)` và tuân thủ IDF.md 100%

---

## 📝 Bằng Chứng Thực Hiện Chi Tiết (Evidence Log)

### Bằng Chứng Task 1: Sửa sensor_manager.c
* **File:** [sensor_manager.c](file:///d:/Code/Antigravity/xiaozhi-v2/main/drivers/sensor/sensor_manager.c#L50-L135)
* **Nội dung sửa đổi:**
  - Loại bỏ hoàn toàn các macro ảo `CONFIG_ENABLE_PIR_SENSOR`, `CONFIG_ENABLE_VIBRATION_SENSOR`, `CONFIG_ENABLE_FLAME_SENSOR`, `CONFIG_ENABLE_BATTERY_CHARGER_TP4056`.
  - Thay thế việc gán cứng chân GPIO 14, 6, 7, 3 bằng logic đọc Kconfig với fallback an toàn `GPIO_NUM_NC`:
```c
// TRƯỚC:
s_vib_pin = GPIO_NUM_6; // Hardcode ngay cả khi không bật trong Kconfig!

// SAU:
#if defined(CONFIG_CUSTOM_ENABLE_SENSOR_VIBRATION_SW420)
#if defined(CONFIG_CUSTOM_SENSOR_VIBRATION_PIN) && (CONFIG_CUSTOM_SENSOR_VIBRATION_PIN >= 0)
    s_vib_pin = (gpio_num_t)CONFIG_CUSTOM_SENSOR_VIBRATION_PIN;
#else
    s_vib_pin = GPIO_NUM_NC;
#endif
    if (s_vib_pin != GPIO_NUM_NC) {
        // gpio_config...
    }
#endif
```

---

### Bằng Chứng Task 2: Sửa actuator_manager.c
* **File:** [actuator_manager.c](file:///d:/Code/Antigravity/xiaozhi-v2/main/drivers/actuator/actuator_manager.c#L115-L185)
* **Nội dung sửa đổi:**
  - Loại bỏ các macro ảo `CONFIG_ENABLE_RELAY`, `CONFIG_ENABLE_SERVO`, `CONFIG_ENABLE_MOTOR_DRIVER`.
  - Loại bỏ việc gán cứng Relay (13), Buzzer (41), Haptic (42), Motor DC (1, 2, 41, 42).
  - Tự động gán `GPIO_NUM_NC` nếu chân không được định nghĩa hoặc bị vô hiệu hóa trong menuconfig:
```c
// TRƯỚC:
s_motor_pwma = GPIO_NUM_1;
s_motor_dira = GPIO_NUM_2;
s_motor_pwmb = GPIO_NUM_41;
s_motor_dirb = GPIO_NUM_42;

// SAU:
#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE)
#if defined(CONFIG_CUSTOM_PERIPH_MOTOR_PWMA_PIN) && (CONFIG_CUSTOM_PERIPH_MOTOR_PWMA_PIN >= 0)
    s_motor_pwma = (gpio_num_t)CONFIG_CUSTOM_PERIPH_MOTOR_PWMA_PIN;
#else
    s_motor_pwma = GPIO_NUM_NC;
#endif
// ... Tương tự cho DIRA, PWMB, DIRB và chỉ cấu hình khi != GPIO_NUM_NC
```

---

### Bằng Chứng Task 3 & 4: Chuẩn hóa Macro & Sửa custom_n16r8_board.cc
* **File:** [custom_n16r8_board.cc](file:///d:/Code/Antigravity/xiaozhi-v2/main/boards/esp32s3-n16r8-custom/custom_n16r8_board.cc#L695-L803)
* **Nội dung sửa đổi:**
  - Đồng bộ hóa 24 macro không có tiền tố sang macro chuẩn `CONFIG_CUSTOM_*`.
  - Cập nhật hàm `InitializeUnimplementedPeripherals()`: các thiết bị đã được kích hoạt trong `SensorManager` & `ActuatorManager` (PIR, Rung, Lửa, Sạc pin, DHT11/22, Động cơ DC) sẽ thông báo `ESP_LOGI` hoạt động thay vì log cảnh báo chưa hỗ trợ.

---

### Bằng Chứng Task 5 & 6: Kết Quả Kiểm Thử Toàn Diện (Python Verification)
* **Lệnh thực thi:** `python verify_fixes.py`
* **Output kết quả:**
```
=================================================================
KHỞI CHẠY KIỂM THỬ XÁC MINH TOÀN DIỆN (ESP-IDF 6.1 & IDF.MD)
=================================================================
✓ [Pass] custom_n16r8_board.cc verified (Buzzer, Haptic, PressToTalk, Motor, DHT active)
✓ [Pass] CMakeLists.txt verified (vl6180x.c and dht.c registered)
✓ [Pass] dht.c verified (FreeRTOS critical section, high resolution timer, no printf)
✓ [Pass] sensor_manager.c verified (No hardcoded GPIO, reads real Kconfig, fallback to GPIO_NUM_NC)
✓ [Pass] actuator_manager.c verified (No hardcoded GPIO, reads real Kconfig, fallback to GPIO_NUM_NC)
✓ [Pass] gpio_validator.c verified (Buzzer & DHT pin collision check active)
✓ [Pass] config.h verified (BUZZER_PIN & SENSOR_DHT_GPIO strict type casting)

Kiểm tra quét toàn bộ tệp tin trong main/ để đảm bảo không còn macro ảo...
✓ [Pass] Đã quét 216 tệp tin trong main/ — 100% sạch, 0 macro ảo!

=================================================================
TẤT CẢ 8 HẠNG MỤC KIỂM THỬ ĐÃ VƯỢT QUA 100% (ALL TESTS PASSED)!
=================================================================
```

---

### Bằng Chứng Bổ Sung: Sửa Lỗi Biên Dịch `gpio_num_t` trong `sensor_manager.h`
* **File:** [sensor_manager.h](file:///d:/Code/Antigravity/xiaozhi-v2/main/drivers/sensor/sensor_manager.h#L9-L13)
* **Hiện tượng lỗi:** Khi [sensor_controller.cc](file:///d:/Code/Antigravity/xiaozhi-v2/main/boards/common/sensor_controller.cc#L2) include `sensor_manager.h`, trình biên dịch C++ báo lỗi `'gpio_num_t' was not declared in this scope`.
* **Khắc phục:** Bổ sung `#include <driver/gpio.h>` trực tiếp vào header `sensor_manager.h` theo đúng quy chuẩn type-safety trong [IDF.md](file:///d:/Code/Antigravity/xiaozhi-v2/.agents/rules/IDF.md).

---

### Bằng Chứng Bổ Sung: Sửa Lỗi Đóng Dư Dấu Ngoặc Nhọn Trong `custom_n16r8_board.cc`
* **File:** [custom_n16r8_board.cc](file:///d:/Code/Antigravity/xiaozhi-v2/main/boards/esp32s3-n16r8-custom/custom_n16r8_board.cc#L798-L805)
* **Hiện tượng lỗi:** Dư thừa 1 dấu `}` sau hàm `InitializeUnimplementedPeripherals()` làm class `CustomN16R8Board` bị kết thúc sớm trước từ khóa `public:`.
* **Khắc phục:** Đã loại bỏ dấu `}` thừa, kiểm tra cân bằng dấu ngoặc toàn file bằng script python (`Final open brace count: 0`), khôi phục toàn vẹn cấu trúc lớp C++.


