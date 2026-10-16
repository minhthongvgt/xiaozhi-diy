# Bộ nhớ SRAM, DMA và luồng xử lý trên ESP32-S3 N16R8

Nghiên cứu ngày 2026-10-16, dựa trên mã nguồn, `sdkconfig` của thiết bị thật (`C:\x`), kết quả `idf.py size` của bản build hiện tại và tài liệu ESP-IDF 6.1 (`docs/en/api-guides/external-ram.rst`, `performance/speed.rst`).

Mục 2.2 là ước tính ban đầu. **Số đo thật trên thiết bị (COM10, cùng ngày) ở mục 8** và thay cho ước tính khi hai bên khác nhau.

---

## 1. Các cấp bộ nhớ của ESP32-S3 N16R8

| Cấp | Dung lượng | Tốc độ | DMA đọc/ghi được | Dùng cho |
|---|---|---|---|---|
| Thanh ghi CPU | — | 1 chu kỳ | — | biến cục bộ đang tính |
| Cache lệnh (I-cache) | 32 KB (`ESP32S3_INSTRUCTION_CACHE_32KB`) | gần như SRAM khi trúng cache | không | lệnh đọc từ PSRAM/flash |
| Cache dữ liệu (D-cache) | 32 KB, dòng 64 B (`ESP32S3_DATA_CACHE_32KB`) | gần như SRAM khi trúng cache | không | dữ liệu đọc từ PSRAM/flash |
| SRAM nội (DIRAM) | 341 760 B dùng được sau khi trừ cache | nhanh nhất, không phụ thuộc cache | **có** | stack task, ISR, bộ đệm DMA, Wi-Fi, mã IRAM |
| RTC FAST / SLOW | 8 KB + 8 KB | chậm hơn SRAM | không | dữ liệu giữ qua deep sleep |
| PSRAM Octal | 8 MB, 80 MHz (`SPIRAM_MODE_OCT`, `SPIRAM_SPEED_80M`) | chậm khi trượt cache, dùng chung cache với flash | có nhưng hạn chế (mục 3) | heap lớn, LVGL, AFE, Opus, ảnh, **mã chương trình** |
| Flash QIO | 16 MB, 80 MHz | chậm nhất | không | lưu ảnh firmware, NVS, phân vùng `model` |

Dự án bật `SPIRAM_FETCH_INSTRUCTIONS` và `SPIRAM_RODATA` (XiP từ PSRAM): lúc khởi động, mã và hằng số được chép từ flash sang PSRAM rồi chạy từ đó. Theo tài liệu ESP-IDF, lợi ích:
- PSRAM Octal 80 MHz nhanh hơn flash Quad 80 MHz.
- Cache **không bị tắt** khi ghi flash (NVS, OTA, tải assets), nên ISR và task có stack trong PSRAM vẫn chạy khi đang ghi flash.

Đổi lại, firmware chiếm khoảng 3 MB PSRAM.

## 2. SRAM nội: đang dùng vào đâu

### 2.1. Phần tĩnh (đo bằng `idf.py size`)

| Phần | Byte |
|---|---|
| DIRAM tổng | 341 760 |
| `.text` (mã IRAM) | 113 195 |
| `.data` | 36 584 |
| `.bss` | 32 152 |
| **Còn lại cho heap lúc khởi động** | **158 801** |

Thư viện chiếm SRAM nội nhiều nhất:

| Thư viện | Byte | Ghi chú |
|---|---|---|
| Wi-Fi (`libpp`, `libnet80211`, `libphy`) | 40 587 | do `ESP_WIFI_IRAM_OPT`, `ESP_WIFI_RX_IRAM_OPT` |
| `libspi_flash` | 20 099 | hàm thao tác flash phải ở IRAM |
| `libesp_hw_support` | 13 613 | |
| `libdl_lib` (esp-sr) | 11 007 | |
| `libfreertos` | 10 243 | |
| `libheap` | 7 364 | |
| `libmain` (mã dự án) | 6 963 | chủ yếu `.bss` |

### 2.2. Phần lúc chạy (ước tính, cấu hình của `C:\x`)

| Hạng mục | Byte | Nguồn |
|---|---|---|
| DMA I2S phát (PCM5102A) | 15 360 | 8 desc × 480 khung × 4 B (mono 32 bit) |
| DMA I2S thu (INMP441) | 15 360 | như trên |
| Bộ đệm vẽ LVGL (DMA) | 9 600 | 240 × 20 dòng × 2 B, một bộ đệm |
| Stack task luôn chạy | ~63 000 | bảng mục 4: main 6 584, sys_evt 10 608, esp_timer 4 096, tcpip 3 584, Wi-Fi ~6 600, IPC 2×1 280, audio_input 6 144, audio_output 8 192, LVGL 7 168, ssl_receive 4 096 |
| Stack task tạm thời | 8 192 / 6 144 | activation, notify_http (chỉ khi đang chạy) |
| Bộ đệm Wi-Fi/lwIP | 5 000 → hơn 50 000 | RX tĩnh 3 cái; RX động tối đa 6 cái, TX động tối đa 32 cái, nằm trong SRAM nội vì `SPIRAM_TRY_ALLOCATE_WIFI_LWIP` đang tắt |

Tổng cộng khoảng 110–150 KB trên 159 KB, nên lúc mạng bận SRAM nội còn trống chỉ khoảng vài chục KB. Đây là tài nguyên khan hiếm nhất của hệ thống, không phải PSRAM.

`SPIRAM_MALLOC_RESERVE_INTERNAL=102400` giữ 100 KB SRAM nội chỉ cho các lệnh cấp phát yêu cầu rõ SRAM nội hoặc DMA. `SPIRAM_MALLOC_ALWAYSINTERNAL=1024`: `malloc` từ 1 KB trở xuống vào SRAM nội, lớn hơn thì vào PSRAM.

## 3. DMA

| Ngoại vi | Bộ đệm | Vị trí | Ghi chú |
|---|---|---|---|
| I2S phát / thu | 8 × 480 khung mỗi chiều | SRAM nội (driver luôn dùng `MALLOC_CAP_INTERNAL \| MALLOC_CAP_DMA`) | 16 kHz → 30 ms mỗi desc, **240 ms đệm mỗi chiều** (IDF.md mục 8 bắt buộc) |
| SPI LCD | `width × 20` dòng, `buff_dma = 1`, `buff_spiram = 0` | SRAM nội | một bộ đệm: CPU chờ DMA đẩy xong mới vẽ tiếp |
| RGB LCD (ST7701) | `double_buffer = true` | xem `lcd_display.cc` | |
| Camera | khung ảnh | PSRAM (`CAMERA_PSRAM_DMA=y`), DMA tối đa 32 KB một lần | |
| Còi thụ động (RMT) | 1 ký hiệu | không dùng DMA | |

Giới hạn DMA từ PSRAM trên ESP32-S3 (tài liệu ESP-IDF):
- Descriptor DMA phải nằm trong SRAM nội.
- Băng thông DMA vào PSRAM rất hạn chế, nhất là khi CPU cũng đang truy cập PSRAM.
- Vì vậy bộ đệm âm thanh và màn hình nên ở SRAM nội như hiện tại. Chỉ camera, vốn không yêu cầu thời gian thực chặt, mới dùng PSRAM.

Bộ đệm I2S 240 ms đánh đổi:
- Chống rè tốt khi Wi-Fi chiếm CPU.
- Thêm khoảng 240 ms độ trễ phát.
- Kênh tham chiếu AEC phải bù độ lệch tương ứng (`soft_aec_reference.cc` đã tự tính).

## 4. Luồng xử lý (task FreeRTOS)

`configMAX_PRIORITIES = 25` (0–24). Task hệ thống của ESP-IDF:
- IPC: ưu tiên 24.
- Wi-Fi: 23, lõi 0.
- esp_timer: 22, lõi 0.
- Vòng sự kiện hệ thống `sys_evt`: 20, lõi 0.
- lwIP `tcpip`: 18, **không ghim lõi**.

| Task | Lõi | Ưu tiên | Stack | Vị trí stack | Mã nguồn |
|---|---|---|---|---|---|
| audio_input | 1 | 23 | 6 144 (AFE) / 4 096 | SRAM nội | `audio/audio_service.cc` |
| audio_output | 1 | 23 | 8 192 | SRAM nội | `audio/audio_service.cc` |
| (AFE "se" của esp-sr) | — | — | — | — | đo thực tế: esp-sr **không tạo task riêng** với cấu hình này; AEC chạy trong `audio_input` |
| audio_afe (fetch: WakeNet, VAD) | 1 | 5 | 4 096 | PSRAM | `audio/engines/afe_audio_engine.cc` |
| encode_wake_word | 1 | 4 | 24 576 / 28 672 | PSRAM | `afe_audio_engine.cc`, `custom_wake_word.cc` |
| opus_codec | 1 | 3 | 24 576 | PSRAM | `audio/audio_service.cc` |
| main (`Application::Run`) | 0 | 10 | 6 584 | SRAM nội | `main.cc`, `application.cc` |
| activation | 1 | 2 | 8 192 | SRAM nội | `application.cc` |
| notify_http | 0 | 4 | 6 144 | SRAM nội | `notify/notify_player.cc` (trước 2026-10-16: lõi 1, ưu tiên 10) |
| ssl_receive / tcp_receive / udp_receive (nhận gói từ máy chủ) | không ghim | 1 | 4 096 | SRAM nội | `managed_components/78__esp-ml307` |
| mqtt_task (esp-mqtt) | không ghim | 5 | 4 096 (đặt cứng trong `esp_mqtt.cc` của `78__esp-ml307`) | SRAM nội | bắt tay TLS chạy trong task này |
| LVGL | 1 | 1 | 7 168 | SRAM nội | `display/lcd_display.cc` |
| LedEvent | 1 | 2 | 4 096 | SRAM nội | `led/gpio_led.cc` |
| uart_code_rx, bt_uart_rx | 1 | 3 | 4 096 | SRAM nội | `boards/common/` |
| uart_disp_rx, periph_touch | không ghim | 3 | 4 096 / 3 072 | SRAM nội | `display/uart_display.cc`, board |
| camera | 0 | mặc định | 4 096 | | `CAMERA_CORE0` |

Phân lõi hiện tại khớp IDF.md mục 5:
- Lõi 0: mạng, hệ thống và vòng sự kiện `Application`.
- Lõi 1: âm thanh và giao diện.

## 5. Đánh giá độ ưu tiên

### 5.1. Nguyên tắc (tài liệu ESP-IDF, `performance/speed.rst`)
1. Trên chip hai lõi, task ở lõi 1 có ưu tiên **từ 19 trở lên** không bị task hệ thống nào chen ngang.
2. Task có thao tác TCP/IP phải có ưu tiên **thấp hơn lwIP (18)** để tránh đảo ưu tiên.
3. Có thể ghim lwIP vào lõi 0 (`LWIP_TCPIP_TASK_AFFINITY_CPU0`) để lõi 1 hoàn toàn dành cho ứng dụng. Thông lượng TCP có thể giảm nhẹ.
4. Không nên đặt task ứng dụng cao hơn Wi-Fi (23) trên cùng lõi với Wi-Fi.
5. Thêm nguyên tắc chung về thời gian thực: hạn chót càng ngắn thì ưu tiên càng cao. Task chủ yếu chờ DMA và làm rất ít việc được phép ở ưu tiên cao. Task tính toán nặng đặt ở mức trung bình, giao diện ở mức thấp.

### 5.2. Từng task

| Task | Hiện tại | Đánh giá |
|---|---|---|
| audio_input / audio_output | 23, lõi 1 | **Hợp lý.** Ở lõi 1, không làm mạng, việc nhẹ (đọc/ghi DMA, đổi định dạng, chép vào ring buffer). Phải phục vụ DMA trong 30 ms. Lệch IDF.md mục 5 ("18–20"); tài liệu ESP-IDF cho thấy 18 là sai (ngang lwIP), nên IDF.md nên sửa thành "19–23, chỉ ở lõi 1, không làm mạng". |
| Tác vụ AFE trong esp-sr (AEC) | không tồn tại | **Đã kiểm chứng trên thiết bị:** không có task riêng; AEC chạy trong `audio_input` (24–31% CPU lõi 1 ở ưu tiên 23, theo từng khối 10 ms). Không cần đặt `afe_perferred_core`. |
| audio_afe (fetch) 5, encode_wake_word 4, opus_codec 3 | lõi 1 | **Hợp lý.** Thu âm trước, nén sau; hàng đợi 4 gói (240 ms) hấp thụ trễ. |
| notify_http | 4, lõi 0 (đã đổi) | Trước là 10 ở lõi 1, cao hơn AFE (5) và Opus (3) trên lõi âm thanh. Lõi 0 đo được rảnh hơn 90%, nên đã chuyển sang lõi 0, ưu tiên 4 (dưới vòng chính và lwIP). |
| main (`Application::Run`) | 10, lõi 0 | **Hợp lý.** Có gửi TLS nên phải dưới 18. Mọi callback hội tụ về đây (rules.md mục 15). |
| ssl_receive / tcp_receive | 1, không ghim | **Rủi ro nhẹ.** Nhận âm thanh TTS từ máy chủ và giải mã TLS ở mức thấp nhất, ngang LVGL. Khi cả hai lõi bận, gói xuống bị trễ. Task nằm trong component quản lý (`78__esp-ml307`) nên không sửa được trực tiếp. Hiện chưa gây lỗi vì các task khác phần lớn đang chờ. Theo dõi bằng đo tải CPU (mục 6). |
| LVGL | 1, lõi 1 | **Hợp lý cho chatbot.** Giao diện nhường âm thanh; khung hình có thể giật khi AFE bận. IDF.md mục 5 ghi 5–9, nhưng đặt LVGL trên AFE (5) sẽ làm hại âm thanh. Nên sửa IDF.md theo thực tế này. |
| activation 2, LedEvent 2, các task UART 3 | | **Hợp lý.** |

## 6. Đo thật trên thiết bị (đề xuất, chưa làm)

Log hiện có: mỗi 10 giây firmware in `free sram: X minimal sram: Y` (`SystemInfo::PrintHeapStats`). Đây là con số đầu tiên cần xem khi cắm thiết bị.

Đo đầy đủ, chỉ làm trong `C:\x` và gỡ ngay sau khi đo:
1. Bật `CONFIG_FREERTOS_USE_TRACE_FACILITY`, `CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS`, `CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS`.
2. Mỗi 10 giây in:
   - `vTaskGetRunTimeStats()`: phần trăm CPU từng task;
   - `vTaskList()`: stack còn trống tối thiểu từng task;
   - `heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA)`.
3. Ghi số liệu ở 4 trạng thái: chờ, nghe từ đánh thức, đang nghe người nói, đang phát TTS.

Từ số đo mới quyết định các mục 7.2–7.4.

## 7. Đề xuất theo thứ tự ưu tiên

### 7.1. Làm ngay (rủi ro thấp): kết quả ngày 2026-10-16
1. Tác vụ AFE của esp-sr: **bỏ**, vì đo thấy esp-sr không tạo task riêng.
2. `notify_http`: **đã làm**, chuyển sang lõi 0, ưu tiên 4.
3. IDF.md mục 5: **đã sửa** theo số đo (âm thanh thời gian thực 19–23 chỉ ở lõi 1; task có mạng dưới 18; LVGL thấp hơn chuỗi âm thanh).
4. **Mới phát hiện, cần quyết định:** `mqtt_task` chỉ còn 432 byte stack (mục 8.3).

### 7.2. Tiết kiệm SRAM nội (làm khi số đo cho thấy còn dưới khoảng 40 KB)
| Thay đổi | Lợi | Giá |
|---|---|---|
| `CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP=y` | bộ đệm Wi-Fi/lwIP động sang PSRAM, đỡ hàng chục KB lúc mạng bận | thông lượng giảm nhẹ; âm thanh Opus chỉ cần vài KB/s |
| Stack LVGL sang PSRAM (`port_cfg.task_stack_caps = MALLOC_CAP_SPIRAM`) | 7 KB | vẽ chậm hơn chút; an toàn khi ghi flash nhờ XiP |
| Giảm `ESP_SYSTEM_EVENT_TASK_STACK_SIZE` (10 096) theo số đo stack còn trống | tới khoảng 6 KB | phải đo trước, vì callback Wi-Fi chạy trong task này |
| Tắt `ESP_WIFI_IRAM_OPT` / `ESP_WIFI_RX_IRAM_OPT` | khoảng 10–20 KB IRAM | Wi-Fi chậm hơn; cần thử với âm thanh 2 chiều |

### 7.3. Tăng hiệu năng (làm khi số đo cho thấy lõi 1 trên 70% CPU hoặc giao diện giật)
| Thay đổi | Lợi | Giá |
|---|---|---|
| Tối ưu biên dịch `-O2` (`COMPILER_OPTIMIZATION_PERF`) thay `-Og` hiện tại | Opus, LVGL, mã dự án nhanh hơn (esp-sr là thư viện dựng sẵn, không đổi) | firmware lớn hơn (còn trống 33%); IRAM có thể tăng |
| D-cache 64 KB (`ESP32S3_DATA_CACHE_64KB`) | ít trượt cache với dữ liệu PSRAM (AFE, LVGL, ảnh) | mất 32 KB SRAM nội: chỉ làm khi 7.2 đã đủ chỗ |
| LVGL hai bộ đệm (`double_buffer = true`) | vẽ song song với DMA | thêm 9,6 KB SRAM nội (màn 240×320) |
| Ghim lwIP vào lõi 0 | lõi 1 chỉ còn âm thanh và giao diện | thông lượng TCP giảm nhẹ |

### 7.4. Giữ nguyên
- DMA I2S 8 × 480 và hàng đợi 4 gói (IDF.md mục 8). Muốn giảm độ trễ thì phải đo rè trước, không đổi theo cảm tính.
- Bộ đệm âm thanh và màn hình ở SRAM nội; camera ở PSRAM.
- Stack của Opus, AFE fetch, encode_wake_word ở PSRAM (đúng chỗ, an toàn nhờ XiP).

## 8. Số đo trên thiết bị thật (2026-10-16, COM10)

Cách đo:
- Gắn mã đo tạm trong `C:\x` (đã gỡ): mỗi 5 giây dùng `uxTaskGetSystemState()` lấy phần trăm CPU từng task trong 5 giây vừa qua và số byte stack còn trống ít nhất.
- Đọc thêm heap nội, heap DMA và heap PSRAM.
- Tự kích hoạt hai phiên trò chuyện bằng `WakeWordInvoke` ở giây 40 và 110 (rảnh → kết nối → nghe → máy chủ trả lời → nghe → rảnh).
- Cấu hình thiết bị: PCM5102A (I2S0) + INMP441 (I2S1), AFE bật, WakeNet9, MQTT + UDP, màn 240×320.

### 8.1. Tải CPU (100% = một lõi)

| Trạng thái | Lõi 0 | Lõi 1 (cao nhất) | audio_input | audio_afe | opus_codec | LVGL | main |
|---|---|---|---|---|---|---|---|
| Rảnh | 3,8% | 34% (38%) | 24,7% | 9,4% | 0% | 0,1% | 2,5% |
| Đang nghe | 5,5% | 38% (50%) | 26,5% | 1,2% | 10% | 0% | 3,4% |
| Đang nói (1 mẫu) | 8,7% | 53% | 31% | 2,0% | 14% | 1,1% | 4,3% |

Lõi 1 còn dư gần một nửa, lõi 0 gần như rảnh. Chưa cần `-O2` hay tăng cache (mục 7.3).

### 8.2. SRAM và PSRAM

| Chỉ số | Giá trị |
|---|---|
| Heap nội lúc chạy | 82,6–87,3 KB còn trống |
| Heap nội thấp nhất từ lúc khởi động | 33,5 KB (trong lúc khởi động/kích hoạt) |
| Khối liên tục lớn nhất (nội và DMA) | 43 KB |
| PSRAM còn trống | 555–612 KB trên khoảng 5,2 MB heap PSRAM |

SRAM nội đủ dùng; chưa cần các mục ở 7.2. PSRAM không thừa như tưởng: firmware chạy từ PSRAM chiếm khoảng 3 MB, phần còn lại gần hết cho LVGL (bộ nhớ 1 MB), mô hình AFE/WakeNet và bộ đệm âm thanh. Khi thêm tính năng dùng PSRAM nhiều (ảnh, camera), cần xem lại con số này.

### 8.3. Stack (byte còn trống ít nhất)

| Task | Lõi / ưu tiên | Stack | Còn trống | Đánh giá |
|---|---|---|---|---|
| **mqtt_task** | bất kỳ / 5 | 4 096 | **432** | **Rủi ro tràn stack.** Bắt tay TLS (cổng 8883) chạy trong task này. Kích thước đặt cứng trong component `78__esp-ml307` (`mqtt_config.task.stack_size = 4096`), Kconfig `MQTT_TASK_STACK_SIZE` không có tác dụng. Mặc định của esp-mqtt là 6 144. |
| udp_receive | bất kỳ / 1 | 4 096 | 1 544 | ổn |
| tcpip | bất kỳ / 18 | 3 584 | 1 656 | ổn |
| audio_afe | 1 / 5 | 4 096 (PSRAM) | 1 868 | ổn |
| taskLVGL | 1 / 1 | 7 168 | 2 128 | ổn |
| main | 0 / 10 | 6 584 | 2 660 | ổn |
| esp_timer | 0 / 22 | 4 096 | 2 756 | ổn |
| wifi | 0 / 23 | | 4 328 | ổn |
| opus_codec | 1 / 3 | 24 576 (PSRAM) | 4 492 | ổn (dùng khoảng 20 KB khi nén) |
| audio_input | 1 / 23 | 6 144 | 4 744 | dư khoảng 3 KB |
| audio_output | 1 / 23 | 8 192 | 6 912 | dư khoảng 5 KB |
| sys_evt | 0 / 20 | 10 608 | 8 052 | dư khoảng 6 KB (chưa đo đường cấu hình Wi-Fi/BluFi) |

Có thể thu nhỏ `audio_output`, `audio_input`, `sys_evt` để lấy lại khoảng 10 KB SRAM nội. Chưa làm vì SRAM đang đủ, và các đường chưa đo (cấu hình Wi-Fi, BluFi, codec khác) có thể dùng nhiều stack hơn.

### 8.4. Cảnh báo trong lúc đo
- `Encode queue is full, dropping oldest frame`: 1 khung micro (60 ms) bị bỏ ở đầu một số phiên, ngay lúc chuyển từ nghe sang nói. Lúc đó `opus_codec` đang giải mã loạt gói TTS đầu tiên; khung micro bị bỏ không còn được gửi đi nữa. Không ảnh hưởng chức năng.
- Không có cảnh báo ring buffer AFE, không tràn stack, không reset.

### 8.5. Trước và sau khi đổi `notify_http`
Tải CPU, stack và heap ở hai lần đo giống nhau trong sai số. Đường phát thông báo đẩy (`notify`) không được kích hoạt trong lúc đo, vì cần máy chủ gửi tin `notify`.
