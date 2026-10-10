import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location(
    "build_default_assets", ROOT / "scripts" / "build_default_assets.py"
)
BUILD = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(BUILD)


class BuildDefaultAssetsTest(unittest.TestCase):
    def test_text_font_metadata_uses_bundle_charset_size_and_bpp(self):
        with tempfile.TemporaryDirectory() as directory:
            assets = Path(directory)
            BUILD.generate_index_json(
                str(assets),
                None,
                "font_noto_sans_common_20_4.bin",
                None,
                font_bundle_id="noto-v1",
            )
            index = json.loads((assets / "index.json").read_text(encoding="utf-8"))
            self.assertEqual(
                index["text_font_meta"],
                {"charset": "common", "size": 20, "bpp": 4, "bundle": "noto-v1"},
            )

    def test_text_font_requires_bundle(self):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaises(ValueError):
                BUILD.generate_index_json(
                    directory, None, "font_noto_sans_common_20_4.bin", None
                )

    def test_max_size_rejects_oversized_assets(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            extra = root / "extra"
            extra.mkdir()
            (extra / "big.bin").write_bytes(b"\x5a" * 4096)
            output = root / "assets.bin"

            ok = BUILD.build_assets_integrated(
                None,
                None,
                None,
                None,
                str(extra),
                str(output),
                max_size=100,
            )
            self.assertFalse(ok)

    def test_max_size_allows_fitting_assets(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            extra = root / "extra"
            extra.mkdir()
            (extra / "small.bin").write_bytes(b"hello")
            output = root / "assets.bin"

            ok = BUILD.build_assets_integrated(
                None,
                None,
                None,
                None,
                str(extra),
                str(output),
                max_size=64 * 1024,
            )
            self.assertTrue(ok)
            self.assertTrue(output.exists())
            self.assertLessEqual(output.stat().st_size, 64 * 1024)

    def test_wakenet10_copy_keeps_s3_p1_slice(self):
        with tempfile.TemporaryDirectory() as directory:
            src = Path(directory) / "wn10_nihaoxiaozhi"
            dst = Path(directory) / "out"
            src.mkdir()
            (src / "_MODEL_INFO_p1").write_text("p1-info", encoding="utf-8")
            (src / "_MODEL_INFO_p2").write_text("p2-info", encoding="utf-8")
            (src / "wn10_data_p1").write_bytes(b"s3-data")
            (src / "wn10_data_p2").write_bytes(b"p4-data")
            (src / "extra.bin").write_bytes(b"keep")

            self.assertTrue(BUILD.copy_wakenet_model(str(src), str(dst), "esp32s3"))
            self.assertEqual((dst / "wn10_data").read_bytes(), b"s3-data")
            self.assertEqual((dst / "_MODEL_INFO_").read_text(encoding="utf-8"), "p1-info")
            self.assertEqual((dst / "extra.bin").read_bytes(), b"keep")
            self.assertFalse((dst / "wn10_data_p1").exists())
            self.assertFalse((dst / "wn10_data_p2").exists())

    def test_wakenet10_copy_keeps_p4_p2_slice(self):
        with tempfile.TemporaryDirectory() as directory:
            src = Path(directory) / "wn10_nihaoxiaozhi"
            dst = Path(directory) / "out"
            src.mkdir()
            (src / "_MODEL_INFO_p2").write_text("p2-info", encoding="utf-8")
            (src / "wn10_data_p2").write_bytes(b"p4-data")

            self.assertTrue(BUILD.copy_wakenet_model(str(src), str(dst), "esp32p4"))
            self.assertEqual((dst / "wn10_data").read_bytes(), b"p4-data")
            self.assertEqual((dst / "_MODEL_INFO_").read_text(encoding="utf-8"), "p2-info")

    def test_wakenet10_copy_rejects_unknown_target(self):
        with tempfile.TemporaryDirectory() as directory:
            src = Path(directory) / "wn10_nihaoxiaozhi"
            src.mkdir()
            (src / "wn10_data_p1").write_bytes(b"s3-data")
            with self.assertRaises(ValueError):
                BUILD.copy_wakenet_model(str(src), str(Path(directory) / "out"), "esp32c3")

if __name__ == "__main__":
    unittest.main()
