/**
 * @file audio_init.c
 * @brief Modern Audio Subsystem Initialization Implementation (ESP-IDF 6.1)
 */

#include "audio_init.h"
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include <driver/i2s_std.h>
#if SOC_I2S_SUPPORTS_PDM_RX
#include <driver/i2s_pdm.h>
#endif

#define TAG "AudioInit"

static i2s_chan_handle_t s_tx_chan = NULL;
static i2s_chan_handle_t s_rx_chan = NULL;

esp_err_t audio_driver_init_speaker(void)
{
#if !defined(CONFIG_ENABLE_CUSTOM_SPEAKER)
    ESP_LOGI(TAG, "Audio Speaker output is disabled in configuration.");
    return ESP_OK;
#else
    ESP_LOGI(TAG, "Initializing Audio Speaker (I2S TX)...");

#if defined(CONFIG_CUSTOM_AUDIO_DAC_PCM5102A)
    ESP_LOGI(TAG, "Hardware Model: PCM5102A Hi-Fi 32-bit DAC");
#elif defined(CONFIG_CUSTOM_AUDIO_DAC_MAX98357A)
    ESP_LOGI(TAG, "Hardware Model: MAX98357A Class-D 3.2W I2S Amplifier");
#elif defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8311)
    ESP_LOGI(TAG, "Hardware Model: Everest ES8311 Low-Power Audio Codec");
#endif

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;

    if (s_tx_chan == NULL) {
        esp_err_t ret = i2s_new_channel(&chan_cfg, &s_tx_chan, NULL);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to allocate I2S TX channel: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    gpio_num_t bclk_pin = GPIO_NUM_15;
    gpio_num_t lrck_pin = GPIO_NUM_16;
    gpio_num_t dout_pin = GPIO_NUM_7;

#if defined(CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_BCLK)
    bclk_pin = (gpio_num_t)CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_BCLK;
#endif
#if defined(CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_LRCK)
    lrck_pin = (gpio_num_t)CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_LRCK;
#endif
#if defined(CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_DOUT)
    dout_pin = (gpio_num_t)CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_DOUT;
#endif

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(24000),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = bclk_pin,
            .ws   = lrck_pin,
            .dout = dout_pin,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    esp_err_t ret = i2s_channel_init_std_mode(s_tx_chan, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I2S STD mode on TX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = i2s_channel_enable(s_tx_chan);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable I2S TX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Audio Speaker initialized on BCLK: %d, LRCK: %d, DOUT: %d",
             bclk_pin, lrck_pin, dout_pin);
    return ESP_OK;
#endif
}

esp_err_t audio_driver_init_mic(void)
{
#if !defined(CONFIG_ENABLE_CUSTOM_MIC)
    ESP_LOGI(TAG, "Audio Microphone is disabled in configuration.");
    return ESP_OK;
#else
    ESP_LOGI(TAG, "Initializing Audio Microphone (I2S RX)...");

#if defined(CONFIG_CUSTOM_AUDIO_MIC_INMP441)
    ESP_LOGI(TAG, "Microphone Model: INMP441 Digital MEMS Microphone");
#elif defined(CONFIG_CUSTOM_AUDIO_MIC_ICS43434)
    ESP_LOGI(TAG, "Microphone Model: ICS-43434 High-SNR RF-shielded MEMS");
#elif defined(CONFIG_CUSTOM_AUDIO_MIC_PDM)
    ESP_LOGI(TAG, "Microphone Model: PDM Digital MEMS Microphone");
#endif

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;

    if (s_rx_chan == NULL) {
        esp_err_t ret = i2s_new_channel(&chan_cfg, NULL, &s_rx_chan);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to allocate I2S RX channel: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    gpio_num_t sck_pin = GPIO_NUM_5;
    gpio_num_t ws_pin  = GPIO_NUM_4;
    gpio_num_t din_pin = GPIO_NUM_6;

#if defined(CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_SCK)
    sck_pin = (gpio_num_t)CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_SCK;
#endif
#if defined(CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_WS)
    ws_pin  = (gpio_num_t)CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_WS;
#endif
#if defined(CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_DIN)
    din_pin = (gpio_num_t)CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_DIN;
#endif

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = sck_pin,
            .ws   = ws_pin,
            .dout = I2S_GPIO_UNUSED,
            .din  = din_pin,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    esp_err_t ret = i2s_channel_init_std_mode(s_rx_chan, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I2S STD mode on RX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = i2s_channel_enable(s_rx_chan);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable I2S RX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Audio Microphone initialized on SCK: %d, WS: %d, DIN: %d",
             sck_pin, ws_pin, din_pin);
    return ESP_OK;
#endif
}

esp_err_t audio_subsystem_init(void)
{
    ESP_LOGI(TAG, "Audio subsystem verified (I2S channels coordinated by C++ AudioCodec).");
    return ESP_OK;
}

esp_err_t audio_init(void)
{
    return audio_subsystem_init();
}
