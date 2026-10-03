#ifndef AUDIO_INIT_H
#define AUDIO_INIT_H
#include <esp_err.h>
#include <driver/i2s_std.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    AUDIO_DAC_TYPE_MAX98357A = 0,
    AUDIO_DAC_TYPE_PCM5102A,
    AUDIO_DAC_TYPE_ES8311,
    AUDIO_DAC_TYPE_TAS5805M,
} audio_dac_type_t;
typedef enum {
    AUDIO_MIC_TYPE_INMP441 = 0,
    AUDIO_MIC_TYPE_ICS43434,
    AUDIO_MIC_TYPE_SPH0645,
    AUDIO_MIC_TYPE_PDM,
} audio_mic_type_t;
esp_err_t audio_driver_init_speaker(void);
esp_err_t audio_driver_init_mic(void);
esp_err_t audio_subsystem_init(void);
esp_err_t audio_init(void);
#ifdef __cplusplus
}
#endif
#endif 
