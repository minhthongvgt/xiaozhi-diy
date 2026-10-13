# Kế hoạch Đồng bộ Menuconfig sang Web UI

Danh sách này chứa tất cả các cấu hình Kconfig sẽ được ánh xạ sang Web UI. Các mục này sẽ ghi thẳng vào file `sdkconfig` để build.

> Tệp tự sinh bằng `scripts/gen_kconfig_webui_doc.py` từ cây menu của `main/Kconfig.projbuild`. Không sửa tay.

| Tên Kconfig (sdkconfig) | Kiểu dữ liệu | Danh mục (Menu) | Mô tả hiển thị (Prompt) | Giá trị mặc định |
| --- | --- | --- | --- | --- |
| `CONFIG_OTA_URL` | string | Xiaozhi Assistant | Default OTA URL | "https://api.tenclass.net/xiaozhi/ota/" |
| `CONFIG_LANGUAGE_ZH_CN` | bool | Xiaozhi Assistant | Chinese |  |
| `CONFIG_LANGUAGE_ZH_TW` | bool | Xiaozhi Assistant | Chinese Traditional |  |
| `CONFIG_LANGUAGE_EN_US` | bool | Xiaozhi Assistant | English |  |
| `CONFIG_LANGUAGE_JA_JP` | bool | Xiaozhi Assistant | Japanese |  |
| `CONFIG_LANGUAGE_KO_KR` | bool | Xiaozhi Assistant | Korean |  |
| `CONFIG_LANGUAGE_VI_VN` | bool | Xiaozhi Assistant | Vietnamese | y (mặc định của choice) |
| `CONFIG_LANGUAGE_TH_TH` | bool | Xiaozhi Assistant | Thai |  |
| `CONFIG_LANGUAGE_DE_DE` | bool | Xiaozhi Assistant | German |  |
| `CONFIG_LANGUAGE_FR_FR` | bool | Xiaozhi Assistant | French |  |
| `CONFIG_LANGUAGE_ES_ES` | bool | Xiaozhi Assistant | Spanish |  |
| `CONFIG_LANGUAGE_IT_IT` | bool | Xiaozhi Assistant | Italian |  |
| `CONFIG_LANGUAGE_RU_RU` | bool | Xiaozhi Assistant | Russian |  |
| `CONFIG_LANGUAGE_AR_SA` | bool | Xiaozhi Assistant | Arabic |  |
| `CONFIG_LANGUAGE_HI_IN` | bool | Xiaozhi Assistant | Hindi |  |
| `CONFIG_LANGUAGE_MR_IN` | bool | Xiaozhi Assistant | Marathi |  |
| `CONFIG_LANGUAGE_PT_PT` | bool | Xiaozhi Assistant | Portuguese |  |
| `CONFIG_LANGUAGE_PT_BR` | bool | Xiaozhi Assistant | Portuguese (Brazil) |  |
| `CONFIG_LANGUAGE_PL_PL` | bool | Xiaozhi Assistant | Polish |  |
| `CONFIG_LANGUAGE_CS_CZ` | bool | Xiaozhi Assistant | Czech |  |
| `CONFIG_LANGUAGE_FI_FI` | bool | Xiaozhi Assistant | Finnish |  |
| `CONFIG_LANGUAGE_TR_TR` | bool | Xiaozhi Assistant | Turkish |  |
| `CONFIG_LANGUAGE_ID_ID` | bool | Xiaozhi Assistant | Indonesian |  |
| `CONFIG_LANGUAGE_UK_UA` | bool | Xiaozhi Assistant | Ukrainian |  |
| `CONFIG_LANGUAGE_RO_RO` | bool | Xiaozhi Assistant | Romanian |  |
| `CONFIG_LANGUAGE_BG_BG` | bool | Xiaozhi Assistant | Bulgarian |  |
| `CONFIG_LANGUAGE_CA_ES` | bool | Xiaozhi Assistant | Catalan |  |
| `CONFIG_LANGUAGE_DA_DK` | bool | Xiaozhi Assistant | Danish |  |
| `CONFIG_LANGUAGE_EL_GR` | bool | Xiaozhi Assistant | Greek |  |
| `CONFIG_LANGUAGE_FA_IR` | bool | Xiaozhi Assistant | Persian |  |
| `CONFIG_LANGUAGE_FIL_PH` | bool | Xiaozhi Assistant | Filipino |  |
| `CONFIG_LANGUAGE_HE_IL` | bool | Xiaozhi Assistant | Hebrew |  |
| `CONFIG_LANGUAGE_HR_HR` | bool | Xiaozhi Assistant | Croatian |  |
| `CONFIG_LANGUAGE_HU_HU` | bool | Xiaozhi Assistant | Hungarian |  |
| `CONFIG_LANGUAGE_MS_MY` | bool | Xiaozhi Assistant | Malay |  |
| `CONFIG_LANGUAGE_NB_NO` | bool | Xiaozhi Assistant | Norwegian |  |
| `CONFIG_LANGUAGE_NL_NL` | bool | Xiaozhi Assistant | Dutch |  |
| `CONFIG_LANGUAGE_SK_SK` | bool | Xiaozhi Assistant | Slovak |  |
| `CONFIG_LANGUAGE_SL_SI` | bool | Xiaozhi Assistant | Slovenian |  |
| `CONFIG_LANGUAGE_SV_SE` | bool | Xiaozhi Assistant | Swedish |  |
| `CONFIG_LANGUAGE_SR_RS` | bool | Xiaozhi Assistant | Serbian |  |
| `CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM` | bool | Xiaozhi Assistant | ESP32-S3 N16R8 Custom DIY Board (Bo mach tuy bien toan dien - Khuyen nghi) | y (mặc định của choice) |
| `CONFIG_BOARD_TYPE_ESP32_P4_FUNCTION_EV_BOARD` | bool | Xiaozhi Assistant | Espressif ESP32-P4-Function-EV-Board |  |
| `CONFIG_BOARD_TYPE_WAVESHARE_ESP32_P4_DEV_KIT` | bool | Xiaozhi Assistant | Waveshare ESP32-P4-Module-DEV-KIT |  |
| `CONFIG_USE_DEFAULT_MESSAGE_STYLE` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Mặc định (Chữ chạy ngang) | y (mặc định của choice) |
| `CONFIG_USE_WECHAT_MESSAGE_STYLE` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | WeChat (Bong bóng chat) |  |
| `CONFIG_USE_EMOTE_MESSAGE_STYLE` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Emote (Biểu cảm khuôn mặt emoji) |  |
| `CONFIG_CUSTOM_UI_STYLE_CHAT_BUBBLE` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Adaptive: Chat Bubble (Bong bóng hội thoại) |  |
| `CONFIG_CUSTOM_UI_STYLE_CLASSIC_AVATAR` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Adaptive: Classic Avatar (Hình đại diện AI cổ điển) |  |
| `CONFIG_CUSTOM_UI_STYLE_CYBER_TERMINAL` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Adaptive: Cyber Terminal (Giao diện dòng lệnh hacker) |  |
| `CONFIG_CUSTOM_UI_STYLE_MINIMAL_ZEN` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Adaptive: Minimal Zen (Tối giản thanh lịch) |  |
| `CONFIG_CUSTOM_UI_STYLE_SMART_DASHBOARD` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Adaptive: Smart Dashboard (Bảng điều khiển thông minh) |  |
| `CONFIG_CUSTOM_UI_ACCENT_CYBER_BLUE` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Cyber Blue (Xanh dương đậm) | y (mặc định của choice) |
| `CONFIG_CUSTOM_UI_ACCENT_EMERALD` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Emerald Green (Xanh lục bảo) |  |
| `CONFIG_CUSTOM_UI_ACCENT_PURPLE` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Cyber Purple (Tím neon) |  |
| `CONFIG_CUSTOM_UI_ACCENT_AMBER` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Warm Amber (Vàng hổ phách) |  |
| `CONFIG_CUSTOM_UI_ACCENT_MONO` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Monochrome (Đơn sắc trắng đen) |  |
| `CONFIG_CUSTOM_UI_AUTO_SCALE` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Tự động căn chỉnh kích thước UI theo độ phân giải màn hình | n |
| `CONFIG_CUSTOM_UI_ROUND_SCREEN_SAFE_AREA` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Vùng hiển thị an toàn cho màn hình tròn (Round Screen Safe Area) | y if CUSTOM_DISPLAY_GC9A01 \|\| CUSTOM_DISPLAY_AMOLED_CO5300 \|\| CUSTOM_DISPLAY_AMOLED_SPD2010 |
| `CONFIG_CUSTOM_UI_VOICE_WAVE_ENABLE` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Hiển thị sóng âm thanh động khi đàm thoại (Voice Waveform) | n |
| `CONFIG_CUSTOM_UI_WEATHER_ENABLE` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Hiển thị thời tiết trên màn hình (Weather Widget) | n |
| `CONFIG_CUSTOM_UI_WEATHER_CITY` | string | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Thành phố lấy dữ liệu thời tiết | "Hanoi" |
| `CONFIG_USE_MULTILINE_CHAT_MESSAGE` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Use multiline chat message display (default mode only) | n |
| `CONFIG_FLASH_DEFAULT_ASSETS` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Flash tài nguyên mặc định (Default Built-in Assets) | y (mặc định của choice) |
| `CONFIG_FLASH_CUSTOM_ASSETS` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Flash tài nguyên tùy chỉnh từ URL hoặc tệp cục bộ (Custom Assets) |  |
| `CONFIG_FLASH_EXPRESSION_ASSETS` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Flash tài nguyên biểu cảm AI Voice (Emote Expression Assets) |  |
| `CONFIG_FLASH_NONE_ASSETS` | bool | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Không nạp phân vùng tài nguyên (No Assets Flashing) |  |
| `CONFIG_CUSTOM_ASSETS_FILE` | string | Giao diện hiển thị & Gói tài nguyên (Display Style & Assets) | Đường dẫn tệp hoặc URL tài nguyên tùy chỉnh (Custom Assets File / URL) | "https://github.com/78/xiaozhi-esp32/releases/download/v0.9/assets.bin" |
| `CONFIG_WAKE_WORD_DISABLED` | bool | Từ khóa đánh thức (Wake Word) | Disabled |  |
| `CONFIG_USE_AFE_WAKE_WORD` | bool | Từ khóa đánh thức (Wake Word) | Wakenet model with AFE | y (mặc định của choice) |
| `CONFIG_USE_CUSTOM_WAKE_WORD` | bool | Từ khóa đánh thức (Wake Word) | Multinet model (Custom Wake Word) |  |
| `CONFIG_CUSTOM_WAKE_WORD` | string | Từ khóa đánh thức (Wake Word) | Custom Wake Word | "xiao tu dou" |
| `CONFIG_CUSTOM_WAKE_WORD_DISPLAY` | string | Từ khóa đánh thức (Wake Word) | Custom Wake Word Display | "小土豆" |
| `CONFIG_CUSTOM_WAKE_WORD_THRESHOLD` | int | Từ khóa đánh thức (Wake Word) | Custom Wake Word Threshold (%) | 20 |
| `CONFIG_SEND_WAKE_WORD_DATA` | bool | Từ khóa đánh thức (Wake Word) | Send Wake Word Data | y |
| `CONFIG_WAKE_WORD_DETECTION_IN_LISTENING` | bool | Từ khóa đánh thức (Wake Word) | Enable Wake Word Detection in Listening Mode | n |
| `CONFIG_USE_AUDIO_PROCESSOR` | bool | Bộ xử lý âm thanh & Khử tiếng vọng (AFE Audio Processing & AEC) | Enable AFE Audio Processing | y |
| `CONFIG_USE_DEVICE_AEC` | bool | Bộ xử lý âm thanh & Khử tiếng vọng (AFE Audio Processing & AEC) | Enable Device-Side AEC | y (mặc định của choice) |
| `CONFIG_USE_SERVER_AEC` | bool | Bộ xử lý âm thanh & Khử tiếng vọng (AFE Audio Processing & AEC) | Enable Server-Side AEC (Unstable) |  |
| `CONFIG_USE_NO_AEC` | bool | Bộ xử lý âm thanh & Khử tiếng vọng (AFE Audio Processing & AEC) | Disable AEC |  |
| `CONFIG_USE_SOFTWARE_AEC_REFERENCE` | bool | Bộ xử lý âm thanh & Khử tiếng vọng (AFE Audio Processing & AEC) | Software AEC Reference (codecs without hardware loopback) | y |
| `CONFIG_USE_AUDIO_DEBUGGER` | bool | Xiaozhi Assistant | Enable Audio Debugger | n |
| `CONFIG_USE_HOTSPOT_WIFI_PROVISIONING` | bool | WiFi Configuration Method | Hotspot | y |
| `CONFIG_USE_ESP_BLUFI_WIFI_PROVISIONING` | bool | WiFi Configuration Method | ESP-BluFi |  |
| `CONFIG_AUDIO_DEBUG_UDP_SERVER` | string | Xiaozhi Assistant | Audio Debug UDP Server Address | "192.168.2.100:8000" |
| `CONFIG_RECEIVE_CUSTOM_MESSAGE` | bool | Xiaozhi Assistant | Enable Custom Message Reception | n |
| `CONFIG_XIAOZHI_CAMERA_MIRROR_CONFIGURED` | bool | Camera Configuration | Override camera mirror settings | n |
| `CONFIG_XIAOZHI_CAMERA_HMIRROR` | bool | Camera Configuration | Mirror Camera Horizontally | n |
| `CONFIG_XIAOZHI_CAMERA_VFLIP` | bool | Camera Configuration | Mirror Camera Vertically | n |
| `CONFIG_XIAOZHI_CAMERA_ALLOW_JPEG_INPUT` | bool | Camera Configuration | Allow JPEG Input | n |
| `CONFIG_XIAOZHI_ENABLE_CAMERA_DEBUG_MODE` | bool | Camera Configuration | Enable Camera Debug Mode | n |
| `CONFIG_XIAOZHI_ENABLE_CAMERA_ENDIANNESS_SWAP` | bool | Camera Configuration | Enable software camera buffer endianness swapping | n |
| `CONFIG_XIAOZHI_ENABLE_ROTATE_CAMERA_IMAGE` | bool | Camera Configuration | Enable Camera Image Rotation | n |
| `CONFIG_XIAOZHI_CAMERA_IMAGE_ROTATION_ANGLE_90` | bool | Enable Camera Image Rotation | 90° | y (mặc định của choice) |
| `CONFIG_XIAOZHI_CAMERA_IMAGE_ROTATION_ANGLE_270` | bool | Enable Camera Image Rotation | 270° |  |
| `CONFIG_ENABLE_CUSTOM_DISPLAY` | bool | 1. Màn hình & Cảm ứng (Display & Touch) | Màn hình hiển thị (Display Panel) | n |
| `CONFIG_CUSTOM_DISPLAY_ST7789` | bool | Màn hình hiển thị (Display Panel) | Sitronix ST7789 (240x320, 240x240, 170x320 SPI) | y (mặc định của choice) |
| `CONFIG_CUSTOM_DISPLAY_ST7796` | bool | Màn hình hiển thị (Display Panel) | Sitronix ST7796 (320x480 SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_ST7735` | bool | Màn hình hiển thị (Display Panel) | Sitronix ST7735 / ST7735S (128x128, 128x160 SPI mini) |  |
| `CONFIG_CUSTOM_DISPLAY_ST7701` | bool | Màn hình hiển thị (Display Panel) | Sitronix ST7701 (480x480 RGB song song) |  |
| `CONFIG_CUSTOM_DISPLAY_ILI9341` | bool | Màn hình hiển thị (Display Panel) | Ilitek ILI9341 (240x320 SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_ILI9486` | bool | Màn hình hiển thị (Display Panel) | Ilitek ILI9486 / ILI9488 (320x480 SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_GC9A01` | bool | Màn hình hiển thị (Display Panel) | GalaxyCore GC9A01 (240x240 Màn hình tròn SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_GC9107` | bool | Màn hình hiển thị (Display Panel) | GalaxyCore GC9107 (128x128 SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_NV3023` | bool | Màn hình hiển thị (Display Panel) | NewVision NV3030B (240x280 SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_JD9853` | bool | Màn hình hiển thị (Display Panel) | Jadard JD9853 (172x320 SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_OLED_SSD1306` | bool | Màn hình hiển thị (Display Panel) | Solomon OLED SSD1306 (128x64 / 128x32 I2C) |  |
| `CONFIG_CUSTOM_DISPLAY_OLED_SH1106` | bool | Màn hình hiển thị (Display Panel) | Sino Wealth OLED SH1106 (128x64 I2C) |  |
| `CONFIG_CUSTOM_DISPLAY_QSPI_AMOLED` | bool | Màn hình hiển thị (Display Panel) | AMOLED QSPI (SH8601 368x448 / CO5300 466x466 / SPD2010 412x412) |  |
| `CONFIG_CUSTOM_DISPLAY_EPAPER_SSD1681` | bool | Màn hình hiển thị (Display Panel) | Solomon SSD1681 (200x200 Mực điện tử E-Paper SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_UART` | bool | Màn hình hiển thị (Display Panel) | Màn hình rời UART HMI (Nextion / TJC / DWIN / Serial Display) |  |
| `CONFIG_CUSTOM_DISPLAY_USER_CUSTOM` | bool | Màn hình hiển thị (Display Panel) | Driver Màn hình tùy chỉnh (Custom User Display Hook) |  |
| `CONFIG_CUSTOM_DISPLAY_NONE` | bool | Màn hình hiển thị (Display Panel) | Không sử dụng màn hình (Audio Only / Headless) |  |
| `CONFIG_ENABLE_CUSTOM_SECONDARY_UART_DISPLAY` | bool | Màn hình hiển thị (Display Panel) | Màn hình rời UART mở rộng / Chạy song song 2 màn hình (Secondary UART Display) | n |
| `CONFIG_CUSTOM_DISPLAY_UART_LINK_WIRED` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | Có dây (UART trực tiếp) | y (mặc định của choice) |
| `CONFIG_CUSTOM_DISPLAY_UART_LINK_BLUETOOTH` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | Không dây qua module Bluetooth UART |  |
| `CONFIG_CUSTOM_DISPLAY_UART_PORT` | int | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | Cổng UART phần cứng (1: UART1, 2: UART2) | 1 |
| `CONFIG_CUSTOM_DISPLAY_UART_TX_PIN` | int | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | Chân TX kết nối chân RX của Màn hình | -1 |
| `CONFIG_CUSTOM_DISPLAY_UART_RX_PIN` | int | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | Chân RX kết nối chân TX của Màn hình | -1 |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_9600` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 9600 bps (Nextion mặc định khi xuất xưởng) |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_19200` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 19200 bps |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_38400` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 38400 bps |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_57600` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 57600 bps |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_115200` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 115200 bps (Khuyên dùng) | y (mặc định của choice) |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_230400` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 230400 bps |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_460800` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 460800 bps |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_921600` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 921600 bps (Tốc độ cao) |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE` | int | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) |  | 9600 if CUSTOM_DISPLAY_UART_BAUDRATE_9600 |
| `CONFIG_CUSTOM_DISPLAY_UART_PROTO_NEXTION` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | Nextion / TJC HMI (Lệnh kết thúc 0xFF 0xFF 0xFF) | y (mặc định của choice) |
| `CONFIG_CUSTOM_DISPLAY_UART_PROTO_JSON` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | JSON Streaming (Chuỗi JSON kèm ký tự xuống dòng n) |  |
| `CONFIG_CUSTOM_DISPLAY_UART_PROTO_RAW_TEXT` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | Raw Text / ASCII (STATUS:..., EMOTION:..., CHAT:...) |  |
| `CONFIG_CUSTOM_DISPLAY_UART_PROTO_DWIN` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | DWIN DGUS (Khung truyền nhị phân 0x5A 0xA5) |  |
| `CONFIG_CUSTOM_DISPLAY_WIDTH` | int | Màn hình hiển thị (Display Panel) | Độ phân giải ngang (Width Pixels) | 240 if CUSTOM_DISPLAY_ST7789 \|\| CUSTOM_DISPLAY_ILI9341 \|\| CUSTOM_DISPLAY_GC9A01 \|\| CUSTOM_DISPLAY_NV3023 |
| `CONFIG_CUSTOM_DISPLAY_HEIGHT` | int | Màn hình hiển thị (Display Panel) | Độ phân giải dọc (Height Pixels) | 320 if CUSTOM_DISPLAY_ST7789 \|\| CUSTOM_DISPLAY_ILI9341 \|\| CUSTOM_DISPLAY_JD9853 |
| `CONFIG_CUSTOM_DISPLAY_PIN_MOSI` | int | Màn hình hiển thị (Display Panel) | Chân dữ liệu SPI MOSI (SDA) | -1 |
| `CONFIG_CUSTOM_DISPLAY_PIN_CLK` | int | Màn hình hiển thị (Display Panel) | Chân xung nhịp SPI CLK (SCL) | -1 |
| `CONFIG_CUSTOM_DISPLAY_PIN_CS` | int | Màn hình hiển thị (Display Panel) | Chân chọn chip SPI CS (-1 nếu không dùng) | -1 |
| `CONFIG_CUSTOM_DISPLAY_PIN_DC` | int | Màn hình hiển thị (Display Panel) | Chân lệnh/dữ liệu SPI DC | -1 |
| `CONFIG_CUSTOM_DISPLAY_USE_RST` | bool | Màn hình hiển thị (Display Panel) | Sử dụng chân Reset màn hình (RST) | y |
| `CONFIG_CUSTOM_DISPLAY_PIN_RST` | int | Màn hình hiển thị (Display Panel) | Chân Reset màn hình RST GPIO | -1 |
| `CONFIG_CUSTOM_DISPLAY_USE_BLK` | bool | Màn hình hiển thị (Display Panel) | Điều khiển đèn nền màn hình (Backlight BLK) | y |
| `CONFIG_CUSTOM_DISPLAY_PIN_BLK` | int | Màn hình hiển thị (Display Panel) | Chân đèn nền BLK GPIO | -1 |
| `CONFIG_CUSTOM_DISPLAY_PIN_I2C_SDA` | int | Màn hình hiển thị (Display Panel) | Chân OLED I2C SDA | -1 |
| `CONFIG_CUSTOM_DISPLAY_PIN_I2C_SCL` | int | Màn hình hiển thị (Display Panel) | Chân OLED I2C SCL | -1 |
| `CONFIG_CUSTOM_DISPLAY_AMOLED_SH8601` | bool | Màn hình hiển thị (Display Panel) | Sino Wealth SH8601 | y (mặc định của choice) |
| `CONFIG_CUSTOM_DISPLAY_AMOLED_CO5300` | bool | Màn hình hiển thị (Display Panel) | Chipone CO5300 |  |
| `CONFIG_CUSTOM_DISPLAY_AMOLED_SPD2010` | bool | Màn hình hiển thị (Display Panel) | Solomon SPD2010 |  |
| `CONFIG_CUSTOM_DISPLAY_QSPI_CS` | int | Màn hình hiển thị (Display Panel) | Chân AMOLED QSPI CS | -1 |
| `CONFIG_CUSTOM_DISPLAY_QSPI_CLK` | int | Màn hình hiển thị (Display Panel) | Chân AMOLED QSPI CLK | -1 |
| `CONFIG_CUSTOM_DISPLAY_QSPI_D0` | int | Màn hình hiển thị (Display Panel) | Chân AMOLED QSPI D0 | -1 |
| `CONFIG_CUSTOM_DISPLAY_QSPI_D1` | int | Màn hình hiển thị (Display Panel) | Chân AMOLED QSPI D1 | -1 |
| `CONFIG_CUSTOM_DISPLAY_QSPI_D2` | int | Màn hình hiển thị (Display Panel) | Chân AMOLED QSPI D2 | -1 |
| `CONFIG_CUSTOM_DISPLAY_QSPI_D3` | int | Màn hình hiển thị (Display Panel) | Chân AMOLED QSPI D3 | -1 |
| `CONFIG_CUSTOM_DISPLAY_QSPI_RST` | int | Màn hình hiển thị (Display Panel) | Chân AMOLED QSPI Reset | -1 |
| `CONFIG_CUSTOM_DISPLAY_EPAPER_PIN_BUSY` | int | Màn hình hiển thị (Display Panel) | Chân E-Paper BUSY | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_PCLK` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB PCLK | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_DE` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DE (Data Enable) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_VSYNC` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB VSYNC | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_HSYNC` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB HSYNC | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D0` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA0 (B0) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D1` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA1 (B1) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D2` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA2 (B2) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D3` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA3 (B3) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D4` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA4 (B4) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D5` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA5 (G0) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D6` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA6 (G1) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D7` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA7 (G2) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D8` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA8 (G3) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D9` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA9 (G4) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D10` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA10 (G5) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D11` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA11 (R0) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D12` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA12 (R1) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D13` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA13 (R2) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D14` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA14 (R3) | -1 |
| `CONFIG_CUSTOM_DISPLAY_RGB_D15` | int | Chân giao tiếp RGB song song ST7701 (RGB565 16-bit) | Chân RGB DATA15 (R4) | -1 |
| `CONFIG_CUSTOM_DISPLAY_OFFSET_X` | int | Màn hình hiển thị (Display Panel) | Dịch tọa độ X (Offset X Pixels) | 34 if CUSTOM_DISPLAY_JD9853 |
| `CONFIG_CUSTOM_DISPLAY_OFFSET_Y` | int | Màn hình hiển thị (Display Panel) | Dịch tọa độ Y (Offset Y Pixels) | 0 |
| `CONFIG_CUSTOM_DISPLAY_MIRROR_X` | bool | Màn hình hiển thị (Display Panel) | Đảo chiều gương trục X (Mirror X) | n |
| `CONFIG_CUSTOM_DISPLAY_MIRROR_Y` | bool | Màn hình hiển thị (Display Panel) | Đảo chiều gương trục Y (Mirror Y) | n |
| `CONFIG_CUSTOM_DISPLAY_SWAP_XY` | bool | Màn hình hiển thị (Display Panel) | Hoán đổi tọa độ X-Y (X/Y Axes Swapped) | n |
| `CONFIG_CUSTOM_DISPLAY_INVERT_COLOR` | bool | Màn hình hiển thị (Display Panel) | Đảo ngược màu sắc hiển thị (Invert Color) | n |
| `CONFIG_ENABLE_CUSTOM_TOUCH` | bool | 1. Màn hình & Cảm ứng (Display & Touch) | Màn hình cảm ứng (Touch Screen Panel) | n |
| `CONFIG_CUSTOM_TOUCH_CST816S` | bool | Màn hình cảm ứng (Touch Screen Panel) | Hynitron CST816S / CST9217 (I2C) | y (mặc định của choice) |
| `CONFIG_CUSTOM_TOUCH_GT911` | bool | Màn hình cảm ứng (Touch Screen Panel) | Goodix GT911 / GT1151 (I2C) |  |
| `CONFIG_CUSTOM_TOUCH_FT5X06` | bool | Màn hình cảm ứng (Touch Screen Panel) | FocalTech FT5x06 / FT6336 (I2C) |  |
| `CONFIG_CUSTOM_TOUCH_USER_CUSTOM` | bool | Màn hình cảm ứng (Touch Screen Panel) | Driver Cảm ứng tùy chỉnh (Custom User Touch Hook) |  |
| `CONFIG_CUSTOM_TOUCH_NONE` | bool | Màn hình cảm ứng (Touch Screen Panel) | Không sử dụng cảm ứng |  |
| `CONFIG_CUSTOM_TOUCH_PIN_SDA` | int | Màn hình cảm ứng (Touch Screen Panel) | Chân cảm ứng I2C SDA | -1 |
| `CONFIG_CUSTOM_TOUCH_PIN_SCL` | int | Màn hình cảm ứng (Touch Screen Panel) | Chân cảm ứng I2C SCL | -1 |
| `CONFIG_CUSTOM_TOUCH_PIN_INT` | int | Màn hình cảm ứng (Touch Screen Panel) | Chân ngắt cảm ứng INT GPIO | -1 |
| `CONFIG_CUSTOM_TOUCH_PIN_RST` | int | Màn hình cảm ứng (Touch Screen Panel) | Chân reset cảm ứng RST GPIO (-1 nếu không nối) | -1 |
| `CONFIG_CUSTOM_ENABLE_PERIPH_TOUCH_SCREEN` | bool | 1. Màn hình & Cảm ứng (Display & Touch) | Mở rộng giao tiếp màn cảm ứng ngoại vi (Peripheral Touch Screen Controller) | n |
| `CONFIG_CUSTOM_PERIPH_TOUCH_I2C_SDA` | int | 1. Màn hình & Cảm ứng (Display & Touch) | Chân ngoại vi cảm ứng I2C SDA | -1 |
| `CONFIG_CUSTOM_PERIPH_TOUCH_I2C_SCL` | int | 1. Màn hình & Cảm ứng (Display & Touch) | Chân ngoại vi cảm ứng I2C SCL | -1 |
| `CONFIG_CUSTOM_PERIPH_TOUCH_INT_PIN` | int | 1. Màn hình & Cảm ứng (Display & Touch) | Chân ngoại vi cảm ứng ngắt INT | -1 |
| `CONFIG_CUSTOM_PERIPH_TOUCH_RST_PIN` | int | 1. Màn hình & Cảm ứng (Display & Touch) | Chân ngoại vi cảm ứng reset RST | -1 |
| `CONFIG_ENABLE_CUSTOM_AUDIO` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) |  | y if ENABLE_CUSTOM_SPEAKER \|\| ENABLE_CUSTOM_MIC |
| `CONFIG_CUSTOM_AUDIO_CODEC_USER_CUSTOM` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) |  | y if CUSTOM_AUDIO_SPK_CODEC_USER_CUSTOM \|\| CUSTOM_AUDIO_MIC_CODEC_USER_CUSTOM |
| `CONFIG_ENABLE_CUSTOM_SPEAKER` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | n |
| `CONFIG_CUSTOM_AUDIO_DAC_MAX98357A` | bool | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | MAX98357A (Mạch công suất I2S Class-D Mono 3W) | y (mặc định của choice) |
| `CONFIG_CUSTOM_AUDIO_DAC_MAX98360A` | bool | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | MAX98360A / MAX98360B (I2S/PDM Class-D Mono 3W thế hệ mới) |  |
| `CONFIG_CUSTOM_AUDIO_DAC_PCM5102A` | bool | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | PCM5102A (Mạch giải mã Hi-Fi I2S Stereo DAC 32-bit/384kHz) |  |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8311` | bool | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | Everest Semi ES8311 Codec (I2C) |  |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8388` | bool | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | Everest Semi ES8388 Stereo Codec (I2C) |  |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8374` | bool | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | Everest Semi ES8374 Codec |  |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8389` | bool | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | Everest Semi ES8389 Codec |  |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_BOX` | bool | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | Box Audio Codec |  |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_USER_CUSTOM` | bool | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | Driver Loa / DAC phát tùy chỉnh (Custom User Speaker Hook) |  |
| `CONFIG_CUSTOM_AUDIO_SPK_GPIO_DOUT` | int | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | Chân dữ liệu Loa I2S DOUT (DIN trên module) | -1 |
| `CONFIG_CUSTOM_AUDIO_SPK_GPIO_BCLK` | int | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | Chân xung nhịp Loa I2S BCLK (BCK) | -1 |
| `CONFIG_CUSTOM_AUDIO_SPK_GPIO_LRCK` | int | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | Chân xung nhịp Loa I2S LRCK / WS (LCK) | -1 |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_I2C_SDA` | int | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | Chân Codec DAC I2C SDA | -1 |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_I2C_SCL` | int | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | Chân Codec DAC I2C SCL | -1 |
| `CONFIG_ENABLE_CUSTOM_MIC` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Microphone thu âm (Microphone & Audio In) | n |
| `CONFIG_CUSTOM_AUDIO_MIC_INMP441` | bool | Microphone thu âm (Microphone & Audio In) | INMP441 / MSM261S (Microphone số I2S MEMS Omnidirectional) | y (mặc định của choice) |
| `CONFIG_CUSTOM_AUDIO_MIC_PDM` | bool | Microphone thu âm (Microphone & Audio In) | PDM Microphone (Micro số giao thức PDM) |  |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8311` | bool | Microphone thu âm (Microphone & Audio In) | Everest Semi ES8311 ADC (I2C) |  |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8388` | bool | Microphone thu âm (Microphone & Audio In) | Everest Semi ES8388 ADC (I2C) |  |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8374` | bool | Microphone thu âm (Microphone & Audio In) | Everest Semi ES8374 ADC |  |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8389` | bool | Microphone thu âm (Microphone & Audio In) | Everest Semi ES8389 ADC |  |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_BOX` | bool | Microphone thu âm (Microphone & Audio In) | Box Audio Codec ADC |  |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_USER_CUSTOM` | bool | Microphone thu âm (Microphone & Audio In) | Driver Micro thu âm tùy chỉnh (Custom User Audio Mic Hook) |  |
| `CONFIG_CUSTOM_AUDIO_MIC_GPIO_DIN` | int | Microphone thu âm (Microphone & Audio In) | Chân dữ liệu Micro I2S DIN (SD trên INMP441/MSM261S) | -1 |
| `CONFIG_CUSTOM_AUDIO_MIC_GPIO_SCK` | int | Microphone thu âm (Microphone & Audio In) | Chân xung nhịp Micro I2S SCK (SCK/BCLK) | -1 |
| `CONFIG_CUSTOM_AUDIO_MIC_GPIO_WS` | int | Microphone thu âm (Microphone & Audio In) | Chân chọn kênh Micro I2S WS (WS/LRCK) | -1 |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_I2C_SDA` | int | Microphone thu âm (Microphone & Audio In) | Chân Codec ADC I2C SDA | -1 |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_I2C_SCL` | int | Microphone thu âm (Microphone & Audio In) | Chân Codec ADC I2C SCL | -1 |
| `CONFIG_CUSTOM_AUDIO_I2S_SIMPLEX` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Simplex (Độc lập: Loa và Micro có chân Clock BCLK/WS riêng) | y (mặc định của choice) |
| `CONFIG_CUSTOM_AUDIO_I2S_DUPLEX` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Duplex (Dùng chung: Loa và Micro chia sẻ chung chân BCLK và WS) |  |
| `CONFIG_CUSTOM_AUDIO_CODEC_AEC_REFERENCE` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Kênh tham chiếu khử tiếng vọng từ Codec (AEC Hardware Reference) | n |
| `CONFIG_ENABLE_CUSTOM_UART` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Cổng UART mở rộng (Serial Communication Port) | n |
| `CONFIG_CUSTOM_UART_PORT_1` | bool | Cổng UART mở rộng (Serial Communication Port) | UART1 (Cổng serial phần cứng 1) | y (mặc định của choice) |
| `CONFIG_CUSTOM_UART_PORT_2` | bool | Cổng UART mở rộng (Serial Communication Port) | UART2 (Cổng serial phần cứng 2) |  |
| `CONFIG_CUSTOM_UART_PIN_TX` | int | Cổng UART mở rộng (Serial Communication Port) | Chân phát UART TX GPIO | -1 |
| `CONFIG_CUSTOM_UART_PIN_RX` | int | Cổng UART mở rộng (Serial Communication Port) | Chân nhận UART RX GPIO | -1 |
| `CONFIG_CUSTOM_UART_BAUDRATE` | int | Cổng UART mở rộng (Serial Communication Port) | Tốc độ baud truyền thông (Baudrate) | 115200 |
| `CONFIG_CUSTOM_UART_USE_FLOW_CONTROL` | bool | Cổng UART mở rộng (Serial Communication Port) | Kích hoạt điều khiển luồng phần cứng (RTS/CTS Hardware Flow Control) | n |
| `CONFIG_CUSTOM_UART_PIN_RTS` | int | Cổng UART mở rộng (Serial Communication Port) | Chân RTS GPIO (Request to Send) | -1 |
| `CONFIG_CUSTOM_UART_PIN_CTS` | int | Cổng UART mở rộng (Serial Communication Port) | Chân CTS GPIO (Clear to Send) | -1 |
| `CONFIG_ENABLE_BT_UART_MODULE` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | n |
| `CONFIG_BT_UART_MODULE_HC05` | bool | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | HC-05 (Bluetooth Classic SPP, chủ/tớ, cần chân KEY để vào AT) | y (mặc định của choice) |
| `CONFIG_BT_UART_MODULE_HC06` | bool | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | HC-06 / linvor (Bluetooth Classic SPP, chỉ làm tớ) |  |
| `CONFIG_BT_UART_MODULE_HM10` | bool | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | HM-10 / CC2541 (BLE, chủ/tớ) |  |
| `CONFIG_BT_UART_MODULE_PRECONFIGURED` | bool | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Module đã cấu hình sẵn (không gửi lệnh AT) |  |
| `CONFIG_BT_UART_MODULE_PORT_1` | bool | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | UART1 |  |
| `CONFIG_BT_UART_MODULE_PORT_2` | bool | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | UART2 | y (mặc định của choice) |
| `CONFIG_BT_UART_MODULE_TX_PIN` | int | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Chân TX (nối chân RXD của module) | -1 |
| `CONFIG_BT_UART_MODULE_RX_PIN` | int | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Chân RX (nối chân TXD của module) | -1 |
| `CONFIG_BT_UART_MODULE_BAUDRATE` | int | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Tốc độ baud dữ liệu của module | 9600 |
| `CONFIG_BT_UART_MODULE_STATE_PIN` | int | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Chân STATE / LED (biết đã kết nối, -1 nếu không nối) | -1 |
| `CONFIG_BT_UART_MODULE_KEY_PIN` | int | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Chân KEY / EN của HC-05 (-1 nếu không nối) | -1 |
| `CONFIG_BT_UART_MODULE_APPLY_SETTINGS` | bool | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Gửi lệnh AT đặt tên, mã PIN và vai trò khi khởi động | n |
| `CONFIG_BT_UART_MODULE_NAME` | string | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Tên Bluetooth của module | "Xiaozhi-BT" |
| `CONFIG_BT_UART_MODULE_PIN_CODE` | string | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Mã PIN ghép đôi (HC-05/HC-06: 4 số, HM-10: 6 số, để trống = giữ nguyên) | "" |
| `CONFIG_BT_UART_MODULE_ROLE_SLAVE` | bool | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Tớ (Slave) | y (mặc định của choice) |
| `CONFIG_BT_UART_MODULE_ROLE_MASTER` | bool | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Chủ (Master), tự kết nối tới địa chỉ cố định |  |
| `CONFIG_BT_UART_MODULE_REMOTE_ADDR` | string | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Địa chỉ Bluetooth của module đầu kia (12 chữ số hex) | "" |
| `CONFIG_BT_UART_MODULE_FRAME_RAW` | bool | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Thô (gửi nguyên văn, nhận theo khoảng nghỉ) |  |
| `CONFIG_BT_UART_MODULE_FRAME_LINE` | bool | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Dòng văn bản (kết thúc bằng ký tự xuống dòng LF) | y (mặc định của choice) |
| `CONFIG_BT_UART_MODULE_FRAME_BINARY4` | bool | Module Bluetooth UART không dây (HC-05 / HC-06 / HM-10) | Khung nhị phân 4 byte (0xAA, chục, đơn vị, XOR) |  |
| `CONFIG_CUSTOM_MCP_TOOL_LAMP` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Đèn điều khiển bằng AI qua GPIO (MCP) | n |
| `CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO` | int | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Chân GPIO điều khiển Đèn | -1 |
| `CONFIG_CUSTOM_MCP_TOOL_LAMP_EXTRA_GPIOS` | string | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Các chân GPIO Đèn thêm (cách nhau dấu phẩy) | "" |
| `CONFIG_CUSTOM_MCP_TOOL_SENSOR` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) |  | y if CUSTOM_ENABLE_SENSOR_DHT11_22 \|\| CUSTOM_ENABLE_SENSOR_AHT20 \|\| CUSTOM_ENABLE_SENSOR_BMP280 \|\| CUSTOM_ENABLE_SENSOR_BH1750 \|\| CUSTOM_ENABLE_SENSOR_LDR \|\| CUSTOM_SENSOR_GAS_SCD4X \|\| CUSTOM_ENABLE_SENSOR_GAS_ANALOG_MQ \|\| CUSTOM_ENABLE_SENSOR_APDS9960 \|\| CUSTOM_ENABLE_SENSOR_VL53LX \|\| CUSTOM_ENABLE_SENSOR_HCSR04 \|\| CUSTOM_ENABLE_SENSOR_PIR \|\| CUSTOM_ENABLE_SENSOR_VIBRATION_SW420 \|\| CUSTOM_ENABLE_SENSOR_FLAME \|\| CUSTOM_ENABLE_SENSOR_INA2XX \|\| CUSTOM_ENABLE_PERIPH_BATTERY_CHARGING_DETECT \|\| CUSTOM_BATTERY_MONITOR_BQ27220 |
| `CONFIG_CUSTOM_MCP_TOOL_ACTUATOR` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) |  | y if CUSTOM_PERIPH_RELAY_ENABLE \|\| CUSTOM_ENABLE_SERVO_DOG \|\| CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE \|\| ENABLE_BUZZER \|\| ENABLE_HAPTIC_MOTOR |
| `CONFIG_CUSTOM_MCP_TOOL_ROBOT` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) |  | y if CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE \|\| CUSTOM_ENABLE_SERVO_DOG |
| `CONFIG_CUSTOM_MCP_TOOL_LED` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) |  | y if ENABLE_CUSTOM_LEDS && !CUSTOM_LED_NONE |
| `CONFIG_CUSTOM_MCP_TOOL_GPIO_LED` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) |  | y if CUSTOM_LED_EXTRA_GPIOS != "" |
| `CONFIG_CUSTOM_MCP_TOOL_PERIPH` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) |  | y if CUSTOM_ENABLE_PERIPH_RTC \|\| CUSTOM_ENABLE_PERIPH_NFC_PN532 \|\| CUSTOM_ENABLE_PERIPH_PCA9685 \|\| CUSTOM_ENABLE_PERIPH_LED_DRIVER_IC \|\| CUSTOM_PERIPH_RC522_SPI_CS >= 0 \|\| CUSTOM_IMU_I2C_SDA >= 0 \|\| CUSTOM_PMIC_I2C_SDA >= 0 \|\| CUSTOM_EXPANDER_I2C_SDA >= 0 \|\| CUSTOM_ETH_SPI_CS_PIN >= 0 |
| `CONFIG_CUSTOM_MCP_TOOL_PRESS_TO_TALK` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) |  | y if CUSTOM_ENABLE_BUTTON_BOOT \|\| CUSTOM_ENABLE_BUTTON_TOUCH |
| `CONFIG_CUSTOM_MCP_TOOL_BT_UART` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) |  | y if ENABLE_BT_UART_MODULE && !CUSTOM_DISPLAY_UART_LINK_BLUETOOTH |
| `CONFIG_ENABLE_CUSTOM_MCP_SERVER` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) |  | y if CUSTOM_MCP_TOOL_LAMP \|\| CUSTOM_MCP_TOOL_SENSOR \|\| CUSTOM_MCP_TOOL_ACTUATOR \|\| CUSTOM_MCP_TOOL_ROBOT \|\| CUSTOM_MCP_TOOL_LED \|\| CUSTOM_MCP_TOOL_GPIO_LED \|\| CUSTOM_MCP_TOOL_PERIPH \|\| CUSTOM_MCP_TOOL_PRESS_TO_TALK \|\| CUSTOM_MCP_TOOL_BT_UART |
| `CONFIG_CUSTOM_ENABLE_BUTTON_BOOT` | bool | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Nút bấm BOOT Onboard (BOOT Button) | y |
| `CONFIG_CUSTOM_BUTTON_BOOT_GPIO` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân GPIO nút Boot | 0 |
| `CONFIG_CUSTOM_BUTTON_BOOT_EXTRA_GPIOS` | string | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Các chân GPIO nút Boot thêm (cách nhau dấu phẩy) | "" |
| `CONFIG_CUSTOM_ENABLE_BUTTON_TOUCH` | bool | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Nút bấm chức năng Action / Chạm (PTT / Touch Key) | n |
| `CONFIG_CUSTOM_BUTTON_TOUCH_GPIO` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân GPIO nút Action | -1 |
| `CONFIG_CUSTOM_ENABLE_BUTTON_VOLUME` | bool | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Cụm phím vật lý Tăng / Giảm âm lượng (Volume Buttons) | n |
| `CONFIG_CUSTOM_BUTTON_VOL_UP_GPIO` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân GPIO nút tăng âm lượng (Vol Up) | -1 |
| `CONFIG_CUSTOM_BUTTON_VOL_DOWN_GPIO` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân GPIO nút giảm âm lượng (Vol Down) | -1 |
| `CONFIG_CUSTOM_ENABLE_TOUCH_SLIDER` | bool | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Thanh trượt cảm ứng điện dung nội bộ ESP32-S3 (Touch Slider) | n |
| `CONFIG_CUSTOM_TOUCH_SLIDER_PAD1_GPIO` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân Touch Pad 1 của thanh trượt (GPIO 1-14) | -1 |
| `CONFIG_CUSTOM_TOUCH_SLIDER_PAD2_GPIO` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân Touch Pad 2 của thanh trượt (GPIO 1-14) | -1 |
| `CONFIG_CUSTOM_TOUCH_SLIDER_PAD3_GPIO` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân Touch Pad 3 của thanh trượt (GPIO 1-14) | -1 |
| `CONFIG_CUSTOM_ENABLE_PERIPH_ROTARY_ENCODER` | bool | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Núm vặn xoay vô cấp cơ học (Rotary Encoder EC11 qua PCNT) | n |
| `CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_A` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân xung pha A (CLK) Encoder | -1 |
| `CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_B` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân xung pha B (DT) Encoder | -1 |
| `CONFIG_CUSTOM_PERIPH_ENCODER_KEY_PIN` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân phím nhấn SW Encoder (-1 nếu không dùng) | -1 |
| `CONFIG_CUSTOM_ENABLE_USER_CUSTOM_SENSORS` | bool | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Driver Cảm biến ngoại vi tùy chỉnh (Custom User Sensor Hook) | n |
| `CONFIG_ENABLE_CUSTOM_CAMERA` | bool | 4.2. Hệ thống chính (Main System) | Camera & Thị giác AI (Camera DVP / USB UVC) | n |
| `CONFIG_CUSTOM_CAMERA_OV2640` | bool | Camera & Thị giác AI (Camera DVP / USB UVC) | OmniVision OV2640 (2 Megapixel) | y (mặc định của choice) |
| `CONFIG_CUSTOM_CAMERA_OV3660` | bool | Camera & Thị giác AI (Camera DVP / USB UVC) | OmniVision OV3660 (3 Megapixel) |  |
| `CONFIG_CUSTOM_CAMERA_OV5640` | bool | Camera & Thị giác AI (Camera DVP / USB UVC) | OmniVision OV5640 (5 Megapixel) |  |
| `CONFIG_CUSTOM_CAMERA_SC030IOT` | bool | Camera & Thị giác AI (Camera DVP / USB UVC) | SmartSens SC030IOT |  |
| `CONFIG_CUSTOM_CAMERA_USB_UVC` | bool | Camera & Thị giác AI (Camera DVP / USB UVC) | USB UVC Web Camera (ESP32-S3 USB Host Port) |  |
| `CONFIG_CUSTOM_CAMERA_USER_CUSTOM` | bool | Camera & Thị giác AI (Camera DVP / USB UVC) | Driver Camera tùy chỉnh (Custom User Camera Hook) |  |
| `CONFIG_CUSTOM_CAM_PIN_XCLK` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Camera XCLK (Master Clock) | -1 |
| `CONFIG_CUSTOM_CAM_PIN_PCLK` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Camera PCLK (Pixel Clock) | -1 |
| `CONFIG_CUSTOM_CAM_PIN_VSYNC` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Camera VSYNC (Vertical Sync) | -1 |
| `CONFIG_CUSTOM_CAM_PIN_HREF` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Camera HREF (Horizontal Reference) | -1 |
| `CONFIG_CUSTOM_CAM_PIN_SIOD` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Camera SIOD (SCCB I2C Data) | -1 |
| `CONFIG_CUSTOM_CAM_PIN_SIOC` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Camera SIOC (SCCB I2C Clock) | -1 |
| `CONFIG_CUSTOM_CAM_USE_RESET` | bool | Camera & Thị giác AI (Camera DVP / USB UVC) | Sử dụng chân Reset Camera (RST) | n |
| `CONFIG_CUSTOM_CAM_PIN_RESET` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Chân Camera Reset GPIO | -1 |
| `CONFIG_CUSTOM_CAM_USE_PWDN` | bool | Camera & Thị giác AI (Camera DVP / USB UVC) | Sử dụng chân Power-Down Camera (PWDN) | n |
| `CONFIG_CUSTOM_CAM_PIN_PWDN` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Chân Camera PWDN GPIO | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D0` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Camera Data D0 Pin | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D1` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Camera Data D1 Pin | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D2` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Camera Data D2 Pin | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D3` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Camera Data D3 Pin | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D4` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Camera Data D4 Pin | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D5` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Camera Data D5 Pin | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D6` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Camera Data D6 Pin | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D7` | int | Camera & Thị giác AI (Camera DVP / USB UVC) | Camera Data D7 Pin | -1 |
| `CONFIG_CUSTOM_CAM_HMIRROR` | bool | Camera & Thị giác AI (Camera DVP / USB UVC) | Lật ảnh theo chiều ngang (Horizontal Mirror) | n |
| `CONFIG_CUSTOM_CAM_VFLIP` | bool | Camera & Thị giác AI (Camera DVP / USB UVC) | Lật ảnh theo chiều dọc (Vertical Flip) | n |
| `CONFIG_ENABLE_CUSTOM_LEDS` | bool | 4.2. Hệ thống chính (Main System) | Đèn LED trạng thái (Status LED WS2812 / Single LED) | y |
| `CONFIG_CUSTOM_LED_WS2812` | bool | Đèn LED trạng thái (Status LED WS2812 / Single LED) | Đèn LED RGB kỹ thuật số WS2812 / SK6812 (Giao thức RMT) | y (mặc định của choice) |
| `CONFIG_CUSTOM_LED_SINGLE_PWM` | bool | Đèn LED trạng thái (Status LED WS2812 / Single LED) | Đèn LED đơn điều chỉnh độ sáng (LEDC PWM) |  |
| `CONFIG_CUSTOM_LED_CIRCULAR_STRIP` | bool | Đèn LED trạng thái (Status LED WS2812 / Single LED) | Vòng tròn LED xoay trạng thái (Circular LED Strip) |  |
| `CONFIG_CUSTOM_LED_USER_CUSTOM` | bool | Đèn LED trạng thái (Status LED WS2812 / Single LED) | Driver Đèn LED tùy chỉnh (Custom User LED Hook) |  |
| `CONFIG_CUSTOM_LED_NONE` | bool | Đèn LED trạng thái (Status LED WS2812 / Single LED) | Không sử dụng đèn LED |  |
| `CONFIG_CUSTOM_LED_GPIO` | int | Đèn LED trạng thái (Status LED WS2812 / Single LED) | Chân GPIO kết nối Đèn LED (Mặc định chân LED onboard) | 48 |
| `CONFIG_CUSTOM_LED_COUNT` | int | Đèn LED trạng thái (Status LED WS2812 / Single LED) | Số lượng mắt LED trên dải/vòng (Hỗ trợ tối đa 1024 mắt LED) | 1 |
| `CONFIG_CUSTOM_LED_BRIGHTNESS_NORMAL` | int | Đèn LED trạng thái (Status LED WS2812 / Single LED) | Độ sáng thường (%) | 10 |
| `CONFIG_CUSTOM_LED_BRIGHTNESS_LOW` | int | Đèn LED trạng thái (Status LED WS2812 / Single LED) | Độ sáng thấp (%) cho trạng thái Chờ | 2 |
| `CONFIG_CUSTOM_LED_BRIGHTNESS_HIGH` | int | Đèn LED trạng thái (Status LED WS2812 / Single LED) | Độ sáng cao (%) cho Lỗi và khi nghe thấy tiếng nói | 40 |
| `CONFIG_CUSTOM_LED_STATE_STARTING_EFFECT_OFF` | bool | Màu và hiệu ứng theo trạng thái | Tắt |  |
| `CONFIG_CUSTOM_LED_STATE_STARTING_EFFECT_STEADY` | bool | Màu và hiệu ứng theo trạng thái | Sáng đứng yên |  |
| `CONFIG_CUSTOM_LED_STATE_STARTING_EFFECT_BLINK_FAST` | bool | Màu và hiệu ứng theo trạng thái | Nháy nhanh (100 ms) | y (mặc định của choice) |
| `CONFIG_CUSTOM_LED_STATE_STARTING_EFFECT_BLINK_SLOW` | bool | Màu và hiệu ứng theo trạng thái | Nháy chậm (500 ms) |  |
| `CONFIG_CUSTOM_LED_STATE_STARTING_EFFECT_BREATHE` | bool | Màu và hiệu ứng theo trạng thái | Thở (sáng dần, tối dần) |  |
| `CONFIG_CUSTOM_LED_STATE_STARTING_EFFECT_RAINBOW` | bool | Màu và hiệu ứng theo trạng thái | Cầu vồng đổi màu |  |
| `CONFIG_CUSTOM_LED_STATE_STARTING_EFFECT_ID` | int | Màu và hiệu ứng theo trạng thái |  | 0 if CUSTOM_LED_STATE_STARTING_EFFECT_OFF |
| `CONFIG_CUSTOM_LED_STATE_STARTING_COLOR` | string | Màu và hiệu ứng theo trạng thái | Khởi động: màu (mã RRGGBB) | "0000FF" |
| `CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_EFFECT_OFF` | bool | Màu và hiệu ứng theo trạng thái | Tắt |  |
| `CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_EFFECT_STEADY` | bool | Màu và hiệu ứng theo trạng thái | Sáng đứng yên |  |
| `CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_EFFECT_BLINK_FAST` | bool | Màu và hiệu ứng theo trạng thái | Nháy nhanh (100 ms) |  |
| `CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_EFFECT_BLINK_SLOW` | bool | Màu và hiệu ứng theo trạng thái | Nháy chậm (500 ms) | y (mặc định của choice) |
| `CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_EFFECT_BREATHE` | bool | Màu và hiệu ứng theo trạng thái | Thở (sáng dần, tối dần) |  |
| `CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_EFFECT_RAINBOW` | bool | Màu và hiệu ứng theo trạng thái | Cầu vồng đổi màu |  |
| `CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_EFFECT_ID` | int | Màu và hiệu ứng theo trạng thái |  | 0 if CUSTOM_LED_STATE_WIFI_CONFIG_EFFECT_OFF |
| `CONFIG_CUSTOM_LED_STATE_WIFI_CONFIG_COLOR` | string | Màu và hiệu ứng theo trạng thái | Cấu hình WiFi: màu (mã RRGGBB) | "0000FF" |
| `CONFIG_CUSTOM_LED_STATE_IDLE_EFFECT_OFF` | bool | Màu và hiệu ứng theo trạng thái | Tắt | y (mặc định của choice) |
| `CONFIG_CUSTOM_LED_STATE_IDLE_EFFECT_STEADY` | bool | Màu và hiệu ứng theo trạng thái | Sáng đứng yên |  |
| `CONFIG_CUSTOM_LED_STATE_IDLE_EFFECT_BLINK_FAST` | bool | Màu và hiệu ứng theo trạng thái | Nháy nhanh (100 ms) |  |
| `CONFIG_CUSTOM_LED_STATE_IDLE_EFFECT_BLINK_SLOW` | bool | Màu và hiệu ứng theo trạng thái | Nháy chậm (500 ms) |  |
| `CONFIG_CUSTOM_LED_STATE_IDLE_EFFECT_BREATHE` | bool | Màu và hiệu ứng theo trạng thái | Thở (sáng dần, tối dần) |  |
| `CONFIG_CUSTOM_LED_STATE_IDLE_EFFECT_RAINBOW` | bool | Màu và hiệu ứng theo trạng thái | Cầu vồng đổi màu |  |
| `CONFIG_CUSTOM_LED_STATE_IDLE_EFFECT_ID` | int | Màu và hiệu ứng theo trạng thái |  | 0 if CUSTOM_LED_STATE_IDLE_EFFECT_OFF |
| `CONFIG_CUSTOM_LED_STATE_IDLE_COLOR` | string | Màu và hiệu ứng theo trạng thái | Chờ: màu (mã RRGGBB) | "0000FF" |
| `CONFIG_CUSTOM_LED_STATE_CONNECTING_EFFECT_OFF` | bool | Màu và hiệu ứng theo trạng thái | Tắt |  |
| `CONFIG_CUSTOM_LED_STATE_CONNECTING_EFFECT_STEADY` | bool | Màu và hiệu ứng theo trạng thái | Sáng đứng yên | y (mặc định của choice) |
| `CONFIG_CUSTOM_LED_STATE_CONNECTING_EFFECT_BLINK_FAST` | bool | Màu và hiệu ứng theo trạng thái | Nháy nhanh (100 ms) |  |
| `CONFIG_CUSTOM_LED_STATE_CONNECTING_EFFECT_BLINK_SLOW` | bool | Màu và hiệu ứng theo trạng thái | Nháy chậm (500 ms) |  |
| `CONFIG_CUSTOM_LED_STATE_CONNECTING_EFFECT_BREATHE` | bool | Màu và hiệu ứng theo trạng thái | Thở (sáng dần, tối dần) |  |
| `CONFIG_CUSTOM_LED_STATE_CONNECTING_EFFECT_RAINBOW` | bool | Màu và hiệu ứng theo trạng thái | Cầu vồng đổi màu |  |
| `CONFIG_CUSTOM_LED_STATE_CONNECTING_EFFECT_ID` | int | Màu và hiệu ứng theo trạng thái |  | 0 if CUSTOM_LED_STATE_CONNECTING_EFFECT_OFF |
| `CONFIG_CUSTOM_LED_STATE_CONNECTING_COLOR` | string | Màu và hiệu ứng theo trạng thái | Đang kết nối: màu (mã RRGGBB) | "0000FF" |
| `CONFIG_CUSTOM_LED_STATE_LISTENING_EFFECT_OFF` | bool | Màu và hiệu ứng theo trạng thái | Tắt |  |
| `CONFIG_CUSTOM_LED_STATE_LISTENING_EFFECT_STEADY` | bool | Màu và hiệu ứng theo trạng thái | Sáng đứng yên | y (mặc định của choice) |
| `CONFIG_CUSTOM_LED_STATE_LISTENING_EFFECT_BLINK_FAST` | bool | Màu và hiệu ứng theo trạng thái | Nháy nhanh (100 ms) |  |
| `CONFIG_CUSTOM_LED_STATE_LISTENING_EFFECT_BLINK_SLOW` | bool | Màu và hiệu ứng theo trạng thái | Nháy chậm (500 ms) |  |
| `CONFIG_CUSTOM_LED_STATE_LISTENING_EFFECT_BREATHE` | bool | Màu và hiệu ứng theo trạng thái | Thở (sáng dần, tối dần) |  |
| `CONFIG_CUSTOM_LED_STATE_LISTENING_EFFECT_RAINBOW` | bool | Màu và hiệu ứng theo trạng thái | Cầu vồng đổi màu |  |
| `CONFIG_CUSTOM_LED_STATE_LISTENING_EFFECT_ID` | int | Màu và hiệu ứng theo trạng thái |  | 0 if CUSTOM_LED_STATE_LISTENING_EFFECT_OFF |
| `CONFIG_CUSTOM_LED_STATE_LISTENING_COLOR` | string | Màu và hiệu ứng theo trạng thái | Đang nghe: màu (mã RRGGBB) | "FF0000" |
| `CONFIG_CUSTOM_LED_STATE_SPEAKING_EFFECT_OFF` | bool | Màu và hiệu ứng theo trạng thái | Tắt |  |
| `CONFIG_CUSTOM_LED_STATE_SPEAKING_EFFECT_STEADY` | bool | Màu và hiệu ứng theo trạng thái | Sáng đứng yên | y (mặc định của choice) |
| `CONFIG_CUSTOM_LED_STATE_SPEAKING_EFFECT_BLINK_FAST` | bool | Màu và hiệu ứng theo trạng thái | Nháy nhanh (100 ms) |  |
| `CONFIG_CUSTOM_LED_STATE_SPEAKING_EFFECT_BLINK_SLOW` | bool | Màu và hiệu ứng theo trạng thái | Nháy chậm (500 ms) |  |
| `CONFIG_CUSTOM_LED_STATE_SPEAKING_EFFECT_BREATHE` | bool | Màu và hiệu ứng theo trạng thái | Thở (sáng dần, tối dần) |  |
| `CONFIG_CUSTOM_LED_STATE_SPEAKING_EFFECT_RAINBOW` | bool | Màu và hiệu ứng theo trạng thái | Cầu vồng đổi màu |  |
| `CONFIG_CUSTOM_LED_STATE_SPEAKING_EFFECT_ID` | int | Màu và hiệu ứng theo trạng thái |  | 0 if CUSTOM_LED_STATE_SPEAKING_EFFECT_OFF |
| `CONFIG_CUSTOM_LED_STATE_SPEAKING_COLOR` | string | Màu và hiệu ứng theo trạng thái | Đang nói: màu (mã RRGGBB) | "00FF00" |
| `CONFIG_CUSTOM_LED_STATE_UPGRADING_EFFECT_OFF` | bool | Màu và hiệu ứng theo trạng thái | Tắt |  |
| `CONFIG_CUSTOM_LED_STATE_UPGRADING_EFFECT_STEADY` | bool | Màu và hiệu ứng theo trạng thái | Sáng đứng yên |  |
| `CONFIG_CUSTOM_LED_STATE_UPGRADING_EFFECT_BLINK_FAST` | bool | Màu và hiệu ứng theo trạng thái | Nháy nhanh (100 ms) | y (mặc định của choice) |
| `CONFIG_CUSTOM_LED_STATE_UPGRADING_EFFECT_BLINK_SLOW` | bool | Màu và hiệu ứng theo trạng thái | Nháy chậm (500 ms) |  |
| `CONFIG_CUSTOM_LED_STATE_UPGRADING_EFFECT_BREATHE` | bool | Màu và hiệu ứng theo trạng thái | Thở (sáng dần, tối dần) |  |
| `CONFIG_CUSTOM_LED_STATE_UPGRADING_EFFECT_RAINBOW` | bool | Màu và hiệu ứng theo trạng thái | Cầu vồng đổi màu |  |
| `CONFIG_CUSTOM_LED_STATE_UPGRADING_EFFECT_ID` | int | Màu và hiệu ứng theo trạng thái |  | 0 if CUSTOM_LED_STATE_UPGRADING_EFFECT_OFF |
| `CONFIG_CUSTOM_LED_STATE_UPGRADING_COLOR` | string | Màu và hiệu ứng theo trạng thái | Nâng cấp firmware: màu (mã RRGGBB) | "00FF00" |
| `CONFIG_CUSTOM_LED_STATE_ACTIVATING_EFFECT_OFF` | bool | Màu và hiệu ứng theo trạng thái | Tắt |  |
| `CONFIG_CUSTOM_LED_STATE_ACTIVATING_EFFECT_STEADY` | bool | Màu và hiệu ứng theo trạng thái | Sáng đứng yên |  |
| `CONFIG_CUSTOM_LED_STATE_ACTIVATING_EFFECT_BLINK_FAST` | bool | Màu và hiệu ứng theo trạng thái | Nháy nhanh (100 ms) |  |
| `CONFIG_CUSTOM_LED_STATE_ACTIVATING_EFFECT_BLINK_SLOW` | bool | Màu và hiệu ứng theo trạng thái | Nháy chậm (500 ms) | y (mặc định của choice) |
| `CONFIG_CUSTOM_LED_STATE_ACTIVATING_EFFECT_BREATHE` | bool | Màu và hiệu ứng theo trạng thái | Thở (sáng dần, tối dần) |  |
| `CONFIG_CUSTOM_LED_STATE_ACTIVATING_EFFECT_RAINBOW` | bool | Màu và hiệu ứng theo trạng thái | Cầu vồng đổi màu |  |
| `CONFIG_CUSTOM_LED_STATE_ACTIVATING_EFFECT_ID` | int | Màu và hiệu ứng theo trạng thái |  | 0 if CUSTOM_LED_STATE_ACTIVATING_EFFECT_OFF |
| `CONFIG_CUSTOM_LED_STATE_ACTIVATING_COLOR` | string | Màu và hiệu ứng theo trạng thái | Kích hoạt thiết bị: màu (mã RRGGBB) | "00FF00" |
| `CONFIG_CUSTOM_LED_STATE_ERROR_EFFECT_OFF` | bool | Màu và hiệu ứng theo trạng thái | Tắt |  |
| `CONFIG_CUSTOM_LED_STATE_ERROR_EFFECT_STEADY` | bool | Màu và hiệu ứng theo trạng thái | Sáng đứng yên |  |
| `CONFIG_CUSTOM_LED_STATE_ERROR_EFFECT_BLINK_FAST` | bool | Màu và hiệu ứng theo trạng thái | Nháy nhanh (100 ms) | y (mặc định của choice) |
| `CONFIG_CUSTOM_LED_STATE_ERROR_EFFECT_BLINK_SLOW` | bool | Màu và hiệu ứng theo trạng thái | Nháy chậm (500 ms) |  |
| `CONFIG_CUSTOM_LED_STATE_ERROR_EFFECT_BREATHE` | bool | Màu và hiệu ứng theo trạng thái | Thở (sáng dần, tối dần) |  |
| `CONFIG_CUSTOM_LED_STATE_ERROR_EFFECT_RAINBOW` | bool | Màu và hiệu ứng theo trạng thái | Cầu vồng đổi màu |  |
| `CONFIG_CUSTOM_LED_STATE_ERROR_EFFECT_ID` | int | Màu và hiệu ứng theo trạng thái |  | 0 if CUSTOM_LED_STATE_ERROR_EFFECT_OFF |
| `CONFIG_CUSTOM_LED_STATE_ERROR_COLOR` | string | Màu và hiệu ứng theo trạng thái | Lỗi: màu (mã RRGGBB) | "FF0000" |
| `CONFIG_CUSTOM_LED_EXTRA_GPIOS` | string | LED đơn GPIO (chatbot điều khiển) | Các chân LED đơn (cách nhau dấu phẩy, để trống là không dùng) | "" |
| `CONFIG_CUSTOM_GPIO_LED_BRIGHTNESS` | int | LED đơn GPIO (chatbot điều khiển) | Độ sáng mặc định (%) | 100 |
| `CONFIG_CUSTOM_GPIO_LED_BOOT_EFFECT_OFF` | bool | LED đơn GPIO (chatbot điều khiển) | Tắt | y (mặc định của choice) |
| `CONFIG_CUSTOM_GPIO_LED_BOOT_EFFECT_STEADY` | bool | LED đơn GPIO (chatbot điều khiển) | Sáng đứng yên |  |
| `CONFIG_CUSTOM_GPIO_LED_BOOT_EFFECT_BLINK` | bool | LED đơn GPIO (chatbot điều khiển) | Nháy |  |
| `CONFIG_CUSTOM_GPIO_LED_BOOT_EFFECT_BREATHE` | bool | LED đơn GPIO (chatbot điều khiển) | Thở (LED không chỉnh độ sáng được sẽ nháy) |  |
| `CONFIG_CUSTOM_GPIO_LED_BOOT_EFFECT_ID` | int | LED đơn GPIO (chatbot điều khiển) |  | 0 if CUSTOM_GPIO_LED_BOOT_EFFECT_OFF |
| `CONFIG_CUSTOM_GPIO_LED_PERIOD_MS` | int | LED đơn GPIO (chatbot điều khiển) | Chu kỳ nháy/thở mặc định (ms) | 1000 |
| `CONFIG_CUSTOM_ENABLE_PERIPH_LED_DRIVER_IC` | bool | 4.2. Hệ thống chính (Main System) | LED qua IC điều khiển LED I2C (AW9523B 16 kênh / IS31FL3236 36 kênh) | n |
| `CONFIG_CUSTOM_PERIPH_LED_IC_I2C_SDA` | int | LED qua IC điều khiển LED I2C (AW9523B 16 kênh / IS31FL3236 36 kênh) | Chân I2C SDA chip điều khiển LED | -1 |
| `CONFIG_CUSTOM_PERIPH_LED_IC_I2C_SCL` | int | LED qua IC điều khiển LED I2C (AW9523B 16 kênh / IS31FL3236 36 kênh) | Chân I2C SCL chip điều khiển LED | -1 |
| `CONFIG_CUSTOM_LED_IC_LED_CHANNELS` | string | LED qua IC điều khiển LED I2C (AW9523B 16 kênh / IS31FL3236 36 kênh) | Kênh dùng làm LED (cách nhau dấu phẩy, để trống là tất cả kênh) | "" |
| `CONFIG_CUSTOM_LED_IC_LED_BRIGHTNESS` | int | LED qua IC điều khiển LED I2C (AW9523B 16 kênh / IS31FL3236 36 kênh) | Độ sáng mặc định (%) | 100 |
| `CONFIG_CUSTOM_LED_IC_LED_BOOT_EFFECT_OFF` | bool | LED qua IC điều khiển LED I2C (AW9523B 16 kênh / IS31FL3236 36 kênh) | Tắt | y (mặc định của choice) |
| `CONFIG_CUSTOM_LED_IC_LED_BOOT_EFFECT_STEADY` | bool | LED qua IC điều khiển LED I2C (AW9523B 16 kênh / IS31FL3236 36 kênh) | Sáng đứng yên |  |
| `CONFIG_CUSTOM_LED_IC_LED_BOOT_EFFECT_BLINK` | bool | LED qua IC điều khiển LED I2C (AW9523B 16 kênh / IS31FL3236 36 kênh) | Nháy |  |
| `CONFIG_CUSTOM_LED_IC_LED_BOOT_EFFECT_BREATHE` | bool | LED qua IC điều khiển LED I2C (AW9523B 16 kênh / IS31FL3236 36 kênh) | Thở (LED không chỉnh độ sáng được sẽ nháy) |  |
| `CONFIG_CUSTOM_LED_IC_LED_BOOT_EFFECT_ID` | int | LED qua IC điều khiển LED I2C (AW9523B 16 kênh / IS31FL3236 36 kênh) |  | 0 if CUSTOM_LED_IC_LED_BOOT_EFFECT_OFF |
| `CONFIG_CUSTOM_LED_IC_LED_PERIOD_MS` | int | LED qua IC điều khiển LED I2C (AW9523B 16 kênh / IS31FL3236 36 kênh) | Chu kỳ nháy/thở mặc định (ms) | 1000 |
| `CONFIG_CUSTOM_PCA9685_LED_CHANNELS` | string | LED qua PCA9685 (16 kênh PWM) | Kênh PCA9685 dùng làm LED (0-15, cách nhau dấu phẩy, để trống là không dùng) | "" |
| `CONFIG_CUSTOM_PCA9685_LED_BRIGHTNESS` | int | LED qua PCA9685 (16 kênh PWM) | Độ sáng mặc định (%) | 100 |
| `CONFIG_CUSTOM_PCA9685_LED_BOOT_EFFECT_OFF` | bool | LED qua PCA9685 (16 kênh PWM) | Tắt | y (mặc định của choice) |
| `CONFIG_CUSTOM_PCA9685_LED_BOOT_EFFECT_STEADY` | bool | LED qua PCA9685 (16 kênh PWM) | Sáng đứng yên |  |
| `CONFIG_CUSTOM_PCA9685_LED_BOOT_EFFECT_BLINK` | bool | LED qua PCA9685 (16 kênh PWM) | Nháy |  |
| `CONFIG_CUSTOM_PCA9685_LED_BOOT_EFFECT_BREATHE` | bool | LED qua PCA9685 (16 kênh PWM) | Thở (LED không chỉnh độ sáng được sẽ nháy) |  |
| `CONFIG_CUSTOM_PCA9685_LED_BOOT_EFFECT_ID` | int | LED qua PCA9685 (16 kênh PWM) |  | 0 if CUSTOM_PCA9685_LED_BOOT_EFFECT_OFF |
| `CONFIG_CUSTOM_PCA9685_LED_PERIOD_MS` | int | LED qua PCA9685 (16 kênh PWM) | Chu kỳ nháy/thở mặc định (ms) | 1000 |
| `CONFIG_CUSTOM_EXPANDER_LED_PINS` | string | LED qua IC mở rộng GPIO (TCA9554 / TCA9555 / TCA9539) | Chân IC mở rộng dùng làm LED (cách nhau dấu phẩy, để trống là không dùng) | "" |
| `CONFIG_CUSTOM_EXPANDER_LED_BRIGHTNESS` | int | LED qua IC mở rộng GPIO (TCA9554 / TCA9555 / TCA9539) | Độ sáng mặc định (%) | 100 |
| `CONFIG_CUSTOM_EXPANDER_LED_BOOT_EFFECT_OFF` | bool | LED qua IC mở rộng GPIO (TCA9554 / TCA9555 / TCA9539) | Tắt | y (mặc định của choice) |
| `CONFIG_CUSTOM_EXPANDER_LED_BOOT_EFFECT_STEADY` | bool | LED qua IC mở rộng GPIO (TCA9554 / TCA9555 / TCA9539) | Sáng đứng yên |  |
| `CONFIG_CUSTOM_EXPANDER_LED_BOOT_EFFECT_BLINK` | bool | LED qua IC mở rộng GPIO (TCA9554 / TCA9555 / TCA9539) | Nháy |  |
| `CONFIG_CUSTOM_EXPANDER_LED_BOOT_EFFECT_BREATHE` | bool | LED qua IC mở rộng GPIO (TCA9554 / TCA9555 / TCA9539) | Thở (LED không chỉnh độ sáng được sẽ nháy) |  |
| `CONFIG_CUSTOM_EXPANDER_LED_BOOT_EFFECT_ID` | int | LED qua IC mở rộng GPIO (TCA9554 / TCA9555 / TCA9539) |  | 0 if CUSTOM_EXPANDER_LED_BOOT_EFFECT_OFF |
| `CONFIG_CUSTOM_EXPANDER_LED_PERIOD_MS` | int | LED qua IC mở rộng GPIO (TCA9554 / TCA9555 / TCA9539) | Chu kỳ nháy/thở mặc định (ms) | 1000 |
| `CONFIG_CUSTOM_ENABLE_SERVO_DOG` | bool | 4.3. Âm Thanh Cảnh Báo & Xúc Giác (Audio & Haptics) | Động cơ Servo tương tác qua PWM (Servo Dog Controller) | n |
| `CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO` | int | 4.3. Âm Thanh Cảnh Báo & Xúc Giác (Audio & Haptics) | Chân GPIO cấp xung PWM cho Động cơ Servo | -1 |
| `CONFIG_ENABLE_BUZZER` | bool | 4.3. Âm Thanh Cảnh Báo & Xúc Giác (Audio & Haptics) | Còi báo động Buzzer (GPIO 41) | n |
| `CONFIG_BUZZER_EXTRA_PINS` | string | 4.3. Âm Thanh Cảnh Báo & Xúc Giác (Audio & Haptics) | Các chân GPIO Còi thêm (cách nhau dấu phẩy) | "" |
| `CONFIG_BUZZER_PIN` | int | 4.3. Âm Thanh Cảnh Báo & Xúc Giác (Audio & Haptics) | Chân GPIO Buzzer | -1 |
| `CONFIG_ENABLE_HAPTIC_MOTOR` | bool | 4.3. Âm Thanh Cảnh Báo & Xúc Giác (Audio & Haptics) | Động cơ rung phản hồi xúc giác (GPIO 42) | n |
| `CONFIG_HAPTIC_PIN` | int | 4.3. Âm Thanh Cảnh Báo & Xúc Giác (Audio & Haptics) | Chân GPIO Động cơ rung | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_DHT11_22` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Cảm biến Nhiệt độ & Độ ẩm 1 dây (DHT11 / DHT22) | n |
| `CONFIG_CUSTOM_SENSOR_DHT_EXTRA_GPIOS` | string | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Các chân GPIO DHT thêm (cách nhau dấu phẩy) | "" |
| `CONFIG_CUSTOM_SENSOR_DHT_GPIO` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân dữ liệu 1-Wire cảm biến DHT GPIO | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_I2C_TEMP_HUMID` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Cảm biến Nhiệt độ & Độ ẩm kỹ thuật số I2C (AHT20 / SHT3x) | n |
| `CONFIG_CUSTOM_ENABLE_SENSOR_AHT20` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Aosong AHT10 / AHT20 / AHT21 (I2C) | y |
| `CONFIG_CUSTOM_SENSOR_AHT20_EXTRA_GPIOS` | string | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Các chân GPIO AHT20 thêm (cách nhau dấu phẩy) | "" |
| `CONFIG_CUSTOM_SENSOR_I2C_SDA` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SDA cảm biến nhiệt ẩm | -1 |
| `CONFIG_CUSTOM_SENSOR_I2C_SCL` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SCL cảm biến nhiệt ẩm | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_BMP280` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Cảm biến Áp suất khí quyển & Độ cao (Bosch BMP280 / BME280 I2C) | n |
| `CONFIG_CUSTOM_SENSOR_BMP280_EXTRA_GPIOS` | string | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Các chân GPIO BMP280 thêm (cách nhau dấu phẩy) | "" |
| `CONFIG_CUSTOM_SENSOR_BMP280_I2C_SDA` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SDA cảm biến khí áp BMP280 | -1 |
| `CONFIG_CUSTOM_SENSOR_BMP280_I2C_SCL` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SCL cảm biến khí áp BMP280 | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_BH1750` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Cảm biến cường độ ánh sáng kỹ thuật số BH1750 (I2C) | n |
| `CONFIG_CUSTOM_SENSOR_BH1750_EXTRA_GPIOS` | string | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Các chân GPIO BH1750 thêm (cách nhau dấu phẩy) | "" |
| `CONFIG_CUSTOM_SENSOR_BH1750_I2C_SDA` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SDA cảm biến ánh sáng BH1750 | -1 |
| `CONFIG_CUSTOM_SENSOR_BH1750_I2C_SCL` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SCL cảm biến ánh sáng BH1750 | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_LDR` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Cảm biến ánh sáng quang trở (LDR qua cầu phân áp ADC) | n |
| `CONFIG_CUSTOM_SENSOR_LDR_EXTRA_CHS` | string | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Các kênh ADC quang trở LDR thêm (cách nhau dấu phẩy) | "" |
| `CONFIG_CUSTOM_SENSOR_LDR_ADC_CHANNEL` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Kênh ADC1 đo quang trở LDR (Kênh N ứng với GPIO N+1, -1 chưa chọn) | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_GAS_CO2` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Cảm biến nồng độ Khí độc TVOC & CO2 NDIR (SCD40/41, SGP30/40 I2C) | n |
| `CONFIG_CUSTOM_SENSOR_GAS_SCD4X` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Sensirion SCD40 / SCD41 (Cảm biến CO2 NDIR I2C) | y |
| `CONFIG_CUSTOM_SENSOR_SCD4X_EXTRA_GPIOS` | string | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Các chân GPIO SCD4X thêm (cách nhau dấu phẩy) | "" |
| `CONFIG_CUSTOM_SENSOR_GAS_I2C_SDA` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SDA cảm biến khí/CO2 | -1 |
| `CONFIG_CUSTOM_SENSOR_GAS_I2C_SCL` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SCL cảm biến khí/CO2 | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_GAS_ANALOG_MQ` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Cảm biến khói & khí gas dễ cháy (MQ-2 / MQ-135 Analog ADC) | n |
| `CONFIG_CUSTOM_SENSOR_MQ_ANALOG_PIN` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân analog đọc điện áp cảm biến MQ (kênh ADC1) | -1 |
| `CONFIG_CUSTOM_SENSOR_DS18B20_PIN` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân dữ liệu 1-Wire cảm biến nhiệt độ DS18B20 | -1 |
| `CONFIG_CUSTOM_SENSOR_FLOW_PULSE_PIN` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân đọc xung lưu lượng chất lỏng (Flow Pulse) | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_APDS9960` | bool | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Cảm biến màu sắc, cử chỉ & tiệm cận (Broadcom APDS-9960 I2C) | n |
| `CONFIG_CUSTOM_SENSOR_APDS9960_I2C_SDA` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân I2C SDA cảm biến APDS9960 | -1 |
| `CONFIG_CUSTOM_SENSOR_APDS9960_I2C_SCL` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân I2C SCL cảm biến APDS9960 | -1 |
| `CONFIG_CUSTOM_SENSOR_APDS9960_INT_PIN` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân ngắt INT cảm biến APDS9960 | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_VL53LX` | bool | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Cảm biến khoảng cách laser ToF (STMicroelectronics VL53L0X / VL53L1X I2C) | n |
| `CONFIG_CUSTOM_SENSOR_VL53LX_I2C_SDA` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân I2C SDA cảm biến laser VL53L | -1 |
| `CONFIG_CUSTOM_SENSOR_VL53LX_I2C_SCL` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân I2C SCL cảm biến laser VL53L | -1 |
| `CONFIG_CUSTOM_SENSOR_VL53LX_XSHUT_PIN` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân tắt nguồn XSHUT cảm biến laser VL53L | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_HCSR04` | bool | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Cảm biến khoảng cách siêu âm (HC-SR04 / US-100) | n |
| `CONFIG_CUSTOM_SENSOR_HCSR04_TRIG_GPIO` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân phát xung siêu âm Trig GPIO | -1 |
| `CONFIG_CUSTOM_SENSOR_HCSR04_ECHO_GPIO` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân nhận tín hiệu siêu âm Echo GPIO | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_PIR` | bool | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Cảm biến phát hiện người chuyển động PIR (HC-SR501 / RCWL-0516) | n |
| `CONFIG_CUSTOM_SENSOR_PIR_GPIO` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân tín hiệu cảm biến PIR GPIO | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_VIBRATION_SW420` | bool | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Cảm biến cảnh báo chấn động / Rung tiếp điểm (SW-420 GPIO Interrupt) | n |
| `CONFIG_CUSTOM_SENSOR_VIBRATION_PIN` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân tín hiệu Digital Out cảm biến rung | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_FLAME` | bool | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Cảm biến cảnh báo hỏa hoạn / Ngọn lửa (Flame Sensor IR) | n |
| `CONFIG_CUSTOM_SENSOR_FLAME_PIN` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân tín hiệu Digital báo lửa | -1 |
| `CONFIG_CUSTOM_PERIPH_RELAY_ENABLE` | bool | 4.6. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Rơ-le đóng cắt thiết bị độc lập (Relay Switch) | n |
| `CONFIG_CUSTOM_PERIPH_RELAY_GPIO` | int | 4.6. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Chân GPIO kích hoạt Relay đóng cắt | -1 |
| `CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE` | bool | 4.6. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Mạch cầu H Động cơ DC 2 chiều cho xe robot (TB6612FNG / L9110S / DRV8833) | n |
| `CONFIG_CUSTOM_MOTOR_DRIVER_TB6612` | bool | 4.6. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Toshiba TB6612FNG (Hiệu suất cao MOSFET H-Bridge) | y (mặc định của choice) |
| `CONFIG_CUSTOM_MOTOR_DRIVER_L9110S` | bool | 4.6. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | L9110S / HG7881 (Mạch 2 kênh DC mini giá rẻ) |  |
| `CONFIG_CUSTOM_MOTOR_DRIVER_DRV8833` | bool | 4.6. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | TI DRV8833 (Cầu H kép bảo vệ quá dòng) |  |
| `CONFIG_CUSTOM_PERIPH_MOTOR_PWMA_PIN` | int | 4.6. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Chân điều tốc PWM Động cơ A (PWMA) | -1 |
| `CONFIG_CUSTOM_PERIPH_MOTOR_DIRA_PIN` | int | 4.6. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Chân hướng quay Động cơ A (DIRA / IN1) | -1 |
| `CONFIG_CUSTOM_PERIPH_MOTOR_PWMB_PIN` | int | 4.6. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Chân điều tốc PWM Động cơ B (PWMB) | -1 |
| `CONFIG_CUSTOM_PERIPH_MOTOR_DIRB_PIN` | int | 4.6. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Chân hướng quay Động cơ B (DIRB / IN3) | -1 |
| `CONFIG_CUSTOM_ENABLE_BATTERY_MONITOR` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Mạch đo dung lượng Pin (Battery Level Monitor) | n |
| `CONFIG_CUSTOM_BATTERY_MONITOR_ADC` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Đo qua ADC nội bộ (Bộ chia điện trở Voltage Divider) | y |
| `CONFIG_CUSTOM_BATTERY_ADC_CHANNEL` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Kênh ADC1 đo pin (Kênh N ứng với GPIO N+1, -1 chưa chọn) | -1 |
| `CONFIG_CUSTOM_BATTERY_DIVIDER_R1` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Điện trở nhánh trên R1 cầu phân áp (kOhm) | 100 |
| `CONFIG_CUSTOM_BATTERY_DIVIDER_R2` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Điện trở nhánh dưới R2 cầu phân áp (kOhm) | 100 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_INA2XX` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Module đo điện áp, dòng điện & công suất tiêu thụ (TI INA219 / INA226 I2C) | n |
| `CONFIG_CUSTOM_SENSOR_INA219` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Texas Instruments INA219 (Điện áp tối đa 26V, độ phân giải 12-bit) | y |
| `CONFIG_CUSTOM_SENSOR_INA2XX_I2C_SDA` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SDA module đo dòng INA | -1 |
| `CONFIG_CUSTOM_SENSOR_INA2XX_I2C_SCL` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SCL module đo dòng INA | -1 |
| `CONFIG_CUSTOM_ENABLE_PERIPH_BATTERY_CHARGING_DETECT` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Giám sát trạng thái sạc pin (TP4056 CHRG / STDBY Pin) | n |
| `CONFIG_CUSTOM_PERIPH_BATTERY_CHRG_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân đọc tín hiệu sạc (kéo trở Pull-up) | -1 |
| `CONFIG_CUSTOM_BATTERY_MONITOR_BQ27220` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Mạch giám sát pin chuyên dụng I2C (TI BQ27220) | n |
| `CONFIG_CUSTOM_BATTERY_BQ27220_I2C_SDA` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SDA IC đo pin BQ27220 | -1 |
| `CONFIG_CUSTOM_BATTERY_BQ27220_I2C_SCL` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SCL IC đo pin BQ27220 | -1 |
| `CONFIG_CUSTOM_PMIC_I2C_SDA` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SDA chip quản lý nguồn PMIC | -1 |
| `CONFIG_CUSTOM_PMIC_I2C_SCL` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SCL chip quản lý nguồn PMIC | -1 |
| `CONFIG_CUSTOM_PMIC_INT_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân ngắt INT chip quản lý nguồn PMIC | -1 |
| `CONFIG_CUSTOM_IMU_I2C_SDA` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SDA cảm biến chuyển động IMU | -1 |
| `CONFIG_CUSTOM_IMU_I2C_SCL` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SCL cảm biến chuyển động IMU | -1 |
| `CONFIG_CUSTOM_EXPANDER_I2C_SDA` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SDA chip mở rộng GPIO (IO Expander) | -1 |
| `CONFIG_CUSTOM_EXPANDER_I2C_SCL` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SCL chip mở rộng GPIO (IO Expander) | -1 |
| `CONFIG_CUSTOM_EXPANDER_INT_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân ngắt INT chip mở rộng GPIO | -1 |
| `CONFIG_CUSTOM_EXPANDER_RST_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân reset RST chip mở rộng GPIO | -1 |
| `CONFIG_CUSTOM_EXPANDER_CHIP_TCA9554` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | TCA9554 / PCA9554 / TCA9554A (8 chân) | y (mặc định của choice) |
| `CONFIG_CUSTOM_EXPANDER_CHIP_TCA95XX_16BIT` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | TCA9555 / PCA9555 / TCA9535 / TCA9539 (16 chân) |  |
| `CONFIG_CUSTOM_EXPANDER_I2C_ADDR` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Địa chỉ I2C chip mở rộng GPIO (thập phân) | 32 |
| `CONFIG_CUSTOM_ENABLE_PERIPH_RTC` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Đồng hồ thời gian thực ngoại vi (RTC I2C DS3231 / PCF8563) | n |
| `CONFIG_CUSTOM_PERIPH_RTC_I2C_SDA` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SDA module RTC | -1 |
| `CONFIG_CUSTOM_PERIPH_RTC_I2C_SCL` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SCL module RTC | -1 |
| `CONFIG_CUSTOM_PERIPH_RTC_INT_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân ngắt báo thức INT module RTC | -1 |
| `CONFIG_CUSTOM_ENABLE_PERIPH_NFC_PN532` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Đầu đọc thẻ từ giao tiếp gần NFC (NXP PN532 I2C) | n |
| `CONFIG_CUSTOM_PERIPH_NFC_I2C_SDA` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SDA module NFC | -1 |
| `CONFIG_CUSTOM_PERIPH_NFC_I2C_SCL` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SCL module NFC | -1 |
| `CONFIG_CUSTOM_PERIPH_NFC_IRQ_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân ngắt IRQ module NFC | -1 |
| `CONFIG_CUSTOM_PERIPH_NFC_RST_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân reset RST module NFC | -1 |
| `CONFIG_CUSTOM_ENABLE_PERIPH_PCA9685` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Mạch mở rộng 16 kênh PWM I2C (NXP PCA9685) | n |
| `CONFIG_CUSTOM_PERIPH_PCA9685_I2C_SDA` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SDA module PCA9685 | -1 |
| `CONFIG_CUSTOM_PERIPH_PCA9685_I2C_SCL` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SCL module PCA9685 | -1 |
| `CONFIG_CUSTOM_PERIPH_PCA9685_OE_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân cho phép đầu ra OE module PCA9685 | -1 |
| `CONFIG_CUSTOM_ENABLE_PERIPH_SDCARD_SPI` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Đầu đọc thẻ nhớ MicroSD qua giao tiếp SPI | n |
| `CONFIG_CUSTOM_PERIPH_SDCARD_SPI_SCK` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân xung nhịp SPI SCK thẻ nhớ | -1 |
| `CONFIG_CUSTOM_PERIPH_SDCARD_SPI_MOSI` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân dữ liệu SPI MOSI thẻ nhớ | -1 |
| `CONFIG_CUSTOM_PERIPH_SDCARD_SPI_MISO` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân dữ liệu SPI MISO thẻ nhớ | -1 |
| `CONFIG_CUSTOM_PERIPH_SDCARD_SPI_CS` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân chọn chip SPI CS thẻ nhớ | -1 |
| `CONFIG_CUSTOM_PERIPH_RC522_SPI_SCK` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân xung nhịp SPI SCK module RFID RC522 | -1 |
| `CONFIG_CUSTOM_PERIPH_RC522_SPI_MOSI` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân dữ liệu SPI MOSI module RFID RC522 | -1 |
| `CONFIG_CUSTOM_PERIPH_RC522_SPI_MISO` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân dữ liệu SPI MISO module RFID RC522 | -1 |
| `CONFIG_CUSTOM_PERIPH_RC522_SPI_CS` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân chọn chip SPI CS module RFID RC522 | -1 |
| `CONFIG_CUSTOM_PERIPH_RC522_RST_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân reset RST module RFID RC522 | -1 |
| `CONFIG_CUSTOM_ETH_SPI_SCK_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân SCK SPI module Ethernet (W5500) | -1 |
| `CONFIG_CUSTOM_ETH_SPI_MOSI_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân MOSI SPI module Ethernet (W5500) | -1 |
| `CONFIG_CUSTOM_ETH_SPI_MISO_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân MISO SPI module Ethernet (W5500) | -1 |
| `CONFIG_CUSTOM_ETH_SPI_CS_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân chọn chip SPI CS module Ethernet (W5500) | -1 |
| `CONFIG_CUSTOM_ETH_SPI_INT_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân ngắt INT module Ethernet | -1 |
| `CONFIG_CUSTOM_ETH_SPI_RST_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân reset RST module Ethernet | -1 |
| `CONFIG_CUSTOM_ENABLE_PERIPH_IR_REMOTE` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Module truyền nhận hồng ngoại điều khiển từ xa (IR Remote) - CHƯA HỖ TRỢ, chưa có driver | n |
| `CONFIG_CUSTOM_PERIPH_IR_TX_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân phát hồng ngoại IR TX (38kHz) | -1 |
| `CONFIG_CUSTOM_PERIPH_IR_RX_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân thu hồng ngoại IR RX (VS1838B) | -1 |
| `CONFIG_CUSTOM_PERIPH_TWAI_TX_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân phát mạng CAN / TWAI TX | -1 |
| `CONFIG_CUSTOM_PERIPH_TWAI_RX_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân nhận mạng CAN / TWAI RX | -1 |
| `CONFIG_CUSTOM_ENABLE_PERIPH_CELLULAR_4G_LTE` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Module truyền thông dữ liệu mạng di động 4G LTE (A7670 / SIM7600) - CHƯA HỖ TRỢ, chưa có driver | n |
| `CONFIG_CUSTOM_PERIPH_4G_PWRKEY_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân bật nguồn PWRKEY module 4G | -1 |
| `CONFIG_CUSTOM_PERIPH_4G_UART_TX_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân truyền UART TX kết nối module 4G | -1 |
| `CONFIG_CUSTOM_PERIPH_4G_UART_RX_PIN` | int | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân nhận UART RX kết nối module 4G | -1 |
| `CONFIG_CUSTOM_BATTERY_MONITOR_NONE` | bool | 4.7. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) |  | !CUSTOM_ENABLE_BATTERY_MONITOR |
| `CONFIG_ESP_MAIN_TASK_STACK_SIZE` | int | ESP-IDF System | Main Task Stack Size | 6584 |
| `CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE` | int | ESP-IDF System | Event Task Stack Size | 10096 |
| `CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240` | bool | ESP-IDF System | CPU Freq 240MHz | y |
| `CONFIG_ESPTOOLPY_FLASHMODE_QIO` | bool | ESP-IDF System | Flash Mode QIO | y |
| `CONFIG_ESPTOOLPY_FLASHFREQ_80M` | bool | ESP-IDF System | Flash Freq 80MHz | y |
