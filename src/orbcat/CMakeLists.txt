cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(orbcat)

include(${ROOT}/src/orbcat/orbuculum.cmake)
target_compile_options(orbuculum PRIVATE -O3)

include(${ROOT}/cmake/flags.cmake)

add_library(${PROJECT_NAME})
file(GLOB_RECURSE src *.cpp)
target_sources(${PROJECT_NAME} PRIVATE
        ${src}
)
target_link_libraries(${PROJECT_NAME} PUBLIC orbuculum)
#set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE -O1)
endif ()