#include "status_activity.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace {

constexpr float kPi = 3.14159265f;

int ScaleByLevel(int full, int level) {
    return full * std::clamp(level, 0, 255) / 255;
}

uint32_t FrameNoise(uint32_t frame, uint32_t salt) {
    uint32_t value = frame * 2654435761u + salt * 97u;
    value ^= value >> 15;
    value *= 2246822519u;
    return value ^ (value >> 13);
}

}

StatusActivityAnimator::StatusActivityAnimator(const StatusActivityConfig& config, LevelSource level_source)
    : config_(config), level_source_(std::move(level_source)) {}

StatusActivityAnimator::~StatusActivityAnimator() {
    Release();
}

StatusActivityStyle StatusActivityAnimator::StyleFor(StatusActivityKind kind) const {
    return kind == StatusActivityKind::kSpeaking ? config_.speaking_style : config_.listening_style;
}

bool StatusActivityAnimator::Show(lv_obj_t* anchor, StatusActivityKind kind) {
    const StatusActivityStyle style = StyleFor(kind);
    if (anchor == nullptr || style == StatusActivityStyle::kText) {
        Hide();
        return false;
    }
    if (container_ != nullptr && anchor_ == anchor && style_ == style && kind_ == kind) {
        SetVisible(true);
        return true;
    }
    Hide();
    anchor_ = anchor;
    kind_ = kind;
    style_ = style;
    Build();
    return container_ != nullptr;
}

void StatusActivityAnimator::Hide() {
    Release();
    anchor_ = nullptr;
}

void StatusActivityAnimator::SetVisible(bool visible) {
    if (container_ == nullptr) {
        return;
    }
    if (visible) {
        lv_obj_remove_flag(container_, LV_OBJ_FLAG_HIDDEN);
        if (timer_ != nullptr) {
            lv_timer_resume(timer_);
        }
    } else {
        lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
        if (timer_ != nullptr) {
            lv_timer_pause(timer_);
        }
    }
}

void StatusActivityAnimator::Release() {
    if (timer_ != nullptr) {
        lv_timer_delete(timer_);
        timer_ = nullptr;
    }
    lv_obj_t* container = container_;
    container_ = nullptr;
    parts_.fill(nullptr);
    part_count_ = 0;
    wave_lines_[0] = nullptr;
    wave_lines_[1] = nullptr;
    if (container != nullptr) {
        lv_obj_delete(container);
    }
}

void StatusActivityAnimator::DeleteCallback(lv_event_t* event) {
    auto* self = static_cast<StatusActivityAnimator*>(lv_event_get_user_data(event));
    if (self == nullptr || lv_event_get_target_obj(event) != self->container_) {
        return;
    }
    if (self->timer_ != nullptr) {
        lv_timer_delete(self->timer_);
        self->timer_ = nullptr;
    }
    self->container_ = nullptr;
    self->anchor_ = nullptr;
    self->parts_.fill(nullptr);
    self->part_count_ = 0;
    self->wave_lines_[0] = nullptr;
    self->wave_lines_[1] = nullptr;
}

void StatusActivityAnimator::FrameCallback(lv_timer_t* timer) {
    auto* self = static_cast<StatusActivityAnimator*>(lv_timer_get_user_data(timer));
    if (self != nullptr && self->container_ != nullptr) {
        self->OnFrame();
    }
}

lv_obj_t* StatusActivityAnimator::CreatePart(int width, int height, int radius) {
    if (part_count_ >= kMaxParts) {
        return nullptr;
    }
    lv_obj_t* part = lv_obj_create(container_);
    lv_obj_remove_style_all(part);
    lv_obj_remove_flag(part, static_cast<lv_obj_flag_t>(LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE));
    lv_obj_set_size(part, width, height);
    lv_obj_set_style_radius(part, radius, 0);
    lv_obj_set_style_bg_opa(part, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(part, PartColor(part_count_, kMaxParts), 0);
    parts_[part_count_++] = part;
    return part;
}

void StatusActivityAnimator::Build() {
    lv_obj_t* parent = lv_obj_get_parent(anchor_);
    if (parent == nullptr) {
        return;
    }
    lv_obj_update_layout(anchor_);
    width_ = std::max<int>(lv_obj_get_width(anchor_), 32);
    height_ = std::max<int>(lv_obj_get_height(anchor_), 18);
    base_color_ = config_.color_mode == StatusActivityColorMode::kCustom ? lv_color_hex(config_.custom_color)
                                                                         : lv_obj_get_style_text_color(anchor_, LV_PART_MAIN);
    frame_ = 0;
    level_ = 0;
    part_levels_.fill(0);
    part_count_ = 0;
    next_blink_frame_ = 30;

    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);
    lv_obj_remove_flag(container_, static_cast<lv_obj_flag_t>(LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE));
    lv_obj_add_flag(container_, LV_OBJ_FLAG_FLOATING);
    lv_obj_set_size(container_, width_, height_);
    lv_obj_align_to(container_, anchor_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(container_, DeleteCallback, LV_EVENT_DELETE, this);

    switch (style_) {
        case StatusActivityStyle::kBars:
            BuildBars();
            break;
        case StatusActivityStyle::kWave:
            BuildWave();
            break;
        case StatusActivityStyle::kRipple:
            BuildRipple();
            break;
        case StatusActivityStyle::kDots:
            BuildDots();
            break;
        case StatusActivityStyle::kEyes:
            BuildEyes();
            break;
        default:
            break;
    }
    timer_ = lv_timer_create(FrameCallback, kFrameMs, this);
    OnFrame();
}

lv_color_t StatusActivityAnimator::PartColor(int index, int count) const {
    if (config_.color_mode != StatusActivityColorMode::kRainbow) {
        return base_color_;
    }
    const int span = std::max(count, 1);
    const uint16_t hue = static_cast<uint16_t>((frame_ * 6 + static_cast<uint32_t>(index) * 360 / span) % 360);
    return lv_color_hsv_to_rgb(hue, 85, 100);
}

void StatusActivityAnimator::OnFrame() {
    frame_++;
    const int target = level_source_ ? level_source_(kind_) : 0;
    level_ = target > level_ ? target : level_ * 85 / 100;
    switch (style_) {
        case StatusActivityStyle::kBars:
            DrawBars();
            break;
        case StatusActivityStyle::kWave:
            DrawWave();
            break;
        case StatusActivityStyle::kRipple:
            DrawRipple();
            break;
        case StatusActivityStyle::kDots:
            DrawDots();
            break;
        case StatusActivityStyle::kEyes:
            DrawEyes();
            break;
        default:
            break;
    }
}

void StatusActivityAnimator::BuildBars() {
    const int count = std::clamp(width_ / 14, 5, kMaxParts);
    const int bar_width = std::clamp(width_ / (count * 2), 3, 8);
    const int total = count * bar_width + (count - 1) * bar_width;
    const int start_x = (width_ - total) / 2;
    for (int i = 0; i < count; i++) {
        lv_obj_t* bar = CreatePart(bar_width, 2, bar_width / 2);
        if (bar != nullptr) {
            lv_obj_set_x(bar, start_x + i * bar_width * 2);
        }
    }
}

void StatusActivityAnimator::DrawBars() {
    const int min_height = std::max(2, height_ / 8);
    const float level = level_ / 255.0f;
    for (int i = 0; i < part_count_; i++) {
        const float shape = 0.35f + 0.65f * std::sin(kPi * (i + 0.5f) / part_count_);
        const float wobble = 0.5f + 0.5f * std::sin(frame_ * 0.35f + i * 1.7f);
        const float jitter = (FrameNoise(frame_ / 2, static_cast<uint32_t>(i)) & 0xFF) / 255.0f;
        float amplitude = 0.0f;
        if (kind_ == StatusActivityKind::kSpeaking) {
            amplitude = level * shape * (0.55f + 0.3f * wobble + 0.15f * jitter);
        } else {
            amplitude = (0.15f + 0.85f * level) * shape * (0.35f + 0.65f * wobble);
        }
        const int value = static_cast<int>(std::clamp(amplitude, 0.0f, 1.0f) * 255.0f);
        part_levels_[i] = static_cast<uint8_t>(std::max(value, part_levels_[i] * 80 / 100));
        const int bar_height = min_height + ScaleByLevel(height_ - min_height, part_levels_[i]);
        lv_obj_set_height(parts_[i], bar_height);
        lv_obj_set_y(parts_[i], (height_ - bar_height) / 2);
        if (config_.color_mode == StatusActivityColorMode::kRainbow) {
            lv_obj_set_style_bg_color(parts_[i], PartColor(i, part_count_), 0);
        }
    }
}

void StatusActivityAnimator::BuildWave() {
    for (int line = 0; line < 2; line++) {
        lv_obj_t* wave = lv_line_create(container_);
        lv_obj_set_pos(wave, 0, 0);
        lv_obj_set_style_line_width(wave, height_ > 24 ? 3 : 2, 0);
        lv_obj_set_style_line_rounded(wave, true, 0);
        lv_obj_set_style_line_color(wave, PartColor(line * 4, 8), 0);
        lv_obj_set_style_line_opa(wave, line == 0 ? LV_OPA_COVER : LV_OPA_50, 0);
        wave_lines_[line] = wave;
    }
}

void StatusActivityAnimator::DrawWave() {
    const float middle = (height_ - 1) / 2.0f;
    const float level = level_ / 255.0f;
    const float strength = kind_ == StatusActivityKind::kSpeaking ? 0.1f + 0.9f * level : 0.3f + 0.7f * level;
    const float amplitude = (middle - 2.0f) * strength;
    const float cycles[2] = {2.0f, 3.0f};
    const float speeds[2] = {0.30f, -0.22f};
    const float scales[2] = {1.0f, 0.6f};
    for (int line = 0; line < 2; line++) {
        if (wave_lines_[line] == nullptr) {
            continue;
        }
        auto& points = wave_points_[line];
        for (int k = 0; k < kWavePoints; k++) {
            const float t = static_cast<float>(k) / (kWavePoints - 1);
            const float envelope = std::sin(kPi * t);
            const float y = middle + amplitude * scales[line] * envelope *
                                         std::sin(2.0f * kPi * cycles[line] * t + frame_ * speeds[line]);
            points[k].x = static_cast<lv_value_precise_t>(t * (width_ - 1));
            points[k].y = static_cast<lv_value_precise_t>(y);
        }
        lv_line_set_points(wave_lines_[line], points.data(), kWavePoints);
        if (config_.color_mode == StatusActivityColorMode::kRainbow) {
            lv_obj_set_style_line_color(wave_lines_[line], PartColor(line * 4, 8), 0);
        }
    }
}

void StatusActivityAnimator::BuildRipple() {
    for (int i = 0; i < 3; i++) {
        lv_obj_t* ring = CreatePart(4, 4, LV_RADIUS_CIRCLE);
        if (ring != nullptr) {
            lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(ring, 2, 0);
            lv_obj_set_style_border_color(ring, PartColor(i, 3), 0);
        }
    }
    CreatePart(6, 6, LV_RADIUS_CIRCLE);
}

void StatusActivityAnimator::DrawRipple() {
    const int period = kind_ == StatusActivityKind::kSpeaking ? 28 : 42;
    const int max_height = height_;
    const int max_width = std::min(width_, height_ * 3);
    for (int i = 0; i < 3 && i < part_count_; i++) {
        const int step = static_cast<int>((frame_ + static_cast<uint32_t>(i * period / 3)) % period);
        const int progress = step * 255 / period;
        const int ring_height = 6 + ScaleByLevel(max_height - 6, progress);
        const int ring_width = 6 + ScaleByLevel(max_width - 6, progress);
        lv_obj_set_size(parts_[i], ring_width, ring_height);
        lv_obj_set_pos(parts_[i], (width_ - ring_width) / 2, (height_ - ring_height) / 2);
        lv_obj_set_style_border_opa(parts_[i], static_cast<lv_opa_t>(255 - progress), 0);
        if (config_.color_mode == StatusActivityColorMode::kRainbow) {
            lv_obj_set_style_border_color(parts_[i], PartColor(i, 3), 0);
        }
    }
    if (part_count_ > 3) {
        const int pulse = kind_ == StatusActivityKind::kSpeaking
                              ? level_
                              : 96 + static_cast<int>(64.0f * std::sin(frame_ * 0.2f)) + level_ / 3;
        const int dot = std::max(4, 4 + ScaleByLevel(height_ / 2, pulse));
        lv_obj_set_size(parts_[3], dot, dot);
        lv_obj_set_pos(parts_[3], (width_ - dot) / 2, (height_ - dot) / 2);
        if (config_.color_mode == StatusActivityColorMode::kRainbow) {
            lv_obj_set_style_bg_color(parts_[3], PartColor(3, 4), 0);
        }
    }
}

void StatusActivityAnimator::BuildDots() {
    const int size = std::clamp(height_ / 4, 4, 12);
    const int count = 5;
    const int total = count * size + (count - 1) * size;
    const int start_x = (width_ - total) / 2;
    for (int i = 0; i < count; i++) {
        lv_obj_t* dot = CreatePart(size, size, LV_RADIUS_CIRCLE);
        if (dot != nullptr) {
            lv_obj_set_x(dot, start_x + i * size * 2);
        }
    }
}

void StatusActivityAnimator::DrawDots() {
    const float level = level_ / 255.0f;
    const float strength = kind_ == StatusActivityKind::kSpeaking ? 0.2f + 0.8f * level : 0.55f + 0.45f * level;
    for (int i = 0; i < part_count_; i++) {
        const int size = lv_obj_get_width(parts_[i]);
        const float travel = (height_ - size) / 2.0f;
        const float hop = std::max(0.0f, std::sin(frame_ * 0.28f - i * 0.75f));
        const int y = static_cast<int>((height_ - size) / 2.0f + travel * 0.9f * strength * (0.5f - hop));
        lv_obj_set_y(parts_[i], std::clamp(y, 0, height_ - size));
        if (config_.color_mode == StatusActivityColorMode::kRainbow) {
            lv_obj_set_style_bg_color(parts_[i], PartColor(i, part_count_), 0);
        }
    }
}

void StatusActivityAnimator::BuildEyes() {
    const int eye_width = std::clamp(height_ * 45 / 100, 5, 28);
    CreatePart(eye_width, height_ / 2, eye_width / 2);
    CreatePart(eye_width, height_ / 2, eye_width / 2);
    lv_obj_t* mouth = CreatePart(eye_width, 2, 2);
    if (mouth != nullptr && kind_ != StatusActivityKind::kSpeaking) {
        lv_obj_add_flag(mouth, LV_OBJ_FLAG_HIDDEN);
    }
}

void StatusActivityAnimator::DrawEyes() {
    static constexpr uint32_t kBlinkFrames = 4;
    static constexpr int kBlinkScale[kBlinkFrames] = {55, 10, 10, 55};
    if (part_count_ < 3) {
        return;
    }
    const bool speaking = kind_ == StatusActivityKind::kSpeaking;
    const int eye_width = lv_obj_get_width(parts_[0]);
    const int open_height = speaking ? height_ * 55 / 100 : height_ * (70 + level_ * 15 / 255) / 100;
    int eye_height = std::max(2, open_height);
    if (frame_ >= next_blink_frame_) {
        const uint32_t blink_step = frame_ - next_blink_frame_;
        if (blink_step < kBlinkFrames) {
            eye_height = std::max(2, open_height * kBlinkScale[blink_step] / 100);
        } else {
            next_blink_frame_ = frame_ + 50 + FrameNoise(frame_, 7) % 60;
        }
    }
    const int glance = speaking ? 0 : static_cast<int>(std::sin(frame_ * 0.05f) * eye_width * 0.6f);
    const int bob = speaking ? static_cast<int>(std::sin(frame_ * 0.4f) * 1.5f) : 0;
    const int spacing = eye_width * 26 / 10;
    const int eye_center_y = speaking ? height_ * 36 / 100 : height_ / 2;
    const int center_x = width_ / 2 + glance;
    for (int i = 0; i < 2; i++) {
        const int x = center_x + (i == 0 ? -spacing / 2 : spacing / 2) - eye_width / 2;
        lv_obj_set_size(parts_[i], eye_width, eye_height);
        lv_obj_set_pos(parts_[i], x, std::max(0, eye_center_y - eye_height / 2 + bob));
        if (config_.color_mode == StatusActivityColorMode::kRainbow) {
            lv_obj_set_style_bg_color(parts_[i], PartColor(i, 3), 0);
        }
    }
    if (speaking) {
        const int mouth_width = eye_width + ScaleByLevel(eye_width * 2, level_);
        const int mouth_height = 2 + ScaleByLevel(std::max(2, height_ * 22 / 100), level_);
        const int mouth_y = std::min(height_ - mouth_height, height_ * 80 / 100 - mouth_height / 2);
        lv_obj_set_size(parts_[2], mouth_width, mouth_height);
        lv_obj_set_pos(parts_[2], (width_ - mouth_width) / 2, std::max(0, mouth_y));
        lv_obj_set_style_radius(parts_[2], mouth_height / 2, 0);
        if (config_.color_mode == StatusActivityColorMode::kRainbow) {
            lv_obj_set_style_bg_color(parts_[2], PartColor(2, 3), 0);
        }
    }
}
