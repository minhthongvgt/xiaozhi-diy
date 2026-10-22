# ESP32-P4 Board Support (Guition M3-Dev, Espressif EV Board & Waveshare Dev Kit)

Shared board support for ESP32-P4 development platforms:
1. **Guition JC-ESP32P4-M3-Dev** (default board for `idf.py set-target esp32p4`)
2. **Espressif ESP32-P4-Function-EV-Board / ESP32-P4X-Function-EV-Board**
3. **Waveshare ESP32-P4-Module-DEV-KIT** (See [WAVESHARE_ESP32_P4_MODULE_DEV_KIT.md](WAVESHARE_ESP32_P4_MODULE_DEV_KIT.md))

## Guition JC-ESP32P4-M3-Dev

Pins come from the Guition schematic, confirmed on a v1.3 board.

| Function | GPIO |
|---|---|
| ESP32-C6 SDIO (slot 1) | CLK 18, CMD 19, D0-D3 14-17, reset 54 (active high) — same as the Function-EV board, so `CONFIG_ESP_HOSTED_P4_DEV_BOARD_FUNC_BOARD=y` |
| I2C (ES8311 + camera SCCB) | SDA 7, SCL 8 |
| ES8311 I2S | MCLK 13, BCLK 12, WS 10, DOUT 9, DIN 48 |
| Speaker amplifier NS4150B enable | 11 (active high) |
| microSD (SDMMC slot 0, LDO channel 4) | 39-44 |
| MIPI CSI camera OV5647 | CSI PHY powered from LDO channel 3 at 2.5 V |
| BOOT button | 35 (ESP32-P4 boot strapping pin, not checked on the schematic) |

- No display is attached: the board runs headless (`NoDisplay`, no backlight). `BOARD_HAS_MIPI_DISPLAY` is 0 in `guition_p4_m3_config.h`.
- The microphone is the ES8311 ADC (the net is named `ES7210_SDOUT` on the schematic, but no ES7210 is fitted).
- Ethernet IP101 (MDC 31, MDIO 52, power 51, clock 50) is on the board but not used by this firmware.
- Board-specific defaults: `sdkconfig.guition-m3-dev` (board type, hosted pin preset, OV5647, Boya flash chip).
- Power it from a 5 V / 2 A supply. On a weak USB port the board browns out (`rst:0x1 POWERON`, COM port disappears) once Wi-Fi, the camera and the speaker run together.
- Tested on a v1.3 board: SD card, OV5647 (35 fps), ES8311 and the Wi-Fi setup AP through the C6 start.

## Board comparison

| | Guition JC-ESP32P4-M3-Dev | Espressif Function-EV | Waveshare ESP32-P4-Module-DEV-KIT |
|---|---|---|---|
| Source | Guition schematic | esp-bsp `esp32_p4_function_ev_board.h` | `docs/ESP32-P4-Module-DEV-KIT_Schematic.pdf` |
| ES8311 I2S | MCLK 13, BCLK 12, WS 10, DOUT 9, DIN 48 | MCLK 13, BCLK 12, WS 10, DOUT 9, DIN 11 | same as Function-EV |
| Amplifier enable | 11 | 53 | 53 (`PA_CTRL` via R52) |
| I2C | SDA 7, SCL 8 | SDA 7, SCL 8 | SDA 7, SCL 8 |
| BOOT button | 35 | 35 | 35 |
| C6 SDIO (slot 1) | CLK 18, CMD 19, D0-D3 14-17, reset 54 | same | same (GPIO 14-19 are internal to the module) |
| Display | none (headless) | EK79007 1024x600 MIPI DSI, backlight 26, reset 27 | 15-pin DSI connector with only DSI lanes, I2C and 3V3: no reset, backlight or touch INT/RST GPIO |
| Touch | none | GT911 (no INT/RST) | GT911 over I2C (no INT/RST) |
| Camera sensors enabled | OV5647 | OV5647, SC2336 | OV5647 |

- The esp_hosted SDIO pins and C6 reset are the same for all three boards. Without a preset (`ESP_HOSTED_P4_DEV_BOARD_NONE`) esp_hosted already defaults to these pins on ESP32-P4.
- Waveshare: the panel driver is still EK79007 1024x600 (Waveshare panels may use another controller and control the backlight over I2C). Without a backlight GPIO the board returns no `Backlight`.

## Switching boards

Each board's hardware follows `CONFIG_BOARD_TYPE_*` automatically:
- Pins and peripherals come from `config.h`, which picks the board header at compile time.
- Selecting a board in menuconfig `imply`s its camera sensors (`CAMERA_OV5647`, `CAMERA_SC2336`).
- `03-build_esp32p4.bat` reads the board from `build_p4/sdkconfig`. "Doi bo mach" regenerates `build_p4/sdkconfig` from that board's defaults.
- Web Configurator: choosing another ESP32-P4 board in **Thiết bị đích** moves `build_p4/sdkconfig` to `sdkconfig.old`, so the next build regenerates it from the new board's defaults. If `build_p4/sdkconfig` names a different board than the one selected (for example after menuconfig), the build tab shows a warning.

## Building (all ESP32-P4 boards)

- Double-click `03-build_esp32p4.bat` in the project root, pick the board, then build or flash. It builds in `build_p4/` with `build_p4/sdkconfig`, so the ESP32-S3 `build/` and `sdkconfig` stay untouched.
- The defaults chain is `sdkconfig.defaults` → `sdkconfig.defaults.esp32p4` → `main/boards/esp32-p4/sdkconfig.<board>`, passed through the `SDKCONFIG_DEFAULTS` environment variable.
- The Web Configurator does the same when **Thiết bị đích** in the build tab is set to an ESP32-P4 board.
- ESP32-P4 builds use their own component lock file `dependencies.lock.esp32p4` (top-level `CMakeLists.txt`).
- The defaults target silicon v1.x (`CONFIG_ESP32P4_SELECTS_REV_LESS_V3`, `CONFIG_ESP32P4_REV_MIN_100`, CPU 360 MHz). A v1.x image does not boot on v3.x silicon and vice versa.
- For v1.x the ISP pipeline controller is off: the prebuilt `esp_ipa` library uses v3-only instructions and crashes with `Illegal instruction`. On v3.x silicon it can be re-enabled (`CONFIG_ESP_VIDEO_ENABLE_ISP_PIPELINE_CONTROLLER=y`).
- `CONFIG_ESP_HOSTED_MEMPOOL_PREFER_SPIRAM=y`: without it the SDIO buffer pool allocation fails at boot (`mempool create failed: no mem`).

Wi-Fi goes through the on-board ESP32-C6 co-processor (`esp_wifi_remote` + `esp_hosted`, SDIO 4-bit). `CONFIG_SLAVE_IDF_TARGET_ESP32C6`, `CONFIG_ESP_HOSTED_SDIO_HOST_INTERFACE` and `CONFIG_ESP_HOSTED_SDIO_4_BIT_BUS` are in `sdkconfig.defaults.esp32p4`; the pin preset `CONFIG_ESP_HOSTED_P4_DEV_BOARD_FUNC_BOARD` is in the per-board defaults.

The board code initializes I2C, LCD, touch, audio, SDMMC and camera directly and does not depend on the `espressif/esp32_p4_function_ev_board` BSP.

Waveshare schematic and mechanical drawings: [docs/](docs/).
