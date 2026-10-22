# Bộ nhớ, DMA và task trên ESP32-S3 N16R8

Số đo trên thiết bị thật: PCM5102A (I2S0) + INMP441 (I2S1), AFE bật, WakeNet9, MQTT + UDP, màn hình 240×320.

## Các cấp bộ nhớ

| Cấp | Dung lượng | DMA | Dùng cho |
|---|---|---|---|
| Cache lệnh / dữ liệu | 32 KB / 32 KB (dòng 64 B) | không | mã và dữ liệu đọc từ PSRAM/flash |
| SRAM nội (DIRAM) | 341 760 B | có | stack task, ISR, bộ đệm DMA, Wi-Fi, mã IRAM |
| RTC FAST / SLOW | 8 KB + 8 KB | không | dữ liệu giữ qua deep sleep |
| PSRAM Octal | 8 MB, 80 MHz | hạn chế | heap lớn, LVGL, AFE, Opus, mã chương trình |
| Flash QIO | 16 MB, 80 MHz | không | ảnh firmware, NVS, phân vùng `model` |

`SPIRAM_FETCH_INSTRUCTIONS` và `SPIRAM_RODATA` bật: mã và hằng số chạy từ PSRAM (khoảng 3 MB). Cache không bị tắt khi ghi flash (NVS, OTA, assets).

Cấp phát:
- `SPIRAM_MALLOC_RESERVE_INTERNAL=102400`: giữ 100 KB SRAM nội cho cấp phát yêu cầu SRAM nội hoặc DMA.
- `SPIRAM_MALLOC_ALWAYSINTERNAL=1024`: `malloc` đến 1 KB vào SRAM nội, lớn hơn vào PSRAM.

## SRAM nội

| Phần (`idf.py size`) | Byte |
|---|---|
| `.text` (IRAM) | 113 195 |
| `.data` | 36 584 |
| `.bss` | 32 152 |
| Còn cho heap lúc khởi động | 158 801 |

Chiếm nhiều nhất: Wi-Fi 40 587 B (`ESP_WIFI_IRAM_OPT`, `ESP_WIFI_RX_IRAM_OPT`), `libspi_flash` 20 099 B, `libesp_hw_support` 13 613 B, `libdl_lib` (esp-sr) 11 007 B, `libfreertos` 10 243 B.

| Lúc chạy | Giá trị |
|---|---|
| Heap nội còn trống | 82,6–87,3 KB |
| Heap nội thấp nhất từ lúc khởi động | 33,5 KB |
| Khối liên tục lớn nhất (nội, DMA) | 43 KB |
| PSRAM còn trống | 555–612 KB trên khoảng 5,2 MB heap PSRAM |

PSRAM còn lại không nhiều: LVGL (1 MB), mô hình AFE/WakeNet và bộ đệm âm thanh dùng gần hết.

## DMA

| Ngoại vi | Bộ đệm | Vị trí |
|---|---|---|
| I2S phát / thu | 8 desc × 480 khung mỗi chiều (240 ms ở 16 kHz) | SRAM nội |
| SPI LCD | `width × 20` dòng, một bộ đệm | SRAM nội |
| RGB LCD (ST7701) | `double_buffer = true` | |
| Camera | khung ảnh, DMA tối đa 32 KB một lần | PSRAM (`CAMERA_PSRAM_DMA=y`) |

Descriptor DMA phải ở SRAM nội và băng thông DMA vào PSRAM hạn chế, nên bộ đệm âm thanh và màn hình giữ ở SRAM nội. Bộ đệm I2S 240 ms chống rè khi Wi-Fi chiếm CPU, đổi lại thêm khoảng 240 ms độ trễ phát; `soft_aec_reference.cc` tự bù độ lệch cho AEC.

## Task

`configMAX_PRIORITIES = 25`. Task hệ thống: IPC 24; Wi-Fi 23, esp_timer 22, `sys_evt` 20 (lõi 0); lwIP `tcpip` 18 (không ghim lõi).

| Task | Lõi | Ưu tiên | Stack | Còn trống ít nhất | Vị trí stack |
|---|---|---|---|---|---|
| audio_input | 1 | 23 | 6 144 (AFE) / 4 096 | 4 744 | SRAM nội |
| audio_output | 1 | 23 | 8 192 | 6 912 | SRAM nội |
| audio_afe (WakeNet, VAD) | 1 | 5 | 4 096 | 1 868 | PSRAM |
| encode_wake_word | 1 | 4 | 24 576 / 28 672 | | PSRAM |
| opus_codec | 1 | 3 | 24 576 | 4 492 | PSRAM |
| main (`Application::Run`) | 0 | 10 | 6 584 | 2 660 | SRAM nội |
| notify_http | 0 | 4 | 6 144 | | SRAM nội |
| activation | 1 | 2 | 8 192 | | SRAM nội |
| mqtt_task | — | 5 | 4 096 | **432** | SRAM nội |
| udp_receive / ssl_receive / tcp_receive | — | 1 | 4 096 | 1 544 | SRAM nội |
| taskLVGL | 1 | 1 | 7 168 | 2 128 | SRAM nội |
| LedEvent | 1 | 2 | 4 096 | | SRAM nội |
| uart_code_rx, bt_uart_rx | 1 | 3 | 4 096 | | SRAM nội |
| uart_disp_rx, periph_touch | — | 3 | 4 096 / 3 072 | | SRAM nội |
| tcpip | — | 18 | 3 584 | 1 656 | |
| esp_timer | 0 | 22 | 4 096 | 2 756 | |
| sys_evt | 0 | 20 | 10 608 | 8 052 | |

- esp-sr không tạo task AFE riêng với cấu hình này; AEC chạy trong `audio_input`.
- `mqtt_task`: stack 4 096 đặt cứng trong component `78__esp-ml307` (`MQTT_TASK_STACK_SIZE` không có tác dụng), bắt tay TLS chạy trong task này, chỉ còn 432 byte trống. Mặc định của esp-mqtt là 6 144.
- Nguyên tắc ưu tiên: âm thanh thời gian thực 19–23 chỉ ở lõi 1 và không làm mạng; task có TCP/IP dưới 18; LVGL thấp hơn chuỗi âm thanh.

## Tải CPU (100 % = một lõi)

| Trạng thái | Lõi 0 | Lõi 1 | audio_input | audio_afe | opus_codec | LVGL | main |
|---|---|---|---|---|---|---|---|
| Rảnh | 3,8 % | 34 % | 24,7 % | 9,4 % | 0 % | 0,1 % | 2,5 % |
| Đang nghe | 5,5 % | 38 % | 26,5 % | 1,2 % | 10 % | 0 % | 3,4 % |
| Đang nói | 8,7 % | 53 % | 31 % | 2,0 % | 14 % | 1,1 % | 4,3 % |

Lúc chuyển từ nghe sang nói, đôi khi log có `Encode queue is full, dropping oldest frame` (bỏ 1 khung micro 60 ms) vì `opus_codec` đang giải mã loạt gói TTS đầu tiên; không ảnh hưởng chức năng.
