# cmake-format: off
function(apx_gensrc TARGET)

    apx_parse_function_args(
        NAME apx_gensrc
        MULTI_VALUE
            SRCS
            DATA
            PREFIX
        REQUIRED
            DATA
        ARGN ${ARGN}
    )
# cmake-format: on

    set(script ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../tools/gensrc.py)
    if(NOT EXISTS ${script})
        message(FATAL_ERROR "Script not found: ${script}")
    endif()

    # glob SRCS when needed
    set(srcs)
    if(NOT SRCS)
        set(SRCS "*.[ch]*.j2")
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
        file(RELATIVE_PATH src ${CMAKE_CURRENT_SOURCE_DIR} ${src})
        list(APPEND targets "${dest}/${src}")
    endforeach()

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
        DEPENDS ${script} ${srcs} ${deps}
        VERBATIM
    )

    # output
    set_source_files_properties(${targets} PROPERTIES GENERATED TRUE)
    add_custom_target(${TARGET} DEPENDS ${targets})

    set_property(TARGET ${TARGET} PROPERTY INTERFACE_SOURCES ${targets})
    set_property(TARGET ${TARGET} PROPERTY INCLUDE_DIRECTORIES ${dest_dir})

endfunction()
