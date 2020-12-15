set(srcs)
foreach(src ${SRCS})
    if(NOT EXISTS ${src})
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
set(SRCS ${srcs})
