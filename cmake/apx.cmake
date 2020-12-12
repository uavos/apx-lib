include_guard()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

set(CMAKE_CONFIGURE_DEPENDS ON)

# set(CMAKE_VERBOSE_MAKEFILE
#     ON
#     CACHE BOOL "ON"
# )

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

find_package(PythonInterp 3 REQUIRED)

include(apx_parse_function_args)

include(apx_module)
include(apx_use_module)
include(apx_gensrc)
