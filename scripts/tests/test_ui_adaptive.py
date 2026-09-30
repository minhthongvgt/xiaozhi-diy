import os
import re
import unittest

class AdaptiveUiTests(unittest.TestCase):
    def setUp(self):
        self.base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
        self.kconfig_path = os.path.join(self.base_dir, "main", "Kconfig.projbuild")
        self.cmakelists_path = os.path.join(self.base_dir, "main", "CMakeLists.txt")
        self.header_path = os.path.join(self.base_dir, "main", "display", "adaptive_ui.h")
        self.source_path = os.path.join(self.base_dir, "main", "display", "adaptive_ui.cc")
        self.lcd_display_path = os.path.join(self.base_dir, "main", "display", "lcd_display.cc")
        self.config_h_path = os.path.join(self.base_dir, "main", "boards", "esp32s3-n16r8-custom", "config.h")

    def test_source_and_header_files_exist(self):
        """Đảm bảo các file mã nguồn của Adaptive UI tồn tại đầy đủ."""
        self.assertTrue(os.path.isfile(self.header_path), f"Missing {self.header_path}")
        self.assertTrue(os.path.isfile(self.source_path), f"Missing {self.source_path}")

    def test_cmakelists_includes_adaptive_ui(self):
        """Đảm bảo CMakeLists.txt đã đăng ký adaptive_ui.cc vào SOURCES."""
        with open(self.cmakelists_path, "r", encoding="utf-8") as f:
            content = f.read()
        self.assertIn("display/adaptive_ui.cc", content, "adaptive_ui.cc must be listed in CMakeLists.txt SOURCES")

    def test_kconfig_contains_all_5_ui_styles(self):
        """Đảm bảo Kconfig.projbuild định nghĩa đầy đủ 5 kiểu giao diện đặc sắc."""
        with open(self.kconfig_path, "r", encoding="utf-8") as f:
            content = f.read()

        required_styles = [
            "CUSTOM_UI_STYLE_SMART_DASHBOARD",
            "CUSTOM_UI_STYLE_CYBER_TERMINAL",
            "CUSTOM_UI_STYLE_CHAT_BUBBLE",
            "CUSTOM_UI_STYLE_CLASSIC_AVATAR",
            "CUSTOM_UI_STYLE_MINIMAL_ZEN",
        ]
        for style in required_styles:
            self.assertIn(style, content, f"Kconfig must contain choice option {style}")

    def test_kconfig_contains_supplementary_ui_options(self):
        """Đảm bảo Kconfig.projbuild có các tùy chọn bổ trợ giao diện (thời tiết, wave, màu sắc, an toàn tròn)."""
        with open(self.kconfig_path, "r", encoding="utf-8") as f:
            content = f.read()

        options = [
            "CUSTOM_UI_WEATHER_ENABLE",
            "CUSTOM_UI_WEATHER_CITY",
            "CUSTOM_UI_VOICE_WAVE_ENABLE",
            "CUSTOM_UI_COLOR_ACCENT",
            "CUSTOM_UI_ACCENT_CYBER_BLUE",
            "CUSTOM_UI_ACCENT_EMERALD",
            "CUSTOM_UI_ACCENT_PURPLE",
            "CUSTOM_UI_ACCENT_AMBER",
            "CUSTOM_UI_ACCENT_MONO",
            "CUSTOM_UI_ROUND_SCREEN_SAFE_AREA",
            "CUSTOM_UI_AUTO_SCALE",
        ]
        for opt in options:
            self.assertIn(opt, content, f"Kconfig must contain UI option {opt}")

    def test_lcd_display_hooks_adaptive_ui(self):
        """Đảm bảo lcd_display.cc đã kết nối AdaptiveUiEngine."""
        with open(self.lcd_display_path, "r", encoding="utf-8") as f:
            content = f.read()

        self.assertIn('#include "adaptive_ui.h"', content)
        self.assertIn("TrySetupAdaptiveUi", content)
        self.assertIn("AdaptiveUiEngine::GetInstance()", content)

    def test_config_h_has_fallback_defaults(self):
        """Đảm bảo config.h có macro fallback mặc định cho UI style."""
        with open(self.config_h_path, "r", encoding="utf-8") as f:
            content = f.read()

        self.assertIn("CONFIG_CUSTOM_UI_STYLE_SMART_DASHBOARD", content)
        self.assertIn("CONFIG_CUSTOM_UI_WEATHER_CITY", content)

    def test_resolution_metrics_simulation(self):
        """Kiểm tra mô phỏng tính toán kích thước (ScreenMetrics) trên các độ phân giải khác nhau."""
        test_resolutions = [
            {"name": "OLED 128x64", "w": 128, "h": 64, "round": False},
            {"name": "GC9A01 240x240 Round", "w": 240, "h": 240, "round": True},
            {"name": "ST7789 240x320 Portrait", "w": 240, "h": 320, "round": False},
            {"name": "ST7789 320x240 Landscape", "w": 320, "h": 240, "round": False},
            {"name": "ST7796 320x480 Portrait", "w": 320, "h": 480, "round": False},
            {"name": "AMOLED 368x448", "w": 368, "h": 448, "round": False},
            {"name": "RGB 800x480 Wide Landscape", "w": 800, "h": 480, "round": False},
        ]

        for res in test_resolutions:
            w = res["w"]
            h = res["h"]
            is_mini = (h <= 64 or w <= 128)
            is_compact = (not is_mini and w <= 240 and h <= 280)
            is_large = (w >= 480 or h >= 480)
            
            if is_mini:
                pad_x = 2
                pad_y = 1
                safe_inset = 0
            elif is_compact:
                pad_x = 4
                pad_y = 3
                safe_inset = (w * 14 // 100) if res["round"] else 0
            elif is_large:
                pad_x = 16
                pad_y = 12
                safe_inset = (w * 12 // 100) if res["round"] else 0
            else:
                pad_x = 8
                pad_y = 6
                safe_inset = (w * 14 // 100) if res["round"] else 0

            content_w = w - 2 * (pad_x + safe_inset)
            content_h = h - 2 * (pad_y + safe_inset)

            self.assertGreater(content_w, 0, f"Content width must be > 0 for {res['name']}")
            self.assertGreater(content_h, 0, f"Content height must be > 0 for {res['name']}")
            self.assertLessEqual(content_w, w, f"Content width must fit inside screen for {res['name']}")
            self.assertLessEqual(content_h, h, f"Content height must fit inside screen for {res['name']}")

if __name__ == "__main__":
    unittest.main()
