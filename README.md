# Xiaozhi-ESP32 (Board: ESP32-S3-N16R8)

Dự án trợ lý thông minh Chatbot Xiaozhi trên vi điều khiển **ESP32-S3-N16R8** (16MB Octal Flash, 8MB Octal PSRAM) sử dụng framework ESP-IDF 6.1.

---

## 1. Công cụ Web Configurator (Zero-Install)

Dự án cung cấp bộ cấu hình phần cứng trực quan trên nền Web chạy hoàn toàn phía client (SPA Zero-Install). Người dùng có thể thiết lập chân GPIO, màn hình, chip âm thanh codec, các giao tiếp và toàn bộ ngoại vi/cảm biến mà không cần sửa mã nguồn C/C++ thủ công.

### 1.1 Khởi động công cụ
- **Cách 1 (Khuyến nghị tối ưu - Khởi chạy ngay tại thư mục gốc mã nguồn)**:
  Nhấp đúp vào tệp ngay tại thư mục gốc dự án:
  ```text
  run_configurator.bat
  ```
  *(Kịch bản tự động nhận diện vị trí thực tế của thư mục dự án dù bạn đặt ở bất kỳ đâu, tự động nạp sẵn cấu hình hiện có vào UI, mở trình duyệt tại `http://localhost:8080`, và cho phép lưu cấu hình trực tiếp vào mã nguồn tức thì).*
- **Cách 2**: Nhấp đúp vào `tools/web-configurator/start_configurator.bat`.
- **Cách 3**: Mở tệp `configurator.html` hoặc `tools/web-configurator/index.html` trực tiếp trên trình duyệt (Google Chrome / Edge).

### 1.2 Danh mục Cấu hình Chuẩn theo Hệ thống Menuconfig

Sidebar hiện có 8 tab (mỗi tab là một component trong `tools/web-configurator/components/`, nạp động qua `js/main.js`):

| Tab (data-target) | Nhóm | Nội dung chính |
| :--- | :--- | :--- |
| `panel-assistant` | 1. Xiaozhi Assistant | Ngôn ngữ, OTA URL, Wake Word, AEC |
| `panel-display` | 1. Xiaozhi Assistant | Màn hình, cảm ứng, giao diện |
| `panel-audio` | 1. Xiaozhi Assistant | Loa, Micro, chế độ I2S |
| `panel-network` | 1. Xiaozhi Assistant | Wi-Fi provisioning, 4G/LTE, UART mở rộng, MCP |
| `panel-peripherals` | 1. Xiaozhi Assistant | Nút bấm, đèn, còi, relay, servo, motor DC, cảm biến |
| `panel-system` | 2. ESP-IDF Cốt lõi | CPU, Flash, PSRAM, bảng phân vùng |
| `panel-build` | 2. ESP-IDF Cốt lõi | Build, flash, log, chọn ESP-IDF, cổng COM |
| `panel-power` | 2. ESP-IDF Cốt lõi | Pin ADC, INA219/INA226, TP4056 |

Phần mô tả chi tiết bên dưới liệt kê tính năng theo nhóm chức năng (một nhóm có thể nằm trong tab khác tên cũ):
Giao diện được căn chỉnh theo tỉ lệ công thái học hiện đại (Sidebar 260px, Khung trung tâm 1380px, Font Outfit + JetBrains Mono), đã **loại bỏ hoàn toàn đánh số thứ tự** giúp danh mục luôn tinh gọn, tích hợp cơ chế **Tự Động Nhận Diện Cấu Hình (Auto-Detect)**, **Lưu theo từng Tab (Per-Tab Save)** và **Bộ theo dõi thay đổi thông minh (Dirty Tracking)**:
- Nút lưu được bố trí ở góc trên bên phải của **từng danh mục**. Khi người dùng ở danh mục nào sẽ lưu thông tin cho danh mục đó.
- Hệ thống tự động kiểm tra thay đổi: hiển thị nhãn `● Chưa lưu` trên thanh điều hướng và chỉ nhắc nhở người dùng khi có sự thay đổi chưa được lưu trước khi chuyển tab.
- **Quy chuẩn Giao diện**: Toàn bộ phong cách giao diện và biểu cảm AI được quy chuẩn tập trung tại **Flash Assets**, loại bỏ hoàn toàn tùy chọn trùng lặp tại danh mục màn hình.

- 🤖 **1. Xiaozhi Assistant - Trợ lý (panel-assistant)**: Ngôn ngữ đàm thoại (Tiếng Việt, English, 中文), URL máy chủ OTA, từ khóa Wake Word (WakeNet/Pinyin/Độ nhạy), và tính năng khử tiếng vọng (AEC).
- 🖥️ **2. Xiaozhi Assistant - Màn hình (panel-display)**: Hỗ trợ đa dạng chuẩn màn hình SPI/I2C/RGB/QSPI/UART HMI, cảm ứng (Touch), xoay/đảo màu, sóng âm thoại động. Quản lý Gói tài nguyên biểu cảm & Giao diện từ Server.
- 🔊 **3. Xiaozhi Assistant - Âm thanh (panel-audio)**: Cấu hình Loa (DAC/Amply như MAX98357A, ES8311, v.v.) và Micro thu âm (INMP441, ES7210, v.v.). Hỗ trợ I2S 16000Hz chuẩn AI Voice.
- 📶 **4. Xiaozhi Assistant - Mạng & Điều khiển (panel-network)**: Cấu hình cấp Wi-Fi (SoftAP/BluFi), Modem 4G LTE, UART mở rộng, và máy chủ MCP cho AI Tool Calling.
- 🔌 **5. Xiaozhi Assistant - Ngoại vi & Cảm biến (panel-peripherals)**: Quản lý thiết bị ngoại vi như Nút bấm, Đèn (Lamp/Relay), Còi (Buzzer), Động cơ Haptic/Servo/DC, và các loại cảm biến (Nhiệt độ, Khoảng cách, Khí áp).
- ⚡ **6. ESP-IDF Cốt lõi - Hệ thống (panel-system)**: Cấu hình xung nhịp CPU, bộ nhớ Flash/PSRAM (khóa cứng bảo vệ chân 26-32) và thiết lập bảng phân vùng tĩnh.
- 🚀 **7. ESP-IDF Cốt lõi - Biên dịch (panel-build)**: Tích hợp công cụ biên dịch tự động, flash firmware, live console, tự dò tìm và quét cài đặt ESP-IDF, cũng như tự động nhận diện Target Chip.
- 🔋 **8. ESP-IDF Cốt lõi - Nguồn điện (panel-power)**: Quản lý năng lượng qua IC chuyên dụng như INA219, INA226, đo điện áp ADC và IC sạc TP4056.

---

## 2. Quy tắc Sơ đồ chân GPIO (ESP32-S3-N16R8 Pinout Rules)

| Phân vùng chân | Dải GPIO | Chú thích an toàn |
| :--- | :--- | :--- |
| **Octal Flash & PSRAM** | **GPIO 26 – 32** | **NGHIÊM CẤM SỬ DỤNG**. Được nối cứng nội bộ cho bus dữ liệu Octal SPI tốc độ cao. Hệ thống tự động khóa và cảnh báo đỏ nếu người dùng gán vào dải này. |
| **Strapping Pins** | **GPIO 0, 3, 45, 46** | GPIO 0 dùng cho phím BOOT. Chú ý mức logic khi khởi động để tránh vào sai chế độ nạp ROM. |
| **I2S Audio (Phần 1: Loa)** | GPIO 14, 15, 7, 2 | BCLK, WS/LRCK, DOUT (MAX98357A / PCM5102A / NS4168 / ES8311 / ES8388). Chân PA bật amply. |
| **I2S Audio (Phần 2: Mic)** | GPIO 4, 5, 6 | SCK, WS, DIN (INMP441 / ICS-43434 / MSM261S / ES7210). |
| **UART Display (Nextion/DWIN/JSON)** | **GPIO 17, 18** | TX=17, RX=18 trên `UART_NUM_1` (Màn hình thông minh rời; tự động escape chuỗi, chống tràn khung DGUS). |
| **Custom UART Subsystem** | **UART_NUM_2** | Cổng UART mở rộng cho Modem 4G, AI Vision, Bus Servo (Tự động cách ly với UART Display). |
| **Touch Controller (Cảm ứng)** | GPIO 8, 9, 3, 10 | I2C SDA=8, SCL=9, ngắt INT=3, reset RST=10 (CST816S, GT911, FT6236). |
| **I2C Bus & Cảm biến** | GPIO 8, 9 | SDA=8, SCL=9 (Hỗ trợ OLED SSD1306, ToF VL6180X, Codec ES8311). |
| **Servo PWM (Động cơ)** | **GPIO 13** | Tần số 50Hz, 14-bit duty trên LEDC Timer 2 (Độc lập chống xung đột backlight/LED). |
| **Relay / Lamp (Ngoại vi)** | **GPIO 45** | Rơ-le 220V / Đèn bàn, liên kết tự động với công cụ AI MCP. |
| **Built-in LED** | GPIO 48 | Đèn LED báo trạng thái hệ thống. |
| **Touch Slider (Thanh trượt cảm ứng)** | **GPIO 1, 2, 3** | Touch Pad 1, 2, 3 nội bộ ESP32-S3 (FSM đo điện dung). **KHÔNG** dùng chung chân với nút bấm Pull-up. |
| **Phím âm lượng (Volume Buttons)** | **GPIO 47, 48 (hoặc chân tự do)** | Nút nhấn kéo trở Pull-up. Tự động cách ly với Touch Slider khi kích hoạt cả hai. |
| **Cổng kết nối mở rộng** | GPIO 11, 12, 35, 36, 37, 39, 43, 44 | Tự do cấu hình cho Còi Buzzer (41), Rung Haptic (42), Động cơ DC TB6612, Encoder EC11, Cảm biến siêu âm, Thẻ nhớ SD Card. |

---

## 3. Quy trình Biên dịch mã nguồn (Windows / ESP-IDF 6.1)

Sau khi bấm **"Lưu mục này" (Save Tab)** trên giao diện Web Configurator, toàn bộ cấu hình phần cứng sẽ được lưu và đồng bộ trực tiếp vào:
- `sdkconfig.defaults`: Tệp cấu hình gốc chuẩn ESP-IDF menuconfig của dự án.
- `sdkconfig`: Đồng bộ tức thì các tiền tố `CONFIG_` (Bo mạch, Chân GPIO, Driver Màn hình, Codec Loa/Mic, Camera, Cảm biến, v.v.).
- Hệ thống tự động kiểm soát chống xung đột chân GPIO và khóa cứng an toàn dải **GPIO 26 – 32** dành riêng cho bus Octal Flash / Octal PSRAM của chip ESP32-S3-N16R8.

### 3.1. Tính Tương đồng 100% Logic với ESP-IDF menuconfig (Kconfig Engine Parity)
Web Configurator hiện tại hoạt động hoàn toàn đồng nhất 100% theo đúng quy tắc của bộ máy Kconfig ESP-IDF:
1. **Ràng buộc phụ thuộc phân cấp (`depends on` / `if ... endif`)**: Khi tắt tính năng cha (Display, Camera, Wakeword, Loa, Mic, v.v.), Web Configurator tự động ẩn giao diện và triệt tiêu toàn bộ biến con, chỉ ghi `# CONFIG_ENABLE_... is not set`, ngăn ngừa triệt để việc rò rỉ cấu hình thừa.
2. **Cơ chế Choice hai chiều (Choice Flag + Derived Value)**: Sinh đồng thời cả cờ chọn (`CONFIG_CHOICE_X=y`) và giá trị tương ứng (ví dụ: `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_115200=y` đi kèm `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE=115200`), giúp khi mở lệnh `idf.py menuconfig` không bao giờ bị nhảy mất cấu hình hoặc reset về mặc định.
3. **Kích hoạt tự động phụ thuộc chéo (`select`)**: Ví dụ chọn Wi-Fi BluFi sẽ tự động kích hoạt Bluetooth stack (`BT_ENABLED`, `BT_BLE_42_FEATURES_SUPPORTED`, `BT_BLE_BLUFI_ENABLE`).
4. **Đồng bộ hóa 2 chiều (Bidirectional Sync)**: Web Configurator ưu tiên đọc từ `sdkconfig` (nơi `menuconfig` lưu) khi khởi động, giúp mọi thay đổi bạn thực hiện bằng lệnh `idf.py menuconfig` trên terminal hiển thị tức thì trên giao diện Web.
5. **Bộ kiểm thử toàn diện (118 test, 6 tệp)**: Chạy `tools/web-configurator/run_all_tests.bat`: `test_modules.mjs` (19, module thật `js/`), `test_sdkconfig_io.py` (12), `test_dom.js` (28, 8 panel), `test_runner.js` (23), `test_integration.js` (16), `test_kconfig_parity.js` (20).

Yêu cầu cài ESP-IDF 6.1 cho ESP32-S3 bằng Windows installer, gồm Python environment và compiler/toolchain. Bạn có thể biên dịch trực tiếp từ **Web Configurator** (Tab *Biên dịch & Nạp*) hoặc chạy dòng lệnh từ thư mục gốc:

```bat
build_windows.bat
```

Launcher tự nạp `export.bat`, đặt Python ở UTF-8 và gọi `idf.py` trong môi trường ESP-IDF. Nó dùng `IDF_PATH` nếu đã có; nếu không, tự tìm một số thư mục cài đặt phổ biến. Lệnh build giữ nguyên `sdkconfig` hiện tại. Có thể truyền lệnh ESP-IDF khác, ví dụ:

```bat
build_windows.bat -p COMx flash monitor
```

Để chạy thủ công, mở ESP-IDF PowerShell/CMD đã export môi trường, rồi đặt UTF-8 trước khi gọi Ninja/build:

```powershell
$env:PYTHONUTF8 = "1"
$env:PYTHONIOENCODING = "utf-8"
idf.py build
```

Nạp firmware và theo dõi cổng Serial:
```bash
idf.py -p COMx flash monitor
```

