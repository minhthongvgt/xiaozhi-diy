# Kế hoạch nhóm LED (bắt đầu 2026-10-13)

Mục tiêu: trong menuconfig mục 4.2 "Hệ thống chính", mỗi nhóm LED có menu con riêng. Mỗi nhóm có tùy chọn độ sáng, màu và hiệu ứng ở mức phần cứng hỗ trợ. Chatbot điều khiển được qua MCP.

Mỗi bước làm theo cùng một quy trình:
1. Sửa Kconfig.
2. Sửa firmware.
3. Chạy `sync_menuconfig.bat` và `run_all_tests.bat`.
4. Build trong `C:\x`.
5. Commit và push.

## Danh sách việc

| # | Việc | Trạng thái |
|---|---|---|
| 1 | Đổi tên mục 4.2 thành "Hệ thống chính (Main System)", giữ nguyên vị trí và nội dung | xong |
| 2 | Nối `CUSTOM_LED_EXTRA_GPIOS` thành LED đơn (PWM LEDC) cho chatbot điều khiển | xong |
| 3 | Tách các nhóm LED trong 4.2 thành menu con riêng (xem bảng nhóm) | xong (PCA9685 và IC mở rộng làm ở 4c, 4e) |
| 4a | LED trạng thái: độ sáng thấp/thường/cao, màu và hiệu ứng cho từng trạng thái | chưa làm |
| 4b | LED đơn GPIO: độ sáng mặc định, hiệu ứng nháy/thở, MCP bật/tắt/độ sáng/hiệu ứng | chưa làm |
| 4c | LED qua PCA9685: chọn kênh dùng cho LED, độ sáng mặc định, hiệu ứng | chưa làm |
| 4d | LED qua IC LED (AW9523B / IS31FL3236): độ sáng mặc định, hiệu ứng | chưa làm |
| 4e | LED qua IC mở rộng (TCA9554 và chip tương tự): chọn chân dùng cho LED, bật/tắt/nháy | chưa làm |

## Các nhóm LED và khả năng phần cứng

| Nhóm | Phần cứng | Màu | Độ sáng | Hiệu ứng |
|---|---|---|---|---|
| LED trạng thái | WS2812/SK6812 (1–1024 mắt), LED đơn PWM, vòng LED | có (WS2812) | có | nháy, thở, cầu vồng (WS2812) |
| LED đơn GPIO | LEDC PWM. ESP32-S3 có 8 kênh LEDC, dùng chung với đèn nền, còi, servo, LED trạng thái PWM | không | có | nháy, thở |
| PCA9685 | 16 kênh PWM 12 bit, chạy 50 Hz (dùng chung với servo) | không | có | nháy, thở (phần mềm) |
| IC LED | AW9523B 16 kênh / IS31FL3236 36 kênh | không | có (256 mức) | nháy, thở (phần mềm) |
| IC mở rộng | TCA9554 8 chân (chip khác cần driver tương ứng) | không | không | bật/tắt, nháy |

## Hiện trạng trước khi làm
- Màu và độ sáng của LED trạng thái đang gán cứng trong `main/led/single_led.cc` và `main/led/circular_strip.cc`.
- `CUSTOM_LED_EXTRA_GPIOS` có trên menuconfig và Web nhưng không có mã nào đọc (đã nối ở bước 2).
- Đèn MCP (`CUSTOM_MCP_TOOL_LAMP*`) chỉ có bật/tắt.
- Công cụ MCP hiện có: PCA9685 `self.pwm.*`, IC LED `self.led_ic.*`, IC mở rộng `self.expander.*`.

## Bước 2 đã làm
- `boards/common/gpio_led_controller.h`: `GpioUserLed` (một LED) và `GpioLedController` (đăng ký MCP).
- Công cụ MCP: `self.gpio_led.get_state`, `turn_on`, `turn_off`, `set_brightness` (0–100), `set_effect` (`steady`/`blink`/`breathe`, `period_ms`). `index` = 0 chọn tất cả.
- Kênh LEDC cho LED đơn: 1, 3, 6, 7 (bỏ kênh 1 khi LED trạng thái là LED đơn PWM). Hết kênh thì LED chỉ bật/tắt và nháy; hiệu ứng thở chuyển thành nháy. Hiệu ứng chạy bằng `esp_timer`.
- Bảng kênh LEDC: đèn nền kênh 0 (timer 0); LED trạng thái PWM và LED đơn kênh 1/3/6/7 (timer 1, 4 kHz, 13 bit); servo kênh 2 (timer 2); motor kênh 4/5 (timer 3).
- Ký hiệu ẩn `CUSTOM_MCP_TOOL_GPIO_LED` tự bật khi danh sách chân không rỗng. Chân trùng LED trạng thái, trùng nhau hoặc không hợp lệ bị bỏ qua và ghi log. `gpio_validator.py` kiểm tra trùng chân và vùng cấm cho danh sách này.

## Bước 3 đã làm
Mục 4.2 "Hệ thống chính" gồm:
- Camera (`ENABLE_CUSTOM_CAMERA`).
- LED trạng thái (`ENABLE_CUSTOM_LEDS`).
- Menu con "LED đơn GPIO (chatbot điều khiển)": `CUSTOM_LED_EXTRA_GPIOS`, không còn phụ thuộc LED trạng thái; để trống là không dùng.
- "LED qua IC điều khiển LED I2C" (`CUSTOM_ENABLE_PERIPH_LED_DRIVER_IC` + SDA/SCL): chuyển từ 4.7 sang 4.2.

PCA9685 và TCA9554 còn phục vụ servo và IO chung, nên chip vẫn ở 4.7. Menu LED của hai chip này sẽ thêm ở 4.2 trong bước 4c và 4e, và phụ thuộc vào việc bật chip.
