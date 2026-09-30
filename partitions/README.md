# Version 2 Partition Table

Bảng phân vùng chuẩn hóa cho ESP32-S3 N16R8 (16MB Flash).

## Quy Ước Đặt Tên (NAMING CONVENTION)

> **Tên phân vùng chuẩn (canonical name): `model`**
>
> Tên `model` được sử dụng **nhất quán** trong toàn bộ dự án:
> - Bảng phân vùng CSV (`partitions.csv`, `partitions/v2/16m.csv`)
> - ESP-SR API: `esp_srmodel_init("model")`
> - Assets loader: `esp_partition_find_first(... "model")`
> - CMakeLists.txt: `partition_table_get_partition_info("model")`
>
> **KHÔNG dùng** tên `assets` cho phân vùng Flash. Tên `assets` chỉ được dùng làm NVS namespace trong `Settings("assets", true)`.

## Partition Layout (16MB Flash — ESP32-S3 N16R8)

| Name       | Type | SubType | Offset     | Size       | Size (KB) | Mục đích                                       |
|------------|------|---------|------------|------------|-----------|------------------------------------------------|
| `nvs`      | data | nvs     | `0x9000`   | `0x10000`  | 64 KB     | Non-Volatile Storage (config, Wi-Fi, settings) |
| `otadata`  | data | ota     | `0x19000`  | `0x2000`   | 8 KB      | OTA boot state tracking                        |
| `phy_init` | data | phy     | `0x1B000`  | `0x1000`   | 4 KB      | PHY calibration data (Wi-Fi/BT)                |
| `app0`     | app  | ota_0   | `0x20000`  | `0x480000` | 4608 KB   | Application firmware (OTA slot 0)              |
| `app1`     | app  | ota_1   | `0x4A0000` | `0x480000` | 4608 KB   | Application firmware (OTA slot 1)              |
| `model`    | data | spiffs  | `0x920000` | `0x6E0000` | 7040 KB   | AI models, fonts, themes, audio assets         |

**Tổng:** 16MB (0x1000000) — sử dụng 100% dung lượng Flash.

### Chi tiết phân vùng `model` (6.9MB)

Phân vùng `model` lưu trữ tất cả nội dung có thể tải qua mạng:
- **Wake word models**: Mô hình nhận diện giọng nói WakeNet (ESP-SR)
- **Theme files**: Fonts, biểu tượng, hình nền, âm thanh UI
- **Audio effects**: Hiệu ứng âm thanh, nhạc chuông
- **Language packs**: File cấu hình ngôn ngữ
- **Emoji packs**: Bộ emoji tùy chỉnh

### Gap 16KB (0x1C000 → 0x20000)

Vùng 16KB giữa `phy_init` và `app0` là **ESP-IDF reserved** — dùng cho partition table metadata. Đây là hành vi chuẩn, không phải lãng phí.

## Thay Đổi So Với Phiên Bản Trước

### Removed: Phân vùng `storage`
- Phân vùng `storage` (4992KB, SubType SPIFFS) đã bị **loại bỏ** vì:
  - Không có code nào mount hoặc sử dụng phân vùng này
  - Không có `esp_vfs_spiffs_register()` hay `littlefs` init nào trỏ đến nó
  - Dung lượng ~4.8MB bị lãng phí hoàn toàn
- Toàn bộ dung lượng đã được dồn vào phân vùng `model` (2MB → 6.9MB)

### Standardized: Tên phân vùng
- Tên chuẩn duy nhất: `model`
- Loại bỏ fallback `"assets"` trong `assets.cc` và `CMakeLists.txt`
- README và code sử dụng cùng một tên

## Technical Details

- **Partition Type**: `data` / SubType `spiffs` — tương thích với cả SPIFFS và memory-mapping
- **Memory Mapping**: Nội dung được mmap qua `esp_partition_mmap()` trong runtime
- **Checksum**: ESP-IDF tự kiểm tra integrity khi flash
- **Progressive Download**: Assets có thể tải OTA với progress tracking
- **Fallback**: Graceful fallback về built-in assets nếu download thất bại

## Lưu Ý Quan Trọng

1. **Flash lại partition table** khi chuyển từ layout cũ (có `storage`) sang layout mới
2. Sau khi flash partition table mới, **erase toàn bộ flash** (`idf.py erase-flash`) để tránh dữ liệu rác từ `storage` cũ
3. Tên phân vùng `model` tối đa **15 ký tự** (giới hạn ESP-IDF)
4. Tất cả phân vùng đều **4KB aligned** để tối ưu hiệu năng Flash