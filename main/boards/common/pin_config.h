/**
 * @file pin_config.h
 * @brief Centralized Hardware Pin Mapping for Xiaozhi ESP32-S3
 */

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <sdkconfig.h>
#include <driver/gpio.h>

// 1. Tương tác & Đầu vào (Inputs)
#define PIN_BUTTON_BOOT        GPIO_NUM_0
#define PIN_BUTTON_WAKE        GPIO_NUM_47
#define PIN_ROTARY_ENCODER_A   GPIO_NUM_17
#define PIN_ROTARY_ENCODER_B   GPIO_NUM_18
#define PIN_ROTARY_ENCODER_KEY GPIO_NUM_21

// 2. Màn hình & Cảm ứng (Display & Touch)
#define PIN_SPI_MOSI           GPIO_NUM_47
#define PIN_SPI_SCLK           GPIO_NUM_21
#define PIN_LCD_CS             GPIO_NUM_41
#define PIN_LCD_DC             GPIO_NUM_40
#define PIN_LCD_RST            GPIO_NUM_42
#define PIN_LCD_BL             GPIO_NUM_38

#define PIN_I2C_SDA            GPIO_NUM_8
#define PIN_I2C_SCL            GPIO_NUM_9
#define PIN_TOUCH_INT          GPIO_NUM_3
#define PIN_TOUCH_RST          GPIO_NUM_21

// 3. Âm thanh (Audio Speaker & Microphone)
#define PIN_I2S_SPK_BCLK       GPIO_NUM_15
#define PIN_I2S_SPK_LRCK       GPIO_NUM_16
#define PIN_I2S_SPK_DOUT       GPIO_NUM_7

#define PIN_I2S_MIC_SCK        GPIO_NUM_5
#define PIN_I2S_MIC_WS         GPIO_NUM_4
#define PIN_I2S_MIC_DIN        GPIO_NUM_6

// 4. Thị giác & Ánh sáng LED (Lighting & Vision)
#define PIN_LED_SINGLE         GPIO_NUM_38
#define PIN_LED_RGB_WS2812     GPIO_NUM_48

// 5. Cảm biến & Cơ cấu chấp hành (Sensors & Actuators)
#define PIN_RELAY              GPIO_NUM_45
#define PIN_SERVO_PWM          GPIO_NUM_13
#define PIN_BUZZER             GPIO_NUM_41
#define PIN_HAPTIC_MOTOR       GPIO_NUM_42
#define PIN_SENSOR_DS18B20     GPIO_NUM_4
#define PIN_SENSOR_FLOW        GPIO_NUM_5
#define PIN_SENSOR_MQ_GAS      GPIO_NUM_1
#define PIN_SENSOR_PIR         GPIO_NUM_14
#define PIN_RADAR_UART_TX      GPIO_NUM_43
#define PIN_RADAR_UART_RX      GPIO_NUM_44
#define PIN_ULTRASONIC_TRIG    GPIO_NUM_40
#define PIN_ULTRASONIC_ECHO    GPIO_NUM_39
#define PIN_SENSOR_SW420       GPIO_NUM_6
#define PIN_SENSOR_FLAME       GPIO_NUM_7

// 6. Lưu trữ & Thẻ từ (Storage & NFC/RFID)
#define PIN_SD_SPI_CS          GPIO_NUM_10
#define PIN_RFID_SPI_CS        GPIO_NUM_21
#define PIN_NFC_IRQ            GPIO_NUM_17
#define PIN_NFC_RST            GPIO_NUM_16

// 7. Động cơ DC & Nguồn (Motors & Power)
#define PIN_MOTOR_PWMA         GPIO_NUM_1
#define PIN_MOTOR_DIRA         GPIO_NUM_2
#define PIN_MOTOR_PWMB         GPIO_NUM_41
#define PIN_MOTOR_DIRB         GPIO_NUM_42
#define PIN_BATTERY_ADC        GPIO_NUM_1
#define PIN_TP4056_CHRG        GPIO_NUM_3
#define PIN_TWAI_TX            GPIO_NUM_15
#define PIN_TWAI_RX            GPIO_NUM_16
#define PIN_CELLULAR_4G_TX     GPIO_NUM_43
#define PIN_CELLULAR_4G_RX     GPIO_NUM_44
#define PIN_CELLULAR_4G_PWR    GPIO_NUM_2

#endif // PIN_CONFIG_H
