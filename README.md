# FreeRTOS Modern C++ Wrappers

[![License](http://img.shields.io/:license-mit-blue.svg?style=flat-square)](http://badges.mit-license.org)

**freertos-mcpp** is a C++ wrapper library that allows developers to use the ubiquitous [FreeRTOS][FreeRTOS] kernel
while simplifying its use by a new API that closely follows the C++ standard classes.

## Features

* No virtual classes, the wrapper classes accurately encapsulate the underlying data structures
* Promotes static allocation, optimizing RAM use and reducing heap fragmentation risks
* Public API closely matches the standard C++ thread support library
* The API selects the threading or interrupt service routine (xFromISR) FreeRTOS API calls by detecting ISR context

## Compatibility

* C++11 and above
* [ESP-IDF][ESP-IDF] platform supported (simply add it to `EXTRA_COMPONENT_DIRS`, or submodule it into `{project}/components`)
* [MCUXpresso SDK][MCUXpresso] supported (as a west module, drop-in, see [mcux/Kconfig](./mcux/Kconfig))

## Porting

In general the library adheres to the FreeRTOS configuration flags,
so there's little integration work needed.
Certain features and functionality require specific configurations for `FreeRTOSConfig.h`,
see below:

```C
// required globally
#define configSUPPORT_STATIC_ALLOCATION         1

// optional, to allow termination of threads and automatic resource freeing (see thread documentation)
// NOTE: statically allocated threads are still not allowed to return!
#define configSUPPORT_DYNAMIC_ALLOCATION        1
    extern void vTaskExitHandler(void);
#define configTASK_RETURN_ADDRESS               vTaskExitHandler

// optional, for thread termination signalling, used by thread::join
// configNUM_THREAD_LOCAL_STORAGE_POINTERS must be higher than configTHREAD_EXIT_CONDITION_INDEX
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 1
#define configTHREAD_EXIT_CONDITION_INDEX       0

```

In addition to the C++ wrappers, there are helper files located in `src/helpers` for some common use-cases:

1. `tasks_static.c` is required as source to support static allocation of kernel objects
2. `runtime_stats_timer.c` is a zero-cost runtime statistics timer for Cortex Mx architectures
3. `malloc_free.c` and `new_delete_ops.cpp` redirect heap allocation to FreeRTOS's heap management


[FreeRTOS]: https://www.freertos.org/
[FreeRTOS-Kernel]: https://github.com/FreeRTOS/FreeRTOS-Kernel
[ESP-IDF]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html
[MCUXpresso]: https://mcuxpresso.nxp.com/mcuxsdk/latest/html/introduction/README.html
