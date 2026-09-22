#include "wifi_board.h"
#include "audio/codecs/no_audio_codec.h"
#include "audio/codecs/es8311_audio_codec.h"
#include "audio/codecs/es8388_audio_codec.h"
#if defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8374)
#include "audio/codecs/es8374_audio_codec.h"
#endif
#if defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8389)
#include "audio/codecs/es8389_audio_codec.h"
#endif
#include "display/lcd_display.h"
#include "display/oled_display.h"
#include "display/uart_display.h"
#include "display/display.h"
#if defined(CONFIG_CUSTOM_AUDIO_CODEC_BOX)
#include "audio/codecs/box_audio_codec.h"
#endif
#include "system_reset.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "mcp_server.h"
#include "lamp_controller.h"
#include "sensor_controller.h"
#include "actuator_controller.h"
#include "led/single_led.h"
#include "led/circular_strip.h"
#include "led/gpio_led.h"
#include "backlight.h"
#include "assets/lang_config.h"

#if CONFIG_IDF_TARGET_ESP32S3
#include "boards/common/esp32_camera.h"
#endif

#if defined(CONFIG_CUSTOM_BATTERY_MONITOR_ADC)
#include "boards/common/adc_battery_monitor.h"
#endif
#include "boards/common/bus_manager.h"

#include <esp_log.h>
#include <driver/i2c_master.h>
#include <driver/spi_common.h>
#include <driver/uart.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>

#include <esp_lcd_touch.h>
#include <esp_lvgl_port.h>

#if defined(CONFIG_CUSTOM_DISPLAY_ILI9341)
#include "esp_lcd_ili9341.h"
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_GC9A01) || defined(CONFIG_CUSTOM_DISPLAY_GC9107)
#include "esp_lcd_gc9a01.h"
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_ST7796)
#include "esp_lcd_st7796.h"
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_ILI9486)
#include "esp_lcd_ili9486.h"
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_NV3023)
#include "esp_lcd_nv3030b.h"
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_JD9853)
#include "esp_lcd_jd9853.h"
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_OLED_SH1106)
#include <esp_lcd_panel_sh1106.h>
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_GC9107)
static const gc9a01_lcd_init_cmd_t gc9107_lcd_init_cmds[] = {
    //  {cmd, { data }, data_size, delay_ms}
    {0xfe, (uint8_t[]){0x00}, 0, 0},
    {0xef, (uint8_t[]){0x00}, 0, 0},
    {0xb0, (uint8_t[]){0xc0}, 1, 0},
    {0xb1, (uint8_t[]){0x80}, 1, 0},
    {0xb2, (uint8_t[]){0x27}, 1, 0},
    {0xb3, (uint8_t[]){0x13}, 1, 0},
    {0xb6, (uint8_t[]){0x19}, 1, 0},
    {0xb7, (uint8_t[]){0x05}, 1, 0},
    {0xac, (uint8_t[]){0xc8}, 1, 0},
    {0xab, (uint8_t[]){0x0f}, 1, 0},
    {0x3a, (uint8_t[]){0x05}, 1, 0},
    {0xb4, (uint8_t[]){0x04}, 1, 0},
    {0xa8, (uint8_t[]){0x08}, 1, 0},
    {0xb8, (uint8_t[]){0x08}, 1, 0},
    {0xea, (uint8_t[]){0x02}, 1, 0},
    {0xe8, (uint8_t[]){0x2A}, 1, 0},
    {0xe9, (uint8_t[]){0x47}, 1, 0},
    {0xe7, (uint8_t[]){0x5f}, 1, 0},
    {0xc6, (uint8_t[]){0x21}, 1, 0},
    {0xc7, (uint8_t[]){0x15}, 1, 0},
    {0xf0,
    (uint8_t[]){0x1D, 0x38, 0x09, 0x4D, 0x92, 0x2F, 0x35, 0x52, 0x1E, 0x0C,
                0x04, 0x12, 0x14, 0x1f},
    14, 0},
    {0xf1,
    (uint8_t[]){0x16, 0x40, 0x1C, 0x54, 0xA9, 0x2D, 0x2E, 0x56, 0x10, 0x0D,
                0x0C, 0x1A, 0x14, 0x1E},
    14, 0},
    {0xf4, (uint8_t[]){0x00, 0x00, 0xFF}, 3, 0},
    {0xba, (uint8_t[]){0xFF, 0xFF}, 2, 0},
};
#endif

#if defined(CONFIG_CUSTOM_TOUCH_CST816S)
#include <esp_lcd_touch_cst816s.h>
#elif defined(CONFIG_CUSTOM_TOUCH_GT911)
#include <esp_lcd_touch_gt911.h>
#elif defined(CONFIG_CUSTOM_TOUCH_FT5X06)
#include <esp_lcd_touch_ft5x06.h>
#endif

#define TAG "CustomN16R8Board"

class CustomN16R8Board : public WifiBoard {
private:
    i2c_master_bus_handle_t i2c_bus_ = nullptr;
    Display* display_ = nullptr;
    Backlight* backlight_ = nullptr;
    Button boot_button_;
    Button touch_button_;
    Button volume_up_button_;
    Button volume_down_button_;
    esp_lcd_touch_handle_t touch_handle_ = nullptr;

#if CONFIG_IDF_TARGET_ESP32S3
    Camera* camera_ = nullptr;
#endif

#if defined(CONFIG_CUSTOM_BATTERY_MONITOR_ADC)
    AdcBatteryMonitor* battery_monitor_ = nullptr;
#endif

    void InitializeI2c() {
        if (i2c_bus_ != nullptr) {
            return;
        }

        i2c_master_bus_handle_t existing = bus_manager_get_i2c_bus();
        if (existing != nullptr) {
            i2c_bus_ = existing;
            ESP_LOGI(TAG, "Reusing shared I2C master bus from bus_manager");
            return;
        }

        gpio_num_t sda_pin = GPIO_NUM_8;
        gpio_num_t scl_pin = GPIO_NUM_9;

#if defined(CONFIG_ENABLE_CUSTOM_DISPLAY) && (defined(CONFIG_CUSTOM_DISPLAY_OLED_SSD1306) || defined(CONFIG_CUSTOM_DISPLAY_OLED_SH1106))
        sda_pin = DISPLAY_I2C_SDA_PIN;
        scl_pin = DISPLAY_I2C_SCL_PIN;
#elif defined(CONFIG_ENABLE_CUSTOM_TOUCH) && !defined(CONFIG_CUSTOM_TOUCH_NONE)
        sda_pin = TOUCH_I2C_SDA_PIN;
        scl_pin = TOUCH_I2C_SCL_PIN;
#elif (defined(CONFIG_ENABLE_CUSTOM_AUDIO) || defined(CONFIG_ENABLE_CUSTOM_SPEAKER) || defined(CONFIG_ENABLE_CUSTOM_MIC)) && \
      (defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8311) || defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8388) || \
       defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8374) || defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8389) || \
       defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8311) || defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8388) || \
       defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8374) || defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8389) || \
       defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8311) || defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8388) || \
       defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8374) || defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8389))
        sda_pin = AUDIO_CODEC_I2C_SDA_PIN;
        scl_pin = AUDIO_CODEC_I2C_SCL_PIN;
#endif

        esp_err_t ret = bus_manager_init_i2c(sda_pin, scl_pin, 400000);
        if (ret == ESP_OK) {
            i2c_bus_ = bus_manager_get_i2c_bus();
            ESP_LOGI(TAG, "I2C master bus initialized via bus_manager (SDA: %d, SCL: %d)", sda_pin, scl_pin);
        } else {
            ESP_LOGE(TAG, "Failed to initialize I2C master bus via bus_manager (SDA: %d, SCL: %d): %s",
                     sda_pin, scl_pin, esp_err_to_name(ret));
            i2c_bus_ = nullptr;
        }
    }

    void InitializeDisplay() {
#if !defined(CONFIG_ENABLE_CUSTOM_DISPLAY) || defined(CONFIG_CUSTOM_DISPLAY_NONE)
        ESP_LOGI(TAG, "Display disabled in configuration (Audio Only / Headless)");
        display_ = new NoDisplay();
        return;
#elif defined(CONFIG_CUSTOM_DISPLAY_UART)
        ESP_LOGI(TAG, "Initializing External UART Display (TX: %d, RX: %d, Baud: %d)",
                 DISPLAY_UART_TX_PIN, DISPLAY_UART_RX_PIN, DISPLAY_UART_BAUDRATE);
        UartDisplayProtocol proto = UartDisplayProtocol::NextionTjc;
#if defined(CONFIG_CUSTOM_DISPLAY_UART_PROTO_JSON)
        proto = UartDisplayProtocol::JsonStream;
#elif defined(CONFIG_CUSTOM_DISPLAY_UART_PROTO_RAW_TEXT)
        proto = UartDisplayProtocol::RawText;
#elif defined(CONFIG_CUSTOM_DISPLAY_UART_PROTO_DWIN)
        proto = UartDisplayProtocol::DwinDgus;
#endif
        display_ = new UartDisplay(DISPLAY_UART_PORT, DISPLAY_UART_TX_PIN, DISPLAY_UART_RX_PIN,
                                   DISPLAY_UART_BAUDRATE, proto);
        return;
#elif defined(CONFIG_CUSTOM_DISPLAY_EPAPER_SSD1680)
        ESP_LOGW(TAG, "E-Paper SSD1680 display driver is not yet implemented. Falling back to NoDisplay.");
        display_ = new NoDisplay();
        return;
#elif defined(CONFIG_CUSTOM_DISPLAY_ST7701)
        ESP_LOGW(TAG, "ST7701 RGB Interface display requires dedicated parallel RGB driver. Not yet implemented. Falling back to NoDisplay.");
        display_ = new NoDisplay();
        return;
#elif defined(CONFIG_CUSTOM_DISPLAY_QSPI_AMOLED)
        ESP_LOGW(TAG, "QSPI AMOLED display requires dedicated QSPI driver (SH8601/CO5300/SPD2010). Not yet implemented. Falling back to NoDisplay.");
        display_ = new NoDisplay();
        return;
#elif defined(CONFIG_CUSTOM_DISPLAY_OLED_SSD1306) || defined(CONFIG_CUSTOM_DISPLAY_OLED_SH1106)
        InitializeI2c();
        if (i2c_bus_ == nullptr) {
            ESP_LOGE(TAG, "I2C bus unavailable, falling back to NoDisplay");
            display_ = new NoDisplay();
            return;
        }

        uint32_t oled_width = DISPLAY_WIDTH;
        uint32_t oled_height = DISPLAY_HEIGHT;
        if (oled_width != 128 && oled_width != 64) {
            oled_width = 128;
        }
        if (oled_height != 32 && oled_height != 64) {
            oled_height = 64;
        }

        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        esp_lcd_panel_io_i2c_config_t io_config = {
            .dev_addr = 0x3C,
            .scl_speed_hz = 400 * 1000,
            .control_phase_bytes = 1,
            .dc_bit_offset = 6,
            .lcd_cmd_bits = 8,
            .lcd_param_bits = 8,
            .on_color_trans_done = nullptr,
            .user_ctx = nullptr,
            .flags = {
                .dc_low_on_data = 0,
                .disable_control_phase = 0,
            },
        };
        esp_err_t ret = esp_lcd_new_panel_io_i2c(i2c_bus_, &io_config, &panel_io);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create OLED panel IO: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_RST_PIN;
        panel_config.bits_per_pixel = 1;

        esp_lcd_panel_ssd1306_config_t ssd1306_config = {
            .height = static_cast<uint8_t>(oled_height),
        };
        panel_config.vendor_config = &ssd1306_config;

#if defined(CONFIG_CUSTOM_DISPLAY_OLED_SH1106)
        ret = esp_lcd_new_panel_sh1106(panel_io, &panel_config, &panel);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create SH1106 panel: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }
        ESP_LOGI(TAG, "SH1106 OLED panel created");
#else
        ret = esp_lcd_new_panel_ssd1306(panel_io, &panel_config, &panel);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create SSD1306 panel: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }
        ESP_LOGI(TAG, "SSD1306 OLED panel created");
#endif

        esp_lcd_panel_reset(panel);
        ret = esp_lcd_panel_init(panel);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize OLED panel: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }
        esp_lcd_panel_invert_color(panel, DISPLAY_INVERT_COLOR);
        esp_lcd_panel_disp_on_off(panel, true);

        display_ = new OledDisplay(panel_io, panel, oled_width, oled_height, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
#else
        // SPI LCD display (ST7789, ST7796, ST7735, ILI9341, ILI9486, GC9A01, GC9107, NV3023, JD9853...)
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_MOSI_PIN;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_CLK_PIN;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        // Allocate enough for LVGL partial refresh lines; capped to prevent DMA exhaustion
        size_t line_buffer_sz = (size_t)DISPLAY_WIDTH * 40 * sizeof(uint16_t);
        if (line_buffer_sz < 4092) line_buffer_sz = 4092;
        if (line_buffer_sz > 65536) line_buffer_sz = 65536;
        buscfg.max_transfer_sz = (int)line_buffer_sz;

        esp_err_t ret = spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
            ESP_LOGE(TAG, "Failed to initialize SPI3 bus: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }

        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_CS_PIN;
        io_config.dc_gpio_num = DISPLAY_DC_PIN;
        io_config.spi_mode = 0;
#if defined(CONFIG_CUSTOM_DISPLAY_ST7735)
        io_config.pclk_hz = 20 * 1000 * 1000;
#elif defined(CONFIG_CUSTOM_DISPLAY_ILI9486)
        io_config.pclk_hz = 26 * 1000 * 1000;
#else
        io_config.pclk_hz = 40 * 1000 * 1000;
#endif
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ret = esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &panel_io);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create SPI panel IO: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_RST_PIN;
#if defined(CONFIG_CUSTOM_DISPLAY_ST7735)
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR;
#else
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
#endif
        panel_config.bits_per_pixel = 16;

#if defined(CONFIG_CUSTOM_DISPLAY_ILI9341)
        ret = esp_lcd_new_panel_ili9341(panel_io, &panel_config, &panel);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create ILI9341 panel: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }
        ESP_LOGI(TAG, "ILI9341 SPI LCD panel created");
#elif defined(CONFIG_CUSTOM_DISPLAY_ILI9486)
        ret = esp_lcd_new_panel_ili9486(panel_io, &panel_config, &panel);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create ILI9486 panel: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }
        ESP_LOGI(TAG, "ILI9486 SPI LCD panel created");
#elif defined(CONFIG_CUSTOM_DISPLAY_GC9A01)
        ret = esp_lcd_new_panel_gc9a01(panel_io, &panel_config, &panel);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create GC9A01 panel: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }
        ESP_LOGI(TAG, "GC9A01 Round LCD panel created");
#elif defined(CONFIG_CUSTOM_DISPLAY_GC9107)
        gc9a01_vendor_config_t gc9107_vendor_config = {
            .init_cmds = gc9107_lcd_init_cmds,
            .init_cmds_size = sizeof(gc9107_lcd_init_cmds) / sizeof(gc9a01_lcd_init_cmd_t),
        };
        panel_config.vendor_config = &gc9107_vendor_config;
        ret = esp_lcd_new_panel_gc9a01(panel_io, &panel_config, &panel);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create GC9107 panel: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }
        ESP_LOGI(TAG, "GC9107 Round LCD panel created");
#elif defined(CONFIG_CUSTOM_DISPLAY_NV3023)
        ret = esp_lcd_new_panel_nv3030b(panel_io, &panel_config, &panel);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create NV3023/NV3030B panel: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }
        ESP_LOGI(TAG, "NV3023/NV3030B LCD panel created");
#elif defined(CONFIG_CUSTOM_DISPLAY_JD9853)
        ret = esp_lcd_new_panel_jd9853(panel_io, &panel_config, &panel);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create JD9853 panel: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }
        ESP_LOGI(TAG, "JD9853 SPI LCD panel created");
#elif defined(CONFIG_CUSTOM_DISPLAY_ST7796)
        ret = esp_lcd_new_panel_st7796(panel_io, &panel_config, &panel);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create ST7796 panel: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }
        ESP_LOGI(TAG, "ST7796 SPI LCD panel created");
#else
        ret = esp_lcd_new_panel_st7789(panel_io, &panel_config, &panel);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create ST7789/ST7735 panel: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }
        ESP_LOGI(TAG, "ST7789 SPI LCD panel created");
#endif

        esp_lcd_panel_reset(panel);
        ret = esp_lcd_panel_init(panel);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize SPI LCD panel: %s. Falling back to NoDisplay", esp_err_to_name(ret));
            display_ = new NoDisplay();
            return;
        }
        esp_lcd_panel_invert_color(panel, DISPLAY_INVERT_COLOR);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
        esp_lcd_panel_set_gap(panel, DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y);
        esp_lcd_panel_disp_on_off(panel, true);

        display_ = new SpiLcdDisplay(panel_io, panel,
                                    DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,
                                    DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);

        if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
            backlight_ = new PwmBacklight(DISPLAY_BACKLIGHT_PIN, false);
            backlight_->RestoreBrightness();
        }
#endif
    }

    void InitializeTouch() {
#if defined(CONFIG_ENABLE_CUSTOM_TOUCH) && !defined(CONFIG_CUSTOM_TOUCH_NONE)
        InitializeI2c();

        esp_lcd_touch_config_t tp_cfg = {
            .x_max = (uint16_t)(DISPLAY_WIDTH - 1),
            .y_max = (uint16_t)(DISPLAY_HEIGHT - 1),
            .rst_gpio_num = TOUCH_RST_PIN,
            .int_gpio_num = TOUCH_INT_PIN,
            .levels = {
                .reset = 0,
                .interrupt = 0,
            },
            .flags = {
                .swap_xy = 0,
                .mirror_x = 0,
                .mirror_y = 0,
            },
        };

        esp_lcd_panel_io_handle_t tp_io_handle = NULL;
        esp_lcd_panel_io_i2c_config_t tp_io_config = {};
        tp_io_config.scl_speed_hz = 400 * 1000;
        tp_io_config.control_phase_bytes = 1;
        tp_io_config.dc_bit_offset = 0;
        tp_io_config.lcd_cmd_bits = 8;
        tp_io_config.lcd_param_bits = 0;
        tp_io_config.flags.disable_control_phase = 1;

#if defined(CONFIG_CUSTOM_TOUCH_CST816S)
        tp_io_config.dev_addr = ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS;
        esp_err_t ret = esp_lcd_new_panel_io_i2c(i2c_bus_, &tp_io_config, &tp_io_handle);
        if (ret == ESP_OK) {
            ret = esp_lcd_touch_new_i2c_cst816s(tp_io_handle, &tp_cfg, &touch_handle_);
        }
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Touch controller CST816S init failed: %s", esp_err_to_name(ret));
            return;
        }
        ESP_LOGI(TAG, "Touch controller CST816S initialized");
#elif defined(CONFIG_CUSTOM_TOUCH_GT911)
        tp_io_config.dev_addr = ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS;
        esp_err_t ret = esp_lcd_new_panel_io_i2c(i2c_bus_, &tp_io_config, &tp_io_handle);
        if (ret == ESP_OK) {
            ret = esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &touch_handle_);
        }
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Touch controller GT911 init failed: %s", esp_err_to_name(ret));
            return;
        }
        ESP_LOGI(TAG, "Touch controller GT911 initialized");
#elif defined(CONFIG_CUSTOM_TOUCH_FT5X06)
        tp_io_config.dev_addr = ESP_LCD_TOUCH_IO_I2C_FT5x06_ADDRESS;
        esp_err_t ret = esp_lcd_new_panel_io_i2c(i2c_bus_, &tp_io_config, &tp_io_handle);
        if (ret == ESP_OK) {
            ret = esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &touch_handle_);
        }
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Touch controller FT5x06 init failed: %s", esp_err_to_name(ret));
            return;
        }
        ESP_LOGI(TAG, "Touch controller FT5x06 initialized");
#endif

        if (touch_handle_ != nullptr && lv_display_get_default() != nullptr) {
            const lvgl_port_touch_cfg_t touch_cfg = {
                .disp = lv_display_get_default(),
                .handle = touch_handle_,
            };
            lvgl_port_add_touch(&touch_cfg);
            ESP_LOGI(TAG, "Touch panel registered to LVGL successfully");
        }
#endif
    }

    void InitializeButtons() {
        if (BOOT_BUTTON_GPIO != GPIO_NUM_NC) {
            boot_button_.OnClick([this]() {
                auto& app = Application::GetInstance();
                if (app.GetDeviceState() == kDeviceStateStarting) {
                    EnterWifiConfigMode();
                    return;
                }
                app.ToggleChatState();
            });
        }

        if (TOUCH_BUTTON_GPIO != GPIO_NUM_NC) {
            touch_button_.OnPressDown([this]() {
                Application::GetInstance().StartListening();
            });
            touch_button_.OnPressUp([this]() {
                Application::GetInstance().StopListening();
            });
        }

        if (VOLUME_UP_BUTTON_GPIO != GPIO_NUM_NC) {
            volume_up_button_.OnClick([this]() {
                auto codec = GetAudioCodec();
                if (codec) {
                    auto volume = codec->output_volume() + 10;
                    if (volume > 100) volume = 100;
                    codec->SetOutputVolume(volume);
                    if (GetDisplay()) {
                        GetDisplay()->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
                    }
                }
            });
        }

        if (VOLUME_DOWN_BUTTON_GPIO != GPIO_NUM_NC) {
            volume_down_button_.OnClick([this]() {
                auto codec = GetAudioCodec();
                if (codec) {
                    auto volume = codec->output_volume() - 10;
                    if (volume < 0) volume = 0;
                    codec->SetOutputVolume(volume);
                    if (GetDisplay()) {
                        GetDisplay()->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
                    }
                }
            });
        }
    }

    void InitializeUart() {
#if defined(CONFIG_ENABLE_CUSTOM_UART)
        uart_config_t uart_config = {
            .baud_rate = CUSTOM_UART_BAUDRATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = (CUSTOM_UART_RTS_PIN != GPIO_NUM_NC && CUSTOM_UART_CTS_PIN != GPIO_NUM_NC) ?
                         UART_HW_FLOWCTRL_CTS_RTS : UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 122,
            .source_clk = UART_SCLK_DEFAULT,
        };
        ESP_ERROR_CHECK(uart_param_config(CUSTOM_UART_PORT, &uart_config));
        ESP_ERROR_CHECK(uart_set_pin(CUSTOM_UART_PORT, CUSTOM_UART_TX_PIN, CUSTOM_UART_RX_PIN,
                                     CUSTOM_UART_RTS_PIN, CUSTOM_UART_CTS_PIN));
        ESP_ERROR_CHECK(uart_driver_install(CUSTOM_UART_PORT, 2048, 2048, 0, NULL, 0));
        ESP_LOGI(TAG, "Custom UART initialized on port %d, TX: %d, RX: %d at %d bps",
                 CUSTOM_UART_PORT, CUSTOM_UART_TX_PIN, CUSTOM_UART_RX_PIN, CUSTOM_UART_BAUDRATE);
#endif
    }

    void InitializeCamera() {
#if defined(CONFIG_ENABLE_CUSTOM_CAMERA) && CONFIG_IDF_TARGET_ESP32S3
#if defined(CONFIG_CUSTOM_CAMERA_USB_UVC)
        ESP_LOGI(TAG, "Custom Camera configured for USB UVC (USB Host Stack)");
#else
        camera_config_t config = {};
        config.pin_d0 = CAM_PIN_D0;
        config.pin_d1 = CAM_PIN_D1;
        config.pin_d2 = CAM_PIN_D2;
        config.pin_d3 = CAM_PIN_D3;
        config.pin_d4 = CAM_PIN_D4;
        config.pin_d5 = CAM_PIN_D5;
        config.pin_d6 = CAM_PIN_D6;
        config.pin_d7 = CAM_PIN_D7;
        config.pin_xclk = CAM_PIN_XCLK;
        config.pin_pclk = CAM_PIN_PCLK;
        config.pin_vsync = CAM_PIN_VSYNC;
        config.pin_href = CAM_PIN_HREF;
        config.pin_sccb_sda = CAM_PIN_SIOD;
        config.pin_sccb_scl = CAM_PIN_SIOC;
        config.sccb_i2c_port = 1;
        config.pin_pwdn = CAM_PIN_PWDN;
        config.pin_reset = CAM_PIN_RESET;
        config.xclk_freq_hz = 20000000;
        config.pixel_format = PIXFORMAT_RGB565;
        config.frame_size = FRAMESIZE_VGA;
        config.jpeg_quality = 12;
        config.fb_count = 2;
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
        camera_ = new Esp32Camera(config);
#if CONFIG_CUSTOM_CAM_HMIRROR
        camera_->SetHMirror(true);
#endif
#if CONFIG_CUSTOM_CAM_VFLIP
        camera_->SetVFlip(true);
#endif
        ESP_LOGI(TAG, "Custom Camera initialized with PSRAM framebuffers");
#endif
#endif
    }

    void InitializeBattery() {
#if defined(CONFIG_CUSTOM_BATTERY_MONITOR_ADC)
        battery_monitor_ = new AdcBatteryMonitor(ADC_UNIT_1, BATTERY_ADC_CHANNEL,
                                                 (float)(BATTERY_DIVIDER_R1 * 1000),
                                                 (float)(BATTERY_DIVIDER_R2 * 1000),
                                                 GPIO_NUM_NC);
        ESP_LOGI(TAG, "ADC Battery Monitor initialized");
#endif
    }

    void InitializeMcpTools() {
#if defined(CONFIG_ENABLE_CUSTOM_MCP_SERVER) || defined(CONFIG_ENABLE_CUSTOM_SENSORS) || \
    defined(CONFIG_CUSTOM_PERIPH_RELAY_ENABLE) || defined(CONFIG_CUSTOM_ENABLE_SERVO_DOG) || \
    defined(CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE)
#if defined(CONFIG_CUSTOM_MCP_TOOL_LAMP) || defined(CONFIG_CUSTOM_PERIPH_RELAY_ENABLE)
        if (LAMP_GPIO != GPIO_NUM_NC) {
            static LampController lamp(LAMP_GPIO);
            ESP_LOGI(TAG, "MCP Lamp Controller registered on GPIO %d", LAMP_GPIO);
        }
#endif
#if defined(CONFIG_CUSTOM_MCP_TOOL_SENSOR) || defined(CONFIG_ENABLE_CUSTOM_SENSORS)
        static SensorController sensor_ctrl;
        ESP_LOGI(TAG, "MCP Sensor Tools registered successfully");
#endif
#if defined(CONFIG_CUSTOM_MCP_TOOL_ACTUATOR) || defined(CONFIG_CUSTOM_PERIPH_RELAY_ENABLE) || \
    defined(CONFIG_CUSTOM_ENABLE_SERVO_DOG) || defined(CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE)
        static ActuatorController actuator_ctrl;
        ESP_LOGI(TAG, "MCP Actuator Tools registered successfully");
#endif
#endif
    }
    void InitializeUnimplementedPeripherals() {
#if defined(CONFIG_CUSTOM_PERIPH_ENCODER_ENABLE)
        ESP_LOGW(TAG, "Driver cho ngoại vi Rotary Encoder chưa được hỗ trợ nhưng đang được bật trong menuconfig");
#endif
#if defined(CONFIG_CUSTOM_PERIPH_NFC_ENABLE)
        ESP_LOGW(TAG, "Driver cho ngoại vi NFC/RFID chưa được hỗ trợ nhưng đang được bật trong menuconfig");
#endif
#if defined(CONFIG_CUSTOM_PERIPH_RTC_ENABLE)
        ESP_LOGW(TAG, "Driver cho ngoại vi RTC chưa được hỗ trợ nhưng đang được bật trong menuconfig");
#endif
#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_PCA9685)
        ESP_LOGW(TAG, "Driver cho ngoại vi PCA9685 chưa được hỗ trợ nhưng đang được bật trong menuconfig");
#endif
#if defined(CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE)
        ESP_LOGW(TAG, "Driver cho ngoại vi Motor DC (H-Bridge) chưa được hỗ trợ nhưng đang được bật trong menuconfig");
#endif
#if defined(CONFIG_CUSTOM_PERIPH_TWAI_ENABLE)
        ESP_LOGW(TAG, "Driver cho ngoại vi CAN/TWAI chưa được hỗ trợ nhưng đang được bật trong menuconfig");
#endif
#if defined(CONFIG_CUSTOM_PERIPH_4G_MODULE_ENABLE)
        ESP_LOGW(TAG, "Driver cho module 4G chưa được hỗ trợ nhưng đang được bật trong menuconfig");
#endif
    }

public:
    CustomN16R8Board() :
        boot_button_(BOOT_BUTTON_GPIO),
        touch_button_(TOUCH_BUTTON_GPIO),
        volume_up_button_(VOLUME_UP_BUTTON_GPIO),
        volume_down_button_(VOLUME_DOWN_BUTTON_GPIO) {

        ESP_LOGI(TAG, "Initializing ESP32-S3 N16R8 Custom Board...");

#if defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8311) || defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8388) || \
    defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8374) || defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8389) || \
    defined(CONFIG_CUSTOM_AUDIO_CODEC_BOX) || \
    defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8311) || defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8388) || \
    defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8374) || defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8389) || \
    defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_BOX) || \
    defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8311) || defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8388) || \
    defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8374) || defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8389) || \
    defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_BOX)
        InitializeI2c();
#endif

        InitializeDisplay();
        InitializeTouch();
        InitializeButtons();
        InitializeUart();
        InitializeCamera();
        InitializeBattery();
        InitializeMcpTools();
        InitializeUnimplementedPeripherals();

        ESP_LOGI(TAG, "ESP32-S3 N16R8 Custom Board initialized successfully");
    }

    virtual Led* GetLed() override {
#if !defined(CONFIG_ENABLE_CUSTOM_LEDS) || defined(CONFIG_CUSTOM_LED_NONE)
        return nullptr;
#elif defined(CONFIG_CUSTOM_LED_CIRCULAR_STRIP)
        static CircularStrip led(BUILTIN_LED_GPIO, BUILTIN_LED_COUNT);
        return &led;
#elif defined(CONFIG_CUSTOM_LED_WS2812)
        if (BUILTIN_LED_COUNT > 1) {
            static CircularStrip led(BUILTIN_LED_GPIO, BUILTIN_LED_COUNT);
            return &led;
        } else {
            static SingleLed led(BUILTIN_LED_GPIO);
            return &led;
        }
#elif defined(CONFIG_CUSTOM_LED_SINGLE_PWM)
        static GpioLed led(BUILTIN_LED_GPIO);
        return &led;
#else
        return nullptr;
#endif
    }

    virtual AudioCodec* GetAudioCodec() override {
#if !defined(CONFIG_ENABLE_CUSTOM_AUDIO) && !defined(CONFIG_ENABLE_CUSTOM_SPEAKER) && !defined(CONFIG_ENABLE_CUSTOM_MIC)
        ESP_LOGI(TAG, "Audio hardware is disabled in configuration (Mute / Headless)");
        return nullptr;
#elif defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8311) || defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8311) || defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8311)
        if (i2c_bus_ == nullptr) {
            InitializeI2c();
        }
        static Es8311AudioCodec audio_codec(i2c_bus_, I2C_NUM_0, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                                            GPIO_NUM_NC, AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK,
                                            AUDIO_I2S_SPK_GPIO_DOUT, AUDIO_I2S_MIC_GPIO_DIN, GPIO_NUM_NC, 0x18, /*use_mclk=*/false);
        return &audio_codec;
#elif defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8388) || defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8388) || defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8388)
        if (i2c_bus_ == nullptr) {
            InitializeI2c();
        }
        static Es8388AudioCodec audio_codec(i2c_bus_, I2C_NUM_0, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                                            GPIO_NUM_NC, AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK,
                                            AUDIO_I2S_SPK_GPIO_DOUT, AUDIO_I2S_MIC_GPIO_DIN, GPIO_NUM_NC, 0x10);
        return &audio_codec;
#elif defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8374) || defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8374) || defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8374)
        if (i2c_bus_ == nullptr) {
            InitializeI2c();
        }
        static Es8374AudioCodec audio_codec(i2c_bus_, I2C_NUM_0, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                                            GPIO_NUM_NC, AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK,
                                            AUDIO_I2S_SPK_GPIO_DOUT, AUDIO_I2S_MIC_GPIO_DIN, GPIO_NUM_NC, 0x10);
        return &audio_codec;
#elif defined(CONFIG_CUSTOM_AUDIO_CODEC_ES8389) || defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8389) || defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8389)
        if (i2c_bus_ == nullptr) {
            InitializeI2c();
        }
        static Es8389AudioCodec audio_codec(i2c_bus_, I2C_NUM_0, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                                            GPIO_NUM_NC, AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK,
                                            AUDIO_I2S_SPK_GPIO_DOUT, AUDIO_I2S_MIC_GPIO_DIN, GPIO_NUM_NC, 0x10);
        return &audio_codec;
#elif defined(CONFIG_CUSTOM_AUDIO_CODEC_BOX) || defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_BOX) || defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_BOX)
        if (i2c_bus_ == nullptr) {
            InitializeI2c();
        }
        static BoxAudioCodec audio_codec(i2c_bus_, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                                         GPIO_NUM_NC, AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK,
                                         AUDIO_I2S_SPK_GPIO_DOUT, AUDIO_I2S_MIC_GPIO_DIN, GPIO_NUM_NC,
                                         0x18, 0x10, false);
        return &audio_codec;
#elif defined(CONFIG_CUSTOM_AUDIO_I2S_DUPLEX)
        static NoAudioCodecDuplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                                              AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK,
                                              AUDIO_I2S_SPK_GPIO_DOUT, AUDIO_I2S_MIC_GPIO_DIN);
        return &audio_codec;
#elif defined(CONFIG_CUSTOM_AUDIO_MIC_PDM)
        static NoAudioCodecSimplexPdm audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                                                  AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT,
                                                  AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_DIN);
        return &audio_codec;
#else
        // Simplex Mode (Separate Clocks) - Supports MAX98357A, PCM5102A + INMP441/MSM261S
#if defined(CONFIG_CUSTOM_AUDIO_DAC_PCM5102A)
        ESP_LOGI(TAG, "Audio DAC output configured for PCM5102A Hi-Fi I2S DAC (32-bit/384kHz)");
#elif defined(CONFIG_CUSTOM_AUDIO_DAC_MAX98357A)
        ESP_LOGI(TAG, "Audio DAC output configured for MAX98357A Class-D I2S Amplifier");
#endif
        static NoAudioCodecSimplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                                               AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT,
                                               AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_WS, AUDIO_I2S_MIC_GPIO_DIN);
        return &audio_codec;
#endif
    }

    virtual Display* GetDisplay() override {
        return display_;
    }

    virtual Backlight* GetBacklight() override {
        return backlight_;
    }

#if CONFIG_IDF_TARGET_ESP32S3
    virtual Camera* GetCamera() override {
        return camera_;
    }
#endif

    virtual bool GetBatteryLevel(int& level, bool& charging, bool& discharging) override {
#if defined(CONFIG_CUSTOM_BATTERY_MONITOR_ADC)
        if (battery_monitor_) {
            charging = battery_monitor_->IsCharging();
            discharging = battery_monitor_->IsDischarging();
            level = battery_monitor_->GetBatteryLevel();
            return true;
        }
#endif
        return false;
    }
};

DECLARE_BOARD(CustomN16R8Board);
