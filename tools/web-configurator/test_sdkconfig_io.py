"""Test sdkconfig_io.py — chạy: python test_sdkconfig_io.py (không cần ESP-IDF)."""
import os, sys, tempfile, unittest
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sdkconfig_io as io

def w(root, name, text):
    with open(os.path.join(root, name), "w", newline="") as f: f.write(text)
def r(root, name):
    with open(os.path.join(root, name), newline="") as f: return f.read()

SDK = ('# Automatically generated file; DO NOT EDIT.\n'
       '# Espressif IoT Development Framework (ESP-IDF) 6.1.0 Project Configuration\n'
       'CONFIG_IDF_TARGET="esp32s3"\nCONFIG_SPIRAM=y\n# CONFIG_ENABLE_BUZZER is not set\n'
       'CONFIG_ESP_MAIN_TASK_STACK_SIZE=3584\n')

class T(unittest.TestCase):
    def setUp(self):
        self.d = tempfile.TemporaryDirectory(); self.root = self.d.name
        os.environ.pop("IDF_TARGET", None)
    def tearDown(self): self.d.cleanup()

    def test_target_from_sdkconfig(self):
        w(self.root, "sdkconfig", SDK)
        i = io.detect_target(self.root)
        self.assertTrue(i["is_set"]); self.assertTrue(i["matches"])
        self.assertEqual(i["target"], "esp32s3"); self.assertEqual(i["idf_version"], "6.1.0")

    def test_target_from_cmakecache_when_no_sdkconfig(self):
        os.makedirs(os.path.join(self.root, "build"))
        w(self.root, "build/CMakeCache.txt", "IDF_TARGET:STRING=esp32s3\n")
        i = io.detect_target(self.root)
        self.assertTrue(i["is_set"]); self.assertEqual(i["source"], "build/CMakeCache.txt")

    def test_only_declared_in_defaults_is_not_set(self):
        w(self.root, "sdkconfig.defaults", 'CONFIG_IDF_TARGET="esp32s3"\n')
        i = io.detect_target(self.root)
        self.assertFalse(i["is_set"]); self.assertEqual(i["target"], "esp32s3")

    def test_wrong_target_detected_and_save_refused(self):
        w(self.root, "sdkconfig", SDK.replace("esp32s3", "esp32"))
        i = io.detect_target(self.root)
        self.assertTrue(i["is_set"]); self.assertFalse(i["matches"])
        with self.assertRaises(ValueError):
            io.save_config(self.root, ["CONFIG_SPIRAM=y"])
        self.assertIn('"esp32"', r(self.root, "sdkconfig"))      # không bị đụng tới

    def test_conflict_between_sources(self):
        w(self.root, "sdkconfig", SDK)
        os.makedirs(os.path.join(self.root, "build"))
        w(self.root, "build/CMakeCache.txt", "IDF_TARGET:STRING=esp32\n")
        self.assertTrue(io.detect_target(self.root)["conflicts"])

    def test_effective_layering_target_defaults_and_sdkconfig_win(self):
        w(self.root, "sdkconfig.defaults", "# CONFIG_SPIRAM is not set\nCONFIG_A=1\nCONFIG_B=1\n")
        w(self.root, "sdkconfig.defaults.esp32s3", "CONFIG_SPIRAM=y\nCONFIG_B=2\n")
        w(self.root, "sdkconfig", "CONFIG_B=3\n")
        m = io.load_effective(self.root)
        self.assertEqual(m["CONFIG_SPIRAM"], "y")     # defaults.esp32s3 thắng defaults
        self.assertEqual(m["CONFIG_A"], "1")
        self.assertEqual(m["CONFIG_B"], "3")          # sdkconfig thắng tất cả
        txt = io.load_effective_text(self.root)
        self.assertIn("CONFIG_SPIRAM=y", txt)

    def test_target_defaults_ignored_without_generic_defaults(self):
        w(self.root, "sdkconfig.defaults.esp32s3", 'CONFIG_A=1\n')
        m = io.load_effective(self.root)
        self.assertNotIn("CONFIG_A", m)
        i = io.detect_target(self.root)
        self.assertFalse(i["is_set"])
        self.assertIsNone(i["target"])

    def test_in_place_update_keeps_other_lines_and_replaces_both_forms(self):
        w(self.root, "sdkconfig", SDK)
        lines = ["# Generated", "", "CONFIG_IDF_TARGET=\"esp32s3\"", "# CONFIG_SPIRAM is not set",
                 "CONFIG_ENABLE_BUZZER=y", "CONFIG_BUZZER_PIN=5", "CONFIG_ESP_MAIN_TASK_STACK_SIZE=6584"]
        io.save_config(self.root, lines)
        s = r(self.root, "sdkconfig")
        self.assertIn("# CONFIG_SPIRAM is not set", s); self.assertNotIn("CONFIG_SPIRAM=y", s)
        self.assertIn("CONFIG_ENABLE_BUZZER=y", s);     self.assertNotIn("# CONFIG_ENABLE_BUZZER is not set", s)
        self.assertIn("CONFIG_ESP_MAIN_TASK_STACK_SIZE=6584", s)
        self.assertIn("CONFIG_BUZZER_PIN=5", s)
        self.assertIn("# Espressif IoT Development Framework", s)   # header giữ nguyên
        self.assertEqual(s.count("CONFIG_IDF_TARGET="), 1)
        self.assertTrue(os.path.exists(os.path.join(self.root, "sdkconfig.bak")))

    def test_both_files_updated_and_reconfigure_flag(self):
        w(self.root, "sdkconfig", SDK)
        res = io.save_config(self.root, ["CONFIG_BUZZER_PIN=5"])
        self.assertTrue(res["needs_reconfigure"])
        self.assertIn("CONFIG_BUZZER_PIN=5", r(self.root, "sdkconfig.defaults.esp32s3"))

    def test_no_sdkconfig_only_defaults_and_note(self):
        res = io.save_config(self.root, ["CONFIG_BUZZER_PIN=5"])
        self.assertIsNone(res["sdkconfig"]); self.assertFalse(res["needs_reconfigure"])
        self.assertFalse(os.path.exists(os.path.join(self.root, "sdkconfig")))
        self.assertIn("set-target", res["note"])

    def test_idempotent_no_rewrite(self):
        w(self.root, "sdkconfig", SDK)
        lines = ["CONFIG_BUZZER_PIN=5"]
        io.save_config(self.root, lines)
        res = io.save_config(self.root, lines)
        self.assertFalse(res["sdkconfig"]["written"])

    def test_crlf_preserved(self):
        w(self.root, "sdkconfig", SDK.replace("\n", "\r\n"))
        io.save_config(self.root, ["CONFIG_BUZZER_PIN=5"])
        s = r(self.root, "sdkconfig")
        self.assertNotIn("\n", s.replace("\r\n", ""))

    def test_string_values_with_equals_and_quotes_roundtrip(self):
        w(self.root, "sdkconfig", SDK)
        io.save_config(self.root, ['CONFIG_OTA_URL="https://a.b/c?x=1&y=2"'])
        self.assertEqual(io.parse_kconfig_text(r(self.root, "sdkconfig"))["CONFIG_OTA_URL"],
                         '"https://a.b/c?x=1&y=2"')

if __name__ == "__main__": unittest.main(verbosity=2)
