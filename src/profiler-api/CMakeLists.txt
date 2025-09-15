cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(profiler)

include(${ROOT}/cmake/flags.cmake)

CPMAddPackage(
        NAME perfetto
        GIT_REPOSITORY https://github.com/google/perfetto.git
        GIT_TAG v50.1
        DOWNLOAD_ONLY YES
)
add_library(perfetto SHARED ${perfetto_SOURCE_DIR}/sdk/perfetto.cc)
target_include_directories(perfetto PUBLIC ${perfetto_SOURCE_DIR}/sdk)
target_compile_options(perfetto PRIVATE -O3 -Wno-error -w)

add_library(${PROJECT_NAME} STATIC)

file(GLOB_RECURSE SOURCES *.cpp)
target_sources(${PROJECT_NAME} PRIVATE
        ${SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ./ ../)
#set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)
target_link_libraries(${PROJECT_NAME} PUBLIC perfetto)
