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

### 1.2 Danh mục Cấu hình Chuẩn theo Hệ thống Menuconfig (Đã Tối Ưu, Không Đánh Số)
Giao diện được căn chỉnh theo tỉ lệ công thái học hiện đại (Sidebar 260px, Khung trung tâm 1380px, Font Outfit + JetBrains Mono), đã **loại bỏ hoàn toàn đánh số thứ tự** giúp danh mục luôn tinh gọn, tích hợp cơ chế **Tự Động Nhận Diện Cấu Hình (Auto-Detect)**, **Lưu theo từng Tab (Per-Tab Save)** và **Bộ theo dõi thay đổi thông minh (Dirty Tracking)**:
- Nút lưu được bố trí ở góc trên bên phải của **từng danh mục**. Khi người dùng ở danh mục nào sẽ lưu thông tin cho danh mục đó.
- Hệ thống tự động kiểm tra thay đổi: hiển thị nhãn `● Chưa lưu` trên thanh điều hướng và chỉ nhắc nhở người dùng khi có sự thay đổi chưa được lưu trước khi chuyển tab.
- **Quy chuẩn Giao diện**: Toàn bộ phong cách giao diện và biểu cảm AI được quy chuẩn tập trung tại **Flash Assets**, loại bỏ hoàn toàn tùy chọn trùng lặp tại danh mục màn hình.

- 🌐 **Cơ bản & Ngôn ngữ (General - Quy chuẩn Giao diện)**:
  - URL máy chủ cập nhật OTA, ngôn ngữ đàm thoại chính (Tiếng Việt, English, 中文).
  - **Gói tài nguyên biểu cảm & Giao diện (Flash Assets - Quy chuẩn duy nhất)**: Gói biểu cảm mặc định, Biểu cảm cơ bản, hoặc **Giao diện tùy chỉnh từ Server Xiaozhi (.bin)**.
  - *Hướng dẫn vị trí file giao diện tùy chỉnh*: Đặt file `assets.bin` (hoặc tên tùy chỉnh) ngay tại **thư mục gốc dự án** (`assets.bin`). Trình biên dịch ESP-IDF sẽ tự động nạp nhị phân vào phân vùng flash `assets` (địa chỉ `0x800000`).
- 🔊 **Âm thanh: Loa & Micro (I2S Audio - Phân tách 2 phần độc lập)**:
  - **Phần 1 - Loa / DAC / Amply**: Chọn Driver (MAX98357A, PCM5102A, NS4168, ES8311, ES8388); chân I2S BCLK, WS/LRCK, DOUT; chân bật/tắt Amply công suất (PA).
  - **Phần 2 - Micro thu âm**: Chọn Driver (INMP441, ICS-43434, MSM261S, ES8311 ADC, ES7210); chân I2S SCK, WS, DIN; chân I2C SDA/SCL cho chip Codec nếu sử dụng.
  - Tần số lấy mẫu mặc định 16000 Hz chuẩn AI Voice (OPUS codec).
- 🖥️ **Màn hình & Cảm ứng (Display & Touch - Phần cứng hiển thị)**:
  - Hỗ trợ màn hình SPI: ST7789, GC9A01, ILI9341, SSD1306 OLED, Headless (Không dùng màn hình).
  - Hỗ trợ **Màn hình rời qua giao tiếp UART (Nextion / D-UART)**: Cấu hình cổng UART (UART 1, UART 2), Baudrate (115200, 921600,...), chân UART TX, UART RX.
  - **Driver Chip Cảm ứng (Touch)**: Hỗ trợ Không dùng cảm ứng (None), CST816S, GT911, FT6236 (cấu hình chân I2C SDA, SCL, ngắt INT, reset RST).
  - Đảo màu màn hình (Invert Color) và sóng âm thoại động. (Không còn trường UI style trùng lặp).
- 🎙️ **Từ khóa & Giọng nói (WakeNet)**: WakeNet tích hợp AFE khử vọng hoặc bản nhẹ; độ nhạy nhận diện; chuỗi Pinyin từ khóa; khử tiếng vọng trên máy chủ (Server AEC).
- 📶 **Cấp Wi-Fi & Mạng (Network)**: Cấp Wi-Fi qua Web Hotspot SoftAP (`Xiaozhi-XXXX`) hoặc Bluetooth BLE BluFi; Mạng mở rộng Modem 4G LTE (Quectel ML307, EC801E) hoặc Ethernet W5500.
- 🤖 **Điều khiển MCP & IoT (AI Tool Calling)**:
  - Kích hoạt máy chủ MCP trên thiết bị (`CONFIG_ENABLE_CUSTOM_MCP_SERVER`), cho phép AI Xiaozhi điều khiển thiết bị qua hàm Tool Calling.
  - *Triệt tiêu xung đột chân*: Tab MCP quản lý tính năng phần mềm AI (Bật/tắt MCP, quyền điều khiển ngoại vi, đọc trạng thái phần cứng, khởi động lại hệ thống, chuyển đổi giao diện). **Toàn bộ chân phần cứng Relay / Đèn bàn được quản lý duy nhất tại Tab Ngoại vi**, loại bỏ hoàn toàn lỗi xung đột chân chéo.
- 🔘 **Phím bấm & Điều khiển**: Nhấn **"+ Thêm phím bấm"** để mở hộp thoại cấu hình: Phím BOOT (GPIO 0), Touch Button, WAKE Button, Volume Up / Down, Chiết áp xoay vô cực EC11.
- 🔌 **Ngoại vi & Cảm biến (Quét từ `main/drivers`)**: Nhấn **"+ Thêm linh kiện"** để chọn: LED trạng thái, Rơ-le/Đèn bàn (`LAMP_GPIO`), Còi Buzzer, Động cơ Haptic, Servo PWM, Cầu H Động cơ DC TB6612, Laser ToF VL6180X, Cảm biến nhiệt độ DHT11/22, Khí áp, Siêu âm HC-SR04, Rung, Lửa, Quản lý sạc TP4056, Pin ADC.
- 📡 **Giao tiếp & Thẻ nhớ**: Nhấn **"+ Thêm kết nối"** để cấu hình: Thẻ nhớ MicroSD qua SPI (CS, MOSI, MISO, SCLK), Cổng UART mở rộng, I2C Bus dùng chung.
- ⚡ **Phần cứng ESP32-S3-N16R8 (Luôn ở dưới cùng danh mục)**:
  - Bảng thông số phần cứng trực quan: Flash 16MB (QIO 80MHz), PSRAM 8MB (Octal 80MHz), CPU 240MHz, Phân vùng `16m.csv` (Dual OTA 6.5MB x 2 + Assets).
  - Khóa cứng an toàn dải **GPIO 26–32** chống xung đột bus Octal Flash/PSRAM.

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
| **Cổng kết nối mở rộng** | GPIO 1, 11, 12, 35, 36, 37, 39, 43, 44 | Tự do cấu hình cho Còi Buzzer (41), Rung Haptic (42), Động cơ DC TB6612, Encoder EC11, Cảm biến siêu âm, Thẻ nhớ SD Card. |

---

## 3. Quy trình Biên dịch mã nguồn (ESP-IDF 6.1)

Sau khi bấm **"Lưu & Áp dụng Cấu hình" (Save & Apply)** trên giao diện Web Configurator, mã nguồn cấu hình sẽ tự động được đồng bộ vào:
- `main/boards/custom-s3-n16r8/config.h`
- `main/boards/custom-s3-n16r8/config.json`
- `main/boards/custom-s3-n16r8/custom_s3_n16r8_board.cc`
- Tự động bổ sung bo mạch vào `main/Kconfig.projbuild` và `main/CMakeLists.txt`.

Mở terminal trong thư mục dự án và tiến hành biên dịch:

```bash
# Lựa chọn 1: Biên dịch và đóng gói firmware release
python scripts/release.py custom-s3-n16r8

# Lựa chọn 2: Biên dịch trực tiếp qua ESP-IDF
idf.py build
```

Nạp firmware và theo dõi cổng Serial:
```bash
idf.py -p COMx flash monitor
```

---

## 4. Ghi chú Môi trường Thử nghiệm (Test Environment)

- **Mã nguồn & Công cụ chính thức**: Được phát triển và lưu trữ tập trung tại `d:\Code\Antigravity\xiaozhi-v2`.
- **Thư mục chạy thử nghiệm của người dùng**: `c:\xiaozhi` (*"đây là nơi tôi chạy thử"*).
- **Quy ước xử lý lỗi**: Mọi báo cáo lỗi, log biên dịch hoặc đường dẫn tệp phát sinh liên quan đến `c:\xiaozhi` đều thuộc về môi trường chạy thử độc lập của người dùng, không phản ánh lỗi của mã nguồn chính thức tại `d:\Code\Antigravity\xiaozhi-v2`.
