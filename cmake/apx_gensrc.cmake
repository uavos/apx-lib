# cmake-format: off
function(apx_gensrc TARGET)

    apx_parse_function_args(
        NAME apx_gensrc
        MULTI_VALUE
            SRCS
            DATA
            PREFIX
            DEPENDS
        REQUIRED
            DATA
        ARGN ${ARGN}
    )
# cmake-format: on

    set(script ${APX_SHARED_TOOLS_DIR}/gensrc.py)
    if(NOT EXISTS ${script})
        message(FATAL_ERROR "Script not found: ${script}")
    endif()

    # glob SRCS when needed
    set(srcs)
    if(NOT SRCS)
        set(SRCS "*.j2")
    endif()
    foreach(src ${SRCS})
        if(NOT EXISTS ${src})
            file(GLOB src ${src})
        endif()
        list(APPEND srcs ${src})
    endforeach()

    # dest
    set(dest_dir ${CMAKE_CURRENT_BINARY_DIR}/gensrc)
    set(dest ${dest_dir})
    if(PREFIX)
        string(APPEND dest "/${PREFIX}")
    endif()

    # target files
    set(targets)
    foreach(src ${srcs})
        string(REGEX REPLACE "(\.j2)$" "" src ${src})
        get_filename_component(ext ${src} LAST_EXT)
        if(ext)
            get_filename_component(src ${src} NAME)
            # file(RELATIVE_PATH src ${CMAKE_CURRENT_SOURCE_DIR} ${src})
            list(APPEND targets "${dest}/${src}")
        endif()
    endforeach()
    if(NOT targets)
        message(FATAL_ERROR "No targets: ${SRCS}")
    endif()

    # data format and deps
    set(deps ${DATA})
    if(DATA MATCHES "^\#.*")
        set(deps)
        set(DATA "${DATA}")
    endif()

    # parse templates command
    add_custom_command(
        OUTPUT ${targets}
        COMMAND ${PYTHON_EXECUTABLE} ${script} --data "${DATA}" --template ${srcs} --dest ${dest}
        DEPENDS ${script} ${srcs} ${deps} ${DEPENDS}
        VERBATIM
    )
    add_custom_target(${TARGET} DEPENDS ${targets})
    # set_source_files_properties(${targets} PROPERTIES GENERATED TRUE)

    # output
    add_library(${TARGET}_lib INTERFACE)
    add_dependencies(${TARGET}_lib ${TARGET})
    target_sources(${TARGET}_lib INTERFACE ${targets})
    target_include_directories(${TARGET}_lib INTERFACE ${dest_dir})

    # add_custom_target(${TARGET} DEPENDS ${targets})

    # set_property(TARGET ${TARGET} PROPERTY INTERFACE_SOURCES ${targets})
    # set_property(TARGET ${TARGET} PROPERTY INCLUDE_DIRECTORIES ${dest_dir})

endfunction()
