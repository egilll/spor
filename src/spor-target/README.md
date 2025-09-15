# Target

## Setup

```cmake
add_subdirectory(spor-target)
target_link_libraries(${PROJECT_NAME} PUBLIC spor-target)
```

## FreeRTOS setup

In your `FreeRTOSConfig.h` file, add:

```c
#define configUSE_TRACE_FACILITY 1
#include "SporFreertosTrace.h"
```

If you're linking FreeRTOS using CMake (which requires adding new include paths to the `freertos_config` target), add:

```cmake
target_include_directories(freertos_config SYSTEM INTERFACE
        $<TARGET_PROPERTY:spor-target,INTERFACE_INCLUDE_DIRECTORIES>
)
```

## Usage



```c++
void* MyMutexHandle = malloc(10);

/**
 * Inform the host of what this symbol points to. 
 * This allows the host to show heap-allocated mutexes etc. as 
 */
SporAnnouncePointer(MyMutexHandle);


// Or:
SporSetName(&MyMutexHandle, "My Custom Name");
```

