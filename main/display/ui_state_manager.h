#ifndef _UI_STATE_MANAGER_H_
#define _UI_STATE_MANAGER_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "ui.h"
void ui_set_device_state(int state, const char* custom_message);
void ui_set_chat_message(const char* role, const char* content);
void ui_set_emotion(const char* emotion);
void ui_set_status_bar(const char* wifi_str, const char* battery_str, const char* time_str);
void ui_set_weather(const char* city, const char* temp, const char* desc);
void ui_set_ota_progress(int percent, const char* speed_str);
void ui_set_audio_level(int level_0_to_100);
#ifdef __cplusplus
} 
#endif
#endif 
