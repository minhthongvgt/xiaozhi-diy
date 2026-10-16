# Âm thanh thông báo (chọn hoặc thay âm)

Menuconfig: `Xiaozhi Assistant → Âm thanh thông báo (Notification Sounds)`.
Web Configurator: tab **Âm thanh**, nhóm 🔔 "Âm thanh thông báo".

Âm được chọn lúc build và nhúng vào firmware. Đổi âm thì build và nạp lại.

## Các sự kiện

| Sự kiện | Ký hiệu choice | Mặc định | Khi nào phát |
|---|---|---|---|
| Khởi động xong | `SOUND_STARTUP` | success | Thiết bị đã kết nối mạng và kích hoạt, sẵn sàng trò chuyện |
| Bắt đầu lắng nghe / có thông báo | `SOUND_LISTENING` | popup | Bắt đầu nghe sau từ đánh thức; trước khi phát thông báo đẩy |
| Báo lỗi | `SOUND_ERROR` | exclamation | Lỗi kết nối máy chủ, lỗi kích hoạt, lỗi tải tài nguyên, OTA thất bại |
| Cảnh báo từ máy chủ | `SOUND_ALERT` | vibration | Máy chủ gửi tin `alert` |
| Pin yếu | `SOUND_LOW_BATTERY` | low_battery | Pin xuống mức thấp |
| Cấu hình Wi-Fi | `SOUND_WIFI_CONFIG` | giọng nói theo ngôn ngữ | Vào chế độ cấu hình Wi-Fi |
| Mã kích hoạt | `SOUND_ACTIVATION` | giọng nói theo ngôn ngữ | Trước khi đọc mã kích hoạt (các chữ số vẫn đọc bằng giọng theo ngôn ngữ) |
| Nâng cấp | `SOUND_UPGRADE` | giọng nói theo ngôn ngữ | Bắt đầu tải tài nguyên hoặc firmware mới |

Mỗi sự kiện chọn một trong:
- một trong 5 âm có sẵn: popup, success, exclamation, vibration, low_battery (`main/assets/common/`);
- giọng nói theo ngôn ngữ (chỉ 3 sự kiện cuối);
- **Tắt tiếng**: sự kiện vẫn xảy ra, màn hình vẫn hiện, chỉ không phát âm;
- **Tệp .ogg tùy chỉnh**: nhập đường dẫn vào ô `SOUND_<SỰ_KIỆN>_CUSTOM_FILE`.

## Tệp tùy chỉnh

- Định dạng: **Ogg Opus**. Khuyên dùng mono, 16 kHz, 16 kbps, khung 60 ms, giống các âm có sẵn.
- Dung lượng tối đa 1 MB. Âm thông báo nên ngắn (dưới 3 giây) để không làm chậm phản hồi.
- Đường dẫn tuyệt đối, hoặc tương đối tính từ thư mục gốc dự án. Dùng dấu `/`, ví dụ `sounds/khoi_dong.ogg`.
- Đổi nội dung tệp mà giữ nguyên đường dẫn: lần build sau tự nhúng lại.

Chuyển từ mp3/wav sang Ogg Opus:

```bash
ffmpeg -i khoi_dong.mp3 -c:a libopus -b:a 16k -ac 1 -ar 16000 -frame_duration 60 sounds/khoi_dong.ogg
```

Hoặc dùng công cụ có giao diện `scripts/ogg_converter/xiaozhi_ogg_converter.py` (cần ffmpeg, có chuẩn hoá độ to -16 LUFS).

## Lỗi khi build

Build dừng ở bước CMake với thông báo `SOUND_<SỰ_KIỆN>: ...` khi:
- chọn tệp tùy chỉnh nhưng để trống đường dẫn;
- không tìm thấy tệp;
- tệp lớn hơn 1 MB;
- tệp không phải Ogg Opus (ví dụ mp3 đổi đuôi, hoặc Ogg Vorbis).

## Cách hoạt động

- `main/sound_theme.cmake` đọc lựa chọn Kconfig. Tệp tùy chỉnh được kiểm tra rồi chép vào `build/esp-idf/main/sound_theme/sound_theme_<sự_kiện>.ogg` và nhúng bằng `EMBED_FILES`.
- CMake sinh `sound_theme_map.h` gán mỗi sự kiện tới âm có sẵn (`Lang::Sounds::OGG_*`), tệp nhúng, hoặc chuỗi rỗng.
- Mã gọi `SoundTheme::Startup()`, `Listening()`, `Error()`, `Alert()`, `LowBattery()`, `WifiConfig()`, `Activation()`, `Upgrade()` (`main/audio/sound_theme.h`) thay vì `Lang::Sounds::OGG_*`.
- `AudioService::PlaySound()` bỏ qua âm rỗng, nên "Tắt tiếng" không bật loa.
- Âm có sẵn không bị nhúng hai lần; chỉ tệp tùy chỉnh làm firmware lớn thêm.
- Khi thêm chỗ phát âm mới cho các sự kiện trên, dùng hàm `SoundTheme::*`. Muốn thêm sự kiện mới: thêm choice vào Kconfig (cùng mẫu), thêm cặp `SỰ_KIỆN:ÂM_MẶC_ĐỊNH` vào `SOUND_THEME_SLOTS`, thêm hàm vào `sound_theme.{h,cc}`, rồi chạy `tools/web-configurator/sync_menuconfig.bat`.
