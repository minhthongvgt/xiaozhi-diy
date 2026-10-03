# Xiaozhi Stability Fixes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reconfigure ESP32-S3 task priorities, disable Wi-Fi power save, and optimize PSRAM cache settings to prevent watchdog resets and audio stuttering on the Xiaozhi-v2 project.

**Architecture:** Modifies existing C++ audio and board configuration modules to lower Opus/AFE thread priorities relative to I2S. Updates `sdkconfig.defaults.esp32s3` to enable IRAM-saving SPIRAM configurations. Disables Wi-Fi modem sleep during BluFi and Station startup.

**Tech Stack:** C++, ESP-IDF, FreeRTOS, CMake.

**Spec:** Current user request and analysis report.

## Global Constraints
- Target ESP32-S3-N16R8.
- Compilation must not produce `-Wunused` errors.
- Ensure all SDK configs match ESP-IDF standard formatting.

## Review Focus
- **Wi-Fi Power Save:** The application might crash if `esp_wifi_set_ps()` is called before `esp_wifi_init()` or after Wi-Fi is stopped. Ensure it's called immediately after `esp_wifi_start()`.
- **Priority Scaling:** Setting Opus too low could cause audio output stuttering. We set it to 3 to keep it above background UI tasks but below critical I2S tasks (23).
- **Wi-Fi Power Save Context:** In `blufi.cpp`, `esp_wifi_start()` is called inside `_wifi_init`. `esp_wifi_set_ps` must be protected by checking `err == ESP_OK`.

---

### Task 1: Khắc Phục Priority của Opus và AFE/WakeWord

**Files:**
- Modify: `main/audio/audio_service.cc`
- Modify: `main/audio/engines/afe_audio_engine.cc`
- Modify: `main/audio/wake_words/custom_wake_word.cc`

**Interfaces:**
- Consumes: Task priority arguments in `xTaskCreateStaticPinnedToCore`.
- Produces: Lower-priority processing tasks, freeing up CPU time for I2S (`audio_input`/`audio_output`).

- [ ] **Step 1: Lower Opus Codec Priority**
  - Modify `main/audio/audio_service.cc`.
  - Locate `opus_codec_task_handle_ = xTaskCreateStaticPinnedToCore(`.
  - Change the priority argument from `21` to `3`.

- [ ] **Step 2: Lower AFE Processing Priority**
  - Modify `main/audio/engines/afe_audio_engine.cc`.
  - Locate `processing_task_ = xTaskCreateStaticPinnedToCore(`.
  - Change the priority argument from `19` to `5`.

- [ ] **Step 3: Lower Wake Word Encode Priority (AFE Engine)**
  - Modify `main/audio/engines/afe_audio_engine.cc`.
  - Locate `wake_word_encode_task_ = xTaskCreateStaticPinnedToCore(`.
  - Change the priority argument from `15` to `4`.

- [ ] **Step 4: Lower Wake Word Encode Priority (Custom Wake Word)**
  - Modify `main/audio/wake_words/custom_wake_word.cc`.
  - Locate `wake_word_encode_task_ = xTaskCreateStaticPinnedToCore(`.
  - Change the priority argument from `15` to `4`.

- [ ] **Step 5: Verify Compilation**
  - Run: `idf.py build` (or equivalent test build)
  - Expected: PASS

---

### Task 2: Tối ưu hoá Kconfig (Cấp phát Code vào PSRAM)

**Files:**
- Modify: `sdkconfig.defaults.esp32s3`

**Interfaces:**
- Consumes: None.
- Produces: ESP-IDF build configuration variables.

- [ ] **Step 1: Thêm cấu hình tối ưu PSRAM và FreeRTOS**
  - Modify `sdkconfig.defaults.esp32s3`.
  - Append the following configurations to the bottom of the file to force instructions/rodata to PSRAM and increase tick rate:
```kconfig
# Optimize memory and RTOS responsiveness for S3 N16R8
CONFIG_SPIRAM_FETCH_INSTRUCTIONS=y
CONFIG_SPIRAM_RODATA=y
CONFIG_FREERTOS_HZ=1000
```

- [ ] **Step 2: Verify Configuration**
  - Run: `idf.py reconfigure`
  - Expected: PASS without config errors.

---

### Task 3: Tắt Wi-Fi Power Save để chống rớt gói tin

**Files:**
- Modify: `main/boards/common/blufi.cpp`

**Interfaces:**
- Consumes: `esp_wifi.h` API.
- Produces: A Wi-Fi interface running without power-save mode to reduce latency.

- [ ] **Step 1: Include esp_wifi.h**
  - Check `main/boards/common/blufi.cpp`. If `#include "esp_wifi.h"` is not present, add it near the top.

- [ ] **Step 2: Disable Power Save after esp_wifi_start()**
  - Modify `main/boards/common/blufi.cpp`.
  - Locate calls to `esp_wifi_start();` in the `_wifi_init` function (around line 706 and 727 depending on the AP/STA branches).
  - Immediately after a successful start, inject the `esp_wifi_set_ps` call:
```cpp
        err = esp_wifi_start();
        if (err == ESP_OK) {
            esp_wifi_set_ps(WIFI_PS_NONE);
        }
```
  - Apply this to all places in that file where `esp_wifi_start()` is called to ensure STA, AP, and APSTA modes all run without power save.

- [ ] **Step 3: Compile and Verify**
  - Run: `idf.py build`
  - Expected: PASS
