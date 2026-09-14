/**
 * @file board_init.c
 * @brief Generic Board Initialization Implementation
 */

#include "boards/board_init.h"
#include "boards/esp32s3_wroom/board_init.h"

esp_err_t board_init(void)
{
    return board_esp32s3_wroom_init();
}
