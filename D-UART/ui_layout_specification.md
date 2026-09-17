# QUY CHUẨN THIẾT KẾ BỐ CỤC GIAO DIỆN MÀN HÌNH RỜI (UI SPECIFICATION)

Tài liệu này hướng dẫn **AI Agent** và **Designer** cách tổ chức không gian đồ họa trên màn hình rời (Nextion, DWIN, hoặc vi điều khiển phụ), đảm bảo giao diện đẹp mắt, trực quan và thể hiện trọn vẹn sức mạnh của Xiaozhi AI Assistant.

---

## 1. Sơ Đồ Bố Cục Chuẩn (Wireframe Layout)

Dành cho màn hình hướng ngang (Landscape 480x320, 800x480, 1024x600) hoặc hướng dọc (Portrait 240x320, 320x480, 480x800):

```
+------------------------------------------------------------------------+
| [Thanh Trạng Thái Trên - Top Status Bar]               (Chiều cao: 10%)|
|  - WiFi Icon       - Trạng Thái (idle/listening/speaking)   - Pin / Giờ|
+------------------------------------------------------------------------+
|                                  |                                     |
|  [VÙNG BÊN TRÁI: AI AVATAR &     |  [VÙNG BÊN PHẢI: KHUNG HỘI THOẠI    |
|   THÔNG TIN THỜI GIAN / THỜI TIẾT] |   & PHỤ ĐỀ TRÒ CHUYỆN]              |
|                                  |                                     |
|  1. Robot Avatar / Khuôn mặt     |  1. Khung câu hỏi Người dùng        |
|     (Thay đổi theo biểu cảm      |     (User Bubble)                   |
|      neutral, happy, thinking...) |                                     |
|                                  |  2. Khung câu trả lời Xiaozhi       |
|  2. Thẻ Thời Gian Thực & Lịch    |     (Assistant Bubble - hỗ trợ     |
|     (Đồng hồ số to, ngày tháng)  |      cuộn văn bản tiếng Việt dài)   |
|                                  |                                     |
|  3. Thẻ Thời Tiết                |                                     |
|     (Thành phố Hồ Chí Minh, 32°C,|                                     |
|      Icon Nắng/Mưa)              |                                     |
|                                  |                                     |
+------------------------------------------------------------------------+
| [Thanh Điều Khiển Dưới - Bottom Bar / Notifications]   (Chiều cao: 12%)|
|  - Nút Chạm Đánh Thức (Mic)    - Nút Tăng/Giảm Âm Lượng   - Thông Báo  |
+------------------------------------------------------------------------+
```

---

## 2. Chi Tiết Từng Phân Vùng

### 2.1. Top Status Bar (Thanh Trạng Thái Hệ Thống)
- **Vị trí**: Nằm sát mép trên màn hình.
- **Thành phần**:
  - Biểu tượng WiFi: Hiển thị 0%, 33%, 66%, 100% theo cường độ sóng.
  - Nhãn trạng thái (Status Label):
    - Đổi màu động theo trạng thái:
      - `idle`: Xám mờ hoặc xanh lục nhẹ.
      - `listening`: Xanh lam phát sáng (biểu thị mic đang bật).
      - `thinking`: Vàng hổ phách / hiệu ứng xoay (AI đang xử lý).
      - `speaking`: Tím / Xanh dạ quang (loa đang phát).
  - Biểu tượng Pin sạc (nếu có mạch sạc TP4056/AXP2101).

### 2.2. Khuôn Mặt Robot / Biểu Cảm (Avatar Area)
- **Kích thước**: Chiếm khoảng 30% - 40% diện tích bên trái hoặc chính giữa phía trên.
- **Tài nguyên ảnh**:
  - Nên chuẩn bị 6-8 hình ảnh khuôn mặt mắt led (mắt tròn, chớp mắt, cười, tò mò, nháy mắt, buồn ngủ).
  - Khi nhận sự kiện `t_emotion.txt="happy"` hoặc `{"type":"emotion","val":"happy"}`, màn hình lập tức đổi sang ID hình ảnh tương ứng.

### 2.3. Khối Thời Gian & Thời Tiết (Clock & Weather Card)
- **Thời gian**: Font chữ to, đậm, hiển thị rõ ràng giờ : phút : giây.
- **Lịch**: Ngày trong tháng, tháng, năm, và thứ trong tuần (Ví dụ: `Thứ Năm, 17/09/2026`).
- **Thời tiết TP.HCM**:
  - Địa điểm: `TP. Hồ Chí Minh`.
  - Nhiệt độ: Font số rõ nét kèm ký hiệu `°C`.
  - Icon thời tiết: Mặt trời (Sunny), Mây (Cloudy), Mưa (Rainy), Dông (Thunderstorm).

### 2.4. Khung Hội Thoại (Chat Dialogue Bubble)
- **Màu sắc nền**:
  - Bong bóng câu hỏi người dùng: Màu xám đậm / xanh đậm, căn lề phải.
  - Bong bóng câu trả lời AI: Màu gradient hiện đại hoặc nền tối với viền neon, căn lề trái.
- **Font chữ**: Chọn font Unicode hỗ trợ đầy đủ dấu tiếng Việt (Arial, Roboto, Montserrat hoặc font Việt hóa trong Nextion Editor).
- **Tính năng tự động cuộn**: Khi câu trả lời của AI dài, vùng hiển thị cần hỗ trợ tự động cuộn dòng (Auto-wrap text / multi-line scroll).

---

## 3. Checklist Cho Agent Khi Thiết Kế Hoặc Tạo Dự Án Màn Hình Rời

Khi người dùng yêu cầu tạo giao diện cho màn hình rời:
- [x] Xác định chuẩn màn hình: Nextion, DWIN hay bo mạch phụ.
- [x] Đặt tên các biến và widget đúng theo bảng quy chuẩn tại [nextion_tjc_protocol.md](file:///d:/Code/Antigravity/Xiaozhi/config/D-UART/nextion_tjc_protocol.md) hoặc [dwin_dgus_protocol.md](file:///d:/Code/Antigravity/Xiaozhi/config/D-UART/dwin_dgus_protocol.md).
- [x] Đảm bảo cấu hình tốc độ Baudrate giữa firmware ESP32-S3 và màn hình rời trùng khớp (khuyên dùng `115200`).
- [x] Kiểm tra an toàn GPIO: chân `TX` và `RX` nối vào màn hình không được rơi vào dải chân cấm `26-37` của chip ESP32-S3 N16R8.
