set(SOURCES "audio/audio_codec.cc"
            "audio/soft_aec_reference.cc"
            "audio/audio_debugger.cc"
            "audio/audio_service.cc"
            "audio/sound_theme.cc"
            "audio/demuxer/ogg_demuxer.cc"
            "notify/notify_player.cc"
            "audio/codecs/no_audio_codec.cc"
            "audio/codecs/box_audio_codec.cc"
            "audio/codecs/es8311_audio_codec.cc"
            "audio/codecs/es8374_audio_codec.cc"
            "audio/codecs/es8388_audio_codec.cc"
            "audio/codecs/es8389_audio_codec.cc"
            "led/single_led.cc"
            "led/circular_strip.cc"
            "led/gpio_led.cc"
            "display/display.cc"
            "display/uart_display.cc"
            "display/text_glyph.cc"
            "display/lcd_display.cc"
            "display/adaptive_ui.cc"
            "display/oled_display.cc"
            "display/lvgl_display/lvgl_display.cc"
            "display/emote_display.cc"
            "display/lvgl_display/emoji_collection.cc"
            "display/lvgl_display/lvgl_theme.cc"
            "display/lvgl_display/lvgl_font.cc"
            "display/lvgl_display/dynamic_glyph_cache.cc"
            "display/lvgl_display/lvgl_image.cc"
            "display/lvgl_display/gif/lvgl_gif.cc"
            "display/lvgl_display/gif/gifdec.c"
            "display/lvgl_display/jpg/image_to_jpeg.cpp"
            "display/lvgl_display/jpg/jpeg_to_image.c"
            "protocols/protocol.cc"
            "protocols/text_glyph_payload.cc"
            "protocols/mqtt_protocol.cc"
            "protocols/websocket_protocol.cc"
            "mcp_server.cc"
            "system_info.cc"
            "application.cc"
            "ota.cc"
            "settings.cc"
            "device_state_machine.cc"
            "assets.cc"
            "main.cc"
            )

set(INCLUDE_DIRS "." "display" "display/lvgl_display" "display/lvgl_display/jpg" "audio" "audio/demuxer" "protocols")

list(APPEND SOURCES
    "boards/common/board.cc"
    "boards/common/wifi_board.cc"
    "boards/common/adc_battery_monitor.cc"
    "boards/common/axp2101.cc"
    "boards/common/backlight.cc"
    "boards/common/button.cc"
    "boards/common/i2c_device.cc"
    "boards/common/knob.cc"
    "boards/common/power_save_timer.cc"
    "boards/common/press_to_talk_mcp_tool.cc"
    "boards/common/sleep_timer.cc"
    "boards/common/sy6970.cc"
    "boards/common/system_reset.cc"
    "boards/common/sensor_controller.cc"
    "boards/common/actuator_controller.cc"
    "boards/common/periph_controller.cc"
    "boards/common/bt_uart_bridge.cc"
)
list(APPEND INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/boards/common)

list(APPEND SOURCES
    "app_main.c"
    "gpio_validator.c"
    "boards/common/bus_manager.c"
    "boards/board_init.c"
    "drivers/input/input_manager.c"
    "drivers/input/input_init.c"
    "drivers/input/rotary_encoder.c"
    "drivers/input/touch_slider.c"
    "drivers/display/display_init.c"
    "drivers/audio/audio_init.c"
    "drivers/sensor/sensor_manager.c"
    "drivers/sensor/sensor_init.c"
    "drivers/sensor/vl6180x.c"
    "drivers/sensor/dht.c"
    "drivers/sensor/mq_sensor.c"
    "drivers/sensor/scd4x.c"
    "drivers/sensor/bmp280.c"
    "drivers/sensor/aht20.c"
    "drivers/sensor/bh1750.c"
    "drivers/sensor/hcsr04.c"
    "drivers/sensor/ina219.c"
    "drivers/sensor/vl53lx.c"
    "drivers/sensor/apds9960.c"
    "drivers/sensor/bq27220.c"
    "drivers/periph/rtc_ext.c"
    "drivers/periph/pn532.c"
    "drivers/periph/pca9685.c"
    "drivers/periph/led_ic.c"
    "drivers/periph/rc522.c"
    "drivers/periph/qmi8658.c"
    "drivers/periph/sy6970.c"
    "drivers/periph/periph_i2c.c"
    "drivers/periph/w5500_eth.c"
    "drivers/periph/bt_uart_module.c"
    "drivers/actuator/actuator_manager.c"
    "drivers/actuator/actuator_init.c"
    "drivers/storage/storage_manager.c"
    "drivers/storage/storage_init.c"
    "utils/config_utils.c"
)
list(APPEND INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/input
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/display
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/sensor
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/actuator
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/storage
    ${CMAKE_CURRENT_SOURCE_DIR}/boards
    ${CMAKE_CURRENT_SOURCE_DIR}/boards/common
)

idf_build_get_property(build_components BUILD_COMPONENTS)
function(find_component_by_pattern PATTERN COMPONENT_VAR PATH_VAR)
    foreach(COMPONENT ${build_components})
        if(COMPONENT MATCHES "${PATTERN}")
            set(${COMPONENT_VAR} ${COMPONENT} PARENT_SCOPE)
            idf_component_get_property(COMPONENT_PATH ${COMPONENT} COMPONENT_DIR)
            set(${PATH_VAR} "${COMPONENT_PATH}" PARENT_SCOPE)
            break()
        endif()
    endforeach()
endfunction()

if(CONFIG_BOARD_TYPE_ESP32_S3_N16R8_CUSTOM)
    set(BOARD_DIR "esp32s3-n16r8-custom")
elseif(CONFIG_BOARD_TYPE_WAVESHARE_ESP32_P4_DEV_KIT)
    set(BOARD_DIR "esp32-p4")
elseif(CONFIG_BOARD_TYPE_ESP32_P4_FUNCTION_EV_BOARD)
    set(BOARD_DIR "esp32-p4")
elseif(CONFIG_BOARD_TYPE_GUITION_ESP32_P4_M3_DEV)
    set(BOARD_DIR "esp32-p4")
else()
    set(BOARD_DIR "esp32s3-n16r8-custom")
endif()

set(BUILTIN_TEXT_FONT font_noto_sans_basic_20_4)
set(BUILTIN_ICON_FONT font_material_symbols_20_4)
set(DEFAULT_EMOJI_COLLECTION noto-color-emoji_64)
set(EMOTE_RESOLUTION "320_240")

if(NOT BOARD_DIR AND NOT CMAKE_BUILD_EARLY_EXPANSION)
    message(FATAL_ERROR "The selected board does not define BOARD_DIR")
endif()

set(BOARD_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD_DIR}/config.json")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${BOARD_CONFIG_FILE}")

set(BOARD_TYPE "")
set(BOARD_MANUFACTURER "")
set(BOARD_TARGET "")
if(EXISTS "${BOARD_CONFIG_FILE}")
    file(READ "${BOARD_CONFIG_FILE}" BOARD_CONFIG_JSON)
    string(REGEX MATCH "\"type\"[ \t\r\n]*:[ \t\r\n]*\"([^\"]+)\"" BOARD_TYPE_MATCH "${BOARD_CONFIG_JSON}")
    if(BOARD_TYPE_MATCH)
        set(BOARD_TYPE "${CMAKE_MATCH_1}")
    endif()
    string(REGEX MATCH "\"manufacturer\"[ \t\r\n]*:[ \t\r\n]*\"([^\"]+)\"" BOARD_MANUFACTURER_MATCH "${BOARD_CONFIG_JSON}")
    if(BOARD_MANUFACTURER_MATCH)
        set(BOARD_MANUFACTURER "${CMAKE_MATCH_1}")
    endif()
    string(REGEX MATCH "\"target\"[ \t\r\n]*:[ \t\r\n]*\"([^\"]+)\"" BOARD_TARGET_MATCH "${BOARD_CONFIG_JSON}")
    if(BOARD_TARGET_MATCH)
        set(BOARD_TARGET "${CMAKE_MATCH_1}")
    endif()
endif()
if(NOT CMAKE_BUILD_EARLY_EXPANSION AND BOARD_TARGET)
    idf_build_get_property(BUILD_IDF_TARGET IDF_TARGET)
    if(NOT BOARD_TARGET STREQUAL BUILD_IDF_TARGET)
        message(FATAL_ERROR
            "Board '${BOARD_DIR}' needs target ${BOARD_TARGET}, but this build uses ${BUILD_IDF_TARGET}. "
            "Pick a ${BUILD_IDF_TARGET} board in menuconfig (Target Board), or switch target: "
            "ESP32-P4 -> 03-build_esp32p4.bat (build_p4), ESP32-S3 -> 02-build_esp32s3.bat."
        )
    endif()
endif()
if(NOT BOARD_TYPE)
    string(REPLACE "/" "-" BOARD_TYPE "${BOARD_DIR}")
endif()
if(NOT BOARD_MANUFACTURER AND BOARD_DIR MATCHES "^([^/]+)/")
    set(BOARD_MANUFACTURER "${CMAKE_MATCH_1}")
endif()
if(NOT CMAKE_BUILD_EARLY_EXPANSION AND NOT BOARD_TYPE MATCHES "^[a-z0-9.-]+$")
    message(FATAL_ERROR
        "Invalid BOARD_TYPE '${BOARD_TYPE}': only lowercase letters, digits, '.' and '-' are allowed"
    )
endif()

file(GLOB BOARD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD_DIR}/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD_DIR}/*.c
)
list(APPEND SOURCES ${BOARD_SOURCES})

list(APPEND SOURCES "audio/engines/afe_audio_engine.cc")
list(APPEND SOURCES "audio/wake_words/custom_wake_word.cc")
list(APPEND SOURCES "audio/wake_words/wake_word_audio_cache.cc")

if (CONFIG_USE_ESP_BLUFI_WIFI_PROVISIONING)
    list(APPEND SOURCES "boards/common/blufi.cpp")
endif ()
if(CONFIG_LANGUAGE_ZH_CN)
    set(LANG_DIR "zh-CN")
elseif(CONFIG_LANGUAGE_ZH_TW)
    set(LANG_DIR "zh-TW")
elseif(CONFIG_LANGUAGE_EN_US)
    set(LANG_DIR "en-US")
elseif(CONFIG_LANGUAGE_JA_JP)
    set(LANG_DIR "ja-JP")
elseif(CONFIG_LANGUAGE_KO_KR)
    set(LANG_DIR "ko-KR")
elseif(CONFIG_LANGUAGE_VI_VN)
    set(LANG_DIR "vi-VN")
elseif(CONFIG_LANGUAGE_TH_TH)
    set(LANG_DIR "th-TH")
elseif(CONFIG_LANGUAGE_DE_DE)
    set(LANG_DIR "de-DE")
elseif(CONFIG_LANGUAGE_FR_FR)
    set(LANG_DIR "fr-FR")
elseif(CONFIG_LANGUAGE_ES_ES)
    set(LANG_DIR "es-ES")
elseif(CONFIG_LANGUAGE_IT_IT)
    set(LANG_DIR "it-IT")
elseif(CONFIG_LANGUAGE_RU_RU)
    set(LANG_DIR "ru-RU")
elseif(CONFIG_LANGUAGE_AR_SA)
    set(LANG_DIR "ar-SA")
elseif(CONFIG_LANGUAGE_HI_IN)
    set(LANG_DIR "hi-IN")
elseif(CONFIG_LANGUAGE_MR_IN)
    set(LANG_DIR "mr-IN")
elseif(CONFIG_LANGUAGE_PT_PT)
    set(LANG_DIR "pt-PT")
elseif(CONFIG_LANGUAGE_PT_BR)
    set(LANG_DIR "pt-BR")
elseif(CONFIG_LANGUAGE_PL_PL)
    set(LANG_DIR "pl-PL")
elseif(CONFIG_LANGUAGE_CS_CZ)
    set(LANG_DIR "cs-CZ")
elseif(CONFIG_LANGUAGE_FI_FI)
    set(LANG_DIR "fi-FI")
elseif(CONFIG_LANGUAGE_TR_TR)
    set(LANG_DIR "tr-TR")
elseif(CONFIG_LANGUAGE_ID_ID)
    set(LANG_DIR "id-ID")
elseif(CONFIG_LANGUAGE_UK_UA)
    set(LANG_DIR "uk-UA")
elseif(CONFIG_LANGUAGE_RO_RO)
    set(LANG_DIR "ro-RO")
elseif(CONFIG_LANGUAGE_BG_BG)
    set(LANG_DIR "bg-BG")
elseif(CONFIG_LANGUAGE_CA_ES)
    set(LANG_DIR "ca-ES")
elseif(CONFIG_LANGUAGE_DA_DK)
    set(LANG_DIR "da-DK")
elseif(CONFIG_LANGUAGE_EL_GR)
    set(LANG_DIR "el-GR")
elseif(CONFIG_LANGUAGE_FA_IR)
    set(LANG_DIR "fa-IR")
elseif(CONFIG_LANGUAGE_FIL_PH)
    set(LANG_DIR "fil-PH")
elseif(CONFIG_LANGUAGE_HE_IL)
    set(LANG_DIR "he-IL")
elseif(CONFIG_LANGUAGE_HR_HR)
    set(LANG_DIR "hr-HR")
elseif(CONFIG_LANGUAGE_HU_HU)
    set(LANG_DIR "hu-HU")
elseif(CONFIG_LANGUAGE_MS_MY)
    set(LANG_DIR "ms-MY")
elseif(CONFIG_LANGUAGE_NB_NO)
    set(LANG_DIR "nb-NO")
elseif(CONFIG_LANGUAGE_NL_NL)
    set(LANG_DIR "nl-NL")
elseif(CONFIG_LANGUAGE_SK_SK)
    set(LANG_DIR "sk-SK")
elseif(CONFIG_LANGUAGE_SL_SI)
    set(LANG_DIR "sl-SI")
elseif(CONFIG_LANGUAGE_SV_SE)
    set(LANG_DIR "sv-SE")
elseif(CONFIG_LANGUAGE_SR_RS)
    set(LANG_DIR "sr-RS")
endif()

set(LANG_JSON "${CMAKE_CURRENT_SOURCE_DIR}/assets/locales/${LANG_DIR}/language.json")
set(LANG_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/assets/lang_config.h")

file(GLOB LANG_SOUNDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/locales/${LANG_DIR}/*.ogg)

if(NOT LANG_DIR STREQUAL "en-US")
    file(GLOB EN_US_SOUNDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/locales/en-US/*.ogg)

    set(EXISTING_NAMES "")
    foreach(SOUND_FILE ${LANG_SOUNDS})
        get_filename_component(FILENAME ${SOUND_FILE} NAME)
        list(APPEND EXISTING_NAMES ${FILENAME})
    endforeach()

    foreach(EN_SOUND ${EN_US_SOUNDS})
        get_filename_component(FILENAME ${EN_SOUND} NAME)
        if(NOT ${FILENAME} IN_LIST EXISTING_NAMES)
            list(APPEND LANG_SOUNDS ${EN_SOUND})
            message(STATUS "Using en-US fallback for missing audio: ${FILENAME}")
        endif()
    endforeach()
endif()

file(GLOB COMMON_SOUNDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/common/*.ogg)

include(${CMAKE_CURRENT_LIST_DIR}/sound_theme.cmake)

list(APPEND SOURCES "boards/common/esp_video.cc")

if(CONFIG_IDF_TARGET_ESP32S3)
    list(APPEND SOURCES "boards/common/esp32_camera.cc")
endif()

set(MAIN_PRIV_REQUIRES_EXTRA "")
idf_component_register(SRCS ${SOURCES}
                    EMBED_FILES ${LANG_SOUNDS} ${COMMON_SOUNDS} ${SOUND_THEME_EMBED_FILES}
                    INCLUDE_DIRS ${INCLUDE_DIRS}
                    WHOLE_ARCHIVE
                    PRIV_REQUIRES
                        esp_pm
                        esp_psram
                        esp_netif
                        esp_eth
                        esp_driver_gpio
                        esp_driver_uart
                        esp_driver_spi
                        esp_driver_i2c
                        esp_driver_i2s
                        esp_driver_ledc
                        esp_driver_rmt
                        esp_driver_jpeg
                        esp_driver_ppa
                        esp_driver_tsens
                        esp_driver_touch_sens
                        esp_app_format
                        app_update
                        spi_flash
                        mbedtls
                        console
                        efuse
                        bt
                        fatfs
                        xiaozhi-fonts
                        ${MAIN_PRIV_REQUIRES_EXTRA}
                    )

if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    target_compile_options(${COMPONENT_LIB} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++23>)
    target_include_directories(${COMPONENT_LIB} PRIVATE ${SOUND_THEME_DIR})
endif()

if(NOT BOARD_NAME)
    set(BOARD_NAME ${BOARD_TYPE})
endif()
if(NOT CMAKE_BUILD_EARLY_EXPANSION AND NOT BOARD_NAME MATCHES "^[a-z0-9.-]+$")
    message(FATAL_ERROR
        "Invalid BOARD_NAME '${BOARD_NAME}': only lowercase letters, digits, '.' and '-' are allowed"
    )
endif()
string(REGEX MATCH "^font_noto_sans_basic_([0-9]+)_([0-9]+)$" _font_match "${BUILTIN_TEXT_FONT}")
if(NOT _font_match)
    message(FATAL_ERROR "Unsupported built-in text font name: ${BUILTIN_TEXT_FONT}")
endif()
set(TEXT_FONT_SIZE "${CMAKE_MATCH_1}")
set(TEXT_FONT_BPP "${CMAKE_MATCH_2}")
if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    target_compile_definitions(${COMPONENT_LIB}
                        PRIVATE BOARD_TYPE=\"${BOARD_TYPE}\" BOARD_NAME=\"${BOARD_NAME}\"
                        PRIVATE BOARD_MANUFACTURER=\"${BOARD_MANUFACTURER}\"
                        PRIVATE BUILTIN_TEXT_FONT=${BUILTIN_TEXT_FONT} BUILTIN_ICON_FONT=${BUILTIN_ICON_FONT}
                        PRIVATE TEXT_FONT_SIZE=${TEXT_FONT_SIZE} TEXT_FONT_BPP=${TEXT_FONT_BPP}
                        )
    if(CONFIG_IDF_TARGET_ESP32S3)
        idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
        if(lvgl_lib)
            target_include_directories(${lvgl_lib} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/display")
            if(CONFIG_SPIRAM)
                target_compile_definitions(${lvgl_lib} PRIVATE
                    "LV_MEM_POOL_INCLUDE=\"lvgl_psram_pool.h\"")
            elseif(CONFIG_LV_USE_BUILTIN_MALLOC)
                target_compile_definitions(${lvgl_lib} PRIVATE
                    "LV_MEM_POOL_INCLUDE=\"lvgl_no_psram_pool.h\"")
            endif()
        endif()
    endif()
endif()

add_custom_command(
    OUTPUT ${LANG_HEADER}
    COMMAND python -X utf8 ${PROJECT_DIR}/scripts/gen_lang.py
            --language "${LANG_DIR}"
            --output "${LANG_HEADER}"
    DEPENDS
        ${LANG_JSON}
        ${PROJECT_DIR}/scripts/gen_lang.py
    COMMENT "Generating ${LANG_DIR} language config"
)

add_custom_target(lang_header ALL
    DEPENDS ${LANG_HEADER}
)

find_component_by_pattern("espressif__esp-sr" ESP_SR_COMPONENT ESP_SR_COMPONENT_PATH)
if(ESP_SR_COMPONENT_PATH)
    set(ESP_SR_MODEL_PATH "${ESP_SR_COMPONENT_PATH}/model")
endif()

find_component_by_pattern("xiaozhi-fonts" NOTO_FONTS_COMPONENT NOTO_FONTS_COMPONENT_PATH)
if(NOTO_FONTS_COMPONENT_PATH)
    set(NOTO_FONTS_PATH "${NOTO_FONTS_COMPONENT_PATH}")
endif()

function(build_default_assets_bin)
    set(GENERATED_ASSETS_BIN "${CMAKE_BINARY_DIR}/generated_assets.bin")
    set(DEFAULT_ASSETS_DEPENDENCIES
        "${SDKCONFIG}"
        "${PROJECT_DIR}/scripts/build_default_assets.py"
    )

    set(BUILD_ARGS
        "--sdkconfig" "${SDKCONFIG}"
        "--output" "${GENERATED_ASSETS_BIN}"
    )

    if(BUILTIN_TEXT_FONT)
        list(APPEND BUILD_ARGS "--builtin_text_font" "${BUILTIN_TEXT_FONT}")
        string(REPLACE "_basic_" "_common_" COMMON_TEXT_FONT "${BUILTIN_TEXT_FONT}")
        set(COMMON_TEXT_FONT_FILE "${NOTO_FONTS_PATH}/cbin/${COMMON_TEXT_FONT}.bin")
        if(EXISTS "${COMMON_TEXT_FONT_FILE}")
            list(APPEND DEFAULT_ASSETS_DEPENDENCIES "${COMMON_TEXT_FONT_FILE}")
        endif()
        if(EXISTS "${NOTO_FONTS_PATH}/manifest.json")
            list(APPEND DEFAULT_ASSETS_DEPENDENCIES "${NOTO_FONTS_PATH}/manifest.json")
        endif()
    endif()

    if(DEFAULT_EMOJI_COLLECTION)
        list(APPEND BUILD_ARGS "--emoji_collection" "${DEFAULT_EMOJI_COLLECTION}")
        file(GLOB DEFAULT_EMOJI_FILES CONFIGURE_DEPENDS
            "${NOTO_FONTS_PATH}/png/${DEFAULT_EMOJI_COLLECTION}/*"
        )
        list(APPEND DEFAULT_ASSETS_DEPENDENCIES ${DEFAULT_EMOJI_FILES})
    endif()

    if(DEFAULT_ASSETS_EXTRA_FILES)
        list(APPEND BUILD_ARGS "--extra_files" "${DEFAULT_ASSETS_EXTRA_FILES}")
    endif()

    list(APPEND BUILD_ARGS "--esp_sr_model_path" "${ESP_SR_MODEL_PATH}")
    list(APPEND BUILD_ARGS "--noto_fonts_path" "${NOTO_FONTS_PATH}")

    partition_table_get_partition_info(assets_partition_size "--partition-name model" "size")
    if(assets_partition_size)
        list(APPEND BUILD_ARGS "--max_size" "${assets_partition_size}")
    endif()

    add_custom_command(
        OUTPUT ${GENERATED_ASSETS_BIN}
        COMMAND python -X utf8 ${PROJECT_DIR}/scripts/build_default_assets.py ${BUILD_ARGS}
        DEPENDS ${DEFAULT_ASSETS_DEPENDENCIES}
        COMMENT "Building default assets.bin based on configuration"
        VERBATIM
    )

    add_custom_target(generated_default_assets ALL
        DEPENDS ${GENERATED_ASSETS_BIN}
    )

    set(GENERATED_ASSETS_LOCAL_FILE ${GENERATED_ASSETS_BIN} PARENT_SCOPE)

    message(STATUS "Default assets build configured: ${GENERATED_ASSETS_BIN}")
endfunction()

function(get_assets_local_file assets_source assets_local_file_var)
    if(assets_source MATCHES "^https?://")
        get_filename_component(ASSETS_FILENAME "${assets_source}" NAME)
        set(ASSETS_LOCAL_FILE "${CMAKE_BINARY_DIR}/${ASSETS_FILENAME}")
        set(ASSETS_TEMP_FILE "${CMAKE_BINARY_DIR}/${ASSETS_FILENAME}.tmp")

        if(EXISTS ${ASSETS_LOCAL_FILE})
            message(STATUS "Assets file ${ASSETS_FILENAME} already exists, skipping download")
        else()
            message(STATUS "Downloading ${ASSETS_FILENAME}")

            if(EXISTS ${ASSETS_TEMP_FILE})
                file(REMOVE ${ASSETS_TEMP_FILE})
            endif()

            file(DOWNLOAD ${assets_source} ${ASSETS_TEMP_FILE}
                 STATUS DOWNLOAD_STATUS)
            list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
            if(NOT STATUS_CODE EQUAL 0)
                if(EXISTS ${ASSETS_TEMP_FILE})
                    file(REMOVE ${ASSETS_TEMP_FILE})
                endif()
                message(FATAL_ERROR "Failed to download ${ASSETS_FILENAME} from ${assets_source}")
            endif()

            file(RENAME ${ASSETS_TEMP_FILE} ${ASSETS_LOCAL_FILE})
            message(STATUS "Successfully downloaded ${ASSETS_FILENAME}")
        endif()
    else()
        if(IS_ABSOLUTE "${assets_source}")
            set(ASSETS_LOCAL_FILE "${assets_source}")
        else()
            set(ASSETS_LOCAL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${assets_source}")
        endif()

        if(NOT EXISTS ${ASSETS_LOCAL_FILE})
            message(FATAL_ERROR "Assets file not found: ${ASSETS_LOCAL_FILE}")
        endif()

        message(STATUS "Using assets file: ${ASSETS_LOCAL_FILE}")
    endif()

    set(${assets_local_file_var} ${ASSETS_LOCAL_FILE} PARENT_SCOPE)
endfunction()

partition_table_get_partition_info(size "--partition-name model" "size")
partition_table_get_partition_info(offset "--partition-name model" "offset")
set(TARGET_ASSETS_PART "model")

if ("${size}" AND "${offset}")
    if(CONFIG_FLASH_DEFAULT_ASSETS)
        build_default_assets_bin()
        esptool_py_flash_to_partition(flash "${TARGET_ASSETS_PART}" "${GENERATED_ASSETS_LOCAL_FILE}")
        message(STATUS "Generated default assets flash configured: ${GENERATED_ASSETS_LOCAL_FILE} -> ${TARGET_ASSETS_PART} partition")
    elseif(CONFIG_FLASH_CUSTOM_ASSETS)
        get_assets_local_file("${CONFIG_CUSTOM_ASSETS_FILE}" ASSETS_LOCAL_FILE)
        esptool_py_flash_to_partition(flash "${TARGET_ASSETS_PART}" "${ASSETS_LOCAL_FILE}")
        message(STATUS "Custom assets flash configured: ${ASSETS_LOCAL_FILE} -> ${TARGET_ASSETS_PART} partition")
    elseif(CONFIG_FLASH_EXPRESSION_ASSETS)
        set(ASSETS_NAME "expression_assets")
        set(ASSETS_PARTITION "${TARGET_ASSETS_PART}")
        set(ASSETS_FILE "${CMAKE_BINARY_DIR}/${ASSETS_NAME}.bin")

        if(EMOTE_EXTERNAL_PATH)
            build_speaker_assets_bin("${ASSETS_PARTITION}" ${EMOTE_RESOLUTION} ${ASSETS_FILE} ${CONFIG_MMAP_FILE_NAME_LENGTH} "${EMOTE_EXTERNAL_PATH}")
        else()
            build_speaker_assets_bin("${ASSETS_PARTITION}" ${EMOTE_RESOLUTION} ${ASSETS_FILE} ${CONFIG_MMAP_FILE_NAME_LENGTH})
        endif()
        message(STATUS "Generated emote assets: ${ASSETS_FILE} -> ${ASSETS_PARTITION} partition")
        esptool_py_flash_to_partition(flash "${ASSETS_PARTITION}" "${ASSETS_FILE}")
    elseif(CONFIG_FLASH_NONE_ASSETS)
        message(STATUS "Assets flashing disabled (FLASH_NONE_ASSETS)")
    endif()
else()
    message(STATUS "Assets partition not found, using v1 partition table")
endif()
