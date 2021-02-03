include_guard()

find_package(Git QUIET)

if(NOT GIT_FOUND)
    message(FATAL_ERROR "Git is required")
endif()

# find root

execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --show-superproject-working-tree
    OUTPUT_VARIABLE APX_GIT_ROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
if(NOT APX_GIT_ROOT)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --show-toplevel
        OUTPUT_VARIABLE APX_GIT_ROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
if(NOT APX_GIT_ROOT)
    message(FATAL_ERROR "git root directory not found")
endif()

# Update submodules as needed
# if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
#     option(GIT_SUBMODULE "Check submodules during build" ON)
#     if(GIT_SUBMODULE)
#         message(STATUS "Submodule update")
#         execute_process(
#             COMMAND ${GIT_EXECUTABLE} submodule update --recursive
#             WORKING_DIRECTORY ${APX_GIT_ROOT}
#             RESULT_VARIABLE GIT_SUBMOD_RESULT
#         )
#         if(NOT GIT_SUBMOD_RESULT EQUAL "0")
#             message(FATAL_ERROR "git submodule update failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
#         endif()
#     endif()
# endif()

# check if we are on specific release
execute_process(
    COMMAND ${GIT_EXECUTABLE} tag --points-at HEAD
    OUTPUT_VARIABLE APX_GIT_TAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${APX_GIT_ROOT}
)
string(REPLACE "\n" ";" APX_GIT_TAGS ${APX_GIT_TAGS})
foreach(tag ${APX_GIT_TAGS})
    if(tag MATCHES "^release-")
        string(REGEX MATCH "^release-([0-9]*\.[0-9]*\.[0-9]*)" APX_GIT_VERSION ${tag})
        set(APX_GIT_VERSION ${CMAKE_MATCH_1})
        if(APX_GIT_VERSION)
            message(STATUS "** RELEASE VERSION BUILD ${APX_GIT_VERSION}**")
            break()
        endif()
    endif()
endforeach()

if(NOT APX_GIT_VERSION)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --always --tags
        OUTPUT_VARIABLE APX_GIT_IDENTITY
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${APX_GIT_ROOT}
    )

    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --always --tags --match "v*.*"
        OUTPUT_VARIABLE APX_GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${APX_GIT_ROOT}
    )

    if(APX_GIT_VERSION MATCHES "^v([0-9]*\.[0-9]*)")
        string(REPLACE "v" "" APX_GIT_VERSION ${APX_GIT_VERSION})
        string(REPLACE "-" "." APX_GIT_VERSION ${APX_GIT_VERSION})
        string(REGEX MATCH "^([0-9]*\.[0-9]*\.[0-9]*)" APX_GIT_VERSION ${APX_GIT_VERSION})
    else()
        set(APX_GIT_VERSION "1.1.1")
        message(STATUS "** OUT OF TREE BUILD **")
    endif()
endif()

execute_process(
    COMMAND ${GIT_EXECUTABLE} branch --show-current
    OUTPUT_VARIABLE APX_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${APX_GIT_ROOT}
)

execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    OUTPUT_VARIABLE APX_GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${APX_GIT_ROOT}
)

execute_process(
    COMMAND ${GIT_EXECUTABLE} show --oneline --format=%ci -s HEAD
    OUTPUT_VARIABLE APX_GIT_TIME
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${APX_GIT_ROOT}
)
string(REGEX MATCH "^([0-9]+)-" APX_GIT_YEAR ${APX_GIT_TIME})
set(APX_GIT_YEAR ${CMAKE_MATCH_1})

string(REPLACE "." ";" version_list ${APX_GIT_VERSION})
list(GET version_list 0 APX_GIT_VERSION_MAJOR)
list(GET version_list 1 APX_GIT_VERSION_MINOR)
list(GET version_list 2 APX_GIT_VERSION_BUILD)

message(STATUS "APX version: ${APX_GIT_VERSION} (${APX_GIT_BRANCH})")
