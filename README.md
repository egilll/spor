# Spor – Profiler for Arm Cortex-M + FreeRTOS 

[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

This is a profiler for Arm Cortex-M chips running FreeRTOS. It supports profiling custom events and instrumenting functions while being aware of threads and interrupts.

The target sends data over Single Wire Output and the resulting analysis can be viewed in [Perfetto](https://perfetto.dev).

Currently only instrumented trace is supported; full Embedded Trace Macrocell (ETM) tracing is not yet supported.

## Usage

```cmake --preset release && \
cmake --preset release && \
cmake --build --preset release --target spor-host \ 
./build/release/spor-host/spor-host \
    --elf-file file.elf \
    --input-file file.swo
```

## Acknowledgements

Builds upon:

* [wolfpld/Tracy](https://github.com/wolfpld/tracy) by Bartosz
  Taudul [![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
* [Novakov/tracecompass-orbcode-trace-plugin](https://github.com/Novakov/tracecompass-orbcode-trace-plugin) by Maciej
  Nowak [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
* [Auterion/embedded-debug-tools](https://github.com/Auterion/embedded-debug-tools) [![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
* [orbcode/orbuculum](https://github.com/orbcode/orbuculum) [![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
