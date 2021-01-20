macro(apx_srcs var)

    if(NOT ${var})
        set(${var} "*.[chsS]*")
    endif()

    # message(STATUS "SRC_LOOKUP: ${${var}}")

    set(srcs)
    foreach(src ${${var}})
        if(EXISTS "${src}/")
            file(
                GLOB_RECURSE src_exp
                RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
                ${src}/*
            )
            if(src_exp)
                set(src ${src_exp})
            endif()
        elseif(src MATCHES "[\\*]")
            file(
                GLOB src_exp
                RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
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

    set(${var} ${srcs})

    # message(STATUS "SRC: ${srcs}")

endmacro()
