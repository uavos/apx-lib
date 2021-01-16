# cmake-format: off
function(apx_use_module module)

    apx_parse_function_args(
        NAME apx_use_module
        ONE_VALUE
            DEST
        ARGN ${ARGN}
    )
# cmake-format: on

    get_property(modules GLOBAL PROPERTY APX_MODULES)
    if(${module} IN_LIST modules)
        return()
    endif()

    string(REPLACE "." ";" module_path_list ${module})
    string(REPLACE "." "/" module_path ${module})
    list(GET module_path_list 0 type)

    if(DEST)
        set(dest_path ${DEST})
    else()
        set(dest_path ${APX_BINARY_DIR}/modules/${module})
    endif()

    if(type STREQUAL "shared")
        list(POP_FRONT module_path_list)
        list(JOIN module_path_list "/" module_path)
        get_filename_component(module_path ${APX_SHARED_DIR}/${module_path} ABSOLUTE)
    else()
        get_filename_component(module_path ${APX_MODULES_DIR}/${module_path} ABSOLUTE)
    endif()

    get_property(module_paths GLOBAL PROPERTY APX_MODULES_PATHS)
    if(${module_path} IN_LIST module_paths)
        return()
    endif()

    set_property(GLOBAL APPEND PROPERTY APX_MODULES_PATHS ${module_path})

    file(GLOB cmake_list_file "${module_path}/CMakeLists.txt")
    file(GLOB cmake_inc_file "${module_path}/*.cmake")

    if(EXISTS "${cmake_list_file}")
        add_subdirectory(${module_path} ${dest_path})
    elseif(EXISTS "${cmake_inc_file}")
        include("${cmake_inc_file}")
    else()
        message(FATAL_ERROR "Module not found: ${module} (${module_path})")
    endif()

endfunction()
