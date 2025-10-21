// SPDX-License-Identifier: MIT
#include "freertos/mutex.hpp"
#include "freertos/cpu.hpp"
#include "freertos/thread.hpp"
#if ESP_PLATFORM
#include <freertos/semphr.h>
#else
#include <semphr.h>
#endif

namespace freertos
{
#if (configUSE_MUTEXES == 1)

void mutex::unlock()
{
    configASSERT(!this_cpu::is_in_isr());

    // the same thread must unlock the mutex that has locked it
    configASSERT(get_locking_thread()->get_id() == this_thread::get_id());

    semaphore::release();
}

mutex::mutex()
{
    // construction not allowed in ISR
    configASSERT(!this_cpu::is_in_isr());

    xSemaphoreCreateMutexStatic(this);
}

#if (configUSE_RECURSIVE_MUTEXES == 1)

void recursive_mutex::unlock()
{
    configASSERT(!this_cpu::is_in_isr());

    // the same thread must unlock the mutex that has locked it
    configASSERT(get_locking_thread()->get_id() == this_thread::get_id());

    xSemaphoreGiveRecursive(handle());
}

bool recursive_mutex::recursive_take(tick_timer::duration timeout)
{
    configASSERT(!this_cpu::is_in_isr());

    return xSemaphoreTakeRecursive(handle(), to_ticks(timeout));
}

recursive_mutex::recursive_mutex()
{
    // construction not allowed in ISR
    configASSERT(!this_cpu::is_in_isr());

    xSemaphoreCreateRecursiveMutexStatic(this);
}

#endif // (configUSE_RECURSIVE_MUTEXES == 1)

#endif // (configUSE_MUTEXES == 1)
} // namespace freertos
