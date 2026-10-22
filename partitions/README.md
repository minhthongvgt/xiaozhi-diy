# Bảng phân vùng 16 MB

`partitions/16m.csv` (bản dùng) và `partitions.csv` ở thư mục gốc (bản sao). Dùng chung cho ESP32-S3 N16R8 và các bo ESP32-P4.

| Tên | Type | SubType | Offset | Kích thước | Nội dung |
|---|---|---|---|---|---|
| `nvs` | data | nvs | `0x9000` | `0x10000` (64 KB) | Cấu hình, Wi-Fi, cài đặt |
| `otadata` | data | ota | `0x19000` | `0x2000` (8 KB) | Trạng thái OTA |
| `phy_init` | data | phy | `0x1B000` | `0x1000` (4 KB) | Hiệu chuẩn RF |
| `app0` | app | ota_0 | `0x20000` | `0x480000` (4,5 MB) | Firmware, khe OTA 0 |
| `app1` | app | ota_1 | `0x4A0000` | `0x480000` (4,5 MB) | Firmware, khe OTA 1 |
| `model` | data | spiffs | `0x920000` | `0x6E0000` (6,875 MB) | `generated_assets.bin`: mô hình WakeNet/MultiNet, font, emoji, giao diện |

Vùng `0x1C000`–`0x20000` để trống do `app0` phải căn lề 64 KB.

## Phân vùng `model`

- Tên chuẩn duy nhất là `model`: `esp_srmodel_init("model")`, `esp_partition_find_first(... "model")`, `partition_table_get_partition_info("model")`. Tên `assets` chỉ là NVS namespace (`Settings("assets", true)`).
- Nội dung được ánh xạ bằng `esp_partition_mmap()` lúc chạy; tải qua mạng được, lỗi thì dùng tài nguyên có sẵn trong firmware.
- `esp-sr` mặc định tự nạp `srmodels.bin` vào phân vùng này, trùng với `generated_assets.bin` (lỗi `Overlap at address`). `main/project_include.cmake` chặn lệnh nạp đó, xem [docs/BUILD_NOTES.md](../docs/BUILD_NOTES.md).

Khi đổi bảng phân vùng trên thiết bị đã nạp bảng khác, chạy `idf.py erase-flash` trước khi nạp lại.
