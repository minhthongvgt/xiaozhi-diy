# Codebase Remediation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restore a reliable host-test suite and make the ESP32-S3 N16R8 board configuration consistent with the project-wide build defaults.

**Architecture:** Keep project defaults in `sdkconfig.defaults`; target defaults contain only target-specific overrides; board variants contain only options that differ from their effective defaults. Make Windows temporary-directory cleanup deterministic by restoring the process working directory before each temporary directory exits.

**Tech Stack:** Python 3 `unittest`, ESP-IDF 6.1, Kconfig, CMake, JSON board variants.

**Spec:** `AGENTS.md`, `config/AGENTS.md`, and the audit findings from 2026-09-13.

## Global Constraints

- Modify source only under `config/`; do not edit `build/`, `managed_components/`, or generated `sdkconfig*` files.
- Preserve the one-board factory and the existing ESP32-S3 N16R8 board identity.
- Do not run source synchronization until the change report has been reviewed.
- Validate host tests, the configured N16R8 build, and physical display/audio/button hardware separately.

---

### Task 1: Make host tests portable on Windows

**Files:**
- Modify: `scripts/tests/test_build.py:20-30, 821-863, 866-906, 1067-1118, 1664-1683`

**Interfaces:**
- Consumes: Python's current working directory and `tempfile.TemporaryDirectory` lifecycle.
- Produces: a test helper that always restores the original current working directory before temporary-directory cleanup.

- [x] **Step 1: Write the failing regression test**

Add a test that enters a temporary working directory, writes a file, leaves the context, and asserts the original directory is restored. It must exercise the Windows-specific cleanup ordering.

```python
def test_temporary_working_directory_restores_cwd(self):
    original_cwd = Path.cwd()
    with temporary_working_directory() as temp_dir:
        self.assertEqual(Path.cwd(), temp_dir)
        (temp_dir / "created-by-test.txt").write_text("ok", encoding="utf-8")
    self.assertEqual(Path.cwd(), original_cwd)
    self.assertFalse(temp_dir.exists())
```

- [x] **Step 2: Run the new test to verify it fails**

Run: `python -m unittest scripts.tests.test_build.TargetConfigurationTests.test_temporary_working_directory_restores_cwd -v`

Expected: FAIL because `temporary_working_directory` does not exist.

- [x] **Step 3: Add the lifecycle helper**

Use the existing `contextlib` import and define this helper next to `ROOT`:

```python
@contextlib.contextmanager
def temporary_working_directory():
    original_cwd = Path.cwd()
    with tempfile.TemporaryDirectory() as temporary_dir:
        os.chdir(temporary_dir)
        try:
            yield Path(temporary_dir)
        finally:
            os.chdir(original_cwd)
```

- [x] **Step 4: Replace unsafe test patterns**

In all five affected tests (`test_configure_build_uses_all_cmake_values_in_one_run`, `test_configure_build_replaces_stale_sdkconfig_backup`, `test_configured_build_options_are_verified`, `test_disabled_build_options_accept_symbols_hidden_by_kconfig`, and `test_zip_is_always_recreated`), replace the `previous_cwd` / `try` / `finally` nesting with:

```python
with temporary_working_directory() as temp_dir:
    Path("sdkconfig").write_text('CONFIG_IDF_TARGET="esp32s3"\n', encoding="utf-8")
```

Keep each test's current setup, invocation, and assertions inside that context without changing their expected values. The helper's `finally` block runs before `TemporaryDirectory` performs Windows cleanup.

- [x] **Step 5: Run host tests**

Run: `python -m unittest discover -s scripts/tests -v`

Expected: all 81 tests pass; no `PermissionError: [WinError 32]` occurs during `TemporaryDirectory` cleanup.

### Task 2: Remove duplicated effective defaults from the custom board

**Files:**
- Modify: `sdkconfig.defaults.esp32s3:1-6`
- Modify: `main/boards/esp32s3-n16r8-custom/config.json:6-50`
- Test: `scripts/tests/test_build.py:197-285`

**Interfaces:**
- Consumes: root defaults (`sdkconfig.defaults`), target defaults, and `sdkconfig_append` board options.
- Produces: one unambiguous effective 16 MB flash / `partitions/v2/16m.csv` configuration inherited from root defaults.

- [x] **Step 1: Preserve the existing failing checks**

Run: `python -m unittest scripts.tests.test_build.VersionTests.test_chip_defaults_only_contain_target_overrides scripts.tests.test_build.VersionTests.test_default_flash_options_are_not_repeated -v`

Expected: FAIL, identifying four duplicates in `sdkconfig.defaults.esp32s3` and the repeated 16 MB flash option in the custom board JSON.

- [x] **Step 2: Remove only inherited entries**

Delete these lines from `sdkconfig.defaults.esp32s3`, because their values equal the root default:

```ini
CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions/v2/16m.csv"
CONFIG_PARTITION_TABLE_OFFSET=0x8000
```

Delete these entries from the N16R8 variant's `sdkconfig_append`, because the variant inherits them:

```json
"CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y",
"CONFIG_PARTITION_TABLE_CUSTOM=y",
"CONFIG_PARTITION_TABLE_CUSTOM_FILENAME=\"partitions/v2/16m.csv\""
```

- [x] **Step 3: Run the focused configuration checks**

Run: `python -m unittest scripts.tests.test_build.VersionTests.test_chip_defaults_only_contain_target_overrides scripts.tests.test_build.VersionTests.test_default_flash_options_are_not_repeated -v`

Expected: PASS. The effective N16R8 values remain 16 MB and `partitions/v2/16m.csv`.

### Task 3: Keep the Kconfig board menu deterministically sorted

**Files:**
- Modify: `main/Kconfig.projbuild:137-140, 195-204`
- Test: `scripts/tests/test_build.py:566-620`

**Interfaces:**
- Consumes: `BOARD_TYPE_*` Kconfig choice entries and matching `if(CONFIG_BOARD_TYPE_*)` conditions in CMake.
- Produces: a naturally sorted, unique board menu with an unchanged `BOARD_TYPE_ESP32_S3_N16R8_CUSTOM` symbol.

- [x] **Step 1: Run the menu regression check**

Run: `python -m unittest scripts.tests.test_build.BoardMenuTests.test_board_menu_is_sorted_and_matches_cmake -v`

Expected: FAIL because `ESP32-S3 N16R8 Custom DIY Board` is at the beginning of the board choice instead of its natural-sort position.

- [x] **Step 2: Move the Kconfig entry without modifying its identity**

Remove the existing N16R8 Kconfig block from the beginning of `choice BOARD_TYPE`. Insert it immediately after `BOARD_TYPE_ELECTRON_BOT` and before `BOARD_TYPE_ESP32_P4_FUNCTION_EV_BOARD`:

```kconfig
    config BOARD_TYPE_ESP32_S3_N16R8_CUSTOM
        bool "ESP32-S3 N16R8 Custom DIY Board (Bo mạch tùy biến toàn diện)"
        depends on IDF_TARGET_ESP32S3
```

- [x] **Step 3: Run the focused menu check**

Run: `python -m unittest scripts.tests.test_build.BoardMenuTests.test_board_menu_is_sorted_and_matches_cmake -v`

Expected: PASS, with exactly the same CMake board mapping.

### Task 4: Verify in the ESP-IDF environment and add CI protection

**Files:**
- Modify: `.github/workflows/build.yml` only if it does not already run `python -m unittest discover -s scripts/tests -v` before firmware builds.
- Test: `scripts/tests/test_build.py`

**Interfaces:**
- Consumes: ESP-IDF 6.1's exported environment and the N16R8 configuration selected by `scripts/build.py`.
- Produces: a reproducible host-test and firmware-build gate.

- [ ] **Step 1: Restore the missing ESP-IDF virtual environment**

Run from the ESP-IDF installation, using the supported installer for the local checkout:

```powershell
powershell -ExecutionPolicy Bypass -File C:\Espressif\v6.1\esp-idf\install.ps1
```

Expected: `C:\Users\admin\.espressif\python_env\idf6.1_py3.11_env\Scripts\python.exe` exists.

- [x] **Step 2: Run the complete host suite**

Run: `python -m unittest discover -s scripts/tests -v`

Expected: all tests PASS.

- [ ] **Step 3: Build the exact custom variant**

Run:

```powershell
powershell -ExecutionPolicy Bypass -Command "& 'C:\Espressif\v6.1\esp-idf\export.ps1'; python scripts/build.py esp32s3-n16r8-custom --name esp32s3-n16r8-custom"
```

Expected: successful ESP32-S3 firmware build with exactly one `DECLARE_BOARD(CustomN16R8Board)` factory.

- [x] **Step 4: Add or confirm the CI host-test job**

Place the test command before board matrix compilation:

```yaml
- name: Run build-script host tests
  run: python -m unittest discover -s scripts/tests -v
```

- [ ] **Step 5: Perform hardware smoke validation**

Flash the N16R8 build and verify boot, Wi-Fi provisioning, ST7796 display/backlight, PCM5102A playback, INMP441 capture, BOOT button, DHT sensor, wake word, reconnect, and OTA recovery. Record board revision and serial logs with the result.

### Task 5: Prevent configuration regressions in future board additions

**Files:**
- Modify: `scripts/tests/test_build.py:197-285, 566-620`
- Modify: `config/AGENTS.md:29-42` only if the team agrees to document the test gate.

**Interfaces:**
- Consumes: every board `config.json`, `sdkconfig.defaults*`, Kconfig menu, and CMake board selection.
- Produces: automatic failures for duplicated inherited options, unsorted boards, and missing board-chain links.

- [ ] **Step 1: Add an explicit N16R8 inheritance assertion**

Add this assertion to the default-flash-options test after loading the N16R8 board JSON. It verifies the board inherits root defaults instead of redundantly encoding them:

```python
options = config["builds"][0]["sdkconfig_append"]
self.assertNotIn("CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y", options)
self.assertNotIn("CONFIG_PARTITION_TABLE_CUSTOM=y", options)
self.assertNotIn(
    'CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions/v2/16m.csv"',
    options,
)
```

- [ ] **Step 2: Add a checklist item to board contribution guidance**

Document that a new board must run `python -m unittest discover -s scripts/tests -v` and must omit values inherited from project/target defaults.

- [ ] **Step 3: Re-run all automated checks**

Run: `python -m unittest discover -s scripts/tests -v`

Expected: PASS before any source synchronization.
