# HƯỚNG DẪN GIAO THỨC DWIN DGUS SMART SCREEN CHO XIAOZHI

Tài liệu này quy định bản đồ vùng nhớ biến (Variable Pointer - VP Address) và cấu trúc khung truyền nhị phân cho các dòng màn hình công nghiệp **DWIN DGUS II / DGUS T5L** kết nối qua UART.

---

## 1. Cấu Trúc Khung Byte DWIN DGUS (Frame Format)

Tất cả các gói tin gửi đến màn hình DWIN đều tuân thủ định dạng chuẩn DGUS:

| Header (2B) | Length (1B) | Command (1B) | VP Address (2B) | Data Payload (N Bytes) |
|---|---|---|---|---|
| `0x5A 0xA5` | Số byte tiếp theo | `0x82` (Ghi biến) | `VP_High VP_Low` | Dữ liệu văn bản / số / trạng thái |

- **Header cố định**: `0x5A 0xA5`.
- **Lệnh ghi RAM**: `0x82` (Write VP).
- **Chuỗi ký tự kết thúc**: DWIN yêu cầu chuỗi kết thúc bằng 2 byte null `0x00 0x00`.

---

## 2. Bản Đồ Địa Chỉ Biến (VP Address Map)

Khi thiết kế giao diện trong phần mềm **DGUS Tool (DGUS Software)**, hãy gán các Widget tương ứng với các địa chỉ VP sau:

| Địa chỉ VP (Hex) | Kiểu Dữ Liệu | Chiều Dài (Word) | Tên Thành Phần & Chức Năng |
|---|---|---|---|
| `0x1000` | Text (ASCII / GBK) | 16 words (32 bytes) | **Trạng thái hệ thống Xiaozhi** (`idle`, `listening`, `thinking`, `speaking`) |
| `0x1100` | Text | 32 words (64 bytes) | **Thông báo nổi** (Notification / Toast) |
| `0x1200` | Text / Integer | 16 words (32 bytes) | **Biểu cảm khuôn mặt** (Emotion Name / Emotion Icon ID) |
| `0x1300` | Text | 64 words (128 bytes) | **Tin nhắn của Người dùng** (User Question) |
| `0x1400` | Text | 128 words (256 bytes) | **Câu trả lời của Xiaozhi** (Assistant Chat Content) |
| `0x1500` | Text | 16 words (32 bytes) | **Đồng hồ thời gian** (HH:mm:ss) |
| `0x1520` | Text | 16 words (32 bytes) | **Lịch ngày tháng** (DD/MM/YYYY) |
| `0x1540` | Text | 32 words (64 bytes) | **Thời tiết Thành phố Hồ Chí Minh** (Nhiệt độ, trạng thái) |
| `0x1600` | Integer (0-100) | 1 word | **Tín hiệu WiFi RSSI** (%) |
| `0x1601` | Integer (0-100) | 1 word | **Dung lượng Pin** (%) |
| `0x1700` | Integer (0-100) | 1 word | **Âm lượng hiện tại** (Volume) |

---

## 3. Mã Nguồn C++ Mẫu Ghi Biến Lên DWIN DGUS

Đoạn mã được tích hợp sẵn trong [uart_display.cc](file:///d:/Code/Antigravity/Xiaozhi/config/main/display/uart_display.cc):
```cpp
void UartDisplay::SendDwinText(uint16_t vp_addr, const std::string& text) {
    if (!is_initialized_) return;
    size_t data_len = text.length() + 2; // text + kết thúc 0x00 0x00
    size_t frame_len = 3 + data_len;     // lệnh 0x82 (1B) + địa chỉ VP (2B) + data
    
    std::vector<uint8_t> frame;
    frame.reserve(3 + frame_len);
    frame.push_back(0x5A); // Header byte 1
    frame.push_back(0xA5); // Header byte 2
    frame.push_back(static_cast<uint8_t>(frame_len));
    frame.push_back(0x82); // Write VP Command
    frame.push_back(static_cast<uint8_t>((vp_addr >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(vp_addr & 0xFF));
    for (char c : text) {
        frame.push_back(static_cast<uint8_t>(c));
    }
    frame.push_back(0x00);
    frame.push_back(0x00);
    uart_write_bytes(uart_num_, (const char*)frame.data(), frame.size());
}
```
