# Module Bluetooth UART (cầu UART không dây)

Cập nhật: 2026-10-13

## Tổng quan

ESP32-S3 nối với một module Bluetooth rời (HC-05, HC-06, HM-10) qua **một cổng UART riêng**. Module ở đầu kia (một vi điều khiển khác, hoặc một module Bluetooth gắn vào vi điều khiển khác) nhận đúng từng byte như khi nối dây UART. Firmware chỉ chuyển dữ liệu, không đổi nội dung.

Module dùng theo một trong hai cách, tự chọn theo cấu hình:

| Cách dùng | Bật khi | Ai đọc/ghi cổng UART |
| --- | --- | --- |
| Cầu dữ liệu cho AI | Bật module, màn hình rời không chọn "Không dây" | Công cụ MCP `self.bluetooth.*` |
| Màn hình rời không dây | Màn hình rời chọn "Không dây qua module Bluetooth UART" | `UartDisplay`, giống hệt màn hình nối dây |

Hai cách không chạy cùng lúc: một module chỉ ghép được với một thiết bị ở đầu kia.

ESP32-S3 không có Bluetooth Classic, nên phần Bluetooth nội của chip không thay được HC-05/HC-06. Module rời cũng không chiếm sóng Wi-Fi và RAM của chatbot.

Cách cài đặt cả hai module (gõ lệnh AT bằng tay, ghép đôi chủ/tớ, mã mẫu cho vi điều khiển đầu kia): [BLUETOOTH_MODULE_SETUP.md](BLUETOOTH_MODULE_SETUP.md).

Mã nguồn: `main/drivers/periph/bt_uart_module.c` (UART, chân STATE/KEY, lệnh AT), `main/boards/common/bt_uart_bridge.cc` (chia gói và công cụ MCP), `main/boards/esp32s3-n16r8-custom/custom_n16r8_board.cc` (`InitializeBtUartModule`, `InitializeBtUartTools`).

## Đấu nối

| ESP32-S3 | Module | Ghi chú |
| --- | --- | --- |
| `BT_UART_MODULE_TX_PIN` | RXD | Bắt buộc |
| `BT_UART_MODULE_RX_PIN` | TXD | Bắt buộc (cần để đọc dữ liệu và phản hồi lệnh AT) |
| `BT_UART_MODULE_STATE_PIN` | STATE (HC-05) / LED (HM-10) | Tuỳ chọn. Mức cao khi đã kết nối |
| `BT_UART_MODULE_KEY_PIN` | KEY / EN (chỉ HC-05) | Tuỳ chọn. Cần khi muốn firmware gửi lệnh AT cho HC-05 |
| GND | GND | Chung mass |
| 5 V hoặc 3,3 V | VCC | Theo bo module. HC-05/HC-06 dạng bo đế thường cần 5 V |

Mức logic của chân dữ liệu là 3,3 V. Bo HC-05/HC-06/HM-10 phổ biến dùng 3,3 V trên RXD/TXD, nối thẳng được. Module chỉ có chân 5 V thì cần mạch chuyển mức.

## Cấu hình (menuconfig mục 3 hoặc Web Configurator tab Mạng)

| Tuỳ chọn | Ý nghĩa |
| --- | --- |
| `ENABLE_BT_UART_MODULE` | Bật module |
| Loại module | HC-05, HC-06, HM-10, hoặc "đã cấu hình sẵn" (không gửi lệnh AT) |
| Cổng UART | UART1 hoặc UART2. Phải khác cổng UART mở rộng và cổng màn hình rời có dây; trùng cổng thì build báo lỗi `static_assert` |
| Baud | Tốc độ dữ liệu của module (xuất xưởng 9600) |
| `BT_UART_MODULE_APPLY_SETTINGS` | Gửi lệnh AT đặt tên, mã PIN, vai trò lúc khởi động |
| Vai trò, địa chỉ đầu kia | HC-05 và HM-10: làm chủ thì tự kết nối tới module có địa chỉ đã nhập (12 chữ số hex, được phép có `:`) |
| Định dạng gói | Chỉ dùng cho công cụ MCP: thô, dòng văn bản, khung 4 byte |

## Lệnh AT firmware gửi khi bật "Gửi lệnh AT"

Chỉ gửi khi module chưa kết nối (theo chân STATE; không nối STATE thì luôn gửi). Mỗi lệnh chờ phản hồi tối đa 1 giây, kết quả ghi vào log với nhãn `BtUart`.

| Module | Chuỗi lệnh |
| --- | --- |
| HC-05 | Kéo KEY lên cao → `AT` → `AT+NAME=<tên>` → `AT+PSWD=<pin>` (lỗi thì thử `AT+PSWD="<pin>"`) → `AT+ROLE=0/1` → (chủ) `AT+CMODE=0`, `AT+BIND=NAP,UAP,LAP` → `AT+RESET` → hạ KEY. Lệnh kết thúc bằng `\r\n` |
| HC-06 | `AT` → `AT+NAME<tên>` → `AT+PIN<pin>`. Không có ký tự kết thúc lệnh |
| HM-10 | `AT` → `AT+NAME<tên>` → `AT+PASS<pin>`, `AT+TYPE2` → `AT+PIO11` (khi có chân STATE) → `AT+IMME0/1` → `AT+ROLE0/1` → `AT+RESET` → (chủ) `AT+CON<địa chỉ>`. Không có ký tự kết thúc lệnh |

Lưu ý:
- HC-05 không có chân KEY thì không nhận lệnh AT; firmware bỏ qua và ghi cảnh báo.
- HM-10 bị ngắt kết nối nếu nhận chuỗi `AT` khi đang kết nối. Nên nối chân STATE để firmware biết mà bỏ qua.
- Các module "nhái" (JDY-31, JDY-08, MLT-BT05…) có bộ lệnh khác. Khi đó cấu hình module bằng công cụ khác rồi chọn "đã cấu hình sẵn".
- Chuỗi lệnh ở trên dựa theo tài liệu của từng module và **chưa thử trên module thật**.

## Công cụ MCP (cách dùng "Cầu dữ liệu cho AI")

| Công cụ | Có khi | Tham số / kết quả |
| --- | --- | --- |
| `self.bluetooth.get_status` | Luôn có | `module`, `link` (`connected`, `disconnected`, `unknown` khi không nối STATE), `frame`, `unread` |
| `self.bluetooth.send_text` | Định dạng thô hoặc dòng | `text` (tối đa 256 byte). Định dạng dòng tự thêm `\n` |
| `self.bluetooth.send_code` | Định dạng khung 4 byte | `code` 0–99 → `0xAA`, chục, đơn vị, XOR ba byte đầu |
| `self.bluetooth.read_messages` | Luôn có | `messages` (chuỗi, hoặc số với khung 4 byte), `dropped`. Đọc xong thì xoá |

Cách chia gói khi nhận:
- **Dòng:** cắt theo `\n`, bỏ `\r`, dòng dài quá 256 byte bị bỏ.
- **Thô:** gom byte tới khi đường truyền nghỉ 50 ms hoặc đủ 256 byte.
- **Khung 4 byte:** tìm byte `0xAA`, kiểm tra XOR và hai chữ số 0–9; khung sai bị bỏ và ghi log.

Hàng đợi giữ tối đa 16 gói; đầy thì bỏ gói cũ nhất và tăng `dropped`. Byte không phải UTF-8 hợp lệ được trả về dạng `\xHH`.

## Màn hình rời không dây

1. Bật module Bluetooth ở mục 3, đặt chân TX/RX và baud.
2. Ở mục 1, "Cấu hình Màn hình rời UART", chọn **Không dây qua module Bluetooth UART**. Các ô cổng, chân, baud của màn hình bị ẩn vì dùng của module.
3. Phía màn hình: gắn một module Bluetooth vào cổng UART của màn hình, ghép đôi với module bên ESP32 (một bên làm chủ), đặt cùng baud.

Giao thức màn hình (Nextion, JSON, Raw, DWIN) giữ nguyên. Với JSON, màn hình nên gửi `{"event":"hello"}` mỗi lần kết nối lại để nhận lại trạng thái, xem [UART_DISPLAY_PROTOCOL.md](UART_DISPLAY_PROTOCOL.md).

## Khắc phục sự cố

| Log | Nguyên nhân |
| --- | --- |
| `Bluetooth module needs valid TX and RX pins` | Chưa đặt chân TX hoặc RX |
| `UART<n> is already used by another feature` | Cổng UART đã bị tính năng khác dùng |
| `HC-05 does not answer AT` | Thiếu hoặc sai chân KEY, sai baud |
| `... does not answer AT; it only accepts commands while not connected` | HC-06/HM-10 đang kết nối, hoặc sai baud |
| `Remote address ... is not 12 hex digits` | Địa chỉ đầu kia nhập sai |
| `Dropped bad frame` | Đầu kia gửi khung 4 byte sai XOR hoặc sai chữ số |
