cmake_minimum_required(VERSION 3.30)

set(HAVE_FLAG_SEARCH_PATHS_FIRST 0) # Remove an annoying default linker flag set by Clang
set(CMAKE_C_COMPILER_WORKS 1) # To prevent CMake from trying to compile for ARM with host flags
set(CMAKE_CXX_COMPILER_WORKS 1)

#set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
#set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
set(CMAKE_SYSTEM_NAME "Generic")

set(SPOR_SYSTEM_HEADER "<gd32f4xx.h>")

set(SPOR_ENABLE ON)
set(SPOR_RETARGET_WRITE ON)
set(SPOR_BUILD_LIBRARY ON)

project(spor-target-test)

set(ROOT ../..)

include(${ROOT}/cmake/get_cpm.cmake)

add_compile_options(
        -mfloat-abi=hard
        -mfpu=fpv4-sp-d16
        -mthumb
        -mcpu=cortex-m4
        #        --target=arm-none-eabi
        -DGD32F450
        --specs=nano.specs
        -Wno-psabi
)
add_link_options(
        --specs=nano.specs
        #        -nostartfiles
        -mfloat-abi=hard
        -mfpu=fpv4-sp-d16
        -mthumb
        -mcpu=cortex-m4
        #        -lstdc++_nano
        #        -lsupc++_nano
        #        -lgcc
        #        -lc_nano
        #        -lm
        #        -lnosys
)
include_directories(SYSTEM
        gd32-libraries/CMSIS
        gd32-libraries/CMSIS/GD/GD32F4xx/Include/
)

add_subdirectory(${ROOT}/src/spor-target spor-target)

set(FREERTOS_PORT GCC_ARM_CM4F CACHE STRING "")

add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE
        .
        ${ROOT}
        $<TARGET_PROPERTY:spor-target,INTERFACE_INCLUDE_DIRECTORIES>
)

CPMAddPackage(
        NAME freertos_kernel
        GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
        GIT_TAG "V11.2.0"
        GIT_SHALLOW TRUE
)

CPMAddPackage(
        NAME libtrace
        GIT_REPOSITORY https://github.com/orbcode/libtrace.git
        GIT_TAG "0779932"
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR libs/trace
)

add_executable(target_test)
target_sources(target_test PRIVATE main.cpp
        gd32-libraries/CMSIS/GD/GD32F4xx/Source/ARM/startup_gd32f450_470.s
)
target_include_directories(target_test PUBLIC .)
target_link_libraries(target_test PUBLIC freertos_kernel)
target_link_libraries(spor-target PUBLIC freertos_kernel Orbcode::Trace)
target_link_libraries(target_test PUBLIC spor-target)
