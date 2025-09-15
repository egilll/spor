cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(spor-host)

add_compile_definitions(
        SPOR_HOST
)
add_subdirectory(${ROOT}/src/orbcat orbcat)
include(${ROOT}/cmake/flags.cmake)

add_executable(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC
        ../
        ${ROOT}/lib
        ${ROOT}/src/symbol-resolver
)
file(GLOB_RECURSE src
        *.cpp
        ../spor-common/*.cpp
        ../spor-devices/*.cpp
        ${ROOT}/src/symbol-resolver/*.cpp
)
target_sources(${PROJECT_NAME} PRIVATE ${src})

target_link_libraries(${PROJECT_NAME} PUBLIC profiler)
target_link_libraries(${PROJECT_NAME} PUBLIC orbcat)
#set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)
