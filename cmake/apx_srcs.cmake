function(apx_glob_srcs)

    if(ARGN)
        set(SRCS "${ARGN}")
    else()
        set(SRCS "*.[chsS]*;*.yml")
    endif()

    # message(STATUS "SRC_GLOB: ${SRCS}")

    set(srcs)
    foreach(src ${SRCS})
        # message(STATUS "NEXT: ${src} ${CMAKE_CURRENT_SOURCE_DIR}")
        set(glob GLOB)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}/" OR EXISTS "${src}/")
            set(src "${src}/**")
        endif()
        if(src MATCHES "\\*\\*")
            set(glob GLOB_RECURSE)
        endif()

        # message(STATUS "DIR: ${src}")
        file(
            ${glob}
            src_exp
            RELATIVE
            ${CMAKE_CURRENT_SOURCE_DIR}
            LIST_DIRECTORIES
            FALSE
            ${src}
        )
        if(src_exp)
            set(src ${src_exp})
        else()
            # message(STATUS "SRCS glob missing: ${src}")
        endif()

        foreach(fsrc ${src})
            if(fsrc MATCHES "[\\*\\?]+")
                # message(STATUS "SRCS glob not expanded: ${fsrc}")
                continue()
            endif()
            get_filename_component(fname ${fsrc} NAME)
            if(fname MATCHES "^[\\._].+")
                # message(FATAL_ERROR ${fname})
                continue()
            endif()
            list(APPEND srcs ${fsrc})
        endforeach()
    endforeach()

    foreach(src ${srcs})
        if(src MATCHES "^/")
            file(RELATIVE_PATH src ${CMAKE_CURRENT_SOURCE_DIR} ${src})
        endif()
        if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${src})
            message(FATAL_ERROR "Not found: ${src}")
        endif()
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}/")
            message(FATAL_ERROR "SRCS glob expanded to directory: ${src}")
        endif()
    endforeach()

    # return value
    set(SRCS
        ${srcs}
        PARENT_SCOPE
    )

    # message(STATUS "SRC_GLOB_OUT: ${srcs}")

endfunction()
