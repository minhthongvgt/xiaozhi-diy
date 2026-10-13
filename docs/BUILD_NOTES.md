# Ghi chú hệ thống build

Dự án không để comment trong mã nguồn (kể cả CMake, Kconfig, `idf_component.yml`). Những lý do thiết kế trước đây nằm trong comment được gom về đây.

## CMakeLists.txt (gốc dự án)

| Thiết lập | Lý do |
| --- | --- |
| `set(ENV{PYTHONUTF8} "1")`, `set(ENV{PYTHONIOENCODING} "utf-8")` | Các script Python chạy lúc cấu hình (sinh assets, ngôn ngữ) đọc/ghi tiếng Việt; trên Windows mặc định không phải UTF-8. |
| `add_compile_options(-Wno-missing-field-initializers)` | Code khởi tạo struct cấu hình ESP-IDF theo kiểu designated initializer một phần; ESP-IDF 6 coi cảnh báo là lỗi. |
| `idf_build_set_property(MINIMAL_BUILD ON)` | Chỉ build `main` và các component nó phụ thuộc, giảm thời gian build. |
| `target_compile_options(espressif__esp-dsp PRIVATE -O1)` | GCC của Xtensa bị lỗi nội bộ (ICE, `RTL pass: ira`, segmentation fault) khi biên dịch `dspi_conv_f32_ansi.c`; hạ tối ưu của riêng component `esp-dsp` xuống `-O1` để tránh. |
| `PROJECT_VER` | Phiên bản firmware báo lên server OTA. |

## Phân vùng `model` và esp-sr (`main/project_include.cmake`)

`esp-sr` mặc định tự tạo target `srmodels_bin` và nạp `srmodels.bin` vào phân vùng `model` (0x920000). Dự án đã đóng gói `srmodels.bin` cùng font và emoji vào một file `generated_assets.bin` (`scripts/build_default_assets.py`) và nạp vào chính phân vùng đó, nên hai lệnh nạp chồng lên nhau (lỗi `Overlap at address`). Sửa trong `managed_components/` không bền vì Component Manager sẽ khôi phục lại. Vì vậy `main/project_include.cmake` ghi đè hai hàm của ESP-IDF:

1. `partition_table_get_partition_info`: khi `esp-sr` hỏi thông tin phân vùng `model` thì trả về rỗng, nên `esp-sr` không tạo target `srmodels_bin` và không chạy `movemodel.py`. Các truy vấn khác chạy như bản gốc của ESP-IDF.
2. `esptool_py_flash_to_partition`: bỏ qua mọi lệnh nạp `srmodels*.bin` vào phân vùng `model`.

Kiểm tra: khi cấu hình, log có dòng `[Xiaozhi] Suppressed esp-sr model partition query`; `build/flash_args` chỉ có `generated_assets.bin` ở `0x920000`.

Trước 2026-10-13, `CMakeLists.txt` gốc có một bản sao y hệt của hai hàm này nhưng không bao giờ có tác dụng (bản trong `project_include.cmake` được nạp sau và đè lên), nên đã bị xoá.

## main/CMakeLists.txt

| Thiết lập | Lý do |
| --- | --- |
| Nhánh `BOARD_TYPE_*` → `BOARD_DIR` | Gắn symbol Kconfig của board với thư mục mã nguồn; `scripts/build.py` đọc nhánh này để kiểm tra và chuẩn hoá cấu hình board. |
| `BOARD_TYPE`, `BOARD_MANUFACTURER` | Firmware báo lên server. Lấy từ `config.json` của board; thiếu thì suy ra từ tên thư mục board. `BOARD_NAME` rỗng thì dùng `BOARD_TYPE`; chỉ cho phép chữ thường, số, `.` và `-`. |
| Một audio engine cho mỗi họ chip | S3/P4: `AfeAudioEngine` + `CustomWakeWord`; chip khác: `LiteAudioEngine` + `EspWakeWord`. |
| Âm thanh ngôn ngữ dự phòng | Nếu ngôn ngữ đang chọn thiếu file `.ogg` nào thì lấy bản `en-US` cùng tên. |
| `-std=gnu++23` cho C++ của `main` | ESP-IDF 6 có thể đổi chuẩn C++ mặc định; cố định để code của dự án không phụ thuộc vào đó. |
| `LV_MEM_POOL_INCLUDE` cho LVGL trên ESP32-S3 | Board có PSRAM cấp bộ nhớ LVGL từ PSRAM (`lvgl_psram_pool.h`). Board `CONFIG_SPIRAM=n` không được giữ pool 1 MB mặc định trong `.dram0.bss` (`lvgl_no_psram_pool.h`); `scripts/build.py` chuyển các biến thể đó về `CLIB malloc`. |
| `--max_size` cho `build_default_assets.py` | `assets.bin` phải vừa phân vùng `model` (kích thước lấy từ bảng phân vùng). |
| Tải assets từ URL | Tải vào file tạm rồi đổi tên (thao tác nguyên tử); lỗi thì xoá file tạm. |

## main/idf_component.yml

| Mục | Lý do |
| --- | --- |
| `espressif/esp_audio_effects: ~1.3.0` | Giữ dưới mốc yêu cầu P4 rev ≥ 3.0 của bản mới hơn, cho tới khi bỏ hỗ trợ board P4 rev 1.x. |
| `rules: - if: ...` | Các điều kiện trong cùng một `rules` được kết hợp theo AND. |

Đã xoá ngày 2026-10-13 vì không bao giờ được dùng: `cube32esp/xpowerslib` (chỉ phục vụ `xpower.h` không ai include); `espressif/touch_slider_sensor` và `espressif/touch_button_sensor` (chỉ được tải khi `idf_version < 6.0.0`, mà dự án yêu cầu ≥ 6.0.1). File `idf_component.yml` ở gốc dự án cũng đã xoá: ESP-IDF chỉ đọc manifest của component (`main/idf_component.yml`); xoá file gốc không làm đổi `dependencies.lock`.

## scripts/build_default_assets.py (sinh `generated_assets.bin`)

- **Font chữ:** firmware tự nhúng bản `font_noto_sans_*_basic_*`; bản `*_common_*` tương ứng (nhiều ký tự hơn) được đóng vào assets và nạp lúc chạy. Bản `common` dùng mô tả ký tự loại lớn, nên firmware phải bật `CONFIG_LV_FONT_FMT_TXT_LARGE=y` (xem `.agents/rules/LOI_THUONG_GAP.md` mục C1).
- **Emoji:** ưu tiên bộ PNG, sau đó tới bộ GIF (ví dụ `noto-emoji_64`); `otto-gif` được xử lý riêng. Emoji PNG cần `CONFIG_LV_USE_LODEPNG=y`.
- **Chọn model giọng nói để đóng gói (theo `sdkconfig`):**
  1. Model WakeNet chỉ đóng gói khi `USE_ESP_WAKE_WORD=y` hoặc `USE_AFE_WAKE_WORD=y`.
  2. `USE_CUSTOM_WAKE_WORD=y` mà không chọn model MultiNet (`CONFIG_SR_MN_*`) thì dừng build với thông báo lỗi.
  3. Model MultiNet chỉ đóng gói khi `USE_CUSTOM_WAKE_WORD=y`; ngôn ngữ (cn/en) suy ra từ tên model, mặc định cn.
- **WakeNet10:** gói model có nhiều lát cắt theo chip. Runtime của ESP-SR tìm `wn10_data` / `_MODEL_INFO_`, nên lát cắt không dùng (p1/p2) bị bỏ và lát cắt được chọn đổi tên trước khi đóng `srmodels.bin`.
- **Cấu trúc `srmodels.bin`:** `uint32` số model; với mỗi model: tên 32 byte, `uint32` số file; với mỗi file: tên 32 byte, `uint32` offset, `uint32` độ dài; sau phần đầu là dữ liệu các file.
- **`assets.bin`:** dữ liệu gộp được thêm tiền tố `0x5A5A`; kích thước tối đa bằng phân vùng `model` (`--max_size`). Không có gì để đóng gói thì tạo file rỗng.
- **Biến môi trường:** script tự ép UTF-8 cho stdout/stderr để chạy được trên console Windows.

## Bảng phân vùng (`partitions.csv`, `partitions/16m.csv`)

Thứ tự cột: `Name, Type, SubType, Offset, Size, Flags`. Bố cục và lý do: `partitions/README.md`.
