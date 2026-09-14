/**
 * @file gpio_validator.c
 * @brief ESP32-S3 N16R8 Hardware GPIO Safety Validator Implementation
 */

#include "gpio_validator.h"
#include <sdkconfig.h>
#include <esp_log.h>

#define TAG "GPIO_Validator"

typedef enum {
    BUS_TYPE_EXCLUSIVE = 0,
    BUS_TYPE_I2C_SDA,
    BUS_TYPE_I2C_SCL,
    BUS_TYPE_I2S_BCLK,
    BUS_TYPE_I2S_WS,
} pin_bus_type_t;

typedef struct {
    int pin;
    const char *name;
    pin_bus_type_t bus_type;
    bool is_active;
} configured_pin_t;

bool gpio_is_pin_safe(gpio_num_t pin, const char* periph_name)
{
    if (pin < 0) {
        return true; // NC (-1) is not connected, always safe
    }

#if defined(CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM) || defined(CONFIG_IDF_TARGET_ESP32S3)
    if (ESP32S3_N16R8_IS_RESERVED_PIN(pin)) {
        ESP_LOGE(TAG, "[CRITICAL] Peripheral '%s' assigned to GPIO %d which is in forbidden Octal PSRAM/Flash range [26..37]!",
                 periph_name ? periph_name : "Unknown", pin);
        return false;
    }
#endif

    // Check strapping / USB pins and warn
    if (pin == 19 || pin == 20) {
        ESP_LOGW(TAG, "[WARNING] Peripheral '%s' assigned to GPIO %d (USB D+/D-). USB CDC/JTAG debugging will be impaired.",
                 periph_name ? periph_name : "Unknown", pin);
    } else if (pin == 0) {
        ESP_LOGW(TAG, "[NOTICE] Peripheral '%s' assigned to GPIO 0 (BOOT Strapping Pin).",
                 periph_name ? periph_name : "Unknown");
    }

    return true;
}

esp_err_t gpio_safety_validate(void)
{
    ESP_LOGI(TAG, "Starting Hardware Safety Validation for ESP32-S3 N16R8...");

    configured_pin_t pins[] = {
        // 1. Display pins
#if defined(CONFIG_ENABLE_CUSTOM_DISPLAY)
#if defined(CONFIG_CUSTOM_DISPLAY_PIN_MOSI)
        { CONFIG_CUSTOM_DISPLAY_PIN_MOSI, "Display SPI MOSI", BUS_TYPE_EXCLUSIVE, true },
#endif
#if defined(CONFIG_CUSTOM_DISPLAY_PIN_CLK)
        { CONFIG_CUSTOM_DISPLAY_PIN_CLK, "Display SPI CLK", BUS_TYPE_EXCLUSIVE, true },
#endif
#if defined(CONFIG_CUSTOM_DISPLAY_PIN_CS)
        { CONFIG_CUSTOM_DISPLAY_PIN_CS, "Display CS", BUS_TYPE_EXCLUSIVE, true },
#endif
#if defined(CONFIG_CUSTOM_DISPLAY_PIN_DC)
        { CONFIG_CUSTOM_DISPLAY_PIN_DC, "Display DC", BUS_TYPE_EXCLUSIVE, true },
#endif
#if defined(CONFIG_CUSTOM_DISPLAY_PIN_RST)
        { CONFIG_CUSTOM_DISPLAY_PIN_RST, "Display RST", BUS_TYPE_EXCLUSIVE, true },
#endif
#if defined(CONFIG_CUSTOM_DISPLAY_PIN_BLK)
        { CONFIG_CUSTOM_DISPLAY_PIN_BLK, "Display Backlight", BUS_TYPE_EXCLUSIVE, true },
#endif
#endif

        // 2. Audio Speaker
#if defined(CONFIG_ENABLE_CUSTOM_SPEAKER)
#if defined(CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_BCLK)
        { CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_BCLK, "Audio Speaker I2S BCLK", BUS_TYPE_I2S_BCLK, true },
#endif
#if defined(CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_LRCK)
        { CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_LRCK, "Audio Speaker I2S LRCK", BUS_TYPE_I2S_WS, true },
#endif
#if defined(CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_DOUT)
        { CONFIG_CUSTOM_AUDIO_I2S_SPK_GPIO_DOUT, "Audio Speaker I2S DOUT", BUS_TYPE_EXCLUSIVE, true },
#endif
#endif

        // 3. Audio Microphone
#if defined(CONFIG_ENABLE_CUSTOM_MIC)
#if defined(CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_SCK)
        { CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_SCK, "Audio Mic I2S SCK",
#if defined(CONFIG_CUSTOM_AUDIO_I2S_DUPLEX)
          BUS_TYPE_I2S_BCLK,
#else
          BUS_TYPE_EXCLUSIVE,
#endif
          true },
#endif
#if defined(CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_WS)
        { CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_WS, "Audio Mic I2S WS",
#if defined(CONFIG_CUSTOM_AUDIO_I2S_DUPLEX)
          BUS_TYPE_I2S_WS,
#else
          BUS_TYPE_EXCLUSIVE,
#endif
          true },
#endif
#if defined(CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_DIN)
        { CONFIG_CUSTOM_AUDIO_I2S_MIC_GPIO_DIN, "Audio Mic I2S DIN", BUS_TYPE_EXCLUSIVE, true },
#endif
#endif

        // 4. I2C Bus Master
#if defined(CONFIG_CUSTOM_I2C_SDA_PIN)
        { CONFIG_CUSTOM_I2C_SDA_PIN, "I2C SDA Bus", BUS_TYPE_I2C_SDA, true },
#endif
#if defined(CONFIG_CUSTOM_I2C_SCL_PIN)
        { CONFIG_CUSTOM_I2C_SCL_PIN, "I2C SCL Bus", BUS_TYPE_I2C_SCL, true },
#endif

        // 5. Buttons
#if defined(CONFIG_CUSTOM_ENABLE_BUTTON_BOOT) && defined(CONFIG_CUSTOM_BUTTON_BOOT_GPIO)
        { CONFIG_CUSTOM_BUTTON_BOOT_GPIO, "Button Boot", BUS_TYPE_EXCLUSIVE, true },
#endif
#if defined(CONFIG_CUSTOM_ENABLE_BUTTON_TOUCH) && defined(CONFIG_CUSTOM_BUTTON_TOUCH_GPIO)
        { CONFIG_CUSTOM_BUTTON_TOUCH_GPIO, "Button Touch", BUS_TYPE_EXCLUSIVE, true },
#endif
#if defined(CONFIG_CUSTOM_ENABLE_BUTTON_VOLUME)
#if defined(CONFIG_CUSTOM_BUTTON_VOLUME_UP_GPIO)
        { CONFIG_CUSTOM_BUTTON_VOLUME_UP_GPIO, "Button Volume UP", BUS_TYPE_EXCLUSIVE, true },
#endif
#if defined(CONFIG_CUSTOM_BUTTON_VOLUME_DOWN_GPIO)
        { CONFIG_CUSTOM_BUTTON_VOLUME_DOWN_GPIO, "Button Volume DOWN", BUS_TYPE_EXCLUSIVE, true },
#endif
#endif

        // 6. LEDs
#if defined(CONFIG_ENABLE_CUSTOM_LEDS)
#if defined(CONFIG_CUSTOM_LED_WS2812_GPIO)
        { CONFIG_CUSTOM_LED_WS2812_GPIO, "LED WS2812 RGB", BUS_TYPE_EXCLUSIVE, true },
#endif
#if defined(CONFIG_CUSTOM_LED_SINGLE_PWM_GPIO)
        { CONFIG_CUSTOM_LED_SINGLE_PWM_GPIO, "LED Single PWM", BUS_TYPE_EXCLUSIVE, true },
#endif
#endif

        // 7. Relay & Actuators
#if defined(CONFIG_CUSTOM_PERIPH_RELAY_ENABLE) && defined(CONFIG_CUSTOM_PERIPH_RELAY_GPIO)
        { CONFIG_CUSTOM_PERIPH_RELAY_GPIO, "Relay Control", BUS_TYPE_EXCLUSIVE, true },
#endif
#if defined(CONFIG_CUSTOM_ENABLE_SERVO_DOG) && defined(CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO)
        { CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO, "Servo Dog PWM", BUS_TYPE_EXCLUSIVE, true },
#endif
    };

    size_t count = sizeof(pins) / sizeof(pins[0]);
    bool has_error = false;

    // Step 1: Check forbidden ranges
    for (size_t i = 0; i < count; i++) {
        if (!pins[i].is_active || pins[i].pin < 0) {
            continue;
        }
        if (!gpio_is_pin_safe((gpio_num_t)pins[i].pin, pins[i].name)) {
            has_error = true;
        }
    }

    // Step 2: Check duplicate conflicts
    for (size_t i = 0; i < count; i++) {
        if (!pins[i].is_active || pins[i].pin < 0) continue;

        for (size_t j = i + 1; j < count; j++) {
            if (!pins[j].is_active || pins[j].pin < 0) continue;

            if (pins[i].pin == pins[j].pin) {
                // Check if this sharing is allowed
                bool allowed = false;
                if (pins[i].bus_type == BUS_TYPE_I2C_SDA && pins[j].bus_type == BUS_TYPE_I2C_SDA) {
                    allowed = true;
                } else if (pins[i].bus_type == BUS_TYPE_I2C_SCL && pins[j].bus_type == BUS_TYPE_I2C_SCL) {
                    allowed = true;
                } else if (pins[i].bus_type == BUS_TYPE_I2S_BCLK && pins[j].bus_type == BUS_TYPE_I2S_BCLK) {
                    allowed = true;
                } else if (pins[i].bus_type == BUS_TYPE_I2S_WS && pins[j].bus_type == BUS_TYPE_I2S_WS) {
                    allowed = true;
                }

                if (!allowed) {
                    ESP_LOGE(TAG, "[PIN CONFLICT] GPIO %d is concurrently claimed by '%s' and '%s'!",
                             pins[i].pin, pins[i].name, pins[j].name);
                    has_error = true;
                }
            }
        }
    }

    if (has_error) {
        ESP_LOGE(TAG, "Hardware Safety Validation FAILED! System boot aborted.");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Hardware Safety Validation PASSED: All assigned GPIOs are safe and collision-free.");
    return ESP_OK;
}

esp_err_t gpio_validator_run(void)
{
    return gpio_safety_validate();
}

const char* gpio_validator_get_error_log(void)
{
    return "Check your menuconfig or board pinout. Pins in range 26..37 or duplicate pin assignments detected.";
}
