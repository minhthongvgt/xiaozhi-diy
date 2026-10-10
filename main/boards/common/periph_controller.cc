#include "periph_controller.h"
#include "drivers/periph/rtc_ext.h"
#include "drivers/periph/pn532.h"
#include "drivers/periph/pca9685.h"
#include "drivers/periph/led_ic.h"
#include <esp_log.h>
#include <sdkconfig.h>
#include <sys/time.h>
#include <cstdio>
#include <ctime>
#include <string>

#define TAG "PeriphController"

namespace {
constexpr int kMinValidYear = 124;
[[maybe_unused]] constexpr int64_t kRtcWriteIntervalUs = 6LL * 3600 * 1000 * 1000;
rtc_ext_handle_t s_rtc = nullptr;
[[maybe_unused]] pn532_handle_t s_nfc = nullptr;
[[maybe_unused]] pca9685_handle_t s_pwm = nullptr;
[[maybe_unused]] led_ic_handle_t s_led_ic = nullptr;

[[maybe_unused]] nlohmann::json ErrorJson(const char* message, esp_err_t err) {
    nlohmann::json j;
    j["success"] = false;
    j["error"] = message;
    j["code"] = esp_err_to_name(err);
    return j;
}

[[maybe_unused]] std::string ToHex(const uint8_t* data, size_t len) {
    std::string out;
    char buf[3];
    for (size_t i = 0; i < len; i++) {
        snprintf(buf, sizeof(buf), "%02X", data[i]);
        out += buf;
    }
    return out;
}
}

PeripheralController::PeripheralController(i2c_master_bus_handle_t i2c_bus) {
    if (i2c_bus == nullptr) {
        ESP_LOGW(TAG, "I2C bus unavailable, I2C peripherals (RTC/NFC/PWM/LED IC) skipped");
        return;
    }
    InitRtc(i2c_bus);
    InitNfc(i2c_bus);
    InitPwm(i2c_bus);
    InitLedIc(i2c_bus);
}

void PeripheralController::InitRtc(i2c_master_bus_handle_t i2c_bus) {
#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_RTC)
    if (rtc_ext_init(i2c_bus, &s_rtc) != ESP_OK) {
        ESP_LOGW(TAG, "RTC enabled but no DS3231/PCF8563 found on the I2C bus");
        return;
    }
    struct tm rtc_time = {};
    bool valid = false;
    time_t now = time(nullptr);
    struct tm sys_time = {};
    localtime_r(&now, &sys_time);
    if (rtc_ext_get_time(s_rtc, &rtc_time, &valid) == ESP_OK && valid && sys_time.tm_year < kMinValidYear &&
        rtc_time.tm_year >= kMinValidYear) {
        struct timeval tv = {.tv_sec = mktime(&rtc_time), .tv_usec = 0};
        settimeofday(&tv, nullptr);
        ESP_LOGI(TAG, "System time restored from RTC: %04d-%02d-%02d %02d:%02d:%02d", rtc_time.tm_year + 1900,
                 rtc_time.tm_mon + 1, rtc_time.tm_mday, rtc_time.tm_hour, rtc_time.tm_min, rtc_time.tm_sec);
    } else if (!valid) {
        ESP_LOGW(TAG, "RTC time is not valid yet (battery lost or never set); it will be set after network time sync");
    }
    esp_timer_create_args_t timer_args = {
        .callback = &PeripheralController::RtcSyncTimerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "rtc_sync",
        .skip_unhandled_events = true,
    };
    if (esp_timer_create(&timer_args, &rtc_sync_timer_) == ESP_OK) {
        esp_timer_start_periodic(rtc_sync_timer_, 60LL * 1000 * 1000);
    }

    auto& mcp_server = McpServer::GetInstance();
    mcp_server.AddTool("self.rtc.get_time",
                       "Read the date and time stored in the external real-time clock (DS3231 / PCF8563), which keeps time "
                       "even without network or power. 'valid' is false if the clock lost its backup battery and has not "
                       "been set again. temperature_c is only reported by DS3231.",
                       PropertyList(), [](const PropertyList& properties) -> ReturnValue {
        struct tm t = {};
        bool valid = false;
        esp_err_t ret = rtc_ext_get_time(s_rtc, &t, &valid);
        if (ret != ESP_OK) {
            return ErrorJson("Failed to read RTC", ret);
        }
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
        nlohmann::json j;
        j["rtc_type"] = rtc_ext_type_name(rtc_ext_get_type(s_rtc));
        j["time"] = buf;
        j["valid"] = valid;
        float temp = 0.0f;
        if (rtc_ext_get_temperature(s_rtc, &temp) == ESP_OK) {
            j["temperature_c"] = temp;
        }
        return j;
    });
    mcp_server.AddTool("self.rtc.sync_from_system",
                       "Write the current system time (obtained from the server) into the external real-time clock. Use it "
                       "when the user says the clock is wrong. Fails if the device has not received network time yet.",
                       PropertyList(), [this](const PropertyList& properties) -> ReturnValue {
        time_t now = time(nullptr);
        struct tm t = {};
        localtime_r(&now, &t);
        if (t.tm_year < kMinValidYear) {
            return ErrorJson("System time is not synchronized yet", ESP_ERR_INVALID_STATE);
        }
        SyncRtcFromSystem(true);
        nlohmann::json j;
        j["success"] = true;
        return j;
    });
    ESP_LOGI(TAG, "RTC (%s) MCP tools registered", rtc_ext_type_name(rtc_ext_get_type(s_rtc)));
#else
    (void)i2c_bus;
#endif
}

void PeripheralController::RtcSyncTimerCallback(void* arg) {
    static_cast<PeripheralController*>(arg)->SyncRtcFromSystem(false);
}

void PeripheralController::SyncRtcFromSystem(bool force) {
    if (s_rtc == nullptr) {
        return;
    }
    time_t now = time(nullptr);
    struct tm t = {};
    localtime_r(&now, &t);
    if (t.tm_year < kMinValidYear) {
        return;
    }
    int64_t now_us = esp_timer_get_time();
    if (!force && last_rtc_write_us_ != 0 && (now_us - last_rtc_write_us_) < kRtcWriteIntervalUs) {
        return;
    }
    esp_err_t ret = rtc_ext_set_time(s_rtc, &t);
    if (ret == ESP_OK) {
        last_rtc_write_us_ = now_us;
        ESP_LOGI(TAG, "RTC updated from system time: %04d-%02d-%02d %02d:%02d:%02d", t.tm_year + 1900, t.tm_mon + 1,
                 t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    } else {
        ESP_LOGW(TAG, "Failed to write RTC: %s", esp_err_to_name(ret));
    }
}

void PeripheralController::InitNfc(i2c_master_bus_handle_t i2c_bus) {
#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_NFC_PN532)
    pn532_config_t cfg = {
        .i2c_bus = i2c_bus,
#if defined(CONFIG_CUSTOM_PERIPH_NFC_IRQ_PIN) && (CONFIG_CUSTOM_PERIPH_NFC_IRQ_PIN >= 0)
        .irq_pin = static_cast<gpio_num_t>(CONFIG_CUSTOM_PERIPH_NFC_IRQ_PIN),
#else
        .irq_pin = static_cast<gpio_num_t>(-1),
#endif
#if defined(CONFIG_CUSTOM_PERIPH_NFC_RST_PIN) && (CONFIG_CUSTOM_PERIPH_NFC_RST_PIN >= 0)
        .rst_pin = static_cast<gpio_num_t>(CONFIG_CUSTOM_PERIPH_NFC_RST_PIN),
#else
        .rst_pin = static_cast<gpio_num_t>(-1),
#endif
    };
    if (pn532_init(&cfg, &s_nfc) != ESP_OK) {
        ESP_LOGW(TAG, "NFC enabled but PN532 did not respond (check I2C mode switches and wiring)");
        return;
    }
    auto& mcp_server = McpServer::GetInstance();
    mcp_server.AddTool("self.nfc.read_card",
                       "Wait for an NFC / RFID card or tag (ISO14443A: MIFARE, NTAG, contactless cards) to be held near "
                       "the PN532 reader and return its UID and type. Ask the user to tap the card on the reader before "
                       "calling. 'found' is false if no card appeared before the timeout.",
                       PropertyList({Property("timeout_ms", kPropertyTypeInteger, 5000, 500, 15000)}),
                       [](const PropertyList& properties) -> ReturnValue {
        int timeout_ms = properties["timeout_ms"].value<int>();
        pn532_card_t card = {};
        esp_err_t ret = pn532_read_passive_target(s_nfc, static_cast<uint32_t>(timeout_ms), &card);
        nlohmann::json j;
        j["found"] = (ret == ESP_OK);
        if (ret == ESP_OK) {
            char buf[8];
            j["uid"] = ToHex(card.uid, card.uid_len);
            j["uid_length"] = card.uid_len;
            snprintf(buf, sizeof(buf), "%04X", card.atqa);
            j["atqa"] = buf;
            snprintf(buf, sizeof(buf), "%02X", card.sak);
            j["sak"] = buf;
            j["card_type"] = pn532_card_type_name(&card);
        }
        return j;
    });
    ESP_LOGI(TAG, "NFC PN532 MCP tool registered");
#else
    (void)i2c_bus;
#endif
}

void PeripheralController::InitPwm(i2c_master_bus_handle_t i2c_bus) {
#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_PCA9685)
    pca9685_config_t cfg = {
        .i2c_bus = i2c_bus,
        .i2c_addr = PCA9685_I2C_ADDR_DEFAULT,
#if defined(CONFIG_CUSTOM_PERIPH_PCA9685_OE_PIN) && (CONFIG_CUSTOM_PERIPH_PCA9685_OE_PIN >= 0)
        .oe_pin = static_cast<gpio_num_t>(CONFIG_CUSTOM_PERIPH_PCA9685_OE_PIN),
#else
        .oe_pin = static_cast<gpio_num_t>(-1),
#endif
        .pwm_freq_hz = 50,
    };
    if (pca9685_init(&cfg, &s_pwm) != ESP_OK) {
        ESP_LOGW(TAG, "PCA9685 enabled but not found at I2C address 0x%02X", PCA9685_I2C_ADDR_DEFAULT);
        return;
    }
    auto& mcp_server = McpServer::GetInstance();
    mcp_server.AddTool("self.pwm.set_servo",
                       "Move a servo connected to the PCA9685 16-channel PWM board. channel is the output number printed on "
                       "the board (0-15), angle is 0-180 degrees (90 = center).",
                       PropertyList({Property("channel", kPropertyTypeInteger, 0, 0, 15),
                                     Property("angle", kPropertyTypeInteger, 90, 0, 180)}),
                       [](const PropertyList& properties) -> ReturnValue {
        int channel = properties["channel"].value<int>();
        int angle = properties["angle"].value<int>();
        esp_err_t ret = pca9685_set_servo_angle(s_pwm, static_cast<uint8_t>(channel), static_cast<float>(angle), 500, 2500);
        if (ret != ESP_OK) {
            return ErrorJson("Failed to set servo", ret);
        }
        return true;
    });
    mcp_server.AddTool("self.pwm.set_duty",
                       "Set the PWM duty cycle (0-100 percent) of one PCA9685 channel (0-15), for dimming LEDs or driving "
                       "small loads. 0 turns the channel fully off, 100 fully on. Note: the board runs at 50 Hz for servos.",
                       PropertyList({Property("channel", kPropertyTypeInteger, 0, 0, 15),
                                     Property("percent", kPropertyTypeInteger, 0, 0, 100)}),
                       [](const PropertyList& properties) -> ReturnValue {
        int channel = properties["channel"].value<int>();
        int percent = properties["percent"].value<int>();
        esp_err_t ret = pca9685_set_duty(s_pwm, static_cast<uint8_t>(channel), static_cast<float>(percent));
        if (ret != ESP_OK) {
            return ErrorJson("Failed to set PWM duty", ret);
        }
        return true;
    });
    mcp_server.AddTool("self.pwm.all_off", "Turn off all 16 PCA9685 outputs (servos go limp, LEDs off).",
                       PropertyList(), [](const PropertyList& properties) -> ReturnValue {
        esp_err_t ret = pca9685_all_off(s_pwm);
        if (ret != ESP_OK) {
            return ErrorJson("Failed to turn off PWM outputs", ret);
        }
        return true;
    });
    ESP_LOGI(TAG, "PCA9685 MCP tools registered");
#else
    (void)i2c_bus;
#endif
}

void PeripheralController::InitLedIc(i2c_master_bus_handle_t i2c_bus) {
#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_LED_DRIVER_IC)
#if defined(CONFIG_ENABLE_CUSTOM_DISPLAY) && (defined(CONFIG_CUSTOM_DISPLAY_OLED_SSD1306) || defined(CONFIG_CUSTOM_DISPLAY_OLED_SH1106))
    bool allow_oled_addresses = false;
#else
    bool allow_oled_addresses = true;
#endif
    led_ic_config_t cfg = {
        .i2c_bus = i2c_bus,
        .allow_oled_addresses = allow_oled_addresses,
    };
    if (led_ic_init(&cfg, &s_led_ic) != ESP_OK) {
        ESP_LOGW(TAG, "LED driver IC enabled but no AW9523B / IS31FL3236 found");
        return;
    }
    auto& mcp_server = McpServer::GetInstance();
    std::string desc = std::string("Set the brightness (0-255) of one LED output on the ") +
                       led_ic_type_name(led_ic_get_type(s_led_ic)) + " LED driver. Valid channels: 0-" +
                       std::to_string(led_ic_get_channel_count(s_led_ic) - 1) + ".";
    mcp_server.AddTool("self.led_ic.set_channel", desc,
                       PropertyList({Property("channel", kPropertyTypeInteger, 0, 0, 35),
                                     Property("brightness", kPropertyTypeInteger, 255, 0, 255)}),
                       [](const PropertyList& properties) -> ReturnValue {
        int channel = properties["channel"].value<int>();
        int brightness = properties["brightness"].value<int>();
        if (channel >= led_ic_get_channel_count(s_led_ic)) {
            return ErrorJson("Channel out of range for this LED driver", ESP_ERR_INVALID_ARG);
        }
        esp_err_t ret = led_ic_set_channel(s_led_ic, static_cast<uint8_t>(channel), static_cast<uint8_t>(brightness));
        if (ret != ESP_OK) {
            return ErrorJson("Failed to set LED channel", ret);
        }
        return true;
    });
    mcp_server.AddTool("self.led_ic.set_all",
                       "Set the same brightness (0-255) on every output of the LED driver IC. 0 turns all LEDs off.",
                       PropertyList({Property("brightness", kPropertyTypeInteger, 0, 0, 255)}),
                       [](const PropertyList& properties) -> ReturnValue {
        int brightness = properties["brightness"].value<int>();
        esp_err_t ret = led_ic_set_all(s_led_ic, static_cast<uint8_t>(brightness));
        if (ret != ESP_OK) {
            return ErrorJson("Failed to set LEDs", ret);
        }
        return true;
    });
    ESP_LOGI(TAG, "LED driver IC (%s) MCP tools registered", led_ic_type_name(led_ic_get_type(s_led_ic)));
#else
    (void)i2c_bus;
#endif
}
