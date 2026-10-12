#include "adaptive_ui.h"
#include "lvgl_theme.h"
#include <esp_log.h>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <ctime>

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
    if (config_.accent == ColorAccent::Monochrome) {
        return lv_color_hex(0x000000);
    }
    switch (config_.style) {
        case UiStyle::CyberTerminal:
            return lv_color_hex(0x050A0E);
        case UiStyle::MinimalZen:
            return lv_color_hex(0x000000);
        case UiStyle::ChatBubble:
            return lv_color_hex(0x111116);
        case UiStyle::ClassicAvatar:
            return lv_color_hex(0x0D1117);
        case UiStyle::SmartDashboard:
        default:
            return lv_color_hex(0x11111B);
    }
}

lv_color_t AdaptiveUiEngine::GetCardBgColor() const {
    if (config_.accent == ColorAccent::Monochrome) {
        return lv_color_hex(0x111111);
    }
    switch (config_.style) {
        case UiStyle::CyberTerminal:
            return lv_color_hex(0x0A141E);
        case UiStyle::MinimalZen:
            return lv_color_hex(0x0A0A0A);
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
    if (config_.accent == ColorAccent::Monochrome) {
        return lv_color_hex(0x333333);
    }
    switch (config_.style) {
        case UiStyle::CyberTerminal:
            return lv_color_hex(0x00F0FF);
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
    if (config_.accent == ColorAccent::Monochrome) {
        return lv_color_hex(0xFFFFFF);
    }
    switch (config_.style) {
        case UiStyle::CyberTerminal:
            return lv_color_hex(0x00FF9D);
        case UiStyle::MinimalZen:
            return lv_color_hex(0xE0E0E0);
        default:
            return lv_color_hex(0xCDD6F4);
    }
}

lv_color_t AdaptiveUiEngine::GetMutedTextColor() const {
    if (config_.accent == ColorAccent::Monochrome) {
        return lv_color_hex(0x888888);
    }
    switch (config_.style) {
        case UiStyle::CyberTerminal:
            return lv_color_hex(0x005544);
        case UiStyle::MinimalZen:
            return lv_color_hex(0x777777);
        default:
            return lv_color_hex(0x6C7086);
    }
}

namespace {

constexpr int kAvatarMinSize = 40;

// Plain container: drops the default LVGL theme styles (padding, border, background,
// scrollbars) so children are placed exactly where the layout puts them.
lv_obj_t* CreateBox(lv_obj_t* parent) {
    lv_obj_t* obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

void StyleCard(lv_obj_t* obj, lv_color_t bg, lv_color_t border, int radius, int border_width) {
    lv_obj_set_style_bg_color(obj, bg, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_border_width(obj, border_width, 0);
    lv_obj_set_style_border_color(obj, border, 0);
}

// Single-line label that scrolls instead of overflowing into its neighbours.
lv_obj_t* CreateLineLabel(lv_obj_t* parent, int width, lv_color_t color, const char* text) {
    lv_obj_t* label = lv_label_create(parent);
    lv_obj_set_width(label, width);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_label_set_text(label, text);
    return label;
}

lv_obj_t* CreateWrapLabel(lv_obj_t* parent, lv_color_t color, const char* text) {
    lv_obj_t* label = lv_label_create(parent);
    lv_obj_set_width(label, lv_pct(100));
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(label, color, 0);
    lv_label_set_text(label, text);
    return label;
}

// Vertical scroll area whose children stack top to bottom.
lv_obj_t* CreateScrollColumn(lv_obj_t* parent, int pad, int gap) {
    lv_obj_t* area = CreateBox(parent);
    lv_obj_add_flag(area, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(area, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(area, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(area, pad, 0);
    lv_obj_set_flex_flow(area, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(area, gap, 0);
    return area;
}

lv_obj_t* CreateWaveBar(lv_obj_t* parent, int width, lv_color_t track, lv_color_t indicator) {
    lv_obj_t* bar = lv_bar_create(parent);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    lv_obj_set_size(bar, width, 4);
    lv_obj_align(bar, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(bar, track, 0);
    lv_obj_set_style_bg_color(bar, indicator, LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 2, 0);
    lv_obj_set_style_radius(bar, 2, LV_PART_INDICATOR);
    lv_obj_add_flag(bar, LV_OBJ_FLAG_HIDDEN);
    return bar;
}

}  // namespace

void AdaptiveUiEngine::Destroy() {
    if (clock_timer_ != nullptr) {
        lv_timer_delete(clock_timer_);
        clock_timer_ = nullptr;
    }
    if (container_root_ != nullptr) {
        lv_obj_delete(container_root_);
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
    chat_scroll_ = nullptr;
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
    line_h_ = config_.text_font != nullptr ? config_.text_font->line_height : 20;
    icon_h_ = config_.icon_font != nullptr ? config_.icon_font->line_height : line_h_;
    CalculateMetrics(width, height);

    root_screen_ = parent_screen ? parent_screen : lv_screen_active();
    if (!root_screen_) {
        ESP_LOGE(TAG, "Root screen is nullptr!");
        return;
    }

    // Full-screen background layer, then a content layer inset by the round-screen safe area.
    // Builders lay out inside the content layer, so every style gets the same safe margin.
    container_root_ = CreateBox(root_screen_);
    lv_obj_set_size(container_root_, metrics_.width, metrics_.height);
    lv_obj_align(container_root_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(container_root_, GetBgColor(), 0);
    lv_obj_set_style_bg_opa(container_root_, LV_OPA_COVER, 0);

    lv_obj_t* content_root = CreateBox(container_root_);
    lv_obj_set_size(content_root, metrics_.width - 2 * metrics_.safe_inset,
                    metrics_.height - 2 * metrics_.safe_inset);
    lv_obj_align(content_root, LV_ALIGN_CENTER, 0, 0);

    switch (config_.style) {
        case UiStyle::CyberTerminal:
            BuildCyberTerminal(content_root);
            break;
        case UiStyle::ChatBubble:
            BuildChatBubble(content_root);
            break;
        case UiStyle::ClassicAvatar:
            BuildClassicAvatar(content_root);
            break;
        case UiStyle::MinimalZen:
            BuildMinimalZen(content_root);
            break;
        case UiStyle::SmartDashboard:
        default:
            BuildSmartDashboard(content_root);
            break;
    }

    if (label_time_ != nullptr) {
        UpdateClock();
        clock_timer_ = lv_timer_create(ClockTimerCallback, 1000, this);
    }

    initialized_ = true;
    ESP_LOGI(TAG, "Adaptive UI initialized with style %d (line height %d)", (int)config_.style,
             line_h_);
}

bool AdaptiveUiEngine::NeedsRelayout(LvglTheme* theme) const {
    if (!initialized_ || theme == nullptr) return false;
    auto text_font = theme->text_font();
    auto icon_font = theme->icon_font();
    int text_h = (text_font && text_font->font()) ? text_font->font()->line_height : line_h_;
    int icon_h = (icon_font && icon_font->font()) ? icon_font->font()->line_height : icon_h_;
    return text_h != line_h_ || icon_h != icon_h_;
}

void AdaptiveUiEngine::ClockTimerCallback(lv_timer_t* timer) {
    static_cast<AdaptiveUiEngine*>(lv_timer_get_user_data(timer))->UpdateClock();
}

void AdaptiveUiEngine::UpdateClock() {
    if (label_time_ == nullptr) return;
    time_t now = time(nullptr);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    char buf[16];
    if (tm_now.tm_year < 2025 - 1900) {
        snprintf(buf, sizeof(buf), "%s",
                 config_.style == UiStyle::CyberTerminal ? "[--:--]" : "--:--");
    } else {
        strftime(buf, sizeof(buf),
                 config_.style == UiStyle::CyberTerminal ? "[%H:%M:%S]" : "%H:%M", &tm_now);
    }
    if (strcmp(lv_label_get_text(label_time_), buf) != 0) {
        lv_label_set_text(label_time_, buf);
    }
}

// Status and notification share one slot: LvglDisplay shows one and hides the other,
// so they must sit on top of each other instead of on top of other widgets.
void AdaptiveUiEngine::BuildStatusPill(lv_obj_t* parent, int y, int height, lv_color_t bg,
                                       lv_color_t border, int radius, lv_color_t dot_color,
                                       lv_color_t text_color) {
    int w = metrics_.content_width;
    panel_state_pill_ = CreateBox(parent);
    lv_obj_set_size(panel_state_pill_, w, height);
    lv_obj_align(panel_state_pill_, LV_ALIGN_TOP_MID, 0, y);
    StyleCard(panel_state_pill_, bg, border, radius, 1);

    int dot_size = height > 18 ? 8 : 6;
    led_state_dot_ = CreateBox(panel_state_pill_);
    lv_obj_set_size(led_state_dot_, dot_size, dot_size);
    lv_obj_align(led_state_dot_, LV_ALIGN_LEFT_MID, 8, 0);
    StyleCard(led_state_dot_, dot_color, dot_color, dot_size / 2, 0);

    int label_w = w - 2 * (8 + dot_size + 4);
    label_status_ = CreateLineLabel(panel_state_pill_, label_w, text_color, "Sẵn sàng");
    lv_obj_align(label_status_, LV_ALIGN_CENTER, 0, 0);

    label_notification_ = CreateLineLabel(panel_state_pill_, label_w, lv_color_hex(0xF9E2AF), "");
    lv_obj_align(label_notification_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(label_notification_, LV_OBJ_FLAG_HIDDEN);
}

// Smallest chat card that still shows the role caption plus one line of text.
int AdaptiveUiEngine::ChatMinHeight() const { return 2 * line_h_ + 16; }

void AdaptiveUiEngine::BuildAvatarPanel(lv_obj_t* parent, int x, int y, int width, int height,
                                        lv_color_t bg, lv_color_t border, int radius,
                                        lv_color_t progress_color) {
    panel_avatar_ = CreateBox(parent);
    lv_obj_set_size(panel_avatar_, width, height);
    lv_obj_align(panel_avatar_, LV_ALIGN_TOP_LEFT, metrics_.pad_x + x, y);
    StyleCard(panel_avatar_, bg, border, radius, 1);

    bar_ota_progress_ = lv_bar_create(panel_avatar_);
    lv_bar_set_range(bar_ota_progress_, 0, 100);
    lv_bar_set_value(bar_ota_progress_, 0, LV_ANIM_OFF);
    lv_obj_set_size(bar_ota_progress_, width * 80 / 100, 4);
    lv_obj_align(bar_ota_progress_, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_set_style_bg_color(bar_ota_progress_, lv_color_hex(0x313244), 0);
    lv_obj_set_style_bg_color(bar_ota_progress_, progress_color, LV_PART_INDICATOR);
    lv_obj_add_flag(bar_ota_progress_, LV_OBJ_FLAG_HIDDEN);
}

// Scrollable chat card: a role caption followed by the wrapped message text.
void AdaptiveUiEngine::BuildChatCard(lv_obj_t* parent, int x, int y, int width, int height,
                                     lv_color_t bg, lv_color_t border, int radius,
                                     const char* role, lv_color_t role_color,
                                     const char* greeting, lv_color_t text_color) {
    panel_chat_ = CreateScrollColumn(parent, 6, 2);
    lv_obj_set_size(panel_chat_, width, height);
    lv_obj_align(panel_chat_, LV_ALIGN_TOP_LEFT, metrics_.pad_x + x, y);
    StyleCard(panel_chat_, bg, border, radius, 1);
    chat_scroll_ = panel_chat_;

    label_chat_role_ = CreateWrapLabel(panel_chat_, role_color, role);
    if (height < ChatMinHeight()) {
        // Too short for a caption: give the single visible line to the message.
        lv_obj_add_flag(label_chat_role_, LV_OBJ_FLAG_HIDDEN);
    }
    label_chat_ = CreateWrapLabel(panel_chat_, text_color, greeting);
}

void AdaptiveUiEngine::BuildBottomBar(lv_obj_t* parent, int height, const char* prompt,
                                      lv_color_t prompt_color, lv_color_t wave_track,
                                      lv_color_t wave_color) {
    const int w = metrics_.content_width;
    panel_bottom_bar_ = CreateBox(parent);
    lv_obj_set_size(panel_bottom_bar_, w, height);
    lv_obj_align(panel_bottom_bar_, LV_ALIGN_BOTTOM_MID, 0, -metrics_.pad_y);

    label_prompt_ = CreateLineLabel(panel_bottom_bar_, w - 16, prompt_color, prompt);
    lv_obj_align(label_prompt_, LV_ALIGN_CENTER, 0, 0);

    if (config_.voice_wave_enabled) {
        bar_audio_wave_ = CreateWaveBar(panel_bottom_bar_, w * 75 / 100, wave_track, wave_color);
    }
}

// Layout rule shared by the stacked styles: the status row and the chat always stay; the
// prompt bar, weather card and avatar are only added while the chat keeps ChatMinHeight().
// Landscape screens put the avatar beside the chat instead of above it.
void AdaptiveUiEngine::BuildSmartDashboard(lv_obj_t* parent) {
    const int w = metrics_.content_width;
    const int inner_h = metrics_.height - 2 * metrics_.safe_inset;
    const int top_bar_h = std::max(line_h_, icon_h_) + 4;
    const int pill_h = line_h_ + 8;
    const int bottom_bar_h = line_h_ + 6;
    const int weather_h = 2 * line_h_ + 10;
    const int gap = metrics_.is_mini ? 2 : 4;
    const int chat_min = ChatMinHeight();

    int y = metrics_.pad_y;
    panel_top_bar_ = CreateBox(parent);
    lv_obj_set_size(panel_top_bar_, w, top_bar_h);
    lv_obj_align(panel_top_bar_, LV_ALIGN_TOP_MID, 0, y);

    label_network_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_network_, "");
    lv_obj_align(label_network_, LV_ALIGN_LEFT_MID, 2, 0);
    lv_obj_set_style_text_color(label_network_, GetAccentColor(), 0);

    label_time_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_time_, "--:--");
    lv_obj_align(label_time_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_time_, GetTextColor(), 0);

    label_battery_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_battery_, "");
    lv_obj_align(label_battery_, LV_ALIGN_RIGHT_MID, -2, 0);
    lv_obj_set_style_text_color(label_battery_, lv_color_hex(0xA6E3A1), 0);
    y += top_bar_h + gap;

    BuildStatusPill(parent, y, pill_h, GetCardBgColor(), GetBorderColor(), pill_h / 2,
                    GetAccentColor(), GetTextColor());
    y += pill_h + gap;

    int avail = inner_h - y - metrics_.pad_y;
    const bool show_bottom = avail >= chat_min + bottom_bar_h + gap;
    if (show_bottom) avail -= bottom_bar_h + gap;

    const bool side_by_side = metrics_.is_landscape;
    const int avatar_reserve = side_by_side ? 0 : kAvatarMinSize + gap;
    if (config_.weather_enabled && avail >= chat_min + avatar_reserve + weather_h + gap) {
        panel_weather_ = CreateBox(parent);
        lv_obj_set_size(panel_weather_, w, weather_h);
        lv_obj_align(panel_weather_, LV_ALIGN_TOP_MID, 0, y);
        StyleCard(panel_weather_, GetCardBgColor(), GetBorderColor(), metrics_.corner_radius, 1);

        label_weather_city_ = lv_label_create(panel_weather_);
        lv_label_set_text(label_weather_city_, config_.weather_city.c_str());
        lv_obj_align(label_weather_city_, LV_ALIGN_TOP_LEFT, 8, 4);
        lv_obj_set_style_text_color(label_weather_city_, GetAccentColor(), 0);

        label_weather_temp_ = lv_label_create(panel_weather_);
        lv_label_set_text(label_weather_temp_, "--");
        lv_obj_align(label_weather_temp_, LV_ALIGN_TOP_RIGHT, -8, 4);
        lv_obj_set_style_text_color(label_weather_temp_, lv_color_hex(0xF9E2AF), 0);

        label_weather_desc_ = lv_label_create(panel_weather_);
        lv_label_set_text(label_weather_desc_, "Chưa có dữ liệu");
        lv_obj_align(label_weather_desc_, LV_ALIGN_BOTTOM_LEFT, 8, -4);
        lv_obj_set_style_text_color(label_weather_desc_, GetMutedTextColor(), 0);
        y += weather_h + gap;
        avail -= weather_h + gap;
    }

    int chat_x = 0;
    int chat_w = w;
    if (side_by_side) {
        int avatar_w = std::clamp(std::min(avail, w * 40 / 100), kAvatarMinSize, 160);
        if (w - avatar_w - gap >= w / 2) {
            BuildAvatarPanel(parent, 0, y, avatar_w, avail, GetCardBgColor(), GetBorderColor(),
                             metrics_.corner_radius, lv_color_hex(0xA6E3A1));
            chat_x = avatar_w + gap;
            chat_w = w - chat_x;
        }
    } else {
        int avatar_h = std::min(std::clamp(avail * 45 / 100, kAvatarMinSize, 140),
                                avail - gap - chat_min);
        if (avatar_h >= kAvatarMinSize) {
            BuildAvatarPanel(parent, 0, y, w, avatar_h, GetCardBgColor(), GetBorderColor(),
                             metrics_.corner_radius, lv_color_hex(0xA6E3A1));
            y += avatar_h + gap;
            avail -= avatar_h + gap;
        }
    }

    BuildChatCard(parent, chat_x, y, chat_w, std::max(avail, line_h_ + 14), GetCardBgColor(),
                  GetBorderColor(), metrics_.corner_radius, "Xiaozhi", lv_color_hex(0xF9E2AF),
                  "Xin chào! Tôi là trợ lý AI Xiaozhi.", GetTextColor());

    if (show_bottom) {
        BuildBottomBar(parent, bottom_bar_h, "Nói 'Xiaozhi' hoặc bấm nút", GetMutedTextColor(),
                       lv_color_hex(0x313244), GetAccentColor());
    }
}

void AdaptiveUiEngine::BuildCyberTerminal(lv_obj_t* parent) {
    const int w = metrics_.content_width;
    const int inner_h = metrics_.height - 2 * metrics_.safe_inset;
    const int header_h = std::max(line_h_, icon_h_) + 6;
    const int pill_h = line_h_ + 6;
    const int bottom_bar_h = line_h_ + 4;
    const int gap = 4;
    const int chat_min = ChatMinHeight();
    const lv_color_t cyan = lv_color_hex(0x00F0FF);
    const lv_color_t green = lv_color_hex(0x00FF66);
    const lv_color_t panel_bg = lv_color_hex(0x081624);

    int y = metrics_.pad_y;
    panel_top_bar_ = CreateBox(parent);
    lv_obj_set_size(panel_top_bar_, w, header_h);
    lv_obj_align(panel_top_bar_, LV_ALIGN_TOP_MID, 0, y);
    StyleCard(panel_top_bar_, lv_color_hex(0x07111A), cyan, 2, 1);

    label_network_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_network_, "");
    lv_obj_align(label_network_, LV_ALIGN_LEFT_MID, 4, 0);
    lv_obj_set_style_text_color(label_network_, cyan, 0);

    label_time_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_time_, "[--:--]");
    lv_obj_align(label_time_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_time_, green, 0);

    label_battery_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_battery_, "");
    lv_obj_align(label_battery_, LV_ALIGN_RIGHT_MID, -4, 0);
    lv_obj_set_style_text_color(label_battery_, lv_color_hex(0xFFB800), 0);
    y += header_h + gap;

    int avail = inner_h - y - metrics_.pad_y - (pill_h + gap);
    const bool show_bottom = avail >= chat_min + bottom_bar_h + gap;
    if (show_bottom) avail -= bottom_bar_h + gap;

    int chat_x = 0;
    int chat_w = w;
    if (metrics_.is_landscape) {
        BuildStatusPill(parent, y, pill_h, lv_color_hex(0x07111A), cyan, 2, green,
                        lv_color_hex(0xCDD6F4));
        y += pill_h + gap;
        int core_w = std::clamp(std::min(avail, w * 40 / 100), kAvatarMinSize, 160);
        if (w - core_w - gap >= w / 2) {
            BuildAvatarPanel(parent, 0, y, core_w, avail, panel_bg, cyan, 4, green);
            chat_x = core_w + gap;
            chat_w = w - chat_x;
        }
    } else {
        int core_h = std::min(std::clamp(avail * 40 / 100, kAvatarMinSize, 140),
                              avail - gap - chat_min);
        if (core_h >= kAvatarMinSize) {
            BuildAvatarPanel(parent, 0, y, w, core_h, panel_bg, cyan, 4, green);
            y += core_h + gap;
            avail -= core_h + gap;
        }
        BuildStatusPill(parent, y, pill_h, lv_color_hex(0x07111A), cyan, 2, green,
                        lv_color_hex(0xCDD6F4));
        y += pill_h + gap;
    }

    BuildChatCard(parent, chat_x, y, chat_w, std::max(avail, line_h_ + 14),
                  lv_color_hex(0x050D14), cyan, 2, "> STREAM LINKED:", cyan,
                  "System ready. Awaiting voice input...", green);

    if (show_bottom) {
        BuildBottomBar(parent, bottom_bar_h, "[LISTEN_PORT: ACTIVE]", cyan,
                       lv_color_hex(0x003344), cyan);
    }
}

void AdaptiveUiEngine::BuildChatBubble(lv_obj_t* parent) {
    const int w = metrics_.content_width;
    const int inner_h = metrics_.height - 2 * metrics_.safe_inset;
    const int header_h = std::max(line_h_, icon_h_) + 4;
    const int pill_h = line_h_ + 6;
    const int bottom_bar_h = line_h_ + 10;
    const int gap = 4;
    const int bubble_min = 2 * line_h_ + 18;  // bubble padding + caption + one line
    const int bubble_w = metrics_.is_landscape ? w * 70 / 100 : w * 85 / 100;

    int y = metrics_.pad_y;
    panel_top_bar_ = CreateBox(parent);
    lv_obj_set_size(panel_top_bar_, w, header_h);
    lv_obj_align(panel_top_bar_, LV_ALIGN_TOP_MID, 0, y);

    label_network_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_network_, "");
    lv_obj_align(label_network_, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_color(label_network_, lv_color_hex(0x89DCEB), 0);

    label_time_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_time_, "--:--");
    lv_obj_align(label_time_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_time_, GetTextColor(), 0);

    label_battery_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_battery_, "");
    lv_obj_align(label_battery_, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_text_color(label_battery_, lv_color_hex(0xA6E3A1), 0);
    y += header_h + gap;

    BuildStatusPill(parent, y, pill_h, lv_color_hex(0x181824), lv_color_hex(0x181824),
                    pill_h / 2, lv_color_hex(0x89DCEB), lv_color_hex(0xCDD6F4));
    y += pill_h + gap;

    int avail = inner_h - y - metrics_.pad_y;
    const bool show_bottom = avail >= bubble_min + bottom_bar_h + gap;
    if (show_bottom) avail -= bottom_bar_h + gap;

    // Conversation column: the user's bubble (right) and Xiaozhi's reply (left) stack
    // in order and scroll together, so a long reply can never cover the other bubble.
    chat_scroll_ = CreateScrollColumn(parent, 0, 6);
    lv_obj_set_size(chat_scroll_, w, std::max(avail, line_h_ + 16));
    lv_obj_align(chat_scroll_, LV_ALIGN_TOP_MID, 0, y);

    panel_user_bubble_ = CreateBox(chat_scroll_);
    lv_obj_set_size(panel_user_bubble_, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(panel_user_bubble_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(panel_user_bubble_, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START);
    lv_obj_add_flag(panel_user_bubble_, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t* user_bubble = CreateBox(panel_user_bubble_);
    lv_obj_set_size(user_bubble, bubble_w, LV_SIZE_CONTENT);
    StyleCard(user_bubble, lv_color_hex(0x07C160), lv_color_hex(0x07C160), 12, 0);
    lv_obj_set_style_pad_all(user_bubble, 8, 0);
    label_user_chat_ = CreateWrapLabel(user_bubble, lv_color_hex(0xFFFFFF), "");

    panel_chat_ = CreateBox(chat_scroll_);
    lv_obj_set_size(panel_chat_, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(panel_chat_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(panel_chat_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START);

    lv_obj_t* ai_bubble = CreateBox(panel_chat_);
    lv_obj_set_size(ai_bubble, bubble_w, LV_SIZE_CONTENT);
    StyleCard(ai_bubble, lv_color_hex(0x252536), lv_color_hex(0x252536), 12, 0);
    lv_obj_set_style_pad_all(ai_bubble, 8, 0);
    lv_obj_set_flex_flow(ai_bubble, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(ai_bubble, 2, 0);
    label_chat_role_ = CreateWrapLabel(ai_bubble, lv_color_hex(0x89DCEB), "Xiaozhi");
    if (avail < bubble_min) {
        lv_obj_add_flag(label_chat_role_, LV_OBJ_FLAG_HIDDEN);
    }
    label_chat_ = CreateWrapLabel(ai_bubble, lv_color_hex(0xFFFFFF),
                                  "Xin chào! Bạn muốn trò chuyện về chủ đề gì?");

    if (show_bottom) {
        BuildBottomBar(parent, bottom_bar_h, "Giữ hoặc nói 'Xiaozhi'...", GetMutedTextColor(),
                       lv_color_hex(0x313244), lv_color_hex(0x07C160));
        StyleCard(panel_bottom_bar_, lv_color_hex(0x1F1F2E), lv_color_hex(0x313244),
                  bottom_bar_h / 2, 1);
    }
}

void AdaptiveUiEngine::BuildClassicAvatar(lv_obj_t* parent) {
    const int w = metrics_.content_width;
    const int inner_h = metrics_.height - 2 * metrics_.safe_inset;
    const int top_bar_h = std::max(line_h_, icon_h_) + 4;
    const int chat_h = line_h_ + 10;

    panel_top_bar_ = CreateBox(parent);
    lv_obj_set_size(panel_top_bar_, w, top_bar_h);
    lv_obj_align(panel_top_bar_, LV_ALIGN_TOP_MID, 0, metrics_.pad_y);

    label_network_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_network_, "");
    lv_obj_align(label_network_, LV_ALIGN_LEFT_MID, 2, 0);
    lv_obj_set_style_text_color(label_network_, GetAccentColor(), 0);

    label_battery_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_battery_, "");
    lv_obj_align(label_battery_, LV_ALIGN_RIGHT_MID, -2, 0);
    lv_obj_set_style_text_color(label_battery_, lv_color_hex(0xA6E3A1), 0);

    // Status gets its own slot between the icons; it used to share the chat label,
    // so every status update wiped the conversation text.
    int status_w = w - 2 * (icon_h_ + 8);
    label_status_ = CreateLineLabel(panel_top_bar_, status_w, GetTextColor(), "Sẵn sàng");
    lv_obj_align(label_status_, LV_ALIGN_CENTER, 0, 0);
    label_notification_ = CreateLineLabel(panel_top_bar_, status_w, lv_color_hex(0xF9E2AF), "");
    lv_obj_align(label_notification_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(label_notification_, LV_OBJ_FLAG_HIDDEN);

    int top = metrics_.pad_y + top_bar_h + 6;
    int bottom = inner_h - metrics_.pad_y - chat_h - 6;
    int face_size = std::max(std::min(w, bottom - top), 40);
    panel_avatar_ = CreateBox(parent);
    lv_obj_set_size(panel_avatar_, face_size, face_size);
    lv_obj_align(panel_avatar_, LV_ALIGN_TOP_MID, 0, top + std::max(0, (bottom - top - face_size) / 2));
    StyleCard(panel_avatar_, lv_color_hex(0x161B22), GetAccentColor(), face_size / 2, 2);

    bar_ota_progress_ = lv_bar_create(panel_avatar_);
    lv_bar_set_range(bar_ota_progress_, 0, 100);
    lv_obj_set_size(bar_ota_progress_, face_size * 60 / 100, 6);
    lv_obj_align(bar_ota_progress_, LV_ALIGN_BOTTOM_MID, 0, -face_size / 8);
    lv_obj_set_style_bg_color(bar_ota_progress_, lv_color_hex(0xA6E3A1), LV_PART_INDICATOR);
    lv_obj_add_flag(bar_ota_progress_, LV_OBJ_FLAG_HIDDEN);

    panel_chat_ = CreateBox(parent);
    lv_obj_set_size(panel_chat_, w, chat_h);
    lv_obj_align(panel_chat_, LV_ALIGN_BOTTOM_MID, 0, -metrics_.pad_y);
    StyleCard(panel_chat_, lv_color_hex(0x1F242C), lv_color_hex(0x30363D), chat_h / 2, 1);

    label_chat_ = CreateLineLabel(panel_chat_, w - 24, GetTextColor(), "Xiaozhi sẵn sàng!");
    lv_obj_align(label_chat_, LV_ALIGN_CENTER, 0, 0);
}

void AdaptiveUiEngine::BuildMinimalZen(lv_obj_t* parent) {
    const int w = metrics_.content_width;
    const int inner_h = metrics_.height - 2 * metrics_.safe_inset;
    const int top_bar_h = std::max(line_h_, icon_h_) + 4;

    panel_top_bar_ = CreateBox(parent);
    lv_obj_set_size(panel_top_bar_, w, top_bar_h);
    lv_obj_align(panel_top_bar_, LV_ALIGN_TOP_MID, 0, metrics_.pad_y);

    label_network_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_network_, "");
    lv_obj_align(label_network_, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_color(label_network_, lv_color_hex(0xFFFFFF), 0);

    label_battery_ = lv_label_create(panel_top_bar_);
    lv_label_set_text(label_battery_, "");
    lv_obj_align(label_battery_, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_text_color(label_battery_, lv_color_hex(0xFFFFFF), 0);

    // Clock, divider and status form one block that sits just above the middle of the
    // screen; the chat text gets everything below it. The clock is the first thing
    // dropped when the screen is too short for the block plus one line of chat.
    const int top = metrics_.pad_y + top_bar_h + 4;
    const int chat_min = line_h_ + 4;
    bool show_time = true;
    int block_h = line_h_ + 4 + 1 + 4 + line_h_;
    int block_y = std::max(top, inner_h / 2 - block_h);
    if (inner_h - metrics_.pad_y - (block_y + block_h + 8) < chat_min) {
        show_time = false;
        block_h = 1 + 4 + line_h_;
        block_y = std::max(top, inner_h / 2 - block_h);
    }

    int y = block_y;
    if (show_time) {
        label_time_ = lv_label_create(parent);
        lv_label_set_text(label_time_, "--:--");
        lv_obj_align(label_time_, LV_ALIGN_TOP_MID, 0, y);
        lv_obj_set_style_text_color(label_time_, lv_color_hex(0xFFFFFF), 0);
        y += line_h_ + 4;
    }

    lv_obj_t* divider = CreateBox(parent);
    lv_obj_set_size(divider, w * 80 / 100, 1);
    lv_obj_align(divider, LV_ALIGN_TOP_MID, 0, y);
    StyleCard(divider, lv_color_hex(0x666666), lv_color_hex(0x666666), 0, 0);
    y += 1 + 4;

    label_status_ = CreateLineLabel(parent, w, lv_color_hex(0xAAAAAA), "SẴN SÀNG");
    lv_obj_align(label_status_, LV_ALIGN_TOP_MID, 0, y);
    label_notification_ = CreateLineLabel(parent, w, lv_color_hex(0xFFFFFF), "");
    lv_obj_align(label_notification_, LV_ALIGN_TOP_MID, 0, y);
    lv_obj_add_flag(label_notification_, LV_OBJ_FLAG_HIDDEN);
    y += line_h_ + 8;

    int chat_h = std::max(inner_h - y - metrics_.pad_y, chat_min);
    panel_chat_ = CreateScrollColumn(parent, 0, 0);
    lv_obj_set_size(panel_chat_, w, chat_h);
    lv_obj_align(panel_chat_, LV_ALIGN_TOP_MID, 0, y);
    chat_scroll_ = panel_chat_;

    label_chat_ = CreateWrapLabel(panel_chat_, lv_color_hex(0xFFFFFF), "");
    lv_obj_set_style_text_align(label_chat_, LV_TEXT_ALIGN_CENTER, 0);
}

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
        case 1:
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0x89B4FA), 0);
            if (label_status_) lv_label_set_text(label_status_, "Khởi động...");
            if (label_avatar_) lv_label_set_text(label_avatar_, "( o _ o )");
            if (label_prompt_) lv_label_set_text(label_prompt_, "Đang khởi động hệ thống...");
            break;
        case 2:
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0xFAB387), 0);
            if (label_status_) lv_label_set_text(label_status_, "Cấu hình WiFi");
            if (label_avatar_) lv_label_set_text(label_avatar_, "( ? _ ? )");
            if (label_prompt_) lv_label_set_text(label_prompt_, "Kết nối vào WiFi thiết bị");
            break;
        case 3:
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, GetAccentColor(), 0);
            if (label_status_) lv_label_set_text(label_status_, "Sẵn sàng");
            if (label_avatar_) lv_label_set_text(label_avatar_, "( ^ _ ^ )");
            if (label_prompt_) lv_label_set_text(label_prompt_, "Nói 'Xiaozhi' hoặc bấm nút");
            break;
        case 4:
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0xF9E2AF), 0);
            if (label_status_) lv_label_set_text(label_status_, "Đang kết nối...");
            if (label_avatar_) lv_label_set_text(label_avatar_, "( - _ . )");
            if (label_prompt_) lv_label_set_text(label_prompt_, "Kết nối máy chủ AI...");
            break;
        case 5:
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0xA6E3A1), 0);
            if (label_status_) lv_label_set_text(label_status_, "Đang lắng nghe...");
            if (label_avatar_) lv_label_set_text(label_avatar_, "( O _ O )");
            if (bar_audio_wave_) {
                lv_obj_remove_flag(bar_audio_wave_, LV_OBJ_FLAG_HIDDEN);
                if (label_prompt_) lv_obj_add_flag(label_prompt_, LV_OBJ_FLAG_HIDDEN);
            }
            break;
        case 6:
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0xCBA6F7), 0);
            if (label_status_) lv_label_set_text(label_status_, "Đang trả lời...");
            if (label_avatar_) lv_label_set_text(label_avatar_, "( ^ o ^ )");
            if (label_prompt_) lv_label_set_text(label_prompt_, "Đang phát âm thanh...");
            break;
        case 8:
            if (led_state_dot_) lv_obj_set_style_bg_color(led_state_dot_, lv_color_hex(0xFAB387), 0);
            if (label_status_) lv_label_set_text(label_status_, "Cập nhật OTA...");
            if (label_avatar_) lv_label_set_text(label_avatar_, "[ O T A ]");
            if (bar_ota_progress_) lv_obj_remove_flag(bar_ota_progress_, LV_OBJ_FLAG_HIDDEN);
            if (label_ota_progress_) lv_obj_remove_flag(label_ota_progress_, LV_OBJ_FLAG_HIDDEN);
            if (label_prompt_) lv_label_set_text(label_prompt_, "Không ngắt nguồn thiết bị!");
            break;
        case 11:
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

void AdaptiveUiEngine::ScrollChatToEnd() {
    if (chat_scroll_ == nullptr) return;
    lv_obj_update_layout(chat_scroll_);
    lv_obj_scroll_to_y(chat_scroll_, LV_COORD_MAX, LV_ANIM_OFF);
}

void AdaptiveUiEngine::SetChatMessage(const char* role, const char* content) {
    if (!initialized_) return;
    const char* text = content ? content : "";

    if (config_.style == UiStyle::ChatBubble) {
        if (role && strcmp(role, "user") == 0) {
            // A new question starts a new exchange: show it and wait for the reply.
            lv_label_set_text(label_user_chat_, text);
            if (text[0] != '\0') {
                lv_obj_remove_flag(panel_user_bubble_, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(panel_user_bubble_, LV_OBJ_FLAG_HIDDEN);
            }
            lv_label_set_text(label_chat_, "");
            lv_obj_add_flag(panel_chat_, LV_OBJ_FLAG_HIDDEN);
            ScrollChatToEnd();
            return;
        }
        if (text[0] == '\0') {
            lv_label_set_text(label_chat_, "");
            lv_obj_add_flag(panel_chat_, LV_OBJ_FLAG_HIDDEN);
            return;
        }
        lv_obj_remove_flag(panel_chat_, LV_OBJ_FLAG_HIDDEN);
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

    if (label_chat_) {
        lv_label_set_text(label_chat_, text);
    }
    ScrollChatToEnd();
}

void AdaptiveUiEngine::ClearChatMessages() {
    if (!initialized_) return;
    if (label_chat_) lv_label_set_text(label_chat_, "");
    if (label_user_chat_) lv_label_set_text(label_user_chat_, "");
    if (panel_user_bubble_) lv_obj_add_flag(panel_user_bubble_, LV_OBJ_FLAG_HIDDEN);
    if (config_.style == UiStyle::ChatBubble && panel_chat_) {
        lv_obj_add_flag(panel_chat_, LV_OBJ_FLAG_HIDDEN);
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
        if (label_time_) lv_obj_set_style_text_font(label_time_, text_font, 0);
        if (label_notification_) lv_obj_set_style_text_font(label_notification_, text_font, 0);
        if (label_chat_) lv_obj_set_style_text_font(label_chat_, text_font, 0);
        if (label_chat_role_) lv_obj_set_style_text_font(label_chat_role_, text_font, 0);
        if (label_user_chat_) lv_obj_set_style_text_font(label_user_chat_, text_font, 0);
        if (label_status_) lv_obj_set_style_text_font(label_status_, text_font, 0);
        if (label_prompt_) lv_obj_set_style_text_font(label_prompt_, text_font, 0);
        if (label_weather_city_) lv_obj_set_style_text_font(label_weather_city_, text_font, 0);
        if (label_weather_desc_) lv_obj_set_style_text_font(label_weather_desc_, text_font, 0);
    }

    // network/battery labels receive Material icon glyphs from LvglDisplay::UpdateStatusBar,
    // so they must use the icon font; a text font renders those glyphs as broken boxes.
    auto icon_font_wrapper = theme->icon_font();
    const lv_font_t* icon_font = icon_font_wrapper ? icon_font_wrapper->font() : nullptr;
    if (icon_font != nullptr) {
        if (label_network_) lv_obj_set_style_text_font(label_network_, icon_font, 0);
        if (label_battery_) lv_obj_set_style_text_font(label_battery_, icon_font, 0);
    }
}
