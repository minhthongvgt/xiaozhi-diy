# HƯỚNG DẪN GIAO THỨC NEXTION / TJC HMI CHO XIAOZHI

Tài liệu này quy định chi tiết quy tắc đặt tên biến (Component Object Name), các lệnh điều khiển, và cách thiết kế giao diện trong **Nextion Editor** hoặc **TJC USART HMI**.

---

## 1. Nguyên Tắc Truyền Thông Nextion / TJC
- **Tốc độ mặc định**: `115200` bps (8 data bits, no parity, 1 stop bit).
- **Đuôi kết thúc lệnh (End of Command)**: Bắt buộc là 3 byte hex: `0xFF 0xFF 0xFF`.
- **Định dạng lệnh gán text**:
  ```text
  <component_name>.txt="<nội_dung>"<0xFF><0xFF><0xFF>
  ```
- **Định dạng lệnh gán số/hình ảnh**:
  ```text
  <component_name>.val=<giá_trị><0xFF><0xFF><0xFF>
  <component_name>.pic=<id_hình_ảnh><0xFF><0xFF><0xFF>
  ```

---

## 2. Bảng Quy Chuẩn Tên Thành Phần Giao Diện (Standard Component IDs)

Khi tạo màn hình trong Nextion/TJC Editor, AI Agent hoặc người thiết kế **bắt buộc phải đặt tên (objname)** cho các widget theo bảng chuẩn sau:

| Tên Widget (objname) | Loại Widget | Chức Năng Hiển Thị | Ví Dụ Lệnh Từ ESP32-S3 |
|---|---|---|---|
| `t_status` | Text | Trạng thái hệ thống Xiaozhi (`idle`, `listening`, `thinking`, `speaking`) | `t_status.txt="listening"` |
| `t_emotion` | Text / Variable | Mã biểu cảm khuôn mặt (`neutral`, `happy`, `sad`, `thinking`, v.v.) | `t_emotion.txt="happy"` |
| `p_avatar` | Picture / Crop | Hình ảnh khuôn mặt biểu cảm tương ứng | `p_avatar.pic=2` |
| `t_user` | Text / Scrolling | Đoạn văn bản người dùng nói | `t_user.txt="Thời tiết hôm nay thế nào?"` |
| `t_chat` | Text / Scrolling | Câu trả lời của Xiaozhi (Assistant Content) | `t_chat.txt="Hôm nay tại TP.HCM trời nắng ráo..."` |
| `t_notify` | Text | Thông báo nổi (Toast Notification) | `t_notify.txt="WiFi đã kết nối thành công"` |
| `t_time` | Text | Đồng hồ thời gian thực (HH:mm:ss) | `t_time.txt="14:35:20"` |
| `t_date` | Text | Lịch ngày tháng năm (DD/MM/YYYY) | `t_date.txt="17/09/2026"` |
| `t_weather` | Text | Thông tin thời tiết tóm tắt | `t_weather.txt="TP.HCM 32°C Nắng"` |
| `p_weather` | Picture | Icon thời tiết (Nắng, Mưa, Mây, Dông) | `p_weather.pic=5` |
| `j_wifi` | Progress bar / Icon | Mức tín hiệu WiFi (0 - 100%) | `j_wifi.val=85` |
| `j_bat` | Progress bar / Icon | Mức pin sạc còn lại (0 - 100%) | `j_bat.val=90` |

---

## 3. Quản Lý Trang & Chế Độ Tiết Kiệm Năng Lượng

1. **Chuyển trang (Page Switching)**:
   - Trang chính trò chuyện: `page main`
   - Trang hiển thị đồng hồ toàn màn hình: `page clock`
   - Trang cài đặt mạng WiFi: `page settings`

2. **Chế độ ngủ / Tiết kiệm điện (Power Save)**:
   - Bật chế độ ngủ màn hình: `sleep=1`
   - Đánh thức màn hình khi phát hiện âm thanh/từ khóa đánh thức: `sleep=0`
   - Chỉnh độ sáng đèn nền: `dim=80` (80% độ sáng) hoặc `dim=20` (20% độ sáng ban đêm).

---

## 4. Giao Tiếp Ngược: Cảm Ứng Màn Hình Gửi Về ESP32-S3

Khi người dùng nhấn vào nút trên màn hình Nextion, màn hình có thể gửi sự kiện UART về cho ESP32-S3 qua chân `RX`:

- **Định dạng chuỗi sự kiện đề xuất**:
  ```text
  EVENT:<action>\r\n
  ```
- **Các sự kiện chuẩn**:
  - `EVENT:WAKE_UP`: Nhấn vào màn hình để kích hoạt Xiaozhi lắng nghe (tương đương nút Touch).
  - `EVENT:VOL_UP`: Tăng âm lượng.
  - `EVENT:VOL_DOWN`: Giảm âm lượng.
  - `EVENT:MUTE`: Tắt/bật tiếng.
  - `EVENT:RESTART`: Khởi động lại thiết bị.
