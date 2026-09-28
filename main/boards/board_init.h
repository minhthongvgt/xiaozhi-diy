/**
 * @file board_init.h
 * @brief Generic Board Initialization Interface
 */

#ifndef BOARDS_BOARD_INIT_H
#define BOARDS_BOARD_INIT_H

#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize generic board-specific configuration
 */
esp_err_t board_init(void);

#ifdef __cplusplus
}
#endif

#endif // BOARDS_BOARD_INIT_H
