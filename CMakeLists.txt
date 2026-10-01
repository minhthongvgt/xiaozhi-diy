# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# Keep configure-time Python tools on UTF-8; build_windows.bat also exports these for Ninja.
set(ENV{PYTHONUTF8} "1")
set(ENV{PYTHONIOENCODING} "utf-8")

# Add this line to disable the specific warning
add_compile_options(-Wno-missing-field-initializers)

# Ensure standard defaults are always loaded when configuring with idf.py set-target or fresh build
if(NOT DEFINED SDKCONFIG_DEFAULTS)
    set(SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults")
    if(DEFINED IDF_TARGET AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults.${IDF_TARGET}")
        list(APPEND SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults.${IDF_TARGET}")
    elseif(DEFINED ENV{IDF_TARGET} AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults.$ENV{IDF_TARGET}")
        list(APPEND SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults.$ENV{IDF_TARGET}")
    endif()
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
idf_build_set_property(MINIMAL_BUILD ON)

# -----------------------------------------------------------------------------
# Khắc phục triệt để lỗi xung đột 2 tệp ghi đè vào phân vùng Flash 'model' (0x920000)
# -----------------------------------------------------------------------------
# Lý do:
# Component 'esp-sr' mặc định cố gắng tạo target 'srmodels_bin' và nạp 'srmodels.bin'
# vào phân vùng 'model' (0x920000).
# Trong khi đó, dự án Xiaozhi đã tự động đóng gói toàn bộ srmodels.bin cùng font chữ
# và emoji vào tệp duy nhất 'generated_assets.bin' (thông qua scripts/build_default_assets.py)
# và nạp vào chính phân vùng 'model'.
# Việc can thiệp vào managed_components/ sẽ bị ESP Component Manager khôi phục lại khi chạy.
# Do đó, giải pháp tận gốc từ mã nguồn dự án là sử dụng cơ chế CMake Override 2 tầng:

# Tầng 1: Chặn esp-sr phát hiện phân vùng 'model' khi truy vấn thông tin phân vùng
# Giúp esp-sr bỏ qua hoàn toàn việc tạo target 'srmodels_bin' và không chạy movemodel.py.
function(partition_table_get_partition_info result get_part_info_args part_info)
    if((CMAKE_CURRENT_LIST_DIR MATCHES "esp-sr" OR COMPONENT_NAME MATCHES "esp-sr") AND get_part_info_args MATCHES "model")
        message(STATUS "[Xiaozhi] Bo qua tao target srmodels_bin cua esp-sr (da duoc dong goi trong generated_assets.bin)")
        set(${result} "" PARENT_SCOPE)
        return()
    endif()

    idf_build_get_property(python PYTHON)
    idf_build_get_property(idf_path IDF_PATH)

    idf_build_get_property(extra_subtypes EXTRA_PARTITION_SUBTYPES)
    if(extra_subtypes)
        string(REPLACE " " "" extra_subtypes "${extra_subtypes}")
        set(extra_partition_subtypes --extra-partition-subtypes ${extra_subtypes})
    else()
        set(extra_partition_subtypes "")
    endif()
    separate_arguments(get_part_info_args)
    execute_process(COMMAND ${python}
        ${idf_path}/components/partition_table/parttool.py -q
        --partition-table-offset ${PARTITION_TABLE_OFFSET}
        --primary-bootloader-offset ${BOOTLOADER_OFFSET}
        ${RECOVERY_BOOTLOADER_OPTION}
        --partition-table-file ${PARTITION_CSV_PATH}
        get_partition_info ${get_part_info_args} --info ${part_info}
        ${extra_partition_subtypes}
        OUTPUT_VARIABLE info
        RESULT_VARIABLE exit_code
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT ${exit_code} EQUAL 0 AND NOT ${exit_code} EQUAL 1)
        message(WARNING "parttool.py execution failed (${result}), problem with partition CSV file (see above)")
    endif()
    set(${result} "${info}" PARENT_SCOPE)
endfunction()

# Tầng 2: Chặn lệnh nạp flash nếu có bất kỳ file srmodels.bin nào cố nạp vào phân vùng 'model'
function(esptool_py_flash_to_partition target_name partition_name binary_path)
    if(binary_path MATCHES "srmodels" AND partition_name STREQUAL "model")
        message(STATUS "[Xiaozhi] Chan thanh cong muc nap trung lap esp-sr srmodels.bin vao phan vung 'model'")
        return()
    endif()

    partition_table_get_partition_info(offset "--partition-name ${partition_name}" "offset")
    if(NOT offset)
        message(FATAL_ERROR "Could not find offset of partition ${partition_name}")
    endif()

    esptool_py_partition_needs_encryption(encrypted ${partition_name})
    if(NOT ${encrypted})
        set(option ALWAYS_PLAINTEXT)
    endif()

    esptool_py_flash_target_image(${target_name} ${partition_name} ${offset}
                                  ${binary_path} ${option})
endfunction()


set(PROJECT_VER "2.5.0")
project(xiaozhi)

# --- ESP-DSP GCC ICE Workaround ---
# The xtensa-esp-elf-gcc compiler has a known bug causing a Segmentation fault
# (internal compiler error during RTL pass: ira) when compiling dspi_conv_f32_ansi.c
# We append -O1 to the esp-dsp component to bypass this compiler bug.
idf_component_get_property(esp_dsp_lib espressif__esp-dsp COMPONENT_LIB)
if(esp_dsp_lib)
    message(STATUS "[Xiaozhi] Applying -O1 workaround to espressif__esp-dsp to avoid GCC ICE")
    target_compile_options(${esp_dsp_lib} PRIVATE "-O1")
endif()
