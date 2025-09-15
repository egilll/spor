cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${ROOT}/cmake/flags.cmake)

project(symbol-resolver)

add_library(${PROJECT_NAME} STATIC)

file(GLOB_RECURSE SOURCES *.cpp)
target_sources(${PROJECT_NAME} PRIVATE
        ${SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ./
)

#set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)

target_link_libraries(${PROJECT_NAME} PUBLIC perfetto)
