# HƯỚNG DẪN KIẾN TRÚC MÀN HÌNH RỜI QUA UART CHO XIAOZHI (D-UART)

Tài liệu này dành cho **AI Agent** và **Developer** để nắm bắt toàn bộ luồng hoạt động, cấu trúc biến, giao thức truyền thông và cách thiết kế giao diện (UI) trên các dòng màn hình HMI rời kết nối qua UART (Nextion, TJC, DWIN DGUS, Màn hình vi điều khiển phụ, Android/Linux HMI qua Serial).

---

## 1. Tổng Quan Kiến Trúc (Architecture Overview)

```
+-------------------------------------------------------------+
|               ESP32-S3 (Xiaozhi Firmware)                  |
|                                                             |
|  [Voice Core / AI Cloud] <---> [Application / StateMachine] |
|                                             |               |
|                                     [UartDisplay Class]     |
|                                             |               |
|                                     [Hardware UART1/2]      |
|                                       TX=GPIO17, RX=GPIO18  |
+-------------------------------------------------------------+
                                       |
                     UART Serial Bus (115200 bps, 8N1)
                                       v
+-------------------------------------------------------------+
|                MÀN HÌNH RỜI (EXTERNAL HMI DISPLAY)          |
|                                                             |
|   Các loại màn hình hỗ trợ:                                 |
|   1. Nextion / TJC HMI Display (Trình biên dịch Nextion Editor)|
|   2. DWIN DGUS Smart Screen (DGUS Tool)                     |
|   3. Arduino / ESP32 / STM32 LCD Controller (JSON / Raw)    |
|   4. Raspberry Pi / Mini PC Dashboard                       |
+-------------------------------------------------------------+
```

### Tại sao dùng Màn hình rời qua UART?
1. **Tiết kiệm chân GPIO**: Chỉ cần đúng 2 chân `TX` và `RX` (thay vì 6-8 chân của SPI/QSPI).
2. **Giải phóng RAM & CPU cho ESP32-S3**: Toàn bộ việc render đồ họa, font chữ tiếng Việt, hiệu ứng chuyển cảnh, nút bấm cảm ứng đều do chip xử lý trên màn hình rời đảm nhiệm. ESP32-S3 chỉ gửi gói tin nhẹ chứa trạng thái và nội dung.
3. **Màn hình lớn & Đa dạng kích thước**: Dễ dàng sử dụng màn hình 3.5", 4.3", 7.0", 10.1" có vỏ công nghiệp hoàn chỉnh.

---

## 2. Các Thành Phần Trạng Thái & Dữ Liệu Cần Hiển Thị Trên UI

Một giao diện hoàn chỉnh cho Xiaozhi trên màn hình rời cần hiển thị các khối dữ liệu sau:

### 2.1. Trạng Thái Hoạt Động (Xiaozhi States)
ESP32-S3 liên tục cập nhật trạng thái thông qua hàm `SetStatus(status)`:
- `idle`: Chế độ chờ, sẵn sàng nhận từ khóa đánh thức ("Hi Xiaozhi", "Nihao Xiaozhi").
- `listening`: Đang lắng nghe giọng nói người dùng (Micro đang ghi âm).
- `thinking`: Đang gửi âm thanh lên AI Cloud và chờ LLM xử lý.
- `speaking`: Đang phát âm thanh câu trả lời từ loa.
- `connected`: Đã kết nối WiFi và WebSocket/MQTT Server thành công.
- `disconnected`: Mất kết nối WiFi hoặc máy chủ.

### 2.2. Biểu Cảm Khuôn Mặt (Emotions / Avatars)
Được cập nhật qua `SetEmotion(emotion)`:
- `neutral`: Bình thường, mỉm cười nhẹ.
- `happy`: Vui vẻ, hào hứng.
- `listening`: Mắt chú ý lắng nghe.
- `thinking`: Mắt chớp, biểu tượng suy nghĩ.
- `speaking`: Mắt mở, miệng mấp máy.
- `sad`: Buồn, tiếc nuối khi gặp lỗi.
- `angry`: Bực bội, ngạc nhiên.
- `sleepy`: Buồn ngủ, nhắm mắt khi ở chế độ Power Save.

### 2.3. Hộp Thoại Trò Chuyện (Chat Dialogue)
Được cập nhật qua `SetChatMessage(role, content)`:
- `role = "user"`: Câu nói người dùng vừa hỏi (hiển thị khung User Bubble).
- `role = "assistant"`: Câu trả lời của Xiaozhi (hiển thị khung AI Bubble dạng phụ đề hoặc văn bản cuộn).
- Lệnh xóa đoạn chat: `ClearChatMessages()`.

### 2.4. Thông Báo Nổi (Toast / Popup Notification)
Được cập nhật qua `ShowNotification(notification, duration_ms)`:
- Hiển thị thông báo trạng thái tạm thời (Ví dụ: *"Đã cập nhật OTA 1.2.0"*, *"Pin yếu 15%"*, *"WiFi đã kết nối"*).

### 2.5. Thời Gian, Lịch & Thời Tiết (Real-time Clock & Weather)
- **Giờ & Ngày**: Cập nhật từ SNTP trên ESP32-S3 (HH:mm:ss, DD/MM/YYYY, Thứ trong tuần).
- **Thời tiết**: Nhiệt độ, độ ẩm, biểu tượng thời tiết (Nắng, Mưa, Mây...).

---

## 3. Danh Sách Các Tài Liệu Chi Tiết Trong Thư Mục `D-UART/`

AI Agent cần tham khảo các file tương ứng khi thiết kế hoặc sinh mã:

1. **[D-UART/README.md](file:///d:/Code/Antigravity/Xiaozhi/config/D-UART/README.md)**: Tài liệu tổng quan (file này).
2. **[D-UART/nextion_tjc_protocol.md](file:///d:/Code/Antigravity/Xiaozhi/config/D-UART/nextion_tjc_protocol.md)**: Chi tiết giao thức Nextion/TJC HMI, quy tắc đặt tên Component ID, lệnh gửi/nhận.
3. **[D-UART/json_protocol.md](file:///d:/Code/Antigravity/Xiaozhi/config/D-UART/json_protocol.md)**: Chuẩn định dạng JSON streaming dành cho bộ vi điều khiển phụ hoặc màn hình thông minh.
4. **[D-UART/dwin_dgus_protocol.md](file:///d:/Code/Antigravity/Xiaozhi/config/D-UART/dwin_dgus_protocol.md)**: Bản đồ địa chỉ VP (Variable Pointer) và khung byte nhị phân cho màn hình DWIN DGUS.
5. **[D-UART/ui_layout_specification.md](file:///d:/Code/Antigravity/Xiaozhi/config/D-UART/ui_layout_specification.md)**: Hướng dẫn bố cục UI, phân vùng màn hình (Status Bar, Avatar Face, Weather Card, Chat Area, Controls).
