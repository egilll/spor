cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

project(spor)

if (IS_TARGET)
    add_subdirectory(test/build-target)
else ()
    include(cmake/get_cpm.cmake)
    add_subdirectory(src/profiler-api profiler-api)
    add_subdirectory(src/spor-host spor-host)
endif ()
