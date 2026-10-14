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
| 4a | LED trạng thái: độ sáng thấp/thường/cao, màu và hiệu ứng cho từng trạng thái | xong (chưa build) |
| 4b | LED đơn GPIO: độ sáng mặc định, hiệu ứng nháy/thở, MCP bật/tắt/độ sáng/hiệu ứng | xong (chưa build) |
| 4c | LED qua PCA9685: chọn kênh dùng cho LED, độ sáng mặc định, hiệu ứng | xong (chưa build) |
| 4d | LED qua IC LED (AW9523B / IS31FL3236): độ sáng mặc định, hiệu ứng | xong (chưa build) |
| 4e | LED qua IC mở rộng (TCA9554 và chip tương tự): chọn chân dùng cho LED, bật/tắt/nháy | xong (chưa build) |

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

## Bước 4a–4e đã làm (2026-10-13, chưa build thử theo yêu cầu)

### 4a. LED trạng thái
- Kconfig trong `ENABLE_CUSTOM_LEDS`:
  - `CUSTOM_LED_BRIGHTNESS_NORMAL` / `_LOW` / `_HIGH` (%, mặc định 10 / 2 / 40).
  - Menu "Màu và hiệu ứng theo trạng thái" với 9 trạng thái: STARTING, WIFI_CONFIG, IDLE, CONNECTING, LISTENING, SPEAKING, UPGRADING, ACTIVATING, ERROR.
  - Mỗi trạng thái có `choice CUSTOM_LED_STATE_<S>_EFFECT` (tắt, đứng yên, nháy nhanh 100 ms, nháy chậm 500 ms, thở, cầu vồng). Cầu vồng chỉ có với WS2812 hoặc vòng LED.
  - Mỗi trạng thái có ký hiệu ẩn `_EFFECT_ID`, và `_COLOR` (chuỗi RRGGBB, chỉ khi LED có màu).
- Độ sáng: Chờ dùng mức thấp; Lỗi và Đang nghe khi có tiếng nói dùng mức cao; các trạng thái còn lại dùng mức thường. Notifying dùng kiểu của Đang nói, AudioTesting dùng kiểu của Đang nghe.
- Mã: `boards/common/configurable_status_led.h` (`ConfigurableSingleLed`, `ConfigurableCircularStrip`, `ConfigurableGpioLed`); `GetLed()` của board dùng các lớp này. Màu sai định dạng thì dùng màu mặc định của trạng thái.
- Bỏ `CUSTOM_LED_FAST_RAINBOW_EFFECT`; thay bằng hiệu ứng "Cầu vồng" theo trạng thái. Web bỏ ô `led_rainbow`. `led/single_led.cc` và `led/circular_strip.cc` bỏ nhánh `#if` của ký hiệu này.

### 4b–4e. Nhóm LED do chatbot điều khiển
- Bộ hiệu ứng dùng chung: `boards/common/user_led_group.h`.
  - `UserLedOutput` là đầu ra; có các loại GPIO PWM/số, PCA9685, IC LED, IC mở rộng.
  - `UserLed` chạy hiệu ứng đứng yên/nháy/thở bằng `esp_timer`.
  - `UserLedGroup` đăng ký các công cụ MCP `<prefix>.get_state`, `turn_on`, `turn_off`, `set_brightness`, `set_effect`.
- Mỗi nhóm có `<P>_BRIGHTNESS` (%), `choice <P>_BOOT_EFFECT` (tắt/đứng yên/nháy/thở) + `<P>_BOOT_EFFECT_ID`, và `<P>_PERIOD_MS`.

| Nhóm | Danh sách | Tiền tố Kconfig | Công cụ MCP |
|---|---|---|---|
| LED đơn GPIO | `CUSTOM_LED_EXTRA_GPIOS` | `CUSTOM_GPIO_LED` | `self.gpio_led.*` |
| PCA9685 | `CUSTOM_PCA9685_LED_CHANNELS` (0-15) | `CUSTOM_PCA9685_LED` | `self.pwm_led.*` (giữ `self.pwm.*` cho servo) |
| IC LED | `CUSTOM_LED_IC_LED_CHANNELS` (trống = tất cả) | `CUSTOM_LED_IC_LED` | `self.led_ic.*` (thay `set_channel`/`set_all` cũ) |
| IC mở rộng | `CUSTOM_EXPANDER_LED_PINS` | `CUSTOM_EXPANDER_LED` | `self.expander_led.*` (giữ `self.expander.*`) |

- IC mở rộng: thêm `choice CUSTOM_EXPANDER_CHIP` (TCA9554/PCA9554/TCA9554A 8 chân; TCA9555/PCA9555/TCA9535/TCA9539 16 chân, driver `esp_io_expander_tca95xx_16bit` đã có sẵn) và `CUSTOM_EXPANDER_I2C_ADDR` (thập phân, mặc định 32 = 0x20). `self.expander.set_pin/get_pin` nhận chân theo số chân của chip.
- Menu LED PCA9685 và IC mở rộng không có `depends on`, vì chip được khai báo sau, ở mục 4.7. Firmware chỉ tạo LED khi chip đã bật và được tìm thấy.

### Trùng kênh PCA9685 giữa LED và servo (2026-10-14)
- Kênh servo trên PCA9685 không khai báo trong Kconfig mà do chatbot chọn lúc chạy, nên việc kiểm tra nằm trong firmware (`InitPwm` trong `periph_controller.cc`):
  - `self.pwm.set_servo` và `self.pwm.set_duty` từ chối kênh có trong `CUSTOM_PCA9685_LED_CHANNELS` và báo dùng `self.pwm_led`. Mô tả công cụ liệt kê các kênh này để chatbot không chọn.
  - `self.pwm.all_off` chỉ tắt các kênh không phải LED; LED tắt bằng `self.pwm_led.turn_off`.
- Đã build trong `C:\x` (WS2812 một mắt, LED đơn GPIO 4/5, IC LED, PCA9685 kênh LED 0/1): không lỗi, không cảnh báo.

### Hiệu ứng vòng LED và theo âm thanh (2026-10-14)
- Mỗi trạng thái có thêm các hiệu ứng:
  - Chỉ cho LED có màu (WS2812 / vòng LED): chạy vòng (6), quét qua lại (7), tô dần (8), cầu vồng xoay (9), thanh dâng theo âm thanh (10), loang hai phía theo âm thanh (11).
  - Cho mọi loại LED: sáng theo nhịp âm thanh (12).
- Hai ô chung:
  - `CUSTOM_LED_EFFECT_SPEED_MS` (10–500, mặc định 50): tốc độ của chạy vòng, quét, tô dần và cầu vồng.
  - `CUSTOM_LED_AUDIO_DELAY_MS` (0–900, mặc định 200): độ trễ để LED khớp tiếng ra loa, bù cho bộ đệm DMA.
- Mức âm lượng: `AudioService::GetOutputLevel()` trả về đỉnh biên độ (0–32767) của khung PCM vừa đưa ra codec. Đây là thay đổi nhỏ trong mã lõi `main/audio/audio_service.{h,cc}`, đã được người dùng đồng ý.
- `StatusLedAudioFollower` (`configurable_status_led.h`):
  - Đọc mức âm lượng mỗi 30 ms và giữ 32 mẫu gần nhất để tạo độ trễ.
  - Mức hiển thị tăng ngay theo âm thanh và giảm dần 20 % mỗi bước.
- Khi chạy trên LED không phải vòng LED:
  - WS2812 1 mắt và LED đơn PWM: chạy vòng, quét, tô dần chuyển thành nháy chậm; cầu vồng xoay chuyển thành cầu vồng.
  - Thanh dâng và loang hai phía trên 1 LED thì sáng theo nhịp.

### Việc còn lại
- Build thử LED trạng thái loại LED đơn PWM, nhóm IC mở rộng; thử trên thiết bị (kiểm tra độ trễ 200 ms có khớp tiếng không).
