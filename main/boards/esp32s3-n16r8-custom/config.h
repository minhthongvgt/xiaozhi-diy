#ifndef _ESP32S3_N16R8_CUSTOM_CONFIG_H_
#define _ESP32S3_N16R8_CUSTOM_CONFIG_H_

#include <sdkconfig.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>

#if defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8311) || defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8388) || \
    defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8374) || defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8389) || \
    defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_BOX) || \
    defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8311) || defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8388) || \
    defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8374) || defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8389) || \
    defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_BOX) || defined(CONFIG_CUSTOM_AUDIO_I2S_DUPLEX)
#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 16000
#else
#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000
#endif

// 1. Màn hình (Display)
#ifdef CONFIG_CUSTOM_DISPLAY_WIDTH
#define DISPLAY_WIDTH CONFIG_CUSTOM_DISPLAY_WIDTH
#else
#define DISPLAY_WIDTH 240
#endif

#ifdef CONFIG_CUSTOM_DISPLAY_HEIGHT
#define DISPLAY_HEIGHT CONFIG_CUSTOM_DISPLAY_HEIGHT
#else
#define DISPLAY_HEIGHT 320
#endif

#ifdef CONFIG_CUSTOM_DISPLAY_OFFSET_X
#define DISPLAY_OFFSET_X CONFIG_CUSTOM_DISPLAY_OFFSET_X
#else
#define DISPLAY_OFFSET_X 0
#endif

#ifdef CONFIG_CUSTOM_DISPLAY_OFFSET_Y
#define DISPLAY_OFFSET_Y CONFIG_CUSTOM_DISPLAY_OFFSET_Y
#else
#define DISPLAY_OFFSET_Y 0
#endif

#ifdef CONFIG_CUSTOM_DISPLAY_PIN_MOSI
#define DISPLAY_MOSI_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_MOSI)
#else
#define DISPLAY_MOSI_PIN ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_DISPLAY_PIN_CLK
#define DISPLAY_CLK_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_CLK)
#else
#define DISPLAY_CLK_PIN ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_DISPLAY_PIN_CS
#define DISPLAY_CS_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_CS)
#else
#define DISPLAY_CS_PIN ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_DISPLAY_PIN_DC
#define DISPLAY_DC_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_DC)
#else
#define DISPLAY_DC_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_USE_RST) && defined(CONFIG_CUSTOM_DISPLAY_PIN_RST)
#define DISPLAY_RST_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_RST)
#elif !defined(CONFIG_CUSTOM_DISPLAY_USE_RST) && defined(CONFIG_CUSTOM_DISPLAY_PIN_RST) && (CONFIG_CUSTOM_DISPLAY_PIN_RST >= 0)
#define DISPLAY_RST_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_RST)
#elif !defined(CONFIG_CUSTOM_DISPLAY_USE_RST) && !defined(CONFIG_CUSTOM_DISPLAY_PIN_RST)
#define DISPLAY_RST_PIN ((gpio_num_t)-1)
#else
#define DISPLAY_RST_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_USE_BLK) && defined(CONFIG_CUSTOM_DISPLAY_PIN_BLK)
#define DISPLAY_BACKLIGHT_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_BLK)
#elif !defined(CONFIG_CUSTOM_DISPLAY_USE_BLK) && defined(CONFIG_CUSTOM_DISPLAY_PIN_BLK) && (CONFIG_CUSTOM_DISPLAY_PIN_BLK >= 0)
#define DISPLAY_BACKLIGHT_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_BLK)
#elif !defined(CONFIG_CUSTOM_DISPLAY_USE_BLK) && !defined(CONFIG_CUSTOM_DISPLAY_PIN_BLK)
#define DISPLAY_BACKLIGHT_PIN ((gpio_num_t)-1)
#else
#define DISPLAY_BACKLIGHT_PIN ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_DISPLAY_PIN_I2C_SDA
#define DISPLAY_I2C_SDA_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_I2C_SDA)
#else
#define DISPLAY_I2C_SDA_PIN ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_DISPLAY_PIN_I2C_SCL
#define DISPLAY_I2C_SCL_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_PIN_I2C_SCL)
#else
#define DISPLAY_I2C_SCL_PIN ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_DISPLAY_SWAP_XY
#define DISPLAY_SWAP_XY true
#else
#define DISPLAY_SWAP_XY false
#endif

#ifdef CONFIG_CUSTOM_DISPLAY_MIRROR_X
#define DISPLAY_MIRROR_X true
#else
#define DISPLAY_MIRROR_X false
#endif

#ifdef CONFIG_CUSTOM_DISPLAY_MIRROR_Y
#define DISPLAY_MIRROR_Y true
#else
#define DISPLAY_MIRROR_Y false
#endif

#ifdef CONFIG_CUSTOM_DISPLAY_INVERT_COLOR
#define DISPLAY_INVERT_COLOR true
#else
#define DISPLAY_INVERT_COLOR false
#endif

// 1.1 Mực điện tử E-Paper SSD1681
#if defined(CONFIG_CUSTOM_DISPLAY_EPAPER_PIN_BUSY) && (CONFIG_CUSTOM_DISPLAY_EPAPER_PIN_BUSY >= 0)
#define DISPLAY_EPAPER_BUSY_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_EPAPER_PIN_BUSY)
#else
#define DISPLAY_EPAPER_BUSY_PIN ((gpio_num_t)-1)
#endif

// 1.2 Màn hình AMOLED QSPI (SH8601, CO5300, SPD2010)
#if defined(CONFIG_CUSTOM_DISPLAY_QSPI_CS) && (CONFIG_CUSTOM_DISPLAY_QSPI_CS >= 0)
#define DISPLAY_QSPI_CS_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_QSPI_CS)
#else
#define DISPLAY_QSPI_CS_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_QSPI_CLK) && (CONFIG_CUSTOM_DISPLAY_QSPI_CLK >= 0)
#define DISPLAY_QSPI_CLK_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_QSPI_CLK)
#else
#define DISPLAY_QSPI_CLK_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_QSPI_D0) && (CONFIG_CUSTOM_DISPLAY_QSPI_D0 >= 0)
#define DISPLAY_QSPI_D0_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_QSPI_D0)
#else
#define DISPLAY_QSPI_D0_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_QSPI_D1) && (CONFIG_CUSTOM_DISPLAY_QSPI_D1 >= 0)
#define DISPLAY_QSPI_D1_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_QSPI_D1)
#else
#define DISPLAY_QSPI_D1_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_QSPI_D2) && (CONFIG_CUSTOM_DISPLAY_QSPI_D2 >= 0)
#define DISPLAY_QSPI_D2_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_QSPI_D2)
#else
#define DISPLAY_QSPI_D2_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_QSPI_D3) && (CONFIG_CUSTOM_DISPLAY_QSPI_D3 >= 0)
#define DISPLAY_QSPI_D3_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_QSPI_D3)
#else
#define DISPLAY_QSPI_D3_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_QSPI_RST) && (CONFIG_CUSTOM_DISPLAY_QSPI_RST >= 0)
#define DISPLAY_QSPI_RST_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_QSPI_RST)
#else
#define DISPLAY_QSPI_RST_PIN ((gpio_num_t)-1)
#endif

// 1.3 Màn hình ST7701 RGB Interface
#if defined(CONFIG_CUSTOM_DISPLAY_RGB_PCLK) && (CONFIG_CUSTOM_DISPLAY_RGB_PCLK >= 0)
#define DISPLAY_RGB_PCLK_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_PCLK)
#else
#define DISPLAY_RGB_PCLK_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_DE) && (CONFIG_CUSTOM_DISPLAY_RGB_DE >= 0)
#define DISPLAY_RGB_DE_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_DE)
#else
#define DISPLAY_RGB_DE_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_VSYNC) && (CONFIG_CUSTOM_DISPLAY_RGB_VSYNC >= 0)
#define DISPLAY_RGB_VSYNC_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_VSYNC)
#else
#define DISPLAY_RGB_VSYNC_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_HSYNC) && (CONFIG_CUSTOM_DISPLAY_RGB_HSYNC >= 0)
#define DISPLAY_RGB_HSYNC_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_HSYNC)
#else
#define DISPLAY_RGB_HSYNC_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D0) && (CONFIG_CUSTOM_DISPLAY_RGB_D0 >= 0)
#define DISPLAY_RGB_D0_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D0)
#else
#define DISPLAY_RGB_D0_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D1) && (CONFIG_CUSTOM_DISPLAY_RGB_D1 >= 0)
#define DISPLAY_RGB_D1_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D1)
#else
#define DISPLAY_RGB_D1_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D2) && (CONFIG_CUSTOM_DISPLAY_RGB_D2 >= 0)
#define DISPLAY_RGB_D2_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D2)
#else
#define DISPLAY_RGB_D2_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D3) && (CONFIG_CUSTOM_DISPLAY_RGB_D3 >= 0)
#define DISPLAY_RGB_D3_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D3)
#else
#define DISPLAY_RGB_D3_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D4) && (CONFIG_CUSTOM_DISPLAY_RGB_D4 >= 0)
#define DISPLAY_RGB_D4_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D4)
#else
#define DISPLAY_RGB_D4_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D5) && (CONFIG_CUSTOM_DISPLAY_RGB_D5 >= 0)
#define DISPLAY_RGB_D5_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D5)
#else
#define DISPLAY_RGB_D5_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D6) && (CONFIG_CUSTOM_DISPLAY_RGB_D6 >= 0)
#define DISPLAY_RGB_D6_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D6)
#else
#define DISPLAY_RGB_D6_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D7) && (CONFIG_CUSTOM_DISPLAY_RGB_D7 >= 0)
#define DISPLAY_RGB_D7_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D7)
#else
#define DISPLAY_RGB_D7_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D8) && (CONFIG_CUSTOM_DISPLAY_RGB_D8 >= 0)
#define DISPLAY_RGB_D8_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D8)
#else
#define DISPLAY_RGB_D8_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D9) && (CONFIG_CUSTOM_DISPLAY_RGB_D9 >= 0)
#define DISPLAY_RGB_D9_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D9)
#else
#define DISPLAY_RGB_D9_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D10) && (CONFIG_CUSTOM_DISPLAY_RGB_D10 >= 0)
#define DISPLAY_RGB_D10_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D10)
#else
#define DISPLAY_RGB_D10_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D11) && (CONFIG_CUSTOM_DISPLAY_RGB_D11 >= 0)
#define DISPLAY_RGB_D11_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D11)
#else
#define DISPLAY_RGB_D11_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D12) && (CONFIG_CUSTOM_DISPLAY_RGB_D12 >= 0)
#define DISPLAY_RGB_D12_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D12)
#else
#define DISPLAY_RGB_D12_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D13) && (CONFIG_CUSTOM_DISPLAY_RGB_D13 >= 0)
#define DISPLAY_RGB_D13_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D13)
#else
#define DISPLAY_RGB_D13_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D14) && (CONFIG_CUSTOM_DISPLAY_RGB_D14 >= 0)
#define DISPLAY_RGB_D14_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D14)
#else
#define DISPLAY_RGB_D14_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_RGB_D15) && (CONFIG_CUSTOM_DISPLAY_RGB_D15 >= 0)
#define DISPLAY_RGB_D15_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_RGB_D15)
#else
#define DISPLAY_RGB_D15_PIN ((gpio_num_t)-1)
#endif

// Cấu hình Màn hình rời qua UART (Nextion, TJC, DWIN, JSON Stream)
#if defined(CONFIG_CUSTOM_DISPLAY_UART_TX_PIN) && (CONFIG_CUSTOM_DISPLAY_UART_TX_PIN >= 0)
#define DISPLAY_UART_TX_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_UART_TX_PIN)
#else
#define DISPLAY_UART_TX_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_UART_RX_PIN) && (CONFIG_CUSTOM_DISPLAY_UART_RX_PIN >= 0)
#define DISPLAY_UART_RX_PIN ((gpio_num_t)CONFIG_CUSTOM_DISPLAY_UART_RX_PIN)
#else
#define DISPLAY_UART_RX_PIN ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE
#define DISPLAY_UART_BAUDRATE CONFIG_CUSTOM_DISPLAY_UART_BAUDRATE
#else
#define DISPLAY_UART_BAUDRATE 115200
#endif

#if defined(CONFIG_CUSTOM_DISPLAY_UART_PORT) && (CONFIG_CUSTOM_DISPLAY_UART_PORT == 2)
#define DISPLAY_UART_PORT UART_NUM_2
#else
#define DISPLAY_UART_PORT UART_NUM_1
#endif

// 2. Màn hình Cảm ứng (Touch Screen)
#ifdef CONFIG_CUSTOM_TOUCH_PIN_SDA
#define TOUCH_I2C_SDA_PIN ((gpio_num_t)CONFIG_CUSTOM_TOUCH_PIN_SDA)
#else
#define TOUCH_I2C_SDA_PIN ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_TOUCH_PIN_SCL
#define TOUCH_I2C_SCL_PIN ((gpio_num_t)CONFIG_CUSTOM_TOUCH_PIN_SCL)
#else
#define TOUCH_I2C_SCL_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_TOUCH_PIN_INT) && (CONFIG_CUSTOM_TOUCH_PIN_INT >= 0)
#define TOUCH_INT_PIN ((gpio_num_t)CONFIG_CUSTOM_TOUCH_PIN_INT)
#else
#define TOUCH_INT_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_TOUCH_PIN_RST) && (CONFIG_CUSTOM_TOUCH_PIN_RST >= 0)
#define TOUCH_RST_PIN ((gpio_num_t)CONFIG_CUSTOM_TOUCH_PIN_RST)
#else
#define TOUCH_RST_PIN ((gpio_num_t)-1)
#endif

// 3. Âm thanh (Audio I2S & Codec)
#ifdef CONFIG_CUSTOM_AUDIO_MIC_GPIO_WS
#define AUDIO_I2S_MIC_GPIO_WS ((gpio_num_t)CONFIG_CUSTOM_AUDIO_MIC_GPIO_WS)
#else
#define AUDIO_I2S_MIC_GPIO_WS ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_AUDIO_MIC_GPIO_SCK
#define AUDIO_I2S_MIC_GPIO_SCK ((gpio_num_t)CONFIG_CUSTOM_AUDIO_MIC_GPIO_SCK)
#else
#define AUDIO_I2S_MIC_GPIO_SCK ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_AUDIO_MIC_GPIO_DIN
#define AUDIO_I2S_MIC_GPIO_DIN ((gpio_num_t)CONFIG_CUSTOM_AUDIO_MIC_GPIO_DIN)
#else
#define AUDIO_I2S_MIC_GPIO_DIN ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_AUDIO_SPK_GPIO_DOUT
#define AUDIO_I2S_SPK_GPIO_DOUT ((gpio_num_t)CONFIG_CUSTOM_AUDIO_SPK_GPIO_DOUT)
#else
#define AUDIO_I2S_SPK_GPIO_DOUT ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_AUDIO_SPK_GPIO_BCLK
#define AUDIO_I2S_SPK_GPIO_BCLK ((gpio_num_t)CONFIG_CUSTOM_AUDIO_SPK_GPIO_BCLK)
#else
#define AUDIO_I2S_SPK_GPIO_BCLK ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_AUDIO_SPK_GPIO_LRCK
#define AUDIO_I2S_SPK_GPIO_LRCK ((gpio_num_t)CONFIG_CUSTOM_AUDIO_SPK_GPIO_LRCK)
#else
#define AUDIO_I2S_SPK_GPIO_LRCK ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_I2C_SDA)
#define AUDIO_CODEC_I2C_SDA_PIN ((gpio_num_t)CONFIG_CUSTOM_AUDIO_SPK_CODEC_I2C_SDA)
#elif defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_I2C_SDA)
#define AUDIO_CODEC_I2C_SDA_PIN ((gpio_num_t)CONFIG_CUSTOM_AUDIO_MIC_CODEC_I2C_SDA)
#else
#define AUDIO_CODEC_I2C_SDA_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_AUDIO_SPK_CODEC_I2C_SCL)
#define AUDIO_CODEC_I2C_SCL_PIN ((gpio_num_t)CONFIG_CUSTOM_AUDIO_SPK_CODEC_I2C_SCL)
#elif defined(CONFIG_CUSTOM_AUDIO_MIC_CODEC_I2C_SCL)
#define AUDIO_CODEC_I2C_SCL_PIN ((gpio_num_t)CONFIG_CUSTOM_AUDIO_MIC_CODEC_I2C_SCL)
#else
#define AUDIO_CODEC_I2C_SCL_PIN ((gpio_num_t)-1)
#endif

// 4. Camera Pins
#ifdef CONFIG_CUSTOM_CAM_PIN_XCLK
#define CAM_PIN_XCLK ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_XCLK)
#else
#define CAM_PIN_XCLK ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_CAM_PIN_PCLK
#define CAM_PIN_PCLK ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_PCLK)
#else
#define CAM_PIN_PCLK ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_CAM_PIN_VSYNC
#define CAM_PIN_VSYNC ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_VSYNC)
#else
#define CAM_PIN_VSYNC ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_CAM_PIN_HREF
#define CAM_PIN_HREF ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_HREF)
#else
#define CAM_PIN_HREF ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_CAM_PIN_SIOD
#define CAM_PIN_SIOD ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_SIOD)
#else
#define CAM_PIN_SIOD ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_CAM_PIN_SIOC
#define CAM_PIN_SIOC ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_SIOC)
#else
#define CAM_PIN_SIOC ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_CAM_PIN_D0
#define CAM_PIN_D0 ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_D0)
#else
#define CAM_PIN_D0 ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_CAM_PIN_D1
#define CAM_PIN_D1 ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_D1)
#else
#define CAM_PIN_D1 ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_CAM_PIN_D2
#define CAM_PIN_D2 ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_D2)
#else
#define CAM_PIN_D2 ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_CAM_PIN_D3
#define CAM_PIN_D3 ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_D3)
#else
#define CAM_PIN_D3 ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_CAM_PIN_D4
#define CAM_PIN_D4 ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_D4)
#else
#define CAM_PIN_D4 ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_CAM_PIN_D5
#define CAM_PIN_D5 ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_D5)
#else
#define CAM_PIN_D5 ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_CAM_PIN_D6
#define CAM_PIN_D6 ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_D6)
#else
#define CAM_PIN_D6 ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_CAM_PIN_D7
#define CAM_PIN_D7 ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_D7)
#else
#define CAM_PIN_D7 ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_CAM_USE_RESET) && defined(CONFIG_CUSTOM_CAM_PIN_RESET)
#define CAM_PIN_RESET ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_RESET)
#elif !defined(CONFIG_CUSTOM_CAM_USE_RESET) && defined(CONFIG_CUSTOM_CAM_PIN_RESET) && (CONFIG_CUSTOM_CAM_PIN_RESET >= 0)
#define CAM_PIN_RESET ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_RESET)
#else
#define CAM_PIN_RESET ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_CAM_USE_PWDN) && defined(CONFIG_CUSTOM_CAM_PIN_PWDN)
#define CAM_PIN_PWDN ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_PWDN)
#elif !defined(CONFIG_CUSTOM_CAM_USE_PWDN) && defined(CONFIG_CUSTOM_CAM_PIN_PWDN) && (CONFIG_CUSTOM_CAM_PIN_PWDN >= 0)
#define CAM_PIN_PWDN ((gpio_num_t)CONFIG_CUSTOM_CAM_PIN_PWDN)
#else
#define CAM_PIN_PWDN ((gpio_num_t)-1)
#endif

// 5. UART
#if defined(CONFIG_CUSTOM_UART_PORT_2)
#define CUSTOM_UART_PORT UART_NUM_2
#elif defined(CONFIG_CUSTOM_UART_PORT_1)
#define CUSTOM_UART_PORT UART_NUM_1
#elif (defined(CONFIG_CUSTOM_DISPLAY_UART) || defined(CONFIG_ENABLE_CUSTOM_SECONDARY_UART_DISPLAY)) && (DISPLAY_UART_PORT == UART_NUM_1)
// Tự động phân bổ sang UART_NUM_2 nếu Display UART đã dùng UART_NUM_1
#define CUSTOM_UART_PORT UART_NUM_2
#else
#define CUSTOM_UART_PORT UART_NUM_1
#endif

#if defined(CONFIG_CUSTOM_UART_PIN_TX) && (CONFIG_CUSTOM_UART_PIN_TX >= 0)
#define CUSTOM_UART_TX_PIN ((gpio_num_t)CONFIG_CUSTOM_UART_PIN_TX)
#else
#define CUSTOM_UART_TX_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_UART_PIN_RX) && (CONFIG_CUSTOM_UART_PIN_RX >= 0)
#define CUSTOM_UART_RX_PIN ((gpio_num_t)CONFIG_CUSTOM_UART_PIN_RX)
#else
#define CUSTOM_UART_RX_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_UART_USE_FLOW_CONTROL) && defined(CONFIG_CUSTOM_UART_PIN_RTS) && (CONFIG_CUSTOM_UART_PIN_RTS >= 0)
#define CUSTOM_UART_RTS_PIN ((gpio_num_t)CONFIG_CUSTOM_UART_PIN_RTS)
#else
#define CUSTOM_UART_RTS_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_UART_USE_FLOW_CONTROL) && defined(CONFIG_CUSTOM_UART_PIN_CTS) && (CONFIG_CUSTOM_UART_PIN_CTS >= 0)
#define CUSTOM_UART_CTS_PIN ((gpio_num_t)CONFIG_CUSTOM_UART_PIN_CTS)
#else
#define CUSTOM_UART_CTS_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_UART_BAUDRATE)
#define CUSTOM_UART_BAUDRATE CONFIG_CUSTOM_UART_BAUDRATE
#else
#define CUSTOM_UART_BAUDRATE 115200
#endif

// 6. Cảm biến & Đo Pin
#ifdef CONFIG_CUSTOM_BATTERY_ADC_CHANNEL
#define BATTERY_ADC_CHANNEL ((adc_channel_t)CONFIG_CUSTOM_BATTERY_ADC_CHANNEL)
#else
#define BATTERY_ADC_CHANNEL ADC_CHANNEL_0
#endif

#ifdef CONFIG_CUSTOM_BATTERY_DIVIDER_R1
#define BATTERY_DIVIDER_R1 CONFIG_CUSTOM_BATTERY_DIVIDER_R1
#else
#define BATTERY_DIVIDER_R1 100
#endif

#ifdef CONFIG_CUSTOM_BATTERY_DIVIDER_R2
#define BATTERY_DIVIDER_R2 CONFIG_CUSTOM_BATTERY_DIVIDER_R2
#else
#define BATTERY_DIVIDER_R2 100
#endif

// 7. LED & Phím bấm
#if defined(CONFIG_ENABLE_CUSTOM_LEDS) && !defined(CONFIG_CUSTOM_LED_NONE)
#ifdef CONFIG_CUSTOM_LED_GPIO
#define BUILTIN_LED_GPIO ((gpio_num_t)CONFIG_CUSTOM_LED_GPIO)
#else
#define BUILTIN_LED_GPIO ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_LED_COUNT
#define BUILTIN_LED_COUNT CONFIG_CUSTOM_LED_COUNT
#else
#define BUILTIN_LED_COUNT 1
#endif
#else
#define BUILTIN_LED_GPIO ((gpio_num_t)-1)
#define BUILTIN_LED_COUNT 0
#endif

#if defined(CONFIG_CUSTOM_ENABLE_BUTTON_BOOT)
#if defined(CONFIG_CUSTOM_BUTTON_BOOT_GPIO) && (CONFIG_CUSTOM_BUTTON_BOOT_GPIO >= 0)
#define BOOT_BUTTON_GPIO ((gpio_num_t)CONFIG_CUSTOM_BUTTON_BOOT_GPIO)
#else
#define BOOT_BUTTON_GPIO ((gpio_num_t)-1)
#endif
#else
#define BOOT_BUTTON_GPIO ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_ENABLE_BUTTON_TOUCH)
#if defined(CONFIG_CUSTOM_BUTTON_TOUCH_GPIO) && (CONFIG_CUSTOM_BUTTON_TOUCH_GPIO >= 0)
#define TOUCH_BUTTON_GPIO ((gpio_num_t)CONFIG_CUSTOM_BUTTON_TOUCH_GPIO)
#else
#define TOUCH_BUTTON_GPIO ((gpio_num_t)-1)
#endif
#else
#define TOUCH_BUTTON_GPIO ((gpio_num_t)-1)
#endif

#if defined(CONFIG_CUSTOM_ENABLE_BUTTON_VOLUME)
#if defined(CONFIG_CUSTOM_BUTTON_VOL_UP_GPIO) && (CONFIG_CUSTOM_BUTTON_VOL_UP_GPIO >= 0)
#define VOLUME_UP_BUTTON_GPIO ((gpio_num_t)CONFIG_CUSTOM_BUTTON_VOL_UP_GPIO)
#else
#define VOLUME_UP_BUTTON_GPIO ((gpio_num_t)-1)
#endif
#if defined(CONFIG_CUSTOM_BUTTON_VOL_DOWN_GPIO) && (CONFIG_CUSTOM_BUTTON_VOL_DOWN_GPIO >= 0)
#define VOLUME_DOWN_BUTTON_GPIO ((gpio_num_t)CONFIG_CUSTOM_BUTTON_VOL_DOWN_GPIO)
#else
#define VOLUME_DOWN_BUTTON_GPIO ((gpio_num_t)-1)
#endif
#else
#define VOLUME_UP_BUTTON_GPIO ((gpio_num_t)-1)
#define VOLUME_DOWN_BUTTON_GPIO ((gpio_num_t)-1)
#endif

// 8. MCP Server Lamp / Relay
#if defined(CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO) && (CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO >= 0)
#define LAMP_GPIO ((gpio_num_t)CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO)
#elif defined(CONFIG_CUSTOM_PERIPH_RELAY_GPIO) && (CONFIG_CUSTOM_PERIPH_RELAY_GPIO >= 0)
#define LAMP_GPIO ((gpio_num_t)CONFIG_CUSTOM_PERIPH_RELAY_GPIO)
#elif defined(PIN_RELAY) && (PIN_RELAY >= 0)
#define LAMP_GPIO PIN_RELAY
#else
#define LAMP_GPIO ((gpio_num_t)-1)
#endif

// 8.1. Buzzer Alarm & Haptic Vibration (ESP-IDF 6.1 Strict Type Safety)
#if defined(CONFIG_ENABLE_BUZZER) && defined(CONFIG_BUZZER_PIN) && (CONFIG_BUZZER_PIN >= 0)
#define BUZZER_PIN ((gpio_num_t)CONFIG_BUZZER_PIN)
#else
#define BUZZER_PIN ((gpio_num_t)-1)
#endif

#if defined(CONFIG_ENABLE_HAPTIC_MOTOR) && defined(CONFIG_HAPTIC_PIN) && (CONFIG_HAPTIC_PIN >= 0)
#define HAPTIC_PIN ((gpio_num_t)CONFIG_HAPTIC_PIN)
#else
#define HAPTIC_PIN ((gpio_num_t)-1)
#endif

// 8.2. Động cơ Servo tương tác qua PWM (Servo Motor Controller)
#if defined(CONFIG_CUSTOM_ENABLE_SERVO_DOG)
#if defined(CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO) && (CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO >= 0)
#define SERVO_PWM_PIN ((gpio_num_t)CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO)
#elif defined(PIN_SERVO_PWM) && (PIN_SERVO_PWM >= 0)
#define SERVO_PWM_PIN PIN_SERVO_PWM
#else
#define SERVO_PWM_PIN ((gpio_num_t)-1)
#endif
#else
#define SERVO_PWM_PIN ((gpio_num_t)-1)
#endif

// 9. Cảm biến phổ biến (Popular Sensors)
#ifdef CONFIG_CUSTOM_SENSOR_DHT_GPIO
#define SENSOR_DHT_GPIO ((gpio_num_t)CONFIG_CUSTOM_SENSOR_DHT_GPIO)
#else
#define SENSOR_DHT_GPIO ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_SENSOR_HCSR04_TRIG_GPIO
#define SENSOR_HCSR04_TRIG_GPIO ((gpio_num_t)CONFIG_CUSTOM_SENSOR_HCSR04_TRIG_GPIO)
#else
#define SENSOR_HCSR04_TRIG_GPIO ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_SENSOR_HCSR04_ECHO_GPIO
#define SENSOR_HCSR04_ECHO_GPIO ((gpio_num_t)CONFIG_CUSTOM_SENSOR_HCSR04_ECHO_GPIO)
#else
#define SENSOR_HCSR04_ECHO_GPIO ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_SENSOR_PIR_GPIO
#define SENSOR_PIR_GPIO ((gpio_num_t)CONFIG_CUSTOM_SENSOR_PIR_GPIO)
#else
#define SENSOR_PIR_GPIO ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_SENSOR_BMP280_I2C_SDA
#define SENSOR_BMP280_I2C_SDA ((gpio_num_t)CONFIG_CUSTOM_SENSOR_BMP280_I2C_SDA)
#define SENSOR_BMP280_I2C_SCL ((gpio_num_t)CONFIG_CUSTOM_SENSOR_BMP280_I2C_SCL)
#else
#define SENSOR_BMP280_I2C_SDA ((gpio_num_t)-1)
#define SENSOR_BMP280_I2C_SCL ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_SENSOR_BH1750_I2C_SDA
#define SENSOR_BH1750_I2C_SDA ((gpio_num_t)CONFIG_CUSTOM_SENSOR_BH1750_I2C_SDA)
#define SENSOR_BH1750_I2C_SCL ((gpio_num_t)CONFIG_CUSTOM_SENSOR_BH1750_I2C_SCL)
#else
#define SENSOR_BH1750_I2C_SDA ((gpio_num_t)-1)
#define SENSOR_BH1750_I2C_SCL ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_BATTERY_BQ27220_I2C_SDA
#define BATTERY_BQ27220_I2C_SDA ((gpio_num_t)CONFIG_CUSTOM_BATTERY_BQ27220_I2C_SDA)
#define BATTERY_BQ27220_I2C_SCL ((gpio_num_t)CONFIG_CUSTOM_BATTERY_BQ27220_I2C_SCL)
#else
#define BATTERY_BQ27220_I2C_SDA ((gpio_num_t)-1)
#define BATTERY_BQ27220_I2C_SCL ((gpio_num_t)-1)
#endif

#ifdef CONFIG_CUSTOM_TOUCH_SLIDER_PAD1_GPIO
#define TOUCH_SLIDER_PAD1_GPIO ((gpio_num_t)CONFIG_CUSTOM_TOUCH_SLIDER_PAD1_GPIO)
#define TOUCH_SLIDER_PAD2_GPIO ((gpio_num_t)CONFIG_CUSTOM_TOUCH_SLIDER_PAD2_GPIO)
#define TOUCH_SLIDER_PAD3_GPIO ((gpio_num_t)CONFIG_CUSTOM_TOUCH_SLIDER_PAD3_GPIO)
#else
#define TOUCH_SLIDER_PAD1_GPIO ((gpio_num_t)-1)
#define TOUCH_SLIDER_PAD2_GPIO ((gpio_num_t)-1)
#define TOUCH_SLIDER_PAD3_GPIO ((gpio_num_t)-1)
#endif

// Cảm biến nồng độ Khí độc TVOC & CO2 (SCD40/41, SGP30/40)
#ifdef CONFIG_CUSTOM_SENSOR_GAS_I2C_SDA
#define SENSOR_GAS_I2C_SDA ((gpio_num_t)CONFIG_CUSTOM_SENSOR_GAS_I2C_SDA)
#define SENSOR_GAS_I2C_SCL ((gpio_num_t)CONFIG_CUSTOM_SENSOR_GAS_I2C_SCL)
#else
#define SENSOR_GAS_I2C_SDA ((gpio_num_t)-1)
#define SENSOR_GAS_I2C_SCL ((gpio_num_t)-1)
#endif

// Cảm biến cử chỉ không chạm 3D & Màu sắc APDS-9960
#ifdef CONFIG_CUSTOM_SENSOR_APDS9960_I2C_SDA
#define SENSOR_APDS9960_I2C_SDA ((gpio_num_t)CONFIG_CUSTOM_SENSOR_APDS9960_I2C_SDA)
#define SENSOR_APDS9960_I2C_SCL ((gpio_num_t)CONFIG_CUSTOM_SENSOR_APDS9960_I2C_SCL)
#else
#define SENSOR_APDS9960_I2C_SDA ((gpio_num_t)-1)
#define SENSOR_APDS9960_I2C_SCL ((gpio_num_t)-1)
#endif
#if defined(CONFIG_CUSTOM_SENSOR_APDS9960_INT_PIN) && (CONFIG_CUSTOM_SENSOR_APDS9960_INT_PIN >= 0)
#define SENSOR_APDS9960_INT_PIN ((gpio_num_t)CONFIG_CUSTOM_SENSOR_APDS9960_INT_PIN)
#else
#define SENSOR_APDS9960_INT_PIN ((gpio_num_t)-1)
#endif

// Cảm biến khoảng cách Laser ToF VL53L0X / VL53L1X
#ifdef CONFIG_CUSTOM_SENSOR_VL53LX_I2C_SDA
#define SENSOR_VL53LX_I2C_SDA ((gpio_num_t)CONFIG_CUSTOM_SENSOR_VL53LX_I2C_SDA)
#define SENSOR_VL53LX_I2C_SCL ((gpio_num_t)CONFIG_CUSTOM_SENSOR_VL53LX_I2C_SCL)
#else
#define SENSOR_VL53LX_I2C_SDA ((gpio_num_t)-1)
#define SENSOR_VL53LX_I2C_SCL ((gpio_num_t)-1)
#endif
#if defined(CONFIG_CUSTOM_SENSOR_VL53LX_XSHUT_PIN) && (CONFIG_CUSTOM_SENSOR_VL53LX_XSHUT_PIN >= 0)
#define SENSOR_VL53LX_XSHUT_PIN ((gpio_num_t)CONFIG_CUSTOM_SENSOR_VL53LX_XSHUT_PIN)
#else
#define SENSOR_VL53LX_XSHUT_PIN ((gpio_num_t)-1)
#endif

// Module thẻ thông minh NFC / RFID PN532
#ifdef CONFIG_CUSTOM_PERIPH_NFC_I2C_SDA
#define PERIPH_NFC_I2C_SDA ((gpio_num_t)CONFIG_CUSTOM_PERIPH_NFC_I2C_SDA)
#define PERIPH_NFC_I2C_SCL ((gpio_num_t)CONFIG_CUSTOM_PERIPH_NFC_I2C_SCL)
#else
#define PERIPH_NFC_I2C_SDA ((gpio_num_t)-1)
#define PERIPH_NFC_I2C_SCL ((gpio_num_t)-1)
#endif
#if defined(CONFIG_CUSTOM_PERIPH_NFC_IRQ_PIN) && (CONFIG_CUSTOM_PERIPH_NFC_IRQ_PIN >= 0)
#define PERIPH_NFC_IRQ_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_NFC_IRQ_PIN)
#else
#define PERIPH_NFC_IRQ_PIN ((gpio_num_t)-1)
#endif
#if defined(CONFIG_CUSTOM_PERIPH_NFC_RST_PIN) && (CONFIG_CUSTOM_PERIPH_NFC_RST_PIN >= 0)
#define PERIPH_NFC_RST_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_NFC_RST_PIN)
#else
#define PERIPH_NFC_RST_PIN ((gpio_num_t)-1)
#endif

// Đồng hồ thời gian thực RTC ngoại tuyến DS3231 / PCF8563
#ifdef CONFIG_CUSTOM_PERIPH_RTC_I2C_SDA
#define PERIPH_RTC_I2C_SDA ((gpio_num_t)CONFIG_CUSTOM_PERIPH_RTC_I2C_SDA)
#define PERIPH_RTC_I2C_SCL ((gpio_num_t)CONFIG_CUSTOM_PERIPH_RTC_I2C_SCL)
#else
#define PERIPH_RTC_I2C_SDA ((gpio_num_t)-1)
#define PERIPH_RTC_I2C_SCL ((gpio_num_t)-1)
#endif
#if defined(CONFIG_CUSTOM_PERIPH_RTC_INT_PIN) && (CONFIG_CUSTOM_PERIPH_RTC_INT_PIN >= 0)
#define PERIPH_RTC_INT_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_RTC_INT_PIN)
#else
#define PERIPH_RTC_INT_PIN ((gpio_num_t)-1)
#endif

// Núm vặn xoay vô cấp cơ học Rotary Encoder EC11
#ifdef CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_A
#define ENCODER_PHASE_A_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_A)
#define ENCODER_PHASE_B_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_B)
#else
#define ENCODER_PHASE_A_PIN ((gpio_num_t)-1)
#define ENCODER_PHASE_B_PIN ((gpio_num_t)-1)
#endif
#if defined(CONFIG_CUSTOM_PERIPH_ENCODER_KEY_PIN) && (CONFIG_CUSTOM_PERIPH_ENCODER_KEY_PIN >= 0)
#define ENCODER_KEY_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_ENCODER_KEY_PIN)
#else
#define ENCODER_KEY_PIN ((gpio_num_t)-1)
#endif

// Mắt phát / thu hồng ngoại điều khiển từ xa IR Transceiver
#ifdef CONFIG_CUSTOM_PERIPH_IR_TX_PIN
#define PERIPH_IR_TX_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_IR_TX_PIN)
#else
#define PERIPH_IR_TX_PIN ((gpio_num_t)-1)
#endif
#if defined(CONFIG_CUSTOM_PERIPH_IR_RX_PIN) && (CONFIG_CUSTOM_PERIPH_IR_RX_PIN >= 0)
#define PERIPH_IR_RX_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_IR_RX_PIN)
#else
#define PERIPH_IR_RX_PIN ((gpio_num_t)-1)
#endif

// Module đo năng lượng tiêu thụ INA219 / INA226
#ifdef CONFIG_CUSTOM_SENSOR_INA2XX_I2C_SDA
#define SENSOR_INA2XX_I2C_SDA ((gpio_num_t)CONFIG_CUSTOM_SENSOR_INA2XX_I2C_SDA)
#define SENSOR_INA2XX_I2C_SCL ((gpio_num_t)CONFIG_CUSTOM_SENSOR_INA2XX_I2C_SCL)
#else
#define SENSOR_INA2XX_I2C_SDA ((gpio_num_t)-1)
#define SENSOR_INA2XX_I2C_SCL ((gpio_num_t)-1)
#endif

// Khe cắm thẻ nhớ mở rộng MicroSD Card (SPI Bus)
#ifdef CONFIG_CUSTOM_PERIPH_SDCARD_SPI_SCK
#define SDCARD_SPI_SCK_PIN  ((gpio_num_t)CONFIG_CUSTOM_PERIPH_SDCARD_SPI_SCK)
#define SDCARD_SPI_MOSI_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_SDCARD_SPI_MOSI)
#define SDCARD_SPI_MISO_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_SDCARD_SPI_MISO)
#define SDCARD_SPI_CS_PIN   ((gpio_num_t)CONFIG_CUSTOM_PERIPH_SDCARD_SPI_CS)
#else
#define SDCARD_SPI_SCK_PIN  ((gpio_num_t)-1)
#define SDCARD_SPI_MOSI_PIN ((gpio_num_t)-1)
#define SDCARD_SPI_MISO_PIN ((gpio_num_t)-1)
#define SDCARD_SPI_CS_PIN   ((gpio_num_t)-1)
#endif

// IC mở rộng điều khiển LED AW9523B / IS31FL3731
#ifdef CONFIG_CUSTOM_PERIPH_LED_IC_I2C_SDA
#define LED_IC_I2C_SDA ((gpio_num_t)CONFIG_CUSTOM_PERIPH_LED_IC_I2C_SDA)
#define LED_IC_I2C_SCL ((gpio_num_t)CONFIG_CUSTOM_PERIPH_LED_IC_I2C_SCL)
#else
#define LED_IC_I2C_SDA ((gpio_num_t)-1)
#define LED_IC_I2C_SCL ((gpio_num_t)-1)
#endif

// Màn hình cảm ứng điện dung I2C rời (CST816D/S, GT911, FT6236)
#ifdef CONFIG_CUSTOM_PERIPH_TOUCH_I2C_SDA
#define PERIPH_TOUCH_I2C_SDA ((gpio_num_t)CONFIG_CUSTOM_PERIPH_TOUCH_I2C_SDA)
#define PERIPH_TOUCH_I2C_SCL ((gpio_num_t)CONFIG_CUSTOM_PERIPH_TOUCH_I2C_SCL)
#else
#define PERIPH_TOUCH_I2C_SDA ((gpio_num_t)-1)
#define PERIPH_TOUCH_I2C_SCL ((gpio_num_t)-1)
#endif
#if defined(CONFIG_CUSTOM_PERIPH_TOUCH_INT_PIN) && (CONFIG_CUSTOM_PERIPH_TOUCH_INT_PIN >= 0)
#define PERIPH_TOUCH_INT_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_TOUCH_INT_PIN)
#else
#define PERIPH_TOUCH_INT_PIN ((gpio_num_t)-1)
#endif
#if defined(CONFIG_CUSTOM_PERIPH_TOUCH_RST_PIN) && (CONFIG_CUSTOM_PERIPH_TOUCH_RST_PIN >= 0)
#define PERIPH_TOUCH_RST_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_TOUCH_RST_PIN)
#else
#define PERIPH_TOUCH_RST_PIN ((gpio_num_t)-1)
#endif

// Mạch mở rộng 16 kênh PWM cho Robot PCA9685
#ifdef CONFIG_CUSTOM_PERIPH_PCA9685_I2C_SDA
#define PERIPH_PCA9685_I2C_SDA ((gpio_num_t)CONFIG_CUSTOM_PERIPH_PCA9685_I2C_SDA)
#define PERIPH_PCA9685_I2C_SCL ((gpio_num_t)CONFIG_CUSTOM_PERIPH_PCA9685_I2C_SCL)
#else
#define PERIPH_PCA9685_I2C_SDA ((gpio_num_t)-1)
#define PERIPH_PCA9685_I2C_SCL ((gpio_num_t)-1)
#endif
#if defined(CONFIG_CUSTOM_PERIPH_PCA9685_OE_PIN) && (CONFIG_CUSTOM_PERIPH_PCA9685_OE_PIN >= 0)
#define PERIPH_PCA9685_OE_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_PCA9685_OE_PIN)
#else
#define PERIPH_PCA9685_OE_PIN ((gpio_num_t)-1)
#endif

// Mạch cầu H Động cơ DC 2 chiều TB6612FNG / L9110S / DRV8833
#ifdef CONFIG_CUSTOM_PERIPH_MOTOR_PWMA_PIN
#define MOTOR_DC_PWMA_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_MOTOR_PWMA_PIN)
#define MOTOR_DC_DIRA_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_MOTOR_DIRA_PIN)
#define MOTOR_DC_PWMB_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_MOTOR_PWMB_PIN)
#define MOTOR_DC_DIRB_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_MOTOR_DIRB_PIN)
#else
#define MOTOR_DC_PWMA_PIN ((gpio_num_t)-1)
#define MOTOR_DC_DIRA_PIN ((gpio_num_t)-1)
#define MOTOR_DC_PWMB_PIN ((gpio_num_t)-1)
#define MOTOR_DC_DIRB_PIN ((gpio_num_t)-1)
#endif

// Cảm biến nhiệt độ công nghiệp 1-Wire DS18B20
#ifdef CONFIG_CUSTOM_SENSOR_DS18B20_PIN
#define SENSOR_DS18B20_PIN ((gpio_num_t)CONFIG_CUSTOM_SENSOR_DS18B20_PIN)
#else
#define SENSOR_DS18B20_PIN ((gpio_num_t)-1)
#endif

// Cảm biến lưu lượng chất lỏng đếm xung PCNT
#ifdef CONFIG_CUSTOM_SENSOR_FLOW_PULSE_PIN
#define SENSOR_FLOW_PULSE_PIN ((gpio_num_t)CONFIG_CUSTOM_SENSOR_FLOW_PULSE_PIN)
#else
#define SENSOR_FLOW_PULSE_PIN ((gpio_num_t)-1)
#endif

// Cảm biến khói & khí gas dễ cháy MQ-2 / MQ-135
#ifdef CONFIG_CUSTOM_SENSOR_MQ_ANALOG_PIN
#define SENSOR_MQ_ANALOG_PIN ((gpio_num_t)CONFIG_CUSTOM_SENSOR_MQ_ANALOG_PIN)
#else
    #define SENSOR_MQ_ANALOG_PIN ((gpio_num_t)-1)
#endif

// Cảm biến cảnh báo chấn động / Rung SW-420
#ifdef CONFIG_CUSTOM_SENSOR_VIBRATION_PIN
#define SENSOR_VIBRATION_PIN ((gpio_num_t)CONFIG_CUSTOM_SENSOR_VIBRATION_PIN)
#else
#define SENSOR_VIBRATION_PIN ((gpio_num_t)-1)
#endif

// Cảm biến cảnh báo hỏa hoạn / Lửa Flame Sensor
#ifdef CONFIG_CUSTOM_SENSOR_FLAME_PIN
#define SENSOR_FLAME_PIN ((gpio_num_t)CONFIG_CUSTOM_SENSOR_FLAME_PIN)
#else
#define SENSOR_FLAME_PIN ((gpio_num_t)-1)
#endif

// Đầu đọc thẻ từ RFID 13.56MHz RC522 (SPI Bus)
#ifdef CONFIG_CUSTOM_PERIPH_RC522_SPI_SCK
#define RC522_SPI_SCK_PIN  ((gpio_num_t)CONFIG_CUSTOM_PERIPH_RC522_SPI_SCK)
#define RC522_SPI_MOSI_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_RC522_SPI_MOSI)
#define RC522_SPI_MISO_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_RC522_SPI_MISO)
#define RC522_SPI_CS_PIN   ((gpio_num_t)CONFIG_CUSTOM_PERIPH_RC522_SPI_CS)
#else
#define RC522_SPI_SCK_PIN  ((gpio_num_t)-1)
#define RC522_SPI_MOSI_PIN ((gpio_num_t)-1)
#define RC522_SPI_MISO_PIN ((gpio_num_t)-1)
#define RC522_SPI_CS_PIN   ((gpio_num_t)-1)
#endif
#if defined(CONFIG_CUSTOM_PERIPH_RC522_RST_PIN) && (CONFIG_CUSTOM_PERIPH_RC522_RST_PIN >= 0)
#define RC522_RST_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_RC522_RST_PIN)
#else
#define RC522_RST_PIN ((gpio_num_t)-1)
#endif

// Giám sát trạng thái sạc pin TP4056
#ifdef CONFIG_CUSTOM_PERIPH_BATTERY_CHRG_PIN
#define BATTERY_CHRG_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_BATTERY_CHRG_PIN)
#else
#define BATTERY_CHRG_PIN ((gpio_num_t)-1)
#endif

// Giao tiếp mạng công nghiệp CAN Bus / TWAI Controller
#ifdef CONFIG_CUSTOM_PERIPH_TWAI_TX_PIN
#define PERIPH_TWAI_TX_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_TWAI_TX_PIN)
#define PERIPH_TWAI_RX_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_TWAI_RX_PIN)
#else
#define PERIPH_TWAI_TX_PIN ((gpio_num_t)-1)
#define PERIPH_TWAI_RX_PIN ((gpio_num_t)-1)
#endif

// Module mạng di động 4G LTE Cat.1
#ifdef CONFIG_CUSTOM_PERIPH_4G_UART_TX_PIN
#define PERIPH_4G_UART_TX_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_4G_UART_TX_PIN)
#define PERIPH_4G_UART_RX_PIN ((gpio_num_t)CONFIG_CUSTOM_PERIPH_4G_UART_RX_PIN)
#define PERIPH_4G_PWRKEY_PIN  ((gpio_num_t)CONFIG_CUSTOM_PERIPH_4G_PWRKEY_PIN)
#else
#define PERIPH_4G_UART_TX_PIN ((gpio_num_t)-1)
#define PERIPH_4G_UART_RX_PIN ((gpio_num_t)-1)
#define PERIPH_4G_PWRKEY_PIN  ((gpio_num_t)-1)
#endif

// ==============================================================================
// ⚠️ BẢO VỆ PHẦN CỨNG ESP32-S3 N16R8 (Octal PSRAM & Quad Flash Interconnect):
// Module ESP32-S3-WROOM-1-N16R8 sử dụng các chân sau kết nối bộ nhớ nội bộ:
// - GPIO 26: SPICS1 (PSRAM Chip Select)
// - GPIO 27, 28, 31, 32: SPI D4..D7 / D0..D1 (Flash & PSRAM Bus)
// - GPIO 29: SPICS0 (Flash Chip Select)
// - GPIO 30: SPICLK (PSRAM Clock)
// - GPIO 33..37: Octal PSRAM IO4..IO7 & DQS Clock Data Strobe
// TUYỆT ĐỐI KHÔNG GÁN BẤT KỲ CHÂN NÀO TỪ GPIO 26 ĐẾN GPIO 37 CHO NGOẠI VI!
// ==============================================================================
#define ESP32S3_N16R8_IS_RESERVED_PIN(p) ((p) >= 26 && (p) <= 37)

#endif // _ESP32S3_N16R8_CUSTOM_CONFIG_H_
