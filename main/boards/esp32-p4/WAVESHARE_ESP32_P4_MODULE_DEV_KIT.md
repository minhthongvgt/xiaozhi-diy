# Tài Liệu Kỹ Thuật Waveshare ESP32-P4-Module-DEV-KIT

Tài liệu này tổng hợp toàn bộ thông số phần cứng, sơ đồ chân kết nối, ngoại vi và hướng dẫn tích hợp cho kit phát triển **Waveshare ESP32-P4-Module-DEV-KIT** trong dự án Xiaozhi.

---

## 1. Thông Số Phần Cứng Cốt Lõi

### Module Trung Tâm: ESP32-P4-Module
- **SoC:** ESP32-P4 (ESP32-P4NRW32)
  - Nhân hiệu năng cao (HP): Dual-core RISC-V 32-bit xung nhịp 360 MHz - 400 MHz (hỗ trợ tập lệnh FPU, Vector & DSP).
  - Nhân công suất thấp (LP): Single-core RISC-V 32-bit xung nhịp 40 MHz.
  - Bộ nhớ L2MEM nội: 768 KB HP L2MEM + 32 KB LP SRAM + 8 KB TCM.
- **PSRAM tích hợp:** 32 MB PSRAM (đóng gói SiP tốc độ cao).
- **Flash tích hợp:** 16 MB Nor Flash.
- **Kết nối không dây:** Tích hợp chip đồng xử lý ESP32-C6 cùng anten gốm SMD. Giao tiếp qua bus SDIO 3.0 (giao thức `esp_hosted`), hỗ trợ Wi-Fi 6 (802.11ax 2.4 GHz) và Bluetooth 5 (LE).

---

## 2. Bố Trí Cổng & Ngoại Vi Trên Bo Mạch

| Ký Hiệu / Nhãn In | Loại Giao Tiếp | Chức Năng Chi Tiết |
| :--- | :--- | :--- |
| **40PIN Header** | 2x20 Pin Header (2.54mm) | Chuẩn chân tương thích Raspberry Pi, cung cấp 28 chân GPIO lập trình tự do |
| **USB Type-A (Dual)** | 2 cổng USB 2.0 High-Speed | USB OTG 2.0 (480 Mbps), có jumper chuyển đổi HOST hoặc DEVICE |
| **100MB RJ45** | Cổng mạng Ethernet 10/100 Mbps | Kết nối mạng có dây, hỗ trợ mở rộng module cấp nguồn PoE qua header PoE |
| **USB TO UART** | USB Type-C | Cổng nạp chương trình, debug UART và cấp nguồn 5V chính |
| **USB** | USB Type-C | Cổng USB 2.0 OTG kết nối trực tiếp vào SoC ESP32-P4 |
| **RTC** | Giắc SH1.0 2-pin | Gắn pin sạc RTC (Rechargeable RTC Battery) duy trì thời gian thực |
| **I2C / I3C** | Giắc SH1.0 4-pin | Mở rộng cảm biến I2C và bus cảm biến thế hệ mới I3C |
| **DISPLAY** | Cổng FPC 15-pin (MIPI-DSI) | Giao diện hiển thị 2-lane MIPI-DSI cho màn hình LCD độ phân giải cao |
| **CAMERA** | Cổng FPC 15-pin (MIPI-CSI) | Giao diện thu hình 2-lane MIPI-CSI cho camera HD (OV5647, SC2336) |
| **SPK** | Giắc MX1.25 2-pin | Ngõ ra loa công suất 8Ω 2W (kèm chip Audio Codec và khuếch đại PA) |
| **MIC** | Microphone SMD | Micro thu âm tích hợp trên bo mạch |
| **C6_UART** | Giắc SH1.0 4-pin | Cổng giao tiếp UART riêng để nạp và debug chip phụ ESP32-C6 |
| **BOOT / RST** | Nút bấm vật lý | Nút BOOT (vào Download Mode) và nút RST (Reset phần cứng) |
| **TF Card** | Khe cắm thẻ MicroSD | Giao tiếp thẻ nhớ ngoài tốc độ cao chuẩn SDIO 3.0 |

---

## 3. Cấu Hình Tương Thích Với Dự Án Xiaozhi

### Thiết Lập Menuconfig Cho ESP32-P4
Khi chuyển target sang ESP32-P4 (`idf.py set-target esp32p4`), các cờ cấu hình bắt buộc sau cần được kích hoạt để kích hoạt Wi-Fi và mạng thông qua ESP32-C6:

```text
CONFIG_SLAVE_IDF_TARGET_ESP32C6=y
CONFIG_ESP_HOSTED_P4_DEV_BOARD_FUNC_BOARD=y
CONFIG_ESP_HOSTED_SDIO_HOST_INTERFACE=y
CONFIG_ESP_HOSTED_SDIO_4_BIT_BUS=y
CONFIG_SPI_FLASH_SUPPORT_GD_CHIP=y
```

### Âm Thanh (Audio Codec)
- Codec âm thanh: ES8311 (I2S + I2C).
- Cấu hình I2S chuẩn:
  - MCLK: GPIO 13
  - BCLK: GPIO 12
  - WS (LRCK): GPIO 10
  - DOUT: GPIO 9
  - DIN: GPIO 11
  - PA PIN: GPIO 53
  - I2C Codec: SDA GPIO 7, SCL GPIO 8

### Màn Hình Hiển Thị (Display)
- Chuẩn giao tiếp: MIPI-DSI 2-lane.
- Chip điều khiển LCD: Hỗ trợ các driver MIPI-DSI thông dụng như JD9365, ST7701, EK79007 (kèm cảm biến cảm ứng GT911 qua bus I2C).

---

## 4. Tài Liệu Kỹ Thuật Đã Tải Về Lưu Offline

Các tệp tài liệu nguyên bản từ nhà sản xuất Waveshare đã được lưu trữ trực tiếp trong thư mục `docs/`:

1. **Sơ đồ nguyên lý mạch hoàn chỉnh (Schematic PDF):**
   - Đường dẫn: `docs/ESP32-P4-Module-DEV-KIT_Schematic.pdf`
   - Dung lượng: 2.37 MB

2. **Bản vẽ kích thước cơ khí và bố trí linh kiện (Diagram ZIP):**
   - Đường dẫn: `docs/ESP32-P4-Module-DEV-KIT_Diagram.zip`
   - Dung lượng: 6.48 MB

---

## 5. Liên Kết Tham Khảo Trực Tuyến

- **Wiki chính thức Waveshare:** https://www.waveshare.com/wiki/ESP32-P4-Module-DEV-KIT
- **Gói Demo mã nguồn (ESP-IDF / Arduino):** https://drive.google.com/file/d/1RizQiBkyXZ7rcbHLdYhvHiFMcQuvkNjN/view?usp=sharing
- **Kho driver màn hình Waveshare:** https://github.com/waveshareteam/ESP32-display-support

---

## 6. Thiết Kế Phân Vùng 16MB & Nạp Firmware ESP32-C6 Slave

### Thiết Kế Phân Vùng Flash 16MB (`partitions/16m.csv`)
Bo mạch Waveshare ESP32-P4 Dev Kit sử dụng chip Flash 16MB (QIO 80MHz). Bảng phân vùng được cấu hình tối ưu như sau:

| Tên phân vùng | Loại | Phân loại con | Offset | Kích thước | Mục đích sử dụng |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `nvs` | `data` | `nvs` | `0x9000` | 64 KB (`0x10000`) | Lưu cấu hình Wi-Fi, token API và tham số hệ thống |
| `otadata` | `data` | `ota` | `0x19000` | 8 KB (`0x2000`) | Quản lý chuyển đổi slot OTA kép |
| `phy_init` | `data` | `phy` | `0x1b000` | 4 KB (`0x1000`) | Tương thích cấu trúc phân vùng chuẩn |
| `app0` | `app` | `ota_0` | `0x20000` | **4.5 MB** (`0x480000`) | Firmware chính thức (ESP-Hosted + LVGL 9 + Audio + Camera) |
| `app1` | `app` | `ota_1` | `0x4A0000` | **4.5 MB** (`0x480000`) | Firmware dự phòng khi cập nhật OTA không dây |
| `model` | `data` | `spiffs` | `0x920000` | **6.875 MB** (`0x6E0000`) | Lưu trữ Assets, Font Unicode, Model nhận diện giọng nói |

### Nạp Firmware Slave Cho Chip ESP32-C6 (Wi-Fi 6 / BLE 5)
Chip ESP32-C6 trên module là một vi điều khiển slave độc lập, kết nối với ESP32-P4 qua bus SDIO 4-bit nội bộ:
1. **Cổng nạp riêng:** Giắc `P2 (C6_UART)` chuẩn SH1.0 4-pin đưa ra các chân: `C6_TXD`, `C6_RXD`, `C6_IO9`, `GND`.
2. **Firmware yêu cầu:** ESP-Hosted-MCU Slave firmware (biên dịch cho target `esp32c6` với giao diện SDIO).
3. **Khi nạp:** Kéo chân `C6_IO9` xuống mức LOW (GND) trong lúc cắm nguồn để C6 vào chế độ UART Download Mode. Khi xuất xưởng từ Waveshare, chip C6 thường đã được nạp sẵn firmware ESP-Hosted slave.

