#ifndef DISPLAY_INIT_H
#define DISPLAY_INIT_H
#include <esp_err.h>
#ifdef __cplusplus
extern "C" {
#endif
esp_err_t display_subsystem_init(void);
esp_err_t display_init(void);
void display_set_backlight(uint8_t brightness_pct);
#ifdef __cplusplus
}
#endif
#endif 
