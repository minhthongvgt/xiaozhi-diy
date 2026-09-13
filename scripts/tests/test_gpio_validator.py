#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Unit tests cho scripts/gpio_validator.py."""

import json
from pathlib import Path
import unittest

from scripts.gpio_validator import (
    FORBIDDEN_GPIOS_N16R8,
    extract_pin_assignments,
    validate_pin_assignments,
    validate_sdkconfig,
)

ROOT = Path(__file__).resolve().parents[2]


class GpioValidatorTests(unittest.TestCase):
    def test_current_n16r8_board_config_is_valid(self):
        """Đảm bảo cấu hình mặc định hiện tại của esp32s3-n16r8-custom là 100% hợp lệ."""
        config_path = ROOT / "main/boards/esp32s3-n16r8-custom/config.json"
        board_json = json.loads(config_path.read_text(encoding="utf-8"))
        append_lines = board_json["builds"][0]["sdkconfig_append"]

        cfg: dict[str, str] = {}
        for line in append_lines:
            if "=" in line:
                k, v = line.split("=", 1)
                cfg[k.strip()] = v.strip().strip('"')

        errors, warnings = validate_sdkconfig(cfg, is_n16r8=True)
        self.assertEqual(errors, [], f"Cấu hình N16R8 mặc định bị lỗi: {errors}")

    def test_preset_n16r8_diy_is_valid(self):
        """Kiểm tra Preset N16R8 DIY: ST7796 + MAX98357A + INMP441 + WS2812 + Button 0."""
        preset_cfg = {
            "CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM": "y",
            "CONFIG_CUSTOM_PRESET_N16R8_DIY_ENABLED": "y",
            "CONFIG_ENABLE_CUSTOM_DISPLAY": "y",
            "CONFIG_CUSTOM_DISPLAY_ST7796": "y",
            "CONFIG_CUSTOM_DISPLAY_PIN_MOSI": "47",
            "CONFIG_CUSTOM_DISPLAY_PIN_CLK": "21",
            "CONFIG_CUSTOM_DISPLAY_PIN_CS": "41",
            "CONFIG_CUSTOM_DISPLAY_PIN_DC": "40",
            "CONFIG_CUSTOM_DISPLAY_USE_RST": "y",
            "CONFIG_CUSTOM_DISPLAY_PIN_RST": "42",
            "CONFIG_CUSTOM_DISPLAY_USE_BLK": "y",
            "CONFIG_CUSTOM_DISPLAY_PIN_BLK": "38",
            "CONFIG_ENABLE_CUSTOM_SPEAKER": "y",
            "CONFIG_CUSTOM_AUDIO_DAC_MAX98357A": "y",
            "CONFIG_CUSTOM_AUDIO_SPK_GPIO_DOUT": "7",
            "CONFIG_CUSTOM_AUDIO_SPK_GPIO_BCLK": "15",
            "CONFIG_CUSTOM_AUDIO_SPK_GPIO_LRCK": "16",
            "CONFIG_ENABLE_CUSTOM_MIC": "y",
            "CONFIG_CUSTOM_AUDIO_MIC_INMP441": "y",
            "CONFIG_CUSTOM_AUDIO_MIC_GPIO_WS": "4",
            "CONFIG_CUSTOM_AUDIO_MIC_GPIO_SCK": "5",
            "CONFIG_CUSTOM_AUDIO_MIC_GPIO_DIN": "6",
            "CONFIG_ENABLE_CUSTOM_LEDS": "y",
            "CONFIG_CUSTOM_LED_WS2812": "y",
            "CONFIG_CUSTOM_LED_WS2812_GPIO": "48",
            "CONFIG_CUSTOM_ENABLE_BUTTON_BOOT": "y",
            "CONFIG_CUSTOM_BUTTON_BOOT_GPIO": "0",
        }
        errors, warnings = validate_sdkconfig(preset_cfg, is_n16r8=True)
        self.assertEqual(errors, [], f"Preset N16R8 DIY bị lỗi: {errors}")

    def test_forbidden_gpio_in_n16r8_range_is_rejected(self):
        """Kiểm tra việc gán bất kỳ chân nào trong dải cấm 26-37 bị báo lỗi nghiêm trọng."""
        for forbidden_pin in [26, 27, 30, 33, 35, 37]:
            cfg = {
                "CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM": "y",
                "CONFIG_CUSTOM_ENABLE_BUTTON_BOOT": "y",
                "CONFIG_CUSTOM_BUTTON_BOOT_GPIO": str(forbidden_pin),
            }
            errors, warnings = validate_sdkconfig(cfg, is_n16r8=True)
            self.assertTrue(
                any(f"GPIO {forbidden_pin} bị CẤM" in err for err in errors),
                f"Không phát hiện lỗi với chân cấm {forbidden_pin}: {errors}",
            )

    def test_duplicate_gpio_between_devices_is_rejected(self):
        """Kiểm tra xung đột khi hai thiết bị độc lập dùng chung một GPIO."""
        cfg = {
            "CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM": "y",
            "CONFIG_CUSTOM_PERIPH_RELAY_ENABLE": "y",
            "CONFIG_CUSTOM_PERIPH_RELAY_GPIO": "13",
            "CONFIG_ENABLE_CUSTOM_LEDS": "y",
            "CONFIG_CUSTOM_ENABLE_SERVO_DOG": "y",
            "CONFIG_CUSTOM_SERVO_DOG_PWM_GPIO": "13",  # Trùng với Relay
        }
        errors, warnings = validate_sdkconfig(cfg, is_n16r8=True)
        self.assertTrue(
            any("Xung đột GPIO 13" in err and "Rơ-le" in err and "Servo" in err for err in errors),
            f"Không phát hiện xung đột chân 13 giữa Relay và Servo: {errors}",
        )

    def test_i2c_bus_sharing_is_allowed(self):
        """Nhiều thiết bị I2C cùng chia sẻ SDA=8, SCL=9 là hợp lệ."""
        cfg = {
            "CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM": "y",
            "CONFIG_ENABLE_CUSTOM_DISPLAY": "y",
            "CONFIG_CUSTOM_DISPLAY_OLED_SSD1306": "y",
            "CONFIG_CUSTOM_DISPLAY_PIN_I2C_SDA": "8",
            "CONFIG_CUSTOM_DISPLAY_PIN_I2C_SCL": "9",
            "CONFIG_ENABLE_CUSTOM_TOUCH": "y",
            "CONFIG_CUSTOM_TOUCH_PIN_SDA": "8",
            "CONFIG_CUSTOM_TOUCH_PIN_SCL": "9",
            "CONFIG_CUSTOM_TOUCH_PIN_INT": "3",
            "CONFIG_CUSTOM_TOUCH_PIN_RST": "2",
            "CONFIG_ENABLE_CUSTOM_IO_EXPANDER_PMIC": "y",
            "CONFIG_CUSTOM_EXPANDER_I2C_SDA": "8",
            "CONFIG_CUSTOM_EXPANDER_I2C_SCL": "9",
            "CONFIG_CUSTOM_ENABLE_IMU_MPU6050": "y",
            "CONFIG_CUSTOM_IMU_I2C_SDA": "8",
            "CONFIG_CUSTOM_IMU_I2C_SCL": "9",
        }
        errors, warnings = validate_sdkconfig(cfg, is_n16r8=True)
        self.assertEqual(errors, [], f"Chia sẻ bus I2C hợp lệ nhưng bị báo lỗi: {errors}")

    def test_i2s_duplex_clock_sharing_allowed_vs_simplex_rejected(self):
        """I2S Duplex cho phép dùng chung xung Clock, trong khi Simplex sẽ báo xung đột."""
        # Duplex: Loa và Mic dùng chung BCLK=15, LRCK/WS=4
        duplex_cfg = {
            "CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM": "y",
            "CONFIG_CUSTOM_AUDIO_I2S_DUPLEX": "y",
            "CONFIG_ENABLE_CUSTOM_SPEAKER": "y",
            "CONFIG_CUSTOM_AUDIO_SPK_GPIO_DOUT": "7",
            "CONFIG_CUSTOM_AUDIO_SPK_GPIO_BCLK": "15",
            "CONFIG_CUSTOM_AUDIO_SPK_GPIO_LRCK": "4",
            "CONFIG_ENABLE_CUSTOM_MIC": "y",
            "CONFIG_CUSTOM_AUDIO_MIC_GPIO_DIN": "6",
            "CONFIG_CUSTOM_AUDIO_MIC_GPIO_SCK": "15",
            "CONFIG_CUSTOM_AUDIO_MIC_GPIO_WS": "4",
        }
        errors, _ = validate_sdkconfig(duplex_cfg, is_n16r8=True)
        self.assertEqual(errors, [], f"Duplex I2S hợp lệ nhưng bị báo lỗi: {errors}")

        # Simplex: Loa và Mic cố tình gán chung BCLK=15 -> Bị lỗi xung đột
        simplex_cfg = dict(duplex_cfg)
        simplex_cfg.pop("CONFIG_CUSTOM_AUDIO_I2S_DUPLEX", None)
        simplex_cfg["CONFIG_CUSTOM_AUDIO_I2S_SIMPLEX"] = "y"
        errors, _ = validate_sdkconfig(simplex_cfg, is_n16r8=True)
        self.assertTrue(
            any("Xung đột GPIO 15" in err for err in errors),
            f"Simplex trùng clock nhưng không bị bắt lỗi: {errors}",
        )

    def test_negative_gpios_are_ignored(self):
        """Chân âm (-1 / NC) không gây xung đột."""
        cfg = {
            "CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM": "y",
            "CONFIG_ENABLE_CUSTOM_DISPLAY": "y",
            "CONFIG_CUSTOM_DISPLAY_ST7789": "y",
            "CONFIG_CUSTOM_DISPLAY_PIN_MOSI": "47",
            "CONFIG_CUSTOM_DISPLAY_PIN_CLK": "21",
            "CONFIG_CUSTOM_DISPLAY_PIN_CS": "41",
            "CONFIG_CUSTOM_DISPLAY_PIN_DC": "40",
            "CONFIG_CUSTOM_DISPLAY_PIN_RST": "-1",
            "CONFIG_CUSTOM_DISPLAY_PIN_BLK": "-1",
        }
        errors, _ = validate_sdkconfig(cfg, is_n16r8=True)
        self.assertEqual(errors, [])


if __name__ == "__main__":
    unittest.main()
