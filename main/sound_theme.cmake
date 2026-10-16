set(SOUND_THEME_SLOTS
    "STARTUP:SUCCESS"
    "LISTENING:POPUP"
    "ERROR:EXCLAMATION"
    "ALERT:VIBRATION"
    "LOW_BATTERY:LOW_BATTERY"
    "WIFI_CONFIG:WIFICONFIG"
    "ACTIVATION:ACTIVATION"
    "UPGRADE:UPGRADE"
)
set(SOUND_THEME_TONES POPUP SUCCESS EXCLAMATION VIBRATION LOW_BATTERY)
set(SOUND_THEME_MAX_BYTES 1048576)
set(SOUND_THEME_DIR "${CMAKE_CURRENT_BINARY_DIR}/sound_theme")
set(SOUND_THEME_EMBED_FILES "")

function(sound_theme_prepare_custom slot slot_lower out_file)
    set(source_path "${CONFIG_SOUND_${slot}_CUSTOM_FILE}")
    if(source_path STREQUAL "")
        message(FATAL_ERROR
            "SOUND_${slot}: da chon 'Tep .ogg tuy chinh' nhung chua nhap duong dan (SOUND_${slot}_CUSTOM_FILE)")
    endif()
    file(TO_CMAKE_PATH "${source_path}" source_path)
    if(NOT IS_ABSOLUTE "${source_path}")
        set(source_path "${PROJECT_DIR}/${source_path}")
    endif()
    if(NOT EXISTS "${source_path}" OR IS_DIRECTORY "${source_path}")
        message(FATAL_ERROR "SOUND_${slot}: khong tim thay tep am thanh '${source_path}'")
    endif()
    file(SIZE "${source_path}" source_size)
    if(source_size GREATER SOUND_THEME_MAX_BYTES)
        message(FATAL_ERROR
            "SOUND_${slot}: tep '${source_path}' nang ${source_size} byte, vuot gioi han ${SOUND_THEME_MAX_BYTES} byte")
    endif()
    file(READ "${source_path}" source_head LIMIT 64 HEX)
    string(SUBSTRING "${source_head}" 0 8 source_magic)
    string(FIND "${source_head}" "4f707573486561" opus_head_pos)
    if(NOT source_magic STREQUAL "4f676753" OR opus_head_pos EQUAL -1)
        message(FATAL_ERROR
            "SOUND_${slot}: '${source_path}' khong phai Ogg Opus. Doi bang scripts/ogg_converter/xiaozhi_ogg_converter.py "
            "hoac: ffmpeg -i vao.mp3 -c:a libopus -b:a 16k -ac 1 -ar 16000 -frame_duration 60 ra.ogg")
    endif()
    set(embedded_path "${SOUND_THEME_DIR}/sound_theme_${slot_lower}.ogg")
    configure_file("${source_path}" "${embedded_path}" COPYONLY)
    message(STATUS "Notification sound ${slot}: ${source_path} (${source_size} bytes)")
    set(${out_file} "${embedded_path}" PARENT_SCOPE)
endfunction()

set(SOUND_THEME_HEADER "#pragma once\n\n#include <cstddef>\n#include <string_view>\n\n")
foreach(slot_entry ${SOUND_THEME_SLOTS})
    string(REPLACE ":" ";" slot_parts "${slot_entry}")
    list(GET slot_parts 0 slot)
    list(GET slot_parts 1 default_sound)
    string(TOLOWER "${slot}" slot_lower)
    set(sound_expr "Lang::Sounds::OGG_${default_sound}")
    if(CONFIG_SOUND_${slot}_NONE)
        set(sound_expr "std::string_view{}")
    elseif(CONFIG_SOUND_${slot}_CUSTOM AND NOT CMAKE_BUILD_EARLY_EXPANSION)
        sound_theme_prepare_custom(${slot} ${slot_lower} embedded_file)
        list(APPEND SOUND_THEME_EMBED_FILES "${embedded_file}")
        string(APPEND SOUND_THEME_HEADER
            "extern const char sound_theme_${slot_lower}_start[] asm(\"_binary_sound_theme_${slot_lower}_ogg_start\");\n"
            "extern const char sound_theme_${slot_lower}_end[] asm(\"_binary_sound_theme_${slot_lower}_ogg_end\");\n")
        set(sound_expr
            "std::string_view(sound_theme_${slot_lower}_start, static_cast<std::size_t>(sound_theme_${slot_lower}_end - sound_theme_${slot_lower}_start))")
    else()
        foreach(tone ${SOUND_THEME_TONES})
            if(CONFIG_SOUND_${slot}_${tone})
                set(sound_expr "Lang::Sounds::OGG_${tone}")
            endif()
        endforeach()
    endif()
    string(APPEND SOUND_THEME_HEADER "#define SOUND_THEME_${slot} ${sound_expr}\n")
endforeach()

if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    file(CONFIGURE OUTPUT "${SOUND_THEME_DIR}/sound_theme_map.h" CONTENT "${SOUND_THEME_HEADER}" @ONLY)
endif()
