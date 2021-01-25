macro(apx_get_all_targets_recursive targets dir)
    get_property(
        subdirectories
        DIRECTORY ${dir}
        PROPERTY SUBDIRECTORIES
    )
    foreach(subdir ${subdirectories})
        apx_get_all_targets_recursive(${targets} ${subdir})
    endforeach()

    get_property(
        current_targets
        DIRECTORY ${dir}
        PROPERTY BUILDSYSTEM_TARGETS
    )
    list(APPEND ${targets} ${current_targets})
endmacro()

function(apx_get_all_targets var)
    set(targets)
    apx_get_all_targets_recursive(targets ${CMAKE_CURRENT_SOURCE_DIR})
    set(${var}
        ${targets}
        PARENT_SCOPE
    )
endfunction()

function(apx_print_all_targets)
    apx_get_all_targets(all_targets)
    message(STATUS "TARGETS: ${all_targets}")
endfunction()
