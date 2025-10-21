// SPDX-License-Identifier: MIT
#include "freertos/scheduler.hpp"
#if ESP_PLATFORM
#include <freertos/task.h>
#else
#include <task.h>
#endif

namespace freertos
{

static_assert(taskSCHEDULER_SUSPENDED == static_cast<::BaseType_t>(scheduler::state::suspended),
              "These values must match!");
static_assert(taskSCHEDULER_NOT_STARTED ==
                  static_cast<::BaseType_t>(scheduler::state::uninitialized),
              "These values must match!");
static_assert(taskSCHEDULER_RUNNING == static_cast<::BaseType_t>(scheduler::state::running),
              "These values must match!");

void scheduler::start()
{
    // this call doesn't return
    vTaskStartScheduler();
}

size_t scheduler::get_threads_count()
{
    return uxTaskGetNumberOfTasks();
}

scheduler::state scheduler::get_state()
{
    return static_cast<scheduler::state>(xTaskGetSchedulerState());
}

void scheduler::critical_section::lock()
{
    vTaskSuspendAll();
}

void scheduler::critical_section::unlock()
{
    if (xTaskResumeAll())
    {
        // a context switch had already happened
    }
}
} // namespace freertos
