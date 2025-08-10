function(apx_glob_srcs paths_var)

    set(paths ${${paths_var}})
    # message(STATUS "SRC_GLOB: ${paths}")

    # expand all source files to srcs list
    set(srcs)
    foreach(src ${paths})
        # message(STATUS "NEXT: ${src} ${CMAKE_CURRENT_SOURCE_DIR}")

        # check for recursive glob options (dir or ** in path)
        set(glob_type GLOB)
        # if folder, add /** to search recursively
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}/" OR EXISTS "${src}/")
            set(src "${src}/**")
        endif()
        if(src MATCHES "\\*\\*")
            set(glob_type GLOB_RECURSE)
        endif()

        # expand glob
        # message(STATUS "DIR: ${src}")
        file(
            ${glob_type}
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
            # message(STATUS "paths glob missing: ${src}")
        endif()

        # filter out paths with glob symbols and begin with underscore
        foreach(fsrc ${src})
            if(fsrc MATCHES "[\\*\\?]+")
                # message(STATUS "paths glob not expanded: ${fsrc}")
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

    # check if srcs contains directories or not existing files
    foreach(src ${srcs})
        if(src MATCHES "^/")
            file(RELATIVE_PATH src ${CMAKE_CURRENT_SOURCE_DIR} ${src})
        endif()
        if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${src})
            message(FATAL_ERROR "Not found: ${src}")
        endif()
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}/")
            message(FATAL_ERROR "paths glob expanded to directory: ${src}")
        endif()
    endforeach()

    # message(STATUS "SRC_GLOB_OUT: ${srcs}")

    # return value
    set(${paths_var}
        ${srcs}
        PARENT_SCOPE
    )

endfunction()
