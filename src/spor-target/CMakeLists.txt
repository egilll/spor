cmake_minimum_required(VERSION 3.30)

project(spor-target)

#[[ 
    OPTIONS
]]

option(SPOR_ENABLE "Enable tracing" ON)

set(SPOR_SYSTEM_HEADER "" CACHE STRING "The system header of your target which includes CMSIS headers, e.g. '<gd32f4xx.h>'")

option(SPOR_INSTRUMENT_FUNCTIONS "Enable function instrumentation" OFF)

option(SPOR_RETARGET_WRITE "Enable retargeting of _write function" ON)

set(SPOR_TARGET_CHIP "Arm Cortex-M4" CACHE STRING "Target chip architecture")
set_property(CACHE SPOR_TARGET_CHIP PROPERTY STRINGS "Arm Cortex-M4")

set(SPOR_TARGET_OS "FreeRTOS" CACHE STRING "Target operating system")
set_property(CACHE SPOR_TARGET_OS PROPERTY STRINGS "None" "FreeRTOS")

option(SPOR_BUILD_LIBRARY "Build spor-target as a library" ON)
# You may wish to change the name of the generated library if you have multiple targets each with different flags
set(SPOR_LIBRARY_NAME "spor-target" CACHE STRING "Name of generated library")

#[[
    END OF OPTIONS
]]

set(CMAKE_CXX_STANDARD 26)

if (NOT SPOR_SYSTEM_HEADER OR SPOR_SYSTEM_HEADER STREQUAL "")
    message(FATAL_ERROR "SPOR_SYSTEM_HEADER must be set to a valid system header file (e.g. 'gd32f4xx.h')")
endif ()

file(GLOB sources
        src/*.cpp
        src/transport/*.cpp
)

set(include_dirs
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../
        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib
)

set(compile_defs SPOR_SYSTEM_HEADER=${SPOR_SYSTEM_HEADER})

if (SPOR_ENABLE)
    list(APPEND compile_defs SPOR_ENABLE)
endif ()

if (SPOR_TARGET_OS STREQUAL "FreeRTOS")
    file(GLOB freertos_sources src/freertos/*.cpp)
    list(APPEND sources ${freertos_sources})
    list(APPEND include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/src/freertos)
endif ()

if (SPOR_INSTRUMENT_FUNCTIONS)
    list(APPEND sources ${CMAKE_CURRENT_SOURCE_DIR}/src/instrument_functions/instrument_functions.cpp)
endif ()

if (SPOR_RETARGET_WRITE)
    list(APPEND compile_defs SPOR_RETARGET_WRITE)
endif ()

set(SPOR_SOURCES ${sources} PARENT_SCOPE)
set(SPOR_INCLUDE_DIRECTORIES ${include_dirs} PARENT_SCOPE)
set(SPOR_COMPILE_DEFINITIONS ${compile_defs} PARENT_SCOPE)

if (SPOR_BUILD_LIBRARY)
    add_library(${SPOR_LIBRARY_NAME} ${sources})
    target_compile_options(${SPOR_LIBRARY_NAME} PRIVATE
            -Wall
            -Werror
            -Wextra
            -Wno-unused-function
            -Wno-unused-parameter
            -Wno-unused-variable
            -Wno-deprecated-declarations
    )
    target_include_directories(${SPOR_LIBRARY_NAME} PUBLIC ${include_dirs})
    target_compile_definitions(${SPOR_LIBRARY_NAME} PUBLIC ${compile_defs})
endif ()
