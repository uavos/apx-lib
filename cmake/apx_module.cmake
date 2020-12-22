# cmake-format: off
function(apx_module)

    apx_parse_function_args(
        NAME apx_module
        ONE_VALUE
            MODULE_NAME
            PREFIX
        MULTI_VALUE
            INCLUDES
            SRCS
            DEPENDS
            GENSRC
        OPTIONS
            INIT
        ARGN ${ARGN}
    )
# cmake-format: on

    # guess module name

    set(path ${CMAKE_CURRENT_LIST_DIR})
    if(${path} MATCHES "^${APX_SHARED_DIR}")
        file(RELATIVE_PATH path ${APX_SHARED_DIR} ${path})
        set(path "shared.${path}")
    elseif(${path} MATCHES "^${APX_MODULES_DIR}")
        file(RELATIVE_PATH path ${APX_MODULES_DIR} ${path})
    else()
        message(FATAL_ERROR "Can't guess module name: ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    string(REPLACE "/" "." MODULE ${path})
    if(NOT MODULE_NAME)
        string(REPLACE "." ";" MODULE_NAME ${MODULE})
        list(GET MODULE_NAME -1 MODULE_NAME)
    endif()

    # first include dependencies (other modules)
    if(DEPENDS)
        foreach(dep ${DEPENDS})
            if(NOT TARGET ${dep})
                apx_use_module(${dep})
            endif()
        endforeach()
    endif()

    # collect global modules list properties
    # add this module to the list *after* all dependencies for natural sorting
    set_property(GLOBAL APPEND PROPERTY APX_MODULES ${MODULE})

    if(INIT)
        set_property(GLOBAL APPEND PROPERTY APX_MODULES_INIT ${MODULE})
    endif()

    set(meta "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}.yml")
    if(EXISTS ${meta})
        set_property(GLOBAL APPEND PROPERTY APX_MODULES_META ${meta})
        # message(STATUS "META: ${meta}")
    else()
        set(meta)
    endif()

    # glob SRCS when needed
    if(NOT SRCS)
        set(SRCS "*.[ch]*")
    endif()
    set(srcs)
    foreach(src ${SRCS})
        # if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/${src})
        if(src MATCHES "[\*]")
            file(
                GLOB src_exp
                RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_LIST_DIR}/${src}
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
    # message(STATUS "SRC: ${MODULE}: ${SRCS}")

    # check if src contains headers only - i.e. interface
    if(GENSRC)
        foreach(gensrc ${GENSRC})
            get_target_property(gensrc_targets ${gensrc} INTERFACE_SOURCES)
            list(APPEND srcs ${gensrc_targets})
        endforeach()
    endif()
    foreach(src ${srcs})
        get_filename_component(ext ${src} LAST_EXT)
        string(TOLOWER ${ext} ext)
        if(ext MATCHES "^\.[cs]")
            set(STATIC ON)
        endif()
    endforeach()

    # add library
    if(STATIC)
        add_library(${MODULE} STATIC EXCLUDE_FROM_ALL ${SRCS})
    else()
        add_library(${MODULE} INTERFACE ${SRCS})
    endif()

    # includes
    if(INCLUDES)
        foreach(inc ${INCLUDES})
            file(REAL_PATH ${inc} inc)
            if(PREFIX)
                file(RELATIVE_PATH inc_rel ${CMAKE_CURRENT_SOURCE_DIR} ${inc})
                get_filename_component(inc_link ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}/${inc_rel} ABSOLUTE)
                get_filename_component(link_dir ${inc_link} DIRECTORY)
                file(MAKE_DIRECTORY ${link_dir})
                file(CREATE_LINK ${inc} ${inc_link} SYMBOLIC)
                set(inc ${CMAKE_CURRENT_BINARY_DIR})
            endif()
            if(STATIC)
                target_include_directories(${MODULE} PUBLIC ${inc})
            else()
                target_include_directories(${MODULE} INTERFACE ${inc})
            endif()
        endforeach()
    endif()

    # defines
    if(STATIC)
        string(TOLOWER ${MODULE_NAME} MODULE_NAME_LC)
        target_compile_definitions(${MODULE} PRIVATE "MODULE_NAME=\"${MODULE_NAME_LC}\"")
    endif()

    # compile flags
    if(COMPILE_FLAGS)
        target_compile_options(${MODULE}_original PRIVATE ${COMPILE_FLAGS})
    endif()
    if(LINK_FLAGS)
        set_target_properties(${MODULE} PROPERTIES LINK_FLAGS ${LINK_FLAGS})
    endif()

    # generated sources
    if(GENSRC)
        foreach(gensrc ${GENSRC})
            add_dependencies(${MODULE} ${gensrc})
            get_target_property(gensrc_targets ${gensrc} INTERFACE_SOURCES)
            get_target_property(gensrc_dir ${gensrc} INCLUDE_DIRECTORIES)
            if(STATIC)
                target_include_directories(${MODULE} PUBLIC ${gensrc_dir})
                target_sources(${MODULE} PRIVATE ${gensrc_targets})
            else()
                target_include_directories(${MODULE} INTERFACE ${gensrc_dir})
            endif()
        endforeach()
    endif()

    # dependencies linking
    if(DEPENDS)
        foreach(dep ${DEPENDS})
            get_target_property(dep_type ${dep} TYPE)
            if(${dep_type} STREQUAL "STATIC_LIBRARY" OR ${dep_type} STREQUAL "INTERFACE_LIBRARY")
                if(STATIC)
                    target_link_libraries(${MODULE} PUBLIC ${dep})
                else()
                    target_link_libraries(${MODULE} INTERFACE ${dep})
                endif()
            else()
                add_dependencies(${MODULE} ${dep})
            endif()
        endforeach()
    endif()

    # report
    set(report ${MODULE})
    if(INIT)
        set(report "${report} INIT")
    endif()
    if(NOT STATIC)
        set(report "${report} IFACE")
    endif()
    if(meta)
        set(report "${report} META")
    endif()
    message(STATUS "MODULE: ${report}")

    # make module name available in parent scope
    set(MODULE
        ${MODULE}
        PARENT_SCOPE
    )

endfunction()
