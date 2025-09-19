cmake_minimum_required(VERSION 3.28)

project(listener CXX)

# Only create the target on macOS when the specified J-Link dylib exists
set(JLINK_DYLIB "/Applications/SEGGER/JLink_V868/libjlinkarm.dylib")

if(APPLE AND EXISTS ${JLINK_DYLIB})
  add_executable(listener
    main.cpp
    JLink_RTT.cpp
  )
  target_include_directories(listener PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  # Link directly against the J-Link dylib as requested
  target_link_libraries(listener PRIVATE ${JLINK_DYLIB})
  target_compile_features(listener PRIVATE cxx_std_17)
else()
  message(STATUS "J-Link dylib not found at '${JLINK_DYLIB}'. Skipping 'listener' target.")
endif()

