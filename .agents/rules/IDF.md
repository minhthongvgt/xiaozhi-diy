# BỘ QUY CHUẨN MÃ NGUỒN ESP-IDF 6.1 ĐẦY ĐỦ CHO AI AGENT (ANTIGRAVITY)

## 1. MỤC TIÊU VÀ PHẠM VI
*   **Mục đích:** Cung cấp bộ quy chuẩn lập trình C++, cấu trúc hệ thống, driver API, giao thức truyền thông, quản lý đa nhiệm FreeRTOS và quy trình build trên ESP-IDF 6.1 cho dự án Xiaozhi ESP32.
*   **Đối tượng áp dụng:** Các AI Coding Agent (Antigravity, Cursor, Copilot, AutoGPT) và Lập trình viên hệ thống.

---

## 2. QUY CHUẨN LẬP TRÌNH C++ VÀ STRICT TYPE-SAFETY (ESP-IDF 6.1)
*   **Code Style:** Tuân thủ Google C++ Style Guide.
*   **Strict Type-Safety (Bắt buộc):**
    *   Mọi tham số GPIO, UART Port, I2C Port lấy từ Kconfig hoặc số nguyên (int) truyền vào hàm API ESP-IDF bắt buộc phải dùng ép kiểu tĩnh: `static_cast<gpio_num_t>(...)`, `static_cast<uart_port_t>(...)`, `static_cast<i2c_port_t>(...)`.
    *   **Tuyệt đối không** gán số nguyên trực tiếp để tránh lỗi biên dịch `-fpermissive` đặc trưng trên ESP-IDF 6.1.
*   **Xử lý Ngoại lệ C++ (C++ Exceptions):**
    *   **Tuyệt đối không** sử dụng khối `try...catch` hay các hàm ném ngoại lệ (như `std::stoi`) vì ESP-IDF vô hiệu hóa C++ exceptions mặc định (`-fno-exceptions`). Việc sử dụng sẽ gây lỗi biên dịch. Thay vào đó, hãy sử dụng các hàm thay thế an toàn như `std::strtol` và kiểm tra con trỏ lỗi.
*   **Quy tắc kiểm tra tính hợp lệ của chân GPIO (Strict Rule - Bắt buộc):**
    *   **TUYỆT ĐỐI KHÔNG DÙNG THAM SỐ `GPIO_NUM_NC` và `GPIO_NUM_MAX` / `NUM_MAX`** trong toàn bộ mã nguồn (cả trong các biểu thức so sánh kiểm tra chân hợp lệ lẫn tham số hàm/gán macro).
    *   **Bắt buộc dùng macro chuẩn duy nhất của ESP-IDF:** `GPIO_IS_VALID_GPIO(gpio_num)` (cho input/tổng quát) hoặc `GPIO_IS_VALID_OUTPUT_GPIO(gpio_num)` (cho output).
    *   Muốn kiểm tra một chân có được cấu hình / kết nối hợp lệ hay không: Chỉ dùng `if (GPIO_IS_VALID_GPIO(pin))` hoặc `if (!GPIO_IS_VALID_GPIO(pin))`.
    *   Khi chân không được kết nối hoặc không dùng: Chỉ gán `-1` hoặc `(gpio_num_t)-1`.
    *   Dùng `UART_PIN_NO_CHANGE` cho các chân UART không thay đổi cấu hình.
*   **Quy chuẩn định nghĩa struct trong C (C Strict Typedef):**
    *   Trong các tệp C (`.c`) và header C (`.h`), BẮT BUỘC dùng cú pháp:
        ```c
        typedef struct my_type_dev_t {
            ...
        } my_type_dev_t;
        ```
    *   **Tuyệt đối không** chỉ khai báo `struct my_type_dev_t { ... };` rồi dùng `my_type_dev_t *dev` mà không có từ khóa `struct`, sẽ gây lỗi biên dịch `error: unknown type name`.
*   **Error Handling & Logging:**
    *   Bọc tất cả các hàm API trả về `esp_err_t` bằng macro `ESP_ERROR_CHECK(...)`.
    *   Sử dụng hệ thống log tiêu chuẩn của ESP-IDF: `ESP_LOGI`, `ESP_LOGW`, `ESP_LOGE`, `ESP_LOGD` với nhãn `static const char* TAG = "ModuleName";`.
    *   **Tuyệt đối không** dùng `printf` hoặc `std::cout` trong mã nguồn.
*   **Thiết kế Kiến trúc (Open-Closed Principle):**
    *   Mọi bo mạch tùy chỉnh (custom board) phải được tạo trong thư mục `main/boards/<board-name>/`.
    *   **Tuyệt đối không** chỉnh sửa mã nguồn lõi trong thư mục `main/` (ngoại trừ file đăng ký board).

---

## 3. CẤU TRÚC LỚP BO MẠCH (CUSTOM BOARD ARCHITECTURE)
*   **Tệp tin bắt buộc phải có trong** `main/boards/<board-name>/` **:**
    1.  `config.h`: Khai báo các hằng số phần cứng, chân GPIO, tần số mẫu âm thanh, thông số hiển thị.
    2.  `config.json`: Khai báo target chip (VD: `esp32s3`), cấu hình dung lượng flash, file phân vùng, ngôn ngữ mặc định.
    3.  `<board_name>_board.cc`: Mã nguồn triển khai chi tiết cho bo mạch.
*   **Lớp kế thừa và ghi đè phương thức (Virtual Methods):**
    *   Lớp bo mạch phải kế thừa từ `WifiBoard` (hoặc `Board`, `Ml307Board`, `DualNetworkBoard` tùy cấu hình).
    *   Bắt buộc ghi đè các hàm ảo: `GetAudioCodec()`, `GetDisplay()`, `GetBacklight()`.
*   **Đăng ký Macro Board:** Bắt buộc phải có dòng `DECLARE_BOARD(MyCustomBoard);` ở cuối file `.cc`.

---

## 4. CẬP NHẬT THƯ VIỆN VÀ DRIVER API (CHUẨN ESP-IDF 6.1)
*   **Driver I2C Master (** `driver/i2c_master.h` **):**
    *   Bắt buộc sử dụng API mới: struct `i2c_master_bus_config_t` và hàm `i2c_new_master_bus(...)`.
    *   Phải bật cờ điện trở kéo lên nội bộ: `.flags.enable_internal_pullup = 1`.
*   **Driver SPI & Display (** `esp_lcd` **):**
    *   Khởi tạo SPI bus chuẩn bằng `spi_bus_initialize()`.
    *   Khởi tạo Panel IO qua `esp_lcd_new_panel_io_spi()` và Panel driver qua `esp_lcd_new_panel_st7789()` (hoặc driver tương ứng).
    *   Ép kiểu `static_cast<gpio_num_t>` cho tất cả các chân điều khiển: CS_PIN, DC_PIN, SCK_PIN, MOSI_PIN.
*   **Driver UART:**
    *   Khai báo `uart_config_t`, thiết lập đầy đủ: baud rate, data bits, parity, stop bits.
    *   Tuân thủ thứ tự gọi: `uart_param_config()` -> `uart_set_pin()` -> `uart_driver_install()`.
*   **Driver Touch Sensor & Xử lý Cảnh báo Deprecation (-Werror=cpp):**
    *   ESP-IDF 6.1 xem mọi chỉ thị `#warning` là lỗi biên dịch nghiêm ngặt (`-Werror=cpp`).
    *   Khi sử dụng `driver/touch_pad.h`, bắt buộc phải bật cấu hình `CONFIG_TOUCH_SUPPRESS_DEPRECATE_WARN=y` trong `sdkconfig.defaults`.
    *   Trong mã nguồn `.c`, bọc `#pragma GCC diagnostic push` / `#pragma GCC diagnostic ignored "-Wcpp"` / `#pragma GCC diagnostic pop` quanh lệnh include `driver/touch_pad.h`.
*   **Đăng ký MCP Tool (Model Context Protocol):**
    *   Sử dụng cấu trúc: `McpServer::GetInstance().AddTool("tool_name", "description", lambda_callback)`.
    *   Xử lý tham số định dạng JSON bằng thư viện `nlohmann::json`.

---

## 5. QUY CHUẨN ĐA NHIỆM FREERTOS VÀ TASK MANAGEMENT
*   **Multi-Core Pinning (Phân luồng lõi CPU với** `xTaskCreatePinnedToCore` **):**
    *   **Core 0 (** `PRO_CPU` **):** Chỉ dành cho Wi-Fi, Bluetooth, MbedTLS, TCP/IP Stack, và System Tasks.
    *   **Core 1 (** `APP_CPU` **):** Chỉ dành cho Audio Pipeline (AEC/I2S), Display Render, MCP Tools, Application Logic.
*   **Hệ thống phân cấp độ ưu tiên (Priority Hierarchy):**
    *   **Mức 18–20 (Cao nhất):** Real-time Audio Processing (I2S DMA Buffer, AEC/Audio Filter).
    *   **Mức 10–15 (Cao):** Audio Streaming (WebSocket/MQTT), Wi-Fi Provisioning.
    *   **Mức 5–9 (Trung bình):** Display Render (LVGL/LCD), Button Scan, MCP Tool Executor.
    *   **Mức 2–4 (Thấp):** Custom UART Task, Background Sensor Processing, NVS Storage.
    *   **Mức 1 (Tối thiểu):** Idle Task.
*   **Cấp phát Stack & PSRAM:**
    *   **Tuyệt đối KHÔNG** dùng `configMINIMAL_STACK_SIZE` (768 bytes) cho Task viết bằng C++ hoặc có xuất Logging.
    *   UART/GPIO Task đơn giản: Tối thiểu cấp phát 3072 - 4096 bytes.
    *   Audio/Network/JSON/MCP Tasks: Tối thiểu cấp phát 8192 - 16384 bytes.
    *   **Stack > 16KB:** Phải cấp phát trên PSRAM bằng `heap_caps_malloc(..., MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)` và khởi tạo qua `xTaskCreateStaticPinnedToCore`.
*   **Tránh lỗi Task Watchdog Reset (TWDT):**
    *   Mọi vòng lặp vô hạn `while(1)` hoặc `for(;;)` bắt buộc phải chứa ít nhất một hàm Block/Yield: `vTaskDelay(pdMS_TO_TICKS(ms))`, `xQueueReceive(...)`, hoặc `ulTaskNotifyTake(...)`.
*   **Đồng bộ đa nhiệm & An toàn ngắt (ISR Safety):**
    *   Dùng `xQueueCreate` cho mô hình Producer-Consumer.
    *   Dùng `xSemaphoreCreateMutex` để khóa tài nguyên phần cứng dùng chung (I2C/SPI/UART).
    *   Ưu tiên dùng Task Notification (`xTaskNotifyGive` / `ulTaskNotifyTake`) cho giao tiếp 1-1.
    *   Trong các trình phục vụ ngắt (ISR), bắt buộc dùng các API có hậu tố `...FromISR` (vd: `xQueueSendFromISR`, `vTaskNotifyGiveFromISR`) và phải gọi kiểm tra ngữ cảnh với `portYIELD_FROM_ISR(xHigherPriorityTaskWoken)`.

---

## 6. GIAO THỨC TRUYỀN DỮ LIỆU NGOẠI VI
*   **Khung dữ liệu nhị phân (Binary Frame - 4 Byte):**
    *   **Byte 0:** `0xAA` (Start Byte - Byte bắt đầu).
    *   **Byte 1:** Hàng chục (Mã Code / 10).
    *   **Byte 2:** Hàng đơn vị (Mã Code % 10).
    *   **Byte 3:** Checksum XOR (Tính bằng: `Byte 0 ^ Byte 1 ^ Byte 2`).
*   **Khung chuỗi (ASCII/Hex):**
    *   Xuất chuỗi 2 chữ số đi kèm với ký tự ngắt dòng (ví dụ: `"45\n"`).

---

## 7. CẤU HÌNH BUILD SYSTEM VÀ KHẮC PHỤC LỖI
*   **Kconfig (** `main/Kconfig.projbuild` **):** Khai báo bo mạch mới trong block `choice BOARD_TYPE`.
*   **CMake (** `main/CMakeLists.txt` **):** Liên kết `BOARD_TYPE`, cấu hình phông chữ văn bản (`BUILTIN_TEXT_FONT`), phông chữ icon (`BUILTIN_ICON_FONT`), và thư viện emoji (`DEFAULT_EMOJI_COLLECTION`).
*   **Cấu trúc Phân vùng Flash 16MB OTA (** `partitions/16m.csv`, bản sao gốc `partitions.csv` **):**
    *   `nvs` 0x9000 (64KB), `otadata` 0x19000 (8KB), `phy_init` 0x1B000 (4KB).
    *   `app0` 0x20000 (4.5MB), `app1` 0x4A0000 (4.5MB).
    *   `model` 0x920000 (0x6E0000 = 6.9MB, type data/spiffs): Tên phân vùng **bắt buộc** là "model" (không dùng "assets"). Chứa chung Voice Models (Wake-word/VAD) VÀ Assets (UI, Fonts, Emoji, Backgrounds) đóng gói trong `generated_assets.bin`.
    *   Không còn phân vùng `storage`. Chi tiết và lý do ở `partitions/README.md`.
    *   **Lưu ý Xung đột Flash:** Thư viện `esp-sr` mặc định cố gắng tự flash `srmodels.bin` vào phân vùng `model`. Để tránh lỗi `Overlap at address`, dự án cần khóa lệnh `esptool_py_flash_to_partition` trong `managed_components/espressif__esp-sr/CMakeLists.txt` hoặc sử dụng cơ chế ghi đè thông minh trong `main/CMakeLists.txt`.
*   **Sửa Lỗi Biên Dịch trên Windows:**
    *   Thiết lập biến môi trường để tránh lỗi mã hóa ký tự:
        *   `set PYTHONUTF8=1`
        *   `set PYTHONIOENCODING=utf-8`
    *   Quy trình xóa sạch cache và build lại khi đổi cấu hình:
        ```bash
        idf.py fullclean
        rm -rf managed_components dependencies.lock sdkconfig
        idf.py set-target esp32s3
        idf.py build
        ```

---

## 8. QUY CHUẨN AUDIO I2S, KHẮC PHỤC TRIỆT ĐỂ LỖI VỠ TIẾNG VÀ ÂM THANH RÈ RẸC
*   **Hiện tượng 1: Vỡ tiếng (Digital Clipping / Saturation):**
    *   **Nguyên nhân:** Âm thanh số từ Opus/TTS ở mức đỉnh gần 0 dBFS khi nhân với `volume_factor` vượt ngưỡng biên độ 16-bit/32-bit bị xén đỉnh (hard clipping). Đồng thời, các chip DAC/Amply rời như MAX98357A có độ lợi phần cứng mặc định lên tới **+15dB** (khi chân GAIN thả nổi), khiến tín hiệu full-scale đập mạnh vào trần nguồn 5V/3.3V gây bão hòa tầng công suất Class-D.
    *   **Quy tắc:**
        1. Áp dụng hệ số Headroom an toàn `0.85` (~-1.4 dB) trong thuật toán nhân `volume_factor` để bù trừ hiện tượng vọt biên (overshoot) do bộ lọc resampler sinh ra.
        2. Kẹp biên thể tích `output_volume_` nghiêm ngặt trong khoảng `[0, 100]`.
        3. Khuyến nghị phần cứng: Nối chân GAIN của MAX98357A xuống GND (9dB) hoặc nối qua điện trở 100kΩ xuống GND (6dB) thay vì thả nổi để âm thanh không bị xé/vỡ khi loa công suất nhỏ hoạt động.
*   **Hiện tượng 2: Âm thanh rè rẹc, nổ lẹt đẹt (DMA Underrun / Slot Mismatch):**
    *   **Nguyên nhân 1 (DMA Buffer Underrun):** Số lượng và độ dài DMA descriptor quá ngắn (chỉ 6 desc x 240 frame = 60ms). Khi Wi-Fi hoặc FreeRTOS chiếm dụng CPU xử lý gói tin, DMA cạn buffer dẫn đến sụt áp về 0 đột ngột sinh ra tiếng "tách", "lẹt đẹt", rè rẹc liên tục.
    *   **Quy tắc DMA:** Bắt buộc duy trì buffer DMA tối thiểu 160ms - 240ms:
        ```c
        #define AUDIO_CODEC_DMA_DESC_NUM 8
        #define AUDIO_CODEC_DMA_FRAME_NUM 480
        ```
    *   **Nguyên nhân 2 (Queue Starvation):** Hàng đợi `MAX_PLAYBACK_TASKS_IN_QUEUE` quá nông (= 2 task = 120ms).
    *   **Quy tắc Queue:** Đặt `MAX_PLAYBACK_TASKS_IN_QUEUE = 4` (240ms PCM data) để hấp thụ biến thiên mạng và độ trễ giải mã Opus.
    *   **Nguyên nhân 3 (I2S Slot Mask lệch kênh):**
        *   Cấu hình `.slot_mask = I2S_STD_SLOT_LEFT` chỉ phát dữ liệu ở kênh Trái, kênh Phải để trống. Các module MAX98357A ở chế độ Mono Mix ($(L+R)/2$) hoặc Stereo DAC (PCM5102A) sẽ bị mất cân bằng, lệch xung WS và sinh ra nhiễu rè.
        *   **Quy tắc Slot:** Đối với kênh phát loa Mono trong chế độ Philips, BẮT BUỘC sử dụng `.slot_mask = I2S_STD_SLOT_BOTH` để phần cứng I2S tự động nhân bản mẫu âm thanh sang cả 2 kênh Trái và Phải, đảm bảo xung WS đối xứng 50% chuẩn mực.
*   **Yêu cầu Nguồn điện Phần cứng (Hardware Power Rule):**
    *   DAC/Amply công suất (MAX98357A, NS4168, PAM8302) BẮT BUỘC cấp nguồn 5V riêng trực tiếp từ cổng USB / nguồn ngoài, TUYỆT ĐỐI KHÔNG lấy từ chân 3.3V của ESP32.
    *   Bắt buộc gắn thêm 1 tụ hóa/gốm 10µF - 100µF song song tụ 0.1µF sát chân VIN và GND của module âm thanh để triệt tiêu sụt áp tức thời.

---

## 9. QUY CHUẨN XỬ LÝ CẢNH BÁO COMPILER WARNING (-Wunused-function, -Wunused-variable)
Trong kiến trúc hệ thống nhúng đa bo mạch (Multi-Board) và Kconfig linh hoạt của Xiaozhi:
*   **Nguyên nhân phát sinh cảnh báo:**
    *   Một hàm nội bộ tĩnh (`static`) hoặc biến tĩnh được định nghĩa cho một cấu hình phần cứng hoặc bo mạch cụ thể, nhưng khi người dùng chọn bo mạch khác (ví dụ: bo mạch C++ `ESP32-S3-N16R8-CUSTOM` tự quản lý Button/Backlight) hoặc khi tắt tính năng trong Kconfig, hàm/biến tĩnh đó không được gọi trong file mã nguồn.
    *   Trình biên dịch GCC của ESP-IDF kích hoạt cờ `-Wall -Wextra`, phát sinh cảnh báo dừng build hoặc làm ô nhiễm log biên dịch (`-Wunused-function`, `-Wunused-variable`, `-Wunused-but-set-variable`).
*   **Quy tắc Chuẩn Hóa Bắt Buộc (Tuyệt đối không bỏ qua):**
    1. **Bọc điều kiện `#if / #endif` tương ứng:** Nếu hàm tĩnh chỉ phục vụ cho một nhánh cấu hình nhất định (ví dụ: `#if !defined(...)`), định nghĩa hàm PHẢI được bọc đúng điều kiện `#if` đó.
    2. **Đánh dấu thuộc tính `__attribute__((unused))` (Phòng thủ kép):** Mọi hàm tĩnh trợ thủ (helper/isr) hoặc biến tĩnh có khả năng không được gọi trong một số tổ hợp Kconfig BẮT BUỘC phải được khai báo với thuộc tính chuẩn GNU:
       `static __attribute__((unused)) esp_err_t func_name(...)`
       `static __attribute__((unused)) type_t var_name = ...;`
    3. **Hiệu quả:**
       * Khi tính năng/bo mạch kích hoạt: Hàm được biên dịch và gọi bình thường với hiệu năng tối đa.
       * Khi tính năng/bo mạch tắt: Trình liên kết (Linker) tự động loại bỏ mã thừa qua cơ chế Dead Code Elimination (`-Wl,--gc-sections`), giải phóng 100% dung lượng Flash/IRAM và **TRIỆT TIÊU 100% CẢNH BÁO COMPILER**.

---

## 10. QUY CHUẨN MÃ NGUỒN SẠCH & TUYỆT ĐỐI KHÔNG SINH COMMENT (CHÚ THÍCH) TRONG CODE
Nhằm duy trì codebase tinh gọn, chuyên nghiệp và tối ưu hóa xử lý bộ nhớ ngữ cảnh của AI:
*   **Nguyên tắc cốt lõi (Strict No-Comment Rule):**
    *   Khi AI Agent viết mới, sinh mã vá lỗi hoặc chỉnh sửa tệp tin trong toàn bộ dự án (C, C++, CMakeLists.txt, Python, JavaScript, HTML, CSS...):
    *   **TUYỆT ĐỐI 100% KHÔNG ĐƯỢC CHÈN BẤT KỲ COMMENT/CHÚ THÍCH NÀO TRONG MÃ NGUỒN** (kể cả chú thích một dòng `//`, `#`, chú thích khối `/* ... */`, hay chú thích tài liệu `<!-- ... -->`).
*   **Mã nguồn tự diễn giải (Self-Documenting Code):**
    *   Tất cả ý nghĩa của mã nguồn phải được thể hiện tường minh qua:
        1. Tên hàm (function name) và tên biến (variable name) rõ nghĩa, đúng ngữ cảnh chuẩn tiếng Anh kỹ thuật.
        2. Cấu trúc hàm đơn nhiệm (Single Responsibility), ngắn gọn, logic chặt chẽ.
        3. Strict Type-safety và hằng số macro/enum chuẩn xác thay vì dùng số ma thuật (magic numbers).
*   **Tách biệt giữa Code và Giải thích:**
    *   Mọi phân tích kỹ thuật, diễn giải thuật toán, lý do sửa đổi và hướng dẫn sử dụng PHẢI được trình bày trực tiếp trong câu trả lời (chat response) hoặc tài liệu nghiệm thu (artifact markdown) gửi tới người dùng.
    *   **TUYỆT ĐỐI KHÔNG ghi chú thích vào bên trong tệp mã nguồn.**