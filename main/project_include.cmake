# Project include for main component
message(STATUS "[Xiaozhi] Loading main/project_include.cmake")

# Override partition_table_get_partition_info to suppress esp-sr's model partition query
function(partition_table_get_partition_info result get_part_info_args part_info)
    if((CMAKE_CURRENT_LIST_DIR MATCHES "esp-sr" OR COMPONENT_NAME MATCHES "esp-sr") AND get_part_info_args MATCHES "model")
        message(STATUS "[Xiaozhi] Suppressed esp-sr model partition query (srmodels already bundled in generated_assets.bin)")
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

# Override esptool_py_flash_to_partition to block duplicate srmodels.bin flash to 'model' partition
function(esptool_py_flash_to_partition target_name partition_name binary_path)
    if(binary_path MATCHES "srmodels" AND partition_name STREQUAL "model")
        message(STATUS "[Xiaozhi] Suppressed duplicate srmodels.bin flash to 'model' partition")
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
