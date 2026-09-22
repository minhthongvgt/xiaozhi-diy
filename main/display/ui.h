// SquareLine Studio Compatible UI for Xiaozhi AI Chatbot
// Designed for 240x320 Display (ESP32-S3 N16R8)

#ifndef _XIAOZHI_UI_H
#define _XIAOZHI_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl.h>
#include <stdbool.h>
#include <stdint.h>

#include "ui_helpers.h"
#include "ui_events.h"
#include "screens/ui_Screen1.h"

///////////////////// WIDGET DECLARATIONS ////////////////////

// Main Screen
extern lv_obj_t * ui_Screen1;

// 1. Top Status Bar (Wifi, Clock, Battery)
extern lv_obj_t * ui_TopBar;
extern lv_obj_t * ui_LabelWifi;
extern lv_obj_t * ui_LabelTime;
extern lv_obj_t * ui_LabelBattery;

// 2. State Indicator Pill
extern lv_obj_t * ui_StatusBar;
extern lv_obj_t * ui_PanelState;
extern lv_obj_t * ui_LedStatus;
extern lv_obj_t * ui_LabelState;

// 3. Central Avatar & Expression Area
extern lv_obj_t * ui_AvatarPanel;
extern lv_obj_t * ui_LabelAvatar;
extern lv_obj_t * ui_BarProgress;
extern lv_obj_t * ui_LabelProgress;

// 4. Weather Card (Central Information)
extern lv_obj_t * ui_WeatherPanel;
extern lv_obj_t * ui_LabelWeatherCity;
extern lv_obj_t * ui_LabelWeatherTemp;
extern lv_obj_t * ui_LabelWeatherDesc;

// 5. Conversation / Subtitle Chat Bubble
extern lv_obj_t * ui_ChatPanel;
extern lv_obj_t * ui_LabelRole;
extern lv_obj_t * ui_LabelChat;

// 6. Bottom Navigation & Voice Wave
extern lv_obj_t * ui_BottomBar;
extern lv_obj_t * ui_LabelPrompt;
extern lv_obj_t * ui_BarAudio;

// Event root
extern lv_obj_t * ui____initial_actions0;

///////////////////// LIFECYCLE API ////////////////////
void ui_init(void);
void ui_destroy(void);

///////////////////// STATE CONTROL API ////////////////////
/**
 * Set current Xiaozhi device state:
 *  0: Unknown / Booting
 *  1: Starting
 *  2: WifiConfiguring
 *  3: Idle / Standby
 *  4: Connecting
 *  5: Listening
 *  6: Speaking
 *  7: Notifying
 *  8: Upgrading (OTA)
 *  9: Activating
 *  10: AudioTesting
 *  11: FatalError
 */
void ui_set_device_state(int state, const char* custom_message);
void ui_set_chat_message(const char* role, const char* content);
void ui_set_emotion(const char* emotion);
void ui_set_status_bar(const char* wifi_str, const char* battery_str, const char* time_str);
void ui_set_weather(const char* city, const char* temp, const char* desc);
void ui_set_ota_progress(int percent, const char* speed_str);
void ui_set_audio_level(int level_0_to_100);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // _XIAOZHI_UI_H
