# Kế hoạch Đồng bộ Menuconfig sang Web UI

Danh sách này chứa tất cả các cấu hình Kconfig sẽ được ánh xạ sang Web UI. Các mục này sẽ ghi thẳng vào file `sdkconfig` để build.

| Tên Kconfig (sdkconfig) | Kiểu dữ liệu | Danh mục (Menu) | Mô tả hiển thị (Prompt) | Giá trị mặc định |
| --- | --- | --- | --- | --- |
| `CONFIG_OTA_URL` | string | Xiaozhi Assistant | Default OTA URL | LANGUAGE_VI_VN |
| `CONFIG_LANGUAGE_ZH_CN` | bool | Xiaozhi Assistant | Chinese |  |
| `CONFIG_LANGUAGE_ZH_TW` | bool | Xiaozhi Assistant | Chinese Traditional |  |
| `CONFIG_LANGUAGE_EN_US` | bool | Xiaozhi Assistant | English |  |
| `CONFIG_LANGUAGE_JA_JP` | bool | Xiaozhi Assistant | Japanese |  |
| `CONFIG_LANGUAGE_KO_KR` | bool | Xiaozhi Assistant | Korean |  |
| `CONFIG_LANGUAGE_VI_VN` | bool | Xiaozhi Assistant | Vietnamese |  |
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
| `CONFIG_LANGUAGE_SR_RS` | bool | Xiaozhi Assistant | Serbian | BOARD_TYPE_ESP32_S3_N16R8_CUSTOM |
| `CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM` | bool | Xiaozhi Assistant | ESP32-S3 N16R8 Custom DIY Board (Bo mach tuy bien toan dien - Khuyen nghi) | USE_DEFAULT_MESSAGE_STYLE |
| `CONFIG_USE_DEFAULT_MESSAGE_STYLE` | bool | Xiaozhi Assistant | Enable default message style |  |
| `CONFIG_USE_WECHAT_MESSAGE_STYLE` | bool | Xiaozhi Assistant | Enable WeChat Message Style |  |
| `CONFIG_USE_EMOTE_MESSAGE_STYLE` | bool | Xiaozhi Assistant | Emote animation style |  |
| `CONFIG_USE_MULTILINE_CHAT_MESSAGE` | bool | Xiaozhi Assistant | Use multiline chat message display (default mode only) | n |
| `CONFIG_WAKE_WORD_DISABLED` | bool | Xiaozhi Assistant | Disabled |  |
| `CONFIG_USE_AFE_WAKE_WORD` | bool | Xiaozhi Assistant | Wakenet model with AFE |  |
| `CONFIG_USE_CUSTOM_WAKE_WORD` | bool | Xiaozhi Assistant | Multinet model (Custom Wake Word) |  |
| `CONFIG_CUSTOM_WAKE_WORD` | string | Xiaozhi Assistant | Custom Wake Word | "xiao tu dou" |
| `CONFIG_CUSTOM_WAKE_WORD_DISPLAY` | string | Xiaozhi Assistant | Custom Wake Word Display | "小土豆" |
| `CONFIG_CUSTOM_WAKE_WORD_THRESHOLD` | int | Xiaozhi Assistant | Custom Wake Word Threshold (%) | 20 |
| `CONFIG_SEND_WAKE_WORD_DATA` | bool | Xiaozhi Assistant | Send Wake Word Data | y |
| `CONFIG_WAKE_WORD_DETECTION_IN_LISTENING` | bool | Xiaozhi Assistant | Enable Wake Word Detection in Listening Mode | n |
| `CONFIG_USE_AUDIO_PROCESSOR` | bool | Xiaozhi Assistant | Enable AFE Audio Processing | y |
| `CONFIG_USE_DEVICE_AEC` | bool | Xiaozhi Assistant | Enable Device-Side AEC | y |
| `CONFIG_USE_SERVER_AEC` | bool | Xiaozhi Assistant | Enable Server-Side AEC (Unstable) | n |
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
| `CONFIG_XIAOZHI_CAMERA_IMAGE_ROTATION_ANGLE_90` | bool | Camera Configuration | 90° |  |
| `CONFIG_XIAOZHI_CAMERA_IMAGE_ROTATION_ANGLE_270` | bool | Camera Configuration | 270° |  |
| `CONFIG_ENABLE_CUSTOM_DISPLAY` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Màn hình hiển thị (Display Panel) | CUSTOM_DISPLAY_ST7789 |
| `CONFIG_CUSTOM_DISPLAY_ST7789` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Sitronix ST7789 (240x320, 240x240, 170x320 SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_ST7796` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Sitronix ST7796 (320x480 SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_ST7735` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Sitronix ST7735 / ST7735S (128x128, 128x160 SPI mini) |  |
| `CONFIG_CUSTOM_DISPLAY_ST7701` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Sitronix ST7701 (RGB Interface) |  |
| `CONFIG_CUSTOM_DISPLAY_ILI9341` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Ilitek ILI9341 (240x320 SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_ILI9486` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Ilitek ILI9486 / ILI9488 (320x480 SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_GC9A01` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | GalaxyCore GC9A01 (240x240 Màn hình tròn SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_GC9107` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | GalaxyCore GC9107 / GC9D01N (Màn hình tròn 0.99-1.28 inch SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_NV3023` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | NewVision NV3023 / NV3030B (TFT SPI giá rẻ) |  |
| `CONFIG_CUSTOM_DISPLAY_JD9853` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Jadard JD9853 / JD9365 (240x240 SPI LCD Panel) |  |
| `CONFIG_CUSTOM_DISPLAY_OLED_SSD1306` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Solomon OLED SSD1306 (128x64 / 128x32 I2C) |  |
| `CONFIG_CUSTOM_DISPLAY_OLED_SH1106` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Sino Wealth OLED SH1106 (128x64 I2C) |  |
| `CONFIG_CUSTOM_DISPLAY_QSPI_AMOLED` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | AMOLED QSPI (SH8601 / CO5300 / SPD2010) |  |
| `CONFIG_CUSTOM_DISPLAY_EPAPER_SSD1681` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Solomon SSD1681 (Mực điện tử E-Paper SPI) |  |
| `CONFIG_CUSTOM_DISPLAY_UART` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Màn hình rời UART HMI (Nextion / TJC / DWIN / Serial Display) |  |
| `CONFIG_CUSTOM_DISPLAY_USER_CUSTOM` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | 🔌 Driver Màn hình tùy chỉnh do người dùng tự viết (Custom User Display Driver Hook) |  |
| `CONFIG_CUSTOM_DISPLAY_NONE` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Không sử dụng màn hình (Audio Only / Headless) |  |
| `CONFIG_ENABLE_CUSTOM_SECONDARY_UART_DISPLAY` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Màn hình rời UART mở rộng / Chạy song song 2 màn hình (Secondary UART Display) | n |
| `CONFIG_CUSTOM_DISPLAY_UART_PORT` | int | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | Cổng UART phần cứng (1: UART1, 2: UART2) | 1 |
| `CONFIG_CUSTOM_DISPLAY_UART_TX_PIN` | int | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | Chân TX kết nối chân RX của Màn hình | -1 |
| `CONFIG_CUSTOM_DISPLAY_UART_RX_PIN` | int | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | Chân RX kết nối chân TX của Màn hình | -1 |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_9600` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 9600 bps (Nextion mặc định khi xuất xưởng) |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_19200` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 19200 bps |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_38400` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 38400 bps |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_57600` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 57600 bps |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_115200` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 115200 bps (Khuyên dùng) |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_230400` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 230400 bps |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_460800` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 460800 bps |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE_921600` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | 921600 bps (Tốc độ cao) |  |
| `CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE` | unknown | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) |  | 921600 if CUSTOM_DISPLAY_UART_BAUDRATE_921600 |
| `CONFIG_CUSTOM_DISPLAY_UART_PROTO_NEXTION` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | Nextion / TJC HMI (Lệnh kết thúc 0xFF 0xFF 0xFF) |  |
| `CONFIG_CUSTOM_DISPLAY_UART_PROTO_JSON` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | JSON Streaming (Chuỗi JSON kèm ký tự xuống dòng \n) |  |
| `CONFIG_CUSTOM_DISPLAY_UART_PROTO_RAW_TEXT` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | Raw Text / ASCII (STATUS:..., EMOTION:..., CHAT:...) |  |
| `CONFIG_CUSTOM_DISPLAY_UART_PROTO_DWIN` | bool | Cấu hình Màn hình rời UART (Nextion / TJC / DWIN) | DWIN DGUS (Khung truyền nhị phân 0x5A 0xA5) |  |
| `CONFIG_CUSTOM_DISPLAY_WIDTH` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Độ phân giải ngang (Width Pixels) | 240 |
| `CONFIG_CUSTOM_DISPLAY_HEIGHT` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Độ phân giải dọc (Height Pixels) | 320 |
| `CONFIG_CUSTOM_DISPLAY_PIN_MOSI` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân dữ liệu SPI MOSI (SDA) | -1 |
| `CONFIG_CUSTOM_DISPLAY_PIN_CLK` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân xung nhịp SPI CLK (SCL) | -1 |
| `CONFIG_CUSTOM_DISPLAY_PIN_CS` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân chọn chip SPI CS (-1 nếu không dùng) | -1 |
| `CONFIG_CUSTOM_DISPLAY_PIN_DC` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân lệnh/dữ liệu SPI DC | -1 |
| `CONFIG_CUSTOM_DISPLAY_USE_RST` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Sử dụng chân Reset màn hình (RST) | y |
| `CONFIG_CUSTOM_DISPLAY_PIN_RST` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân Reset màn hình RST GPIO | -1 |
| `CONFIG_CUSTOM_DISPLAY_USE_BLK` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Điều khiển đèn nền màn hình (Backlight BLK) | y |
| `CONFIG_CUSTOM_DISPLAY_PIN_BLK` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân đèn nền BLK GPIO | -1 |
| `CONFIG_CUSTOM_DISPLAY_PIN_I2C_SDA` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân OLED I2C SDA | -1 |
| `CONFIG_CUSTOM_DISPLAY_PIN_I2C_SCL` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân OLED I2C SCL | -1 |
| `CONFIG_CUSTOM_DISPLAY_AMOLED_SH8601` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Sino Wealth SH8601 |  |
| `CONFIG_CUSTOM_DISPLAY_AMOLED_CO5300` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chipone CO5300 |  |
| `CONFIG_CUSTOM_DISPLAY_AMOLED_SPD2010` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Solomon SPD2010 |  |
| `CONFIG_CUSTOM_DISPLAY_QSPI_CS` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân AMOLED QSPI CS | -1 |
| `CONFIG_CUSTOM_DISPLAY_QSPI_CLK` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân AMOLED QSPI CLK | -1 |
| `CONFIG_CUSTOM_DISPLAY_QSPI_D0` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân AMOLED QSPI D0 | -1 |
| `CONFIG_CUSTOM_DISPLAY_QSPI_D1` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân AMOLED QSPI D1 | -1 |
| `CONFIG_CUSTOM_DISPLAY_QSPI_D2` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân AMOLED QSPI D2 | -1 |
| `CONFIG_CUSTOM_DISPLAY_QSPI_D3` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân AMOLED QSPI D3 | -1 |
| `CONFIG_CUSTOM_DISPLAY_QSPI_RST` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân AMOLED QSPI Reset | -1 |
| `CONFIG_CUSTOM_DISPLAY_EPAPER_PIN_BUSY` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân E-Paper BUSY | -1 |
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
| `CONFIG_CUSTOM_DISPLAY_OFFSET_X` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Dịch tọa độ X (Offset X Pixels) | 0 |
| `CONFIG_CUSTOM_DISPLAY_OFFSET_Y` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Dịch tọa độ Y (Offset Y Pixels) | 0 |
| `CONFIG_CUSTOM_DISPLAY_MIRROR_X` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Đảo chiều gương trục X (Mirror X) | n |
| `CONFIG_CUSTOM_DISPLAY_MIRROR_Y` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Đảo chiều gương trục Y (Mirror Y) | n |
| `CONFIG_CUSTOM_DISPLAY_SWAP_XY` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Hoán đổi tọa độ X-Y (X/Y Axes Swapped) | n |
| `CONFIG_CUSTOM_DISPLAY_INVERT_COLOR` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Đảo ngược màu sắc hiển thị (Invert Color) | FLASH_DEFAULT_ASSETS |
| `CONFIG_FLASH_DEFAULT_ASSETS` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Flash tài nguyên mặc định (Default Built-in Assets) |  |
| `CONFIG_FLASH_CUSTOM_ASSETS` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Flash tài nguyên tùy chỉnh từ URL hoặc tệp cục bộ (Custom Assets) |  |
| `CONFIG_FLASH_EXPRESSION_ASSETS` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Flash tài nguyên biểu cảm AI Voice (Emote Expression Assets) |  |
| `CONFIG_FLASH_NONE_ASSETS` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Không nạp phân vùng tài nguyên (No Assets Flashing) |  |
| `CONFIG_CUSTOM_ASSETS_FILE` | string | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Đường dẫn tệp hoặc URL tài nguyên tùy chỉnh (Custom Assets File / URL) | "https://github.com/78/xiaozhi-esp32/releases/download/v0.9/assets.bin" |
| `CONFIG_ENABLE_CUSTOM_TOUCH` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Màn hình cảm ứng (Touch Screen Panel) | CUSTOM_TOUCH_CST816S |
| `CONFIG_CUSTOM_TOUCH_CST816S` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Hynitron CST816S / CST9217 (I2C) |  |
| `CONFIG_CUSTOM_TOUCH_GT911` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Goodix GT911 / GT1151 (I2C) |  |
| `CONFIG_CUSTOM_TOUCH_FT5X06` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | FocalTech FT5x06 / FT6336 (I2C) |  |
| `CONFIG_CUSTOM_TOUCH_USER_CUSTOM` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | 🔌 Driver Cảm ứng tùy chỉnh do người dùng tự viết (Custom User Touch Driver Hook) |  |
| `CONFIG_CUSTOM_TOUCH_NONE` | bool | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Không sử dụng cảm ứng |  |
| `CONFIG_CUSTOM_TOUCH_PIN_SDA` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân cảm ứng I2C SDA | -1 |
| `CONFIG_CUSTOM_TOUCH_PIN_SCL` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân cảm ứng I2C SCL | -1 |
| `CONFIG_CUSTOM_TOUCH_PIN_INT` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân ngắt cảm ứng INT GPIO | -1 |
| `CONFIG_CUSTOM_TOUCH_PIN_RST` | int | 1. Màn hình, Giao diện & Flash Assets (Display, UI Style & Assets) | Chân reset cảm ứng RST GPIO (-1 nếu không nối) | -1 |
| `CONFIG_ENABLE_CUSTOM_SPEAKER` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Loa phát thanh & Mạch DAC (Speaker & Audio Out) | CUSTOM_AUDIO_DAC_MAX98357A |
| `CONFIG_CUSTOM_AUDIO_DAC_MAX98357A` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | MAX98357A (Mạch công suất I2S Class-D Mono 3W) |  |
| `CONFIG_CUSTOM_AUDIO_DAC_MAX98360A` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | MAX98360A / MAX98360B (I2S/PDM Class-D Mono 3W thế hệ mới) |  |
| `CONFIG_CUSTOM_AUDIO_DAC_PCM5102A` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | PCM5102A (Mạch giải mã Hi-Fi I2S Stereo DAC 32-bit/384kHz) |  |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8311` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Everest Semi ES8311 Codec (I2C) |  |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8388` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Everest Semi ES8388 Stereo Codec (I2C) |  |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8374` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Everest Semi ES8374 Codec |  |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8389` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Everest Semi ES8389 Codec |  |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_BOX` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Box Audio Codec |  |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_USER_CUSTOM` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | 🔌 Driver Audio Codec / DAC phát loa tùy chỉnh do người dùng tự viết (Custom User Audio Speaker Driver Hook) |  |
| `CONFIG_CUSTOM_AUDIO_SPK_GPIO_DOUT` | int | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Chân dữ liệu Loa I2S DOUT (DIN trên module) | -1 |
| `CONFIG_CUSTOM_AUDIO_SPK_GPIO_BCLK` | int | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Chân xung nhịp Loa I2S BCLK (BCK) | -1 |
| `CONFIG_CUSTOM_AUDIO_SPK_GPIO_LRCK` | int | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Chân xung nhịp Loa I2S LRCK / WS (LCK) | -1 |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_I2C_SDA` | int | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Chân Codec DAC I2C SDA | -1 |
| `CONFIG_CUSTOM_AUDIO_SPK_CODEC_I2C_SCL` | int | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Chân Codec DAC I2C SCL | -1 |
| `CONFIG_ENABLE_CUSTOM_MIC` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Microphone thu âm (Microphone & Audio In) | CUSTOM_AUDIO_MIC_INMP441 |
| `CONFIG_CUSTOM_AUDIO_MIC_INMP441` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | INMP441 / MSM261S (Microphone số I2S MEMS Omnidirectional) |  |
| `CONFIG_CUSTOM_AUDIO_MIC_PDM` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | PDM Microphone (Micro số giao thức PDM) |  |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8311` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Everest Semi ES8311 ADC (I2C) |  |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8388` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Everest Semi ES8388 ADC (I2C) |  |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8374` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Everest Semi ES8374 ADC |  |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8389` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Everest Semi ES8389 ADC |  |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_BOX` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Box Audio Codec ADC |  |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_USER_CUSTOM` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | 🔌 Driver Micro thu âm tùy chỉnh do người dùng tự viết (Custom User Audio Mic Driver Hook) |  |
| `CONFIG_CUSTOM_AUDIO_MIC_GPIO_DIN` | int | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Chân dữ liệu Micro I2S DIN (SD trên INMP441/MSM261S) | -1 |
| `CONFIG_CUSTOM_AUDIO_MIC_GPIO_SCK` | int | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Chân xung nhịp Micro I2S SCK (SCK/BCLK) | -1 |
| `CONFIG_CUSTOM_AUDIO_MIC_GPIO_WS` | int | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Chân chọn kênh Micro I2S WS (WS/LRCK) | -1 |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_I2C_SDA` | int | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Chân Codec ADC I2C SDA | -1 |
| `CONFIG_CUSTOM_AUDIO_MIC_CODEC_I2C_SCL` | int | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Chân Codec ADC I2C SCL | -1 |
| `CONFIG_CUSTOM_AUDIO_I2S_SIMPLEX` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Simplex (Độc lập: Loa và Micro có chân Clock BCLK/WS riêng) |  |
| `CONFIG_CUSTOM_AUDIO_I2S_DUPLEX` | bool | 2. Âm thanh: Loa & Micro (Audio: Speaker & Microphone) | Duplex (Dùng chung: Loa và Micro chia sẻ chung chân BCLK và WS) |  |
| `CONFIG_ENABLE_CUSTOM_UART` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Cổng UART mở rộng (Serial Communication Port) | CUSTOM_UART_PORT_1 |
| `CONFIG_CUSTOM_UART_PORT_1` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | UART1 (Cổng serial phần cứng 1) |  |
| `CONFIG_CUSTOM_UART_PORT_2` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | UART2 (Cổng serial phần cứng 2) |  |
| `CONFIG_CUSTOM_UART_PIN_TX` | int | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Chân phát UART TX GPIO | -1 |
| `CONFIG_CUSTOM_UART_PIN_RX` | int | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Chân nhận UART RX GPIO | -1 |
| `CONFIG_CUSTOM_UART_BAUDRATE` | int | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Tốc độ baud truyền thông (Baudrate) | 115200 |
| `CONFIG_CUSTOM_UART_USE_FLOW_CONTROL` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Kích hoạt điều khiển luồng phần cứng (RTS/CTS Hardware Flow Control) | n |
| `CONFIG_CUSTOM_UART_PIN_RTS` | int | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Chân RTS GPIO (Request to Send) | -1 |
| `CONFIG_CUSTOM_UART_PIN_CTS` | int | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Chân CTS GPIO (Clear to Send) | -1 |
| `CONFIG_ENABLE_CUSTOM_MCP_SERVER` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Giao thức IoT & Tính năng MCP Server (Model Context Protocol) | n |
| `CONFIG_CUSTOM_MCP_TOOL_LAMP` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Công cụ MCP: Điều khiển Đèn thông minh / Relay bật tắt qua AI | y |
| `CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO` | int | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Chân GPIO điều khiển Đèn / Relay thông minh | -1 |
| `CONFIG_CUSTOM_MCP_TOOL_SENSOR` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Công cụ MCP: Đọc dữ liệu Cảm biến môi trường, khoảng cách, an ninh qua AI | y |
| `CONFIG_CUSTOM_MCP_TOOL_ACTUATOR` | bool | 3. Giao thức truyền thông & Mạng (Communication, 4G, Ethernet & MCP) | Công cụ MCP: Điều khiển Động cơ Servo, Động cơ DC, Còi Buzzer qua AI | y |
| `CONFIG_CUSTOM_ENABLE_BUTTON_BOOT` | bool | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Nút bấm BOOT Onboard (BOOT Button) | y |
| `CONFIG_CUSTOM_BUTTON_BOOT_GPIO` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân GPIO nút Boot | 0 |
| `CONFIG_CUSTOM_ENABLE_BUTTON_TOUCH` | bool | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Nút bấm chức năng Action / Chạm (PTT / Touch Key) | n |
| `CONFIG_CUSTOM_BUTTON_TOUCH_GPIO` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân GPIO nút Action | -1 |
| `CONFIG_CUSTOM_ENABLE_BUTTON_VOLUME` | bool | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Cụm phím vật lý Tăng / Giảm âm lượng (Volume Buttons) | n |
| `CONFIG_CUSTOM_BUTTON_VOL_UP_GPIO` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân GPIO nút tăng âm lượng (Vol Up) | -1 |
| `CONFIG_CUSTOM_BUTTON_VOL_DOWN_GPIO` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân GPIO nút giảm âm lượng (Vol Down) | -1 |
| `CONFIG_CUSTOM_ENABLE_TOUCH_SLIDER` | bool | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Thanh trượt cảm ứng điện dung nội bộ ESP32-S3 (Touch Slider) | n |
| `CONFIG_CUSTOM_TOUCH_SLIDER_PAD1_GPIO` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân Touch Pad 1 của thanh trượt (Mặc định GPIO 1) | -1 |
| `CONFIG_CUSTOM_TOUCH_SLIDER_PAD2_GPIO` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân Touch Pad 2 của thanh trượt (Mặc định GPIO 2) | -1 |
| `CONFIG_CUSTOM_TOUCH_SLIDER_PAD3_GPIO` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân Touch Pad 3 của thanh trượt (Mặc định GPIO 3) | -1 |
| `CONFIG_CUSTOM_ENABLE_PERIPH_ROTARY_ENCODER` | bool | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Núm vặn xoay vô cấp cơ học (Rotary Encoder EC11 qua PCNT) | n |
| `CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_A` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân xung pha A (CLK) Encoder | -1 |
| `CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_B` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân xung pha B (DT) Encoder | -1 |
| `CONFIG_CUSTOM_PERIPH_ENCODER_KEY_PIN` | int | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | Chân phím nhấn SW Encoder (-1 nếu không dùng) | -1 |
| `CONFIG_CUSTOM_ENABLE_USER_CUSTOM_SENSORS` | bool | 4.1. Tương Tác & Điều Khiển Đầu Vào (User Inputs) | 🔌 Driver Cảm biến / Mạch ngoại vi tùy chỉnh do người dùng tự viết (Custom User Sensor Driver Hook) | n |
| `CONFIG_ENABLE_CUSTOM_CAMERA` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera & Thị giác AI (Camera DVP / USB UVC) | CUSTOM_CAMERA_OV2640 |
| `CONFIG_CUSTOM_CAMERA_OV2640` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | OmniVision OV2640 (2 Megapixel) |  |
| `CONFIG_CUSTOM_CAMERA_OV3660` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | OmniVision OV3660 (3 Megapixel) |  |
| `CONFIG_CUSTOM_CAMERA_OV5640` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | OmniVision OV5640 (5 Megapixel) |  |
| `CONFIG_CUSTOM_CAMERA_SC030IOT` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | SmartSens SC030IOT |  |
| `CONFIG_CUSTOM_CAMERA_USB_UVC` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | USB UVC Web Camera (ESP32-S3 USB Host Port) |  |
| `CONFIG_CUSTOM_CAMERA_USER_CUSTOM` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | 🔌 Driver Camera tùy chỉnh do người dùng tự viết (Custom User Camera Driver Hook) |  |
| `CONFIG_CUSTOM_CAM_PIN_XCLK` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera XCLK (Master Clock) | -1 |
| `CONFIG_CUSTOM_CAM_PIN_PCLK` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera PCLK (Pixel Clock) | -1 |
| `CONFIG_CUSTOM_CAM_PIN_VSYNC` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera VSYNC (Vertical Sync) | -1 |
| `CONFIG_CUSTOM_CAM_PIN_HREF` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera HREF (Horizontal Reference) | -1 |
| `CONFIG_CUSTOM_CAM_PIN_SIOD` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera SIOD (SCCB I2C Data) | -1 |
| `CONFIG_CUSTOM_CAM_PIN_SIOC` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera SIOC (SCCB I2C Clock) | -1 |
| `CONFIG_CUSTOM_CAM_USE_RESET` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Sử dụng chân Reset Camera (RST) | n |
| `CONFIG_CUSTOM_CAM_PIN_RESET` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Chân Camera Reset GPIO | -1 |
| `CONFIG_CUSTOM_CAM_USE_PWDN` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Sử dụng chân Power-Down Camera (PWDN) | n |
| `CONFIG_CUSTOM_CAM_PIN_PWDN` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Chân Camera PWDN GPIO | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D0` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera Data D0 Pin | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D1` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera Data D1 Pin | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D2` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera Data D2 Pin | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D3` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera Data D3 Pin | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D4` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera Data D4 Pin | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D5` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera Data D5 Pin | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D6` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera Data D6 Pin | -1 |
| `CONFIG_CUSTOM_CAM_PIN_D7` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Camera Data D7 Pin | -1 |
| `CONFIG_CUSTOM_CAM_HMIRROR` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Lật ảnh theo chiều ngang (Horizontal Mirror) | n |
| `CONFIG_CUSTOM_CAM_VFLIP` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Lật ảnh theo chiều dọc (Vertical Flip) | n |
| `CONFIG_ENABLE_CUSTOM_LEDS` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Đèn LED trạng thái (Status LED WS2812 / Single LED) | CUSTOM_LED_WS2812 |
| `CONFIG_CUSTOM_LED_WS2812` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Đèn LED RGB kỹ thuật số WS2812 / SK6812 (Giao thức RMT) |  |
| `CONFIG_CUSTOM_LED_SINGLE_PWM` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Đèn LED đơn điều chỉnh độ sáng (LEDC PWM) |  |
| `CONFIG_CUSTOM_LED_CIRCULAR_STRIP` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Vòng tròn LED xoay trạng thái (Circular LED Strip) |  |
| `CONFIG_CUSTOM_LED_USER_CUSTOM` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | 🔌 Driver Đèn LED tùy chỉnh do người dùng tự viết (Custom User LED Driver Hook) |  |
| `CONFIG_CUSTOM_LED_NONE` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Không sử dụng đèn LED |  |
| `CONFIG_CUSTOM_LED_GPIO` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Chân GPIO kết nối Đèn LED (Mặc định chân LED onboard) | 48 |
| `CONFIG_CUSTOM_LED_COUNT` | int | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Số lượng mắt LED trên dải/vòng (Hỗ trợ tối đa 1024 mắt LED) | 1 |
| `CONFIG_CUSTOM_LED_FAST_RAINBOW_EFFECT` | bool | 4.2. Thị Giác & Ánh Sáng LED (Lighting & Vision) | Hiệu ứng Đổi Màu Cầu Vồng Nhanh (Fast Rainbow Effect khi nói / khởi động) | y |
| `CONFIG_CUSTOM_ENABLE_SERVO_DOG` | bool | 4.3. Âm Thanh Cảnh Báo & Xúc Giác (Audio & Haptics) | Động cơ Servo tương tác qua PWM (Servo Dog Controller) | n |
| `CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO` | int | 4.3. Âm Thanh Cảnh Báo & Xúc Giác (Audio & Haptics) | Chân GPIO cấp xung PWM cho Động cơ Servo | -1 |
| `CONFIG_ENABLE_BUZZER` | bool | 4.3. Âm Thanh Cảnh Báo & Xúc Giác (Audio & Haptics) | Còi báo động Buzzer (GPIO 41) | n |
| `CONFIG_BUZZER_PIN` | int | 4.3. Âm Thanh Cảnh Báo & Xúc Giác (Audio & Haptics) | Chân GPIO Buzzer | -1 |
| `CONFIG_ENABLE_HAPTIC_MOTOR` | bool | 4.3. Âm Thanh Cảnh Báo & Xúc Giác (Audio & Haptics) | Động cơ rung phản hồi xúc giác (GPIO 42) | n |
| `CONFIG_HAPTIC_PIN` | int | 4.3. Âm Thanh Cảnh Báo & Xúc Giác (Audio & Haptics) | Chân GPIO Động cơ rung | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_DHT11_22` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Cảm biến Nhiệt độ & Độ ẩm 1 dây (DHT11 / DHT22) | n |
| `CONFIG_CUSTOM_SENSOR_DHT_GPIO` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân dữ liệu 1-Wire cảm biến DHT GPIO | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_I2C_TEMP_HUMID` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Cảm biến Nhiệt độ & Độ ẩm kỹ thuật số I2C (AHT20 / SHT3x) | n |
| `CONFIG_CUSTOM_ENABLE_SENSOR_AHT20` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Aosong AHT10 / AHT20 / AHT21 (I2C) | y |
| `CONFIG_CUSTOM_SENSOR_I2C_SDA` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SDA cảm biến nhiệt ẩm | -1 |
| `CONFIG_CUSTOM_SENSOR_I2C_SCL` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SCL cảm biến nhiệt ẩm | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_BMP280` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Cảm biến Áp suất khí quyển & Độ cao (Bosch BMP280 / BME280 I2C) | n |
| `CONFIG_CUSTOM_SENSOR_BMP280_I2C_SDA` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SDA cảm biến khí áp BMP280 | -1 |
| `CONFIG_CUSTOM_SENSOR_BMP280_I2C_SCL` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SCL cảm biến khí áp BMP280 | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_BH1750` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Cảm biến cường độ ánh sáng kỹ thuật số BH1750 (I2C) | n |
| `CONFIG_CUSTOM_SENSOR_BH1750_I2C_SDA` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SDA cảm biến ánh sáng BH1750 | -1 |
| `CONFIG_CUSTOM_SENSOR_BH1750_I2C_SCL` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SCL cảm biến ánh sáng BH1750 | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_LDR` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Cảm biến ánh sáng quang trở (LDR qua cầu phân áp ADC) | n |
| `CONFIG_CUSTOM_SENSOR_LDR_ADC_CHANNEL` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Kênh ADC1 đo quang trở LDR (Kênh 1 ứng với GPIO 2) | 1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_GAS_CO2` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Cảm biến nồng độ Khí độc TVOC & CO2 NDIR (SCD40/41, SGP30/40 I2C) | n |
| `CONFIG_CUSTOM_SENSOR_GAS_SCD4X` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Sensirion SCD40 / SCD41 (Cảm biến CO2 quang học Photoacoustic NDIR thực 400-5000 ppm) | y |
| `CONFIG_CUSTOM_SENSOR_GAS_I2C_SDA` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SDA cảm biến khí/CO2 | -1 |
| `CONFIG_CUSTOM_SENSOR_GAS_I2C_SCL` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân I2C SCL cảm biến khí/CO2 | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_GAS_ANALOG_MQ` | bool | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Cảm biến khói & khí gas dễ cháy (MQ-2 / MQ-135 Analog ADC) | n |
| `CONFIG_CUSTOM_SENSOR_MQ_ANALOG_PIN` | int | 4.4. Cảm Biến Môi Trường, Khí & Lưu Lượng (Environment, Gas & Flow) | Chân analog đọc điện áp cảm biến MQ (kênh ADC1) | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_HCSR04` | bool | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Cảm biến khoảng cách siêu âm (HC-SR04 / US-100) | n |
| `CONFIG_CUSTOM_SENSOR_HCSR04_TRIG_GPIO` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân phát xung siêu âm Trig GPIO | -1 |
| `CONFIG_CUSTOM_SENSOR_HCSR04_ECHO_GPIO` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân nhận tín hiệu siêu âm Echo GPIO | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_PIR` | bool | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Cảm biến phát hiện người chuyển động PIR (HC-SR501 / RCWL-0516) | n |
| `CONFIG_CUSTOM_SENSOR_PIR_GPIO` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân tín hiệu cảm biến PIR GPIO | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_VIBRATION_SW420` | bool | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Cảm biến cảnh báo chấn động / Rung tiếp điểm (SW-420 GPIO Interrupt) | n |
| `CONFIG_CUSTOM_SENSOR_VIBRATION_PIN` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân tín hiệu Digital Out cảm biến rung | -1 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_FLAME` | bool | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Cảm biến cảnh báo hỏa hoạn / Ngọn lửa (Flame Sensor IR) | n |
| `CONFIG_CUSTOM_SENSOR_FLAME_PIN` | int | 4.5. Không Gian, Khoảng Cách & An Ninh (Space, Distance & Security) | Chân tín hiệu Digital báo lửa | -1 |
| `CONFIG_CUSTOM_PERIPH_RELAY_ENABLE` | bool | 4.7. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Rơ-le đóng cắt thiết bị độc lập (Relay Switch) | n |
| `CONFIG_CUSTOM_PERIPH_RELAY_GPIO` | int | 4.7. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Chân GPIO kích hoạt Relay đóng cắt | -1 |
| `CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE` | bool | 4.7. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Mạch cầu H Động cơ DC 2 chiều cho xe robot (TB6612FNG / L9110S / DRV8833) | CUSTOM_MOTOR_DRIVER_TB6612 |
| `CONFIG_CUSTOM_MOTOR_DRIVER_TB6612` | bool | 4.7. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Toshiba TB6612FNG (Hiệu suất cao MOSFET H-Bridge) |  |
| `CONFIG_CUSTOM_MOTOR_DRIVER_L9110S` | bool | 4.7. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | L9110S / HG7881 (Mạch 2 kênh DC mini giá rẻ) |  |
| `CONFIG_CUSTOM_MOTOR_DRIVER_DRV8833` | bool | 4.7. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | TI DRV8833 (Cầu H kép bảo vệ quá dòng) |  |
| `CONFIG_CUSTOM_PERIPH_MOTOR_PWMA_PIN` | int | 4.7. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Chân điều tốc PWM Động cơ A (PWMA) | -1 |
| `CONFIG_CUSTOM_PERIPH_MOTOR_DIRA_PIN` | int | 4.7. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Chân hướng quay Động cơ A (DIRA / IN1) | -1 |
| `CONFIG_CUSTOM_PERIPH_MOTOR_PWMB_PIN` | int | 4.7. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Chân điều tốc PWM Động cơ B (PWMB) | -1 |
| `CONFIG_CUSTOM_PERIPH_MOTOR_DIRB_PIN` | int | 4.7. Động Cơ, Robot & Cơ Cấu Chấp Hành (Motors, Robotics & Actuators) | Chân hướng quay Động cơ B (DIRB / IN3) | -1 |
| `CONFIG_CUSTOM_ENABLE_BATTERY_MONITOR` | bool | 4.8. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Mạch đo dung lượng Pin (Battery Level Monitor) | n |
| `CONFIG_CUSTOM_BATTERY_MONITOR_ADC` | bool | 4.8. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Đo qua ADC nội bộ (Bộ chia điện trở Voltage Divider) | y |
| `CONFIG_CUSTOM_BATTERY_ADC_CHANNEL` | int | 4.8. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Kênh ADC1 đo pin (Kênh 0 ứng với chân GPIO 1) | 0 |
| `CONFIG_CUSTOM_BATTERY_DIVIDER_R1` | int | 4.8. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Điện trở nhánh trên R1 cầu phân áp (kOhm) | 100 |
| `CONFIG_CUSTOM_BATTERY_DIVIDER_R2` | int | 4.8. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Điện trở nhánh dưới R2 cầu phân áp (kOhm) | 100 |
| `CONFIG_CUSTOM_ENABLE_SENSOR_INA2XX` | bool | 4.8. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Module đo điện áp, dòng điện & công suất tiêu thụ (TI INA219 / INA226 I2C) | n |
| `CONFIG_CUSTOM_SENSOR_INA219` | bool | 4.8. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Texas Instruments INA219 (Điện áp tối đa 26V, độ phân giải 12-bit) | y |
| `CONFIG_CUSTOM_SENSOR_INA2XX_I2C_SDA` | int | 4.8. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SDA module đo dòng INA | -1 |
| `CONFIG_CUSTOM_SENSOR_INA2XX_I2C_SCL` | int | 4.8. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân I2C SCL module đo dòng INA | -1 |
| `CONFIG_CUSTOM_ENABLE_PERIPH_BATTERY_CHARGING_DETECT` | bool | 4.8. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Giám sát trạng thái sạc pin (TP4056 CHRG / STDBY Pin) | n |
| `CONFIG_CUSTOM_PERIPH_BATTERY_CHRG_PIN` | int | 4.8. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) | Chân đọc tín hiệu sạc (kéo trở Pull-up) | -1 |
| `CONFIG_ENABLE_CUSTOM_SENSORS` | bool | 4.8. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) |  | y |
| `CONFIG_CUSTOM_BATTERY_MONITOR_NONE` | bool | 4.8. Nguồn Điện, Bộ Nhớ & Viễn Thông (Power, Storage & Telecom) |  | !CUSTOM_ENABLE_BATTERY_MONITOR |
| `CONFIG_ESP_MAIN_TASK_STACK_SIZE` | int | ESP-IDF System | Main Task Stack Size | 6584 |
| `CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE` | int | ESP-IDF System | Event Task Stack Size | 10096 |
| `CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240` | bool | ESP-IDF System | CPU Freq 240MHz | y |
| `CONFIG_ESPTOOLPY_FLASHMODE_QIO` | bool | ESP-IDF System | Flash Mode QIO | y |
| `CONFIG_ESPTOOLPY_FLASHFREQ_80M` | bool | ESP-IDF System | Flash Freq 80MHz | y |
