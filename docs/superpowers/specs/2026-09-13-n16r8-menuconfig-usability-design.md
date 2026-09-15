# ESP32-S3 N16R8 Menuconfig Usability Design

**Goal:** Make the N16R8 `menuconfig` flow easier to configure correctly and reject unsafe GPIO assignments before firmware compilation.

## Scope

This design applies only when `CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM=y`. Existing board variants and their configuration contracts remain unchanged.

## User flow

The N16R8 menu becomes a numbered flow:

1. `0. Thiết lập nhanh` explains the recommended profile and provides a single opt-in switch for advanced hardware editing.
2. `1. Màn hình & cảm ứng`, `2. Âm thanh`, `3. Camera`, `4. Kết nối & MCP`, and `5. Ngoại vi & cảm biến` contain only the configuration relevant to their category.
3. `6. Kiểm tra cấu hình` explains how to run the pre-build GPIO validator and lists the protected GPIO range 26–37.

The existing N16R8 defaults remain the recommended profile: ST7796 display, PCM5102A speaker, INMP441 microphone, BOOT button, and DHT sensor. No existing symbol is renamed or removed. The advanced switch controls visibility of manual GPIO and optional peripheral controls; default board settings in `config.json` continue to configure the same hardware for scripted builds.

## GPIO validation

Create `scripts/validate_n16r8_config.py`, a standard-library Python module and CLI. It accepts an SDKCONFIG file path and performs these checks only when the N16R8 board symbol is enabled:

- Parse `CONFIG_*=` values and `# CONFIG_* is not set` records.
- Resolve enabled hardware and the exact GPIO symbols it uses.
- Ignore disabled or not-applicable pins, including `-1` / `GPIO_NUM_NC` equivalents.
- Reject GPIO 26 through 37 because they are reserved for N16R8 Flash/Octal-PSRAM.
- Reject one GPIO assigned to two distinct active hardware roles. I2C SDA/SCL are represented as one shared I2C bus role, so OLED, touch, and compatible I2C peripherals do not produce false collisions.
- Report every conflict in one error message using role names and the conflicting GPIO value, for example: `GPIO 47: display SPI MOSI; camera D4`.

The validator must recognise display, touch, speaker/microphone, camera, UART, LED, buttons, relay/MCP lamp, and all configurable sensors. It must also validate the selected default configuration successfully.

## Integration

`scripts/build.py` calls the validator immediately after `_configure_build()` creates `sdkconfig` and after Kconfig-option acceptance checks. A validation failure raises `ValueError` before `idf.py build` starts. The error includes a command that lets the user rerun the validator against `sdkconfig`.

Kconfig adds concise contextual warnings for feature pairs that are known to collide under default pins: display plus camera, camera plus DHT, camera plus relay/MCP lamp, camera plus LED, and camera plus HC-SR04/PIR. These warnings are educational; the Python validator is authoritative because users can modify GPIO values.

## UI constraints

- Keep labels short, Vietnamese-first, and use English protocol/device terms only where they improve recognition.
- The board selector remains a flat Kconfig `choice`; add a short help hint that `/` searches menu entries rather than attempting to restructure all boards.
- Do not keep a static “currently used GPIO” table: it becomes inaccurate after custom pin edits. Replace it with a concise validator command and current-configuration guidance.
- Sensitive MCP controls retain their current defaults. Their help text explicitly states that enabling a tool authorizes the remote assistant to invoke that action.

## Tests and acceptance criteria

- Host unit tests cover SDKCONFIG parsing, disabled features, valid default N16R8 configuration, duplicate GPIO conflicts, reserved-pin conflicts, and allowed I2C sharing.
- A build-script test proves validation runs before `idf.py build` and prevents the build call after a validation error.
- Existing host tests pass.
- The N16R8 variant builds with ESP-IDF 6.1 once the local SDK environment is available.
- Physical validation confirms the default display, audio, BOOT button, DHT sensor, network, and OTA paths.

