cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)
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
#target_link_libraries(${PROJECT_NAME} PUBLIC orbcat)
#set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)


include(FindPkgConfig)
pkg_check_modules(Capstone REQUIRED capstone)
if (Capstone_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${Capstone_LDFLAGS})
    target_include_directories(${PROJECT_NAME} PUBLIC ${Capstone_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "Capstone not found")
endif ()

pkg_check_modules(LIBELF REQUIRED libelf)
if (LIBELF_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBELF_LDFLAGS})
    target_include_directories(${PROJECT_NAME} PUBLIC ${LIBELF_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "Libelf not found")
endif ()

CPMAddPackage(
        NAME libdwarf
        GIT_REPOSITORY https://github.com/davea42/libdwarf-code.git
        GIT_TAG "a5a9232"
        GIT_SHALLOW TRUE
        OPTIONS
        "BUILD_NON_SHARED OFF"
        "BUILD_SHARED ON"
)
target_link_libraries(${PROJECT_NAME} PUBLIC libdwarf::dwarf-shared)
