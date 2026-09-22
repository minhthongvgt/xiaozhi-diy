# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

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

set(PROJECT_VER "2.5.0")
project(xiaozhi)
