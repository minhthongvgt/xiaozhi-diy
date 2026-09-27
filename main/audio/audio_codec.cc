#include "audio_codec.h"
#include "board.h"
#include "settings.h"

#include <esp_log.h>
#include <cstring>
#include <driver/i2s_common.h>

#define TAG "AudioCodec"

AudioCodec::AudioCodec() {
    esp_timer_create_args_t timer_args = {
        .callback = &AudioCodec::VolumeSaveTimerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "vol_save",
        .skip_unhandled_events = true,
    };
    esp_err_t err = esp_timer_create(&timer_args, &volume_save_timer_);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to create volume save timer: %s", esp_err_to_name(err));
        volume_save_timer_ = nullptr;
    }
}

AudioCodec::~AudioCodec() {
    if (volume_save_timer_ != nullptr) {
        esp_timer_stop(volume_save_timer_);
        esp_timer_delete(volume_save_timer_);
        volume_save_timer_ = nullptr;
    }
}

void AudioCodec::OutputData(std::vector<int16_t>& data) {
    Write(data.data(), data.size());
}

bool AudioCodec::InputData(std::vector<int16_t>& data) {
    int samples = Read(data.data(), data.size());
    if (samples > 0) {
        return true;
    }
    return false;
}

void AudioCodec::Start() {
    Settings settings("audio", false);
    output_volume_ = settings.GetInt("output_volume", output_volume_);
    if (output_volume_ <= 0) {
        ESP_LOGW(TAG, "Output volume value (%d) is too small, setting to default (10)", output_volume_);
        output_volume_ = 10;
    } else if (output_volume_ > 100) {
        ESP_LOGW(TAG, "Output volume value (%d) is too large, setting to max (100)", output_volume_);
        output_volume_ = 100;
    }

    ESP_LOGI(TAG, "Audio codec started with volume: %d", output_volume_);
}

void AudioCodec::SetOutputVolume(int volume) {
    int target_volume = (volume < 0) ? 0 : (volume > 100) ? 100 : volume;
    if (output_volume_ == target_volume) {
        return;
    }
    output_volume_ = target_volume;
    ESP_LOGI(TAG, "Set output volume to %d", output_volume_);
    
    // Debounce NVS Flash write: DO NOT block I2S DMA with synchronous nvs_commit()
    // Restart one-shot timer (1500 ms) so that rapid slider swipes or button presses
    // only write to SPI Flash ONCE after the user stops adjusting.
    if (volume_save_timer_ != nullptr) {
        esp_timer_stop(volume_save_timer_);
        esp_timer_start_once(volume_save_timer_, 1500000); // 1.5s in microseconds
    } else {
        SaveVolumeToNvs();
    }
}

void AudioCodec::VolumeSaveTimerCallback(void* arg) {
    auto self = static_cast<AudioCodec*>(arg);
    if (self) {
        self->SaveVolumeToNvs();
    }
}

void AudioCodec::SaveVolumeToNvs() {
    ESP_LOGI(TAG, "Saving volume %d to NVS Flash (debounced)", output_volume_);
    Settings settings("audio", true);
    settings.SetInt("output_volume", output_volume_);
}

void AudioCodec::SetInputGain(float gain) {
    input_gain_ = gain;
    ESP_LOGI(TAG, "Set input gain to %.1f", input_gain_);
}

void AudioCodec::EnableInput(bool enable) {
    if (enable == input_enabled_) {
        return;
    }
    input_enabled_ = enable;
    ESP_LOGI(TAG, "Set input enable to %s", enable ? "true" : "false");
}

void AudioCodec::EnableOutput(bool enable) {
    if (enable == output_enabled_) {
        return;
    }
    output_enabled_ = enable;
    ESP_LOGI(TAG, "Set output enable to %s", enable ? "true" : "false");
}
