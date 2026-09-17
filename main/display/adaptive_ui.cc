#include "adaptive_ui.h"
#include "lvgl_theme.h"
#include <esp_log.h>
#include <cstring>
#include <cstdio>
#include <algorithm>

static const char* TAG = "AdaptiveUI";

AdaptiveUiEngine& AdaptiveUiEngine::GetInstance() {
    static AdaptiveUiEngine instance;
    return instance;
}

AdaptiveUiEngine::AdaptiveUiEngine() {}

AdaptiveUiEngine::~AdaptiveUiEngine() {
    Destroy();
}

void AdaptiveUiEngine::CalculateMetrics(int width, int height) {
    metrics_.width = (width > 0) ? width : 240;
    metrics_.height = (height > 0) ? height : 320;
    metrics_.is_portrait = (metrics_.height > metrics_.width);
    metrics_.is_landscape = (metrics_.width > metrics_.height);
    metrics_.is_square_or_round = (metrics_.width == metrics_.height);
    metrics_.is_mini = (metrics_.height <= 64 || metrics_.width <= 128);
    metrics_.is_compact = (!metrics_.is_mini && metrics_.width <= 240 && metrics_.height <= 280);
    metrics_.is_large = (metrics_.width >= 480 || metrics_.height >= 480);

    // Dynamic padding calculation
    if (metrics_.is_mini) {
        metrics_.pad_x = 2;
        metrics_.pad_y = 1;
        metrics_.corner_radius = 0;
        metrics_.safe_inset = 0;
    } else if (metrics_.is_compact) {
        metrics_.pad_x = 4;
        metrics_.pad_y = 3;
        metrics_.corner_radius = 6;
        metrics_.safe_inset = config_.round_screen_safe_area ? (metrics_.width * 14 / 100) : 0;
    } else if (metrics_.is_large) {
        metrics_.pad_x = 16;
        metrics_.pad_y = 12;
        metrics_.corner_radius = 16;
        metrics_.safe_inset = config_.round_screen_safe_area ? (metrics_.width * 12 / 100) : 0;
    } else {
        // Standard (240x320, 320x240, 320x480)
        metrics_.pad_x = 8;
        metrics_.pad_y = 6;
        metrics_.corner_radius = 10;
        metrics_.safe_inset = config_.round_screen_safe_area ? (metrics_.width * 14 / 100) : 0;
    }

    metrics_.content_width = metrics_.width - 2 * (metrics_.pad_x + metrics_.safe_inset);
    metrics_.content_height = metrics_.height - 2 * (metrics_.pad_y + metrics_.safe_inset);
    if (metrics_.content_width < 10) metrics_.content_width = 10;
    if (metrics_.content_height < 10) metrics_.content_height = 10;

    ESP_LOGI(TAG, "Screen metrics: %dx%d (P:%d, L:%d, Sq:%d, Mini:%d, Large:%d, Inset:%d)",
             metrics_.width, metrics_.height, metrics_.is_portrait, metrics_.is_landscape,
             metrics_.is_square_or_round, metrics_.is_mini, metrics_.is_large, metrics_.safe_inset);
}

lv_color_t AdaptiveUiEngine::GetAccentColor() const {
    switch (config_.accent) {
        case ColorAccent::EmeraldGreen:
            return lv_color_hex(0xA6E3A1);
        case ColorAccent::ElectricPurple:
            return lv_color_hex(0xCBA6F7);
        case ColorAccent::SunsetAmber:
            return lv_color_hex(0xF9E2AF);
        case ColorAccent::Monochrome:
            return lv_color_hex(0xFFFFFF);
        case ColorAccent::CyberBlue:
        default:
            return lv_color_hex(0x89DCEB);
    }
}

lv_color_t AdaptiveUiEngine::GetBgColor() const {
    switch (config_.style) {
        case UiStyle::CyberTerminal:
            return lv_color_hex(0x050A0E); // Deep sci-fi black
        case UiStyle::MinimalZen:
            return lv_color_hex(0x000000); // Pure OLED black
        case UiStyle::ChatBubble:
            return lv_color_hex(0x111116); // Soft charcoal
        case UiStyle::ClassicAvatar:
            return lv_color_hex(0x0D1117); // Dark navy slate
        case UiStyle::SmartDashboard:
        default:
            return lv_color_hex(0x11111B); // Catppuccin deep crust
    }
}

lv_color_t AdaptiveUiEngine::GetCardBgColor() const {
    switch (config_.style) {
        case UiStyle::CyberTerminal:
            return lv_color_hex(0x0A141E);
        case UiStyle::MinimalZen:
            return lv_color_hex(0x000000);
        case UiStyle::ChatBubble:
            return lv_color_hex(0x1E1E28);
        case UiStyle::ClassicAvatar:
            return lv_color_hex(0x161B22);
        case UiStyle::SmartDashboard:
        default:
            return lv_color_hex(0x1E1E2E);
    }
}

lv_color_t AdaptiveUiEngine::GetBorderColor() const {
    switch (config_.style) {
        case UiStyle::CyberTerminal:
            return lv_color_hex(0x00F0FF); // Neon cyan border
        case UiStyle::MinimalZen:
            return lv_color_hex(0x444444);
        case UiStyle::ChatBubble:
            return lv_color_hex(0x2D2D3D);
        case UiStyle::ClassicAvatar:
            return lv_color_hex(0x30363D);
        case UiStyle::SmartDashboard:
        default:
            return lv_color_hex(0x313244);
    }
}

lv_color_t AdaptiveUiEngine::GetTextColor() const {
    return lv_color_hex(0xCDD6F4);
}

lv_color_t AdaptiveUiEngine::GetMutedTextColor() const {
    return lv_color_hex(0x6C7086);
}

void AdaptiveUiEngine::Destroy() {
    if (container_root_ != nullptr) {
        lv_obj_del(container_root_);
        container_root_ = nullptr;
    }
    root_screen_ = nullptr;
    panel_top_bar_ = nullptr;
    label_network_ = nullptr;
    label_time_ = nullptr;
    label_battery_ = nullptr;
    label_notification_ = nullptr;
    panel_state_pill_ = nullptr;
    led_state_dot_ = nullptr;
    label_status_ = nullptr;
    panel_weather_ = nullptr;
    label_weather_city_ = nullptr;
    label_weather_temp_ = nullptr;
    label_weather_desc_ = nullptr;
    panel_avatar_ = nullptr;
    label_avatar_ = nullptr;
    bar_ota_progress_ = nullptr;
    label_ota_progress_ = nullptr;
    panel_chat_ = nullptr;
    label_chat_role_ = nullptr;
    label_chat_ = nullptr;
    panel_user_bubble_ = nullptr;
    label_user_chat_ = nullptr;
    panel_bottom_bar_ = nullptr;
    label_prompt_ = nullptr;
    bar_audio_wave_ = nullptr;
    initialized_ = false;
}

void AdaptiveUiEngine::Initialize(lv_obj_t* parent_screen, int width, int height, const AdaptiveUiConfig& config) {
    Destroy();
    config_ = config;
    CalculateMetrics(width, height);

    root_screen_ = parent_screen ? parent_screen : lv_screen_active();
    if (!root_screen_) {
        ESP_LOGE(TAG, "Root screen is nullptr!");
        return;
    }

    // Root container covering the entire display area
    container_root_ = lv_obj_create(root_screen_);
    lv_obj_set_size(container_root_, metrics_.width, metrics_.height);
    lv_obj_align(container_root_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(container_root_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(container_root_, GetBgColor(), 0);
    lv_obj_set_style_bg_opa(container_root_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container_root_, 0, 0);
    lv_obj_set_style_radius(container_root_, 0, 0);
    lv_obj_set_style_pad_all(container_root_, 0, 0);

    // Dispatch builder according to chosen UI style
    switch (config_.style) {
        case UiStyle::CyberTerminal:
            BuildCyberTerminal(container_root_);
            break;
        case UiStyle::ChatBubble:
            BuildChatBubble(container_root_);
            break;
        case UiStyle::ClassicAvatar:
            BuildClassicAvatar(container_root_);
            break;
        case UiStyle::MinimalZen:
            BuildMinimalZen(container_root_);
            break;
        case UiStyle::SmartDashboard:
        default:
            BuildSmartDashboard(container_root_);
            break;
    }

    initialized_ = true;
    ESP_LOGI(TAG, "Adaptive UI initialized successfully with style %d", (int)config_.style);
}

/* ========================================================================= */
/*                      1. MODERN SMART DASHBOARD                            */
/* ========================================================================= */
void AdaptiveUiEngine::BuildSmartDashboard(lv_obj_t* parent) {
    int top_bar_h = metrics_.is_mini ? 16 : (metrics_.is_large ? 28 : 20);
    int pill_h = metrics_.is_mini ? 14 : (metrics_.is_large ? 28 : 22);
    int weather_h = metrics_.is_mini ? 0 : (metrics_.is_large ? 72 : 50);
    int avatar_h = metrics_.is_mini ? 16 : (metrics_.is_large ? 80 : 54);
    int bottom_bar_h = metrics_.is_mini ? 12 : (metrics_.is_large ? 32 : 24);

    // 1. Top Bar
    panel_top_bar_ = lv_obj_create(parent);
    lv_obj_set_size(panel_top_bar_, metrics_.content_width, top_bar_h);
    lv_obj_align(panel_top_bar_, LV_ALIGN_TOP_MID, 0, metrics_.pad_y);
    lv_obj_clear_flag(panel_top_bar_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(panel_top_bar_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(panel_top_bar_, 0, 0);
    lv_obj_set_style_pad_all(panel_top_bar_, 0, 0);

    label_network_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_network_, "WiFi");
    lv_obj_align(label_network_, LV_ALIGN_LEFT_MID, 2, 0);
    lv_obj_set_style_text_color(label_network_, GetAccentColor(), 0);

    label_time_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_time_, "12:00");
    lv_obj_align(label_time_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_time_, GetTextColor(), 0);

    label_battery_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_battery_, "100%");
    lv_obj_align(label_battery_, LV_ALIGN_RIGHT_MID, -2, 0);
    lv_obj_set_style_text_color(label_battery_, lv_color_hex(0xA6E3A1), 0);

    // Notification label overlay (hidden by default)
    label_notification_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_notification_, "");
    lv_obj_align(label_notification_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_notification_, lv_color_hex(0xF9E2AF), 0);
    lv_obj_add_flag(label_notification_, LV_OBJ_FLAG_HIDDEN);

    int cur_y = metrics_.pad_y + top_bar_h + 3;

    // 2. State Pill
    panel_state_pill_ = lv_obj_create(parent);
    lv_obj_set_size(panel_state_pill_, metrics_.content_width, pill_h);
    lv_obj_align(panel_state_pill_, LV_ALIGN_TOP_MID, 0, cur_y);
    lv_obj_clear_flag(panel_state_pill_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(panel_state_pill_, GetCardBgColor(), 0);
    lv_obj_set_style_bg_opa(panel_state_pill_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(panel_state_pill_, pill_h / 2, 0);
    lv_obj_set_style_border_width(panel_state_pill_, 1, 0);
    lv_obj_set_style_border_color(panel_state_pill_, GetBorderColor(), 0);
    lv_obj_set_style_pad_all(panel_state_pill_, 0, 0);

    led_state_dot_ = lv_obj_create(panel_state_pill_);
    int dot_size = pill_h > 18 ? 8 : 6;
    lv_obj_set_size(led_state_dot_, dot_size, dot_size);
    lv_obj_align(led_state_dot_, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_style_radius(led_state_dot_, dot_size / 2, 0);
    lv_obj_set_style_bg_color(led_state_dot_, GetAccentColor(), 0);
    lv_obj_set_style_border_width(led_state_dot_, 0, 0);

    label_status_ = lv_label_create(panel_state_pill_);
    lv_label_set_text(label_status_, "Sẵn sàng");
    lv_obj_align(label_status_, LV_ALIGN_CENTER, 6, 0);
    lv_obj_set_style_text_color(label_status_, GetTextColor(), 0);

    cur_y += pill_h + 4;

    // 3. Weather Card (if enabled and space permits)
    if (config_.weather_enabled && !metrics_.is_mini && (metrics_.height >= 240)) {
        panel_weather_ = lv_obj_create(parent);
        lv_obj_set_size(panel_weather_, metrics_.content_width, weather_h);
        lv_obj_align(panel_weather_, LV_ALIGN_TOP_MID, 0, cur_y);
        lv_obj_clear_flag(panel_weather_, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(panel_weather_, GetCardBgColor(), 0);
        lv_obj_set_style_bg_opa(panel_weather_, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(panel_weather_, metrics_.corner_radius, 0);
        lv_obj_set_style_border_width(panel_weather_, 1, 0);
        lv_obj_set_style_border_color(panel_weather_, GetBorderColor(), 0);
        lv_obj_set_style_pad_all(panel_weather_, 0, 0);

        label_weather_city_ = lv_label_create(panel_weather_);
        lv_label_set_text(label_weather_city_, config_.weather_city.c_str());
        lv_obj_align(label_weather_city_, LV_ALIGN_TOP_LEFT, 8, 4);
        lv_obj_set_style_text_color(label_weather_city_, GetAccentColor(), 0);

        label_weather_temp_ = lv_label_create(panel_weather_);
        lv_label_set_text(label_weather_temp_, "31°C");
        lv_obj_align(label_weather_temp_, LV_ALIGN_TOP_RIGHT, -8, 4);
        lv_obj_set_style_text_color(label_weather_temp_, lv_color_hex(0xF9E2AF), 0);

        label_weather_desc_ = lv_label_create(panel_weather_);
        lv_label_set_text(label_weather_desc_, "Nhiều mây, mưa rào nhẹ - 78%");
        lv_obj_align(label_weather_desc_, LV_ALIGN_BOTTOM_LEFT, 8, -4);
        lv_obj_set_style_text_color(label_weather_desc_, GetMutedTextColor(), 0);

        cur_y += weather_h + 4;
    }

    // 4. Central Avatar & Expression Card
    if (!metrics_.is_mini) {
        panel_avatar_ = lv_obj_create(parent);
        lv_obj_set_size(panel_avatar_, metrics_.content_width, avatar_h);
        lv_obj_align(panel_avatar_, LV_ALIGN_TOP_MID, 0, cur_y);
        lv_obj_clear_flag(panel_avatar_, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(panel_avatar_, GetCardBgColor(), 0);
        lv_obj_set_style_bg_opa(panel_avatar_, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(panel_avatar_, metrics_.corner_radius, 0);
        lv_obj_set_style_border_width(panel_avatar_, 1, 0);
        lv_obj_set_style_border_color(panel_avatar_, GetBorderColor(), 0);
        lv_obj_set_style_pad_all(panel_avatar_, 0, 0);

        label_avatar_ = lv_label_create(panel_avatar_);
        lv_label_set_text(label_avatar_, "( ^ _ ^ )");
        lv_obj_align(label_avatar_, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(label_avatar_, GetAccentColor(), 0);

        // OTA progress bar (hidden by default)
        bar_ota_progress_ = lv_bar_create(panel_avatar_);
        lv_bar_set_range(bar_ota_progress_, 0, 100);
        lv_bar_set_value(bar_ota_progress_, 0, LV_ANIM_OFF);
        lv_obj_set_size(bar_ota_progress_, metrics_.content_width * 80 / 100, 6);
        lv_obj_align(bar_ota_progress_, LV_ALIGN_BOTTOM_MID, 0, -12);
        lv_obj_set_style_bg_color(bar_ota_progress_, lv_color_hex(0x313244), 0);
        lv_obj_set_style_bg_color(bar_ota_progress_, lv_color_hex(0xA6E3A1), LV_PART_INDICATOR);
        lv_obj_add_flag(bar_ota_progress_, LV_OBJ_FLAG_HIDDEN);

        label_ota_progress_ = lv_label_create(panel_avatar_);
        lv_label_set_text(label_ota_progress_, "0%");
        lv_obj_align(label_ota_progress_, LV_ALIGN_BOTTOM_MID, 0, -2);
        lv_obj_set_style_text_color(label_ota_progress_, lv_color_hex(0xA6E3A1), 0);
        lv_obj_add_flag(label_ota_progress_, LV_OBJ_FLAG_HIDDEN);

        cur_y += avatar_h + 4;
    }

    // 5. Chat & Subtitle Panel (takes remaining vertical space)
    int chat_h = metrics_.height - cur_y - bottom_bar_h - metrics_.pad_y - 4;
    if (chat_h < 30) chat_h = 30;

    panel_chat_ = lv_obj_create(parent);
    lv_obj_set_size(panel_chat_, metrics_.content_width, chat_h);
    lv_obj_align(panel_chat_, LV_ALIGN_TOP_MID, 0, cur_y);
    lv_obj_set_style_bg_color(panel_chat_, GetCardBgColor(), 0);
    lv_obj_set_style_bg_opa(panel_chat_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(panel_chat_, metrics_.corner_radius, 0);
    lv_obj_set_style_border_width(panel_chat_, 1, 0);
    lv_obj_set_style_border_color(panel_chat_, GetBorderColor(), 0);
    lv_obj_set_style_pad_all(panel_chat_, 6, 0);

    label_chat_role_ = lv_label_create(panel_chat_);
    lv_label_set_text(label_chat_role_, "Xiaozhi");
    lv_obj_align(label_chat_role_, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_color(label_chat_role_, lv_color_hex(0xF9E2AF), 0);

    label_chat_ = lv_label_create(panel_chat_);
    lv_obj_set_width(label_chat_, metrics_.content_width - 16);
    lv_label_set_long_mode(label_chat_, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label_chat_, "Xin chào! Tôi là trợ lý AI Xiaozhi.");
    lv_obj_align(label_chat_, LV_ALIGN_TOP_LEFT, 0, 16);
    lv_obj_set_style_text_color(label_chat_, GetTextColor(), 0);

    // 6. Bottom Navigation & Voice Wave Bar
    panel_bottom_bar_ = lv_obj_create(parent);
    lv_obj_set_size(panel_bottom_bar_, metrics_.content_width, bottom_bar_h);
    lv_obj_align(panel_bottom_bar_, LV_ALIGN_BOTTOM_MID, 0, -metrics_.pad_y);
    lv_obj_clear_flag(panel_bottom_bar_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(panel_bottom_bar_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(panel_bottom_bar_, 0, 0);
    lv_obj_set_style_pad_all(panel_bottom_bar_, 0, 0);

    label_prompt_ = lv_label_create(panel_bottom_bar_);
    lv_label_set_text(label_prompt_, "Nói 'Xiaozhi' hoặc bấm nút");
    lv_obj_align(label_prompt_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_prompt_, GetMutedTextColor(), 0);

    if (config_.voice_wave_enabled) {
        bar_audio_wave_ = lv_bar_create(panel_bottom_bar_);
        lv_bar_set_range(bar_audio_wave_, 0, 100);
        lv_bar_set_value(bar_audio_wave_, 0, LV_ANIM_OFF);
        lv_obj_set_size(bar_audio_wave_, metrics_.content_width * 75 / 100, 4);
        lv_obj_align(bar_audio_wave_, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(bar_audio_wave_, lv_color_hex(0x313244), 0);
        lv_obj_set_style_bg_color(bar_audio_wave_, GetAccentColor(), LV_PART_INDICATOR);
        lv_obj_set_style_radius(bar_audio_wave_, 2, 0);
        lv_obj_set_style_radius(bar_audio_wave_, 2, LV_PART_INDICATOR);
        lv_obj_add_flag(bar_audio_wave_, LV_OBJ_FLAG_HIDDEN);
    }
}

/* ========================================================================= */
/*                      2. CYBERPUNK SCI-FI HUD                              */
/* ========================================================================= */
void AdaptiveUiEngine::BuildCyberTerminal(lv_obj_t* parent) {
    // Cyberpunk style: angular frame, neon cyan, terminal log style
    int header_h = metrics_.is_large ? 32 : 22;

    panel_top_bar_ = lv_obj_create(parent);
    lv_obj_set_size(panel_top_bar_, metrics_.content_width, header_h);
    lv_obj_align(panel_top_bar_, LV_ALIGN_TOP_MID, 0, metrics_.pad_y);
    lv_obj_set_style_bg_color(panel_top_bar_, lv_color_hex(0x07111A), 0);
    lv_obj_set_style_border_color(panel_top_bar_, lv_color_hex(0x00F0FF), 0);
    lv_obj_set_style_border_width(panel_top_bar_, 1, 0);
    lv_obj_set_style_radius(panel_top_bar_, 2, 0);
    lv_obj_set_style_pad_all(panel_top_bar_, 2, 0);

    label_network_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_network_, "[NET:OK]");
    lv_obj_align(label_network_, LV_ALIGN_LEFT_MID, 4, 0);
    lv_obj_set_style_text_color(label_network_, lv_color_hex(0x00F0FF), 0);

    label_time_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_time_, "[12:00:00]");
    lv_obj_align(label_time_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_time_, lv_color_hex(0x00FF66), 0);

    label_battery_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_battery_, "[PWR:98%]");
    lv_obj_align(label_battery_, LV_ALIGN_RIGHT_MID, -4, 0);
    lv_obj_set_style_text_color(label_battery_, lv_color_hex(0xFFB800), 0);

    label_notification_ = lv_label_create(parent);
    lv_label_set_text(label_notification_, "");
    lv_obj_align(label_notification_, LV_ALIGN_TOP_MID, 0, metrics_.pad_y + header_h + 2);
    lv_obj_set_style_text_color(label_notification_, lv_color_hex(0xFFB800), 0);
    lv_obj_add_flag(label_notification_, LV_OBJ_FLAG_HIDDEN);

    int cur_y = metrics_.pad_y + header_h + 6;

    // Hologram AI Core Center
    panel_avatar_ = lv_obj_create(parent);
    int core_h = metrics_.is_large ? 100 : (metrics_.is_compact ? 45 : 65);
    lv_obj_set_size(panel_avatar_, metrics_.content_width, core_h);
    lv_obj_align(panel_avatar_, LV_ALIGN_TOP_MID, 0, cur_y);
    lv_obj_set_style_bg_color(panel_avatar_, lv_color_hex(0x081624), 0);
    lv_obj_set_style_border_color(panel_avatar_, lv_color_hex(0x00F0FF), 0);
    lv_obj_set_style_border_width(panel_avatar_, 1, 0);
    lv_obj_set_style_radius(panel_avatar_, 4, 0);

    label_avatar_ = lv_label_create(panel_avatar_);
    lv_label_set_text(label_avatar_, "« [ AI REACTOR ONLINE ] »");
    lv_obj_align(label_avatar_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_avatar_, lv_color_hex(0x00F0FF), 0);

    bar_ota_progress_ = lv_bar_create(panel_avatar_);
    lv_bar_set_range(bar_ota_progress_, 0, 100);
    lv_obj_set_size(bar_ota_progress_, metrics_.content_width * 80 / 100, 4);
    lv_obj_align(bar_ota_progress_, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_color(bar_ota_progress_, lv_color_hex(0x003344), 0);
    lv_obj_set_style_bg_color(bar_ota_progress_, lv_color_hex(0x00FF66), LV_PART_INDICATOR);
    lv_obj_add_flag(bar_ota_progress_, LV_OBJ_FLAG_HIDDEN);

    label_ota_progress_ = lv_label_create(panel_avatar_);
    lv_label_set_text(label_ota_progress_, "0%");
    lv_obj_align(label_ota_progress_, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_obj_set_style_text_color(label_ota_progress_, lv_color_hex(0x00FF66), 0);
    lv_obj_add_flag(label_ota_progress_, LV_OBJ_FLAG_HIDDEN);

    cur_y += core_h + 6;

    // Telemetry Line (State & Weather)
    panel_state_pill_ = lv_obj_create(parent);
    lv_obj_set_size(panel_state_pill_, metrics_.content_width, 24);
    lv_obj_align(panel_state_pill_, LV_ALIGN_TOP_MID, 0, cur_y);
    lv_obj_set_style_bg_color(panel_state_pill_, lv_color_hex(0x07111A), 0);
    lv_obj_set_style_border_color(panel_state_pill_, lv_color_hex(0x00F0FF), 0);
    lv_obj_set_style_border_width(panel_state_pill_, 1, 0);
    lv_obj_set_style_radius(panel_state_pill_, 2, 0);

    led_state_dot_ = lv_obj_create(panel_state_pill_);
    lv_obj_set_size(led_state_dot_, 6, 6);
    lv_obj_align(led_state_dot_, LV_ALIGN_LEFT_MID, 6, 0);
    lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0x00FF66), 0);
    lv_obj_set_style_radius(led_state_dot_, 3, 0);

    label_status_ = lv_label_create(panel_state_pill_);
    lv_label_set_text(label_status_, "SYS: STANDBY // READY");
    lv_obj_align(label_status_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_status_, lv_color_hex(0xCDD6F4), 0);

    cur_y += 28;

    // Terminal Data Stream Log (Chat card)
    int chat_h = metrics_.height - cur_y - 28 - metrics_.pad_y;
    if (chat_h < 35) chat_h = 35;

    panel_chat_ = lv_obj_create(parent);
    lv_obj_set_size(panel_chat_, metrics_.content_width, chat_h);
    lv_obj_align(panel_chat_, LV_ALIGN_TOP_MID, 0, cur_y);
    lv_obj_set_style_bg_color(panel_chat_, lv_color_hex(0x050D14), 0);
    lv_obj_set_style_border_color(panel_chat_, lv_color_hex(0x00F0FF), 0);
    lv_obj_set_style_border_width(panel_chat_, 1, 0);
    lv_obj_set_style_radius(panel_chat_, 2, 0);
    lv_obj_set_style_pad_all(panel_chat_, 6, 0);

    label_chat_role_ = lv_label_create(panel_chat_);
    lv_label_set_text(label_chat_role_, "> STREAM LINKED:");
    lv_obj_align(label_chat_role_, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_color(label_chat_role_, lv_color_hex(0x00F0FF), 0);

    label_chat_ = lv_label_create(panel_chat_);
    lv_obj_set_width(label_chat_, metrics_.content_width - 16);
    lv_label_set_long_mode(label_chat_, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label_chat_, "System ready. Awaiting voice input...");
    lv_obj_align(label_chat_, LV_ALIGN_TOP_LEFT, 0, 16);
    lv_obj_set_style_text_color(label_chat_, lv_color_hex(0x00FF66), 0);

    // Audio Visualizer Bar at bottom
    panel_bottom_bar_ = lv_obj_create(parent);
    lv_obj_set_size(panel_bottom_bar_, metrics_.content_width, 20);
    lv_obj_align(panel_bottom_bar_, LV_ALIGN_BOTTOM_MID, 0, -metrics_.pad_y);
    lv_obj_clear_flag(panel_bottom_bar_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(panel_bottom_bar_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(panel_bottom_bar_, 0, 0);

    bar_audio_wave_ = lv_bar_create(panel_bottom_bar_);
    lv_bar_set_range(bar_audio_wave_, 0, 100);
    lv_bar_set_value(bar_audio_wave_, 0, LV_ANIM_OFF);
    lv_obj_set_size(bar_audio_wave_, metrics_.content_width * 90 / 100, 4);
    lv_obj_align(bar_audio_wave_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(bar_audio_wave_, lv_color_hex(0x003344), 0);
    lv_obj_set_style_bg_color(bar_audio_wave_, lv_color_hex(0x00F0FF), LV_PART_INDICATOR);
    lv_obj_add_flag(bar_audio_wave_, LV_OBJ_FLAG_HIDDEN);

    label_prompt_ = lv_label_create(panel_bottom_bar_);
    lv_label_set_text(label_prompt_, "[LISTEN_PORT: ACTIVE]");
    lv_obj_align(label_prompt_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_prompt_, lv_color_hex(0x00F0FF), 0);
}

/* ========================================================================= */
/*                      3. CHAT BUBBLE MESSENGER                             */
/* ========================================================================= */
void AdaptiveUiEngine::BuildChatBubble(lv_obj_t* parent) {
    // WeChat / Telegram style dual bubble flow
    int header_h = 20;

    panel_top_bar_ = lv_obj_create(parent);
    lv_obj_set_size(panel_top_bar_, metrics_.content_width, header_h);
    lv_obj_align(panel_top_bar_, LV_ALIGN_TOP_MID, 0, metrics_.pad_y);
    lv_obj_set_style_bg_opa(panel_top_bar_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(panel_top_bar_, 0, 0);
    lv_obj_set_style_pad_all(panel_top_bar_, 0, 0);

    label_network_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_network_, "Xiaozhi AI");
    lv_obj_align(label_network_, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_color(label_network_, lv_color_hex(0x89DCEB), 0);

    label_battery_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_battery_, "100%");
    lv_obj_align(label_battery_, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_text_color(label_battery_, lv_color_hex(0xA6E3A1), 0);

    label_notification_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_notification_, "");
    lv_obj_align(label_notification_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_notification_, lv_color_hex(0xF9E2AF), 0);
    lv_obj_add_flag(label_notification_, LV_OBJ_FLAG_HIDDEN);

    int cur_y = metrics_.pad_y + header_h + 4;

    // Status strip
    panel_state_pill_ = lv_obj_create(parent);
    lv_obj_set_size(panel_state_pill_, metrics_.content_width, 20);
    lv_obj_align(panel_state_pill_, LV_ALIGN_TOP_MID, 0, cur_y);
    lv_obj_set_style_bg_color(panel_state_pill_, lv_color_hex(0x181824), 0);
    lv_obj_set_style_border_width(panel_state_pill_, 0, 0);
    lv_obj_set_style_radius(panel_state_pill_, 10, 0);

    led_state_dot_ = lv_obj_create(panel_state_pill_);
    lv_obj_set_size(led_state_dot_, 6, 6);
    lv_obj_align(led_state_dot_, LV_ALIGN_LEFT_MID, 8, 0);
    lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0x89DCEB), 0);
    lv_obj_set_style_radius(led_state_dot_, 3, 0);

    label_status_ = lv_label_create(panel_state_pill_);
    lv_label_set_text(label_status_, "Trực tuyến");
    lv_obj_align(label_status_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_status_, lv_color_hex(0xCDD6F4), 0);

    cur_y += 24;

    // Chat Bubbles area
    int bubble_w = metrics_.content_width * 85 / 100;

    // Assistant Bubble (Left)
    panel_chat_ = lv_obj_create(parent);
    lv_obj_set_size(panel_chat_, bubble_w, LV_SIZE_CONTENT);
    lv_obj_align(panel_chat_, LV_ALIGN_TOP_LEFT, metrics_.pad_x, cur_y);
    lv_obj_set_style_bg_color(panel_chat_, lv_color_hex(0x252536), 0);
    lv_obj_set_style_radius(panel_chat_, 12, 0);
    lv_obj_set_style_border_width(panel_chat_, 0, 0);
    lv_obj_set_style_pad_all(panel_chat_, 8, 0);

    label_chat_role_ = lv_label_create(panel_chat_);
    lv_label_set_text(label_chat_role_, "Xiaozhi");
    lv_obj_align(label_chat_role_, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_color(label_chat_role_, lv_color_hex(0x89DCEB), 0);

    label_chat_ = lv_label_create(panel_chat_);
    lv_obj_set_width(label_chat_, bubble_w - 16);
    lv_label_set_long_mode(label_chat_, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label_chat_, "Xin chào! Bạn muốn trò chuyện về chủ đề gì?");
    lv_obj_align(label_chat_, LV_ALIGN_TOP_LEFT, 0, 16);
    lv_obj_set_style_text_color(label_chat_, lv_color_hex(0xFFFFFF), 0);

    // User Bubble (Right)
    panel_user_bubble_ = lv_obj_create(parent);
    lv_obj_set_size(panel_user_bubble_, bubble_w, LV_SIZE_CONTENT);
    lv_obj_align(panel_user_bubble_, LV_ALIGN_TOP_RIGHT, -metrics_.pad_x, cur_y + 70);
    lv_obj_set_style_bg_color(panel_user_bubble_, lv_color_hex(0x07C160), 0); // WeChat Green
    lv_obj_set_style_radius(panel_user_bubble_, 12, 0);
    lv_obj_set_style_border_width(panel_user_bubble_, 0, 0);
    lv_obj_set_style_pad_all(panel_user_bubble_, 8, 0);
    lv_obj_add_flag(panel_user_bubble_, LV_OBJ_FLAG_HIDDEN); // Show when user speaks

    label_user_chat_ = lv_label_create(panel_user_bubble_);
    lv_obj_set_width(label_user_chat_, bubble_w - 16);
    lv_label_set_long_mode(label_user_chat_, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label_user_chat_, "");
    lv_obj_align(label_user_chat_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_user_chat_, lv_color_hex(0xFFFFFF), 0);

    // Bottom Voice Pill
    panel_bottom_bar_ = lv_obj_create(parent);
    lv_obj_set_size(panel_bottom_bar_, metrics_.content_width, 28);
    lv_obj_align(panel_bottom_bar_, LV_ALIGN_BOTTOM_MID, 0, -metrics_.pad_y);
    lv_obj_set_style_bg_color(panel_bottom_bar_, lv_color_hex(0x1F1F2E), 0);
    lv_obj_set_style_radius(panel_bottom_bar_, 14, 0);
    lv_obj_set_style_border_width(panel_bottom_bar_, 1, 0);
    lv_obj_set_style_border_color(panel_bottom_bar_, lv_color_hex(0x313244), 0);

    label_prompt_ = lv_label_create(panel_bottom_bar_);
    lv_label_set_text(label_prompt_, "Giữ hoặc nói 'Xiaozhi'...");
    lv_obj_align(label_prompt_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_prompt_, GetMutedTextColor(), 0);

    bar_audio_wave_ = lv_bar_create(panel_bottom_bar_);
    lv_bar_set_range(bar_audio_wave_, 0, 100);
    lv_obj_set_size(bar_audio_wave_, metrics_.content_width * 70 / 100, 4);
    lv_obj_align(bar_audio_wave_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(bar_audio_wave_, lv_color_hex(0x07C160), LV_PART_INDICATOR);
    lv_obj_add_flag(bar_audio_wave_, LV_OBJ_FLAG_HIDDEN);
}

/* ========================================================================= */
/*                      4. CLASSIC CUTE ROBOT AVATAR                         */
/* ========================================================================= */
void AdaptiveUiEngine::BuildClassicAvatar(lv_obj_t* parent) {
    // 70-80% of screen is giant emotion face, perfect for round GC9A01 & square screens!
    panel_top_bar_ = lv_obj_create(parent);
    lv_obj_set_size(panel_top_bar_, metrics_.content_width, 18);
    lv_obj_align(panel_top_bar_, LV_ALIGN_TOP_MID, 0, metrics_.pad_y + metrics_.safe_inset / 2);
    lv_obj_set_style_bg_opa(panel_top_bar_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(panel_top_bar_, 0, 0);

    label_network_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_network_, "•");
    lv_obj_align(label_network_, LV_ALIGN_LEFT_MID, 2, 0);
    lv_obj_set_style_text_color(label_network_, GetAccentColor(), 0);

    label_battery_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_battery_, "100%");
    lv_obj_align(label_battery_, LV_ALIGN_RIGHT_MID, -2, 0);
    lv_obj_set_style_text_color(label_battery_, lv_color_hex(0xA6E3A1), 0);

    label_notification_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_notification_, "");
    lv_obj_align(label_notification_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_notification_, lv_color_hex(0xF9E2AF), 0);
    lv_obj_add_flag(label_notification_, LV_OBJ_FLAG_HIDDEN);

    // Huge Avatar Center Stage
    panel_avatar_ = lv_obj_create(parent);
    int face_size = std::min(metrics_.content_width, metrics_.content_height * 65 / 100);
    lv_obj_set_size(panel_avatar_, face_size, face_size);
    lv_obj_align(panel_avatar_, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_bg_color(panel_avatar_, lv_color_hex(0x161B22), 0);
    lv_obj_set_style_radius(panel_avatar_, face_size / 2, 0); // Circular face container
    lv_obj_set_style_border_color(panel_avatar_, GetAccentColor(), 0);
    lv_obj_set_style_border_width(panel_avatar_, 2, 0);

    label_avatar_ = lv_label_create(panel_avatar_);
    lv_label_set_text(label_avatar_, "( ^ _ ^ )");
    lv_obj_align(label_avatar_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_avatar_, GetAccentColor(), 0);

    bar_ota_progress_ = lv_bar_create(panel_avatar_);
    lv_bar_set_range(bar_ota_progress_, 0, 100);
    lv_obj_set_size(bar_ota_progress_, face_size * 70 / 100, 6);
    lv_obj_align(bar_ota_progress_, LV_ALIGN_BOTTOM_MID, 0, -15);
    lv_obj_set_style_bg_color(bar_ota_progress_, lv_color_hex(0xA6E3A1), LV_PART_INDICATOR);
    lv_obj_add_flag(bar_ota_progress_, LV_OBJ_FLAG_HIDDEN);

    label_ota_progress_ = lv_label_create(panel_avatar_);
    lv_label_set_text(label_ota_progress_, "0%");
    lv_obj_align(label_ota_progress_, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_obj_set_style_text_color(label_ota_progress_, lv_color_hex(0xA6E3A1), 0);
    lv_obj_add_flag(label_ota_progress_, LV_OBJ_FLAG_HIDDEN);

    // Bottom compact Subtitle strip
    panel_chat_ = lv_obj_create(parent);
    lv_obj_set_size(panel_chat_, metrics_.content_width, 32);
    lv_obj_align(panel_chat_, LV_ALIGN_BOTTOM_MID, 0, -metrics_.pad_y - metrics_.safe_inset / 2);
    lv_obj_set_style_bg_color(panel_chat_, lv_color_hex(0x1F242C), 0);
    lv_obj_set_style_radius(panel_chat_, 16, 0);
    lv_obj_set_style_border_width(panel_chat_, 1, 0);
    lv_obj_set_style_border_color(panel_chat_, lv_color_hex(0x30363D), 0);
    lv_obj_set_style_pad_all(panel_chat_, 4, 0);

    label_chat_ = lv_label_create(panel_chat_);
    lv_obj_set_width(label_chat_, metrics_.content_width - 24);
    lv_label_set_long_mode(label_chat_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(label_chat_, "Xiaozhi sẵn sàng!");
    lv_obj_align(label_chat_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_chat_, GetTextColor(), 0);

    label_status_ = label_chat_; // Alias status to subtitle bar
}

/* ========================================================================= */
/*                      5. MINIMALIST ZEN / E-PAPER                          */
/* ========================================================================= */
void AdaptiveUiEngine::BuildMinimalZen(lv_obj_t* parent) {
    // Scandinavian monochrome zen style: clean dividers, high contrast
    panel_top_bar_ = lv_obj_create(parent);
    lv_obj_set_size(panel_top_bar_, metrics_.content_width, 22);
    lv_obj_align(panel_top_bar_, LV_ALIGN_TOP_MID, 0, metrics_.pad_y);
    lv_obj_set_style_bg_opa(panel_top_bar_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(panel_top_bar_, 0, 0);

    label_network_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_network_, "ONLINE");
    lv_obj_align(label_network_, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_color(label_network_, lv_color_hex(0xFFFFFF), 0);

    label_battery_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_battery_, "BAT: 100%");
    lv_obj_align(label_battery_, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_text_color(label_battery_, lv_color_hex(0xFFFFFF), 0);

    label_notification_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_notification_, "");
    lv_obj_align(label_notification_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_notification_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_add_flag(label_notification_, LV_OBJ_FLAG_HIDDEN);

    // Large Typography Clock in Center
    label_time_ = lv_label_create(parent);
    lv_label_set_text(label_time_, "12:00");
    lv_obj_align(label_time_, LV_ALIGN_CENTER, 0, -30);
    lv_obj_set_style_text_color(label_time_, lv_color_hex(0xFFFFFF), 0);

    // Elegant Hairline Divider
    lv_obj_t* divider = lv_obj_create(parent);
    lv_obj_set_size(divider, metrics_.content_width * 80 / 100, 1);
    lv_obj_align(divider, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(divider, lv_color_hex(0x666666), 0);
    lv_obj_set_style_border_width(divider, 0, 0);

    // State Text
    label_status_ = lv_label_create(parent);
    lv_label_set_text(label_status_, "SẴN SÀNG");
    lv_obj_align(label_status_, LV_ALIGN_CENTER, 0, 18);
    lv_obj_set_style_text_color(label_status_, lv_color_hex(0xAAAAAA), 0);

    // Subtitle Line at bottom
    label_chat_ = lv_label_create(parent);
    lv_obj_set_width(label_chat_, metrics_.content_width);
    lv_label_set_long_mode(label_chat_, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label_chat_, "");
    lv_obj_align(label_chat_, LV_ALIGN_BOTTOM_MID, 0, -metrics_.pad_y);
    lv_obj_set_style_text_align(label_chat_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label_chat_, lv_color_hex(0xFFFFFF), 0);
}

/* ========================================================================= */
/*                      STATE & EVENT HANDLERS                               */
/* ========================================================================= */
void AdaptiveUiEngine::SetDeviceState(int state, const char* custom_message) {
    if (!initialized_) return;

    if (bar_ota_progress_ && state != 8) {
        lv_obj_add_flag(bar_ota_progress_, LV_OBJ_FLAG_HIDDEN);
    }
    if (label_ota_progress_ && state != 8) {
        lv_obj_add_flag(label_ota_progress_, LV_OBJ_FLAG_HIDDEN);
    }
    if (bar_audio_wave_ && state != 5) {
        lv_obj_add_flag(bar_audio_wave_, LV_OBJ_FLAG_HIDDEN);
        if (label_prompt_) lv_obj_remove_flag(label_prompt_, LV_OBJ_FLAG_HIDDEN);
    }

    switch (state) {
        case 1: // kDeviceStateStarting
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0x89B4FA), 0);
            if (label_status_) lv_label_set_text(label_status_, "Khởi động...");
            if (label_avatar_) lv_label_set_text(label_avatar_, "( o _ o )");
            if (label_prompt_) lv_label_set_text(label_prompt_, "Đang khởi động hệ thống...");
            break;
        case 2: // kDeviceStateWifiConfiguring
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0xFAB387), 0);
            if (label_status_) lv_label_set_text(label_status_, "Cấu hình WiFi");
            if (label_avatar_) lv_label_set_text(label_avatar_, "( ? _ ? )");
            if (label_prompt_) lv_label_set_text(label_prompt_, "Kết nối vào WiFi thiết bị");
            break;
        case 3: // kDeviceStateIdle
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, GetAccentColor(), 0);
            if (label_status_) lv_label_set_text(label_status_, "Sẵn sàng");
            if (label_avatar_) lv_label_set_text(label_avatar_, "( ^ _ ^ )");
            if (label_prompt_) lv_label_set_text(label_prompt_, "Nói 'Xiaozhi' hoặc bấm nút");
            break;
        case 4: // kDeviceStateConnecting
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0xF9E2AF), 0);
            if (label_status_) lv_label_set_text(label_status_, "Đang kết nối...");
            if (label_avatar_) lv_label_set_text(label_avatar_, "( - _ . )");
            if (label_prompt_) lv_label_set_text(label_prompt_, "Kết nối máy chủ AI...");
            break;
        case 5: // kDeviceStateListening
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0xA6E3A1), 0);
            if (label_status_) lv_label_set_text(label_status_, "Đang lắng nghe...");
            if (label_avatar_) lv_label_set_text(label_avatar_, "( O _ O )");
            if (bar_audio_wave_) {
                lv_obj_remove_flag(bar_audio_wave_, LV_OBJ_FLAG_HIDDEN);
                if (label_prompt_) lv_obj_add_flag(label_prompt_, LV_OBJ_FLAG_HIDDEN);
            }
            break;
        case 6: // kDeviceStateSpeaking
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0xCBA6F7), 0);
            if (label_status_) lv_label_set_text(label_status_, "Đang trả lời...");
            if (label_avatar_) lv_label_set_text(label_avatar_, "( ^ o ^ )");
            if (label_prompt_) lv_label_set_text(label_prompt_, "Đang phát âm thanh...");
            break;
        case 8: // kDeviceStateUpgrading (OTA)
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0xFAB387), 0);
            if (label_status_) lv_label_set_text(label_status_, "Cập nhật OTA...");
            if (label_avatar_) lv_label_set_text(label_avatar_, "[ O T A ]");
            if (bar_ota_progress_) lv_obj_remove_flag(bar_ota_progress_, LV_OBJ_FLAG_HIDDEN);
            if (label_ota_progress_) lv_obj_remove_flag(label_ota_progress_, LV_OBJ_FLAG_HIDDEN);
            if (label_prompt_) lv_label_set_text(label_prompt_, "Không ngắt nguồn thiết bị!");
            break;
        case 11: // kDeviceStateFatalError
        default:
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0xF38BA8), 0);
            if (label_status_) lv_label_set_text(label_status_, "Lỗi hệ thống");
            if (label_avatar_) lv_label_set_text(label_avatar_, "( X _ X )");
            if (label_prompt_) lv_label_set_text(label_prompt_, "Kiểm tra kết nối mạng");
            break;
    }

    if (custom_message && custom_message[0] != '\0' && label_status_) {
        lv_label_set_text(label_status_, custom_message);
    }
}

void AdaptiveUiEngine::SetChatMessage(const char* role, const char* content) {
    if (!initialized_) return;

    if (config_.style == UiStyle::ChatBubble && panel_user_bubble_ && label_user_chat_) {
        if (role && strcmp(role, "user") == 0) {
            lv_obj_remove_flag(panel_user_bubble_, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(label_user_chat_, content ? content : "");
            return;
        }
    }

    if (label_chat_role_ && role) {
        if (strcmp(role, "user") == 0) {
            lv_label_set_text(label_chat_role_, "Bạn");
            lv_obj_set_style_text_color(label_chat_role_, lv_color_hex(0xA6E3A1), 0);
        } else if (strcmp(role, "system") == 0) {
            lv_label_set_text(label_chat_role_, "Hệ thống");
            lv_obj_set_style_text_color(label_chat_role_, lv_color_hex(0xFAB387), 0);
        } else {
            lv_label_set_text(label_chat_role_, "Xiaozhi");
            lv_obj_set_style_text_color(label_chat_role_, GetAccentColor(), 0);
        }
    }

    if (label_chat_ && content) {
        lv_label_set_text(label_chat_, content);
    }
}

void AdaptiveUiEngine::ClearChatMessages() {
    if (!initialized_) return;
    if (label_chat_) lv_label_set_text(label_chat_, "");
    if (panel_user_bubble_) lv_obj_add_flag(panel_user_bubble_, LV_OBJ_FLAG_HIDDEN);
}

void AdaptiveUiEngine::SetEmotion(const char* emotion) {
    if (!label_avatar_ || !emotion) return;

    if (strcmp(emotion, "happy") == 0 || strcmp(emotion, "laughing") == 0) {
        lv_label_set_text(label_avatar_, "( ^ _ ^ )");
        lv_obj_set_style_text_color(label_avatar_, lv_color_hex(0xA6E3A1), 0);
    } else if (strcmp(emotion, "speaking") == 0) {
        lv_label_set_text(label_avatar_, "( ^ o ^ )");
        lv_obj_set_style_text_color(label_avatar_, GetAccentColor(), 0);
    } else if (strcmp(emotion, "listening") == 0) {
        lv_label_set_text(label_avatar_, "( O _ O )");
        lv_obj_set_style_text_color(label_avatar_, lv_color_hex(0x89B4FA), 0);
    } else if (strcmp(emotion, "thinking") == 0) {
        lv_label_set_text(label_avatar_, "( - _ . )");
        lv_obj_set_style_text_color(label_avatar_, lv_color_hex(0xF9E2AF), 0);
    } else if (strcmp(emotion, "sad") == 0 || strcmp(emotion, "crying") == 0) {
        lv_label_set_text(label_avatar_, "( T _ T )");
        lv_obj_set_style_text_color(label_avatar_, lv_color_hex(0x89B4FA), 0);
    } else if (strcmp(emotion, "angry") == 0) {
        lv_label_set_text(label_avatar_, "( > _ < )");
        lv_obj_set_style_text_color(label_avatar_, lv_color_hex(0xF38BA8), 0);
    } else if (strcmp(emotion, "sleepy") == 0 || strcmp(emotion, "sleeping") == 0) {
        lv_label_set_text(label_avatar_, "( - _ - ) zZ");
        lv_obj_set_style_text_color(label_avatar_, GetMutedTextColor(), 0);
    } else if (strcmp(emotion, "surprise") == 0 || strcmp(emotion, "shocked") == 0) {
        lv_label_set_text(label_avatar_, "( O _ o !)");
        lv_obj_set_style_text_color(label_avatar_, lv_color_hex(0xFAB387), 0);
    } else {
        lv_label_set_text(label_avatar_, "( ^ _ ^ )");
        lv_obj_set_style_text_color(label_avatar_, GetAccentColor(), 0);
    }
}

void AdaptiveUiEngine::SetStatusBar(const char* wifi_str, const char* battery_str, const char* time_str) {
    if (!initialized_) return;
    if (wifi_str && label_network_) lv_label_set_text(label_network_, wifi_str);
    if (battery_str && label_battery_) lv_label_set_text(label_battery_, battery_str);
    if (time_str && label_time_) lv_label_set_text(label_time_, time_str);
}

void AdaptiveUiEngine::SetWeather(const char* city, const char* temp, const char* desc) {
    if (!initialized_) return;
    if (city && label_weather_city_) lv_label_set_text(label_weather_city_, city);
    if (temp && label_weather_temp_) lv_label_set_text(label_weather_temp_, temp);
    if (desc && label_weather_desc_) lv_label_set_text(label_weather_desc_, desc);
}

void AdaptiveUiEngine::SetAudioLevel(int level_0_to_100) {
    if (!initialized_ || !bar_audio_wave_) return;
    int val = std::max(0, std::min(100, level_0_to_100));
    lv_bar_set_value(bar_audio_wave_, val, LV_ANIM_OFF);
}

void AdaptiveUiEngine::SetOtaProgress(int percent, const char* speed_str) {
    if (!initialized_) return;
    if (bar_ota_progress_) {
        lv_obj_remove_flag(bar_ota_progress_, LV_OBJ_FLAG_HIDDEN);
        lv_bar_set_value(bar_ota_progress_, percent, LV_ANIM_OFF);
    }
    if (label_ota_progress_) {
        lv_obj_remove_flag(label_ota_progress_, LV_OBJ_FLAG_HIDDEN);
        char buf[64];
        if (speed_str && speed_str[0] != '\0') {
            snprintf(buf, sizeof(buf), "%d%% (%s)", percent, speed_str);
        } else {
            snprintf(buf, sizeof(buf), "%d%%", percent);
        }
        lv_label_set_text(label_ota_progress_, buf);
    }
}

void AdaptiveUiEngine::ShowNotification(const char* notification, int duration_ms) {
    if (!initialized_ || !label_notification_) return;
    if (notification && notification[0] != '\0') {
        lv_label_set_text(label_notification_, notification);
        lv_obj_remove_flag(label_notification_, LV_OBJ_FLAG_HIDDEN);
        if (label_time_) lv_obj_add_flag(label_time_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(label_notification_, LV_OBJ_FLAG_HIDDEN);
        if (label_time_) lv_obj_remove_flag(label_time_, LV_OBJ_FLAG_HIDDEN);
    }
}

void AdaptiveUiEngine::ApplyTheme(LvglTheme* theme) {
    if (!initialized_ || theme == nullptr) return;

    auto text_font_wrapper = theme->text_font();
    const lv_font_t* text_font = text_font_wrapper ? text_font_wrapper->font() : nullptr;
    if (text_font != nullptr) {
        if (root_screen_) lv_obj_set_style_text_font(root_screen_, text_font, 0);
        if (label_chat_) lv_obj_set_style_text_font(label_chat_, text_font, 0);
        if (label_user_chat_) lv_obj_set_style_text_font(label_user_chat_, text_font, 0);
        if (label_status_) lv_obj_set_style_text_font(label_status_, text_font, 0);
        if (label_prompt_) lv_obj_set_style_text_font(label_prompt_, text_font, 0);
        if (label_weather_city_) lv_obj_set_style_text_font(label_weather_city_, text_font, 0);
        if (label_weather_desc_) lv_obj_set_style_text_font(label_weather_desc_, text_font, 0);
    }
}
