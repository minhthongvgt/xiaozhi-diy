#include "ui_state_manager.h"
#include <stdio.h>
#include <string.h>

void ui_set_device_state(int state, const char* custom_message)
{
    if (!ui_Screen1) return;

    if (ui_BarProgress && state != 8) {
        lv_obj_add_flag(ui_BarProgress, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui_LabelProgress && state != 8) {
        lv_obj_add_flag(ui_LabelProgress, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui_BarAudio && state != 5) {
        lv_obj_add_flag(ui_BarAudio, LV_OBJ_FLAG_HIDDEN);
        if (ui_LabelPrompt) lv_obj_clear_flag(ui_LabelPrompt, LV_OBJ_FLAG_HIDDEN);
    }

    switch (state) {
        case 1: // kDeviceStateStarting
            if (ui_LedStatus) lv_obj_set_style_bg_color(ui_LedStatus, lv_color_hex(0x89B4FA), 0); // Blue
            if (ui_LabelState) lv_label_set_text(ui_LabelState, "Khoi dong...");
            if (ui_LabelAvatar) lv_label_set_text(ui_LabelAvatar, "( o _ o )");
            if (ui_LabelPrompt) lv_label_set_text(ui_LabelPrompt, "Dang khoi dong he thong...");
            break;

        case 2: // kDeviceStateWifiConfiguring
            if (ui_LedStatus) lv_obj_set_style_bg_color(ui_LedStatus, lv_color_hex(0xFAB387), 0); // Orange
            if (ui_LabelState) lv_label_set_text(ui_LabelState, "Cau hinh WiFi");
            if (ui_LabelAvatar) lv_label_set_text(ui_LabelAvatar, "( ? _ ? )");
            if (ui_LabelPrompt) lv_label_set_text(ui_LabelPrompt, "Ket noi vao WiFi cua thiet bi");
            break;

        case 3: // kDeviceStateIdle
            if (ui_LedStatus) lv_obj_set_style_bg_color(ui_LedStatus, lv_color_hex(0x89DCEB), 0); // Cyan
            if (ui_LabelState) lv_label_set_text(ui_LabelState, "San sang");
            if (ui_LabelAvatar) lv_label_set_text(ui_LabelAvatar, "( ^ _ ^ )");
            if (ui_LabelPrompt) lv_label_set_text(ui_LabelPrompt, "Noi 'Xiaozhi' hoac bam nut");
            break;

        case 4: // kDeviceStateConnecting
            if (ui_LedStatus) lv_obj_set_style_bg_color(ui_LedStatus, lv_color_hex(0xF9E2AF), 0); // Yellow
            if (ui_LabelState) lv_label_set_text(ui_LabelState, "Dang ket noi...");
            if (ui_LabelAvatar) lv_label_set_text(ui_LabelAvatar, "( - _ . )");
            if (ui_LabelPrompt) lv_label_set_text(ui_LabelPrompt, "Dang ket noi may chu AI...");
            break;

        case 5: // kDeviceStateListening
            if (ui_LedStatus) lv_obj_set_style_bg_color(ui_LedStatus, lv_color_hex(0xA6E3A1), 0); // Green
            if (ui_LabelState) lv_label_set_text(ui_LabelState, "Dang lang nghe...");
            if (ui_LabelAvatar) lv_label_set_text(ui_LabelAvatar, "( O _ O )");
            if (ui_BarAudio) {
                lv_obj_clear_flag(ui_BarAudio, LV_OBJ_FLAG_HIDDEN);
                if (ui_LabelPrompt) lv_obj_add_flag(ui_LabelPrompt, LV_OBJ_FLAG_HIDDEN);
            }
            break;

        case 6: // kDeviceStateSpeaking
            if (ui_LedStatus) lv_obj_set_style_bg_color(ui_LedStatus, lv_color_hex(0xCBA6F7), 0); // Purple
            if (ui_LabelState) lv_label_set_text(ui_LabelState, "Dang tra loi...");
            if (ui_LabelAvatar) lv_label_set_text(ui_LabelAvatar, "( ^ o ^ )");
            if (ui_LabelPrompt) lv_label_set_text(ui_LabelPrompt, "Dang phat am thanh...");
            break;

        case 7: // kDeviceStateNotifying
            if (ui_LedStatus) lv_obj_set_style_bg_color(ui_LedStatus, lv_color_hex(0x94E2D5), 0); // Teal
            if (ui_LabelState) lv_label_set_text(ui_LabelState, "Thong bao");
            if (ui_LabelAvatar) lv_label_set_text(ui_LabelAvatar, "( ! _ ! )");
            break;

        case 8: // kDeviceStateUpgrading
            if (ui_LedStatus) lv_obj_set_style_bg_color(ui_LedStatus, lv_color_hex(0xFAB387), 0); // Orange
            if (ui_LabelState) lv_label_set_text(ui_LabelState, "Cap nhat OTA...");
            if (ui_LabelAvatar) lv_label_set_text(ui_LabelAvatar, "[ O T A ]");
            if (ui_BarProgress) lv_obj_clear_flag(ui_BarProgress, LV_OBJ_FLAG_HIDDEN);
            if (ui_LabelProgress) lv_obj_clear_flag(ui_LabelProgress, LV_OBJ_FLAG_HIDDEN);
            if (ui_LabelPrompt) lv_label_set_text(ui_LabelPrompt, "Vui long khong ngat nguon!");
            break;

        case 9: // kDeviceStateActivating
            if (ui_LedStatus) lv_obj_set_style_bg_color(ui_LedStatus, lv_color_hex(0x89B4FA), 0);
            if (ui_LabelState) lv_label_set_text(ui_LabelState, "Kich hoat thiet bi");
            if (ui_LabelAvatar) lv_label_set_text(ui_LabelAvatar, "( * _ * )");
            break;

        case 10: // kDeviceStateAudioTesting
            if (ui_LedStatus) lv_obj_set_style_bg_color(ui_LedStatus, lv_color_hex(0xCBA6F7), 0);
            if (ui_LabelState) lv_label_set_text(ui_LabelState, "Kiem tra am thanh");
            if (ui_LabelAvatar) lv_label_set_text(ui_LabelAvatar, "( > _ < )");
            break;

        case 11: // kDeviceStateFatalError
        default:
            if (ui_LedStatus) lv_obj_set_style_bg_color(ui_LedStatus, lv_color_hex(0xF38BA8), 0); // Red
            if (ui_LabelState) lv_label_set_text(ui_LabelState, "Loi he thong");
            if (ui_LabelAvatar) lv_label_set_text(ui_LabelAvatar, "( X _ X )");
            if (ui_LabelPrompt) lv_label_set_text(ui_LabelPrompt, "Kiem tra lai ket noi / nguon");
            break;
    }

    if (custom_message && custom_message[0] != '\0' && ui_LabelState) {
        lv_label_set_text(ui_LabelState, custom_message);
    }
}

void ui_set_chat_message(const char* role, const char* content)
{
    if (!ui_Screen1) return;

    if (ui_LabelRole && role) {
        if (strcmp(role, "user") == 0) {
            lv_label_set_text(ui_LabelRole, "Ban");
            lv_obj_set_style_text_color(ui_LabelRole, lv_color_hex(0xA6E3A1), 0);
        } else if (strcmp(role, "system") == 0) {
            lv_label_set_text(ui_LabelRole, "He thong");
            lv_obj_set_style_text_color(ui_LabelRole, lv_color_hex(0xFAB387), 0);
        } else {
            lv_label_set_text(ui_LabelRole, "Xiaozhi");
            lv_obj_set_style_text_color(ui_LabelRole, lv_color_hex(0xF9E2AF), 0);
        }
    }

    if (ui_LabelChat && content) {
        lv_label_set_text(ui_LabelChat, content);
    }
}

void ui_set_emotion(const char* emotion)
{
    if (!ui_LabelAvatar || !emotion) return;

    if (strcmp(emotion, "happy") == 0 || strcmp(emotion, "laughing") == 0) {
        lv_label_set_text(ui_LabelAvatar, "( ^ _ ^ )");
        lv_obj_set_style_text_color(ui_LabelAvatar, lv_color_hex(0xA6E3A1), 0);
    } else if (strcmp(emotion, "speaking") == 0) {
        lv_label_set_text(ui_LabelAvatar, "( ^ o ^ )");
        lv_obj_set_style_text_color(ui_LabelAvatar, lv_color_hex(0x89DCEB), 0);
    } else if (strcmp(emotion, "listening") == 0) {
        lv_label_set_text(ui_LabelAvatar, "( O _ O )");
        lv_obj_set_style_text_color(ui_LabelAvatar, lv_color_hex(0x89B4FA), 0);
    } else if (strcmp(emotion, "thinking") == 0) {
        lv_label_set_text(ui_LabelAvatar, "( - _ . )");
        lv_obj_set_style_text_color(ui_LabelAvatar, lv_color_hex(0xF9E2AF), 0);
    } else if (strcmp(emotion, "sad") == 0 || strcmp(emotion, "crying") == 0) {
        lv_label_set_text(ui_LabelAvatar, "( T _ T )");
        lv_obj_set_style_text_color(ui_LabelAvatar, lv_color_hex(0x89B4FA), 0);
    } else if (strcmp(emotion, "angry") == 0) {
        lv_label_set_text(ui_LabelAvatar, "( > _ < )");
        lv_obj_set_style_text_color(ui_LabelAvatar, lv_color_hex(0xF38BA8), 0);
    } else if (strcmp(emotion, "sleepy") == 0 || strcmp(emotion, "sleeping") == 0) {
        lv_label_set_text(ui_LabelAvatar, "( - _ - ) zZ");
        lv_obj_set_style_text_color(ui_LabelAvatar, lv_color_hex(0x6C7086), 0);
    } else if (strcmp(emotion, "surprise") == 0 || strcmp(emotion, "shocked") == 0) {
        lv_label_set_text(ui_LabelAvatar, "( O _ o !)");
        lv_obj_set_style_text_color(ui_LabelAvatar, lv_color_hex(0xFAB387), 0);
    } else {
        lv_label_set_text(ui_LabelAvatar, "( ^ _ ^ )");
        lv_obj_set_style_text_color(ui_LabelAvatar, lv_color_hex(0x89DCEB), 0);
    }
}

void ui_set_status_bar(const char* wifi_str, const char* battery_str, const char* time_str)
{
    if (wifi_str && ui_LabelWifi) {
        lv_label_set_text(ui_LabelWifi, wifi_str);
    }
    if (battery_str && ui_LabelBattery) {
        lv_label_set_text(ui_LabelBattery, battery_str);
    }
    if (time_str && ui_LabelTime) {
        lv_label_set_text(ui_LabelTime, time_str);
    }
}

void ui_set_weather(const char* city, const char* temp, const char* desc)
{
    if (!ui_Screen1) return;
    if (city && ui_LabelWeatherCity) {
        lv_label_set_text(ui_LabelWeatherCity, city);
    }
    if (temp && ui_LabelWeatherTemp) {
        lv_label_set_text(ui_LabelWeatherTemp, temp);
    }
    if (desc && ui_LabelWeatherDesc) {
        lv_label_set_text(ui_LabelWeatherDesc, desc);
    }
}

void ui_set_ota_progress(int percent, const char* speed_str)
{
    if (ui_BarProgress) {
        lv_obj_clear_flag(ui_BarProgress, LV_OBJ_FLAG_HIDDEN);
        lv_bar_set_value(ui_BarProgress, percent, LV_ANIM_OFF);
    }
    if (ui_LabelProgress) {
        lv_obj_clear_flag(ui_LabelProgress, LV_OBJ_FLAG_HIDDEN);
        char buf[64];
        if (speed_str && speed_str[0] != '\0') {
            snprintf(buf, sizeof(buf), "%d%% (%s)", percent, speed_str);
        } else {
            snprintf(buf, sizeof(buf), "%d%%", percent);
        }
        lv_label_set_text(ui_LabelProgress, buf);
    }
}

void ui_set_audio_level(int level_0_to_100)
{
    if (ui_BarAudio) {
        lv_bar_set_value(ui_BarAudio, level_0_to_100, LV_ANIM_OFF);
    }
}
