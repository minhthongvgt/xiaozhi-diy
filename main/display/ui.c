// This file is generated / managed for SquareLine Studio
// SquareLine Studio version: 1.6.2
// LVGL version: 8.3.11 / 9.x compatible

#include "ui.h"

///////////////////// VARIABLES ////////////////////
lv_obj_t * ui____initial_actions0 = NULL;

///////////////////// FUNCTIONS ////////////////////
void ui_init(void)
{
    ui_Screen1_screen_init();
    ui____initial_actions0 = lv_obj_create(NULL);

#if LVGL_VERSION_MAJOR >= 9
    lv_screen_load(ui_Screen1);
#else
    lv_disp_load_scr(ui_Screen1);
#endif
}

void ui_destroy(void)
{
    ui_Screen1_screen_destroy();
}
