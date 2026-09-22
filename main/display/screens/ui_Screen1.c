// Modern 240x320 Portrait UI for Xiaozhi AI Chatbot
// Includes Central Weather Card for Ho Chi Minh City & Full State Management
// Compatible with LVGL 8.x and 9.x / SquareLine Studio

#include "../ui.h"
#include <stdio.h>

// Global widget instances
lv_obj_t * ui_Screen1 = NULL;

lv_obj_t * ui_TopBar = NULL;
lv_obj_t * ui_LabelWifi = NULL;
lv_obj_t * ui_LabelTime = NULL;
lv_obj_t * ui_LabelBattery = NULL;

lv_obj_t * ui_StatusBar = NULL;
lv_obj_t * ui_PanelState = NULL;
lv_obj_t * ui_LedStatus = NULL;
lv_obj_t * ui_LabelState = NULL;

lv_obj_t * ui_WeatherPanel = NULL;
lv_obj_t * ui_LabelWeatherCity = NULL;
lv_obj_t * ui_LabelWeatherTemp = NULL;
lv_obj_t * ui_LabelWeatherDesc = NULL;

lv_obj_t * ui_AvatarPanel = NULL;
lv_obj_t * ui_LabelAvatar = NULL;
lv_obj_t * ui_BarProgress = NULL;
lv_obj_t * ui_LabelProgress = NULL;

lv_obj_t * ui_ChatPanel = NULL;
lv_obj_t * ui_LabelRole = NULL;
lv_obj_t * ui_LabelChat = NULL;

lv_obj_t * ui_BottomBar = NULL;
lv_obj_t * ui_LabelPrompt = NULL;
lv_obj_t * ui_BarAudio = NULL;

void ui_Screen1_screen_init(void)
{
    // 1. Root Screen: 240 x 320, Deep Dark Canvas (#11111B)
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(0x11111B), 0);
    lv_obj_set_style_bg_opa(ui_Screen1, LV_OPA_COVER, 0);

    // ==========================================
    // 2. Top Bar (Indicators): Y = 2, H = 20
    // ==========================================
    ui_TopBar = lv_obj_create(ui_Screen1);
    lv_obj_set_size(ui_TopBar, 230, 20);
    lv_obj_align(ui_TopBar, LV_ALIGN_TOP_MID, 0, 2);
    lv_obj_clear_flag(ui_TopBar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(ui_TopBar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ui_TopBar, 0, 0);
    lv_obj_set_style_pad_all(ui_TopBar, 0, 0);

    // Wi-Fi Label (Left)
    ui_LabelWifi = lv_label_create(ui_TopBar);
    lv_label_set_text(ui_LabelWifi, "WiFi");
    lv_obj_align(ui_LabelWifi, LV_ALIGN_LEFT_MID, 4, 0);
    lv_obj_set_style_text_color(ui_LabelWifi, lv_color_hex(0x89B4FA), 0);

    // Clock Label (Center)
    ui_LabelTime = lv_label_create(ui_TopBar);
    lv_label_set_text(ui_LabelTime, "12:00");
    lv_obj_align(ui_LabelTime, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(ui_LabelTime, lv_color_hex(0xCDD6F4), 0);

    // Battery Label (Right)
    ui_LabelBattery = lv_label_create(ui_TopBar);
    lv_label_set_text(ui_LabelBattery, "100%");
    lv_obj_align(ui_LabelBattery, LV_ALIGN_RIGHT_MID, -4, 0);
    lv_obj_set_style_text_color(ui_LabelBattery, lv_color_hex(0xA6E3A1), 0);

    // ==========================================
    // 3. State Indicator Pill: Y = 24, H = 22
    // ==========================================
    ui_StatusBar = lv_obj_create(ui_Screen1);
    lv_obj_set_size(ui_StatusBar, 220, 22);
    lv_obj_align(ui_StatusBar, LV_ALIGN_TOP_MID, 0, 24);
    lv_obj_clear_flag(ui_StatusBar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_StatusBar, lv_color_hex(0x1E1E2E), 0);
    lv_obj_set_style_bg_opa(ui_StatusBar, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ui_StatusBar, 11, 0);
    lv_obj_set_style_border_width(ui_StatusBar, 1, 0);
    lv_obj_set_style_border_color(ui_StatusBar, lv_color_hex(0x313244), 0);
    lv_obj_set_style_pad_all(ui_StatusBar, 0, 0);

    // LED Status Indicator Dot
    ui_LedStatus = lv_obj_create(ui_StatusBar);
    lv_obj_set_size(ui_LedStatus, 8, 8);
    lv_obj_align(ui_LedStatus, LV_ALIGN_LEFT_MID, 12, 0);
    lv_obj_set_style_radius(ui_LedStatus, 4, 0);
    lv_obj_set_style_bg_color(ui_LedStatus, lv_color_hex(0x89B4FA), 0);
    lv_obj_set_style_bg_opa(ui_LedStatus, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(ui_LedStatus, 0, 0);

    // State Text Label
    ui_LabelState = lv_label_create(ui_StatusBar);
    lv_label_set_text(ui_LabelState, "Sẵn sàng");
    lv_obj_align(ui_LabelState, LV_ALIGN_CENTER, 6, 0);
    lv_obj_set_style_text_color(ui_LabelState, lv_color_hex(0xCDD6F4), 0);

    // ==========================================
    // 4. Central Weather Card (TP. Hồ Chí Minh): Y = 50, H = 52
    // ==========================================
    ui_WeatherPanel = lv_obj_create(ui_Screen1);
    lv_obj_set_size(ui_WeatherPanel, 220, 52);
    lv_obj_align(ui_WeatherPanel, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_clear_flag(ui_WeatherPanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_WeatherPanel, lv_color_hex(0x1E1E2E), 0);
    lv_obj_set_style_bg_opa(ui_WeatherPanel, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ui_WeatherPanel, 12, 0);
    lv_obj_set_style_border_width(ui_WeatherPanel, 1, 0);
    lv_obj_set_style_border_color(ui_WeatherPanel, lv_color_hex(0x45475A), 0);
    lv_obj_set_style_pad_all(ui_WeatherPanel, 0, 0);

    // City Name
    ui_LabelWeatherCity = lv_label_create(ui_WeatherPanel);
    lv_label_set_text(ui_LabelWeatherCity, "TP. Hồ Chí Minh");
    lv_obj_align(ui_LabelWeatherCity, LV_ALIGN_TOP_LEFT, 10, 6);
    lv_obj_set_style_text_color(ui_LabelWeatherCity, lv_color_hex(0x89DCEB), 0);

    // Temperature (Gold / Warm yellow)
    ui_LabelWeatherTemp = lv_label_create(ui_WeatherPanel);
    lv_label_set_text(ui_LabelWeatherTemp, "31°C");
    lv_obj_align(ui_LabelWeatherTemp, LV_ALIGN_TOP_RIGHT, -10, 5);
    lv_obj_set_style_text_color(ui_LabelWeatherTemp, lv_color_hex(0xF9E2AF), 0);

    // Weather Description & Humidity
    ui_LabelWeatherDesc = lv_label_create(ui_WeatherPanel);
    lv_label_set_text(ui_LabelWeatherDesc, "Nhiều mây, mưa rào nhẹ • 78%");
    lv_obj_align(ui_LabelWeatherDesc, LV_ALIGN_BOTTOM_LEFT, 10, -6);
    lv_obj_set_style_text_color(ui_LabelWeatherDesc, lv_color_hex(0xA6ADC8), 0);

    // ==========================================
    // 5. Central Avatar & Expression: Y = 106, H = 58
    // ==========================================
    ui_AvatarPanel = lv_obj_create(ui_Screen1);
    lv_obj_set_size(ui_AvatarPanel, 220, 58);
    lv_obj_align(ui_AvatarPanel, LV_ALIGN_TOP_MID, 0, 106);
    lv_obj_clear_flag(ui_AvatarPanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_AvatarPanel, lv_color_hex(0x181825), 0);
    lv_obj_set_style_bg_opa(ui_AvatarPanel, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ui_AvatarPanel, 12, 0);
    lv_obj_set_style_border_width(ui_AvatarPanel, 1, 0);
    lv_obj_set_style_border_color(ui_AvatarPanel, lv_color_hex(0x313244), 0);
    lv_obj_set_style_pad_all(ui_AvatarPanel, 0, 0);

    // Avatar Face / Emoticon
    ui_LabelAvatar = lv_label_create(ui_AvatarPanel);
    lv_label_set_text(ui_LabelAvatar, "( ^ _ ^ )");
    lv_obj_align(ui_LabelAvatar, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(ui_LabelAvatar, lv_color_hex(0x89DCEB), 0);

    // OTA Progress Bar (Hidden by default)
    ui_BarProgress = lv_bar_create(ui_AvatarPanel);
    lv_bar_set_range(ui_BarProgress, 0, 100);
    lv_bar_set_value(ui_BarProgress, 0, LV_ANIM_OFF);
    lv_obj_set_size(ui_BarProgress, 180, 6);
    lv_obj_align(ui_BarProgress, LV_ALIGN_BOTTOM_MID, 0, -14);
    lv_obj_set_style_bg_color(ui_BarProgress, lv_color_hex(0x313244), 0);
    lv_obj_set_style_bg_color(ui_BarProgress, lv_color_hex(0xA6E3A1), LV_PART_INDICATOR);
    lv_obj_set_style_radius(ui_BarProgress, 3, 0);
    lv_obj_set_style_radius(ui_BarProgress, 3, LV_PART_INDICATOR);
    lv_obj_add_flag(ui_BarProgress, LV_OBJ_FLAG_HIDDEN);

    // OTA Progress Label
    ui_LabelProgress = lv_label_create(ui_AvatarPanel);
    lv_label_set_text(ui_LabelProgress, "0%");
    lv_obj_align(ui_LabelProgress, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_obj_set_style_text_color(ui_LabelProgress, lv_color_hex(0xA6E3A1), 0);
    lv_obj_add_flag(ui_LabelProgress, LV_OBJ_FLAG_HIDDEN);

    // ==========================================
    // 6. Chat & Subtitle Panel: Y = 168, H = 118
    // ==========================================
    ui_ChatPanel = lv_obj_create(ui_Screen1);
    lv_obj_set_size(ui_ChatPanel, 220, 118);
    lv_obj_align(ui_ChatPanel, LV_ALIGN_TOP_MID, 0, 168);
    lv_obj_set_style_bg_color(ui_ChatPanel, lv_color_hex(0x181825), 0);
    lv_obj_set_style_bg_opa(ui_ChatPanel, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ui_ChatPanel, 14, 0);
    lv_obj_set_style_border_width(ui_ChatPanel, 1, 0);
    lv_obj_set_style_border_color(ui_ChatPanel, lv_color_hex(0x313244), 0);
    lv_obj_set_style_pad_left(ui_ChatPanel, 10, 0);
    lv_obj_set_style_pad_right(ui_ChatPanel, 10, 0);
    lv_obj_set_style_pad_top(ui_ChatPanel, 6, 0);
    lv_obj_set_style_pad_bottom(ui_ChatPanel, 6, 0);

    // Speaker / Role Label
    ui_LabelRole = lv_label_create(ui_ChatPanel);
    lv_label_set_text(ui_LabelRole, "Xiaozhi");
    lv_obj_align(ui_LabelRole, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_color(ui_LabelRole, lv_color_hex(0xF9E2AF), 0);

    // Subtitle Text Body
    ui_LabelChat = lv_label_create(ui_ChatPanel);
    lv_obj_set_width(ui_LabelChat, 200);
    lv_label_set_long_mode(ui_LabelChat, LV_LABEL_LONG_WRAP);
    lv_label_set_text(ui_LabelChat, "Xin chao! Thoi tiet TP.HCM hom nay rat de chiu.");
    lv_obj_align(ui_LabelChat, LV_ALIGN_TOP_LEFT, 0, 18);
    lv_obj_set_style_text_color(ui_LabelChat, lv_color_hex(0xBAC2DE), 0);

    // ==========================================
    // 7. Bottom Bar & Audio Wave: Y = 290, H = 26
    // ==========================================
    ui_BottomBar = lv_obj_create(ui_Screen1);
    lv_obj_set_size(ui_BottomBar, 230, 26);
    lv_obj_align(ui_BottomBar, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_clear_flag(ui_BottomBar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(ui_BottomBar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ui_BottomBar, 0, 0);
    lv_obj_set_style_pad_all(ui_BottomBar, 0, 0);

    // Prompt Hint Label
    ui_LabelPrompt = lv_label_create(ui_BottomBar);
    lv_label_set_text(ui_LabelPrompt, "Noi 'Xiaozhi' hoac bam nut");
    lv_obj_align(ui_LabelPrompt, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(ui_LabelPrompt, lv_color_hex(0x6C7086), 0);

    // Dynamic Voice Level Bar
    ui_BarAudio = lv_bar_create(ui_BottomBar);
    lv_bar_set_range(ui_BarAudio, 0, 100);
    lv_bar_set_value(ui_BarAudio, 0, LV_ANIM_OFF);
    lv_obj_set_size(ui_BarAudio, 160, 4);
    lv_obj_align(ui_BarAudio, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(ui_BarAudio, lv_color_hex(0x313244), 0);
    lv_obj_set_style_bg_color(ui_BarAudio, lv_color_hex(0x89B4FA), LV_PART_INDICATOR);
    lv_obj_set_style_radius(ui_BarAudio, 2, 0);
    lv_obj_set_style_radius(ui_BarAudio, 2, LV_PART_INDICATOR);
    lv_obj_add_flag(ui_BarAudio, LV_OBJ_FLAG_HIDDEN);
}

void ui_Screen1_screen_destroy(void)
{
    if (ui_Screen1) {
        lv_obj_del(ui_Screen1);
        ui_Screen1 = NULL;
    }
}
