# Xiaozhi-ESP32 — ESP32-S3-N16R8

Firmware chatbot Xiaozhi cho **ESP32-S3-N16R8** (16 MB Octal Flash, 8 MB Octal PSRAM) và các bo ESP32-P4, build bằng ESP-IDF 6.1.

## Mục lục

1. [Yêu cầu](#1-yêu-cầu)
2. [Web Configurator](#2-web-configurator)
3. [GPIO](#3-gpio)
4. [Biên dịch và nạp](#4-biên-dịch-và-nạp)
5. [Khắc phục lỗi](#5-khắc-phục-lỗi)
6. [Tài liệu kỹ thuật](#6-tài-liệu-kỹ-thuật)
7. [Liên hệ và ủng hộ](#7-liên-hệ-và-ủng-hộ)

---

## 1. Yêu cầu

| Thành phần | Yêu cầu |
| :--- | :--- |
| Hệ điều hành | Windows 10/11 |
| ESP-IDF | 6.1 (ESP-IDF Installation Manager hoặc bản cài cũ) |
| Python | Đi kèm ESP-IDF; Web Configurator cần Python 3 |
| Trình duyệt | Chrome hoặc Edge |
| Cáp | USB có truyền dữ liệu |

---

## 2. Web Configurator

Chọn chân GPIO, màn hình, codec, camera, cảm biến và ngoại vi mà không sửa mã nguồn.

Mở: nhấp đúp `run_configurator.bat` (hoặc `tools/web-configurator/start_configurator.bat`). Trình duyệt mở `http://localhost:8080`; cổng bận thì tự chuyển sang 8081–8099. Mở thẳng `index.html` chỉ để xem, không lưu được.

| Tab | Menuconfig tương ứng |
| :--- | :--- |
| Trợ lý | Ngôn ngữ, OTA, Wake Word, AFE/AEC, gỡ lỗi âm thanh |
| Màn hình | Màn hình (giao diện, thanh trạng thái, Driver Display) |
| Âm thanh | Driver - Loa và Micro, Âm thanh thông báo |
| Mạng | Mạng & Kết nối; Giao tiếp UART, Bluetooth & MCP |
| Ngoại vi | Thiết bị ngoại vi (nút bấm, camera, LED, còi, cảm biến, động cơ), Camera |
| Nguồn | Nguồn điện & Pin; Mở rộng I/O & Ngoại vi khác |
| Hệ thống | CPU, Flash, PSRAM, bảng phân vùng |
| Biên dịch & Nạp | Build, flash, log, chọn ESP-IDF, thiết bị đích, cổng COM |

- Nút **Lưu** ghi toàn bộ cấu hình vào `sdkconfig.defaults` và `sdkconfig`. Web đọc lại `sdkconfig` khi mở, nên thay đổi từ `idf.py menuconfig` cũng hiện lên.
- Hai chức năng trùng chân GPIO thì nút Lưu bị khóa. Thiết bị I2C/SPI dùng chung bus khi cùng vai trò (SDA–SDA, SCK–SCK).
- Mục có nhãn `[ESP32-S3]` chỉ dành cho bo ESP32-S3 và bị ẩn khi thiết bị đích là ESP32-P4.

---

## 3. GPIO

Mọi chân mặc định là `-1` (không dùng), trừ nút BOOT (GPIO 0) và LED WS2812 (GPIO 48). Web Configurator kiểm tra các ràng buộc sau trước khi lưu:

| GPIO | Ràng buộc |
| :--- | :--- |
| 26–37 | Octal Flash và PSRAM. Không dùng được. |
| 19, 20 | USB Serial/JTAG (nạp, log). Hạn chế dùng. |
| 0, 3, 45, 46 | Strapping, ảnh hưởng chế độ khởi động. GPIO 0 là nút BOOT. |
| 1–10 | Kênh ADC1: đo pin, LDR, cảm biến khí MQ chỉ dùng được các chân này. |
| 1–14 | Kênh cảm ứng điện dung (thanh trượt, phím chạm). |

Phân kênh LEDC (đèn nền, LED, servo, motor): [docs/LED.md](docs/LED.md). Địa chỉ I2C của từng thiết bị: [docs/MCP_TOOLS.md](docs/MCP_TOOLS.md) mục 9.

---

## 4. Biên dịch và nạp

### ESP32-S3

- **Web Configurator:** tab **Biên dịch & Nạp** → Build / Flash.
- **Dòng lệnh:** nhấp đúp `02-build_esp32s3.bat`. Script tìm ESP-IDF 6.1, chỉ chạy `set-target esp32s3` khi target khác (set-target xóa `sdkconfig`), rồi hiện menu `menuconfig` / `build`.
- **menuconfig:** chạy `01 set-UTF8-Fix-windows.bat` một lần, mở terminal mới, rồi `idf.py menuconfig`.
- **Nạp và xem log:** `idf.py -p COMx flash monitor`, thoát bằng `Ctrl + ]`.
- Nạp không được: giữ **BOOT**, nhấn và thả **RESET**, thả **BOOT**, rồi nạp lại.
- Sau khi sửa `main/Kconfig.projbuild`: chạy `idf.py reconfigure` trước khi build.

### ESP32-P4

Bo hỗ trợ: Guition JC-ESP32P4-M3-Dev (mặc định), Espressif ESP32-P4-Function-EV-Board, Waveshare ESP32-P4-Module-DEV-KIT. Chân cố định theo bo.

- Dòng lệnh: `03-build_esp32p4.bat`. Web: ô **Thiết bị đích** → chọn bo → `idf.py set-target esp32p4` → Build.
- Build nằm riêng trong `build_p4/` (`build_p4/sdkconfig`), không đụng `build/` và `sdkconfig` của ESP32-S3. Web chỉ ghi phần cài đặt chung vào `sdkconfig.defaults.esp32p4`.
- Mặc định cho chip P4 v1.x; chip v3.x đổi `CONFIG_ESP32P4_SELECTS_REV_LESS_V3` / `CONFIG_ESP32P4_REV_MIN_100`.
- Chi tiết: [main/boards/esp32-p4/README.md](main/boards/esp32-p4/README.md).

---

## 5. Khắc phục lỗi

| Triệu chứng | Cách xử lý |
| :--- | :--- |
| `run_configurator.bat` báo không có Python | Cài ESP-IDF hoặc Python 3. |
| Lưu xong nhưng mã nguồn không đổi | Đang mở HTML trực tiếp; mở qua `run_configurator.bat`. |
| Sửa `Kconfig.projbuild` nhưng Web không đổi | Chạy `tools/web-configurator/sync_menuconfig.bat`. |
| Chữ tiếng Việt lỗi trong terminal | Chạy `01 set-UTF8-Fix-windows.bat`, mở terminal mới. |
| `Khong tim thay ESP-IDF 6.1` | Cài ESP-IDF 6.1, hoặc chạy `02-build_esp32s3.bat` từ terminal ESP-IDF 6.1. |
| `Missing file ... .h`, `undefined reference` sau khi thêm/đổi tên file | Xóa `build/` rồi build lại. |
| Build lỗi sau khi đổi target | Xóa `build/` và `sdkconfig`, chạy `idf.py set-target esp32s3`. |
| Không thấy cổng COM | Thay cáp dữ liệu, kiểm tra Device Manager. |
| `Failed to connect to ESP32-S3` | Đưa board vào chế độ nạp (mục 4). |
| Khởi động lại liên tục, `Guru Meditation Error`, `Out of memory` | Xem log `idf.py monitor`; kiểm tra PSRAM (tab Hệ thống) và chân vừa đổi. |
| Màn hình trắng/đen | Chân SPI/I2C, nguồn 3,3 V, đúng driver màn hình. |
| Loa không có tiếng | Chân I2S (BCLK, WS, DOUT), chân PA bật amply. |
| Thiết bị I2C timeout | Điện trở kéo lên SDA/SCL, không trùng địa chỉ. |

---

## 6. Tài liệu kỹ thuật

| Tài liệu | Nội dung |
| :--- | :--- |
| [docs/MCP_TOOLS.md](docs/MCP_TOOLS.md) | Công cụ MCP: tham số, dữ liệu trả về, điều kiện xuất hiện, địa chỉ I2C |
| [docs/LED.md](docs/LED.md) | Nhóm LED, kênh LEDC, hiệu ứng và màu LED trạng thái |
| [docs/UART_DISPLAY_PROTOCOL.md](docs/UART_DISPLAY_PROTOCOL.md) | Giao thức màn hình rời UART (Nextion, JSON, Raw, DWIN) |
| [docs/BLUETOOTH_UART.md](docs/BLUETOOTH_UART.md) | Module Bluetooth HC-05/HC-06/HM-10: đấu nối, lệnh AT, MCP |
| [docs/STATUS_BAR_UI.md](docs/STATUS_BAR_UI.md) | Dòng trạng thái và hiệu ứng nghe/nói |
| [docs/NOTIFICATION_SOUNDS.md](docs/NOTIFICATION_SOUNDS.md) | Âm thanh thông báo, tệp `.ogg` tùy chỉnh |
| [docs/MEMORY_DMA_TASKS.md](docs/MEMORY_DMA_TASKS.md) | Bộ nhớ, DMA, task FreeRTOS và số đo |
| [docs/BUILD_NOTES.md](docs/BUILD_NOTES.md) | Lý do các thiết lập CMake, phân vùng, đóng gói assets |
| [docs/PROMPT_XIAOZHI.md](docs/PROMPT_XIAOZHI.md) | Prompt mẫu cho xiaozhi.me |
| [main/audio/README.md](main/audio/README.md) | Kiến trúc AudioService |
| [partitions/README.md](partitions/README.md) | Bảng phân vùng 16 MB |
| [main/boards/esp32-p4/README.md](main/boards/esp32-p4/README.md) | Các bo ESP32-P4 |
| [Danh_Sach_Kconfig_WebUI.md](Danh_Sach_Kconfig_WebUI.md) | Bảng Kconfig ↔ Web Configurator (tự sinh) |

Kiểm thử Web Configurator: `tools/web-configurator/run_all_tests.bat` (lần đầu chạy `npm install` trong thư mục đó).

---

## 7. Liên hệ và ủng hộ

- **Nhóm Zalo:** https://zalo.me/g/fkyi7ksh3rcddiqvkufu
- **YouTube:** https://www.youtube.com/@tuyhung-diy
- **TikTok:** https://www.tiktok.com/@gocdiy
- **GitHub:** https://github.com/minhthongvgt/xiaozhi-diy

### Ủng hộ dự án qua BIDV

Quét mã QR bằng ứng dụng ngân hàng BIDV:

<img src="tools/web-configurator/logo/BIDV-nganhang.png" alt="Mã QR ủng hộ qua BIDV" width="260">

### Tham gia nhóm Zalo

Quét mã QR bằng Zalo để vào nhóm:

<img src="tools/web-configurator/logo/Zalogroup.png" alt="Mã QR nhóm Zalo" width="260">
