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

**Chỉ thiết bị đã bật mới có công cụ.** Tắt một cảm biến trong Web Configurator thì công cụ của nó biến mất khỏi danh sách, chatbot sẽ không gọi được nữa.

---

## 2. Quy tắc đọc kết quả

| Trường / giá trị | Ý nghĩa |
| :--- | :--- |
| `"valid": false` | Không có cảm biến nào trả dữ liệu (chưa gắn, gắn sai chân, hỏng). **Không** được đọc các số còn lại như số đo thật. |
| Một số đo bằng `0` khi `valid` là `true` | Cảm biến tương ứng không có mặt. Ví dụ có AHT20 nhưng không có BMP280 thì `pressure_hpa` = 0. |
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
| `self.set_press_to_talk` | Đổi chế độ nói: giữ nút hoặc bấm một lần | `mode`: `press_to_talk` / `click_to_talk` |

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
{"temperature": 28.4, "humidity": 65.0, "light_lux": 320.5, "pressure_hpa": 1008.2, "co2_ppm": 0, "tvoc_ppb": 0, "valid": true}
```
Đơn vị: °C, %, lux, hPa, ppm, ppb. Nhiệt độ lấy theo thứ tự ưu tiên AHT20 → DHT → BMP280 → SCD4x. Ánh sáng: BH1750 → APDS-9960 → VL6180X → LDR.

**`self.sensor.get_distance`**
```json
{"laser_distance_mm": 412.0, "ultrasonic_distance_cm": 0.0, "valid": true}
```
`laser_distance_mm` từ VL53L0X/VL53L1X (hoặc VL6180X), `ultrasonic_distance_cm` từ HC-SR04.

**`self.sensor.get_security`**
```json
{"motion_detected": false, "vibration_detected": false, "flame_detected": false, "gas_ppm": 120.5, "gas_leak_alert": false, "valid": true}
```

**`self.sensor.get_power`**
```json
{"battery_voltage": 3.92, "battery_percentage": 78, "is_charging": true, "current_ma": 412.0, "power_mw": 1615.0, "valid": true}
```
`battery_percentage` chỉ có khi dùng BQ27220; các mạch khác trả 0.

**`self.sensor.get_proximity_color`**
```json
{"proximity": 180, "clear": 1520, "red": 610, "green": 540, "blue": 330, "light_lux": 412.3, "valid": true}
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
| `self.pwm.set_servo` | PCA9685 | `channel` 0–15, `angle` 0–180 | `true` |
| `self.pwm.set_duty` | PCA9685 | `channel` 0–15, `percent` 0–100 | `true` |
| `self.pwm.all_off` | PCA9685 | — | `true` |
| `self.led_ic.set_channel` | IC LED AW9523B / IS31FL3236 | `channel` (AW9523B: 0–15, IS31FL3236: 0–35), `brightness` 0–255 | `true` |
| `self.led_ic.set_all` | IC LED AW9523B / IS31FL3236 | `brightness` 0–255 | `true` |

**RTC tự đồng bộ:** khi khởi động mà chưa có mạng, thiết bị lấy giờ từ RTC. Khi đã nhận giờ từ server, thiết bị ghi lại vào RTC (mỗi 6 giờ một lần). `valid: false` nghĩa là RTC mất pin nuôi và chưa được đặt lại giờ.

**Màn cảm ứng ngoại vi** (Màn hình → "Mở rộng giao tiếp màn cảm ứng ngoại vi") không có công cụ MCP. Có màn hình thì cảm ứng điều khiển giao diện; không có màn hình thì chạm vào để bắt đầu/dừng hội thoại.

---

## 6. Cơ cấu chấp hành, LED, robot

| Công cụ | Bật khi (Web Configurator → mục) | Tham số |
| :--- | :--- | :--- |
| `self.actuator.set_servo` | Ngoại vi → 4.3: Servo | `angle` 0–180 |
| `self.actuator.get_servo` | Ngoại vi → 4.3: Servo | — |
| `self.actuator.detach_servo` | Ngoại vi → 4.3: Servo | — |
| `self.actuator.set_motor` | Ngoại vi → 4.6: Motor DC (TB6612 / L9110S / DRV8833) | `motor_id` 1–2, `speed` −100…100 (âm = quay ngược) |
| `self.actuator.beep` | Ngoại vi → 4.3: Còi | `duration_ms` 10–5000, `frequency_hz` 100–10000 |
| `self.actuator.vibrate` | Ngoại vi → 4.3: Motor rung | `duration_ms` |
| `self.actuator.set_relay` | Ngoại vi → 4.6: Relay | `state` true/false |
| `self.actuator.get_relay` | Ngoại vi → 4.6: Relay | — |
| `self.lamp.turn_on` / `turn_off` / `get_state` | Mạng → 3. MCP Server: công cụ Đèn, hoặc Ngoại vi → 4.6: Relay | — |
| `self.led.set_color` | Ngoại vi → 4.2: LED WS2812 | `red`, `green`, `blue` 0–255 |
| `self.led.set_pixel` | Ngoại vi → 4.2: LED WS2812 (dải nhiều LED) | `index`, `red`, `green`, `blue` |
| `self.led.set_effect` | Ngoại vi → 4.2: LED WS2812 | `effect`: `rainbow`, `chase`, `breathe`, `blink`, `scanner`, `wipe`, `off`, `auto`; `speed_ms` 10–1000 |
| `self.led.set_brightness` | Ngoại vi → 4.2: LED WS2812 | `brightness` 0–100 |
| `self.led.get_state` | Ngoại vi → 4.2: LED WS2812 | — |
| `self.robot.move` | Motor DC hoặc Servo Dog | `direction`: `forward`, `backward`, `left`, `right`, `stop`; `speed` 0–100; `duration_ms` 100–5000 |
| `self.robot.gesture` | Motor DC hoặc Servo Dog | `action`: `nod`, `shake`, `center`, `relax` |

Motor DC được điều khiển tốc độ bằng PWM 20 kHz. Với L9110S và DRV8833, firmware tự đảo xung khi quay ngược.

---

## 7. Công cụ chưa hoạt động

| Công cụ | Tình trạng |
| :--- | :--- |
| `self.ir.send_remote` | Chỉ có khung lệnh, chưa phát tín hiệu hồng ngoại thật. Chưa được bật trong firmware. |
| `self.cellular.send_sms`, `self.cellular.get_status` | Chỉ có khung lệnh, chưa giao tiếp với modem 4G (luôn trả "đã gửi", sóng 28). Chưa được bật trong firmware. |

Hai nhóm này sẽ không xuất hiện cho tới khi có driver thật, để chatbot không báo sai kết quả.

---

## 8. Công cụ chỉ dành cho người dùng

Các công cụ sau **không hiển thị cho AI**, chỉ dùng từ trang quản lý: `self.get_system_info`, `self.reboot`, `self.upgrade_firmware`, `self.screen.get_info`, `self.screen.snapshot`, `self.screen.preview_image`, `self.assets.set_download_url`.

---

## 9. Địa chỉ I2C

Tất cả thiết bị I2C dùng chung một bus. Hai thiết bị trùng địa chỉ sẽ xung đột.

| Thiết bị | Địa chỉ | Ghi chú |
| :--- | :--- | :--- |
| BH1750 | 0x23 / 0x5C | |
| PN532 | 0x24 | Đặt công tắc module sang chế độ I2C |
| VL53L0X / VL53L1X / VL6180X | 0x29 | Chỉ gắn **một** cảm biến laser |
| AHT20 / FT5x06 (cảm ứng) | 0x38 | Trùng nhau; FT5x06 ngoại vi không được dò khi bật cảm biến nhiệt ẩm I2C |
| APDS-9960 | 0x39 | |
| IS31FL3236 | 0x3C–0x3F | 0x3C/0x3D trùng màn OLED, không được dò khi dùng OLED |
| INA219 / PCA9685 | 0x40 | Trùng nhau; khi bật PCA9685 mà không bật INA2xx thì không dò INA219 |
| PCF8563 | 0x51 | |
| BQ27220 | 0x55 | |
| AW9523B | 0x58–0x5B | |
| GT911 (cảm ứng) | 0x5D / 0x14 | |
| SCD4x (CO2) | 0x62 | |
| DS3231 | 0x68 | |
| BMP280 | 0x76 / 0x77 | |
| CST816S (cảm ứng) | 0x15 | |
