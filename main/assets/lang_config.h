// Auto-generated language config
// Language: vi-VN with en-US fallback
#pragma once

#include <string_view>

#ifndef vi_vn
    #define vi_vn  // 預設語言
#endif

namespace Lang {
    // 语言元数据
    constexpr const char* CODE = "vi-VN";

    // 字符串资源 (en-US as fallback for missing keys)
    namespace Strings {
        constexpr const char* ACCESS_VIA_BROWSER = " URL cấu hình: ";
        constexpr const char* ACTIVATION = "Kích hoạt";
        constexpr const char* BATTERY_CHARGING = "Đang sạc";
        constexpr const char* BATTERY_FULL = "Pin đầy";
        constexpr const char* BATTERY_LOW = "Pin yếu";
        constexpr const char* BATTERY_NEED_CHARGE = "Pin yếu, vui lòng sạc";
        constexpr const char* CHECKING_NEW_VERSION = "Đang kiểm tra phiên bản mới...";
        constexpr const char* CHECK_NEW_VERSION_FAILED = "Kiểm tra phiên bản mới thất bại, sẽ thử lại sau %d giây: %s";
        constexpr const char* CONNECTED_TO = "Đã kết nối đến ";
        constexpr const char* CONNECTING = "Đang kết nối...";
        constexpr const char* CONNECTION_SUCCESSFUL = "Kết nối thành công";
        constexpr const char* CONNECT_TO = "Kết nối đến ";
        constexpr const char* CONNECT_TO_HOTSPOT = "Điểm phát sóng: ";
        constexpr const char* DETECTING_MODULE = "Đang phát hiện module...";
        constexpr const char* DOWNLOAD_ASSETS_FAILED = "Tải xuống tài nguyên thất bại";
        constexpr const char* ENTERING_WIFI_CONFIG_MODE = "Đang vào chế độ cấu hình Wi-Fi...";
        constexpr const char* ERROR = "Lỗi";
        constexpr const char* FLIGHT_MODE_OFF = "Chế độ máy bay đang tắt";
        constexpr const char* FLIGHT_MODE_ON = "Chế độ máy bay đang bật";
        constexpr const char* FOUND_NEW_ASSETS = "Tìm thấy tài nguyên mới: %s";
        constexpr const char* HELLO_MY_FRIEND = "Xin chào, bạn của tôi!";
        constexpr const char* INFO = "Thông tin";
        constexpr const char* INITIALIZING = "Đang khởi tạo...";
        constexpr const char* LISTENING = "Đang lắng nghe...";
        constexpr const char* LOADING_ASSETS = "Đang tải tài nguyên...";
        constexpr const char* LOADING_PROTOCOL = "Đang đăng nhập...";
        constexpr const char* MAX_VOLUME = "Âm lượng tối đa";
        constexpr const char* MODEM_INIT_ERROR = "Khởi tạo modem thất bại";
        constexpr const char* MUTED = "Tắt tiếng";
        constexpr const char* NEW_VERSION = "Phiên bản mới ";
        constexpr const char* OTA_UPGRADE = "Nâng cấp OTA";
        constexpr const char* PIN_ERROR = "Vui lòng cắm thẻ SIM";
        constexpr const char* PLEASE_WAIT = "Vui lòng đợi...";
        constexpr const char* REGISTERING_NETWORK = "Đang chờ mạng...";
        constexpr const char* REG_ERROR = "Không thể truy cập mạng, vui lòng kiểm tra trạng thái thẻ SIM";
        constexpr const char* RTC_MODE_OFF = "Tắt AEC";
        constexpr const char* RTC_MODE_ON = "Bật AEC";
        constexpr const char* SCANNING_WIFI = "Đang quét Wi-Fi...";
        constexpr const char* SERVER_ERROR = "Gửi thất bại, vui lòng kiểm tra mạng";
        constexpr const char* SERVER_NOT_CONNECTED = "Không thể kết nối đến dịch vụ, vui lòng thử lại sau";
        constexpr const char* SERVER_NOT_FOUND = "Đang tìm dịch vụ khả dụng";
        constexpr const char* SERVER_TIMEOUT = "Hết thời gian chờ phản hồi";
        constexpr const char* SPEAKING = "Đang nói...";
        constexpr const char* STANDBY = "Chờ";
        constexpr const char* SWITCH_TO_4G_NETWORK = "Đang chuyển sang 4G...";
        constexpr const char* SWITCH_TO_WIFI_NETWORK = "Đang chuyển sang Wi-Fi...";
        constexpr const char* UPGRADE_FAILED = "Nâng cấp thất bại";
        constexpr const char* UPGRADING = "Hệ thống đang nâng cấp...";
        constexpr const char* VERSION = "Phiên bản ";
        constexpr const char* VOLUME = "Âm lượng ";
        constexpr const char* WARNING = "Cảnh báo";
        constexpr const char* WIFI_CONFIG_MODE = "Chế độ cấu hình Wi-Fi";
    }

    // 音效资源 (en-US as fallback for missing audio files)
    namespace Sounds {

        extern const char ogg_0_start[] asm("_binary_0_ogg_start");
        extern const char ogg_0_end[] asm("_binary_0_ogg_end");
        static const std::string_view OGG_0 {
        static_cast<const char*>(ogg_0_start),
        static_cast<size_t>(ogg_0_end - ogg_0_start)
        };

        extern const char ogg_1_start[] asm("_binary_1_ogg_start");
        extern const char ogg_1_end[] asm("_binary_1_ogg_end");
        static const std::string_view OGG_1 {
        static_cast<const char*>(ogg_1_start),
        static_cast<size_t>(ogg_1_end - ogg_1_start)
        };

        extern const char ogg_2_start[] asm("_binary_2_ogg_start");
        extern const char ogg_2_end[] asm("_binary_2_ogg_end");
        static const std::string_view OGG_2 {
        static_cast<const char*>(ogg_2_start),
        static_cast<size_t>(ogg_2_end - ogg_2_start)
        };

        extern const char ogg_3_start[] asm("_binary_3_ogg_start");
        extern const char ogg_3_end[] asm("_binary_3_ogg_end");
        static const std::string_view OGG_3 {
        static_cast<const char*>(ogg_3_start),
        static_cast<size_t>(ogg_3_end - ogg_3_start)
        };

        extern const char ogg_4_start[] asm("_binary_4_ogg_start");
        extern const char ogg_4_end[] asm("_binary_4_ogg_end");
        static const std::string_view OGG_4 {
        static_cast<const char*>(ogg_4_start),
        static_cast<size_t>(ogg_4_end - ogg_4_start)
        };

        extern const char ogg_5_start[] asm("_binary_5_ogg_start");
        extern const char ogg_5_end[] asm("_binary_5_ogg_end");
        static const std::string_view OGG_5 {
        static_cast<const char*>(ogg_5_start),
        static_cast<size_t>(ogg_5_end - ogg_5_start)
        };

        extern const char ogg_6_start[] asm("_binary_6_ogg_start");
        extern const char ogg_6_end[] asm("_binary_6_ogg_end");
        static const std::string_view OGG_6 {
        static_cast<const char*>(ogg_6_start),
        static_cast<size_t>(ogg_6_end - ogg_6_start)
        };

        extern const char ogg_7_start[] asm("_binary_7_ogg_start");
        extern const char ogg_7_end[] asm("_binary_7_ogg_end");
        static const std::string_view OGG_7 {
        static_cast<const char*>(ogg_7_start),
        static_cast<size_t>(ogg_7_end - ogg_7_start)
        };

        extern const char ogg_8_start[] asm("_binary_8_ogg_start");
        extern const char ogg_8_end[] asm("_binary_8_ogg_end");
        static const std::string_view OGG_8 {
        static_cast<const char*>(ogg_8_start),
        static_cast<size_t>(ogg_8_end - ogg_8_start)
        };

        extern const char ogg_9_start[] asm("_binary_9_ogg_start");
        extern const char ogg_9_end[] asm("_binary_9_ogg_end");
        static const std::string_view OGG_9 {
        static_cast<const char*>(ogg_9_start),
        static_cast<size_t>(ogg_9_end - ogg_9_start)
        };

        extern const char ogg_activation_start[] asm("_binary_activation_ogg_start");
        extern const char ogg_activation_end[] asm("_binary_activation_ogg_end");
        static const std::string_view OGG_ACTIVATION {
        static_cast<const char*>(ogg_activation_start),
        static_cast<size_t>(ogg_activation_end - ogg_activation_start)
        };

        extern const char ogg_err_pin_start[] asm("_binary_err_pin_ogg_start");
        extern const char ogg_err_pin_end[] asm("_binary_err_pin_ogg_end");
        static const std::string_view OGG_ERR_PIN {
        static_cast<const char*>(ogg_err_pin_start),
        static_cast<size_t>(ogg_err_pin_end - ogg_err_pin_start)
        };

        extern const char ogg_err_reg_start[] asm("_binary_err_reg_ogg_start");
        extern const char ogg_err_reg_end[] asm("_binary_err_reg_ogg_end");
        static const std::string_view OGG_ERR_REG {
        static_cast<const char*>(ogg_err_reg_start),
        static_cast<size_t>(ogg_err_reg_end - ogg_err_reg_start)
        };

        extern const char ogg_exclamation_start[] asm("_binary_exclamation_ogg_start");
        extern const char ogg_exclamation_end[] asm("_binary_exclamation_ogg_end");
        static const std::string_view OGG_EXCLAMATION {
        static_cast<const char*>(ogg_exclamation_start),
        static_cast<size_t>(ogg_exclamation_end - ogg_exclamation_start)
        };

        extern const char ogg_low_battery_start[] asm("_binary_low_battery_ogg_start");
        extern const char ogg_low_battery_end[] asm("_binary_low_battery_ogg_end");
        static const std::string_view OGG_LOW_BATTERY {
        static_cast<const char*>(ogg_low_battery_start),
        static_cast<size_t>(ogg_low_battery_end - ogg_low_battery_start)
        };

        extern const char ogg_popup_start[] asm("_binary_popup_ogg_start");
        extern const char ogg_popup_end[] asm("_binary_popup_ogg_end");
        static const std::string_view OGG_POPUP {
        static_cast<const char*>(ogg_popup_start),
        static_cast<size_t>(ogg_popup_end - ogg_popup_start)
        };

        extern const char ogg_success_start[] asm("_binary_success_ogg_start");
        extern const char ogg_success_end[] asm("_binary_success_ogg_end");
        static const std::string_view OGG_SUCCESS {
        static_cast<const char*>(ogg_success_start),
        static_cast<size_t>(ogg_success_end - ogg_success_start)
        };

        extern const char ogg_upgrade_start[] asm("_binary_upgrade_ogg_start");
        extern const char ogg_upgrade_end[] asm("_binary_upgrade_ogg_end");
        static const std::string_view OGG_UPGRADE {
        static_cast<const char*>(ogg_upgrade_start),
        static_cast<size_t>(ogg_upgrade_end - ogg_upgrade_start)
        };

        extern const char ogg_vibration_start[] asm("_binary_vibration_ogg_start");
        extern const char ogg_vibration_end[] asm("_binary_vibration_ogg_end");
        static const std::string_view OGG_VIBRATION {
        static_cast<const char*>(ogg_vibration_start),
        static_cast<size_t>(ogg_vibration_end - ogg_vibration_start)
        };

        extern const char ogg_welcome_start[] asm("_binary_welcome_ogg_start");
        extern const char ogg_welcome_end[] asm("_binary_welcome_ogg_end");
        static const std::string_view OGG_WELCOME {
        static_cast<const char*>(ogg_welcome_start),
        static_cast<size_t>(ogg_welcome_end - ogg_welcome_start)
        };

        extern const char ogg_wificonfig_start[] asm("_binary_wificonfig_ogg_start");
        extern const char ogg_wificonfig_end[] asm("_binary_wificonfig_ogg_end");
        static const std::string_view OGG_WIFICONFIG {
        static_cast<const char*>(ogg_wificonfig_start),
        static_cast<size_t>(ogg_wificonfig_end - ogg_wificonfig_start)
        };
    }
}
