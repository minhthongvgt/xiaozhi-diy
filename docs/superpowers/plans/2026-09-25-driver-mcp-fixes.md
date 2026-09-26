# Kế Hoạch Sửa Chữa Và Hoàn Thiện Driver & Đăng Ký MCP Tool (ESP-IDF 6.1)

> **Dành cho Agentic Workers:** Tuân thủ quy chuẩn nghiêm ngặt trong [IDF.md](file:///d:/Code/Antigravity/xiaozhi-v2/.agents/rules/IDF.md).
> Mọi bước thực thi chia thành từng Task độc lập, kiểm tra và đánh dấu `[x]` sau khi hoàn thành.

**Mục tiêu:** Sửa chữa các lỗ hổng khởi tạo driver, kết nối đăng ký MCP Tool cho Còi báo động (Buzzer), Nút bấm đàm thoại (Press-to-talk), tích hợp cảm biến Laser ToF (VL6180X), xây dựng driver DHT11/22 hỗ trợ chân GPIO người dùng tùy biến, và rà soát chống xung đột chân GPIO theo chuẩn ESP-IDF 6.1.

**Kiến trúc:** 
- Tầng C Driver: `sensor_manager`, `actuator_manager`, `input_manager`, `gpio_validator`, `dht`.
- Tầng C++ Board: `CustomN16R8Board`, `ActuatorController`, `SensorController`, `PressToTalkMcpTool`.
- Strict Type-Safety: `static_cast<gpio_num_t>`, `GPIO_NUM_NC`, `ESP_ERROR_CHECK`, không dùng `printf`.

---

## Danh Sách Các Task Đã Hoàn Thành (Tasks Completed Checklist)

- [x] **Task 1: Sửa điều kiện đăng ký MCP Tool cho Còi Báo Động (Buzzer) & Rung Haptic trong Board**
  - File: [custom_n16r8_board.cc](file:///d:/Code/Antigravity/xiaozhi-v2/main/boards/esp32s3-n16r8-custom/custom_n16r8_board.cc#L665-L701)
  - Đã bổ sung `CONFIG_ENABLE_BUZZER` và `CONFIG_ENABLE_HAPTIC_MOTOR` vào khối `#if` kiểm tra đăng ký `ActuatorController`.
  - Công cụ `self.actuator.beep` và `self.actuator.vibrate` được đăng ký tự động vào MCP Server khi còi báo động được bật.

- [x] **Task 2: Kích hoạt MCP Tool Chế độ đàm thoại Nút bấm (Press-to-talk MCP Tool)**
  - File: [custom_n16r8_board.cc](file:///d:/Code/Antigravity/xiaozhi-v2/main/boards/esp32s3-n16r8-custom/custom_n16r8_board.cc#L31)
  - Đã include `boards/common/press_to_talk_mcp_tool.h`.
  - Đã khởi tạo `static PressToTalkMcpTool press_to_talk_tool; press_to_talk_tool.Initialize();` trong `InitializeMcpTools()`.
  - Công cụ `self.set_press_to_talk` sẵn sàng trên MCP Server cho AI điều khiển chế độ nói.

- [x] **Task 3: Cập nhật GPIO Safety Validator & config.h cho Còi Buzzer**
  - File: [gpio_validator.c](file:///d:/Code/Antigravity/xiaozhi-v2/main/gpio_validator.c#L154-L160)
  - Đã thêm kiểm tra `CONFIG_ENABLE_BUZZER` và `CONFIG_BUZZER_PIN` vào mảng `pins` của `gpio_safety_validate()`.
  - File: [config.h](file:///d:/Code/Antigravity/xiaozhi-v2/main/boards/esp32s3-n16r8-custom/config.h#L454-L466)
  - Đã định nghĩa chuẩn `BUZZER_PIN` và `HAPTIC_PIN` với `((gpio_num_t)CONFIG_BUZZER_PIN)` theo quy chuẩn type-safety.

- [x] **Task 4: Đấu nối Cảm biến Khoảng cách Laser ToF (VL6180X) vào Sensor Manager**
  - File: [CMakeLists.txt](file:///d:/Code/Antigravity/xiaozhi-v2/main/CMakeLists.txt#L88) (bổ sung `drivers/sensor/vl6180x.c`).
  - File: [sensor_manager.c](file:///d:/Code/Antigravity/xiaozhi-v2/main/drivers/sensor/sensor_manager.c#L121-L175)
  - Kết nối driver `vl6180x.h` / `vl6180x.c` với shared I2C Master Bus từ `bus_manager`.
  - Cập nhật hàm `sensor_read_distance()` để đọc dữ liệu khoảng cách laser mm thực tế và cập nhật `sensor_read_environment()` để đọc cường độ ánh sáng Lux từ ALS của VL6180X.

- [x] **Task 5: Xây dựng Driver DHT11/DHT22 Hỗ Trợ Chân GPIO Tùy Biến Động**
  - File tạo mới: [dht.h](file:///d:/Code/Antigravity/xiaozhi-v2/main/drivers/sensor/dht.h) & [dht.c](file:///d:/Code/Antigravity/xiaozhi-v2/main/drivers/sensor/dht.c).
  - Sử dụng `esp_timer_get_time()` đo độ rộng xung cấp vi giây, bảo vệ bằng FreeRTOS `portENTER_CRITICAL` tránh xung đột ngắt mạng.
  - Hỗ trợ cơ chế bộ nhớ đệm tự động (2 giây) tránh polling liên tục làm đơ cảm biến.
  - Tích hợp hàm `sensor_set_dht_pin(gpio_num_t pin)` và `sensor_get_dht_pin()` trong [sensor_manager.c](file:///d:/Code/Antigravity/xiaozhi-v2/main/drivers/sensor/sensor_manager.c#L166-L180), cho phép thay đổi GPIO bất kỳ lúc runtime hoặc theo cấu hình người dùng.
  - Cập nhật `sensor_read_environment()` trả về nhiệt độ & độ ẩm thực tế vào MCP tool `self.sensor.get_environment`.

- [x] **Task 6: Rà soát toàn diện theo IDF.md & Xác minh mã nguồn**
  - Đã chạy kiểm thử tự động qua [verify_fixes.py](file:///d:/Code/Antigravity/xiaozhi-v2/verify_fixes.py), 100% assertions passed.
  - Không còn sử dụng `printf` (tuân thủ mục 2 IDF.md).
  - Ép kiểu tĩnh `static_cast<gpio_num_t>` / `(gpio_num_t)` đầy đủ, không gán số nguyên -1.
