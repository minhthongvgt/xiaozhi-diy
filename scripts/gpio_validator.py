#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Bộ kiểm định phần cứng và chân GPIO cho bo mạch ESP32-S3 N16R8 Custom (Xiaozhi).
Phát hiện:
1. GPIO xung đột trùng lặp giữa 2 thiết bị khác nhau (cho phép I2C bus sharing và I2S duplex clock).
2. GPIO 26–37 bị cấm tuyệt đối trên ESP32-S3 N16R8 (chân dùng cho Flash và Octal PSRAM).
3. Cảnh báo các chân strapping / USB nhạy cảm (GPIO 0, 3, 45, 46, 19, 20).
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys
from typing import Dict, List, Optional, Set, Tuple, Union

if sys.platform == "win32":
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
        sys.stderr.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass

# Dải GPIO cấm tuyệt đối trên ESP32-S3 N16R8 (Flash & Octal PSRAM bus)
FORBIDDEN_GPIOS_N16R8: Set[int] = set(range(26, 38))  # 26 đến 37

# Các chân Strapping / JTAG / Native USB
STRAPPING_PINS: Dict[int, str] = {
    0: "Strapping Pin (Boot Mode / Nút BOOT - Cần mức HIGH khi boot bình thường)",
    3: "Strapping Pin (JTAG / Boot control)",
    45: "Strapping Pin (VDD_SPI voltage select - Cần đặc biệt cẩn trọng)",
    46: "Strapping Pin (ROM log printing / Boot mode - Chỉ nhận input khi reset)",
}

USB_PINS: Dict[int, str] = {
    19: "Chân USB D- (Native USB OTG / Serial JTAG)",
    20: "Chân USB D+ (Native USB OTG / Serial JTAG)",
}


def parse_sdkconfig_file(filepath: Union[str, Path]) -> Dict[str, str]:
    """Đọc file sdkconfig và trích xuất cặp key-value CONFIG_*."""
    config: Dict[str, str] = {}
    path = Path(filepath)
    if not path.is_file():
        return config

    with open(path, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if "=" in line:
                key, val = line.split("=", 1)
                key = key.strip()
                val = val.strip().strip('"')
                config[key] = val
    return config


def _get_int(config: Dict[str, str], key: str, default: int = -1) -> int:
    val = config.get(key)
    if val is None:
        return default
    try:
        return int(val)
    except ValueError:
        return default


def _is_yes(config: Dict[str, str], key: str) -> bool:
    return config.get(key) == "y"


class PinAssignment:
    def __init__(self, device: str, pin: int, pin_type: str = "GPIO"):
        self.device = device
        self.pin = pin
        self.pin_type = pin_type  # "GPIO", "I2C_SDA", "I2C_SCL", "I2S_BCLK", "I2S_WS", "USB_DM", "USB_DP"

    def __repr__(self) -> str:
        return f"PinAssignment({self.device}, GPIO={self.pin}, type={self.pin_type})"


def extract_pin_assignments(config: Dict[str, str]) -> List[PinAssignment]:
    """Trích xuất danh sách tất cả các chân GPIO được kích hoạt trong cấu hình."""
    assignments: List[PinAssignment] = []

    is_duplex = _is_yes(config, "CONFIG_CUSTOM_AUDIO_I2S_DUPLEX")

    # 1. Màn hình (Display)
    if _is_yes(config, "CONFIG_ENABLE_CUSTOM_DISPLAY"):
        is_oled = _is_yes(config, "CONFIG_CUSTOM_DISPLAY_OLED_SSD1306") or _is_yes(
            config, "CONFIG_CUSTOM_DISPLAY_OLED_SH1106"
        )
        is_qspi = _is_yes(config, "CONFIG_CUSTOM_DISPLAY_QSPI_AMOLED")

        if is_oled:
            sda = _get_int(config, "CONFIG_CUSTOM_DISPLAY_PIN_I2C_SDA", 8)
            scl = _get_int(config, "CONFIG_CUSTOM_DISPLAY_PIN_I2C_SCL", 9)
            assignments.append(PinAssignment("Màn hình OLED (SDA)", sda, "I2C_SDA"))
            assignments.append(PinAssignment("Màn hình OLED (SCL)", scl, "I2C_SCL"))
        elif is_qspi:
            assignments.append(
                PinAssignment("Màn hình AMOLED QSPI (CS)", _get_int(config, "CONFIG_CUSTOM_DISPLAY_QSPI_CS", 10))
            )
            assignments.append(
                PinAssignment("Màn hình AMOLED QSPI (CLK)", _get_int(config, "CONFIG_CUSTOM_DISPLAY_QSPI_CLK", 15))
            )
            assignments.append(
                PinAssignment("Màn hình AMOLED QSPI (D0)", _get_int(config, "CONFIG_CUSTOM_DISPLAY_QSPI_D0", 11))
            )
            assignments.append(
                PinAssignment("Màn hình AMOLED QSPI (D1)", _get_int(config, "CONFIG_CUSTOM_DISPLAY_QSPI_D1", 12))
            )
            assignments.append(
                PinAssignment("Màn hình AMOLED QSPI (D2)", _get_int(config, "CONFIG_CUSTOM_DISPLAY_QSPI_D2", 13))
            )
            assignments.append(
                PinAssignment("Màn hình AMOLED QSPI (D3)", _get_int(config, "CONFIG_CUSTOM_DISPLAY_QSPI_D3", 14))
            )
            assignments.append(
                PinAssignment("Màn hình AMOLED QSPI (RST)", _get_int(config, "CONFIG_CUSTOM_DISPLAY_QSPI_RST", 16))
            )
        else:
            # SPI Display (ST7789, ST7796, ST7701, ILI9341, GC9A01...)
            mosi = _get_int(config, "CONFIG_CUSTOM_DISPLAY_PIN_MOSI", 47)
            clk = _get_int(config, "CONFIG_CUSTOM_DISPLAY_PIN_CLK", 21)
            cs = _get_int(config, "CONFIG_CUSTOM_DISPLAY_PIN_CS", 41)
            dc = _get_int(config, "CONFIG_CUSTOM_DISPLAY_PIN_DC", 40)
            assignments.append(PinAssignment("Màn hình SPI (MOSI)", mosi))
            assignments.append(PinAssignment("Màn hình SPI (CLK)", clk))
            assignments.append(PinAssignment("Màn hình SPI (CS)", cs))
            assignments.append(PinAssignment("Màn hình SPI (DC)", dc))

            if _is_yes(config, "CONFIG_CUSTOM_DISPLAY_USE_RST"):
                rst = _get_int(config, "CONFIG_CUSTOM_DISPLAY_PIN_RST", 42)
                assignments.append(PinAssignment("Màn hình SPI (RST)", rst))

            if _is_yes(config, "CONFIG_CUSTOM_DISPLAY_USE_BLK"):
                blk = _get_int(config, "CONFIG_CUSTOM_DISPLAY_PIN_BLK", 38)
                assignments.append(PinAssignment("Màn hình SPI (Đèn nền BLK)", blk))

    # Màn hình cảm ứng (Touch)
    if _is_yes(config, "CONFIG_ENABLE_CUSTOM_TOUCH"):
        sda = _get_int(config, "CONFIG_CUSTOM_TOUCH_PIN_SDA", 8)
        scl = _get_int(config, "CONFIG_CUSTOM_TOUCH_PIN_SCL", 9)
        int_pin = _get_int(config, "CONFIG_CUSTOM_TOUCH_PIN_INT", 3)
        rst_pin = _get_int(config, "CONFIG_CUSTOM_TOUCH_PIN_RST", 2)
        assignments.append(PinAssignment("Màn hình cảm ứng (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("Màn hình cảm ứng (SCL)", scl, "I2C_SCL"))
        assignments.append(PinAssignment("Màn hình cảm ứng (INT)", int_pin))
        assignments.append(PinAssignment("Màn hình cảm ứng (RST)", rst_pin))

    # 2. Loa phát thanh & Mạch DAC (Speaker)
    if _is_yes(config, "CONFIG_ENABLE_CUSTOM_SPEAKER"):
        spk_dout = _get_int(config, "CONFIG_CUSTOM_AUDIO_SPK_GPIO_DOUT", 7)
        spk_bclk = _get_int(config, "CONFIG_CUSTOM_AUDIO_SPK_GPIO_BCLK", 15)
        spk_lrck = _get_int(config, "CONFIG_CUSTOM_AUDIO_SPK_GPIO_LRCK", 16)
        assignments.append(PinAssignment("Loa phát thanh (DOUT)", spk_dout))
        assignments.append(
            PinAssignment("Loa phát thanh (BCLK)", spk_bclk, "I2S_BCLK" if is_duplex else "GPIO")
        )
        assignments.append(
            PinAssignment("Loa phát thanh (LRCK/WS)", spk_lrck, "I2S_WS" if is_duplex else "GPIO")
        )

        is_codec_spk = any(
            _is_yes(config, k)
            for k in [
                "CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8311",
                "CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8388",
                "CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8374",
                "CONFIG_CUSTOM_AUDIO_SPK_CODEC_ES8389",
                "CONFIG_CUSTOM_AUDIO_SPK_CODEC_BOX",
                "CONFIG_CUSTOM_AUDIO_DAC_TAS5805M",
                "CONFIG_CUSTOM_AUDIO_DAC_AW88298",
            ]
        )
        if is_codec_spk:
            sda = _get_int(config, "CONFIG_CUSTOM_AUDIO_SPK_CODEC_I2C_SDA", 8)
            scl = _get_int(config, "CONFIG_CUSTOM_AUDIO_SPK_CODEC_I2C_SCL", 9)
            assignments.append(PinAssignment("Loa Codec DAC (I2C SDA)", sda, "I2C_SDA"))
            assignments.append(PinAssignment("Loa Codec DAC (I2C SCL)", scl, "I2C_SCL"))

    # Microphone thu âm
    if _is_yes(config, "CONFIG_ENABLE_CUSTOM_MIC"):
        is_pdm = _is_yes(config, "CONFIG_CUSTOM_AUDIO_MIC_PDM")
        mic_din = _get_int(config, "CONFIG_CUSTOM_AUDIO_MIC_GPIO_DIN", 6)
        mic_sck = _get_int(config, "CONFIG_CUSTOM_AUDIO_MIC_GPIO_SCK", 5)
        mic_ws = _get_int(config, "CONFIG_CUSTOM_AUDIO_MIC_GPIO_WS", 4)

        if is_pdm:
            assignments.append(PinAssignment("Microphone PDM (DATA)", mic_din))
            assignments.append(PinAssignment("Microphone PDM (CLK)", mic_sck))
        else:
            assignments.append(PinAssignment("Microphone I2S (DIN)", mic_din))
            assignments.append(
                PinAssignment("Microphone I2S (SCK)", mic_sck, "I2S_BCLK" if is_duplex else "GPIO")
            )
            assignments.append(
                PinAssignment("Microphone I2S (WS)", mic_ws, "I2S_WS" if is_duplex else "GPIO")
            )

        is_codec_mic = any(
            _is_yes(config, k)
            for k in [
                "CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8311",
                "CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8388",
                "CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8374",
                "CONFIG_CUSTOM_AUDIO_MIC_CODEC_ES8389",
                "CONFIG_CUSTOM_AUDIO_MIC_CODEC_BOX",
                "CONFIG_CUSTOM_AUDIO_MIC_ADC_ES7210",
                "CONFIG_CUSTOM_AUDIO_MIC_ADC_ES7243E",
            ]
        )
        if is_codec_mic:
            sda = _get_int(config, "CONFIG_CUSTOM_AUDIO_MIC_CODEC_I2C_SDA", 8)
            scl = _get_int(config, "CONFIG_CUSTOM_AUDIO_MIC_CODEC_I2C_SCL", 9)
            assignments.append(PinAssignment("Microphone Codec ADC (I2C SDA)", sda, "I2C_SDA"))
            assignments.append(PinAssignment("Microphone Codec ADC (I2C SCL)", scl, "I2C_SCL"))

    # 3. Giao thức truyền thông & Mạng (Communication & Network)
    if _is_yes(config, "CONFIG_ENABLE_CUSTOM_UART"):
        tx = _get_int(config, "CONFIG_CUSTOM_UART_PIN_TX", 17)
        rx = _get_int(config, "CONFIG_CUSTOM_UART_PIN_RX", 18)
        assignments.append(PinAssignment("Cổng UART mở rộng (TX)", tx))
        assignments.append(PinAssignment("Cổng UART mở rộng (RX)", rx))
        if _is_yes(config, "CONFIG_CUSTOM_UART_USE_FLOW_CONTROL"):
            rts = _get_int(config, "CONFIG_CUSTOM_UART_PIN_RTS", 19)
            cts = _get_int(config, "CONFIG_CUSTOM_UART_PIN_CTS", 20)
            assignments.append(PinAssignment("Cổng UART mở rộng (RTS)", rts))
            assignments.append(PinAssignment("Cổng UART mở rộng (CTS)", cts))

    if _is_yes(config, "CONFIG_ENABLE_CUSTOM_SECONDARY_NETWORK"):
        is_4g = _is_yes(config, "CONFIG_CUSTOM_NETWORK_4G_ML307") or _is_yes(
            config, "CONFIG_CUSTOM_NETWORK_4G_NT26"
        )
        if is_4g:
            tx = _get_int(config, "CONFIG_CUSTOM_MODEM_UART_TX_PIN", 17)
            rx = _get_int(config, "CONFIG_CUSTOM_MODEM_UART_RX_PIN", 18)
            pwrkey = _get_int(config, "CONFIG_CUSTOM_MODEM_PWRKEY_PIN", 2)
            assignments.append(PinAssignment("Modem 4G (TX)", tx))
            assignments.append(PinAssignment("Modem 4G (RX)", rx))
            assignments.append(PinAssignment("Modem 4G (PWRKEY)", pwrkey))

        is_eth = _is_yes(config, "CONFIG_CUSTOM_NETWORK_ETH_W5500") or _is_yes(
            config, "CONFIG_CUSTOM_NETWORK_ETH_DM9051"
        )
        if is_eth:
            cs = _get_int(config, "CONFIG_CUSTOM_ETH_SPI_CS_PIN", 10)
            int_pin = _get_int(config, "CONFIG_CUSTOM_ETH_SPI_INT_PIN", 11)
            rst = _get_int(config, "CONFIG_CUSTOM_ETH_SPI_RST_PIN", 14)
            assignments.append(PinAssignment("Ethernet SPI (CS)", cs))
            assignments.append(PinAssignment("Ethernet SPI (INT)", int_pin))
            assignments.append(PinAssignment("Ethernet SPI (RST)", rst))

    if _is_yes(config, "CONFIG_ENABLE_CUSTOM_MCP_SERVER") and _is_yes(config, "CONFIG_CUSTOM_MCP_TOOL_LAMP"):
        lamp = _get_int(config, "CONFIG_CUSTOM_MCP_TOOL_LAMP_GPIO", 13)
        assignments.append(PinAssignment("Đèn thông minh MCP (Relay)", lamp))

    # 4. Ngoại vi & Cảm biến (Peripherals & Sensors)
    # Camera
    if _is_yes(config, "CONFIG_ENABLE_CUSTOM_CAMERA"):
        if _is_yes(config, "CONFIG_CUSTOM_CAMERA_USB_UVC"):
            assignments.append(PinAssignment("Camera USB UVC (D-)", 19, "USB_DM"))
            assignments.append(PinAssignment("Camera USB UVC (D+)", 20, "USB_DP"))
        else:
            assignments.append(
                PinAssignment("Camera DVP (XCLK)", _get_int(config, "CONFIG_CUSTOM_CAM_PIN_XCLK", 10))
            )
            assignments.append(
                PinAssignment("Camera DVP (PCLK)", _get_int(config, "CONFIG_CUSTOM_CAM_PIN_PCLK", 12))
            )
            assignments.append(
                PinAssignment("Camera DVP (VSYNC)", _get_int(config, "CONFIG_CUSTOM_CAM_PIN_VSYNC", 13))
            )
            assignments.append(
                PinAssignment("Camera DVP (HREF)", _get_int(config, "CONFIG_CUSTOM_CAM_PIN_HREF", 14))
            )
            assignments.append(
                PinAssignment("Camera DVP (SIOD)", _get_int(config, "CONFIG_CUSTOM_CAM_PIN_SIOD", 8), "I2C_SDA")
            )
            assignments.append(
                PinAssignment("Camera DVP (SIOC)", _get_int(config, "CONFIG_CUSTOM_CAM_PIN_SIOC", 9), "I2C_SCL")
            )
            # Camera D0..D7
            cam_d_pins = [11, 2, 46, 21, 47, 48, 45, 40]
            for i in range(8):
                pin = _get_int(config, f"CONFIG_CUSTOM_CAM_PIN_D{i}", cam_d_pins[i])
                assignments.append(PinAssignment(f"Camera DVP (D{i})", pin))
            reset_pin = _get_int(config, "CONFIG_CUSTOM_CAM_PIN_RESET", -1)
            if reset_pin >= 0:
                assignments.append(PinAssignment("Camera DVP (RESET)", reset_pin))
            pwdn_pin = _get_int(config, "CONFIG_CUSTOM_CAM_PIN_PWDN", -1)
            if pwdn_pin >= 0:
                assignments.append(PinAssignment("Camera DVP (PWDN)", pwdn_pin))

    # LEDs
    if _is_yes(config, "CONFIG_ENABLE_CUSTOM_LEDS"):
        led_pin = _get_int(config, "CONFIG_CUSTOM_LED_GPIO", 48)
        if _is_yes(config, "CONFIG_CUSTOM_LED_WS2812"):
            assignments.append(PinAssignment("Đèn LED RGB WS2812", led_pin))
        elif _is_yes(config, "CONFIG_CUSTOM_LED_SINGLE_PWM"):
            assignments.append(PinAssignment("Đèn LED đơn PWM", led_pin))
        elif _is_yes(config, "CONFIG_CUSTOM_LED_CIRCULAR_STRIP"):
            assignments.append(PinAssignment("Vòng tròn LED xoay", led_pin))

    # Servo Dog Controller (Hỗ trợ cả độc lập và legacy lồng)
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SERVO_DOG"):
        servo_pin = _get_int(config, "CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO", 48)
        assignments.append(PinAssignment("Động cơ Servo PWM", servo_pin))

    # Còi báo Buzzer
    if _is_yes(config, "CONFIG_ENABLE_BUZZER"):
        buzzer_pin = _get_int(config, "CONFIG_BUZZER_PIN", 41)
        assignments.append(PinAssignment("Còi báo Buzzer", buzzer_pin))

    # Động cơ rung phản hồi xúc giác Haptic Motor
    if _is_yes(config, "CONFIG_ENABLE_HAPTIC_MOTOR"):
        haptic_pin = _get_int(config, "CONFIG_HAPTIC_PIN", 42)
        assignments.append(PinAssignment("Động cơ rung Haptic", haptic_pin))

    # Buttons
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_BUTTON_BOOT"):
        assignments.append(
            PinAssignment("Nút bấm BOOT Onboard", _get_int(config, "CONFIG_CUSTOM_BUTTON_BOOT_GPIO", 0))
        )
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_BUTTON_TOUCH"):
        assignments.append(
            PinAssignment("Nút bấm Action/Chạm", _get_int(config, "CONFIG_CUSTOM_BUTTON_TOUCH_GPIO", 1))
        )
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_BUTTON_VOLUME"):
        assignments.append(
            PinAssignment("Phím Tăng âm lượng", _get_int(config, "CONFIG_CUSTOM_BUTTON_VOL_UP_GPIO", 2))
        )
        assignments.append(
            PinAssignment("Phím Giảm âm lượng", _get_int(config, "CONFIG_CUSTOM_BUTTON_VOL_DOWN_GPIO", 3))
        )

    # Relay
    if _is_yes(config, "CONFIG_CUSTOM_PERIPH_RELAY_ENABLE"):
        assignments.append(
            PinAssignment("Rơ-le điều khiển thiết bị", _get_int(config, "CONFIG_CUSTOM_PERIPH_RELAY_GPIO", 13))
        )

    # IO Expander (Độc lập hoặc Legacy)
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_IO_EXPANDER") or (
        _is_yes(config, "CONFIG_ENABLE_CUSTOM_IO_EXPANDER_PMIC") and not _is_yes(config, "CONFIG_CUSTOM_IO_EXPANDER_NONE")
    ):
        sda = _get_int(config, "CONFIG_CUSTOM_EXPANDER_I2C_SDA", 8)
        scl = _get_int(config, "CONFIG_CUSTOM_EXPANDER_I2C_SCL", 9)
        assignments.append(PinAssignment("IC Mở rộng IO I2C (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("IC Mở rộng IO I2C (SCL)", scl, "I2C_SCL"))
        int_pin = _get_int(config, "CONFIG_CUSTOM_EXPANDER_INT_PIN", -1)
        if int_pin >= 0:
            assignments.append(PinAssignment("IC Mở rộng IO (INT)", int_pin))
        rst_pin = _get_int(config, "CONFIG_CUSTOM_EXPANDER_RST_PIN", -1)
        if rst_pin >= 0:
            assignments.append(PinAssignment("IC Mở rộng IO (RST)", rst_pin))

    # PMIC (Độc lập hoặc Legacy)
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PMIC") or (
        _is_yes(config, "CONFIG_ENABLE_CUSTOM_IO_EXPANDER_PMIC") and not _is_yes(config, "CONFIG_CUSTOM_PMIC_NONE")
    ):
        sda = _get_int(config, "CONFIG_CUSTOM_PMIC_I2C_SDA", _get_int(config, "CONFIG_CUSTOM_EXPANDER_I2C_SDA", 8))
        scl = _get_int(config, "CONFIG_CUSTOM_PMIC_I2C_SCL", _get_int(config, "CONFIG_CUSTOM_EXPANDER_I2C_SCL", 9))
        assignments.append(PinAssignment("IC Quản lý nguồn PMIC (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("IC Quản lý nguồn PMIC (SCL)", scl, "I2C_SCL"))
        int_pin = _get_int(config, "CONFIG_CUSTOM_PMIC_INT_PIN", -1)
        if int_pin >= 0:
            assignments.append(PinAssignment("IC Quản lý nguồn PMIC (INT)", int_pin))

    # Sensors
    sensors_active = _is_yes(config, "CONFIG_ENABLE_CUSTOM_SENSORS")

    # IMU
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_IMU_SENSORS") or _is_yes(config, "CONFIG_CUSTOM_ENABLE_IMU_MPU6050") or _is_yes(config, "CONFIG_CUSTOM_ENABLE_IMU_BMI270"):
        sda = _get_int(config, "CONFIG_CUSTOM_IMU_I2C_SDA", 8)
        scl = _get_int(config, "CONFIG_CUSTOM_IMU_I2C_SCL", 9)
        assignments.append(PinAssignment("Cảm biến IMU 6 trục (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("Cảm biến IMU 6 trục (SCL)", scl, "I2C_SCL"))

    # DHT11 / DHT22
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_DHT11_22") or (sensors_active and "CONFIG_CUSTOM_SENSOR_DHT_GPIO" in config):
        dht = _get_int(config, "CONFIG_CUSTOM_SENSOR_DHT_GPIO", 14)
        assignments.append(PinAssignment("Cảm biến Nhiệt ẩm DHT11/22", dht))

    # I2C Temp/Humid (AHT20, SHT3x)
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_I2C_TEMP_HUMID") or _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_AHT20") or _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_SHT3X"):
        sda = _get_int(config, "CONFIG_CUSTOM_SENSOR_I2C_SDA", 8)
        scl = _get_int(config, "CONFIG_CUSTOM_SENSOR_I2C_SCL", 9)
        assignments.append(PinAssignment("Cảm biến nhiệt ẩm I2C (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("Cảm biến nhiệt ẩm I2C (SCL)", scl, "I2C_SCL"))

    # BMP280 / BME280
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_BMP280"):
        sda = _get_int(config, "CONFIG_CUSTOM_SENSOR_BMP280_I2C_SDA", _get_int(config, "CONFIG_CUSTOM_SENSOR_I2C_SDA", 8))
        scl = _get_int(config, "CONFIG_CUSTOM_SENSOR_BMP280_I2C_SCL", _get_int(config, "CONFIG_CUSTOM_SENSOR_I2C_SCL", 9))
        assignments.append(PinAssignment("Cảm biến khí áp BMP280 (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("Cảm biến khí áp BMP280 (SCL)", scl, "I2C_SCL"))

    # BH1750
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_BH1750"):
        sda = _get_int(config, "CONFIG_CUSTOM_SENSOR_BH1750_I2C_SDA", _get_int(config, "CONFIG_CUSTOM_SENSOR_I2C_SDA", 8))
        scl = _get_int(config, "CONFIG_CUSTOM_SENSOR_BH1750_I2C_SCL", _get_int(config, "CONFIG_CUSTOM_SENSOR_I2C_SCL", 9))
        assignments.append(PinAssignment("Cảm biến ánh sáng BH1750 (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("Cảm biến ánh sáng BH1750 (SCL)", scl, "I2C_SCL"))

    # HC-SR04
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_HCSR04"):
        trig = _get_int(config, "CONFIG_CUSTOM_SENSOR_HCSR04_TRIG_GPIO", 11)
        echo = _get_int(config, "CONFIG_CUSTOM_SENSOR_HCSR04_ECHO_GPIO", 12)
        assignments.append(PinAssignment("Cảm biến siêu âm (TRIG)", trig))
        assignments.append(PinAssignment("Cảm biến siêu âm (ECHO)", echo))

    # PIR
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_PIR"):
        pir = _get_int(config, "CONFIG_CUSTOM_SENSOR_PIR_GPIO", 10)
        assignments.append(PinAssignment("Cảm biến chuyển động PIR", pir))

    # Touch Slider
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_TOUCH_SLIDER"):
        pad1 = _get_int(config, "CONFIG_CUSTOM_TOUCH_SLIDER_PAD1_GPIO", 1)
        pad2 = _get_int(config, "CONFIG_CUSTOM_TOUCH_SLIDER_PAD2_GPIO", 2)
        pad3 = _get_int(config, "CONFIG_CUSTOM_TOUCH_SLIDER_PAD3_GPIO", 3)
        assignments.append(PinAssignment("Thanh trượt cảm ứng (Pad 1)", pad1))
        assignments.append(PinAssignment("Thanh trượt cảm ứng (Pad 2)", pad2))
        assignments.append(PinAssignment("Thanh trượt cảm ứng (Pad 3)", pad3))

    # Battery IC BQ27220
    if _is_yes(config, "CONFIG_CUSTOM_BATTERY_MONITOR_BQ27220"):
        sda = _get_int(config, "CONFIG_CUSTOM_BATTERY_BQ27220_I2C_SDA", _get_int(config, "CONFIG_CUSTOM_SENSOR_I2C_SDA", 8))
        scl = _get_int(config, "CONFIG_CUSTOM_BATTERY_BQ27220_I2C_SCL", _get_int(config, "CONFIG_CUSTOM_SENSOR_I2C_SCL", 9))
        assignments.append(PinAssignment("IC đo pin BQ27220 (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("IC đo pin BQ27220 (SCL)", scl, "I2C_SCL"))

    # Cảm biến Khí / CO2 (SCD40/41 hoặc SGP30/40)
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_GAS_CO2"):
        sda = _get_int(config, "CONFIG_CUSTOM_SENSOR_GAS_I2C_SDA", 8)
        scl = _get_int(config, "CONFIG_CUSTOM_SENSOR_GAS_I2C_SCL", 9)
        assignments.append(PinAssignment("Cảm biến Khí/CO2 (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("Cảm biến Khí/CO2 (SCL)", scl, "I2C_SCL"))

    # Cảm biến cử chỉ APDS-9960
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_APDS9960"):
        sda = _get_int(config, "CONFIG_CUSTOM_SENSOR_APDS9960_I2C_SDA", 8)
        scl = _get_int(config, "CONFIG_CUSTOM_SENSOR_APDS9960_I2C_SCL", 9)
        assignments.append(PinAssignment("Cảm biến cử chỉ APDS-9960 (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("Cảm biến cử chỉ APDS-9960 (SCL)", scl, "I2C_SCL"))
        int_pin = _get_int(config, "CONFIG_CUSTOM_SENSOR_APDS9960_INT_PIN", -1)
        if int_pin >= 0:
            assignments.append(PinAssignment("Cảm biến cử chỉ APDS-9960 (INT)", int_pin))

    # Cảm biến khoảng cách Laser ToF VL53L0X / VL53L1X
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_VL53LX"):
        sda = _get_int(config, "CONFIG_CUSTOM_SENSOR_VL53LX_I2C_SDA", 8)
        scl = _get_int(config, "CONFIG_CUSTOM_SENSOR_VL53LX_I2C_SCL", 9)
        assignments.append(PinAssignment("Cảm biến Laser ToF (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("Cảm biến Laser ToF (SCL)", scl, "I2C_SCL"))
        xshut = _get_int(config, "CONFIG_CUSTOM_SENSOR_VL53LX_XSHUT_PIN", -1)
        if xshut >= 0:
            assignments.append(PinAssignment("Cảm biến Laser ToF (XSHUT)", xshut))

    # Module thẻ thông minh NFC PN532
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PERIPH_NFC_PN532"):
        sda = _get_int(config, "CONFIG_CUSTOM_PERIPH_NFC_I2C_SDA", 8)
        scl = _get_int(config, "CONFIG_CUSTOM_PERIPH_NFC_I2C_SCL", 9)
        assignments.append(PinAssignment("Module NFC PN532 (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("Module NFC PN532 (SCL)", scl, "I2C_SCL"))
        irq = _get_int(config, "CONFIG_CUSTOM_PERIPH_NFC_IRQ_PIN", -1)
        if irq >= 0:
            assignments.append(PinAssignment("Module NFC PN532 (IRQ)", irq))
        rst = _get_int(config, "CONFIG_CUSTOM_PERIPH_NFC_RST_PIN", -1)
        if rst >= 0:
            assignments.append(PinAssignment("Module NFC PN532 (RST)", rst))

    # Đồng hồ thời gian thực RTC ngoại tuyến
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PERIPH_RTC"):
        sda = _get_int(config, "CONFIG_CUSTOM_PERIPH_RTC_I2C_SDA", 8)
        scl = _get_int(config, "CONFIG_CUSTOM_PERIPH_RTC_I2C_SCL", 9)
        assignments.append(PinAssignment("Đồng hồ RTC ngoại tuyến (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("Đồng hồ RTC ngoại tuyến (SCL)", scl, "I2C_SCL"))
        int_pin = _get_int(config, "CONFIG_CUSTOM_PERIPH_RTC_INT_PIN", -1)
        if int_pin >= 0:
            assignments.append(PinAssignment("Đồng hồ RTC ngoại tuyến (INT)", int_pin))

    # Rotary Encoder EC11
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PERIPH_ROTARY_ENCODER"):
        pha_a = _get_int(config, "CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_A", 17)
        pha_b = _get_int(config, "CONFIG_CUSTOM_PERIPH_ENCODER_PHASE_B", 18)
        assignments.append(PinAssignment("Rotary Encoder EC11 (Pha A)", pha_a))
        assignments.append(PinAssignment("Rotary Encoder EC11 (Pha B)", pha_b))
        key = _get_int(config, "CONFIG_CUSTOM_PERIPH_ENCODER_KEY_PIN", -1)
        if key >= 0:
            assignments.append(PinAssignment("Rotary Encoder EC11 (Phím SW)", key))

    # Hồng ngoại IR Remote Transceiver
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PERIPH_IR_REMOTE"):
        tx = _get_int(config, "CONFIG_CUSTOM_PERIPH_IR_TX_PIN", 17)
        assignments.append(PinAssignment("Hồng ngoại IR Remote (TX)", tx))
        rx = _get_int(config, "CONFIG_CUSTOM_PERIPH_IR_RX_PIN", -1)
        if rx >= 0:
            assignments.append(PinAssignment("Hồng ngoại IR Remote (RX)", rx))

    # Module đo năng lượng INA219 / INA226
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_INA2XX"):
        sda = _get_int(config, "CONFIG_CUSTOM_SENSOR_INA2XX_I2C_SDA", 8)
        scl = _get_int(config, "CONFIG_CUSTOM_SENSOR_INA2XX_I2C_SCL", 9)
        assignments.append(PinAssignment("Module đo năng lượng INA (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("Module đo năng lượng INA (SCL)", scl, "I2C_SCL"))

    # Thẻ nhớ MicroSD SPI
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PERIPH_SDCARD_SPI"):
        sck = _get_int(config, "CONFIG_CUSTOM_PERIPH_SDCARD_SPI_SCK", 12)
        mosi = _get_int(config, "CONFIG_CUSTOM_PERIPH_SDCARD_SPI_MOSI", 11)
        miso = _get_int(config, "CONFIG_CUSTOM_PERIPH_SDCARD_SPI_MISO", 13)
        cs = _get_int(config, "CONFIG_CUSTOM_PERIPH_SDCARD_SPI_CS", 10)
        assignments.append(PinAssignment("Thẻ nhớ MicroSD (SPI SCK)", sck, "SPI_SCK"))
        assignments.append(PinAssignment("Thẻ nhớ MicroSD (SPI MOSI)", mosi, "SPI_MOSI"))
        assignments.append(PinAssignment("Thẻ nhớ MicroSD (SPI MISO)", miso, "SPI_MISO"))
        assignments.append(PinAssignment("Thẻ nhớ MicroSD (SPI CS)", cs))

    # IC điều khiển LED AW9523B / IS31FL3731
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PERIPH_LED_DRIVER_IC"):
        sda = _get_int(config, "CONFIG_CUSTOM_PERIPH_LED_IC_I2C_SDA", 8)
        scl = _get_int(config, "CONFIG_CUSTOM_PERIPH_LED_IC_I2C_SCL", 9)
        assignments.append(PinAssignment("IC điều khiển LED (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("IC điều khiển LED (SCL)", scl, "I2C_SCL"))

    # 30. Màn hình cảm ứng rời I2C (CST816D/S, GT911, FT6236)
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PERIPH_TOUCH_SCREEN"):
        sda = _get_int(config, "CONFIG_CUSTOM_PERIPH_TOUCH_I2C_SDA", 8)
        scl = _get_int(config, "CONFIG_CUSTOM_PERIPH_TOUCH_I2C_SCL", 9)
        assignments.append(PinAssignment("Màn hình cảm ứng I2C (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("Màn hình cảm ứng I2C (SCL)", scl, "I2C_SCL"))
        int_pin = _get_int(config, "CONFIG_CUSTOM_PERIPH_TOUCH_INT_PIN", -1)
        if int_pin >= 0:
            assignments.append(PinAssignment("Màn hình cảm ứng I2C (INT)", int_pin))
        rst = _get_int(config, "CONFIG_CUSTOM_PERIPH_TOUCH_RST_PIN", -1)
        if rst >= 0:
            assignments.append(PinAssignment("Màn hình cảm ứng I2C (RST)", rst))

    # 31. Mạch mở rộng 16 kênh PWM cho Robot PCA9685
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PERIPH_PCA9685"):
        sda = _get_int(config, "CONFIG_CUSTOM_PERIPH_PCA9685_I2C_SDA", 8)
        scl = _get_int(config, "CONFIG_CUSTOM_PERIPH_PCA9685_I2C_SCL", 9)
        assignments.append(PinAssignment("Mạch PWM PCA9685 (SDA)", sda, "I2C_SDA"))
        assignments.append(PinAssignment("Mạch PWM PCA9685 (SCL)", scl, "I2C_SCL"))
        oe = _get_int(config, "CONFIG_CUSTOM_PERIPH_PCA9685_OE_PIN", -1)
        if oe >= 0:
            assignments.append(PinAssignment("Mạch PWM PCA9685 (OE)", oe))

    # 32. Mạch cầu H Động cơ DC 2 chiều TB6612FNG / L9110S / DRV8833
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PERIPH_MOTOR_DC_HBRIDGE"):
        pwma = _get_int(config, "CONFIG_CUSTOM_PERIPH_MOTOR_PWMA_PIN", 1)
        dira = _get_int(config, "CONFIG_CUSTOM_PERIPH_MOTOR_DIRA_PIN", 2)
        pwmb = _get_int(config, "CONFIG_CUSTOM_PERIPH_MOTOR_PWMB_PIN", 41)
        dirb = _get_int(config, "CONFIG_CUSTOM_PERIPH_MOTOR_DIRB_PIN", 42)
        assignments.append(PinAssignment("Động cơ DC Cầu H (PWMA)", pwma))
        assignments.append(PinAssignment("Động cơ DC Cầu H (DIRA)", dira))
        assignments.append(PinAssignment("Động cơ DC Cầu H (PWMB)", pwmb))
        assignments.append(PinAssignment("Động cơ DC Cầu H (DIRB)", dirb))

    # 33. Cảm biến nhiệt độ công nghiệp 1-Wire DS18B20
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_DS18B20"):
        dq = _get_int(config, "CONFIG_CUSTOM_SENSOR_DS18B20_PIN", 4)
        assignments.append(PinAssignment("Cảm biến nhiệt độ DS18B20", dq))

    # 34. Cảm biến lưu lượng chất lỏng đếm xung PCNT
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_FLOW_PCNT"):
        pulse = _get_int(config, "CONFIG_CUSTOM_SENSOR_FLOW_PULSE_PIN", 5)
        assignments.append(PinAssignment("Cảm biến lưu lượng PCNT", pulse))

    # 35. Cảm biến khói & khí gas dễ cháy MQ-2 / MQ-135
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_GAS_ANALOG_MQ"):
        aout = _get_int(config, "CONFIG_CUSTOM_SENSOR_MQ_ANALOG_PIN", 1)
        assignments.append(PinAssignment("Cảm biến khí gas MQ (ADC)", aout))

    # 36. Cảm biến cảnh báo chấn động / Rung SW-420
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_VIBRATION_SW420"):
        dout = _get_int(config, "CONFIG_CUSTOM_SENSOR_VIBRATION_PIN", 6)
        assignments.append(PinAssignment("Cảm biến rung SW-420", dout))

    # 37. Cảm biến cảnh báo hỏa hoạn / Lửa Flame Sensor
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_FLAME"):
        dout = _get_int(config, "CONFIG_CUSTOM_SENSOR_FLAME_PIN", 7)
        assignments.append(PinAssignment("Cảm biến ngọn lửa Flame", dout))

    # 38. Đầu đọc thẻ từ RFID 13.56MHz RC522 (SPI)
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PERIPH_RFID_RC522"):
        sck = _get_int(config, "CONFIG_CUSTOM_PERIPH_RC522_SPI_SCK", 12)
        mosi = _get_int(config, "CONFIG_CUSTOM_PERIPH_RC522_SPI_MOSI", 11)
        miso = _get_int(config, "CONFIG_CUSTOM_PERIPH_RC522_SPI_MISO", 13)
        cs = _get_int(config, "CONFIG_CUSTOM_PERIPH_RC522_SPI_CS", 21)
        assignments.append(PinAssignment("Đầu đọc thẻ RFID RC522 (SPI SCK)", sck, "SPI_SCK"))
        assignments.append(PinAssignment("Đầu đọc thẻ RFID RC522 (SPI MOSI)", mosi, "SPI_MOSI"))
        assignments.append(PinAssignment("Đầu đọc thẻ RFID RC522 (SPI MISO)", miso, "SPI_MISO"))
        assignments.append(PinAssignment("Đầu đọc thẻ RFID RC522 (SPI CS)", cs))
        rst = _get_int(config, "CONFIG_CUSTOM_PERIPH_RC522_RST_PIN", -1)
        if rst >= 0:
            assignments.append(PinAssignment("Đầu đọc thẻ RFID RC522 (RST)", rst))

    # 39. Giám sát trạng thái sạc pin TP4056
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PERIPH_BATTERY_CHARGING_DETECT"):
        chrg = _get_int(config, "CONFIG_CUSTOM_PERIPH_BATTERY_CHRG_PIN", 3)
        assignments.append(PinAssignment("Giám sát sạc pin TP4056 (CHRG)", chrg))

    # 40. Giao tiếp mạng công nghiệp CAN Bus / TWAI Controller
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PERIPH_CAN_TWAI"):
        tx = _get_int(config, "CONFIG_CUSTOM_PERIPH_TWAI_TX_PIN", 15)
        rx = _get_int(config, "CONFIG_CUSTOM_PERIPH_TWAI_RX_PIN", 16)
        assignments.append(PinAssignment("Mạng CAN/TWAI Controller (TX)", tx))
        assignments.append(PinAssignment("Mạng CAN/TWAI Controller (RX)", rx))

    # 41. Module mạng di động không dây ngoài trời 4G LTE Cat.1
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PERIPH_CELLULAR_4G_LTE"):
        tx = _get_int(config, "CONFIG_CUSTOM_PERIPH_4G_UART_TX_PIN", 43)
        rx = _get_int(config, "CONFIG_CUSTOM_PERIPH_4G_UART_RX_PIN", 44)
        pwr = _get_int(config, "CONFIG_CUSTOM_PERIPH_4G_PWRKEY_PIN", 2)
        assignments.append(PinAssignment("Modem di động 4G LTE (TX)", tx))
        assignments.append(PinAssignment("Modem di động 4G LTE (RX)", rx))
        assignments.append(PinAssignment("Modem di động 4G LTE (PWRKEY)", pwr))

    # Công tắc gạt 2 trạng thái Slide Switch
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SLIDE_SWITCH"):
        sw_pin = _get_int(config, "CONFIG_CUSTOM_SLIDE_SWITCH_PIN", 48)
        assignments.append(PinAssignment("Công tắc gạt Slide Switch", sw_pin))

    # Radar vi sóng phát hiện hiện diện 24GHz HLK-LD2410
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_RADAR_LD2410"):
        tx = _get_int(config, "CONFIG_CUSTOM_SENSOR_RADAR_TX_PIN", 43)
        rx = _get_int(config, "CONFIG_CUSTOM_SENSOR_RADAR_RX_PIN", 44)
        assignments.append(PinAssignment("Radar vi sóng LD2410 (TX)", tx))
        assignments.append(PinAssignment("Radar vi sóng LD2410 (RX)", rx))

    # Cảm biến từ tính Hall / Reed Switch
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_SENSOR_HALL_REED"):
        hall_pin = _get_int(config, "CONFIG_CUSTOM_SENSOR_HALL_REED_PIN", 21)
        assignments.append(PinAssignment("Cảm biến từ tính Hall/Reed", hall_pin))

    # Cổng giao tiếp ngoại vi phụ Sub UART / RS485
    if _is_yes(config, "CONFIG_CUSTOM_ENABLE_PERIPH_SUB_UART_RS485"):
        tx = _get_int(config, "CONFIG_CUSTOM_PERIPH_SUB_UART_TX_PIN", 17)
        rx = _get_int(config, "CONFIG_CUSTOM_PERIPH_SUB_UART_RX_PIN", 18)
        assignments.append(PinAssignment("Cổng Sub UART/RS485 (TX)", tx))
        assignments.append(PinAssignment("Cổng Sub UART/RS485 (RX)", rx))
        rts = _get_int(config, "CONFIG_CUSTOM_PERIPH_SUB_UART_RTS_PIN", -1)
        if rts >= 0:
            assignments.append(PinAssignment("Cổng Sub UART/RS485 (RTS)", rts))

    return assignments


def validate_pin_assignments(
    assignments: List[PinAssignment],
    is_n16r8: bool = True,
) -> Tuple[List[str], List[str]]:
    """
    Kiểm tra tính hợp lệ của các chân GPIO đã gán.
    Trả về: (errors, warnings)
    """
    errors: List[str] = []
    warnings: List[str] = []

    # Nhóm assignments theo chân pin (bỏ qua pin < 0 - tức không dùng / NC)
    by_pin: Dict[int, List[PinAssignment]] = {}
    for item in assignments:
        if item.pin < 0:
            continue
        by_pin.setdefault(item.pin, []).append(item)

    # 1. Kiểm tra dải chân cấm trên N16R8 [26..37]
    if is_n16r8:
        for pin, items in by_pin.items():
            if pin in FORBIDDEN_GPIOS_N16R8:
                dev_names = ", ".join(repr(i.device) for i in items)
                errors.append(
                    f"GPIO {pin} bị CẤM trên ESP32-S3 N16R8 (chân dành riêng cho Flash & Octal PSRAM: 26-37)! "
                    f"Thiết bị vi phạm: {dev_names}."
                )

    # 2. Kiểm tra xung đột chân trùng giữa các thiết bị
    for pin, items in by_pin.items():
        if len(items) <= 1:
            continue

        # Kiểm tra xem có phải tất cả đều chia sẻ bus hợp lệ hay không
        types = set(i.pin_type for i in items)
        if types == {"I2C_SDA"} or types == {"I2C_SCL"}:
            # Hợp lệ: Dùng chung bus I2C
            continue
        if types == {"I2S_BCLK"} or types == {"I2S_WS"}:
            # Hợp lệ: Dùng chung xung I2S Duplex
            continue
        if types == {"SPI_SCK"} or types == {"SPI_MOSI"} or types == {"SPI_MISO"}:
            # Hợp lệ: Dùng chung bus SPI (MicroSD, RFID RC522...)
            continue

        # Nếu không thuộc ngoại lệ bus hợp lệ -> XUNG ĐỘT
        # Nhóm theo tên thiết bị
        devs = [i.device for i in items]
        errors.append(
            f"Xung đột GPIO {pin}: Được sử dụng đồng thời bởi {devs[0]} và {', '.join(devs[1:])}!"
        )

    # 3. Cảnh báo các chân strapping / USB nhạy cảm
    for pin, items in by_pin.items():
        if pin in STRAPPING_PINS:
            # Riêng GPIO 0 thường làm nút Boot: nếu chỉ có nút boot thì cảnh báo mức thấp, còn lại lưu ý
            strap_desc = STRAPPING_PINS[pin]
            dev_names = ", ".join(i.device for i in items)
            if pin == 0 and len(items) == 1 and "BOOT" in items[0].device:
                pass  # Nút BOOT trên GPIO 0 là thiết kế chuẩn của ESP32-S3 DevKit
            else:
                warnings.append(
                    f"GPIO {pin} là {strap_desc}. Ngoại vi '{dev_names}' đang dùng chân này có thể ảnh hưởng đến quá trình khởi động nạp firmware."
                )

        if pin in USB_PINS:
            usb_desc = USB_PINS[pin]
            # Nếu chỉ là camera USB thì hợp lệ, còn nếu thiết bị khác (UART/GPIO) dùng chân này thì cảnh báo
            non_usb = [i.device for i in items if not i.pin_type.startswith("USB")]
            if non_usb:
                warnings.append(
                    f"GPIO {pin} là {usb_desc}. Ngoại vi '{', '.join(non_usb)}' sử dụng chân này sẽ vô hiệu hóa cổng USB OTG/Serial JTAG của ESP32-S3."
                )

    return errors, warnings


def validate_sdkconfig(
    sdkconfig_path_or_content: Union[str, Path, Dict[str, str]],
    is_n16r8: bool = True,
) -> Tuple[List[str], List[str]]:
    """
    Hàm API chính để kiểm tra file cấu hình sdkconfig.
    """
    if isinstance(sdkconfig_path_or_content, (str, Path)):
        config = parse_sdkconfig_file(sdkconfig_path_or_content)
    else:
        config = dict(sdkconfig_path_or_content)

    # Nếu cấu hình có chứa BOARD_TYPE_ESP32_S3_N16R8_CUSTOM, tự động coi là N16R8
    if _is_yes(config, "CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM"):
        is_n16r8 = True

    assignments = extract_pin_assignments(config)
    return validate_pin_assignments(assignments, is_n16r8=is_n16r8)


def format_validation_report(errors: List[str], warnings: List[str]) -> str:
    """Tạo báo cáo định dạng chuẩn an toàn trên mọi bảng mã console Windows/Linux."""
    lines: List[str] = []
    if errors:
        lines.append("")
        lines.append("================================================================================")
        lines.append("             [ERROR] PHAT HIEN XUNG DOT PHAN CUNG GPIO / CHAN CAM              ")
        lines.append("================================================================================")
        for err in errors:
            lines.append(f"  * {err}")
        lines.append("================================================================================")
        lines.append("")

    if warnings:
        lines.append("")
        lines.append("--------------------------------------------------------------------------------")
        lines.append("  [WARNING] CANH BAO CHAN GPIO NHAY CAM (Vui long kiem tra lai so do mach):")
        for warn in warnings:
            lines.append(f"  * {warn}")
        lines.append("--------------------------------------------------------------------------------")
        lines.append("")

    return "\n".join(lines)


def safe_print(text: str, file=None) -> None:
    """In chuỗi an toàn không bao giờ văng lỗi UnicodeEncodeError trên Windows."""
    out = file or sys.stdout
    enc = getattr(out, "encoding", None) or "utf-8"
    try:
        out.write(text + "\n")
    except UnicodeEncodeError:
        out.write(text.encode(enc, errors="replace").decode(enc) + "\n")
    out.flush()


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Kiểm tra xung đột GPIO và dải chân cấm cho ESP32-S3 N16R8 Xiaozhi."
    )
    parser.add_argument(
        "sdkconfig",
        nargs="?",
        default="sdkconfig",
        help="Đường dẫn đến file sdkconfig (mặc định: ./sdkconfig)",
    )
    args = parser.parse_args()

    sdkconfig_path = Path(args.sdkconfig)
    if not sdkconfig_path.is_file():
        safe_print(f"[ERROR] Không tìm thấy file cấu hình: {sdkconfig_path}", file=sys.stderr)
        return 1

    errors, warnings = validate_sdkconfig(sdkconfig_path)
    report = format_validation_report(errors, warnings)
    if report:
        safe_print(report)

    if errors:
        safe_print(
            f"[FAILED] Phát hiện {len(errors)} lỗi phần cứng nghiêm trọng. Quá trình biên dịch bị chặn!",
            file=sys.stderr,
        )
        return 1

    safe_print("[OK] Cấu hình GPIO hoàn toàn hợp lệ và an toàn cho bo mạch ESP32-S3 N16R8.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
