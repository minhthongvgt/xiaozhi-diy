# Module Bluetooth UART

ESP32-S3 nối với module Bluetooth rời (HC-05, HC-06, HM-10) qua một cổng UART riêng. Thiết bị ở đầu kia nhận đúng từng byte như khi nối dây UART. ESP32-S3 không có Bluetooth Classic nên không thay được HC-05/HC-06 bằng Bluetooth nội.

Module dùng theo một trong hai cách:

| Cách dùng | Bật khi | Ai dùng cổng UART |
|---|---|---|
| Cầu dữ liệu cho AI | Bật module, màn hình rời không chọn «Không dây» | Công cụ MCP `self.bluetooth.*` |
| Màn hình rời không dây | Màn hình rời chọn «Không dây qua module Bluetooth UART» | `UartDisplay` ([UART_DISPLAY_PROTOCOL.md](UART_DISPLAY_PROTOCOL.md)) |

Mã nguồn: `main/drivers/periph/bt_uart_module.c` (UART, STATE/KEY, lệnh AT), `main/boards/common/bt_uart_bridge.cc` (chia gói, công cụ MCP).

> Lệnh AT trong tài liệu này lấy theo tài liệu của nhà sản xuất và chưa thử trên module thật. Module nhái (JDY-31, JDY-08, MLT-BT05…) có bộ lệnh khác: cấu hình bằng tay rồi chọn «đã cấu hình sẵn».

## Đấu nối

| ESP32-S3 | Module | Ghi chú |
|---|---|---|
| `BT_UART_MODULE_TX_PIN` | RXD | Bắt buộc |
| `BT_UART_MODULE_RX_PIN` | TXD | Bắt buộc |
| `BT_UART_MODULE_STATE_PIN` | STATE (HC-05) / LED (HM-10, cần `AT+PIO11`) | Tùy chọn, mức cao khi đã kết nối |
| `BT_UART_MODULE_KEY_PIN` | KEY / EN (HC-05) | Cần khi firmware gửi lệnh AT cho HC-05 |
| GND | GND | |
| 5 V hoặc 3,3 V | VCC | Theo bo module |

Chân dữ liệu là 3,3 V. Module chỉ có mức 5 V thì cần mạch chuyển mức.

## Cấu hình

Menuconfig `Giao tiếp UART, Bluetooth & MCP [ESP32-S3] → Module Bluetooth UART`, hoặc Web Configurator tab **Mạng**.

| Tùy chọn | Ý nghĩa |
|---|---|
| `ENABLE_BT_UART_MODULE` | Bật module |
| `BT_UART_MODULE_TYPE` | HC-05, HC-06, HM-10, hoặc «đã cấu hình sẵn» (không gửi lệnh AT) |
| `BT_UART_MODULE_PORT` | UART1 hoặc UART2; trùng cổng với tính năng khác thì build lỗi `static_assert` |
| `BT_UART_MODULE_BAUDRATE` | Tốc độ dữ liệu (xuất xưởng 9600) |
| `BT_UART_MODULE_APPLY_SETTINGS` | Gửi lệnh AT đặt tên, PIN, vai trò lúc khởi động |
| `BT_UART_MODULE_ROLE`, `BT_UART_MODULE_REMOTE_ADDR` | HC-05/HM-10 làm chủ thì tự nối tới địa chỉ này (12 chữ số hex, được có `:`) |
| `BT_UART_MODULE_FRAME` | Định dạng gói cho MCP: thô, dòng, khung 4 byte |

## Chọn cặp module

Classic (HC-05, HC-06) và BLE (HM-10) không nối được với nhau.

| Cặp | Chủ (Master) |
|---|---|
| HC-05 ↔ HC-05 | Một trong hai |
| HC-05 ↔ HC-06 | HC-05 (HC-06 chỉ làm tớ) |
| HM-10 ↔ HM-10 | Một trong hai |
| Module ↔ điện thoại | Điện thoại (Classic: app «Serial Bluetooth Terminal»; BLE: nRF Connect). iPhone chỉ có BLE |
| HC-06 ↔ HC-06, Classic ↔ BLE | Không nối được |

Nên đặt module phía Xiaozhi làm chủ khi đầu kia là vi điều khiển. Hai đầu cùng PIN (Classic); baud của mỗi module khớp với vi điều khiển nối với chính nó. Lấy địa chỉ module tớ trước rồi nhập vào module chủ.

## Lệnh AT

Gõ tay bằng bộ chuyển USB–UART (TXD ↔ RXD, 8N1) hoặc Arduino làm cầu nối (`SoftwareSerial` chuyển byte qua lại giữa `Serial` và module).

| Module | Vào chế độ AT | Baud khi gõ | Kết thúc lệnh |
|---|---|---|---|
| HC-05 | Giữ KEY/EN mức cao lúc cấp nguồn (đèn nháy chậm) | 38400 | CR+LF |
| HC-06 | Khi chưa kết nối | Baud dữ liệu | Không có; gõ trọn lệnh trong 1 giây |
| HM-10 | Khi chưa kết nối; gửi `AT` lúc đang nối sẽ cắt kết nối | Baud dữ liệu | Không có |

| Việc | HC-05 | HC-06 | HM-10 |
|---|---|---|---|
| Kiểm tra | `AT` | `AT` | `AT` |
| Khôi phục mặc định | `AT+ORGL` | — | `AT+RENEW` |
| Đọc địa chỉ | `AT+ADDR?` | — (dò bằng điện thoại) | `AT+ADDR?` |
| Đặt tên | `AT+NAME=Xiaozhi-BT` | `AT+NAMEXiaozhi-BT` | `AT+NAMEXiaozhi-BT` |
| Đặt PIN | `AT+PSWD=1234` (FW 3.0: `AT+PSWD="1234"`) | `AT+PIN1234` | `AT+PASS123456` + `AT+TYPE2` |
| Baud | `AT+UART=9600,0,0` | `AT+BAUD4` (4 = 9600, 8 = 115200) | `AT+BAUD0` (0 = 9600, 4 = 115200) |
| Vai trò tớ / chủ | `AT+ROLE=0` / `AT+ROLE=1` | chỉ tớ | `AT+ROLE0` / `AT+ROLE1` |
| Chủ nối địa chỉ cố định | `AT+CMODE=0`, `AT+BIND=98d3,31,f5b9e7` | — | `AT+IMME1`, `AT+CON<12 hex>` |
| Khởi động lại | `AT+RESET` | — | `AT+RESET` |

HM-10: `AT+IMME0` để tự nối lại thiết bị cũ khi bật nguồn; để `AT+NOTI0` khi dùng định dạng dòng hoặc khung 4 byte.

### Lệnh firmware gửi khi bật `BT_UART_MODULE_APPLY_SETTINGS`

Chỉ gửi khi module chưa kết nối (theo STATE; không nối STATE thì luôn gửi). Mỗi lệnh chờ phản hồi tối đa 1 giây, kết quả ghi log nhãn `BtUart`.

| Module | Chuỗi lệnh |
|---|---|
| HC-05 | KEY lên cao → `AT` → `AT+NAME=` → `AT+PSWD=` → `AT+ROLE=` → (chủ) `AT+CMODE=0`, `AT+BIND=` → `AT+RESET` → KEY xuống thấp |
| HC-06 | `AT` → `AT+NAME` → `AT+PIN` |
| HM-10 | `AT` → `AT+NAME` → `AT+PASS`, `AT+TYPE2` → `AT+PIO11` (khi có STATE) → `AT+IMME` → `AT+ROLE` → `AT+RESET` → (chủ) `AT+CON` |

HC-05 không nối chân KEY thì firmware bỏ qua lệnh AT và ghi cảnh báo.

## Công cụ MCP

| Công cụ | Có khi | Tham số / kết quả |
|---|---|---|
| `self.bluetooth.get_status` | Luôn có | `module`, `link` (`connected`, `disconnected`, `unknown` khi không nối STATE), `frame`, `unread` |
| `self.bluetooth.send_text` | Định dạng thô hoặc dòng | `text` (tối đa 256 byte); định dạng dòng tự thêm `\n` |
| `self.bluetooth.send_code` | Khung 4 byte | `code` 0–99 → `0xAA`, chục, đơn vị, XOR ba byte đầu |
| `self.bluetooth.read_messages` | Luôn có | `messages`, `dropped`; đọc xong thì xoá |

Chia gói khi nhận:
- **Dòng:** cắt theo `\n`, bỏ `\r`; dòng dài hơn 256 byte bị bỏ.
- **Thô:** gom byte tới khi nghỉ 50 ms hoặc đủ 256 byte.
- **Khung 4 byte:** tìm `0xAA`, kiểm tra XOR và hai chữ số 0–9; khung sai bị bỏ.

Hàng đợi giữ 16 gói; đầy thì bỏ gói cũ nhất và tăng `dropped`. Byte không phải UTF-8 trả về dạng `\xHH`.

Mã mẫu Arduino cho đầu kia, khung 4 byte:

```cpp
#include <SoftwareSerial.h>

SoftwareSerial bt(10, 11);
const uint8_t kStart = 0xAA;
uint8_t frame[4];
uint8_t frameLen = 0;

void sendCode(uint8_t code) {
  uint8_t tens = code / 10;
  uint8_t units = code % 10;
  uint8_t out[4] = {kStart, tens, units, (uint8_t)(kStart ^ tens ^ units)};
  bt.write(out, 4);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  bt.begin(9600);
}

void loop() {
  while (bt.available()) {
    uint8_t b = bt.read();
    if (frameLen == 0 && b != kStart) continue;
    frame[frameLen++] = b;
    if (frameLen < 4) continue;
    frameLen = 0;
    if ((frame[0] ^ frame[1] ^ frame[2]) != frame[3] || frame[1] > 9 || frame[2] > 9) continue;
    uint8_t code = frame[1] * 10 + frame[2];
    digitalWrite(LED_BUILTIN, code == 1 ? HIGH : LOW);
    sendCode(code);
  }
}
```

Ghi ý nghĩa từng mã vào Prompt trên xiaozhi.me (ví dụ «mã 1 bật đèn, mã 2 tắt đèn») để AI chọn đúng mã.

## Màn hình rời không dây

1. Bật module Bluetooth, đặt chân TX/RX và baud.
2. Trong «Cấu hình Màn hình rời UART», chọn **Không dây qua module Bluetooth UART**; cổng, chân, baud của màn hình bị ẩn.
3. Module phía màn hình làm tớ, cùng baud với màn hình (Nextion xuất xưởng 9600).
4. Giao thức JSON: màn hình gửi `{"event":"hello"}` mỗi lần kết nối lại để nhận lại trạng thái.

## Khắc phục sự cố

| Hiện tượng / log | Kiểm tra |
|---|---|
| `Bluetooth module needs valid TX and RX pins` | Chưa đặt chân TX hoặc RX |
| `UART<n> is already used by another feature` | Cổng UART trùng tính năng khác |
| `HC-05 does not answer AT` | Thiếu hoặc sai chân KEY, sai baud |
| `... only accepts commands while not connected` | HC-06/HM-10 đang kết nối, hoặc sai baud |
| `Remote address ... is not 12 hex digits` | Địa chỉ đầu kia sai; chỉ dùng 0–9, A–F, có thể kèm `:` |
| `Dropped bad frame` | Đầu kia gửi khung 4 byte sai XOR hoặc sai chữ số |
| Gõ `AT` không trả lời | Đảo TX/RX, sai baud, sai kiểu kết thúc dòng |
| Hai module không tự nối | Chưa có bên làm chủ, địa chỉ `BIND`/`CON` sai, PIN khác, ghép Classic với BLE |
| Nối được nhưng chữ lỗi | Baud module khác baud vi điều khiển cùng đầu |
| HM-10 tự ngắt định kỳ | Có bên gửi `AT` khi đang nối; nối chân STATE cho Xiaozhi |
