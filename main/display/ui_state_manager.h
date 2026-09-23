#ifndef _UI_STATE_MANAGER_H_
#define _UI_STATE_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ui.h"

/**
 * Điều khiển trạng thái thiết bị Xiaozhi lên giao diện SquareLine Studio:
 *  1: Starting (Khởi động)
 *  2: WifiConfiguring (Cấu hình WiFi)
 *  3: Idle (Sẵn sàng)
 *  4: Connecting (Đang kết nối)
 *  5: Listening (Đang nghe)
 *  6: Speaking (Đang nói)
 *  7: Notifying (Thông báo)
 *  8: Upgrading (Nâng cấp OTA)
 *  9: Activating (Kích hoạt)
 *  10: AudioTesting (Kiểm tra audio)
 *  11: FatalError (Báo lỗi)
 */
void ui_set_device_state(int state, const char* custom_message);

/**
 * Cập nhật nội dung chat của AI hoặc người dùng lên giao diện
 */
void ui_set_chat_message(const char* role, const char* content);

/**
 * Cập nhật biểu cảm khuôn mặt AI
 */
void ui_set_emotion(const char* emotion);

/**
 * Cập nhật thông tin thanh trạng thái trên cùng (WiFi, Pin, Giờ)
 */
void ui_set_status_bar(const char* wifi_str, const char* battery_str, const char* time_str);

/**
 * Cập nhật thông tin thời tiết (Thành phố, Nhiệt độ, Tình trạng thời tiết)
 */
void ui_set_weather(const char* city, const char* temp, const char* desc);

/**
 * Cập nhật thanh tiến trình nâng cấp OTA (0 - 100%)
 */
void ui_set_ota_progress(int percent, const char* speed_str);

/**
 * Cập nhật thanh mức sóng âm thanh khi người dùng nói
 */
void ui_set_audio_level(int level_0_to_100);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // _UI_STATE_MANAGER_H_
