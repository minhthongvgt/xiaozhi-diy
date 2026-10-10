#ifndef _SOFT_AEC_REFERENCE_H
#define _SOFT_AEC_REFERENCE_H

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include <driver/i2s_common.h>
#include <freertos/FreeRTOS.h>

class SoftAecReference {
public:
    enum class TxFormat {
        kInt16Mono,
        kInt16Stereo,
        kInt32Mono,
    };

    struct Config {
        int input_sample_rate;
        int output_sample_rate;
        int mic_channels;
        TxFormat tx_format;
        int tx_frames_per_block;
        int rx_frames_per_block;
        int rx_desc_num;
    };

    static std::unique_ptr<SoftAecReference> Create(const Config& config,
                                                    i2s_chan_handle_t tx_handle,
                                                    i2s_chan_handle_t rx_handle);
    ~SoftAecReference();

    bool Read(int16_t* dest, int frames, const std::function<int(int16_t*, int)>& read_mic);

    void ResetInput();

private:
    struct StreamClock {
        int64_t last_block = -1;
        int64_t seg_first_block = 0;
        int64_t seg_start_us = 0;
        int64_t last_isr_us = 0;
        uint32_t dropped_blocks = 0;
    };

    explicit SoftAecReference(const Config& config);
    bool Allocate();
    void BuildInterpolator();

    static bool OnTxSent(i2s_chan_handle_t handle, i2s_event_data_t* event, void* user_ctx);
    static bool OnRxReceived(i2s_chan_handle_t handle, i2s_event_data_t* event, void* user_ctx);
    static bool OnRxOverflow(i2s_chan_handle_t handle, i2s_event_data_t* event, void* user_ctx);
    static void UpdateClock(StreamClock& clock, int64_t now_us, int frames, int sample_rate);
    void CopyTxBlock(const void* dma_buf, size_t size, int16_t* dest) const;

    StreamClock SnapshotTx();
    StreamClock SnapshotRx();
    int FifoFrames() const;
    bool ReadBlock(const std::function<int(int16_t*, int)>& read_mic);
    void Produce(int16_t* dest, int frames);
    void FillReference(int16_t* out, int frames, int64_t first_time_q16);

    Config config_;
    i2s_chan_handle_t tx_handle_ = nullptr;
    i2s_chan_handle_t rx_handle_ = nullptr;
    portMUX_TYPE lock_;
    StreamClock tx_clock_;
    StreamClock rx_clock_;
    int16_t* tx_ring_ = nullptr;

    int64_t tx_block_us_ = 0;
    int64_t blocked_read_us_ = 0;
    int64_t lead_q16_ = 0;
    int64_t max_wait_us_ = 0;
    int delay_frames_ = 0;

    int slot_count_ = 0;
    int head_slot_ = 0;
    int used_slots_ = 0;
    int head_offset_ = 0;
    std::vector<int16_t> slot_samples_;
    std::vector<int64_t> slot_start_q16_;

    bool rx_synced_ = false;
    int64_t rx_next_block_ = 0;
    uint32_t rx_dropped_seen_ = 0;

    std::vector<int16_t> taps_;
    std::vector<int16_t> window_;
    std::vector<int16_t> reference_;
};

#endif
