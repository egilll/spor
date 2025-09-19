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
    add_subdirectory(listener)

    # Round-trip wire format test (host-side)
    enable_testing()
    add_executable(roundtrip_test test/roundtrip.cpp src/spor-host/Decoder.cpp)
    target_include_directories(roundtrip_test PUBLIC ${ROOT} ${ROOT}/src ${ROOT}/src/spor-common)
    target_compile_features(roundtrip_test PRIVATE cxx_std_23)
    target_compile_definitions(roundtrip_test PRIVATE SPOR_HOST)
    add_test(NAME roundtrip COMMAND roundtrip_test)
    add_custom_target(run-roundtrip-test
            COMMAND $<TARGET_FILE:roundtrip_test>
            DEPENDS roundtrip_test
    )
endif ()
