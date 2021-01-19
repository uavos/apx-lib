macro(apx_srcs var)

    if(NOT ${var})
        set(${var} "*.[chsS]*")
    endif()

    # message(STATUS "SRC_LOOKUP: ${${var}}")

    set(srcs)
    foreach(src ${${var}})
        if(src MATCHES "[\*]")
            file(
                GLOB src_exp
                RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
                ${src}
            )
            if(src_exp)
                set(src ${src_exp})
            endif()
        endif()
        if(NOT src MATCHES "[\*]")
            list(APPEND srcs ${src})
        endif()
    endforeach()

    set(${var} ${srcs})

    # message(STATUS "SRC: ${srcs}")

endmacro()
