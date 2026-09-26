/**
 * @file display_init.c
 * @brief Display Subsystem Initialization Implementation (ESP-IDF 6.1)
 */

#include "display_init.h"
#include "boards/common/bus_manager.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <driver/gpio.h>
#include <driver/ledc.h>

#define TAG "DisplayInit"

static gpio_num_t s_bl_pin = GPIO_NUM_NC;
static bool s_ledc_initialized = false;

/**
 * @brief Initialize LEDC timer and channel for backlight PWM control.
 *        Must be called before display_set_backlight().
 */
static esp_err_t display_ledc_init(gpio_num_t bl_pin)
{
    if (s_ledc_initialized) {
        return ESP_OK;
    }
    if (bl_pin == GPIO_NUM_NC) {
        return ESP_ERR_INVALID_ARG;
    }

    // Configure LEDC timer: 13-bit resolution at 5kHz (suitable for LCD backlight)
    ledc_timer_config_t timer_conf = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK,
    };
    esp_err_t ret = ledc_timer_config(&timer_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LEDC timer config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configure LEDC channel
    ledc_channel_config_t channel_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = bl_pin,
        .duty       = 8191, // 100% brightness on startup
        .hpoint     = 0,
    };
    ret = ledc_channel_config(&channel_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LEDC channel config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    s_bl_pin = bl_pin;
    s_ledc_initialized = true;
    ESP_LOGI(TAG, "Backlight LEDC initialized on GPIO %d (13-bit, 5kHz)", bl_pin);
    return ESP_OK;
}

void display_set_backlight(uint8_t brightness_pct)
{
    if (!s_ledc_initialized || s_bl_pin == GPIO_NUM_NC) {
        ESP_LOGD(TAG, "display_set_backlight(%d%%) skipped: LEDC not initialized", brightness_pct);
        return;
    }
    if (brightness_pct > 100) brightness_pct = 100;
    uint32_t duty = ((uint32_t)brightness_pct * 8191U) / 100U;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ESP_LOGD(TAG, "Backlight duty set to %d%% (raw: %lu)", brightness_pct, (unsigned long)duty);
}

esp_err_t display_subsystem_init(void)
{
    // SAFE-02: For Custom N16R8, backlight is managed by C++ Backlight class.
    // We pre-initialize LEDC here only if a CONFIG pin is defined, so that
    // any C code calling display_set_backlight() directly is safe.
#if defined(CONFIG_CUSTOM_DISPLAY_BACKLIGHT_GPIO) && (CONFIG_CUSTOM_DISPLAY_BACKLIGHT_GPIO >= 0)
    esp_err_t bl_ret = display_ledc_init((gpio_num_t)CONFIG_CUSTOM_DISPLAY_BACKLIGHT_GPIO);
    if (bl_ret != ESP_OK) {
        ESP_LOGW(TAG, "Backlight LEDC init warning: %s (non-critical)", esp_err_to_name(bl_ret));
    }
#else
    ESP_LOGI(TAG, "Backlight GPIO not configured — LEDC init deferred to C++ Display driver.");
#endif
    ESP_LOGI(TAG, "Display subsystem verified (SPI/I2C/LEDC coordinated by C++ Display driver).");
    return ESP_OK;
}

esp_err_t display_init(void)
{
    return display_subsystem_init();
}
