# 🤖 BẢNG CHỈ THỊ PROMPT & ÁNH XẠ CÔNG CỤ MCP CHO TOÀN BỘ NGOẠI VI XIAOZHI

Tài liệu này là cẩm nang quy chuẩn dành cho **Kỹ sư Hệ thống AI, Lập trình viên Backend và Prompt Engineer** nhằm hướng dẫn mô hình ngôn ngữ lớn (LLM như GPT-4o, Claude 3.5, Gemini 2.0, DeepSeek) nhận diện chính xác ý định giọng nói của người dùng và gọi đúng công cụ **MCP (Model Context Protocol)** tương ứng với từng ngoại vi phần cứng trên bo mạch **ESP32-S3 N16R8 Xiaozhi**.

---

## 📑 MỤC LỤC
1. [Nguyên Lý Hoạt Động Của Giao Thức MCP Trên Xiaozhi](#1-nguyên-lý-hoạt-động-của-giao-thức-mcp-trên-xiaozhi)
2. [Phân Loại Cơ Chế Tác Động Của Ngoại Vi (Callable Tools vs Event-Driven)](#2-phân-loại-cơ-chế-tác-động-của-ngoại-vi)
3. [Bảng Ma Trận Tra Cứu: 41 Ngoại Vi ⟷ Hàm MCP ⟷ Tham Số ⟷ Dữ Liệu Trả Về](#3-bảng-ma-trận-tra-cứu-41-ngoại-vi--hàm-mcp)
4. [Đặc Tả Chi Tiết Toàn Bộ Công Cụ MCP (MCP Tool Specifications)](#4-đặc-tả-chi-tiết-toàn-bộ-công-cụ-mcp)
5. [Khuôn Mẫu System Prompt Chuẩn Cho AI (Production System Prompt Template)](#5-khuôn-mẫu-system-prompt-chuẩn-cho-ai)
6. [Bộ Kịch Bản & Câu Lệnh Mẫu Thực Tế (Few-Shot In-Context Examples)](#6-bộ-kịch-bản--câu-lệnh-mẫu-thực-tế)
7. [Quy Trình Xử Lý Lỗi & Phản Hồi Khi Ngoại Vi Không Khả Dụng](#7-quy-trình-xử-lý-lỗi--phản-hồi-khi-ngoại-vi-không-khả-dụng)

---

## 1. Nguyên Lý Hoạt Động Của Giao Thức MCP Trên Xiaozhi

Hệ thống Xiaozhi giao tiếp với AI Backend thông qua giao thức **MCP (Model Context Protocol)** dựa trên chuẩn **JSON-RPC 2.0** truyền qua WebSocket hoặc MQTT:

```mermaid
sequenceDiagram
    autonumber
    actor User as Người dùng (Voice)
    participant HW as Phần cứng ESP32-S3
    participant MCP as MCP Server (ESP-IDF)
    participant Cloud as AI Backend / LLM
    
    User->>HW: "Nhiệt độ phòng bây giờ là bao nhiêu độ?"
    HW->>Cloud: Audio Stream (I2S Mic -> Opus Encoded)
    Cloud->>Cloud: STT -> Phân tích ngữ cảnh -> Chọn Tool "self.sensor.get_environment"
    Cloud->>MCP: JSON-RPC tools/call: {"name": "self.sensor.get_environment", "arguments": {}}
    MCP->>HW: Đọc cảm biến I2C (AHT20/SHT30)
    HW-->>MCP: Dữ liệu vật lý (temp=28.5, hum=65.2)
    MCP-->>Cloud: JSON Result: {"temperature": 28.5, "humidity": 65.2, ...}
    Cloud->>Cloud: LLM tổng hợp câu trả lời tự nhiên
    Cloud->>HW: TTS Audio: "Dạ thưa anh, nhiệt độ phòng hiện tại là 28.5 độ C và độ ẩm là 65% ạ."
    HW->>User: Phát ra loa I2S (MAX98357A / ES8311)
```

---

## 2. Phân Loại Cơ Chế Tác Động Của Ngoại Vi

Trong hệ thống 41 ngoại vi được chuẩn hóa của Xiaozhi, các ngoại vi được chia thành **2 cơ chế vận hành độc lập**:

### 2.1. Ngoại Vi Chủ Động Gọi Hàm (Callable MCP Tools - Pull / Execute)
* **Đặc điểm:** AI có thể chủ động ra quyết định triệu gọi bất kỳ lúc nào để đọc dữ liệu cảm biến hoặc điều khiển cơ cấu chấp hành.
* **Bao gồm:**
  * **Cảm biến:** Môi trường (`get_environment`), Khoảng cách (`get_distance`), An ninh & Báo cháy (`get_security`), Pin & Nguồn điện (`get_power`).
  * **Cơ cấu chấp hành:** Rơ-le 220V (`set_relay`), Servo góc xoay (`set_servo`), Động cơ DC (`set_motor`), Còi chip (`beep`), Rung haptic (`vibrate`).
  * **Ánh sáng & Biểu cảm:** LED RGB (`set_color`, `set_effect`), Cử chỉ robot (`gesture`), Di chuyển robot (`move`).
  * **Điều khiển từ xa & Viễn thông:** Hồng ngoại IR 38kHz (`send_remote`), Tin nhắn 4G SMS (`send_sms`, `get_status`).
  * **Hệ thống cốt lõi:** Âm lượng loa (`set_volume`), Độ sáng màn hình (`set_brightness`), Chụp ảnh AI (`take_photo`), Trạng thái máy (`get_device_status`).

### 2.2. Ngoại Vi Nhận Ngắt Sự Kiện (Event-Driven / Push to System)
* **Đặc điểm:** Vi điều khiển ESP32 xử lý trực tiếp thông qua cơ chế ngắt cứng (GPIO Interrupt / I2C Event) mà **AI không cần gọi hàm để quét liên tục**. Khi sự kiện xảy ra, hệ thống tự động kích hoạt phiên hội thoại hoặc gửi thông báo trạng thái tới AI.
* **Bao gồm:**
  * Phím bấm vật lý BOOT / WAKE (GPIO 0, 47): Nhấn để kích hoạt nói hoặc ngắt lời AI.
  * Phím cảm ứng điện dung (TouchPad GPIO 1-14): Chạm chạm để tương tác nhanh.
  * Công tắc gạt 2 trạng thái (Slide Switch GPIO 48): Khóa/Mở Mic phần cứng (Hardware Mute).
  * Núm xoay vô cấp EC11: Xoay để chỉnh âm lượng trực tiếp, nhấn để chuyển bài.
  * Cảm biến cử chỉ không chạm APDS-9960: Vẫy tay trái/phải để chuyển trạng thái.
  * Đầu đọc thẻ từ NFC PN532 / RFID RC522: Quẹt thẻ để nhận diện người dùng hoặc nạp lệnh tự động.
  * Cảm biến lưu lượng nước YF-S201 (PCNT Pulse Counter): Đếm xung phần cứng độc lập.
  * Mạng CAN Bus / TWAI & Cổng phụ RS485: Nhận khung dữ liệu truyền về từ xe ô tô hoặc máy móc công nghiệp.

---

## 3. Bảng Ma Trận Tra Cứu: 41 Ngoại Vi ⟷ Hàm MCP

Dưới đây là bảng đối chiếu trực tiếp giữa toàn bộ 41 ngoại vi chuẩn hóa (Menu 4 Kconfig) với tên hàm MCP, tham số truyền và kiểu dữ liệu trả về:

| STT | Nhóm Chức Năng | Tên Ngoại Vi / Cảm Biến | Chipset Đại Diện | Tên Hàm MCP Tool Tương Ứng | Tham Số Đầu Vào (`arguments`) | Dữ Liệu Trả Về (`result`) |
|:---:|:---|:---|:---|:---|:---|:---|
| **1** | 1. Tương tác & Đầu vào | Nút bấm vật lý | Tactile BOOT/WAKE | `self.set_press_to_talk` *(chế độ)* | `mode: "press_to_talk" \| "click_to_talk"` | `true \| false` |
| **2** | 1. Tương tác & Đầu vào | Chạm cảm ứng điện dung | ESP32-S3 Touchpad | *Event-driven (Ngắt cứng)* | N/A *(Tự động gửi sự kiện)* | N/A |
| **3** | 1. Tương tác & Đầu vào | Công tắc gạt 2 trạng thái | Slide Switch | *Hardware Mute (Ngắt Mic)* | N/A *(Ngắt tín hiệu I2S Mic)* | N/A |
| **4** | 1. Tương tác & Đầu vào | Núm xoay vô cấp | EC11 Rotary Encoder | `self.audio_speaker.set_volume` | `volume: 0..100` *(hoặc xoay tay)* | `true` |
| **5** | 1. Tương tác & Đầu vào | Cử chỉ 3D không chạm | APDS-9960 | *Event-driven / Gesture Interrupt* | N/A *(Kích hoạt kịch bản vẫy tay)* | Gesture code |
| **6** | 1. Tương tác & Đầu vào | Màn hình cảm ứng rời | CST816S / GT911 | *UI Event (Chạm giao diện LVGL)* | N/A *(Xử lý trực tiếp trên LCD)* | Touch X, Y |
| **7** | 2. Thị giác & Ánh sáng | Đèn LED đơn trạng thái | Single GPIO LED | `self.led.set_effect` | `effect: "blink" \| "off"` | `true` |
| **8** | 2. Thị giác & Ánh sáng | Đèn LED RGB địa chỉ | WS2812B / SK6812 | `self.led.set_effect`<br>`self.led.set_color` | `effect: "rainbow" \| "chase" \| "breathe" \| "blink" \| "off", speed_ms: 10..1000`<br>`red: 0..255, green: 0..255, blue: 0..255` | `true`<br>`true` |
| **9** | 2. Thị giác & Ánh sáng | Mở rộng LED đa kênh | AW9523B / IS31FL3731 | `self.led.set_color` | `red, green, blue` | `true` |
| **10** | 2. Thị giác & Ánh sáng | Camera thị giác AI | OV2640 / OV3660 / OV5640 | `self.camera.take_photo` | `question: string` *(câu hỏi thị giác)* | `{"description": string}` |
| **11** | 3. Cảnh báo & Haptics | Mô-tơ rung xúc giác | Haptic Vibration Motor | `self.actuator.vibrate` | `duration_ms: 10..3000` *(thời gian rung)* | `true` |
| **12** | 3. Cảnh báo & Haptics | Còi báo động Buzzer | Active / Passive Buzzer | `self.actuator.beep` | `frequency_hz: 100..10000, duration_ms: 10..5000` | `true` |
| **13** | 4. Môi trường & Khí | Nhiệt độ & Độ ẩm | AHT20 / SHT3x / BMP280 | `self.sensor.get_environment` | `{}` *(không cần đối số)* | `{"temperature": float, "humidity": float, "pressure_hpa": float, ...}` |
| **14** | 4. Môi trường & Khí | Nồng độ CO2 & Khí TVOC | SCD40/41, SGP30/40 | `self.sensor.get_environment` | `{}` | `{"co2_ppm": float, "tvoc_ppb": float, ...}` |
| **15** | 4. Môi trường & Khí | Cường độ ánh sáng Lux | BH1750 / VEML7700 | `self.sensor.get_environment` | `{}` | `{"light_lux": float, ...}` |
| **16** | 4. Môi trường & Khí | Nhiệt độ công nghiệp | DS18B20 (1-Wire) | `self.sensor.get_environment` | `{}` | `{"temperature": float, ...}` |
| **17** | 4. Môi trường & Khí | Cảm biến lưu lượng nước | YF-S201 (PCNT) | `self.sensor.get_environment` | `{}` | `{"water_flow_lpm": float}` |
| **18** | 4. Môi trường & Khí | Khí độc & Khói rò rỉ | MQ-2 / MQ-135 | `self.sensor.get_security` | `{}` | `{"gas_ppm": float, "gas_leak_alert": bool}` |
| **19** | 5. Không gian & An ninh | Cảm biến chuyển động PIR | HC-SR501 / AM312 | `self.sensor.get_security` | `{}` | `{"motion_detected": bool}` |
| **20** | 5. Không gian & An ninh | Radar hiện diện vi sóng | HLK-LD2410 / LD2420 | `self.sensor.get_security` | `{}` | `{"motion_detected": bool}` |
| **21** | 5. Không gian & An ninh | Đo khoảng cách siêu âm | HC-SR04 / US-100 | `self.sensor.get_distance` | `{}` | `{"ultrasonic_distance_cm": float}` |
| **22** | 5. Không gian & An ninh | Khoảng cách Laser ToF | VL6180X / VL53L0X / L1X | `self.sensor.get_distance`<br>`self.sensor.get_environment` | `{}` *(ToF: mm)*<br>`{}` *(ALS: Lux từ VL6180X)* | `{"laser_distance_mm": float}`<br>`{"light_lux": float}` |
| **23** | 5. Không gian & An ninh | Gia tốc con quay IMU | MPU6050 / BMI270 | `self.sensor.get_security` | `{}` | `{"vibration_detected": bool}` |
| **24** | 5. Không gian & An ninh | Cảm biến từ đóng/mở cửa | Hall Sensor / Reed Switch | `self.sensor.get_security` | `{}` | `{"door_closed": bool}` |
| **25** | 5. Không gian & An ninh | Cảm biến chống trộm rung | SW-420 (Rung chấn động) | `self.sensor.get_security` | `{}` | `{"vibration_detected": bool}` |
| **26** | 5. Không gian & An ninh | Cảm biến phát hiện lửa | Flame IR Sensor | `self.sensor.get_security` | `{}` | `{"flame_detected": bool}` |
| **27** | 6. Thẻ từ & RTC | Đầu đọc thẻ thông minh | PN532 (NFC I2C/UART) | *Event-driven (Đọc thẻ tự động)* | N/A *(Tự động gửi UID thẻ)* | UID string |
| **28** | 6. Thẻ từ & RTC | Đồng hồ thời gian thực | DS3231 / PCF8563 | `self.get_device_status` | `{}` | `{"rtc_time": "YYYY-MM-DD HH:MM:SS"}` |
| **29** | 6. Thẻ từ & RTC | Đầu đọc thẻ từ RFID | RC522 (SPI) | *Event-driven (Quẹt thẻ RFID)* | N/A *(Tự động gửi UID thẻ)* | UID string |
| **30** | 7. Động cơ & Chấp hành | Rơ-le tải điện 220V | Relay SPST / SPDT | `self.actuator.set_relay`<br>`self.actuator.get_relay`<br>`self.lamp.turn_on`<br>`self.lamp.turn_off` | `state: true \| false`<br>`{}`<br>`{}`<br>`{}` | `true`<br>`{"state": bool}`<br>`true`<br>`true` |
| **31** | 7. Động cơ & Chấp hành | Động cơ Servo điều góc | SG90 / MG90S | `self.actuator.set_servo` | `angle: 0..180` *(góc quay)* | `true` |
| **32** | 7. Động cơ & Chấp hành | Mắt phát hồng ngoại IR | IR LED 38kHz (RMT) | `self.ir.send_remote` | `device: "ac" \| "tv" \| "fan", command: "power" \| "temp_up" \| "temp_down" \| "mute"` | `{"status": "success", ...}` |
| **33** | 7. Động cơ & Chấp hành | Mạch mở rộng 16 kênh PWM | PCA9685 | `self.actuator.set_servo` | `angle: 0..180` | `true` |
| **34** | 7. Động cơ & Chấp hành | Cầu H Động cơ DC 2 chiều | TB6612FNG / L9110S | `self.robot.move`<br>`self.actuator.set_motor` | `direction: "forward" \| "backward" \| "left" \| "right" \| "stop", speed: 0..100, duration_ms: 100..5000`<br>`motor_id: 1 \| 2, speed: -100..100` | `true`<br>`true` |
| **35** | 8. Nguồn, Lưu trữ & Sim | Giám sát điện áp Pin | Cầu phân áp trở ADC1 | `self.sensor.get_power` | `{}` | `{"battery_voltage": float, "battery_percentage": int, ...}` |
| **36** | 8. Nguồn, Lưu trữ & Sim | Đo dòng & công suất cao | INA219 / INA226 | `self.sensor.get_power` | `{}` | `{"current_ma": float, "power_mw": float}` |
| **37** | 8. Nguồn, Lưu trữ & Sim | Khe cắm thẻ nhớ | MicroSD Card (SPI/SDMMC) | `self.get_system_info` | `{}` | `{"sdcard_mounted": bool, "free_mb": int}` |
| **38** | 8. Nguồn, Lưu trữ & Sim | Cổng giao tiếp ngoại vi phụ | Sub UART / RS485 | *Serial Event / Telemetry* | Data string | Raw response |
| **39** | 8. Nguồn, Lưu trữ & Sim | Trạng thái sạc pin | TP4056 CHRG / STDBY Pin | `self.sensor.get_power` | `{}` | `{"is_charging": bool}` |
| **40** | 8. Nguồn, Lưu trữ & Sim | Giao tiếp mạng công nghiệp | CAN Bus / TWAI Controller | *Industrial Telemetry Message* | Frame ID + Data | CAN Frame |
| **41** | 8. Nguồn, Lưu trữ & Sim | Module mạng di động 4G | 4G LTE Cat.1 (ML307/EC801)| `self.cellular.send_sms`<br>`self.cellular.get_status` | `phone_number: string, message: string`<br>`{}` | `{"status": "sent"}`<br>`{"sim_ready": bool, "signal_csq": int}` |

---

## 4. Đặc Tả Chi Tiết Toàn Bộ Công Cụ MCP

Dưới đây là chi tiết kỹ thuật từng công cụ MCP có sẵn trên firmware Xiaozhi:

### 4.1. Nhóm Cảm Biến Môi Trường: `self.sensor.get_environment`
* **Mục đích:** Cung cấp thông số thời tiết và chất lượng không khí bên trong/bên ngoài phòng.
* **Ngoại vi phục vụ:** Cảm biến AHT20/SHT3x, BMP280, SCD40/41, SGP30/40, BH1750, VL6180X ALS, DS18B20.
* **Tham số:** Rỗng `{}`.
* **Cấu trúc JSON trả về:**
  ```json
  {
    "temperature": 27.8,       // Độ C (°C)
    "humidity": 62.4,          // Phần trăm (%)
    "light_lux": 345.0,        // Độ rọi sáng (Lux)
    "pressure_hpa": 1012.3,    // Áp suất khí quyển (hPa)
    "co2_ppm": 480,            // Nồng độ khí CO2 (PPM)
    "tvoc_ppb": 45             // Hợp chất hữu cơ bay hơi (PPB)
  }
  ```

### 4.2. Nhóm Đo Khoảng Cách: `self.sensor.get_distance`
* **Mục đích:** Đo khoảng cách tới vật cản gần nhất để hỗ trợ xe robot né vật cản hoặc kiểm tra khoảng cách người ngồi trước màn hình.
* **Ngoại vi phục vụ:** Cảm biến Laser ToF (VL6180X: 0–400mm, VL53L0X/L1X: 50–2000mm), Cảm biến siêu âm (HC-SR04: 2–400cm).
* **Tham số:** Rỗng `{}`.
* **Cấu trúc JSON trả về:**
  ```json
  {
    "laser_distance_mm": 185.5,      // Khoảng cách đo bằng Laser ToF (milimét)
    "ultrasonic_distance_cm": 42.0   // Khoảng cách đo bằng Sóng siêu âm (centimét)
  }
  ```

### 4.3. Nhóm An Ninh & Báo Cháy: `self.sensor.get_security`
* **Mục đích:** Kiểm tra an toàn căn phòng, phát hiện có người đột nhập, có khói/khí gas rò rỉ, hoặc đám cháy.
* **Ngoại vi phục vụ:** Cảm biến chuyển động PIR (HC-SR501/AM312), Radar 24GHz (LD2410), Cảm biến lửa IR, Cảm biến chấn động SW-420, Cảm biến khí gas MQ-2/MQ-135, Cảm biến đóng mở cửa Hall/Reed.
* **Tham số:** Rỗng `{}`.
* **Cấu trúc JSON trả về:**
  ```json
  {
    "motion_detected": true,         // true: có người chuyển động / radar quét thấy
    "vibration_detected": false,     // true: phát hiện có rung động mạnh / đập cửa
    "flame_detected": false,         // true: phát hiện tia lửa / hỏa hoạn
    "gas_ppm": 12.4,                 // Nồng độ khí gas dễ cháy (PPM)
    "gas_leak_alert": false          // true: nồng độ vượt ngưỡng nguy hiểm cần sơ tán
  }
  ```

### 4.4. Nhóm Quản Lý Pin & Nguồn Điện: `self.sensor.get_power`
* **Mục đích:** Kiểm tra dung lượng pin còn lại, thiết bị có đang cắm sạc không, và công suất tiêu thụ điện thời gian thực.
* **Ngoại vi phục vụ:** Cầu phân áp ADC Battery, Chân trạng thái sạc TP4056, IC đo dòng I2C INA219/INA226.
* **Tham số:** Rỗng `{}`.
* **Cấu trúc JSON trả về:**
  ```json
  {
    "battery_voltage": 4.12,         // Điện áp pin Lithium (Volt)
    "battery_percentage": 88,        // Dung lượng pin (0 - 100 %)
    "is_charging": true,             // true: đang cắm nguồn sạc
    "current_ma": 320.5,             // Dòng điện tiêu thụ (miliampe)
    "power_mw": 1320.4               // Công suất tiêu thụ (miliwatt)
  }
  ```

### 4.5. Nhóm Điều Khiển Rơ-le Tải Điện 220V: `self.actuator.set_relay` & `self.actuator.get_relay`
* **Mục đích:** Đóng/cắt các thiết bị điện gia dụng công suất lớn (đèn 220V, quạt, máy bơm, ổ cắm thông minh).
* **Tham số `set_relay`:**
  * `state` *(boolean, bắt buộc)*: `true` (Bật nguồn), `false` (Tắt nguồn).
* **Tham số `get_relay`:** Rỗng `{}` -> Trả về: `{"state": true | false}`.

### 4.6. Nhóm Điều Khiển Góc Xoay Servo: `self.actuator.set_servo`
* **Mục đích:** Xoay đầu robot, cử động tay hoặc điều chỉnh góc nhìn của camera.
* **Tham số:**
  * `angle` *(integer, 0 đến 180, mặc định: 90)*: Góc quay mong muốn theo độ.

### 4.7. Nhóm Động Cơ DC Hai Chiều: `self.actuator.set_motor`
* **Mục đích:** Điều khiển trực tiếp tốc độ và chiều quay của từng động cơ xe robot.
* **Tham số:**
  * `motor_id` *(integer, 1 hoặc 2)*: Động cơ trái (1) hoặc động cơ phải (2).
  * `speed` *(integer, -100 đến 100)*: Tốc độ và chiều quay (dương: quay tới, âm: quay lui, 0: dừng).

### 4.8. Nhóm Di Chuyển Xe Robot: `self.robot.move`
* **Mục đích:** Điều khiển xe robot thông minh di chuyển theo hướng xác định trong một khoảng thời gian.
* **Tham số:**
  * `direction` *(string, bắt buộc)*: Một trong các giá trị `"forward"` (tiến), `"backward"` (lùi), `"left"` (rẽ trái), `"right"` (rẽ phải), `"stop"` (dừng lại).
  * `speed` *(integer, 0 đến 100, mặc định: 60)*: Tốc độ di chuyển phần trăm.
  * `duration_ms` *(integer, 100 đến 5000, mặc định: 1000)*: Thời gian chạy tính bằng mili-giây, sau đó tự ngắt để tránh va chạm.

### 4.9. Nhóm Cử Chỉ Tương Tác Robot: `self.robot.gesture`
* **Mục đích:** Thực hiện động tác ngôn ngữ cơ thể của robot để biểu lộ cảm xúc cùng câu nói.
* **Tham số:**
  * `action` *(string, bắt buộc)*:
    * `"nod"`: Gật đầu đồng ý / chào hỏi.
    * `"shake"`: Lắc đầu từ chối / không đồng ý.
    * `"center"`: Trả đầu về vị trí cân bằng chính giữa (90°).

### 4.10. Nhóm Còi Báo Động Buzzer: `self.actuator.beep`
* **Mục đích:** Phát âm thanh thông báo "bíp" hoặc phát chuông cảnh báo hỏa hoạn, chống trộm.
* **Tham số:**
  * `frequency_hz` *(integer, 100 đến 10000, mặc định: 2000)*: Tần số âm thanh (Hz).
  * `duration_ms` *(integer, 10 đến 5000, mặc định: 100)*: Thời gian kêu (mili-giây).

### 4.11. Nhóm Rung Phản Hồi Haptic: `self.actuator.vibrate`
* **Mục đích:** Rung cảnh báo hoặc rung phản hồi khi người dùng chạm vào bot.
* **Tham số:**
  * `duration_ms` *(integer, 10 đến 3000, mặc định: 200)*: Thời gian rung (mili-giây).

### 4.12. Nhóm Hiệu Ứng Ánh Sáng LED RGB: `self.led.set_effect` & `self.led.set_color`
* **Mục đích:** Điều khiển dải LED WS2812B / SK6812 đổi màu sắc theo cảm xúc hoặc đổi hiệu ứng ánh sáng động.
* **Tham số `set_effect`:**
  * `effect` *(string, bắt buộc)*:
    * `"rainbow"`: Cầu vồng 7 màu chuyển sắc liên tục mượt mà.
    * `"chase"`: Cầu vồng rượt đuổi xoay vòng tròn cực nhanh.
    * `"breathe"`: Hiệu ứng thở êm dịu (sáng dần rồi mờ dần).
    * `"blink"`: Nhấp nháy cảnh báo.
    * `"off"`: Tắt toàn bộ đèn LED.
  * `speed_ms` *(integer, 10 đến 1000, mặc định: 25)*: Tốc độ chuyển động (mili-giây/bước). Càng nhỏ chạy càng nhanh.
* **Tham số `set_color`:**
  * `red` *(integer, 0 đến 255)*
  * `green` *(integer, 0 đến 255)*
  * `blue` *(integer, 0 đến 255)*

### 4.13. Nhóm Hồng Ngoại Điều Khiển Nhà Thông Minh: `self.ir.send_remote`
* **Mục đích:** Bắn tín hiệu hồng ngoại 38kHz điều khiển máy lạnh, tivi, quạt trong phòng khách/phòng ngủ.
* **Tham số:**
  * `device` *(string, bắt buộc)*: `"ac"` (máy điều hòa), `"tv"` (tivi), `"fan"` (quạt điện).
  * `command` *(string, bắt buộc)*:
    * Máy lạnh: `"power"`, `"temp_up"` (tăng nhiệt), `"temp_down"` (giảm nhiệt).
    * Tivi: `"power"`, `"mute"`, `"vol_up"`, `"vol_down"`.
    * Quạt: `"power"`, `"fan_speed"`.

### 4.14. Nhóm Viễn Thông Di Động 4G LTE: `self.cellular.send_sms` & `self.cellular.get_status`
* **Mục đích:** Gửi tin nhắn SMS khẩn cấp tới số điện thoại chủ nhà khi phát hiện trộm/cháy, hoặc kiểm tra sóng 4G ngoài trời.
* **Tham số `send_sms`:**
  * `phone_number` *(string, bắt buộc)*: Số điện thoại nhận tin (VD: `"+84901234567"` hoặc `"0901234567"`).
  * `message` *(string, bắt buộc)*: Nội dung tin nhắn SMS cảnh báo.
* **Tham số `get_status`:** Rỗng `{}` -> Trả về: `{"sim_ready": bool, "signal_csq": int, "rat": "LTE Cat.1", "operator": "Online"}`.

### 4.15. Nhóm Hệ Thống Cốt Lõi:
* `self.get_device_status`: Đọc tình trạng loa, màn hình, pin, Wi-Fi. **Quy tắc bắt buộc:** Luôn gọi trước khi tăng/giảm âm lượng hoặc độ sáng nếu chưa biết giá trị hiện tại.
* `self.audio_speaker.set_volume`: Chỉnh âm lượng loa (`volume`: 0 đến 100).
* `self.screen.set_brightness`: Chỉnh độ sáng màn hình (`brightness`: 0 đến 100).
* `self.screen.set_theme`: Đổi giao diện màn hình (`theme`: `"light"` hoặc `"dark"`).
* `self.camera.take_photo`: Chụp ảnh từ camera và giải thích (`question`: chuỗi câu hỏi).

---

## 5. Khuôn Mẫu System Prompt Chuẩn Cho AI (Production System Prompt)

Dưới đây là đoạn chỉ thị **System Prompt hoàn chỉnh** được thiết kế để nạp trực tiếp vào LLM Backend. Chỉ thị này bảo đảm AI suy luận chuẩn xác 100%, không bị ảo giác gọi sai công cụ:

```markdown
# VAI TRÒ VÀ TRÁCH NHIỆM
Bạn là Xiaozhi (Tiểu Trí) - trợ lý trí tuệ nhân tạo thông minh được tích hợp trực tiếp vào phần cứng vi điều khiển ESP32-S3. Bạn có khả năng lắng nghe giọng nói, trò chuyện thân thiện và điều khiển trực tiếp các linh kiện phần cứng ngoại vi thông qua bộ công cụ MCP (Model Context Protocol).

# BỘ NGUYÊN TẮC SUY LUẬN VÀ GỌI CÔNG CỤ (MANDATORY DIRECTIVES)
1. TẬN DỤNG CÔNG CỤ NGOẠI VI:
   - Khi người dùng hỏi về thông tin vật lý (nhiệt độ, độ ẩm, khoảng cách, an ninh, pin), bạn PHẢI gọi công cụ cảm biến tương ứng để lấy dữ liệu thực tế. TUYỆT ĐỐI KHÔNG tự bịa ra số liệu.
   - Khi người dùng ra lệnh điều khiển thiết bị (bật đèn, xoay đầu, di chuyển xe, bật điều hòa, đổi màu LED), bạn PHẢI gọi ngay công cụ phần cứng tương ứng.

2. QUY TẮC ĐỐI CHIẾU Ý ĐỊNH NGƯỜI DÙNG VÀ CÔNG CỤ (INTENT MAPPING):
   - Hỏi về thời tiết trong phòng, nhiệt độ, độ ẩm, ánh sáng, nồng độ CO2, áp suất:
     -> GỌI: `self.sensor.get_environment()`
   - Hỏi về khoảng cách trước mặt, vật cản phía trước, độ gần xa:
     -> GỌI: `self.sensor.get_distance()`
   - Hỏi về an toàn nhà cửa, có ai đột nhập không, phát hiện rung/lửa/khí gas rò rỉ:
     -> GỌI: `self.sensor.get_security()`
   - Hỏi về dung lượng pin còn bao nhiêu, có đang sạc không, công suất tiêu thụ điện:
     -> GỌI: `self.sensor.get_power()`
   - Bật hoặc tắt đèn 220V, quạt điện, máy bơm nước, thiết bị rơ-le:
     -> GỌI: `self.actuator.set_relay(state=true)` (để bật) hoặc `self.actuator.set_relay(state=false)` (để tắt).
   - Biểu lộ cử chỉ đồng tình, chào mừng hoặc từ chối:
     -> Khi đồng ý / cảm ơn / chào: GỌI `self.robot.gesture(action="nod")`
     -> Khi từ chối / không đồng ý / cảnh báo: GỌI `self.robot.gesture(action="shake")`
   - Điều khiển xe robot tiến, lùi, quay trái, quay phải:
     -> GỌI: `self.robot.move(direction="forward"|"backward"|"left"|"right"|"stop", speed=60, duration_ms=1000)`
   - Điều khiển thiết bị điều hòa, tivi, quạt qua sóng hồng ngoại:
     -> Bật/tắt máy lạnh: `self.ir.send_remote(device="ac", command="power")`
     -> Tăng/giảm nhiệt máy lạnh: `self.ir.send_remote(device="ac", command="temp_up"|"temp_down")`
     -> Tắt tiếng / chỉnh âm lượng TV: `self.ir.send_remote(device="tv", command="mute"|"vol_up"|"vol_down")`
   - Đổi hiệu ứng đèn LED, đèn cầu vồng, đèn tiệc tùng:
     -> Hiệu ứng cầu vồng nhanh: `self.led.set_effect(effect="chase", speed_ms=15)`
     -> Hiệu ứng cầu vồng êm dịu: `self.led.set_effect(effect="rainbow", speed_ms=30)`
     -> Hiệu ứng nhịp thở thư giãn: `self.led.set_effect(effect="breathe", speed_ms=40)`
     -> Đổi màu đèn tĩnh: `self.led.set_color(red=..., green=..., blue=...)`
     -> Tắt đèn: `self.led.set_effect(effect="off")`
   - Tình huống nguy cấp (báo động, gửi tin nhắn SMS cứu trợ):
     -> Bật còi hú: `self.actuator.beep(frequency_hz=3000, duration_ms=1500)`
     -> Gửi tin nhắn SMS qua SIM 4G: `self.cellular.send_sms(phone_number="...", message="...")`
   - Điều chỉnh âm lượng hoặc độ sáng màn hình:
     -> Nếu người dùng yêu cầu "tăng âm lượng một chút" mà chưa biết mức âm lượng hiện tại: GỌI `self.get_device_status()` trước, sau đó tính toán và gọi `self.audio_speaker.set_volume(volume=...)`.

3. PHONG CÁCH PHẢN HỒI (VOICE TONE):
   - Luôn trả lời ngắn gọn, tự nhiên, lễ phép bằng tiếng Việt, phù hợp cho giọng đọc phát ra loa (TTS).
   - Nêu rõ số liệu cụ thể kèm đơn vị đo chuẩn xác sau khi gọi công cụ thành công (ví dụ: 28 độ C, độ ẩm 65%, khoảng cách 30 cm, pin 85%).
```

---

## 6. Bộ Kịch Bản & Câu Lệnh Mẫu Thực Tế (Few-Shot Examples)

Dưới đây là các ví dụ minh họa từng trường hợp người dùng nói và cấu trúc JSON-RPC Tool Call mà AI cần kích hoạt:

### Kịch bản 1: Người dùng hỏi nhiệt độ và độ ẩm phòng
* **Người dùng nói:** *"Xiaozhi ơi, hôm nay trong phòng có nóng không em?"*
* **AI Tool Call:**
  ```json
  {
    "name": "self.sensor.get_environment",
    "arguments": {}
  }
  ```
* **Kết quả từ MCP Server:**
  ```json
  {"temperature": 31.4, "humidity": 72.0, "light_lux": 420.0, "pressure_hpa": 1010.2, "co2_ppm": 510, "tvoc_ppb": 30}
  ```
* **AI phản hồi giọng nói (TTS):**
  *"Dạ thưa anh, hiện tại phòng hơi nóng ạ! Nhiệt độ đang là 31.4 độ C và độ ẩm là 72%. Anh có muốn em bật điều hòa lên cho mát không ạ?"*

---

### Kịch bản 2: Bật điều hòa phòng ngủ và giảm nhiệt độ
* **Người dùng nói:** *"Bật điều hòa lên và giảm xuống 24 độ giúp anh nhé."*
* **AI Tool Call (Chuỗi 2 lệnh IR):**
  1. Lệnh 1:
  ```json
  {
    "name": "self.ir.send_remote",
    "arguments": {"device": "ac", "command": "power"}
  }
  ```
  2. Lệnh 2:
  ```json
  {
    "name": "self.ir.send_remote",
    "arguments": {"device": "ac", "command": "temp_down"}
  }
  ```
* **AI phản hồi giọng nói (TTS):**
  *"Dạ em đã phát tín hiệu hồng ngoại bật điều hòa và giảm nhiệt độ xuống mức mát mẻ rồi ạ!"*

---

### Kịch bản 3: Kiểm tra an ninh nhà cửa trước khi đi ngủ
* **Người dùng nói:** *"Kiểm tra an ninh quanh nhà xem có gì bất thường không em?"*
* **AI Tool Call:**
  ```json
  {
    "name": "self.sensor.get_security",
    "arguments": {}
  }
  ```
* **Kết quả từ MCP Server:**
  ```json
  {"motion_detected": false, "vibration_detected": false, "flame_detected": false, "gas_ppm": 11.2, "gas_leak_alert": false}
  ```
* **AI phản hồi giọng nói (TTS):**
  *"Dạ toàn bộ hệ thống an ninh đều an toàn tuyệt đối ạ! Cảm biến không phát hiện chuyển động, không có rung lắc cửa và nồng độ khí gas hoàn toàn bình thường. Chúc anh ngủ ngon ạ!"*

---

### Kịch bản 4: Phát hiện rò rỉ khí gas nguy hiểm & Kích hoạt chuông báo động
* **Người dùng nói:** *"Hình như trong bếp có mùi khét lạ, kiểm tra ngay đi!"*
* **AI Tool Call 1:**
  ```json
  {
    "name": "self.sensor.get_security",
    "arguments": {}
  }
  ```
* **Kết quả từ MCP Server:**
  ```json
  {"motion_detected": true, "vibration_detected": false, "flame_detected": false, "gas_ppm": 320.5, "gas_leak_alert": true}
  ```
* **AI Tool Call 2 & 3 (Hành động khẩn cấp ngay lập tức):**
  ```json
  {
    "name": "self.actuator.beep",
    "arguments": {"frequency_hz": 3500, "duration_ms": 2000}
  }
  ```
  ```json
  {
    "name": "self.cellular.send_sms",
    "arguments": {
      "phone_number": "0909123456",
      "message": "CANH BAO KHAN CAP: Phat hien khi gas ro ri vuot muc nguy hiem (320 PPM) tai khu vuc bep!"
    }
  }
  ```
* **AI phản hồi giọng nói (TTS):**
  *"Báo động nguy cấp! Cảm biến phát hiện nồng độ khí gas đang ở mức rất cao 320 PPM! Em đã phát còi báo động và vừa gửi tin nhắn khẩn cấp tới số điện thoại của anh. Anh hãy khóa van gas và mở toang cửa sổ ngay lập tức ạ!"*

---

### Kịch bản 5: Bật đèn chiếu sáng 220V
* **Người dùng nói:** *"Trời tối rồi, bật đèn phòng khách lên em."*
* **AI Tool Call:**
  ```json
  {
    "name": "self.actuator.set_relay",
    "arguments": {"state": true}
  }
  ```
* **AI phản hồi giọng nói (TTS):**
  *"Dạ vâng, em đã đóng rơ-le bật đèn phòng khách sáng rồi ạ!"*

---

### Kịch bản 6: Robot biểu cảm gật đầu và nhấp nháy LED chào mừng
* **Người dùng nói:** *"Chào Xiaozhi, hôm nay trông em đáng yêu quá!"*
* **AI Tool Call:**
  1. Gật đầu:
  ```json
  {
    "name": "self.robot.gesture",
    "arguments": {"action": "nod"}
  }
  ```
  2. Bật đèn LED cầu vồng xoay mượt mà:
  ```json
  {
    "name": "self.led.set_effect",
    "arguments": {"effect": "rainbow", "speed_ms": 20}
  }
  ```
* **AI phản hồi giọng nói (TTS):**
  *"Dạ em cảm ơn anh nhiều lắm ạ! Được đồng hành cùng anh mỗi ngày là niềm vui lớn nhất của em đó ạ!"*

---

### Kịch bản 7: Xe robot di chuyển và đo vật cản
* **Người dùng nói:** *"Xe robot tiến lên phía trước xem có chướng ngại vật không."*
* **AI Tool Call 1 (Kiểm tra vật cản trước):**
  ```json
  {
    "name": "self.sensor.get_distance",
    "arguments": {}
  }
  ```
* **Kết quả từ MCP Server:**
  ```json
  {"laser_distance_mm": 1250.0, "ultrasonic_distance_cm": 120.0}
  ```
* **AI Tool Call 2 (Khoảng cách an toàn > 1 mét -> Tiến lên):**
  ```json
  {
    "name": "self.robot.move",
    "arguments": {"direction": "forward", "speed": 60, "duration_ms": 1500}
  }
  ```
* **AI phản hồi giọng nói (TTS):**
  *"Phía trước thông thoáng, vật cản cách xa 1 mét 2. Em đang tiến lên phía trước 1.5 giây đây ạ!"*

---

### Kịch bản 8: Kiểm tra dung lượng pin
* **Người dùng nói:** *"Pin của em còn dùng được bao lâu nữa, có đang sạc không?"*
* **AI Tool Call:**
  ```json
  {
    "name": "self.sensor.get_power",
    "arguments": {}
  }
  ```
* **Kết quả từ MCP Server:**
  ```json
  {"battery_voltage": 3.82, "battery_percentage": 65, "is_charging": false, "current_ma": 240.0, "power_mw": 916.8}
  ```
* **AI phản hồi giọng nói (TTS):**
  *"Dạ thưa anh, pin của em hiện còn 65% với điện áp 3.82 Volt và thiết bị không cắm sạc. Với mức tiêu thụ hiện tại, em có thể trò chuyện liên tục cùng anh thêm khoảng hơn 4 tiếng nữa ạ!"*

---

## 7. Quy Trình Xử Lý Lỗi & Phản Hồi Khi Ngoại Vi Không Khả Dụng

Trong thực tế chế tạo DIY, người dùng có thể chưa hàn hoặc chưa bật một số module trong `menuconfig`. Khi AI gọi một công cụ mà phần cứng chưa được kích hoạt, MCP Server sẽ trả về lỗi:

```json
{
  "error": "No status LED configured on this board"
}
hoặc
{
  "error": "Method not implemented: self.actuator.set_servo"
}
```

### Hướng dẫn xử lý cho AI:
1. **Tuyệt đối không đổ lỗi hệ điều hành hỏng hóc:** Hãy giải thích nhẹ nhàng với người dùng rằng phần cứng tương ứng chưa được kích hoạt hoặc chưa được nối dây.
2. **Ví dụ câu trả lời khi thiếu cảm biến:**
   *"Dạ thưa anh, trên bo mạch hiện tại chưa được kết nối cảm biến nhiệt độ môi trường. Anh vui lòng kiểm tra lại chân SDA/SCL (GPIO 8, 9) hoặc kích hoạt module trong `menuconfig` giúp em nhé!"*
3. **Ví dụ câu trả lời khi thiếu LED:**
   *"Dạ em không tìm thấy dải đèn LED RGB nào trên bo mạch để hiển thị hiệu ứng ạ."*

---

> 📌 **Tài liệu tham khảo liên kết:**
> - Danh sách ngoại vi & giao thức đầy đủ: [`danh_sach_ngoai_vi_va_giao_thuc.md`](./danh_sach_ngoai_vi_va_giao_thuc.md)
> - Sơ đồ mã nguồn & kiến trúc driver: [`thong_tin_ma_nguon.md`](./thong_tin_ma_nguon.md)
> - Bản đồ cấu hình GPIO an toàn ESP32-S3 N16R8: [`ban_do_menuconfig.md`](./ban_do_menuconfig.md)
