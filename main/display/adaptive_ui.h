#pragma once
#include <lvgl.h>
#include <string>
#include <memory>
#include <cstdint>
class LvglTheme;
enum class UiStyle {
    SmartDashboard = 0,
    CyberTerminal  = 1,
    ChatBubble     = 2,
    ClassicAvatar  = 3,
    MinimalZen     = 4
};
enum class ColorAccent {
    CyberBlue,
    EmeraldGreen,
    ElectricPurple,
    SunsetAmber,
    Monochrome
};
struct ScreenMetrics {
    int width = 240;
    int height = 320;
    bool is_portrait = true;
    bool is_landscape = false;
    bool is_square_or_round = false;
    bool is_mini = false;
    bool is_compact = false;
    bool is_large = false;
    int pad_x = 8;
    int pad_y = 6;
    int corner_radius = 10;
    int safe_inset = 0;
    int content_width = 224;
    int content_height = 308;
};
struct AdaptiveUiConfig {
    UiStyle style = UiStyle::SmartDashboard;
    ColorAccent accent = ColorAccent::CyberBlue;
    bool weather_enabled = true;
    std::string weather_city = "TP. Hồ Chí Minh";
    bool voice_wave_enabled = true;
    bool round_screen_safe_area = false;
    bool auto_scale = true;
    const lv_font_t* text_font = nullptr;
    const lv_font_t* icon_font = nullptr;
};
class AdaptiveUiEngine {
public:
    static AdaptiveUiEngine& GetInstance();
    void Initialize(lv_obj_t* parent_screen, int width, int height, const AdaptiveUiConfig& config);
    void Destroy();
    bool IsInitialized() const { return initialized_; }
    void SetDeviceState(int state, const char* custom_message = nullptr);
    void SetChatMessage(const char* role, const char* content);
    void ClearChatMessages();
    void SetStatusBar(const char* wifi_str, const char* battery_str, const char* time_str);
    void SetWeather(const char* city, const char* temp, const char* desc);
    void SetAudioLevel(int level_0_to_100);
    void SetOtaProgress(int percent, const char* speed_str = nullptr);
    void ShowNotification(const char* notification, int duration_ms = 3000);
    void ApplyTheme(LvglTheme* theme);
    bool NeedsRelayout(LvglTheme* theme) const;
    UiStyle GetCurrentStyle() const { return config_.style; }
    const AdaptiveUiConfig& GetConfig() const { return config_; }
    const ScreenMetrics& GetMetrics() const { return metrics_; }
    lv_color_t GetAccentColor() const;
    lv_obj_t* GetRoot() const { return container_root_; }
    lv_obj_t* GetEmotionContainer() const { return panel_avatar_; }
    lv_obj_t* GetStatusLabel() const { return label_status_; }
    lv_obj_t* GetBatteryLabel() const { return label_battery_; }
    lv_obj_t* GetNetworkLabel() const { return label_network_; }
    lv_obj_t* GetNotificationLabel() const { return label_notification_; }
    lv_obj_t* GetChatMessageLabel() const { return label_chat_; }
private:
    AdaptiveUiEngine();
    ~AdaptiveUiEngine();
    AdaptiveUiEngine(const AdaptiveUiEngine&) = delete;
    AdaptiveUiEngine& operator=(const AdaptiveUiEngine&) = delete;
    void CalculateMetrics(int width, int height);
    lv_color_t GetBgColor() const;
    lv_color_t GetCardBgColor() const;
    lv_color_t GetBorderColor() const;
    lv_color_t GetTextColor() const;
    lv_color_t GetMutedTextColor() const;
    void BuildSmartDashboard(lv_obj_t* parent);
    void BuildCyberTerminal(lv_obj_t* parent);
    void BuildChatBubble(lv_obj_t* parent);
    void BuildClassicAvatar(lv_obj_t* parent);
    void BuildMinimalZen(lv_obj_t* parent);
    void BuildStatusPill(lv_obj_t* parent, int y, int height, lv_color_t bg, lv_color_t border,
                         int radius, lv_color_t dot_color, lv_color_t text_color);
    void BuildAvatarPanel(lv_obj_t* parent, int x, int y, int width, int height, lv_color_t bg,
                          lv_color_t border, int radius, lv_color_t progress_color);
    void BuildChatCard(lv_obj_t* parent, int x, int y, int width, int height, lv_color_t bg,
                       lv_color_t border, int radius, const char* role, lv_color_t role_color,
                       const char* greeting, lv_color_t text_color);
    void BuildBottomBar(lv_obj_t* parent, int height, const char* prompt, lv_color_t prompt_color,
                        lv_color_t wave_track, lv_color_t wave_color);
    int ChatMinHeight() const;
    void ScrollChatToEnd();
    void UpdateClock();
    static void ClockTimerCallback(lv_timer_t* timer);
    bool initialized_ = false;
    AdaptiveUiConfig config_;
    ScreenMetrics metrics_;
    int line_h_ = 20;
    int icon_h_ = 20;
    lv_timer_t* clock_timer_ = nullptr;
    lv_obj_t* root_screen_ = nullptr;
    lv_obj_t* container_root_ = nullptr;
    lv_obj_t* panel_top_bar_ = nullptr;
    lv_obj_t* label_network_ = nullptr;
    lv_obj_t* label_time_ = nullptr;
    lv_obj_t* label_battery_ = nullptr;
    lv_obj_t* label_notification_ = nullptr;
    lv_obj_t* panel_state_pill_ = nullptr;
    lv_obj_t* led_state_dot_ = nullptr;
    lv_obj_t* label_status_ = nullptr;
    lv_obj_t* panel_weather_ = nullptr;
    lv_obj_t* label_weather_city_ = nullptr;
    lv_obj_t* label_weather_temp_ = nullptr;
    lv_obj_t* label_weather_desc_ = nullptr;
    lv_obj_t* panel_avatar_ = nullptr;
    lv_obj_t* label_avatar_ = nullptr;
    lv_obj_t* bar_ota_progress_ = nullptr;
    lv_obj_t* label_ota_progress_ = nullptr;
    lv_obj_t* chat_scroll_ = nullptr;
    lv_obj_t* panel_chat_ = nullptr;
    lv_obj_t* label_chat_role_ = nullptr;
    lv_obj_t* label_chat_ = nullptr;
    lv_obj_t* panel_user_bubble_ = nullptr;
    lv_obj_t* label_user_chat_ = nullptr;
    lv_obj_t* panel_bottom_bar_ = nullptr;
    lv_obj_t* label_prompt_ = nullptr;
    lv_obj_t* bar_audio_wave_ = nullptr;
};
