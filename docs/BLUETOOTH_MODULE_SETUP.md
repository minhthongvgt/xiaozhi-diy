# Cấu hình module Bluetooth đầu phát và đầu nhận

Cập nhật: 2026-10-13

Tài liệu này hướng dẫn cài đặt **cả hai module** của một đường UART không dây: module gắn vào Xiaozhi (ESP32-S3) và module gắn vào thiết bị ở đầu kia (vi điều khiển khác, màn hình rời, điện thoại). Phần firmware và công cụ MCP được mô tả ở [BLUETOOTH_UART.md](BLUETOOTH_UART.md).

> Các lệnh AT dưới đây lấy theo tài liệu của nhà sản xuất (HC-05 firmware 2.0/3.0, HC-06 linvor, HM-10 HMSoft) và **chưa được thử trên module thật** trong dự án. Module nhái có thể trả lời khác; luôn kiểm tra bằng lệnh hỏi (`?`) sau khi đặt.

---

## 1. Chọn cặp module

Hai đầu phải cùng **một loại Bluetooth**. Bluetooth Classic (HC-05, HC-06) và BLE (HM-10) **không kết nối được với nhau**.

| Cặp | Được không | Ai làm chủ (Master) |
| --- | --- | --- |
| HC-05 ↔ HC-05 | Được | Một trong hai, bên còn lại làm tớ |
| HC-05 ↔ HC-06 | Được | HC-05 (HC-06 chỉ làm tớ) |
| HC-06 ↔ HC-06 | **Không** | Không bên nào chủ động kết nối được |
| HM-10 ↔ HM-10 | Được | Một trong hai |
| HC-05/HC-06 ↔ HM-10 | **Không** | Khác chuẩn Bluetooth |
| Module ↔ điện thoại | Được | Điện thoại. HC-05/HC-06 dùng app "Serial Bluetooth Terminal" (Classic); HM-10 dùng nRF Connect hoặc chế độ BLE của app |

**Gợi ý:** đặt module **chủ ở phía Xiaozhi** khi đầu kia là vi điều khiển, vì Xiaozhi có firmware tự cấu hình và tự kết nối lại. Khi cần thử nhanh bằng điện thoại, cho module phía Xiaozhi làm tớ.

### Quy tắc chung cho mọi cặp

1. **Baud dữ liệu** của mỗi module phải khớp với vi điều khiển nối với **chính nó**. Hai module có thể chạy baud khác nhau, nhưng để dễ gỡ lỗi nên đặt cùng một giá trị (khuyên dùng 9600 hoặc 115200).
2. **Mã PIN** của hai đầu phải giống nhau (Classic). HM-10 mặc định không cần PIN.
3. Lấy **địa chỉ của module tớ** trước, rồi nhập vào module chủ.
4. Sau khi đặt xong, khởi động lại cả hai module.

---

## 2. Dụng cụ để gõ lệnh AT bằng tay

Cần làm khi cài module **ở đầu kia**, hoặc khi không muốn firmware Xiaozhi tự cấu hình.

### Cách A: bộ chuyển USB–UART (CP2102, CH340, FT232)

| USB–UART | Module |
| --- | --- |
| TXD | RXD |
| RXD | TXD |
| GND | GND |
| 5 V hoặc 3,3 V | VCC (theo bo module) |
| 3,3 V (chỉ HC-05) | KEY / EN, xem mục 3.1 |

Mở một chương trình terminal (Arduino Serial Monitor, PuTTY, RealTerm, Termite), chọn cổng COM, đặt 8N1 và:

| Module | Baud khi gõ AT | Ký tự kết thúc dòng |
| --- | --- | --- |
| HC-05 (vào AT bằng KEY lúc cấp nguồn) | 38400 | CR+LF (Serial Monitor: "Both NL & CR") |
| HC-06 | Baud dữ liệu hiện tại (xuất xưởng 9600) | Không có ("No line ending"), gõ trọn lệnh trong 1 giây |
| HM-10 | Baud dữ liệu hiện tại (xuất xưởng 9600) | Không có ("No line ending") |

### Cách B: Arduino làm cầu nối

Nạp chương trình sau vào Arduino Uno/Nano, nối TXD module → chân 10, RXD module → chân 11 (qua cầu chia áp nếu module chỉ chịu 3,3 V), rồi gõ lệnh trong Serial Monitor ở 9600:

```cpp
#include <SoftwareSerial.h>

const long kModuleBaud = 38400;
SoftwareSerial bt(10, 11);

void setup() {
  Serial.begin(9600);
  bt.begin(kModuleBaud);
}

void loop() {
  if (bt.available()) Serial.write(bt.read());
  if (Serial.available()) bt.write(Serial.read());
}
```

Đổi `kModuleBaud` thành 9600 với HC-06 và HM-10.

### Cách C: để firmware Xiaozhi tự cấu hình module phía Xiaozhi

Bật `BT_UART_MODULE_APPLY_SETTINGS` trong menuconfig (mục 3 → Module Bluetooth UART). Lúc khởi động, firmware gửi các lệnh ở mục "Lệnh AT firmware gửi khi bật Gửi lệnh AT" của [BLUETOOTH_UART.md](BLUETOOTH_UART.md) và ghi kết quả vào log nhãn `BtUart`. Cách này **không** cấu hình được module ở đầu kia.

---

## 3. Bảng lệnh AT theo module

### 3.1. HC-05

**Vào chế độ AT đầy đủ:** giữ nút trên bo (hoặc nối KEY/EN lên 3,3 V) **trong lúc cấp nguồn**. Đèn nháy chậm (khoảng 2 giây một lần) là đã vào AT, baud 38400. Thả nút sau khi cấp nguồn vẫn ở AT cho tới khi khởi động lại.

**Chế độ AT rút gọn:** module đang chạy bình thường, kéo KEY lên cao rồi gửi lệnh ở **baud dữ liệu**. Firmware Xiaozhi dùng cách này. Một số lệnh hỏi (`AT+NAME?`) không trả lời ở chế độ rút gọn.

| Việc | Lệnh | Trả lời mong đợi |
| --- | --- | --- |
| Kiểm tra | `AT` | `OK` |
| Phiên bản | `AT+VERSION?` | `+VERSION:...` |
| Khôi phục mặc định | `AT+ORGL` | `OK` (sau đó kiểm tra lại `AT+UART?`, `AT+PSWD?`) |
| Địa chỉ của module này | `AT+ADDR?` | `+ADDR:98d3:31:f5b9e7` |
| Đặt tên | `AT+NAME=Xiaozhi-BT` | `OK` |
| Đặt PIN | `AT+PSWD=1234` (firmware 3.0: `AT+PSWD="1234"`) | `OK` |
| Baud dữ liệu | `AT+UART=9600,0,0` (baud, stop bit, parity) | `OK` |
| Vai trò | `AT+ROLE=0` (tớ) / `AT+ROLE=1` (chủ) | `OK` |
| Chủ chỉ nối địa chỉ cố định | `AT+CMODE=0` | `OK` |
| Địa chỉ module tớ để nối | `AT+BIND=98d3,31,f5b9e7` (dấu `:` đổi thành `,`) | `OK` |
| Khởi động lại | `AT+RESET` | `OK` |

Đèn sau khi khởi động lại: nháy nhanh là đang tìm kết nối, nháy hai lần mỗi 2 giây là **đã kết nối**. Chân STATE lên mức cao khi đã kết nối.

### 3.2. HC-06

Nhận lệnh bất cứ lúc nào **chưa kết nối**, ở baud dữ liệu hiện tại, không có ký tự kết thúc. HC-06 chỉ làm tớ, không đặt được vai trò và địa chỉ đích.

| Việc | Lệnh | Trả lời mong đợi |
| --- | --- | --- |
| Kiểm tra | `AT` | `OK` |
| Phiên bản | `AT+VERSION` | `OKlinvorV1.8` |
| Đặt tên | `AT+NAMEXiaozhi-BT` | `OKsetname` |
| Đặt PIN | `AT+PIN1234` | `OKsetPIN` |
| Baud dữ liệu | `AT+BAUD4` | `OK9600` |

Mã baud: 1 = 1200, 2 = 2400, 3 = 4800, 4 = 9600, 5 = 19200, 6 = 38400, 7 = 57600, 8 = 115200. Đổi baud xong phải đổi baud của terminal theo.

Một số HC-06 đời mới dùng lệnh có `=` và kết thúc bằng CR+LF (`AT+NAME=...`, trả lời `+OK`). Thử `AT` với cả hai kiểu kết thúc dòng để biết module thuộc loại nào.

### 3.3. HM-10 (HMSoft)

Nhận lệnh khi **chưa kết nối**, ở baud dữ liệu hiện tại, không có ký tự kết thúc. Gửi `AT` khi đang kết nối sẽ **cắt kết nối**.

| Việc | Lệnh | Trả lời mong đợi |
| --- | --- | --- |
| Kiểm tra | `AT` | `OK` |
| Khôi phục mặc định | `AT+RENEW` | `OK+RENEW` |
| Địa chỉ của module này | `AT+ADDR?` | `OK+ADDR:A4C1381234AB` |
| Đặt tên | `AT+NAMEXiaozhi-BT` | `OK+Set:Xiaozhi-BT` |
| Baud dữ liệu | `AT+BAUD0` | `OK+Set:0` |
| Vai trò | `AT+ROLE0` (tớ) / `AT+ROLE1` (chủ) | `OK+Set:0` / `OK+Set:1` |
| Chủ chờ lệnh, không tự nối khi bật nguồn | `AT+IMME1` | `OK+Set:1` |
| Chủ tự nối lại thiết bị cũ khi bật nguồn | `AT+IMME0` | `OK+Set:0` |
| Nối tới module tớ | `AT+CON<địa chỉ 12 hex>` | `OK+CONNA`, rồi `OK+CONN` |
| PIN | `AT+PASS123456` (6 số) và `AT+TYPE2` (bắt buộc PIN) | `OK+Set:...` |
| Chân LED thành chân trạng thái | `AT+PIO11` (thấp: chưa nối, cao: đã nối) | `OK+Set:1` |
| Báo khi nối/ngắt qua UART | `AT+NOTI1` | `OK+Set:1` |
| Khởi động lại | `AT+RESET` | `OK+RESET` |

Mã baud: 0 = 9600, 1 = 19200, 2 = 38400, 3 = 57600, 4 = 115200. Mặc định `AT+PIO10` làm chân LED nhấp nháy khi chưa nối, nên phải đặt `AT+PIO11` nếu dùng chân này làm STATE cho Xiaozhi.

`AT+NOTI1` làm module gửi chuỗi `OK+CONN` / `OK+LOST` vào luồng dữ liệu. Chỉ bật khi đầu nhận biết lọc các chuỗi này; với định dạng dòng hoặc khung 4 byte thì nên để `AT+NOTI0`.

---

## 4. Các tình huống cài đặt mẫu

Trong mỗi tình huống: **đầu phía Xiaozhi** là module nối với ESP32-S3, **đầu kia** là module nối với vi điều khiển khác hoặc màn hình rời. Địa chỉ mẫu cần thay bằng địa chỉ thật đọc từ `AT+ADDR?`.

### 4.1. Xiaozhi (HC-05, chủ) ↔ vi điều khiển (HC-05 hoặc HC-06, tớ) — khuyên dùng

**Đầu kia (tớ), gõ tay:**

HC-05:
```
AT+ORGL
AT+ROLE=0
AT+PSWD=1234
AT+UART=9600,0,0
AT+ADDR?
AT+RESET
```
HC-06:
```
AT+PIN1234
AT+BAUD4
```
HC-06 không có lệnh đọc địa chỉ: dò bằng điện thoại (Cài đặt Bluetooth hoặc app "Serial Bluetooth Terminal" → thiết bị → địa chỉ) hoặc bằng `AT+INQ` trên một HC-05 làm chủ.

**Đầu phía Xiaozhi (chủ):** chọn một trong hai cách.

- Để firmware tự cấu hình, đặt trong menuconfig:

  | Tuỳ chọn | Giá trị |
  | --- | --- |
  | Loại module | HC-05 |
  | Chân KEY | GPIO nối chân KEY/EN (bắt buộc) |
  | Baud | 9600 |
  | Gửi lệnh AT khi khởi động | bật |
  | Mã PIN | `1234` |
  | Vai trò | Chủ |
  | Địa chỉ đầu kia | `98D331F5B9E7` |

- Hoặc gõ tay rồi chọn "Module đã cấu hình sẵn":
  ```
  AT+ORGL
  AT+ROLE=1
  AT+CMODE=0
  AT+BIND=98d3,31,f5b9e7
  AT+PSWD=1234
  AT+UART=9600,0,0
  AT+RESET
  ```

### 4.2. Xiaozhi (HC-05 hoặc HC-06, tớ) ← vi điều khiển (HC-05, chủ)

**Đầu phía Xiaozhi (tớ):** HC-06 xuất xưởng đã là tớ, PIN 1234, 9600, nên có thể dùng ngay với "Module đã cấu hình sẵn". HC-05 thì đặt `AT+ROLE=0`, hoặc trong menuconfig chọn vai trò Tớ. Đọc địa chỉ module này (`AT+ADDR?`, hoặc dò bằng điện thoại với HC-06).

**Đầu kia (chủ, HC-05):** gõ tay `AT+ROLE=1`, `AT+CMODE=0`, `AT+BIND=<địa chỉ module Xiaozhi>`, `AT+PSWD=<cùng PIN>`, `AT+UART=9600,0,0`, `AT+RESET`.

### 4.3. Xiaozhi (HM-10, chủ) ↔ vi điều khiển (HM-10, tớ)

**Đầu kia (tớ), gõ tay:**
```
AT+RENEW
AT+ROLE0
AT+BAUD0
AT+NOTI0
AT+ADDR?
AT+RESET
```

**Đầu phía Xiaozhi (chủ):** trong menuconfig chọn HM-10, baud 9600, bật gửi lệnh AT, vai trò Chủ, địa chỉ đầu kia (ví dụ `A4C1381234AB`), để trống mã PIN. Nối thêm chân LED của HM-10 vào `BT_UART_MODULE_STATE_PIN` để firmware không gửi `AT` khi đang kết nối. Mỗi lần khởi động, firmware đặt `AT+IMME1`, `AT+ROLE1`, khởi động lại module rồi gửi `AT+CON<địa chỉ>`.

Muốn tự cấu hình bằng tay thay vì firmware: `AT+RENEW`, `AT+ROLE1`, `AT+IMME1`, `AT+PIO11`, `AT+RESET`, rồi `AT+CON<địa chỉ>`. Khi nối thành công một lần, đặt `AT+IMME0` để module tự nối lại sau mỗi lần bật nguồn, rồi chọn "Module đã cấu hình sẵn".

### 4.4. Màn hình rời không dây

Giống 4.1 hoặc 4.3: module phía Xiaozhi làm chủ, module gắn vào màn hình làm tớ. Thêm các điểm sau:

1. Baud của module **phía màn hình** phải bằng baud màn hình đang dùng (Nextion xuất xưởng chạy 9600, màn tự làm theo code của bạn).
2. Menuconfig mục 1, "Cấu hình Màn hình rời UART": chọn **Không dây qua module Bluetooth UART** và giao thức của màn hình.
3. Với giao thức JSON, màn hình gửi `{"event":"hello"}` mỗi khi phát hiện kết nối lại (ví dụ đọc chân STATE của module phía màn hình) để Xiaozhi gửi lại toàn bộ trạng thái.

### 4.5. Thử nhanh bằng điện thoại

1. Đặt module phía Xiaozhi làm **tớ** (HC-05 `AT+ROLE=0`, HC-06 sẵn có, HM-10 `AT+ROLE0`).
2. Điện thoại Android: ghép đôi trong Cài đặt Bluetooth (PIN 1234) với HC-05/HC-06, rồi mở "Serial Bluetooth Terminal" → Devices → Bluetooth Classic. Với HM-10, chọn tab Bluetooth LE, không cần ghép đôi trong Cài đặt.
3. Gõ một dòng trên app, rồi hỏi Xiaozhi "đọc tin nhắn Bluetooth" để AI gọi `self.bluetooth.read_messages`. Bảo Xiaozhi "gửi chữ xin chào qua Bluetooth" để thấy chữ hiện trên app.

iPhone chỉ hỗ trợ BLE: chỉ thử được với HM-10.

---

## 5. Đặt định dạng gói và mã cho vi điều khiển ở đầu kia

Chọn định dạng trong menuconfig (`BT_UART_MODULE_FRAME`) theo cách vi điều khiển đầu kia đọc dữ liệu.

| Định dạng | Xiaozhi gửi | Đầu kia gửi về | Hợp với |
| --- | --- | --- | --- |
| Dòng văn bản | `text` + `\n` | Mỗi tin một dòng kết thúc `\n` | Lệnh dạng chữ, JSON, giá trị cảm biến |
| Thô | `text` đúng như gõ | Byte bất kỳ, tách tin theo khoảng nghỉ 50 ms | Giao thức có sẵn của thiết bị |
| Khung 4 byte | `0xAA`, chục, đơn vị, XOR | Cùng dạng khung | Mã lệnh 0–99, mạch đơn giản |

### Mã mẫu Arduino: định dạng dòng

```cpp
#include <SoftwareSerial.h>

SoftwareSerial bt(10, 11);
String line;

void handleCommand(const String& cmd) {
  if (cmd == "LED ON") {
    digitalWrite(LED_BUILTIN, HIGH);
    bt.println("OK LED ON");
  } else if (cmd == "LED OFF") {
    digitalWrite(LED_BUILTIN, LOW);
    bt.println("OK LED OFF");
  } else if (cmd == "TEMP?") {
    bt.print("TEMP ");
    bt.println(analogRead(A0));
  } else {
    bt.println("UNKNOWN");
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  bt.begin(9600);
}

void loop() {
  while (bt.available()) {
    char c = bt.read();
    if (c == '\n') {
      line.trim();
      if (line.length() > 0) handleCommand(line);
      line = "";
    } else if (c != '\r' && line.length() < 200) {
      line += c;
    }
  }
}
```

### Mã mẫu Arduino: khung 4 byte

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

void handleCode(uint8_t code) {
  if (code == 1) digitalWrite(LED_BUILTIN, HIGH);
  if (code == 2) digitalWrite(LED_BUILTIN, LOW);
  sendCode(code);
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
    bool valid = (frame[0] ^ frame[1] ^ frame[2]) == frame[3] && frame[1] <= 9 && frame[2] <= 9;
    if (valid) handleCode(frame[1] * 10 + frame[2]);
  }
}
```

Mẫu trên gửi lại đúng mã vừa nhận để xác nhận; Xiaozhi đọc được bằng `self.bluetooth.read_messages`.

Nên thêm vào Prompt trên xiaozhi.me bảng ý nghĩa mã lệnh (ví dụ "mã 1 bật đèn, mã 2 tắt đèn") để AI chọn đúng mã khi gọi `self.bluetooth.send_code`.

---

## 6. Danh sách kiểm tra khi không kết nối được

| Hiện tượng | Kiểm tra |
| --- | --- |
| Gõ `AT` không có trả lời | Đảo TX/RX; sai baud (HC-05 AT đầy đủ là 38400); sai kiểu kết thúc dòng; HC-06/HM-10 đang kết nối |
| HC-05 trả lời `ERROR:(0)` | Lệnh sai cú pháp hoặc đang ở AT rút gọn với lệnh hỏi; thử vào AT đầy đủ |
| Hai module không tự nối | Một bên chưa làm chủ; địa chỉ `BIND`/`CON` sai; PIN khác nhau; ghép Classic với BLE |
| Nối được nhưng chữ lỗi | Baud của module khác baud vi điều khiển ở cùng đầu |
| Mất ký tự khi gửi nhanh | Hạ baud xuống 9600 hoặc gửi ít dữ liệu hơn; SoftwareSerial trên Arduino không ổn định ở 115200 |
| HM-10 tự ngắt định kỳ | Firmware hoặc vi điều khiển gửi `AT` khi đang nối; nối chân STATE cho Xiaozhi |
| Xiaozhi log `Remote address ... is not 12 hex digits` | Địa chỉ nhập sai; chỉ dùng 0–9, A–F, có thể kèm `:` |
