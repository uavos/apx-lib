function(apx_glob_srcs)

    if(ARGN)
        set(SRCS "${ARGN}")
    else()
        set(SRCS "*.[chsS]*")
    endif()

    # message(STATUS "SRC_GLOB: ${SRCS}")

    set(srcs)
    foreach(src ${SRCS})
        # message(STATUS "NEXT: ${src} ${CMAKE_CURRENT_SOURCE_DIR}")
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}/" OR EXISTS "${src}/")
            # message(STATUS "DIR: ${src}")
            file(
                GLOB_RECURSE src_exp
                RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
                LIST_DIRECTORIES FALSE
                ${src}/*
            )
            if(src_exp)
                set(src ${src_exp})
            endif()
        elseif(src MATCHES "[\\*]")
            file(
                GLOB src_exp
                RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
                LIST_DIRECTORIES FALSE
                ${src}
            )
            if(src_exp)
                set(src ${src_exp})
            endif()
        endif()
        foreach(fsrc ${src})
            if(fsrc MATCHES "[\\*]+")
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
            message(FATAL_ERROR "SRC is a directory: ${src}")
        endif()
    endforeach()

    # return value
    set(SRCS
        ${srcs}
        PARENT_SCOPE
    )

    message(STATUS "SRC_GLOB_OUT: ${srcs}")

endfunction()
