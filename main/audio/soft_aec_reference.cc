#include "soft_aec_reference.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include <esp_attr.h>
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/task.h>
#include <sdkconfig.h>

#define TAG "SoftAecReference"

#if CONFIG_I2S_ISR_IRAM_SAFE
#define SOFT_AEC_ISR_ATTR IRAM_ATTR
#else
#define SOFT_AEC_ISR_ATTR
#endif

namespace {

constexpr int64_t kUsPerSecond = 1000000;
constexpr int64_t kLeadUs = 3000;
constexpr int64_t kGuardUs = 3000;
constexpr int64_t kMaxTimelineUs = kUsPerSecond;
constexpr int kTxRingBlocks = 8;
constexpr int kHalfTaps = 12;
constexpr int kTaps = kHalfTaps * 2;
constexpr int kPhases = 32;
constexpr float kKaiserBeta = 6.0f;
constexpr int kMaxChunkFrames = 1024;

int64_t FramesToQ16Us(int64_t frames, int sample_rate) {
    const int64_t scaled = frames * kUsPerSecond;
    return ((scaled / sample_rate) << 16) + (((scaled % sample_rate) << 16) / sample_rate);
}

float BesselI0(float x) {
    float sum = 1.0f;
    float term = 1.0f;
    for (int k = 1; k < 32; ++k) {
        term *= x / (2.0f * k);
        const float squared = term * term;
        sum += squared;
        if (squared < sum * 1e-9f) {
            break;
        }
    }
    return sum;
}

}

SoftAecReference::SoftAecReference(const Config& config) : config_(config) {
    portMUX_INITIALIZE(&lock_);
    tx_block_us_ = config.tx_frames_per_block * kUsPerSecond / config.output_sample_rate;
    const int64_t rx_block_us = config.rx_frames_per_block * kUsPerSecond / config.input_sample_rate;
    blocked_read_us_ = rx_block_us / 4;
    lead_q16_ = kLeadUs << 16;
    max_wait_us_ = tx_block_us_ + kGuardUs;
    const int64_t interpolator_us = kHalfTaps * kUsPerSecond / config.output_sample_rate;
    const int64_t delay_us = tx_block_us_ + kLeadUs + kGuardUs + interpolator_us;
    delay_frames_ = static_cast<int>(
        (delay_us * config.input_sample_rate + kUsPerSecond - 1) / kUsPerSecond);
}

SoftAecReference::~SoftAecReference() {
    i2s_event_callbacks_t none = {};
    if (tx_handle_ != nullptr) {
        i2s_channel_register_event_callback(tx_handle_, &none, nullptr);
    }
    if (rx_handle_ != nullptr) {
        i2s_channel_register_event_callback(rx_handle_, &none, nullptr);
    }
    if (tx_ring_ != nullptr) {
        heap_caps_free(tx_ring_);
    }
}

std::unique_ptr<SoftAecReference> SoftAecReference::Create(const Config& config,
                                                           i2s_chan_handle_t tx_handle,
                                                           i2s_chan_handle_t rx_handle) {
    if (tx_handle == nullptr || rx_handle == nullptr || config.input_sample_rate <= 0 ||
        config.output_sample_rate <= 0 || config.mic_channels <= 0 ||
        config.tx_frames_per_block <= 0 || config.rx_frames_per_block <= 0 ||
        config.rx_desc_num < 2) {
        return nullptr;
    }

    std::unique_ptr<SoftAecReference> reference(new SoftAecReference(config));
    if (!reference->Allocate()) {
        ESP_LOGE(TAG, "Not enough memory for the software AEC reference");
        return nullptr;
    }

    i2s_event_callbacks_t tx_callbacks = {};
    tx_callbacks.on_sent = &SoftAecReference::OnTxSent;
    esp_err_t err = i2s_channel_register_event_callback(tx_handle, &tx_callbacks, reference.get());
    if (err == ESP_OK) {
        reference->tx_handle_ = tx_handle;
        i2s_event_callbacks_t rx_callbacks = {};
        rx_callbacks.on_recv = &SoftAecReference::OnRxReceived;
        rx_callbacks.on_recv_q_ovf = &SoftAecReference::OnRxOverflow;
        err = i2s_channel_register_event_callback(rx_handle, &rx_callbacks, reference.get());
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register I2S DMA callbacks: %s", esp_err_to_name(err));
        return nullptr;
    }
    reference->rx_handle_ = rx_handle;

    ESP_LOGI(TAG,
             "Software AEC reference: speaker %d Hz / %d-frame blocks, mic %d Hz / %d-frame "
             "blocks, mic delay %d ms, lead %d ms",
             config.output_sample_rate, config.tx_frames_per_block, config.input_sample_rate,
             config.rx_frames_per_block, reference->delay_frames_ * 1000 / config.input_sample_rate,
             static_cast<int>(kLeadUs / 1000));
    return reference;
}

bool SoftAecReference::Allocate() {
    const size_t ring_bytes =
        static_cast<size_t>(kTxRingBlocks) * config_.tx_frames_per_block * sizeof(int16_t);
#if CONFIG_I2S_ISR_IRAM_SAFE
    tx_ring_ = static_cast<int16_t*>(
        heap_caps_calloc(1, ring_bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
#else
    tx_ring_ = static_cast<int16_t*>(heap_caps_calloc(1, ring_bytes, MALLOC_CAP_SPIRAM));
    if (tx_ring_ == nullptr) {
        tx_ring_ = static_cast<int16_t*>(
            heap_caps_calloc(1, ring_bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    }
#endif
    if (tx_ring_ == nullptr) {
        return false;
    }

    const int frames = config_.rx_frames_per_block;
    slot_count_ = (kMaxChunkFrames + delay_frames_ + frames - 1) / frames + 2;
    slot_samples_.assign(static_cast<size_t>(slot_count_) * frames * config_.mic_channels, 0);
    slot_start_q16_.assign(slot_count_, 0);
    const int span = (frames * config_.output_sample_rate + config_.input_sample_rate - 1) /
                         config_.input_sample_rate +
                     kTaps + 4;
    window_.assign(span, 0);
    reference_.assign(frames, 0);
    BuildInterpolator();
    return true;
}

void SoftAecReference::BuildInterpolator() {
    const float cutoff =
        0.45f * std::min(config_.input_sample_rate, config_.output_sample_rate) /
        config_.output_sample_rate;
    const float i0_beta = BesselI0(kKaiserBeta);
    taps_.assign(kPhases * kTaps, 0);
    float h[kTaps];
    for (int phase = 0; phase < kPhases; ++phase) {
        const float frac = static_cast<float>(phase) / kPhases;
        float sum = 0.0f;
        for (int k = 0; k < kTaps; ++k) {
            const float distance = static_cast<float>(k - (kHalfTaps - 1)) - frac;
            const float x = 2.0f * cutoff * distance;
            const float sinc = std::fabs(x) < 1e-6f ? 1.0f
                                                    : std::sin(float(M_PI) * x) / (float(M_PI) * x);
            const float v = distance / kHalfTaps;
            const float window =
                std::fabs(v) >= 1.0f ? 0.0f : BesselI0(kKaiserBeta * std::sqrt(1.0f - v * v)) / i0_beta;
            h[k] = 2.0f * cutoff * sinc * window;
            sum += h[k];
        }
        for (int k = 0; k < kTaps; ++k) {
            taps_[phase * kTaps + k] = static_cast<int16_t>(std::lround(h[k] / sum * 32768.0f));
        }
    }
}

void SOFT_AEC_ISR_ATTR SoftAecReference::UpdateClock(StreamClock& clock, int64_t now_us,
                                                     int frames, int sample_rate) {
    const int64_t block = clock.last_block + 1;
    const int64_t block_us = frames * kUsPerSecond / sample_rate;
    bool restart = clock.last_block < 0 || now_us - clock.last_isr_us > block_us * 3 / 2;
    if (!restart) {
        const int64_t blocks = block - clock.seg_first_block + 1;
        const int64_t expected_end =
            clock.seg_start_us + blocks * frames * kUsPerSecond / sample_rate;
        const int64_t error = now_us - expected_end;
        if (error < -block_us / 2 || error > block_us / 2) {
            restart = true;
        } else if (error < 0) {
            clock.seg_start_us += error;
        } else {
            clock.seg_start_us += error / 32;
        }
    }
    if (restart) {
        clock.seg_first_block = block;
        clock.seg_start_us = now_us - block_us;
    }
    clock.last_block = block;
    clock.last_isr_us = now_us;
}

void SOFT_AEC_ISR_ATTR SoftAecReference::CopyTxBlock(const void* dma_buf, size_t size,
                                                     int16_t* dest) const {
    const int frames = config_.tx_frames_per_block;
    if (dma_buf != nullptr) {
        switch (config_.tx_format) {
            case TxFormat::kInt32Mono:
                if (size == frames * sizeof(int32_t)) {
                    const int32_t* src = static_cast<const int32_t*>(dma_buf);
                    for (int i = 0; i < frames; ++i) {
                        dest[i] = static_cast<int16_t>(src[i] >> 16);
                    }
                    return;
                }
                break;
            case TxFormat::kInt16Mono:
                if (size == frames * sizeof(int16_t)) {
                    memcpy(dest, dma_buf, size);
                    return;
                }
                break;
            case TxFormat::kInt16Stereo:
                if (size == frames * 2 * sizeof(int16_t)) {
                    const int16_t* src = static_cast<const int16_t*>(dma_buf);
                    for (int i = 0; i < frames; ++i) {
                        dest[i] = src[i * 2];
                    }
                    return;
                }
                break;
        }
    }
    memset(dest, 0, frames * sizeof(int16_t));
}

bool SOFT_AEC_ISR_ATTR SoftAecReference::OnTxSent(i2s_chan_handle_t handle,
                                                  i2s_event_data_t* event, void* user_ctx) {
    auto* self = static_cast<SoftAecReference*>(user_ctx);
    const int64_t now = esp_timer_get_time();
    const int frames = self->config_.tx_frames_per_block;
    const int64_t block = self->tx_clock_.last_block + 1;
    self->CopyTxBlock(event->dma_buf, event->size,
                      self->tx_ring_ + (block % kTxRingBlocks) * frames);
    portENTER_CRITICAL_ISR(&self->lock_);
    UpdateClock(self->tx_clock_, now, frames, self->config_.output_sample_rate);
    portEXIT_CRITICAL_ISR(&self->lock_);
    return false;
}

bool SOFT_AEC_ISR_ATTR SoftAecReference::OnRxReceived(i2s_chan_handle_t handle,
                                                      i2s_event_data_t* event, void* user_ctx) {
    auto* self = static_cast<SoftAecReference*>(user_ctx);
    const int64_t now = esp_timer_get_time();
    portENTER_CRITICAL_ISR(&self->lock_);
    UpdateClock(self->rx_clock_, now, self->config_.rx_frames_per_block,
                self->config_.input_sample_rate);
    portEXIT_CRITICAL_ISR(&self->lock_);
    return false;
}

bool SOFT_AEC_ISR_ATTR SoftAecReference::OnRxOverflow(i2s_chan_handle_t handle,
                                                      i2s_event_data_t* event, void* user_ctx) {
    auto* self = static_cast<SoftAecReference*>(user_ctx);
    portENTER_CRITICAL_ISR(&self->lock_);
    self->rx_clock_.dropped_blocks++;
    portEXIT_CRITICAL_ISR(&self->lock_);
    return false;
}

SoftAecReference::StreamClock SoftAecReference::SnapshotTx() {
    portENTER_CRITICAL(&lock_);
    StreamClock clock = tx_clock_;
    portEXIT_CRITICAL(&lock_);
    return clock;
}

SoftAecReference::StreamClock SoftAecReference::SnapshotRx() {
    portENTER_CRITICAL(&lock_);
    StreamClock clock = rx_clock_;
    portEXIT_CRITICAL(&lock_);
    return clock;
}

void SoftAecReference::ResetInput() {
    head_slot_ = 0;
    used_slots_ = 0;
    head_offset_ = 0;
    rx_synced_ = false;
}

int SoftAecReference::FifoFrames() const {
    return used_slots_ * config_.rx_frames_per_block - head_offset_;
}

bool SoftAecReference::Read(int16_t* dest, int frames,
                            const std::function<int(int16_t*, int)>& read_mic) {
    const int out_channels = config_.mic_channels + 1;
    while (frames > 0) {
        const int chunk = std::min(frames, kMaxChunkFrames);
        while (FifoFrames() < chunk + delay_frames_) {
            if (!ReadBlock(read_mic)) {
                return false;
            }
        }
        Produce(dest, chunk);
        dest += chunk * out_channels;
        frames -= chunk;
    }
    return true;
}

bool SoftAecReference::ReadBlock(const std::function<int(int16_t*, int)>& read_mic) {
    const int frames = config_.rx_frames_per_block;
    const int samples = frames * config_.mic_channels;
    if (used_slots_ == slot_count_) {
        head_slot_ = (head_slot_ + 1) % slot_count_;
        used_slots_--;
        head_offset_ = 0;
    }
    const int slot = (head_slot_ + used_slots_) % slot_count_;
    int16_t* buffer = &slot_samples_[static_cast<size_t>(slot) * samples];

    for (int attempt = 0;; ++attempt) {
        const StreamClock before = SnapshotRx();
        const int64_t start_us = esp_timer_get_time();
        int got = read_mic(buffer, samples);
        if (got > 0 && got < samples) {
            const int rest = read_mic(buffer + got, samples - got);
            got = rest > 0 ? got + rest : got;
            rx_synced_ = false;
        }
        if (got != samples) {
            rx_synced_ = false;
            return false;
        }
        const bool waited = esp_timer_get_time() - start_us >= blocked_read_us_;
        const StreamClock after = SnapshotRx();
        const uint32_t new_drops = after.dropped_blocks - rx_dropped_seen_;
        rx_dropped_seen_ = after.dropped_blocks;

        int64_t block;
        if (waited) {
            block = std::min(before.last_block + 1, after.last_block);
        } else if (rx_synced_) {
            block = rx_next_block_ + new_drops;
        } else if (attempt <= config_.rx_desc_num) {
            continue;
        } else {
            block = after.last_block;
        }
        block = std::min(block, after.last_block);
        block = std::max(block, after.last_block - (config_.rx_desc_num - 2));
        block = std::max<int64_t>(block, 0);
        rx_synced_ = true;
        rx_next_block_ = block + 1;

        const int64_t last_end_q16 =
            (after.seg_start_us << 16) +
            FramesToQ16Us((after.last_block - after.seg_first_block + 1) * frames,
                          config_.input_sample_rate);
        slot_start_q16_[slot] =
            last_end_q16 -
            FramesToQ16Us((after.last_block - block + 1) * frames, config_.input_sample_rate);
        used_slots_++;
        return true;
    }
}

void SoftAecReference::Produce(int16_t* dest, int frames) {
    const int block_frames = config_.rx_frames_per_block;
    const int mic_channels = config_.mic_channels;
    const int out_channels = mic_channels + 1;
    while (frames > 0) {
        const int run = std::min(frames, block_frames - head_offset_);
        const int16_t* mic =
            &slot_samples_[(static_cast<size_t>(head_slot_) * block_frames + head_offset_) *
                           mic_channels];
        const int64_t capture_q16 =
            slot_start_q16_[head_slot_] + FramesToQ16Us(head_offset_, config_.input_sample_rate);
        FillReference(reference_.data(), run, capture_q16 + lead_q16_);
        for (int i = 0; i < run; ++i) {
            for (int c = 0; c < mic_channels; ++c) {
                dest[i * out_channels + c] = mic[i * mic_channels + c];
            }
            dest[i * out_channels + mic_channels] = reference_[i];
        }
        dest += run * out_channels;
        frames -= run;
        head_offset_ += run;
        if (head_offset_ == block_frames) {
            head_offset_ = 0;
            head_slot_ = (head_slot_ + 1) % slot_count_;
            used_slots_--;
        }
    }
}

void SoftAecReference::FillReference(int16_t* out, int frames, int64_t first_time_q16) {
    const int block_frames = config_.tx_frames_per_block;
    const int out_rate = config_.output_sample_rate;
    const int64_t step = (static_cast<int64_t>(out_rate) << 16) / config_.input_sample_rate;
    const int64_t wait_start_us = esp_timer_get_time();

    StreamClock tx = SnapshotTx();
    int64_t first_pos = 0;
    int64_t available_end = 0;
    while (true) {
        if (tx.last_block < 0) {
            std::fill(out, out + frames, 0);
            return;
        }
        available_end = (tx.last_block + 1) * block_frames;
        const int64_t available_end_q16 =
            (tx.seg_start_us << 16) +
            FramesToQ16Us((tx.last_block - tx.seg_first_block + 1) * block_frames, out_rate);
        const int64_t offset_q16 = first_time_q16 - available_end_q16;
        if (offset_q16 > (kMaxTimelineUs << 16) || offset_q16 < -(kMaxTimelineUs << 16)) {
            std::fill(out, out + frames, 0);
            return;
        }
        first_pos = (available_end << 16) + offset_q16 * out_rate / kUsPerSecond;
        const int64_t needed_end = ((first_pos + step * (frames - 1)) >> 16) + kHalfTaps + 2;
        const int64_t now = esp_timer_get_time();
        const bool tx_running = now - tx.last_isr_us < 2 * tx_block_us_;
        if (needed_end <= available_end || !tx_running || now - wait_start_us > max_wait_us_) {
            break;
        }
        vTaskDelay(1);
        tx = SnapshotTx();
    }

    const int64_t last_pos = first_pos + step * (frames - 1);
    const int64_t window_begin = (first_pos >> 16) - (kHalfTaps - 1);
    const int64_t window_end = (last_pos >> 16) + kHalfTaps + 2;
    const int64_t valid_begin =
        std::max(tx.seg_first_block, tx.last_block - kTxRingBlocks + 2) * block_frames;
    const int64_t valid_end = available_end;
    if (window_end <= valid_begin || window_begin >= valid_end ||
        window_end - window_begin > static_cast<int64_t>(window_.size())) {
        std::fill(out, out + frames, 0);
        return;
    }
    const int64_t ring_size = static_cast<int64_t>(kTxRingBlocks) * block_frames;
    for (int64_t pos = window_begin; pos < window_end; ++pos) {
        window_[pos - window_begin] =
            (pos >= valid_begin && pos < valid_end) ? tx_ring_[pos % ring_size] : 0;
    }

    int64_t pos = first_pos;
    for (int i = 0; i < frames; ++i, pos += step) {
        int64_t index = pos >> 16;
        int phase = static_cast<int>(((pos & 0xFFFF) * kPhases + 0x8000) >> 16);
        if (phase == kPhases) {
            ++index;
            phase = 0;
        }
        const int16_t* x = &window_[index - (kHalfTaps - 1) - window_begin];
        const int16_t* h = &taps_[phase * kTaps];
        int32_t acc = 0;
        for (int k = 0; k < kTaps; ++k) {
            acc += static_cast<int32_t>(x[k]) * h[k];
        }
        acc >>= 15;
        out[i] = static_cast<int16_t>(std::clamp<int32_t>(acc, INT16_MIN, INT16_MAX));
    }
}
