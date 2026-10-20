#ifndef STATUS_ACTIVITY_H
#define STATUS_ACTIVITY_H
#include <lvgl.h>
#include <array>
#include <cstdint>
#include <functional>

enum class StatusActivityStyle : int {
    kText = 0,
    kBars = 1,
    kWave = 2,
    kRipple = 3,
    kDots = 4,
    kEyes = 5,
};

enum class StatusActivityKind : uint8_t {
    kListening = 0,
    kSpeaking = 1,
};

enum class StatusActivityColorMode : uint8_t {
    kTheme = 0,
    kRainbow = 1,
    kCustom = 2,
};

typedef struct StatusActivityConfig {
    StatusActivityStyle listening_style;
    StatusActivityStyle speaking_style;
    StatusActivityColorMode color_mode;
    uint32_t custom_color;
} StatusActivityConfig;

class StatusActivityAnimator {
public:
    using LevelSource = std::function<uint8_t(StatusActivityKind)>;

    StatusActivityAnimator(const StatusActivityConfig& config, LevelSource level_source);
    ~StatusActivityAnimator();

    bool Show(lv_obj_t* anchor, StatusActivityKind kind);
    void Hide();
    void SetVisible(bool visible);
    bool IsActive() const { return container_ != nullptr; }
    StatusActivityStyle StyleFor(StatusActivityKind kind) const;

private:
    static constexpr int kFrameMs = 40;
    static constexpr int kMaxParts = 16;
    static constexpr int kWavePoints = 33;

    StatusActivityConfig config_;
    LevelSource level_source_;
    StatusActivityKind kind_ = StatusActivityKind::kListening;
    StatusActivityStyle style_ = StatusActivityStyle::kText;
    lv_obj_t* anchor_ = nullptr;
    lv_obj_t* container_ = nullptr;
    lv_timer_t* timer_ = nullptr;
    std::array<lv_obj_t*, kMaxParts> parts_{};
    int part_count_ = 0;
    std::array<uint8_t, kMaxParts> part_levels_{};
    lv_obj_t* wave_lines_[2] = {nullptr, nullptr};
    std::array<lv_point_precise_t, kWavePoints> wave_points_[2]{};
    int width_ = 0;
    int height_ = 0;
    uint32_t frame_ = 0;
    int level_ = 0;
    uint32_t next_blink_frame_ = 0;
    lv_color_t base_color_{};

    void Build();
    void BuildBars();
    void BuildWave();
    void BuildRipple();
    void BuildDots();
    void BuildEyes();
    lv_obj_t* CreatePart(int width, int height, int radius);
    void Release();
    void OnFrame();
    void DrawBars();
    void DrawWave();
    void DrawRipple();
    void DrawDots();
    void DrawEyes();
    lv_color_t PartColor(int index, int count) const;
    static void FrameCallback(lv_timer_t* timer);
    static void DeleteCallback(lv_event_t* event);
};
#endif
