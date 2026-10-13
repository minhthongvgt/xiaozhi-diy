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
| 2 | Nối `CUSTOM_LED_EXTRA_GPIOS` thành LED đơn (PWM LEDC) cho chatbot điều khiển | chưa làm |
| 3 | Tách các nhóm LED trong 4.2 thành menu con riêng (xem bảng nhóm) | chưa làm |
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
- `CUSTOM_LED_EXTRA_GPIOS` có trên menuconfig và Web nhưng không có mã nào đọc.
- Đèn MCP (`CUSTOM_MCP_TOOL_LAMP*`) chỉ có bật/tắt.
- Công cụ MCP hiện có: PCA9685 `self.pwm.*`, IC LED `self.led_ic.*`, IC mở rộng `self.expander.*`.
