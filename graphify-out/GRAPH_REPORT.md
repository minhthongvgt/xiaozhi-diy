# Graph Report - xiaozhi-v2  (2026-10-03)

## Corpus Check
- cluster-only mode — file stats not available

## Summary
- 4277 nodes · 7875 edges · 224 communities (149 shown, 75 thin omitted)
- Extraction: 94% EXTRACTED · 6% INFERRED · 0% AMBIGUOUS · INFERRED: 477 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- McpServer
- gifdec.c
- CircularStrip
- AudioService
- Xpowers
- AfeAudioEngine
- GpioLed
- CustomN16R8Board
- LvglTheme
- NotifyPlayer
- LvglDisplay
- ui_helpers.c
- EspWakeWord
- build_default_assets.py
- spiffs_assets_gen.py
- emote_display.cc
- OggDemuxer
- ethernet_board.cc
- Nt26Board
- AdaptiveUiEngine
- CustomWakeWord
- memory
- rotary_encoder.c
- GUIController
- Application
- Button
- versions.py
- UartDisplay
- AudioCodec
- RndisBoard
- sensor_manager.c
- LcdDisplay
- Es8374AudioCodec
- DualNetworkBoard
- Protocol
- GUIController
- Ml307Board
- OledDisplay
- wifi_board.cc
- board.h
- _collect_variants
- cstring
- os
- Es8311AudioCodec
- EspVideo
- SingleLed
- actuator_manager.c
- BoxAudioCodec
- Es8388AudioCodec
- Es8389AudioCodec
- LVGLImage.py
- validate_sdkconfig
- blufi.cpp
- Blufi
- Board
- LiteAudioEngine
- Ota
- scripts/build.py
- SleepTimer
- app_main.c
- websocket_protocol.cc
- build_board
- no_audio_codec.cc
- DeviceStateMachine
- MqttProtocol
- protocol.h
- PowerSaveTimer
- spiffs_assets/build.py
- AdcBatteryMonitor
- configurator_server.py
- Settings
- Led
- DualDisplay
- gpio_validator.py
- BuildController
- sdkconfig
- esp_video.cc
- system_info.cc
- ui_state_manager.c
- LVGLImage
- esp_log
- Backlight
- test_bugs_regression.old.js
- sdkconfig_io.py
- BuildController
- image_to_jpeg.cpp
- Esp32Camera
- test_build.py
- audio_service.h
- lcd_display.cc
- bus_manager.c
- DynamicGlyphCache
- LvglThemeManager
- T
- display_init.c
- esp_lcd_ili9486.c
- esp_lcd_jd9853.c
- esp_lcd_nv3030b.c
- AudioEngine
- vl6180x.c
- BuildOptionTests
- LVGLImageHeader
- Knob
- esp_lcd_st7735.c
- Display
- single_led.cc
- ImageConverterApp
- AudioConverterApp
- AdaptiveUiTests
- app.js
- mq_sensor.c
- assets.cc
- mqtt_protocol.cc
- AdaptiveUiConfig
- ScreenMetrics
- RLEImage
- Assets
- AudioStreamPacket
- TextGlyph
- AudioConverterApp
- TargetConfigurationTests
- .do_GET
- FixedQueue
- detect_idf
- LvglStrategy
- WakeWordAudioCache
- lv_font_t
- .StartMqttClient
- ColorFormat
- ._handle_event
- jpeg_to_image.c
- BlufiSecurity
- NetworkEvent
- .GetInt
- system_reset.cc
- uint32_t
- stdlib
- ProjectContext
- AssetStrategy
- Theme
- P3PlayerApp
- build_all.py
- BoardSelectionTests
- Camera
- Entry
- pack_model.py
- _resolve_board_config
- CliTests
- VersionTests
- guiController.js
- test_integration.js
- argparse
- bmp280.c
- BuildJobManager
- dht.c
- hcsr04.c
- ina219.c
- .Schedule
- TextFontCapability
- AudioServiceCallbacks
- scd4x.c
- test_modules.mjs
- Path
- SdkconfigParser
- lcd_display.h
- audio_init.c
- _sdkconfig_assignments
- BuildDefaultAssetsTest
- SdkconfigParser
- bh1750.c
- PowerSaveLevel
- Nt26CeregState
- UiStyle
- is_valid_xiaozhi_project
- ListeningMode
- .Download
- mmap_assets_table
- DebugStatistics
- storage_init
- clean_comments.py
- AudioDebugger
- Command
- .SetEmojiCollection
- DisplayLockGuard
- NoLed
- EffectMode
- actuator_init
- TaskPriorityReset
- detect_com_ports
- WakeDetector
- AbortReason
- _wake_word_sdkconfig_options
- PngQuant
- BoardMenuTests
- VariantSelectionTests
- mp3_to_ogg.sh

## God Nodes (most connected - your core abstractions)
1. `AudioService` - 106 edges
2. `Application` - 103 edges
3. `Board` - 89 edges
4. `AfeAudioEngine` - 82 edges
5. `AdaptiveUiEngine` - 70 edges
6. `CircularStrip` - 58 edges
7. `CustomWakeWord` - 51 edges
8. `Blufi` - 51 edges
9. `SingleLed` - 44 edges
10. `GpioLed` - 44 edges

## Surprising Connections (you probably didn't know these)
- `ImageConverterApp` --uses--> `ColorFormat`  [INFERRED]
  scripts/Image_Converter/lvgl_tools_gui.py → scripts/Image_Converter/LVGLImage.py
- `ImageConverterApp` --uses--> `CompressMethod`  [INFERRED]
  scripts/Image_Converter/lvgl_tools_gui.py → scripts/Image_Converter/LVGLImage.py
- `ImageConverterApp` --uses--> `LVGLImage`  [INFERRED]
  scripts/Image_Converter/lvgl_tools_gui.py → scripts/Image_Converter/LVGLImage.py
- `Assets::LvglStrategy::UnApplyPartition()` --references--> `Assets`  [EXTRACTED]
  main/assets.cc → main/assets.h
- `MqttProtocol::MqttProtocol()` --references--> `Application`  [EXTRACTED]
  main/protocols/mqtt_protocol.cc → main/application.h

## Import Cycles
- None detected.

## Communities (224 total, 75 thin omitted)

### Community 0 - "McpServer"
Cohesion: 0.05
Nodes (29): LampController, gpio_num_, power_, ImageContent, encoded_data_, mime_type_, McpServer, tools_ (+21 more)

### Community 1 - "gifdec.c"
Cohesion: 0.07
Nodes (38): add_entry(), discard_sub_blocks(), dispose(), f_gif_close(), f_gif_open(), f_gif_read(), f_gif_seek(), gd_close_gif() (+30 more)

### Community 2 - "CircularStrip"
Cohesion: 0.05
Nodes (27): CircularStrip, blink_counter_, blink_interval_ms_, blink_on_, breathe_color_, breathe_up_, CircularStrip::CircularStrip(), colors_ (+19 more)

### Community 3 - "AudioService"
Cohesion: 0.03
Nodes (47): AudioService, audio_debugger_, audio_decode_queue_, audio_encode_queue_, audio_engine_, audio_engine_initialized_, audio_input_need_warmup_, audio_input_task_handle_ (+39 more)

### Community 4 - "Xpowers"
Cohesion: 0.06
Nodes (12): Axp2101, Axp2101::Axp2101(), I2cDevice, device_address_, i2c_bus_, i2c_device_, I2cDevice::I2cDevice(), Sy6970 (+4 more)

### Community 5 - "AfeAudioEngine"
Cohesion: 0.04
Nodes (39): AfeAudioEngine, afe_control_dirty_, afe_data_, afe_iface_, codec_, control_generation_, custom_wake_word_, device_aec_enabled_ (+31 more)

### Community 6 - "GpioLed"
Cohesion: 0.06
Nodes (16): GpioLed, blink_counter_, blink_interval_ms_, blink_task_, blink_timer_, brightness_, custom_mode_, duty_ (+8 more)

### Community 7 - "CustomN16R8Board"
Cohesion: 0.07
Nodes (17): CreateUserCustomAudioCodecDriver(), CreateUserCustomCameraDriver(), CreateUserCustomDisplayDriver(), CreateUserCustomTouchDriver(), InitializeUserCustomSensors(), CustomN16R8Board, backlight_, boot_button_ (+9 more)

### Community 8 - "LvglTheme"
Cohesion: 0.09
Nodes (3): EmojiCollection, emoji_collection_, LvglTheme

### Community 9 - "NotifyPlayer"
Cohesion: 0.06
Nodes (24): IsSupportedUrl(), NotifyPlayer, active_, audio_url_, cancelled_, completion_reported_, displayed_text_, finished_callback_ (+16 more)

### Community 10 - "LvglDisplay"
Cohesion: 0.05
Nodes (18): DynamicGlyphCache, LvglDisplay, battery_icon_, battery_label_, display_, dynamic_glyph_cache_, last_displayed_clock_min_, last_status_update_time_ (+10 more)

### Community 11 - "ui_helpers.c"
Cohesion: 0.08
Nodes (38): scr_unloaded_delete_cb(), _ui_anim_callback_free_user_data(), _ui_anim_callback_get_height(), _ui_anim_callback_get_image_angle(), _ui_anim_callback_get_image_frame(), _ui_anim_callback_get_image_zoom(), _ui_anim_callback_get_opacity(), _ui_anim_callback_get_width() (+30 more)

### Community 12 - "EspWakeWord"
Cohesion: 0.05
Nodes (12): WakeWord, EspWakeWord, codec_, input_buffer_, input_buffer_mutex_, last_detected_wake_word_, owns_models_, running_ (+4 more)

### Community 13 - "build_default_assets.py"
Cohesion: 0.08
Nodes (25): build_assets_integrated(), compute_checksum(), copy_directory(), copy_file(), copy_wakenet_model(), ensure_dir(), generate_config_json(), generate_index_json() (+17 more)

### Community 15 - "spiffs_assets_gen.py"
Cohesion: 0.08
Nodes (18): AssetCopyConfig, compute_checksum(), convert_image_to_qoi(), convert_image_to_raw(), convert_image_to_simg(), copy_assets(), create_header(), download_v8_script() (+10 more)

### Community 16 - "emote_display.cc"
Cohesion: 0.09
Nodes (6): EmoteDisplay, emote_handle_, EmoteDisplay::EmoteDisplay(), InitializeEmote(), OnFlushCallback(), OnFlushIoReady()

### Community 17 - "OggDemuxer"
Cohesion: 0.06
Nodes (30): context_t, body_offset, body_size, bytes_needed, data_offset, header, packet_buf, packet_continued (+22 more)

### Community 18 - "ethernet_board.cc"
Cohesion: 0.08
Nodes (9): CreateEmacConfig(), CreatePhy(), EthernetBoard, connected_, eth_glue_, eth_handle_, eth_netif_, ip_address_ (+1 more)

### Community 19 - "Nt26Board"
Cohesion: 0.08
Nodes (12): Nt26Board, current_power_level_, dtr_pin_, modem_, network_event_callback_, network_ready_timer_, Nt26Board::Nt26Board(), pm_lock_cpu_max_ (+4 more)

### Community 20 - "AdaptiveUiEngine"
Cohesion: 0.06
Nodes (30): AdaptiveUiEngine, bar_audio_wave_, bar_ota_progress_, config_, container_root_, initialized_, label_avatar_, label_battery_ (+22 more)

### Community 22 - "CustomWakeWord"
Cohesion: 0.06
Nodes (23): CustomWakeWord, codec_, commands_, duration_, input_buffer_, input_buffer_mutex_, language_, last_detected_wake_word_ (+15 more)

### Community 23 - "memory"
Cohesion: 0.10
Nodes (7): LvglTheme, LvglFont, LvglAllocatedImage, valid_, LvglCBinImage, LvglImage, LvglRawImage

### Community 24 - "rotary_encoder.c"
Cohesion: 0.08
Nodes (16): input_init(), input_manager_get_encoder_diff(), input_manager_get_encoder_value(), input_manager_get_touch_slider(), input_manager_init(), input_manager_is_encoder_key_pressed(), rotary_encoder_deinit(), rotary_encoder_get_diff() (+8 more)

### Community 27 - "Application"
Cohesion: 0.06
Nodes (25): AecMode, kAecOff, kAecOnDeviceSide, kAecOnServerSide, Application, aborted_, activation_task_handle_, aec_mode_ (+17 more)

### Community 28 - "Button"
Cohesion: 0.09
Nodes (13): AdcButton, AdcButton::AdcButton(), Button, Button::Button(), button_handle_, gpio_num_, on_click_, on_double_click_ (+5 more)

### Community 29 - "versions.py"
Cohesion: 0.09
Nodes (16): download_artifact(), get_artifacts(), get_default_releases_dir(), main(), parse_github_run_url(), rename_artifact(), extract_zip(), find_app_partition() (+8 more)

### Community 30 - "UartDisplay"
Cohesion: 0.13
Nodes (12): escape_nextion_string(), UartDisplay, is_initialized_, mutex_, protocol_, uart_num_, UartDisplay::UartDisplay(), UartDisplayProtocol (+4 more)

### Community 31 - "AudioCodec"
Cohesion: 0.08
Nodes (4): AudioCodec, rx_handle_, tx_handle_, volume_save_timer_

### Community 32 - "RndisBoard"
Cohesion: 0.09
Nodes (5): RndisBoard, network_event_callback_, rndis_eth_driver, s_event_group, s_rndis_netif

### Community 33 - "sensor_manager.c"
Cohesion: 0.11
Nodes (11): bus_manager_get_i2c_bus(), get_vl6180x_dev(), sensor_get_dht_pin(), sensor_get_mq_pin(), sensor_manager_init(), sensor_manager_read_all(), sensor_read_distance(), sensor_read_environment() (+3 more)

### Community 34 - "LcdDisplay"
Cohesion: 0.07
Nodes (20): LcdDisplay, bottom_bar_, chat_message_label_, container_, content_, draw_buf_, emoji_box_, emoji_image_ (+12 more)

### Community 35 - "Es8374AudioCodec"
Cohesion: 0.08
Nodes (10): Es8374AudioCodec, codec_if_, ctrl_if_, data_if_, data_if_mutex_, Es8374AudioCodec::Es8374AudioCodec(), gpio_if_, input_dev_ (+2 more)

### Community 36 - "DualNetworkBoard"
Cohesion: 0.11
Nodes (9): DualNetworkBoard, current_board_, ml307_dtr_pin_, ml307_rx_pin_, ml307_tx_pin_, network_type_, NetworkType, ML307 (+1 more)

### Community 37 - "Protocol"
Cohesion: 0.10
Nodes (4): Protocol, error_occurred_, last_incoming_time_, session_id_

### Community 39 - "Ml307Board"
Cohesion: 0.10
Nodes (7): Ml307Board, dtr_pin_, Ml307Board::Ml307Board(), modem_, network_event_callback_, rx_pin_, tx_pin_

### Community 40 - "OledDisplay"
Cohesion: 0.09
Nodes (12): OledDisplay, chat_message_label_, container_, content_, content_left_, content_right_, emotion_label_, panel_ (+4 more)

### Community 41 - "wifi_board.cc"
Cohesion: 0.11
Nodes (4): WifiBoard, connect_timer_, in_config_mode_, network_event_callback_

### Community 43 - "_collect_variants"
Cohesion: 0.09
Nodes (15): _board_source_text(), _build_option_definitions(), _build_options_sdkconfig(), _collect_variants(), _get_builds_for_idf(), _get_full_name(), _get_manufacturer(), _get_release_full_name() (+7 more)

### Community 44 - "cstring"
Cohesion: 0.11
Nodes (5): DualDisplay, LvglImage, operator==(), TextGlyphAllocator, TextGlyphStorageUsesPsram()

### Community 45 - "os"
Cohesion: 0.13
Nodes (5): encode_audio_to_opus(), decode_p3_to_audio(), play_p3_file(), main(), play_p3_file()

### Community 46 - "Es8311AudioCodec"
Cohesion: 0.10
Nodes (10): Es8311AudioCodec, codec_if_, ctrl_if_, data_if_, data_if_mutex_, dev_, Es8311AudioCodec::Es8311AudioCodec(), gpio_if_ (+2 more)

### Community 47 - "EspVideo"
Cohesion: 0.08
Nodes (18): EspVideo, encoder_thread_, explain_token_, explain_url_, frame_, mmap_buffers_, sensor_format_, streaming_on_ (+10 more)

### Community 48 - "SingleLed"
Cohesion: 0.08
Nodes (15): SingleLed, b_, blink_counter_, blink_interval_ms_, breathe_up_, breathe_val_, custom_mode_, g_ (+7 more)

### Community 49 - "actuator_manager.c"
Cohesion: 0.13
Nodes (16): RobotMcpController, actuator_beep(), actuator_get_relay(), actuator_get_servo_angle(), actuator_is_servo_attached(), actuator_manager_init(), actuator_servo_detach(), actuator_set_dc_motor() (+8 more)

### Community 50 - "BoxAudioCodec"
Cohesion: 0.09
Nodes (13): BoxAudioCodec, BoxAudioCodec::BoxAudioCodec(), data_if_, data_if_mutex_, gpio_if_, in_codec_if_, in_ctrl_if_, input_dev_ (+5 more)

### Community 51 - "Es8388AudioCodec"
Cohesion: 0.09
Nodes (11): Es8388AudioCodec, codec_if_, ctrl_if_, data_if_, data_if_mutex_, Es8388AudioCodec::Es8388AudioCodec(), gpio_if_, input_dev_ (+3 more)

### Community 52 - "Es8389AudioCodec"
Cohesion: 0.09
Nodes (11): Es8389AudioCodec, codec_if_, ctrl_if_, data_if_, data_if_mutex_, Es8389AudioCodec::Es8389AudioCodec(), gpio_if_, input_dev_ (+3 more)

### Community 54 - "LVGLImage.py"
Cohesion: 0.11
Nodes (11): bit_extend(), CompressMethod, main(), NotSupported, OutputFormat, PNGConverter, RAWImage, test() (+3 more)

### Community 55 - "validate_sdkconfig"
Cohesion: 0.11
Nodes (3): parse_sdkconfig_file(), validate_sdkconfig(), GpioValidatorTests

### Community 57 - "Blufi"
Cohesion: 0.08
Nodes (18): Blufi, inited_, m_ap_records, m_ble_is_connected, m_deinited, m_provisioned, m_scan_in_progress, m_scan_should_save_ssid (+10 more)

### Community 60 - "LiteAudioEngine"
Cohesion: 0.08
Nodes (16): EspWakeWord, LiteAudioEngine, codec_, empty_wake_word_, frame_samples_, models_list_, output_buffer_, output_callback_ (+8 more)

### Community 61 - "Ota"
Cohesion: 0.08
Nodes (17): Ota, activation_challenge_, activation_code_, activation_message_, activation_timeout_ms_, current_version_, firmware_url_, firmware_version_ (+9 more)

### Community 62 - "scripts/build.py"
Cohesion: 0.13
Nodes (14): _board_type_exists(), _collect_languages(), _collect_wake_words(), _detect_idf_version(), _detect_idf_version_for_listing(), _language_sdkconfig_option(), main(), _normalize_language() (+6 more)

### Community 63 - "SleepTimer"
Cohesion: 0.12
Nodes (10): SleepTimer, enabled_, in_light_sleep_mode_, on_enter_deep_sleep_mode_, on_enter_light_sleep_mode_, on_exit_light_sleep_mode_, seconds_to_deep_sleep_, seconds_to_light_sleep_ (+2 more)

### Community 64 - "app_main.c"
Cohesion: 0.11
Nodes (8): app_main_hardware_init(), init_event_loop(), init_nvs(), sensor_init(), gpio_is_pin_safe(), gpio_validator_get_error_log(), gpio_validator_run(), app_main()

### Community 65 - "websocket_protocol.cc"
Cohesion: 0.11
Nodes (4): WebsocketProtocol, event_group_handle_, version_, websocket_

### Community 66 - "build_board"
Cohesion: 0.10
Nodes (14): build_board(), _configure_build(), _configured_target(), _emit_build_stage(), _get_idf_command(), get_project_version(), merge_bin(), _prepare_target() (+6 more)

### Community 67 - "no_audio_codec.cc"
Cohesion: 0.12
Nodes (10): NoAudioCodec, data_if_mutex_, rx_buffer_, tx_buffer_, NoAudioCodecDuplex, NoAudioCodecDuplex::NoAudioCodecDuplex(), NoAudioCodecSimplex, NoAudioCodecSimplex::NoAudioCodecSimplex() (+2 more)

### Community 68 - "DeviceStateMachine"
Cohesion: 0.15
Nodes (5): DeviceStateMachine, current_state_, listeners_, mutex_, next_listener_id_

### Community 69 - "MqttProtocol"
Cohesion: 0.09
Nodes (15): MqttProtocol, aes_key_id_, aes_nonce_, alive_, channel_mutex_, crypto_mutex_, event_group_handle_, local_sequence_ (+7 more)

### Community 70 - "protocol.h"
Cohesion: 0.10
Nodes (14): CJsonDeleter, CJsonStringDeleter, BinaryProtocol2, payload, payload_size, reserved, timestamp, type (+6 more)

### Community 72 - "PowerSaveTimer"
Cohesion: 0.13
Nodes (12): PowerSaveTimer, cpu_max_freq_, enabled_, in_sleep_mode_, is_wake_word_running_, on_enter_sleep_mode_, on_exit_sleep_mode_, on_shutdown_request_ (+4 more)

### Community 73 - "spiffs_assets/build.py"
Cohesion: 0.17
Nodes (14): copy_directory(), copy_file(), ensure_dir(), generate_config_json(), generate_index_json(), load_emoji_config(), main(), process_board_collection() (+6 more)

### Community 74 - "AdcBatteryMonitor"
Cohesion: 0.12
Nodes (7): AdcBatteryMonitor, adc_battery_estimation_handle_, AdcBatteryMonitor::AdcBatteryMonitor(), charging_pin_, is_charging_, on_charging_status_changed_, timer_handle_

### Community 75 - "configurator_server.py"
Cohesion: 0.10
Nodes (8): detect_target(), parse_lines_to_dict(), read_active_config(), read_sdkconfig_defaults(), reset_custom_idf_config(), save_configuration(), sync_sdkconfig(), write_sdkconfig_defaults()

### Community 76 - "Settings"
Cohesion: 0.16
Nodes (6): Settings, dirty_, ns_, nvs_handle_, read_write_, Settings::Settings()

### Community 78 - "DualDisplay"
Cohesion: 0.11
Nodes (3): DualDisplay, primary_, secondary_

### Community 79 - "gpio_validator.py"
Cohesion: 0.14
Nodes (9): _validate_gpio_configuration(), extract_pin_assignments(), format_validation_report(), _get_int(), _is_yes(), main(), PinAssignment, safe_print() (+1 more)

### Community 81 - "sdkconfig"
Cohesion: 0.13
Nodes (7): Esp32Camera::Esp32Camera(), JpegChunk, data, len, JpegChunk, data, len

### Community 83 - "esp_video.cc"
Cohesion: 0.12
Nodes (3): esp_video_deinit(), EspVideo::EspVideo(), log_available_video_devices()

### Community 87 - "ui_state_manager.c"
Cohesion: 0.12
Nodes (4): ui_Screen1_screen_destroy(), ui_Screen1_screen_init(), ui_destroy(), ui_init()

### Community 89 - "esp_log"
Cohesion: 0.13
Nodes (5): ActuatorController, CellularMcpController, IrMcpController, LedMcpController, SensorController

### Community 90 - "Backlight"
Cohesion: 0.13
Nodes (6): Backlight, step_, target_brightness_, transition_timer_, PwmBacklight, PwmBacklight::PwmBacklight()

### Community 91 - "test_bugs_regression.old.js"
Cohesion: 0.11
Nodes (10): assert, {
  DEFAULT_CONFIG,
  ConfigParser,
  CodeGenerator,
  PinValidator
}, fs, path, componentsDir, fs, html, htmlPath (+2 more)

### Community 92 - "sdkconfig_io.py"
Cohesion: 0.16
Nodes (9): apply_to_kconfig_file(), _atomic_write(), detect_target(), load_effective(), load_effective_text(), _normalize_lines(), parse_kconfig_text(), _read() (+1 more)

### Community 93 - "BuildController"
Cohesion: 0.12
Nodes (3): BuildController, bootstrap(), loadComponents()

### Community 94 - "image_to_jpeg.cpp"
Cohesion: 0.22
Nodes (10): convert_input_to_encoder_buf(), convert_input_to_hw_encoder_buf(), encode_with_esp_new_jpeg(), encode_with_hw_jpeg(), expand_5_to_8(), expand_6_to_8(), hw_jpeg_ensure_inited(), image_to_jpeg() (+2 more)

### Community 95 - "Esp32Camera"
Cohesion: 0.12
Nodes (9): Esp32Camera, current_fb_, encode_buf_, encode_buf_size_, encoder_thread_, explain_token_, explain_url_, streaming_on_ (+1 more)

### Community 96 - "test_build.py"
Cohesion: 0.12
Nodes (3): BoardSourceTests, InvalidConfigTests, PreviewTargetTests

### Community 97 - "audio_service.h"
Cohesion: 0.12
Nodes (10): AudioTask, media_position_ms, pcm, playback_id, timestamp, type, AudioTaskType, kAudioTaskTypeDecodeToPlaybackQueue (+2 more)

### Community 98 - "lcd_display.cc"
Cohesion: 0.15
Nodes (4): LcdDisplay::LcdDisplay(), MipiLcdDisplay::MipiLcdDisplay(), RgbLcdDisplay::RgbLcdDisplay(), SpiLcdDisplay::SpiLcdDisplay()

### Community 99 - "bus_manager.c"
Cohesion: 0.19
Nodes (10): bus_manager_add_i2c_device(), bus_manager_i2c_lock(), bus_manager_i2c_unlock(), bus_manager_init(), bus_manager_init_i2c(), bus_manager_init_spi(), bus_manager_probe_i2c_address(), bus_manager_set_i2c_bus() (+2 more)

### Community 100 - "DynamicGlyphCache"
Cohesion: 0.11
Nodes (14): DynamicGlyphCache, bitmap_blob_, bpp_, cmaps_, dsc_, entries_, font_, glyph_dsc_ (+6 more)

### Community 101 - "LvglThemeManager"
Cohesion: 0.14
Nodes (4): LvglTheme::LvglTheme(), LvglThemeManager, themes_, OledDisplay::OledDisplay()

### Community 102 - "T"
Cohesion: 0.22
Nodes (3): r(), T, w()

### Community 103 - "display_init.c"
Cohesion: 0.15
Nodes (4): board_init(), display_init(), display_ledc_init(), display_subsystem_init()

### Community 104 - "esp_lcd_ili9486.c"
Cohesion: 0.24
Nodes (10): esp_lcd_new_panel_ili9486(), panel_ili9486_del(), panel_ili9486_disp_on_off(), panel_ili9486_draw_bitmap(), panel_ili9486_init(), panel_ili9486_invert_color(), panel_ili9486_mirror(), panel_ili9486_reset() (+2 more)

### Community 105 - "esp_lcd_jd9853.c"
Cohesion: 0.24
Nodes (10): esp_lcd_new_panel_jd9853(), panel_jd9853_del(), panel_jd9853_disp_on_off(), panel_jd9853_draw_bitmap(), panel_jd9853_init(), panel_jd9853_invert_color(), panel_jd9853_mirror(), panel_jd9853_reset() (+2 more)

### Community 106 - "esp_lcd_nv3030b.c"
Cohesion: 0.24
Nodes (10): esp_lcd_new_panel_nv3030b(), panel_nv3030b_del(), panel_nv3030b_disp_on_off(), panel_nv3030b_draw_bitmap(), panel_nv3030b_init(), panel_nv3030b_invert_color(), panel_nv3030b_mirror(), panel_nv3030b_reset() (+2 more)

### Community 109 - "vl6180x.c"
Cohesion: 0.37
Nodes (12): vl6180x_deinit(), vl6180x_get_gain_multiplier(), vl6180x_init(), vl6180x_read_ambient_lux(), vl6180x_read_distance_mm(), vl6180x_read_reg16(), vl6180x_read_reg8(), vl6180x_set_als_gain() (+4 more)

### Community 111 - "LVGLImageHeader"
Cohesion: 0.16
Nodes (4): Error, FormatError, LVGLImageHeader, ParameterError

### Community 112 - "Knob"
Cohesion: 0.15
Nodes (6): Knob, Knob::Knob(), knob_handle_, on_rotate_, pin_a_, pin_b_

### Community 113 - "esp_lcd_st7735.c"
Cohesion: 0.27
Nodes (10): esp_lcd_new_panel_st7735(), panel_st7735_del(), panel_st7735_disp_on_off(), panel_st7735_draw_bitmap(), panel_st7735_init(), panel_st7735_invert_color(), panel_st7735_mirror(), panel_st7735_reset() (+2 more)

### Community 114 - "Display"
Cohesion: 0.12
Nodes (3): Display, current_theme_, setup_ui_called_

### Community 119 - "app.js"
Cohesion: 0.17
Nodes (8): configState, conflictErrors, DEFAULT_CONFIG, ESP32S3, PinValidator, SdkconfigGenerator, { SdkconfigParser, SdkconfigGenerator }, { SdkconfigParser, SdkconfigGenerator, PinValidator, DEFAULT_CONFIG, ESP32S3 }

### Community 120 - "mq_sensor.c"
Cohesion: 0.22
Nodes (7): gpio_to_adc1_channel(), mq_sensor_calibrate(), mq_sensor_deinit(), mq_sensor_init(), mq_sensor_is_leak_alert(), mq_sensor_read(), sensor_set_mq_pin()

### Community 121 - "assets.cc"
Cohesion: 0.17
Nodes (4): Assets::EmoteStrategy::InitializePartition(), Assets::EmoteStrategy::UnApplyPartition(), Assets::LvglStrategy::InitializePartition(), Assets::LvglStrategy::UnApplyPartition()

### Community 123 - "AdaptiveUiConfig"
Cohesion: 0.13
Nodes (14): AdaptiveUiConfig, accent, auto_scale, round_screen_safe_area, style, voice_wave_enabled, weather_city, weather_enabled (+6 more)

### Community 124 - "ScreenMetrics"
Cohesion: 0.13
Nodes (15): ScreenMetrics, content_height, content_width, corner_radius, height, is_compact, is_landscape, is_large (+7 more)

### Community 126 - "Assets"
Cohesion: 0.16
Nodes (7): Assets, Assets::EmoteStrategy::Apply(), Assets::LvglStrategy::Apply(), models_data_, models_list_, partition_, strategy_

### Community 127 - "AudioStreamPacket"
Cohesion: 0.16
Nodes (7): AudioStreamPacket, frame_duration, media_position_ms, payload, playback_id, sample_rate, timestamp

### Community 128 - "TextGlyph"
Cohesion: 0.14
Nodes (9): TextGlyph, adv_w, bitmap, box_h, box_w, codepoint, ofs_x, ofs_y (+1 more)

### Community 130 - "TargetConfigurationTests"
Cohesion: 0.16
Nodes (3): TargetConfigurationTests, temporary_working_directory(), ZipTests

### Community 131 - ".do_GET"
Cohesion: 0.28
Nodes (4): ConfiguratorHandler, origin_allowed(), safe_baud(), safe_port()

### Community 132 - "FixedQueue"
Cohesion: 0.21
Nodes (3): FixedQueue, head_, storage_

### Community 133 - "detect_idf"
Cohesion: 0.18
Nodes (7): detect_idf(), extract_idf_version(), get_custom_idf_config(), resolve_idf_path(), save_custom_idf_config(), scan_all_idf_installations(), add_candidate()

### Community 134 - "LvglStrategy"
Cohesion: 0.15
Nodes (8): Asset, offset, size, LvglStrategy, assets_, checksum_valid_, mmap_handle_, mmap_root_

### Community 136 - "WakeWordAudioCache"
Cohesion: 0.22
Nodes (5): WakeWordAudioCache, buffer_, capacity_, mutex_, write_position_

### Community 138 - "lv_font_t"
Cohesion: 0.23
Nodes (3): LvglBuiltInFont, LvglCBinFont, LvglFont

### Community 139 - ".StartMqttClient"
Cohesion: 0.21
Nodes (7): on_audio_channel_closed_, on_audio_channel_opened_, on_connected_, on_disconnected_, on_incoming_audio_, on_incoming_json_, on_network_error_

### Community 143 - "jpeg_to_image.c"
Cohesion: 0.23
Nodes (3): decode_with_hardware_jpeg(), decode_with_new_jpeg(), jpeg_to_image()

### Community 144 - "BlufiSecurity"
Cohesion: 0.17
Nodes (10): BlufiSecurity, aes_key, dec_operation, dh_param, dh_param_len, enc_operation, psk, self_public_key (+2 more)

### Community 145 - "NetworkEvent"
Cohesion: 0.17
Nodes (12): NetworkEvent, Connected, Connecting, Disconnected, ModemDetecting, ModemErrorInitFailed, ModemErrorNoSim, ModemErrorRegDenied (+4 more)

### Community 147 - "system_reset.cc"
Cohesion: 0.27
Nodes (4): SystemReset, reset_factory_pin_, reset_nvs_pin_, SystemReset::SystemReset()

### Community 148 - "uint32_t"
Cohesion: 0.20
Nodes (7): color_pre_multiply(), pack(), multiply(), RLEHeader, uint16_t(), uint24_t(), uint32_t()

### Community 149 - "stdlib"
Cohesion: 0.24
Nodes (4): aht20_deinit(), aht20_init(), aht20_read(), parse_extra_gpios()

### Community 155 - "build_all.py"
Cohesion: 0.22
Nodes (4): build_assets(), ensure_dir(), get_file_path(), main()

### Community 158 - "Entry"
Cohesion: 0.20
Nodes (9): Entry, adv_w, bitmap, box_h, box_w, codepoint, last_use, ofs_x (+1 more)

### Community 159 - "pack_model.py"
Cohesion: 0.29
Nodes (6): pack_models(), read_data(), struct_pack_string(), pack_models(), read_data(), struct_pack_string()

### Community 160 - "_resolve_board_config"
Cohesion: 0.22
Nodes (5): _extract_board_config_from_sdkconfig_append(), _find_board_config_candidates(), _resolve_board_config(), validate_target(), _symbol_supports_target()

### Community 163 - "guiController.js"
Cohesion: 0.36
Nodes (3): SdkconfigGenerator, ESP32S3, PinValidator

### Community 164 - "test_integration.js"
Cohesion: 0.20
Nodes (8): fs, kconfigProjPath, partitionPath, path, projectRoot, { SdkconfigParser, SdkconfigGenerator }, sdkDefaultsPath, sdkS3DefaultsPath

### Community 165 - "argparse"
Cohesion: 0.28
Nodes (3): generate_header(), get_sound_files(), load_base_language()

### Community 166 - "bmp280.c"
Cohesion: 0.50
Nodes (5): bmp280_deinit(), bmp280_init(), bmp280_read(), bmp280_read_regs(), bmp280_write_reg()

### Community 168 - "dht.c"
Cohesion: 0.42
Nodes (4): dht_init(), dht_read_data(), dht_read_raw(), wait_for_level()

### Community 169 - "hcsr04.c"
Cohesion: 0.33
Nodes (4): hcsr04_deinit(), hcsr04_init(), hcsr04_read_distance(), sensor_set_hcsr04_pins()

### Community 170 - "ina219.c"
Cohesion: 0.47
Nodes (5): ina219_deinit(), ina219_init(), ina219_read(), ina219_read_reg16(), ina219_write_reg16()

### Community 172 - "TextFontCapability"
Cohesion: 0.22
Nodes (6): TextFontCapability, bpp, bundle, charset, glyph_push, size

### Community 173 - "AudioServiceCallbacks"
Cohesion: 0.22
Nodes (7): AudioServiceCallbacks, on_audio_testing_queue_full, on_playback_drained, on_playback_progress, on_send_queue_available, on_vad_change, on_wake_word_detected

### Community 175 - "scd4x.c"
Cohesion: 0.44
Nodes (5): scd4x_deinit(), scd4x_init(), scd4x_read_measurement(), scd4x_send_cmd(), sensirion_crc8()

### Community 177 - "Path"
Cohesion: 0.22
Nodes (4): _board_config_symbol_exists(), _get_board_display_name(), _kconfig_config_board_dependencies(), _sync_vscode_target()

### Community 180 - "lcd_display.h"
Cohesion: 0.29
Nodes (3): MipiLcdDisplay, RgbLcdDisplay, SpiLcdDisplay

### Community 181 - "audio_init.c"
Cohesion: 0.43
Nodes (4): audio_driver_init_mic(), audio_driver_init_speaker(), audio_init(), audio_subsystem_init()

### Community 182 - "_sdkconfig_assignments"
Cohesion: 0.29
Nodes (4): _apply_auto_selects(), _board_supports_wake_word(), _merge_sdkconfig_options(), _sdkconfig_assignments()

### Community 185 - "bh1750.c"
Cohesion: 0.43
Nodes (3): bh1750_deinit(), bh1750_init(), bh1750_read_lux()

### Community 188 - "PowerSaveLevel"
Cohesion: 0.29
Nodes (4): PowerSaveLevel, BALANCED, LOW_POWER, PERFORMANCE

### Community 189 - "Nt26CeregState"
Cohesion: 0.33
Nodes (5): Nt26CeregState, AcT, ci, stat, tac

### Community 190 - "UiStyle"
Cohesion: 0.29
Nodes (6): UiStyle, ChatBubble, ClassicAvatar, CyberTerminal, MinimalZen, SmartDashboard

### Community 193 - "is_valid_xiaozhi_project"
Cohesion: 0.40
Nodes (4): detect_project_info(), is_valid_xiaozhi_project(), scan_all_xiaozhi_projects(), add_p()

### Community 194 - "ListeningMode"
Cohesion: 0.33
Nodes (4): ListeningMode, kListeningModeAutoStop, kListeningModeManualStop, kListeningModeRealtime

### Community 196 - "mmap_assets_table"
Cohesion: 0.33
Nodes (6): mmap_assets_table, asset_height, asset_name, asset_offset, asset_size, asset_width

### Community 197 - "DebugStatistics"
Cohesion: 0.33
Nodes (6): DebugStatistics, decode_count, encode_count, encode_drop_count, input_count, playback_count

### Community 199 - "clean_comments.py"
Cohesion: 0.60
Nodes (3): main(), process_file(), remove_c_comments()

### Community 200 - "AudioDebugger"
Cohesion: 0.40
Nodes (3): AudioDebugger, udp_server_addr_, udp_sockfd_

### Community 202 - "Command"
Cohesion: 0.40
Nodes (4): Command, action, command, text

### Community 204 - "DisplayLockGuard"
Cohesion: 0.40
Nodes (3): DisplayLockGuard, display_, locked_

### Community 206 - "EffectMode"
Cohesion: 0.40
Nodes (5): EffectMode, kBlink, kBreathe, kNone, kRainbow

### Community 212 - "WakeDetector"
Cohesion: 0.50
Nodes (4): WakeDetector, kMultiNet, kNone, kWakeNet

### Community 214 - "AbortReason"
Cohesion: 0.50
Nodes (3): AbortReason, kAbortReasonNone, kAbortReasonWakeWordDetected

## Knowledge Gaps
- **784 isolated node(s):** `LvglTheme`, `LvglFont`, `AudioCodec`, `Display`, `DualDisplay` (+779 more)
  These have ≤1 connection - possible missing edges. (Counts symbols only; 2013 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **75 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Application` connect `Application` to `McpServer`, `CircularStrip`, `AudioService`, `GpioLed`, `CustomN16R8Board`, `NotifyPlayer`, `LvglDisplay`, `.StartMqttClient`, `Nt26Board`, `string`, `DualNetworkBoard`, `Protocol`, `wifi_board.cc`, `.Schedule`, `.GetDeviceState`, `Ota`, `SleepTimer`, `app_main.c`, `ListeningMode`, `DeviceStateMachine`, `PowerSaveTimer`, `system_info.cc`, `application.cc`, `single_led.cc`, `mqtt_protocol.cc`, `Assets`?**
  _High betweenness centrality (0.075) - this node is a cross-community bridge._
- **Why does `Board` connect `Board` to `McpServer`, `.SetupHttp`, `display.cc`, `LvglDisplay`, `NotifyPlayer`, `.StartMqttClient`, `ota.cc`, `jpeg_to_image.c`, `emote_display.cc`, `ethernet_board.cc`, `Nt26Board`, `string`, `Application`, `Camera`, `AudioCodec`, `RndisBoard`, `DualNetworkBoard`, `Ml307Board`, `wifi_board.cc`, `board.h`, `.Schedule`, `EspVideo`, `.GetDeviceState`, `SleepTimer`, `websocket_protocol.cc`, `.Download`, `audio_service.cc`, `PowerSaveTimer`, `Settings`, `system_info.cc`, `application.cc`, `esp_log`, `Backlight`, `Esp32Camera`, `lcd_display.cc`, `assets.cc`, `mqtt_protocol.cc`, `Assets`?**
  _High betweenness centrality (0.050) - this node is a cross-community bridge._
- **Why does `AudioService` connect `AudioService` to `audio_service.h`, `FixedQueue`, `DebugStatistics`, `audio_service.cc`, `AudioDebugger`, `NotifyPlayer`, `AudioEngine`, `.PushTaskToEncodeQueue`, `AudioServiceCallbacks`, `string`, `Application`, `AudioStreamPacket`?**
  _High betweenness centrality (0.042) - this node is a cross-community bridge._
- **What connects `LvglTheme`, `LvglFont`, `AudioCodec` to the rest of the system?**
  _784 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `McpServer` be split into smaller, more focused modules?**
  _Cohesion score 0.05063291139240506 - nodes in this community are weakly interconnected._
- **Should `gifdec.c` be split into smaller, more focused modules?**
  _Cohesion score 0.07199297629499561 - nodes in this community are weakly interconnected._
- **Should `CircularStrip` be split into smaller, more focused modules?**
  _Cohesion score 0.05336951605608322 - nodes in this community are weakly interconnected._