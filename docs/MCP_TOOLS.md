# Danh sách công cụ MCP (hàm điều khiển phần cứng)

Tài liệu này liệt kê các **công cụ MCP** mà firmware cung cấp cho chatbot Xiaozhi: tên, chức năng, tham số, dữ liệu trả về và điều kiện để công cụ xuất hiện.

---

## 1. Chatbot đọc cảm biến và điều khiển phần cứng như thế nào

```
Người dùng nói  →  Server Xiaozhi (LLM)  →  gọi công cụ MCP  →  Firmware ESP32
                                                                     │
                     LLM đọc kết quả JSON  ←  trả về JSON  ←  đọc/ghi phần cứng
```

1. Khi kết nối, firmware **tự gửi danh sách công cụ** (tên, mô tả, tham số) lên server.
2. Khi người dùng hỏi (ví dụ "nhiệt độ phòng bao nhiêu?"), LLM chọn công cụ phù hợp (`self.sensor.get_environment`) và gọi nó.
3. Firmware đọc cảm biến, trả về JSON. LLM đọc JSON rồi trả lời bằng lời.

**Không cần chép tên hàm vào Prompt trên xiaozhi.me.** Firmware đã gửi sẵn danh sách và mô tả. Prompt chỉ cần mô tả vai trò và quy tắc trả lời, xem [PROMPT_XIAOZHI.md](PROMPT_XIAOZHI.md).

**MCP tự bật/tắt theo phần cứng đã chọn, không có công tắc tay.**

- Chọn một phần cứng bất kỳ ở mục 4 (cảm biến, relay, servo, động cơ, còi, rung, LED, nút bấm, RTC, NFC...) hoặc chọn **Đèn** ở mục 3 → MCP Server và nhóm công cụ tương ứng tự bật.
- Không chọn gì → MCP tắt, không có công cụ phần cứng nào.
- Bỏ chọn phần cứng → nhóm công cụ của nó tự tắt theo (các ký hiệu `CUSTOM_MCP_TOOL_*` và `ENABLE_CUSTOM_MCP_SERVER` là ký hiệu Kconfig ẩn, luôn được tính lại; giá trị cũ trong `sdkconfig` bị bỏ qua).
- Trong nhóm, chỉ thiết bị có chân GPIO hợp lệ (khác `-1`) và (với chip I2C/SPI) được tìm thấy lúc khởi động mới có công cụ. Không thấy thì chỉ ghi cảnh báo vào log.

| Nhóm công cụ | Tự bật khi chọn |
| :--- | :--- |
| Cảm biến (`self.sensor.*`) | Bất kỳ cảm biến nào ở 4.4/4.5, INA2xx, TP4056 CHRG, BQ27220 |
| Cơ cấu chấp hành (`self.actuator.*`) | Relay, Servo, Motor DC, Còi, Motor rung |
| Robot (`self.robot.*`) | Motor DC hoặc Servo |
| LED (`self.led.*`) | LED trạng thái khác "Không có LED" |
| LED đơn GPIO (`self.gpio_led.*`) | Mục 4.2 → "LED đơn GPIO": danh sách chân không để trống |
| LED qua PCA9685 (`self.pwm_led.*`) | Bật PCA9685 (4.7) và nhập kênh LED ở mục 4.2 → "LED qua PCA9685" |
| LED qua IC LED (`self.led_ic.*`) | Mục 4.2 → "LED qua IC điều khiển LED I2C", chip được tìm thấy |
| LED qua IC mở rộng (`self.expander_led.*`) | Đặt chân I2C cho IO Expander (4.7) và nhập chân LED ở mục 4.2 → "LED qua IC mở rộng GPIO" |
| Ngoại vi (`self.rtc/nfc/pwm/rc522/imu/expander/pmic/ethernet.*`) | RTC, PN532, PCA9685, IC LED, hoặc đặt chân cho RC522, IMU, PMIC, IO Expander, Ethernet |
| Nhấn để nói (`self.set_press_to_talk`) | Nút BOOT hoặc nút chạm |
| Đèn (`self.lamp.*`) | Mục 3: "Đèn điều khiển bằng AI qua GPIO" và có chân đèn |
| Bluetooth UART (`self.bluetooth.*`) | Mục 3: module Bluetooth UART, khi màn hình rời không dùng module này |

Firmware không còn tự dò cảm biến chưa được chọn (trước đây AHT20, BMP280, BH1750, SCD4x, INA219, VL6180X bị dò dù không bật, dễ nhận nhầm chip khác cùng địa chỉ thành cảm biến).

---

## 2. Quy tắc đọc kết quả

| Trường / giá trị | Ý nghĩa |
| :--- | :--- |
| `"valid": false` | Lần đọc này không cảm biến nào trả dữ liệu. Kết quả không chứa số đo nào. |
| Thiếu một trường | Thiết bị không đo đại lượng đó (không gắn cảm biến, hoặc cảm biến đọc lỗi lần này). Firmware không bao giờ trả `0`/`false` thay cho "không có dữ liệu". Mô tả công cụ chỉ liệt kê các trường của cảm biến đã bật. |
| `"success": false` + `"error"` | Lệnh điều khiển thất bại; `code` là mã lỗi ESP-IDF. |
| `true` | Lệnh điều khiển thành công. |

---

## 3. Công cụ chung (luôn có)

| Công cụ | Chức năng | Tham số |
| :--- | :--- | :--- |
| `self.get_device_status` | Trạng thái thiết bị: âm lượng, màn hình, pin, mạng... Nên gọi trước khi điều chỉnh. | — |
| `self.audio_speaker.set_volume` | Đặt âm lượng loa | `volume` 0–100 |
| `self.screen.set_brightness` | Độ sáng màn hình (khi có màn hình có đèn nền) | `brightness` 0–100 |
| `self.screen.set_theme` | Giao diện sáng/tối (khi có màn hình) | `theme`: `light` / `dark` |
| `self.camera.take_photo` | Chụp ảnh và nhờ AI mô tả (khi có camera) | `question`: câu hỏi về ảnh |
| `self.set_press_to_talk` | Đổi chế độ nói: giữ nút hoặc bấm một lần (khi có nút BOOT hoặc nút chạm) | `mode`: `press_to_talk` / `click_to_talk` |

---

## 4. Cảm biến

| Công cụ | Bật khi (Web Configurator → mục) | Tham số |
| :--- | :--- | :--- |
| `self.sensor.get_environment` | Ngoại vi → 4.4: DHT11/22, AHT20/SHT3x, BMP280, BH1750, LDR, CO2 · 4.5: APDS-9960 | — |
| `self.sensor.get_distance` | Ngoại vi → 4.5: HC-SR04 hoặc VL53L0X/VL53L1X | — |
| `self.sensor.get_security` | Ngoại vi → 4.5: PIR, SW-420, cảm biến lửa · 4.4: MQ khí gas | — |
| `self.sensor.get_power` | Nguồn → 4.7: TP4056 (chân CHRG), INA2xx, BQ27220 | — |
| `self.sensor.get_proximity_color` | Ngoại vi → 4.5: APDS-9960 | — |
| `self.sensor.read_gesture` | Ngoại vi → 4.5: APDS-9960 | `timeout_ms` 500–10000 (mặc định 5000) |

### Dữ liệu trả về

**`self.sensor.get_environment`**
```json
{"valid": true, "temperature_c": 28.4, "humidity_pct": 65.0, "light_lux": 320.5}
```
Ví dụ trên: có AHT20 và BH1750, không có BMP280 và SCD4x nên không có `pressure_hpa`, `co2_ppm`. Đơn vị: °C, %, hPa, lux, ppm. Nhiệt độ lấy theo thứ tự ưu tiên AHT20 → DHT → BMP280 → SCD4x. Ánh sáng: BH1750 → APDS-9960 → LDR.

**`self.sensor.get_distance`**
```json
{"valid": true, "laser_distance_mm": 412.0}
```
`laser_distance_mm` từ VL53L0X/VL53L1X, `ultrasonic_distance_cm` từ HC-SR04.

**`self.sensor.get_security`**
```json
{"valid": true, "motion_detected": false, "gas_ppm": 120.5, "gas_leak_alert": false}
```
Ví dụ trên chỉ có PIR và MQ: không gắn cảm biến lửa thì không có `flame_detected` (trước đây trả `false`, tức là khẳng định sai "không có lửa").

**`self.sensor.get_power`**
```json
{"valid": true, "is_charging": true, "battery_voltage": 3.92, "current_ma": 412.0, "power_mw": 1615.0, "battery_percentage": 78}
```
`battery_percentage` chỉ có khi dùng BQ27220.

**`self.sensor.get_proximity_color`**
```json
{"valid": true, "proximity": 180, "clear": 1520, "red": 610, "green": 540, "blue": 330, "light_lux": 412.3}
```
`proximity`: 0 = không có gì phía trước, 255 = vật rất gần.

**`self.sensor.read_gesture`**
```json
{"gesture": "left", "valid": true}
```
Giá trị: `up`, `down`, `left`, `right`, `none`. Công cụ chờ người dùng vẫy tay trong thời gian `timeout_ms`, cách cảm biến khoảng 5–10 cm. Hướng phụ thuộc cách lắp cảm biến.

---

## 5. Ngoại vi I2C

| Công cụ | Bật khi (Nguồn → 4.7) | Tham số | Kết quả |
| :--- | :--- | :--- | :--- |
| `self.rtc.get_time` | RTC DS3231 / PCF8563 | — | `{"rtc_type": "DS3231", "time": "2026-10-10 19:30:00", "valid": true, "temperature_c": 29.25}` |
| `self.rtc.sync_from_system` | RTC DS3231 / PCF8563 | — | `{"success": true}` hoặc lỗi nếu chưa có giờ mạng |
| `self.nfc.read_card` | NFC PN532 | `timeout_ms` 500–15000 (mặc định 5000) | `{"found": true, "uid": "04A1B2C3D4E5F6", "uid_length": 7, "atqa": "0044", "sak": "00", "card_type": "MIFARE Ultralight / NTAG"}` |
| `self.pwm.set_servo` | PCA9685 | `channel` 0–15, `angle` 0–180 | `true`; kênh đã gán cho LED bị từ chối |
| `self.pwm.set_duty` | PCA9685 | `channel` 0–15, `percent` 0–100 | `true`; kênh đã gán cho LED bị từ chối |
| `self.pwm.all_off` | PCA9685 | — | `true`; chỉ tắt các kênh không phải LED |
| `self.expander.set_pin` / `get_pin` | IO Expander TCA9554 (8 chân) hoặc TCA9555/PCA9555/TCA9535/TCA9539 (16 chân) | `pin` theo số chân của chip, `level` 0/1 | `true` / `{"pin": 3, "level": 1}` |

Các công cụ LED của PCA9685, IC LED và IC mở rộng ở mục 6.1.

**RTC tự đồng bộ:** khi khởi động mà chưa có mạng, thiết bị lấy giờ từ RTC. Khi đã nhận giờ từ server, thiết bị ghi lại vào RTC (mỗi 6 giờ một lần). `valid: false` nghĩa là RTC mất pin nuôi và chưa được đặt lại giờ.

**Màn cảm ứng ngoại vi** (Màn hình → "Mở rộng giao tiếp màn cảm ứng ngoại vi") không có công cụ MCP. Có màn hình thì cảm ứng điều khiển giao diện; không có màn hình thì chạm vào để bắt đầu/dừng hội thoại.

---

## 6. Cơ cấu chấp hành, LED, robot

| Công cụ | Bật khi (Web Configurator → mục) | Tham số |
| :--- | :--- | :--- |
| `self.actuator.set_servo` | Ngoại vi → 4.3: Servo | `angle` 0–180 |
| `self.actuator.get_servo` | Ngoại vi → 4.3: Servo | — |
| `self.actuator.detach_servo` | Ngoại vi → 4.3: Servo | — |
| `self.actuator.set_motor` | Ngoại vi → 4.6: Motor DC (TB6612 / L9110S / DRV8833) | `motor_id` chỉ gồm động cơ đã đặt chân PWM và DIR, `speed` −100…100 (âm = quay ngược) |
| `self.actuator.beep` | Ngoại vi → 4.3: Còi | `duration_ms` 10–5000, `frequency_hz` 100–10000 |
| `self.actuator.vibrate` | Ngoại vi → 4.3: Motor rung | `duration_ms` |
| `self.actuator.set_relay` | Ngoại vi → 4.6: Relay | `state` true/false |
| `self.actuator.get_relay` | Ngoại vi → 4.6: Relay | — |
| `self.lamp.turn_on` / `turn_off` / `get_state` | Mạng → 3: "Đèn điều khiển bằng AI qua GPIO", có chân GPIO đèn. Nhiều đèn thì thành `self.lamp_1.*`, `self.lamp_2.*`... Relay không còn tạo công cụ đèn. | — |
| `self.led.set_color` | Ngoại vi → 4.2: LED WS2812 | `red`, `green`, `blue` 0–255 |
| `self.led.set_pixel` | Ngoại vi → 4.2: LED WS2812 (dải nhiều LED) | `index`, `red`, `green`, `blue` |
| `self.led.set_effect` | Ngoại vi → 4.2: LED WS2812 | `effect`: `rainbow`, `chase`, `breathe`, `blink`, `scanner`, `wipe`, `off`, `auto`; `speed_ms` 10–1000 |
| `self.led.set_brightness` | Ngoại vi → 4.2: LED WS2812 | `brightness` 0–100 |
| `self.led.get_state` | Ngoại vi → 4.2: LED WS2812 | — |
| `self.robot.move` | Motor DC, đủ cả hai động cơ | `direction`: `forward`, `backward`, `left`, `right`, `stop`; `speed` 0–100; `duration_ms` 100–5000 |
| `self.robot.gesture` | Servo có chân PWM | `action`: `nod`, `shake`, `center`, `relax` |

Các nhóm tự bật theo phần cứng, xem bảng ở mục 1.

### 6.1 Nhóm LED do chatbot điều khiển

Bốn nhóm dùng chung 5 công cụ, chỉ khác tiền tố: `self.gpio_led`, `self.pwm_led` (PCA9685), `self.led_ic` (AW9523B / IS31FL3236), `self.expander_led` (IC mở rộng).

| Công cụ | Tham số | Kết quả |
| :--- | :--- | :--- |
| `<nhóm>.get_state` | — | `[{"index": 1, "output": "GPIO 4", "power": true, "brightness": 80, "effect": "blink", "period_ms": 1000, "dimmable": true}]` |
| `<nhóm>.turn_on` / `turn_off` | `index`: 1..N chọn một LED, 0 chọn tất cả | `true` |
| `<nhóm>.set_brightness` | `index`, `brightness` 0–100 (0 = tắt) | `true` |
| `<nhóm>.set_effect` | `index`, `effect`: `steady`, `blink`, `breathe`, `off`; `period_ms` 200–10000 | `true` |

- Mô tả công cụ liệt kê LED nào ứng với chân/kênh nào, nên chatbot hiểu "đèn số 2" là LED nào.
- LED không chỉnh độ sáng được (IC mở rộng, LED GPIO vượt số kênh PWM) chỉ bật/tắt; hiệu ứng `breathe` thành `blink`.
- Trong menuconfig, mỗi nhóm có độ sáng mặc định, trạng thái khi khởi động (tắt/sáng/nháy/thở) và chu kỳ nháy/thở.
- LED đơn GPIO: 4 LED đầu chỉnh được độ sáng (kênh LEDC 1, 3, 6, 7; còn 3 nếu LED trạng thái là LED đơn PWM).

### 6.2 LED trạng thái theo từng trạng thái

LED trạng thái (`self.led.*`) tự đổi màu và hiệu ứng theo trạng thái của chatbot: khởi động, cấu hình WiFi, chờ, kết nối, nghe, nói, nâng cấp, kích hoạt, lỗi. Mỗi trạng thái chọn được hiệu ứng và màu trong menuconfig (mục 4.2 → LED trạng thái → "Màu và hiệu ứng theo trạng thái"):
- Mọi loại LED: tắt, đứng yên, nháy nhanh/chậm, thở, sáng theo nhịp tiếng nói.
- LED có màu (WS2812 / vòng LED): thêm cầu vồng, chạy vòng, quét, tô dần, cầu vồng xoay, thanh dâng theo tiếng nói, loang hai phía theo tiếng nói.
- Ba mức độ sáng: thấp cho trạng thái Chờ; cao cho Lỗi và khi đang nghe thấy tiếng nói; thường cho các trạng thái còn lại.
- "Độ trễ hiệu ứng theo âm thanh" (mặc định 200 ms) để LED khớp với tiếng ra loa.

`self.led.set_effect` với `effect: auto` trả LED về chế độ tự đổi theo trạng thái.

**Bluetooth UART** (`self.bluetooth.get_status`, `send_text` hoặc `send_code`, `read_messages`): gửi và nhận dữ liệu với vi điều khiển ở đầu kia qua module HC-05/HC-06/HM-10. Chi tiết ở [BLUETOOTH_UART.md](BLUETOOTH_UART.md).

Motor DC được điều khiển tốc độ bằng PWM 20 kHz. Với L9110S và DRV8833, firmware tự đảo xung khi quay ngược.

---

## 7. Chưa hỗ trợ

Hai mục sau có trong Kconfig nhưng **chưa có driver**, nên firmware không đăng ký công cụ MCP nào cho chúng:

| Mục Kconfig | Tình trạng |
| :--- | :--- |
| IR Remote (hồng ngoại) | Chưa có driver phát/thu tín hiệu. Không có công cụ `self.ir.*`. |
| 4G LTE (A7670 / SIM7600) | Chưa có driver AT cho module. Không có công cụ `self.cellular.*`. Các board mạng 4G/Ethernet/RNDIS (`ml307_board`, `nt26_board`, `dual_network_board`, `rndis_board`, `ethernet_board`) đã xoá ngày 2026-10-13 vì không board nào dùng. |

Hai mục này đã được ghi chú "CHƯA HỖ TRỢ" trong Web Configurator, để không bật nhầm.

## 8. Công cụ chỉ dành cho người dùng

Các công cụ sau **không hiển thị cho AI**, chỉ dùng từ trang quản lý: `self.get_system_info`, `self.reboot`, `self.upgrade_firmware`, `self.screen.get_info`, `self.screen.snapshot`, `self.screen.preview_image`, `self.assets.set_download_url`.

---

## 9. Địa chỉ I2C

Tất cả thiết bị I2C dùng chung một bus. Hai thiết bị trùng địa chỉ sẽ xung đột.

| Thiết bị | Địa chỉ | Ghi chú |
| :--- | :--- | :--- |
| BH1750 | 0x23 / 0x5C | |
| PN532 | 0x24 | Đặt công tắc module sang chế độ I2C |
| VL53L0X / VL53L1X | 0x29 | Chỉ gắn **một** cảm biến laser. VL6180X không có mục chọn nên không được dò |
| AHT20 / FT5x06 (cảm ứng) | 0x38 | Trùng nhau; FT5x06 ngoại vi không được dò khi bật cảm biến nhiệt ẩm I2C |
| APDS-9960 | 0x39 | |
| IS31FL3236 | 0x3C–0x3F | 0x3C/0x3D trùng màn OLED, không được dò khi dùng OLED |
| INA219 / PCA9685 | 0x40 | Trùng nhau; INA219 chỉ được dò khi bật INA2xx |
| TCA9554 / TCA9555 / PCA9555 / TCA9535 (IO Expander) | 0x20–0x27 | Đặt trong "Địa chỉ I2C chip mở rộng GPIO" (thập phân, 32 = 0x20) |
| TCA9554A (IO Expander) | 0x38–0x3F | 0x38 trùng AHT20 / FT5x06, 0x3C–0x3F trùng IS31FL3236 / OLED |
| TCA9539 (IO Expander) | 0x74–0x77 | |
| PCF8563 | 0x51 | |
| BQ27220 | 0x55 | |
| AW9523B | 0x58–0x5B | |
| GT911 (cảm ứng) | 0x5D / 0x14 | |
| SCD4x (CO2) | 0x62 | |
| DS3231 | 0x68 | |
| BMP280 | 0x76 / 0x77 | |
| CST816S (cảm ứng) | 0x15 | |
