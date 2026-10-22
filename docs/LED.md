# Đèn LED

Menuconfig: `Thiết bị ngoại vi [ESP32-S3] → Camera & Đèn LED`. Web Configurator: tab **Ngoại vi**.

## Nhóm LED

| Nhóm | Phần cứng | Màu | Độ sáng | Hiệu ứng | Công cụ MCP |
|---|---|---|---|---|---|
| LED trạng thái | WS2812/SK6812 (1–1024 mắt), vòng LED, LED đơn PWM | có (WS2812) | có | theo trạng thái (bảng dưới) | `self.led.*` |
| LED đơn GPIO | `CUSTOM_LED_EXTRA_GPIOS`, LEDC | không | 4 LED đầu | đứng yên, nháy, thở | `self.gpio_led.*` |
| PCA9685 | `CUSTOM_PCA9685_LED_CHANNELS` (0–15), 12 bit, 50 Hz | không | có | đứng yên, nháy, thở | `self.pwm_led.*` |
| IC LED | AW9523B (16 kênh) / IS31FL3236 (36 kênh), `CUSTOM_LED_IC_LED_CHANNELS` (trống = tất cả) | không | có (256 mức) | đứng yên, nháy, thở | `self.led_ic.*` |
| IC mở rộng | TCA9554 / TCA9555 và tương tự, `CUSTOM_EXPANDER_LED_PINS` | không | không | bật/tắt, nháy | `self.expander_led.*` |

- Mỗi nhóm do chatbot điều khiển có `<P>_BRIGHTNESS` (%), `<P>_BOOT_EFFECT` (trạng thái lúc khởi động) và `<P>_PERIOD_MS` (chu kỳ nháy/thở).
- LED không chỉnh được độ sáng chỉ bật/tắt; `breathe` chuyển thành `blink`.
- Kênh PCA9685 đã gán cho LED bị `self.pwm.set_servo` và `self.pwm.set_duty` từ chối; `self.pwm.all_off` không tắt các kênh này.
- Menu LED qua PCA9685 và IC mở rộng không có `depends on`, vì chip được khai báo sau ở mục «Mở rộng I/O & Ngoại vi khác». Firmware chỉ tạo LED khi chip đã bật và dò thấy.

## Kênh LEDC

| Kênh | Timer | Dùng cho |
|---|---|---|
| 0 | 0 | Đèn nền màn hình |
| 1, 3, 6, 7 | 1 (4 kHz, 13 bit) | LED trạng thái loại PWM (kênh 1) và LED đơn GPIO |
| 2 | 2 | Servo |
| 4, 5 | 3 | Motor DC |

Tính năng mới cần LEDC thì lấy kênh còn trống và cập nhật bảng này.

## LED trạng thái

Chín trạng thái, mỗi trạng thái một menu con: Khởi động, Cấu hình WiFi, Chờ, Đang kết nối, Đang nghe, Đang nói, Nâng cấp firmware, Kích hoạt thiết bị, Lỗi.

**Hiệu ứng** (`CUSTOM_LED_STATE_<S>_EFFECT`, mã trong `_EFFECT_ID`):

| Mã | Hiệu ứng | Mã | Hiệu ứng (chỉ vòng LED) |
|---|---|---|---|
| 0 | Tắt | 6 | Chạy vòng |
| 1 | Sáng đứng yên | 7 | Quét qua lại |
| 2 | Nháy nhanh (100 ms) | 8 | Tô dần |
| 3 | Nháy chậm (500 ms) | 9 | Rạp hát |
| 4 | Thở | 15 | Sao chổi |
| 5 | Nhịp tim | 16 | Sóng sáng |
| 13 | Chớp sáng | 17 | Lấp lánh |
| 14 | Bập bùng | 18 | Hai đầu xoay ngược chiều |
| 12 | Sáng theo nhịp âm thanh | 10 | Thanh dâng theo âm thanh |
| | | 11 | Loang hai phía theo âm thanh |

Trên LED một mắt và LED đơn PWM: rạp hát, sao chổi, hai đầu xoay, chạy vòng, quét, tô dần chuyển thành nháy chậm; sóng sáng thành thở; thanh dâng và loang hai phía thành sáng theo nhịp.

**Nguồn màu** (chỉ WS2812 / vòng LED, `CUSTOM_LED_STATE_<S>_COLOR_SOURCE`, mã trong `_COLOR_SOURCE_ID`): 0 màu cố định (`_COLOR`, dạng `RRGGBB`), 1 cầu vồng, 2 chuyển sắc hai màu (`_COLOR2`), 3 Lửa, 4 Đại dương, 5 Rừng, 6 Hoàng hôn, 7 Băng giá, 8 Kẹo ngọt, 9 Lễ hội.

Nguồn màu khác «cố định» có thêm:
- `_RAINBOW_MODE`: `_WHOLE` đổi màu toàn bộ, `_CHASE` chạy đuổi.
- `_RAINBOW_SPEED`: 1–200 %, mặc định 20. 100 % = 10 ms mỗi bước (0,64 giây một vòng màu).

**Ô chung:**
- `CUSTOM_LED_BRIGHTNESS_LOW` / `_NORMAL` / `_HIGH`: mặc định 2 / 10 / 40 %. Chờ dùng mức thấp; Lỗi và lúc nghe thấy tiếng nói dùng mức cao; các trạng thái khác dùng mức thường.
- `CUSTOM_LED_EFFECT_SPEED_MS` (10–500, mặc định 50): tốc độ chạy vòng, quét, tô dần.
- `CUSTOM_LED_AUDIO_DELAY_MS` (0–900, mặc định 200): độ trễ để LED khớp tiếng ra loa, bù bộ đệm DMA.

## Mã nguồn

- `main/boards/common/configurable_status_led.h`: `ConfigurableSingleLed`, `ConfigurableCircularStrip`, `ConfigurableGpioLed`.
  - `StatusLedTicker`: vẽ khung hình mỗi 30 ms (dải dài: `số LED × 3 / 40` ms).
  - `StatusLedEffectMask`: độ sáng theo hiệu ứng. `StatusLedSourceColor`: màu từ bảng màu.
  - `StatusLedAudioFollower`: đọc `AudioService::GetOutputLevel()` mỗi 30 ms, giữ 32 mẫu để tạo độ trễ.
- `main/boards/common/user_led_group.h`: `UserLedOutput`, `UserLed`, `UserLedGroup` (công cụ `<prefix>.get_state`, `turn_on`, `turn_off`, `set_brightness`, `set_effect`).

Ràng buộc khi sửa mã:
- Bộ vẽ và bộ theo âm thanh bỏ qua khung hình khi `IsCustomMode()`, để không ghi đè lệnh `self.led.*` của chatbot.
- Lớp `Configurable*` ghi đè `TurnOff()` để dừng các timer riêng.
- Không gọi `SetBrightness()` trong `OnStateChanged()` của vòng LED: hàm này gọi lại `OnStateChanged()` và gây tràn stack.
