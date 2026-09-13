# Thiết kế Hệ thống Preset, Tái cấu trúc Menuconfig và Bộ Kiểm định GPIO (ESP32-S3 N16R8)

## 1. Tổng quan & Mục tiêu
Dự án nhằm nâng cao trải nghiệm tùy biến và tính an toàn phần cứng cho bo mạch `ESP32-S3 N16R8 Custom`:
1. **Thiết lập nhanh (Quick Preset):** Giúp người dùng mới hoặc cấu hình chuẩn khởi động ngay bo mạch với combo phổ biến nhất (ST7796, MAX98357A, INMP441, WS2812, Boot Button 0) mà không cần cấu hình thủ công từng mục.
2. **Tái cấu trúc Menu 0–4 nhất quán, gộp phẳng ngoại vi:**
   - `0. Thiết lập nhanh (Quick Setup & Hardware Presets)`
   - `1. Màn hình & Cảm ứng (Display & Touch Screen)`
   - `2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone)`
   - `3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP)`
   - `4. Thiết bị ngoại vi (Peripherals)`: Gộp toàn bộ Camera, LED, Nút bấm, Rơ-le, Mở rộng IO/PMIC, Cảm biến môi trường và Mạch đo pin thành danh sách phẳng. Mỗi mục có checkbox `[ ]` (Dùng / Không dùng). Khi bật `[*]`, menu con chọn model và GPIO mới xổ xuống, khi tắt thì ẩn đi.
3. **Bộ kiểm định GPIO Python (`scripts/gpio_validator.py`):**
   - Đọc trực tiếp file `sdkconfig` đã sinh.
   - Phát hiện xung đột trùng lặp GPIO giữa các thiết bị độc lập.
   - Cho phép các ngoại vi chia sẻ bus hợp lệ (I2C SDA/SCL, I2S Duplex Clock).
   - Phát hiện và cấm tuyệt đối các chân GPIO trong dải `[26, 37]` trên ESP32-S3 N16R8 (chân dùng cho Octal PSRAM & Flash).
   - Cảnh báo các chân strapping / USB nhạy cảm.
4. **Tích hợp build gate vào `scripts/build.py`:**
   - Chặn build trước khi biên dịch nếu phát hiện lỗi vi phạm GPIO.
   - Báo lỗi chi tiết chỉ rõ hai thiết bị và chân GPIO xung đột.
5. **Cảnh báo Kconfig TUI trực quan:**
   - Thêm các dòng cảnh báo động tại bảng tổng hợp GPIO ở đáy menuconfig.
6. **Bộ Unit Test:**
   - Đảm bảo cấu hình N16R8 hiện tại tiếp tục hoạt động 100% tương thích ngược và kiểm tra toàn diện các trường hợp hợp lệ/lỗi.

---

## 2. Chi tiết Cấu trúc Menu `main/Kconfig.projbuild`

### Menu 0: Thiết lập nhanh (Quick Setup & Hardware Presets)
- Symbol: `CUSTOM_PRESET_N16R8_DIY_ENABLED` (bool, default `n`)
- Sử dụng `imply` để thiết lập các giá trị khởi tạo chuẩn:
  - `ENABLE_CUSTOM_DISPLAY=y`, `CUSTOM_DISPLAY_ST7796=y` (MOSI: 47, CLK: 21, CS: 41, DC: 40, RST: 42, BLK: 38)
  - `ENABLE_CUSTOM_SPEAKER=y`, `CUSTOM_AUDIO_DAC_MAX98357A=y` (DOUT: 7, BCLK: 15, LRCK: 16)
  - `ENABLE_CUSTOM_MIC=y`, `CUSTOM_AUDIO_MIC_INMP441=y` (DIN: 6, SCK: 5, WS: 4)
  - `ENABLE_CUSTOM_LEDS=y`, `CUSTOM_LED_WS2812=y` (GPIO 48)
  - `CUSTOM_ENABLE_BUTTON_BOOT=y` (GPIO 0)
- Không dùng `select` cưỡng bức, cho phép người dùng tùy chỉnh sâu ở Menu 1–4 mà không sinh dependency loops.

### Menu 1: Màn hình & Cảm ứng (Display & Touch Screen)
- `ENABLE_CUSTOM_DISPLAY` (menuconfig bool): ST7789, ST7796, ST7701, ILI9341, GC9A01, OLED SSD1306/SH1106, AMOLED QSPI.
- `ENABLE_CUSTOM_TOUCH` (menuconfig bool): CST816S, GT911, FT5x06.

### Menu 2: Âm thanh: Loa & Micro (Audio: Speaker & Microphone)
- `ENABLE_CUSTOM_SPEAKER` (menuconfig bool): MAX98357A, PCM5102A, ES83xx Codec.
- `ENABLE_CUSTOM_MIC` (menuconfig bool): INMP441, PDM, ES83xx Codec ADC.
- `choice`: Simplex vs Duplex I2S.

### Menu 3: Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP)
- `ENABLE_CUSTOM_UART` (menuconfig bool): TX, RX, RTS, CTS, Baudrate.
- `ENABLE_CUSTOM_SECONDARY_NETWORK` (menuconfig bool): Modem 4G (ML307/NT26), Ethernet SPI (W5500/DM9051).
- `ENABLE_CUSTOM_MCP_SERVER` (menuconfig bool): MCP Server tools.

### Menu 4: Thiết bị ngoại vi (Peripherals)
Danh sách phẳng, tất cả các mục đều có định dạng:
`config/menuconfig ENABLE_...` (bool "Tên thiết bị")
`if ENABLE_...`
    (Lựa chọn model / tham số và cấu hình GPIO)
`endif`

Bao gồm:
1. `ENABLE_CUSTOM_CAMERA`: Camera DVP (OV2640, OV3660, OV5640) hoặc USB UVC.
2. `ENABLE_CUSTOM_LEDS`: Đèn LED WS2812, Single LED PWM, Servo DOG PWM.
3. `CUSTOM_ENABLE_BUTTON_BOOT`: Nút BOOT Onboard (GPIO 0).
4. `CUSTOM_ENABLE_BUTTON_TOUCH`: Nút Action / Chạm (PTT/Touch GPIO 1).
5. `CUSTOM_ENABLE_BUTTON_VOLUME`: Phím tăng giảm âm lượng (VOL+ 2, VOL- 3).
6. `CUSTOM_PERIPH_RELAY_ENABLE`: Rơ-le điều khiển thiết bị (GPIO 13).
7. `ENABLE_CUSTOM_IO_EXPANDER_PMIC`: TCA9554, CH32V003, AXP2101, SY6970 (I2C SDA 8, SCL 9).
8. `CUSTOM_ENABLE_IMU_SENSORS`: Cảm biến IMU 6 trục MPU-6050 / BMI270 (I2C SDA 8, SCL 9).
9. `CUSTOM_ENABLE_SENSOR_DHT11_22`: Cảm biến nhiệt ẩm DHT11/22 (GPIO 14).
10. `CUSTOM_ENABLE_SENSOR_I2C_TEMP_HUMID`: Cảm biến nhiệt ẩm số AHT20 / SHT3x (I2C SDA 8, SCL 9).
11. `CUSTOM_ENABLE_SENSOR_BMP280`: Cảm biến khí áp BMP280 / BME280 (I2C SDA 8, SCL 9).
12. `CUSTOM_ENABLE_SENSOR_BH1750`: Cảm biến ánh sáng số BH1750 (I2C SDA 8, SCL 9).
13. `CUSTOM_ENABLE_SENSOR_LDR`: Cảm biến quang trở LDR (ADC1_CH1 GPIO 2).
14. `CUSTOM_ENABLE_SENSOR_HCSR04`: Cảm biến siêu âm HC-SR04 (Trig 11, Echo 12).
15. `CUSTOM_ENABLE_SENSOR_PIR`: Cảm biến chuyển động PIR (GPIO 10).
16. `CUSTOM_ENABLE_TOUCH_SLIDER`: Thanh trượt cảm ứng nội bộ.
17. `CUSTOM_ENABLE_BATTERY_MONITOR`: Đo pin qua ADC phân áp hoặc IC BQ27220 (I2C).

---

## 3. Thiết kế Module `scripts/gpio_validator.py`

### 3.1. Phân loại kiểm tra
1. **Lỗi nghiêm trọng (Errors):**
   - **Xung đột chân trùng:** Hai thiết bị khác nhau sử dụng cùng một GPIO (ngoại trừ bus I2C hợp lệ và I2S Duplex clock).
   - **Chân cấm N16R8 [26..37]:** Bất kỳ chân nào nằm trong khoảng 26 đến 37 được gán cho ngoại vi trên ESP32-S3 N16R8.
2. **Cảnh báo (Warnings):**
   - Dùng chân strapping boot/reset (GPIO 0, 45, 46, 3).
   - Chiếm dụng chân USB OTG/CDC (GPIO 19, 20).

### 3.2. Giao diện hàm (API)
```python
def validate_sdkconfig(
    sdkconfig_path_or_content: Union[str, Path, dict[str, str]],
    is_n16r8: bool = True,
) -> tuple[list[str], list[str]]:
    """
    Returns:
        (errors, warnings): danh sách thông báo lỗi và cảnh báo.
    """
```

---

## 4. Tích hợp Build Gate và Kiểm thử
- `scripts/build.py`: Gọi `validate_sdkconfig` ngay sau bước `_configure_build`. Nếu có `errors`, in thông báo lỗi định dạng nổi bật và dừng build.
- `scripts/tests/test_gpio_validator.py`: Suite test bao phủ preset, valid configs, pin duplicate conflicts, 26-37 forbidden pins, và bus sharing.
