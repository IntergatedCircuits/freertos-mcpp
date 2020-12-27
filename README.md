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
* Tested with [FreeRTOS][FreeRTOS-Kernel] 10, its public API is stable enough to enable the use on a wide range of versions
* Only works with FreeRTOS ports that have `xPortIsInsideInterrupt()` call implemented

## Porting

This library requires certain configuration values to be set for correct operation.
Consider the recommended settings for `FreeRTOSConfig.h`:

```C
// required globally
#define configSUPPORT_STATIC_ALLOCATION         1

// required to allow termination of threads and automatic resource freeing (see thread documentation)
// and for thread_owner that builds on it
#define configSUPPORT_DYNAMIC_ALLOCATION        1
    extern void vTaskExitHandler(void);
#define configTASK_RETURN_ADDRESS               vTaskExitHandler

// required for thread termination signalling, used by thread_owner
// configNUM_THREAD_LOCAL_STORAGE_POINTERS must be higher than configTHREAD_EXIT_SEMAPHORE_INDEX
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 1
#define configTHREAD_EXIT_SEMAPHORE_INDEX       0

// required to support mutex, timed_mutex
#define configUSE_MUTEXES                       1

// required to support recursive_mutex, recursive_timed_mutex
#define configUSE_RECURSIVE_MUTEXES             1

// required to support counting_semaphore
#define configUSE_COUNTING_SEMAPHORES           1


// recommended to use on Cortex Mx architectures (see src/helpers/runtime_stats_timer.c)
#define configGENERATE_RUN_TIME_STATS           1
    extern void ConfigureTimerForRunTimeStats(void);
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()    ConfigureTimerForRunTimeStats()
    extern uint32_t GetRuntimeCounterValueFromISR(void);
#define portGET_RUN_TIME_COUNTER_VALUE()            GetRuntimeCounterValueFromISR()
```

In addition to the C++ wrappers, there are helper files located in `src/helpers` for some common use-cases:

1. `tasks_static.c` is required as source to support static allocation of kernel objects
2. `runtime_stats_timer.c` is a zero-cost runtime statistics timer for Cortex Mx architectures
3. `malloc_free.c` and `new_delete_ops.cpp` redirect heap allocation to FreeRTOS's heap management


[FreeRTOS]: https://www.freertos.org/
[FreeRTOS-Kernel]: https://github.com/FreeRTOS/FreeRTOS-Kernel
