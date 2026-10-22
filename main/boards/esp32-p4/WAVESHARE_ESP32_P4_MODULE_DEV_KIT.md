# Tài Liệu Kỹ Thuật Waveshare ESP32-P4-Module-DEV-KIT

Thông số phần cứng và ngoại vi của **Waveshare ESP32-P4-Module-DEV-KIT**.

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

## 3. Cấu Hình Trong Dự Án

Chọn bo trong menuconfig hoặc ô «Thiết bị đích» của Web Configurator; cấu hình ESP-Hosted, chân và cách build ở [README.md](README.md).

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
- Firmware dùng driver EK79007 1024×600; cảm ứng GT911 qua I2C. Cổng DSI 15 chân không có chân reset, đèn nền hay INT/RST cảm ứng.

---

## 4. Tài Liệu

- Sơ đồ nguyên lý: [docs/ESP32-P4-Module-DEV-KIT_Schematic.pdf](docs/ESP32-P4-Module-DEV-KIT_Schematic.pdf)
- Bản vẽ cơ khí: [docs/ESP32-P4-Module-DEV-KIT_Diagram.zip](docs/ESP32-P4-Module-DEV-KIT_Diagram.zip)
- Wiki Waveshare: https://www.waveshare.com/wiki/ESP32-P4-Module-DEV-KIT
- Driver màn hình Waveshare: https://github.com/waveshareteam/ESP32-display-support
- Bảng phân vùng: [partitions/README.md](../../../partitions/README.md)

## 5. Nạp Firmware Cho ESP32-C6

Chip ESP32-C6 trên module là một vi điều khiển slave độc lập, kết nối với ESP32-P4 qua bus SDIO 4-bit nội bộ:
1. **Cổng nạp riêng:** Giắc `P2 (C6_UART)` chuẩn SH1.0 4-pin đưa ra các chân: `C6_TXD`, `C6_RXD`, `C6_IO9`, `GND`.
2. **Firmware yêu cầu:** ESP-Hosted-MCU Slave firmware (biên dịch cho target `esp32c6` với giao diện SDIO).
3. **Khi nạp:** Kéo chân `C6_IO9` xuống mức LOW (GND) trong lúc cắm nguồn để C6 vào chế độ UART Download Mode. Khi xuất xưởng từ Waveshare, chip C6 thường đã được nạp sẵn firmware ESP-Hosted slave.

