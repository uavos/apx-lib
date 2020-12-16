# cmake-format: off
function(apx_module)

    apx_parse_function_args(
        NAME apx_module
        ONE_VALUE
            MODULE
            PREFIX
        MULTI_VALUE
            SRCS
            INCLUDES
            DEPENDS
            COMPILE_FLAGS
            LINK_FLAGS
            GENSRC
        OPTIONS
            INTERFACE
            INIT
        ARGN ${ARGN}
    )
# cmake-format: on

    if(NOT MODULE)
        # guess module name
        set(path ${CMAKE_CURRENT_SOURCE_DIR})
        if(${path} MATCHES "^${APX_SHARED_DIR}")
            file(RELATIVE_PATH path ${APX_SHARED_DIR} ${path})
            set(path "shared.${path}")
            get_filename_component(GROUP_DIR ${APX_SHARED_DIR} ABSOLUTE)
        elseif(${path} MATCHES "^${APX_MODULES_DIR}")
            file(RELATIVE_PATH path ${APX_MODULES_DIR} ${path})
            get_filename_component(GROUP_DIR ${APX_SHARED_DIR} ABSOLUTE)
        else()
            message(FATAL_ERROR "Can't guess module name: ${CMAKE_CURRENT_SOURCE_DIR}")
        endif()
        string(REPLACE "/" "." MODULE ${path})
    endif()
    string(REPLACE "." ";" MODULE_NAME ${MODULE})
    list(GET MODULE_NAME -1 MODULE_NAME)
    string(TOLOWER ${MODULE_NAME} MODULE_NAME_LC)

    # first include dependencies (other modules)
    if(DEPENDS)
        foreach(dep ${DEPENDS})
            apx_use_module(${dep})
        endforeach()
    endif()

    # collect global modules list properties
    # add this module to the list *after* all dependencies for natural sorting
    set_property(GLOBAL APPEND PROPERTY APX_MODULES ${module})

    if(INIT)
        set_property(GLOBAL APPEND PROPERTY APX_MODULES_INIT ${module})
    endif()

    set(meta "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}.yml")
    if(EXISTS ${meta})
        set_property(GLOBAL APPEND PROPERTY APX_MODULES_META ${meta})
        # message(STATUS "META: ${meta}")
    endif()

    # glob SRCS when needed
    if(NOT SRCS)
        if(INTERFACE)
            set(SRCS "*.h*")
        else()
            set(SRCS "*.[ch]*")
        endif()
    endif()
    include(srcs)

    # add library
    if(INTERFACE)
        add_library(${MODULE} INTERFACE ${srcs})
    else()
        add_library(${MODULE} STATIC EXCLUDE_FROM_ALL ${srcs})
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
            if(INTERFACE)
                target_include_directories(${MODULE} INTERFACE ${inc})
            else()
                target_include_directories(${MODULE} PUBLIC ${inc})
            endif()
        endforeach()
    endif()

    # defines
    if(NOT INTERFACE)
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
            if(INTERFACE)
                target_include_directories(${MODULE} INTERFACE ${gensrc_dir})
            else()
                target_include_directories(${MODULE} PUBLIC ${gensrc_dir})
                target_sources(${MODULE} PRIVATE ${gensrc_targets})
            endif()
        endforeach()
    endif()

    # dependencies linking
    if(DEPENDS)
        foreach(dep ${DEPENDS})
            get_target_property(dep_type ${dep} TYPE)
            if(${dep_type} STREQUAL "STATIC_LIBRARY" OR ${dep_type} STREQUAL "INTERFACE_LIBRARY")
                if(INTERFACE)
                    target_link_libraries(${MODULE} INTERFACE ${dep})
                else()
                    target_link_libraries(${MODULE} PUBLIC ${dep})
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
    message(STATUS "MODULE: ${report}")

    # make module name available in parent scope
    set(MODULE
        ${MODULE}
        PARENT_SCOPE
    )

endfunction()
