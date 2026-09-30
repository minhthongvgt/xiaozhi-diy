# Kế Hoạch Chi Tiết & Checklist Cập Nhật Driver Màn Hình (ESP-IDF 6.1)

> **Tuân thủ quy chuẩn nghiêm ngặt:** [IDF.md](../../.agents/rules/IDF.md) và [agent-hardware-bus](../../.agents/skills/agent-hardware-bus/SKILL.md).  
> **Mục tiêu:** Cập nhật và hoàn thiện từng driver màn hình còn thiếu hoặc bị ánh xạ sai trong danh sách cấu hình của dự án.

---

## 1. Danh Sách Chi Tiết Các Chip Cần Cập Nhật

| STT | Chip / Nhóm Màn Hình | Giao Tiếp | Tình Trạng Hiện Tại | Giải Pháp Cập Nhật |
| :---: | :--- | :--- | :--- | :--- |
| **1** | **Sitronix ST7735 / ST7735S** | SPI 1-data | Bị gọi nhầm sang driver ST7789 | Viết driver chuyên biệt `esp_lcd_st7735.c/.h` với bảng lệnh chuẩn ST7735 và offset linh hoạt |
| **2** | **Solomon SSD1681** | SPI E-Paper | Sai tên macro (`SSD1680`) & rơi xuống ST7789 | Sửa macro, kết nối component `espressif/esp_lcd_ssd1681` đã có trong dự án |
| **3** | **Sitronix ST7701** | RGB Parallel | Bị gán `NoDisplay()` | Kết nối component `espressif/esp_lcd_st7701`, khởi tạo RGB bus và panel |
| **4** | **AMOLED QSPI (SH8601 / CO5300 / SPD2010)** | QSPI (4-data) | Bị gán `NoDisplay()` | Khởi tạo bus QSPI, kết nối các driver `esp_lcd_sh8601`, `co5300`, `spd2010` |

---

## 2. Checklist Kiểm Tra Toàn Diện (Inspection Checklist)

- [x] **Task 1: Cập nhật Driver Sitronix ST7735 / ST7735S (SPI Mini)**
  - [x] Tạo tệp header `main/boards/esp32s3-n16r8-custom/esp_lcd_st7735.h`
  - [x] Tạo tệp mã nguồn `main/boards/esp32s3-n16r8-custom/esp_lcd_st7735.c` chuẩn `esp_lcd_panel_t`
  - [x] Đăng ký `esp_lcd_st7735.c` tự động qua CMake (`boards/${BOARD_DIR}/*.c`)
  - [x] Thêm nhánh `#elif defined(CONFIG_CUSTOM_DISPLAY_ST7735)` gọi `esp_lcd_new_panel_st7735()` trong [custom_n16r8_board.cc](../../main/boards/esp32s3-n16r8-custom/custom_n16r8_board.cc)

- [x] **Task 2: Cập nhật Driver Solomon SSD1681 (Mực điện tử E-Paper)**
  - [x] Sửa điều kiện macro trong [custom_n16r8_board.cc](../../main/boards/esp32s3-n16r8-custom/custom_n16r8_board.cc) để nhận diện `CONFIG_CUSTOM_DISPLAY_EPAPER_SSD1681`
  - [x] Kết nối header `<esp_lcd_panel_ssd1681.h>` từ component `espressif/esp_lcd_ssd1681`
  - [x] Khởi tạo bus SPI chuyên dụng cho E-Paper và gọi `esp_lcd_new_panel_ssd1681()`
  - [x] Xử lý an toàn chân BUSY và RESET theo đúng chuẩn timing E-Paper, khởi tạo `OledDisplay`

- [x] **Task 3: Cập nhật Driver Sitronix ST7701 (RGB Interface)**
  - [x] Kết nối header `<esp_lcd_st7701.h>` và `<esp_lcd_panel_io_additions.h>`
  - [x] Cấu hình 3-wire SPI điều khiển thanh ghi kết hợp RGB Parallel Bus truyền điểm ảnh (16-bit)
  - [x] Khởi tạo panel qua `esp_lcd_new_panel_st7701()` trong [custom_n16r8_board.cc](../../main/boards/esp32s3-n16r8-custom/custom_n16r8_board.cc)
  - [x] Gắn panel vào `RgbLcdDisplay` với timing chuẩn

- [x] **Task 4: Cập nhật Driver AMOLED QSPI (SH8601 / CO5300 / SPD2010)**
  - [x] Kết nối các header `<esp_lcd_sh8601.h>`, `<esp_lcd_co5300.h>`, `<esp_lcd_spd2010.h>`
  - [x] Cấu hình SPI bus ở chế độ Quad-SPI (4 đường dữ liệu IO0-IO3)
  - [x] Tạo panel IO QSPI qua `esp_lcd_new_panel_io_spi()` với cờ `quad_mode` và cmd 32-bit
  - [x] Khởi tạo panel tương ứng và liên kết vào `SpiLcdDisplay`

- [x] **Task 5: Kiểm Thử Tự Động & Xác Minh Toàn Diện**
  - [x] Viết kịch bản kiểm thử tĩnh `tools/test_display_drivers.py`
  - [x] Xác minh 100% các option trong Kconfig đều có driver tương ứng (17/17)
  - [x] Xác minh không còn bất kỳ màn hình nào bị fallback về `NoDisplay()` khi được chọn
  - [x] Báo cáo kết quả chi tiết từng driver cho người dùng
