#ifndef USER_DRIVER_REGISTRY_H
#define USER_DRIVER_REGISTRY_H
#include <esp_log.h>
#include <driver/i2c_master.h>
#include <esp_lcd_touch.h>
#include "display/display.h"
#include "audio_codec.h"
#include "led/led.h"
#if CONFIG_IDF_TARGET_ESP32S3
#include "boards/common/esp32_camera.h"
#endif
__attribute__((weak)) Display* CreateUserCustomDisplayDriver(i2c_master_bus_handle_t i2c_bus) {
    ESP_LOGW("UserDriverRegistry", "Custom User Display Driver hook invoked but not overridden. Falling back to NoDisplay.");
    return new NoDisplay();
}
__attribute__((weak)) esp_lcd_touch_handle_t CreateUserCustomTouchDriver(i2c_master_bus_handle_t i2c_bus) {
    ESP_LOGW("UserDriverRegistry", "Custom User Touch Driver hook invoked but not overridden.");
    return nullptr;
}
__attribute__((weak)) AudioCodec* CreateUserCustomAudioCodecDriver(i2c_master_bus_handle_t i2c_bus) {
    ESP_LOGW("UserDriverRegistry", "Custom User Audio Codec Driver hook invoked but not overridden.");
    return nullptr;
}
#if CONFIG_IDF_TARGET_ESP32S3
__attribute__((weak)) Camera* CreateUserCustomCameraDriver() {
    ESP_LOGW("UserDriverRegistry", "Custom User Camera Driver hook invoked but not overridden.");
    return nullptr;
}
#endif
__attribute__((weak)) Led* CreateUserCustomLedDriver() {
    ESP_LOGW("UserDriverRegistry", "Custom User LED Driver hook invoked but not overridden.");
    return nullptr;
}
__attribute__((weak)) void InitializeUserCustomSensors(i2c_master_bus_handle_t i2c_bus) {
    ESP_LOGI("UserDriverRegistry", "Custom User Sensors initialization hook called.");
}
#endif 
