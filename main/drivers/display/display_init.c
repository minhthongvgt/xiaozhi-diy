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

void display_set_backlight(uint8_t brightness_pct)
{
    if (s_bl_pin == GPIO_NUM_NC) return;
    uint32_t duty = (brightness_pct * 8191) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

esp_err_t display_subsystem_init(void)
{
#if !defined(CONFIG_ENABLE_CUSTOM_DISPLAY)
    ESP_LOGI(TAG, "Display is disabled in configuration.");
    return ESP_OK;
#else
    ESP_LOGI(TAG, "Initializing Display Subsystem...");

    // SPI Display Bus Initialization
#if defined(CONFIG_CUSTOM_DISPLAY_ST7796) || defined(CONFIG_CUSTOM_DISPLAY_ST7789) || \
    defined(CONFIG_CUSTOM_DISPLAY_ILI9341) || defined(CONFIG_CUSTOM_DISPLAY_GC9A01)

    gpio_num_t mosi_pin = GPIO_NUM_47;
    gpio_num_t sclk_pin = GPIO_NUM_21;
#if defined(CONFIG_CUSTOM_DISPLAY_PIN_MOSI)
    mosi_pin = (gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_MOSI;
#endif
#if defined(CONFIG_CUSTOM_DISPLAY_PIN_CLK)
    sclk_pin = (gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_CLK;
#endif

    esp_err_t ret = bus_manager_init_spi(SPI2_HOST, mosi_pin, GPIO_NUM_NC, sclk_pin, 320 * 480 * 2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Display SPI Bus: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "Display SPI Bus initialized on MOSI: %d, SCLK: %d", mosi_pin, sclk_pin);
#endif

    // Backlight Control (LEDC PWM)
#if defined(CONFIG_CUSTOM_DISPLAY_PIN_BLK)
    s_bl_pin = (gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_BLK;
    if (s_bl_pin >= 0) {
        ledc_timer_config_t ledc_timer = {
            .speed_mode       = LEDC_LOW_SPEED_MODE,
            .timer_num        = LEDC_TIMER_0,
            .duty_resolution  = LEDC_TIMER_13_BIT,
            .freq_hz          = 5000,
            .clk_cfg          = LEDC_AUTO_CLK
        };
        ledc_timer_config(&ledc_timer);

        ledc_channel_config_t ledc_channel = {
            .speed_mode     = LEDC_LOW_SPEED_MODE,
            .channel        = LEDC_CHANNEL_0,
            .timer_sel      = LEDC_TIMER_0,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = s_bl_pin,
            .duty           = 8191, // 100% brightness default
            .hpoint         = 0
        };
        ledc_channel_config(&ledc_channel);
        ESP_LOGI(TAG, "Display Backlight initialized via LEDC PWM on GPIO %d", s_bl_pin);
    }
#endif

    ESP_LOGI(TAG, "Display subsystem initialized successfully.");
    return ESP_OK;
#endif
}

esp_err_t display_init(void)
{
    return display_subsystem_init();
}
