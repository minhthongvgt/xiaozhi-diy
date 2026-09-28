/**
 * @file i2c_priority.h
 * @brief Shared I2C SDA/SCL pin priority resolution for esp32s3-n16r8-custom
 *
 * The board exposes a single physical I2C master bus (I2C_NUM_0) that may be
 * shared by several optional Kconfig-enabled peripherals (OLED display,
 * external touch panel, audio codec, sensors, RTC, PCA9685, ...). Only one
 * SDA/SCL pin pair can drive that bus, so this resolves a single priority
 * order in one place instead of duplicating it across board_init.c and
 * custom_n16r8_board.cc.
 */

#ifndef BOARDS_ESP32S3_N16R8_CUSTOM_I2C_PRIORITY_H
#define BOARDS_ESP32S3_N16R8_CUSTOM_I2C_PRIORITY_H

#include <driver/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Resolve the SDA/SCL pins to use for the shared I2C master bus.
 *
 * Priority order: Display OLED (SSD1306/SH1106) > External Touch Panel >
 * Audio Codec > BMP280 > BH1750 > BQ27220 Battery > Gas/TVOC (SCD4x/SGP) >
 * APDS9960 > VL53LX > NFC > RTC > INA2xx > LED IC (AW9523B/IS31FL3731) >
 * Discrete Capacitive Touch (CST816D/GT911/FT6236) > PCA9685.
 *
 * If none of the above is configured with a valid pin, both outputs are set
 * to -1 ("not configured" — nothing is selected until the user chooses it).
 *
 * @param[out] sda_pin Resolved SDA pin, or (gpio_num_t)-1 if none configured.
 * @param[out] scl_pin Resolved SCL pin, or (gpio_num_t)-1 if none configured.
 */
void board_resolve_i2c_pins(gpio_num_t *sda_pin, gpio_num_t *scl_pin);

#ifdef __cplusplus
}
#endif

#endif // BOARDS_ESP32S3_N16R8_CUSTOM_I2C_PRIORITY_H
