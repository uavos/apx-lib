cmake_minimum_required(VERSION 3.19 FATAL_ERROR)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(apx)

project(apx-shared LANGUAGES CXX C ASM)

set(APX_SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(APX_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# apx_use_module(shared.mandala)
# apx_use_module(shared.xbus.can)
# apx_use_module(shared.xbus.tcp)
# apx_use_module(shared.xbus.uart)
# apx_use_module(shared.xbus.telemetry)
