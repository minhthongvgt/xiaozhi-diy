# HƯỚNG DẪN GIAO THỨC JSON STREAMING CHO MÀN HÌNH RỜI

Giao thức JSON Streaming (`CONFIG_CUSTOM_DISPLAY_UART_PROTO_JSON`) được thiết kế cho các trường hợp:
1. Màn hình rời sử dụng một vi điều khiển độc lập khác (ESP32-S3 phụ, STM32, RP2040, Arduino) chạy thư viện đồ họa LVGL hoặc TFT_eSPI.
2. Màn hình điều khiển thông minh chạy hệ điều hành Linux (Raspberry Pi), Android, hoặc Web Dashboard qua Serial.

---

## 1. Định Dạng Gói Tin (Line-Delimited JSON)
- **Tốc độ**: `115200` bps (hoặc cấu hình lên tới `921600` bps để truyền tải nhanh).
- **Phân cách gói tin**: Mỗi gói JSON là một dòng đơn kết thúc bằng ký tự xuống dòng `\n` (LF - ASCII 0x0A).
- **Cấu trúc JSON cơ bản**:
  ```json
  {"type": "<loại_sự_kiện>", ... <các trường dữ liệu>}\n
  ```

---

## 2. Chi Tiết Các Gói Tin ESP32-S3 Gửi Cho Màn Hình

### 2.1. Cập Nhật Trạng Thái Hệ Thống (`status`)
Được gửi mỗi khi trạng thái Xiaozhi thay đổi:
```json
{"type":"status","val":"listening"}
```
Các giá trị `val`:
- `"idle"`: Chế độ chờ.
- `"listening"`: Đang lắng nghe giọng nói.
- `"thinking"`: Đang suy nghĩ / xử lý trên AI Cloud.
- `"speaking"`: Đang phát âm thanh câu trả lời.
- `"connected"`: Đã kết nối mạng thành công.
- `"disconnected"`: Mất kết nối mạng.

### 2.2. Cập Nhật Biểu Cảm Khuôn Mặt (`emotion`)
```json
{"type":"emotion","val":"happy"}
```
Các giá trị `val`: `"neutral"`, `"happy"`, `"sad"`, `"angry"`, `"surprised"`, `"thinking"`, `"sleepy"`.

### 2.3. Cập Nhật Hộp Thoại Trò Chuyện (`chat`)
- Khi người dùng nói xong:
  ```json
  {"type":"chat","role":"user","content":"Bật đèn phòng khách giúp tôi"}
  ```
- Khi Xiaozhi trả lời:
  ```json
  {"type":"chat","role":"assistant","content":"Dạ, em đã bật đèn phòng khách rồi ạ."}
  ```

### 2.4. Xóa Hộp Thoại (`clear_chat`)
Được gửi khi bắt đầu một phiên hội thoại mới hoặc khi hết thời gian chờ:
```json
{"type":"clear_chat"}
```

### 2.5. Thông Báo Nổi (`notify`)
```json
{"type":"notify","val":"Đã kết nối vào WiFi 'NhaThongMinh'"}
```

### 2.6. Đồng Hồ, Lịch & Thời Tiết Thời Gian Thực (`env_clock`)
Gói tin đồng bộ thời gian và thời tiết (gửi định kỳ mỗi giây hoặc mỗi phút):
```json
{
  "type": "clock",
  "time": "14:35:00",
  "date": "17/09/2026",
  "weekday": "Thursday",
  "weather": {
    "city": "Thành phố Hồ Chí Minh",
    "temp": 32,
    "humidity": 75,
    "condition": "sunny"
  }
}
```

### 2.7. Trạng Thái Nguồn & Pin (`power`)
```json
{"type":"power","state":"sleep"}
{"type":"power","state":"wake"}
```

---

## 3. Gói Tin Màn Hình Gửi Ngược Lại Cho ESP32-S3 (Touch Event / User Input)

Khi người dùng thao tác trên màn hình rời (chạm nút bấm, gạt thanh trượt âm lượng):
```json
{"event":"wake_up"}
{"event":"volume","val":80}
{"event":"button","id":"mic_toggle"}
```
Phần mềm vi điều khiển chính chỉ cần parse chuỗi JSON nhận được ở hàm đọc UART để kích hoạt sự kiện tương ứng.
