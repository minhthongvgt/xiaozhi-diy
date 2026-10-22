# Dòng trạng thái và hiệu ứng nghe/nói

Menu: «Màn hình» → «Thanh trạng thái và hiệu ứng nghe/nói». Trên Web Configurator nằm ở tab Màn hình. Áp dụng cho mọi kiểu giao diện LVGL (Mặc định, WeChat, các kiểu Adaptive), không áp dụng cho kiểu Emote.

## Nội dung khi chờ (`CUSTOM_UI_IDLE_STATUS`)
| Lựa chọn | Hiện gì | Khi thiếu dữ liệu |
|---|---|---|
| Đồng hồ | `HH:MM` | chữ «Chờ» khi chưa có giờ mạng |
| Thứ và ngày tháng | `Thứ hai, 20/10/2026` (tiếng Việt), ngôn ngữ khác dùng `%a %d/%m/%Y` | chữ «Chờ» |
| Thông số cảm biến | nhiệt độ, độ ẩm, áp suất, ánh sáng, CO2, khí gas, báo lửa của cảm biến đã bật | ngày tháng, rồi «Chờ» |
| Luân phiên (mặc định) | lần lượt các mục bật bằng `CUSTOM_UI_IDLE_ROTATE_{CLOCK,DATE,SENSORS,STANDBY}`, mỗi mục `CUSTOM_UI_IDLE_ROTATE_SEC` giây | bỏ mục rỗng |
| Chữ «Chờ» | `Lang::Strings::STANDBY` | |
| Để trống | không hiện gì | |

- Cảm biến được đọc lại mỗi `CUSTOM_UI_IDLE_SENSOR_REFRESH_SEC` giây (mặc định 15) trong task `Application::Run`, cùng task với công cụ MCP nên không đọc chồng nhau. Việc đọc nằm ngoài khoá LVGL.
- Dòng trạng thái chỉ được ghi lại khi nội dung đổi, nên chữ cảnh báo (`Alert`) hiện trong lúc chờ vẫn giữ đến lần đổi nội dung kế tiếp.
- Giao diện Adaptive có đồng hồ riêng nên mặc định không hiện đồng hồ ở dòng trạng thái.

## Hiệu ứng nghe/nói (`CUSTOM_UI_LISTEN_ANIM`, `CUSTOM_UI_SPEAK_ANIM`)
Hiệu ứng thay cho chữ «Đang lắng nghe...» / «Đang nói...», cùng vị trí và kích thước với dòng trạng thái.

| Mã | Lựa chọn | Đang nghe | Đang nói |
|---|---|---|---|
| 0 | Chỉ hiện chữ | chữ | chữ |
| 1 | Sóng âm dạng cột | cột nhấp nhô, cao hơn khi có tiếng nói (VAD) | cột theo âm lượng loa |
| 2 | Sóng âm uốn lượn | hai đường sóng chạy ngược nhau | biên độ theo âm lượng |
| 3 | Vòng sóng lan tỏa | ba vòng elip lan ra, chấm giữa phập phồng | chấm giữa theo âm lượng |
| 4 | Chấm nhảy | năm chấm nảy lần lượt | độ nảy theo âm lượng |
| 5 | Đôi mắt | chớp ngẫu nhiên 2–4 giây, liếc qua lại, mở to khi nghe thấy tiếng | chớp mắt, miệng mở theo âm lượng |

Mặc định: nghe = đôi mắt, nói = sóng cột. Màu (`CUSTOM_UI_ANIM_COLOR_SOURCE`): theo màu chữ giao diện, cầu vồng đổi màu liên tục, hoặc mã màu tự chọn `CUSTOM_UI_ANIM_COLOR`.

## Cách hoạt động
- `display/lvgl_display/status_activity.{h,cc}`: lớp `StatusActivityAnimator` tạo một khung nổi (`LV_OBJ_FLAG_FLOATING`) cùng cha với nhãn trạng thái, vẽ bằng `lv_timer` 40 ms trong task LVGL.
- `LvglDisplay::SetStatus()` so chuỗi với `Lang::Strings::LISTENING` / `SPEAKING`: trùng thì bật hiệu ứng và cho nhãn trong suốt (giữ chỗ trong bố cục flex của OLED); chuỗi khác thì tắt hiệu ứng.
- Thông báo (`ShowNotification`) ẩn hiệu ứng, hết thời gian thông báo thì hiện lại.
- Mức âm thanh: khi nói lấy `AudioService::GetOutputLevel()` (đỉnh 24000 = đầy); khi nghe chỉ có cờ VAD `IsVoiceDetected()`.
- Giao diện Adaptive dựng lại (đổi font sau khi nạp assets) xoá khung theo cây đối tượng; sự kiện `LV_EVENT_DELETE` xoá timer và con trỏ, lần `SetStatus` kế tiếp tạo lại.
