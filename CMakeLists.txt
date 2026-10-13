cmake_minimum_required(VERSION 3.16)

set(ENV{PYTHONUTF8} "1")
set(ENV{PYTHONIOENCODING} "utf-8")

add_compile_options(-Wno-missing-field-initializers)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)

set(PROJECT_VER "2.5.0")
project(xiaozhi)

idf_component_get_property(esp_dsp_lib espressif__esp-dsp COMPONENT_LIB)
if(esp_dsp_lib)
    message(STATUS "[Xiaozhi] Applying -O1 workaround to espressif__esp-dsp to avoid GCC ICE")
    target_compile_options(${esp_dsp_lib} PRIVATE "-O1")
endif()
