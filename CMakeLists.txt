
set(SHIM_SOURCES
    src/condition_variable.cpp
    src/cpu.cpp
    src/event_group.cpp
    src/mutex.cpp
    src/pend_call.cpp
    src/queue.cpp
    src/scheduler.cpp
    src/semaphore.cpp
    src/thread.cpp
    src/tick_timer.cpp
    src/timed_service.cpp
)

if (ESP_PLATFORM)
    # ESP-IDF specific component registration
    # https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/build-system.html#component-cmakelists-files
    idf_component_register(
        REQUIRES freertos
        INCLUDE_DIRS include
        SRCS ${SHIM_SOURCES}
    )
elseif (MCUX_SDK_PROJECT_NAME)
    # MCUXpresso SDK specific component registration
    if (CONFIG_FREERTOS_MCPP_SHIM)
        mcux_add_include(INCLUDES include)
        mcux_add_source(SOURCES ${SHIM_SOURCES})
    endif()
else()
    # ?
endif()
