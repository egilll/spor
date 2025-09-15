include_guard(GLOBAL)

CPMAddPackage(
        NAME orbuculum
        GIT_REPOSITORY https://github.com/orbcode/orbuculum.git
        GIT_TAG "ceec063"
        DOWNLOAD_ONLY TRUE
        GIT_SHALLOW TRUE
)

if (WIN32)
    set(ORBUCULUM_STREAM_SRCS
            ${orbuculum_SOURCE_DIR}/Src/stream_win32.c
            ${orbuculum_SOURCE_DIR}/Src/stream_file_win32.c
            ${orbuculum_SOURCE_DIR}/Src/stream_socket_win32.c
    )
else ()
    set(ORBUCULUM_STREAM_SRCS
            ${orbuculum_SOURCE_DIR}/Src/stream_file_posix.c
            ${orbuculum_SOURCE_DIR}/Src/stream_socket_posix.c
    )
endif ()

add_library(orbuculum STATIC
        ${ORBUCULUM_STREAM_SRCS}
        ${orbuculum_SOURCE_DIR}/Src/itmDecoder.c
        ${orbuculum_SOURCE_DIR}/Src/tpiuDecoder.c
        ${orbuculum_SOURCE_DIR}/Src/msgDecoder.c
        ${orbuculum_SOURCE_DIR}/Src/cobs.c
        ${orbuculum_SOURCE_DIR}/Src/oflow.c
        ${orbuculum_SOURCE_DIR}/Src/msgSeq.c
        ${orbuculum_SOURCE_DIR}/Src/traceDecoder_etm35.c
        ${orbuculum_SOURCE_DIR}/Src/traceDecoder_etm4.c
        ${orbuculum_SOURCE_DIR}/Src/traceDecoder_mtb.c
        ${orbuculum_SOURCE_DIR}/Src/traceDecoder.c
        ${orbuculum_SOURCE_DIR}/Src/generics.c
        ${orbuculum_SOURCE_DIR}/Src/readsource.c

        ####
        ${orbuculum_SOURCE_DIR}/Src/loadelf.c
)

target_include_directories(orbuculum PUBLIC SYSTEM
        ${orbuculum_SOURCE_DIR}/Inc
        ${orbuculum_SOURCE_DIR}/Inc/external
)

find_package(Threads REQUIRED)
target_link_libraries(orbuculum PUBLIC Threads::Threads)

if (WIN32)
    target_link_libraries(orbuculum PUBLIC ws2_32)
    target_compile_definitions(orbuculum PRIVATE WIN32)
elseif (APPLE)
    target_compile_definitions(orbuculum PRIVATE OSX)
else ()
    target_compile_definitions(orbuculum PRIVATE LINUX)
endif ()

target_compile_definitions(orbuculum PRIVATE GIT_VERSION=0) # Ignored

target_compile_options(orbuculum PRIVATE -Wno-error=deprecated-declarations -include uicolours_default.h)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(orbuculum PRIVATE -Wno-sign-compare)
endif ()


include(FindPkgConfig)
pkg_check_modules(Capstone REQUIRED capstone)
if (Capstone_FOUND)
    target_link_libraries(orbuculum PUBLIC ${Capstone_LDFLAGS})
    target_include_directories(orbuculum PUBLIC ${Capstone_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "Capstone not found")
endif ()

pkg_check_modules(LIBELF REQUIRED libelf)
if (LIBELF_FOUND)
    target_link_libraries(orbuculum PUBLIC ${LIBELF_LDFLAGS})
    target_include_directories(orbuculum PUBLIC ${LIBELF_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "Libelf not found")
endif ()

pkg_check_modules(LIBDWARF REQUIRED libdwarf)
if (LIBDWARF_FOUND)
    target_link_libraries(orbuculum PUBLIC ${LIBDWARF_LDFLAGS})
    target_include_directories(orbuculum PUBLIC ${LIBDWARF_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "Libdwarf not found")
endif ()

