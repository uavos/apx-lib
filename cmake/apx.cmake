include_guard()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CONFIGURE_DEPENDS ON)

get_filename_component(APX_SHARED_TOOLS_DIR "${CMAKE_CURRENT_LIST_DIR}/../tools" ABSOLUTE)

define_property(
    GLOBAL
    PROPERTY APX_MODULES
    BRIEF_DOCS "APX modules"
    FULL_DOCS "List of all used APX modules"
)

define_property(
    GLOBAL
    PROPERTY APX_MODULES_PATHS
    BRIEF_DOCS "APX modules paths"
    FULL_DOCS "List of paths to all used APX modules"
)

define_property(
    GLOBAL
    PROPERTY APX_MODULES_META
    BRIEF_DOCS "APX modules metadata"
    FULL_DOCS "List of YAML files with metadata of all used APX modules"
)

if(APPLE)
    set(Python3_ROOT_DIR "/usr/bin")
endif()

find_package(
    Python3
    COMPONENTS Interpreter
    REQUIRED
)
set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})

include(apx_parse_function_args)

include(apx_srcs)
include(apx_module)
include(apx_use_module)
include(apx_gensrc)

include(apx_targets)
