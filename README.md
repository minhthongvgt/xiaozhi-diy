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

Sidebar hiện có 8 tab (mỗi tab là một component trong `tools/web-configurator/components/`, nạp động qua `js/main.js`). Các tab phần cứng được sinh tự động từ `main/Kconfig.projbuild`: nhóm, thứ tự, tên mục, danh sách lựa chọn và điều kiện ẩn/hiện giống hệt `idf.py menuconfig`.

| Tab (data-target) | Nhóm | Menu tương ứng trong menuconfig |
| :--- | :--- | :--- |
| `panel-assistant` | 1. Xiaozhi Assistant | Ngôn ngữ, OTA URL, Audio Debugger · Từ khóa đánh thức (Wake Word) · Bộ xử lý âm thanh & Khử tiếng vọng (AEC) |
| `panel-display` | 1. Xiaozhi Assistant | Giao diện hiển thị & Gói tài nguyên · 1. Màn hình & Cảm ứng |
| `panel-audio` | 1. Xiaozhi Assistant | 2. Âm thanh: Loa & Micro |
| `panel-network` | 1. Xiaozhi Assistant | WiFi Configuration Method · 3. Giao thức truyền thông & Mạng (UART mở rộng, MCP Server) |
| `panel-peripherals` | 1. Xiaozhi Assistant | 4.1 – 4.6 (nút bấm, camera, LED, còi, rung, servo, cảm biến, motor, relay) · Camera Configuration |
| `panel-power` | 1. Xiaozhi Assistant | 4.7. Nguồn điện, Bộ nhớ & Viễn thông (pin, INA2xx, TP4056, RTC, NFC, SD, 4G, IR...) |
| `panel-system` | 2. ESP-IDF Cốt lõi | CPU, Flash, PSRAM, bảng phân vùng |
| `panel-build` | 2. ESP-IDF Cốt lõi | Build, flash, nhật ký idf.py, chọn ESP-IDF, target, cổng COM |

Giao diện tích hợp **Lưu theo từng Tab**, **Theo dõi thay đổi** và **Kiểm tra trùng chân GPIO**:
- Nút lưu nằm ở góc trên bên phải mỗi tab. Lưu sẽ ghi toàn bộ cấu hình, nên mọi tab đều chuyển sang trạng thái "Đã lưu".
- Khi rời tab có thay đổi chưa lưu, hộp thoại cho phép **Lưu rồi chuyển**, **Bỏ thay đổi** (khôi phục giá trị đã lưu của tab đó) hoặc **Ở lại**.
- Trùng chân GPIO sẽ khoá nút lưu; nhiều thiết bị được dùng chung một bus I2C/SPI khi nối cùng vai trò (SDA với SDA, SCK với SCK...).
- Mở thẳng một tab qua URL, ví dụ `http://localhost:8080/#panel-build`.

---

## 2. Quy tắc Sơ đồ chân GPIO (ESP32-S3-N16R8 Pinout Rules)

| Phân vùng chân | Dải GPIO | Chú thích an toàn |
| :--- | :--- | :--- |
| **Octal Flash & Octal PSRAM** | **GPIO 26 – 37** | **NGHIÊM CẤM SỬ DỤNG**. Được nối cứng nội bộ cho bus dữ liệu Octal SPI (26-32) và Octal PSRAM (33-37) tốc độ cao trên ESP32-S3-N16R8. Hệ thống tự động khóa và cảnh báo đỏ nếu người dùng gán vào dải này. |
| **Native USB & JTAG Console** | **GPIO 19, 20** | Dùng cho cổng nạp và debug USB Serial/JTAG tích hợp. Hạn chế sử dụng làm GPIO thông thường để tránh mất kết nối nạp/debug. |
| **Strapping Pins** | **GPIO 0, 3, 45, 46** | GPIO 0 dùng cho phím BOOT Onboard (mặc định Bật). Chú ý mức logic khi khởi động để tránh vào sai chế độ nạp ROM. |
| **Trạng thái Mặc định (Clean Defaults)** | **Boot = 0, LED = 48** | Sau khi `set-target esp32s3`, phím Boot (GPIO 0) và Đèn LED WS2812 (GPIO 48, 1 LED) được bật sẵn. Mọi ngoại vi chưa kết nối đều có chân mặc định là `-1` (an toàn tuyệt đối, không crash khởi động). |
| **I2S Audio (Phần 1: Loa)** | GPIO 14, 15, 7, 2 | BCLK, WS/LRCK, DOUT (MAX98357A / PCM5102A / NS4168 / ES8311 / ES8388). Chân PA bật amply. Mặc định chưa nối là `-1`. |
| **I2S Audio (Phần 2: Mic)** | GPIO 4, 5, 6 | SCK, WS, DIN (INMP441 / ICS-43434 / MSM261S / ES7210). Mặc định chưa nối là `-1`. |
| **UART Display (Nextion/DWIN/JSON)** | **GPIO 17, 18** | TX=17, RX=18 trên `UART_NUM_1` (Màn hình thông minh rời; tự động escape chuỗi, chống tràn khung DGUS). |
| **Custom UART Subsystem** | **UART_NUM_2** | Cổng UART mở rộng cho Modem 4G, AI Vision, Bus Servo (Tự động cách ly với UART Display). |
| **Touch Controller (Cảm ứng)** | GPIO 8, 9, 3, 10 | I2C SDA=8, SCL=9, ngắt INT=3, reset RST=10 (CST816S, GT911, FT6236). |
| **I2C Bus & Cảm biến** | GPIO 8, 9 | SDA=8, SCL=9 (Hỗ trợ OLED SSD1306, ToF VL6180X, Codec ES8311, cảm biến môi trường). |
| **Servo PWM (Động cơ)** | **GPIO 13** | Tần số 50Hz, 14-bit duty trên LEDC Timer 2 (Độc lập chống xung đột backlight/LED). |
| **Relay / Lamp (Ngoại vi)** | **GPIO 45** | Rơ-le 220V / Đèn bàn, liên kết tự động với công cụ AI MCP. |
| **Built-in LED** | GPIO 48 | Đèn LED WS2812 báo trạng thái hệ thống (Mặc định 1 LED, Bật sẵn). |
| **Touch Slider (Thanh trượt cảm ứng)** | **GPIO 1, 2, 3** | Touch Pad 1, 2, 3 nội bộ ESP32-S3 (FSM đo điện dung). Dải kênh cho phép từ -1 đến 48. |
| **Phím âm lượng (Volume Buttons)** | **GPIO 47, 48 (hoặc chân tự do)** | Nút nhấn kéo trở Pull-up. Tự động cách ly với Touch Slider khi kích hoạt cả hai. |
| **Cổng kết nối mở rộng an toàn** | GPIO 1, 2, 4..7, 10..13, 14..18, 21, 38..44, 47 | Tự do cấu hình cho Còi Buzzer (41), Rung Haptic (42), Động cơ DC TB6612, Encoder EC11, Cảm biến siêu âm, Thẻ nhớ SD Card SPI, Ethernet W5500. |

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
5. **Bộ kiểm thử**: chạy `tools/web-configurator/run_all_tests.bat` (`test_modules.mjs`, `test_sdkconfig_io.py`, `test_dom.cjs`, `test_project_integration.mjs`, `test_kconfig_parity.mjs`, `test_gui_dom.mjs`; test cuối cần chạy `npm install` một lần trong `tools/web-configurator/`).
6. **Sau khi sửa `main/Kconfig.projbuild`**: chạy `tools/web-configurator/sync_menuconfig.bat` để sinh lại giao diện, cập nhật `Danh_Sach_Kconfig_WebUI.md` và đối chiếu sdkconfig sinh ra với kconfiglib của ESP-IDF (cần đã cấu hình build một lần, ví dụ `idf.py -B build_sim reconfigure`).

Yêu cầu cài ESP-IDF 6.1 cho ESP32-S3 bằng Windows installer, gồm Python environment và compiler/toolchain. Bạn có thể biên dịch trực tiếp từ **Web Configurator** (Tab *Biên dịch & Nạp*) hoặc chạy dòng lệnh từ thư mục gốc:

```bat
build_windows.bat
```

Launcher tự nạp `export.bat`, đặt Python ở UTF-8 và gọi `idf.py` trong môi trường ESP-IDF. Nó dùng `IDF_PATH` nếu đã có; nếu không, tự tìm một số thư mục cài đặt phổ biến. Lệnh build giữ nguyên `sdkconfig` hiện tại. Có thể truyền lệnh ESP-IDF khác, ví dụ:

```bat
build_windows.bat -p COMx flash monitor
```

Để mở giao diện `idf.py menuconfig` mượt mà và hiển thị đúng tiếng Việt / ký tự đặc biệt trên Windows:
1. Nhấp đúp chạy file `01-set_utf8.bat` ở thư mục gốc (tự động thiết lập codepage 65001 và biến môi trường UTF-8).
2. Chạy lệnh:
```powershell
idf.py menuconfig
```

Để biên dịch thủ công qua dòng lệnh, mở ESP-IDF PowerShell/CMD đã export môi trường:
```powershell
idf.py build
```

Nạp firmware và theo dõi cổng Serial:
```bash
idf.py -p COMx flash monitor
```

