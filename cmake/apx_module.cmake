# cmake-format: off
function(apx_module)

    apx_parse_function_args(
        NAME apx_module
        ONE_VALUE
            MODULE_NAME
            PREFIX
            BEFORE
            TYPE
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
    # file(REAL_PATH "${CMAKE_CURRENT_LIST_DIR}" path)
    if(${path} MATCHES "^${APX_SHARED_DIR}")
        file(RELATIVE_PATH path ${APX_SHARED_DIR} ${path})
        set(path "shared.${path}")
    elseif(${path} MATCHES "^${APX_MODULES_DIR}")
        file(RELATIVE_PATH path ${APX_MODULES_DIR} ${path})
    else()
        message(FATAL_ERROR "Can't guess module name: ${CMAKE_CURRENT_SOURCE_DIR} in ${APX_MODULES_DIR}")
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
        if(BEFORE)
            get_property(all_modules GLOBAL PROPERTY APX_MODULES_INIT)
            list(FIND all_modules ${BEFORE} before_index)
            if(before_index LESS 0)
                message(FATAL_ERROR "Cant insert module ${MODULE} before ${BEFORE}")
            endif()
            list(INSERT all_modules ${before_index} ${MODULE})
            set_property(GLOBAL PROPERTY APX_MODULES_INIT ${all_modules})
        else()
            set_property(GLOBAL APPEND PROPERTY APX_MODULES_INIT ${MODULE})
        endif()
    endif()

    # meta data
    set(meta)
    file(GLOB meta_list ${CMAKE_CURRENT_LIST_DIR}/${MODULE_NAME}.yml ${CMAKE_CURRENT_LIST_DIR}/${MODULE_NAME}-*.yml ${CMAKE_CURRENT_LIST_DIR}/*.yml)
    foreach(meta_f ${meta_list})
        set(meta ${meta_f})
        set_property(GLOBAL APPEND PROPERTY APX_MODULES_META ${meta})
        # message(STATUS "META: ${meta}")
        break()
    endforeach()

    # glob SRCS when needed
    apx_glob_srcs(${SRCS})
    # message(STATUS "SRC: ${SRCS}")

    # check if src contains headers only - i.e. interface
    if(GENSRC)
        foreach(gensrc ${GENSRC})
            get_target_property(gensrc_targets ${gensrc} INTERFACE_SOURCES)
            list(APPEND SRCS ${gensrc_targets})
        endforeach()
    endif()
    foreach(src ${SRCS})
        get_filename_component(ext ${src} LAST_EXT)
        string(TOLOWER ${ext} ext)
        if(ext MATCHES "^\.[csS]")
            set(has_sources ON)
        endif()
    endforeach()

    # add library
    if(has_sources)
        if(TYPE)
            add_library(${MODULE} ${TYPE} ${SRCS})
        else()
            add_library(${MODULE} STATIC EXCLUDE_FROM_ALL ${SRCS})
        endif()
    else()
        add_library(${MODULE} INTERFACE ${SRCS})
    endif()

    # includes
    if(INCLUDES)
        foreach(inc ${INCLUDES})
            get_filename_component(inc ${inc} ABSOLUTE)
            # file(REAL_PATH ${inc} inc)
            if(PREFIX)
                message(STATUS "PREFIX: ${PREFIX}")
                file(RELATIVE_PATH inc_rel ${CMAKE_CURRENT_SOURCE_DIR} ${inc})
                get_filename_component(inc_link ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}/${inc_rel} ABSOLUTE)
                get_filename_component(link_dir ${inc_link} DIRECTORY)
                file(MAKE_DIRECTORY ${link_dir})
                file(CREATE_LINK ${inc} ${inc_link} SYMBOLIC)
                set(inc ${CMAKE_CURRENT_BINARY_DIR})
            endif()
            if(has_sources)
                target_include_directories(${MODULE} PUBLIC ${inc})
            else()
                target_include_directories(${MODULE} INTERFACE ${inc})
            endif()
        endforeach()
    endif()

    # defines
    if(has_sources)
        string(TOLOWER ${MODULE_NAME} MODULE_NAME_LC)
        target_compile_definitions(${MODULE} PRIVATE "MODULE_NAME=\"${MODULE_NAME_LC}\"")
    endif()

    # generated sources
    if(GENSRC)
        foreach(gensrc ${GENSRC})
            add_dependencies(${MODULE} ${gensrc})
            get_target_property(gensrc_targets ${gensrc} INTERFACE_SOURCES)
            get_target_property(gensrc_dir ${gensrc} INCLUDE_DIRECTORIES)
            if(has_sources)
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
            set(dep_public STATIC_LIBRARY INTERFACE_LIBRARY SHARED_LIBRARY)
            if(dep_type IN_LIST dep_public)
                if(has_sources)
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
    if(NOT has_sources)
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
