// SPDX-License-Identifier: MIT
#include <cstring>
#include "freertos/cpu.hpp"
#include "freertos/semaphore.hpp"
#if ESP_PLATFORM
#include <freertos/semphr.h>
#else
#include <semphr.h>
#endif

namespace freertos
{
#if 0 // doesn't seem like a good idea
semaphore::semaphore(semaphore&& other)
{
    std::memmove(this, &other, sizeof(semaphore));
    std::memset(&other, 0, sizeof(semaphore));
}

semaphore& semaphore::operator=(semaphore&& other)
{
    if (this != &other)
    {
        std::memmove(this, &other, sizeof(semaphore));
        std::memset(&other, 0, sizeof(semaphore));
    }
    return *this;
}
#endif

bool semaphore::take(tick_timer::duration timeout)
{
    if (!this_cpu::is_in_isr())
    {
        return xSemaphoreTake(handle(), to_ticks(timeout));
    }
    else
    {
        // cannot wait in ISR
        configASSERT(to_ticks(timeout) == 0);

        BaseType_t needs_yield = false;
        bool success = xSemaphoreTakeFromISR(handle(), &needs_yield);
        portYIELD_FROM_ISR(needs_yield);
        return success;
    }
}

bool semaphore::give(count_type update)
{
    // the API only allows giving a single count
    if (!this_cpu::is_in_isr())
    {
        bool success = true;
        while (success && (update > 0))
        {
            success = xSemaphoreGive(handle());
            update--;
        }
        return success;
    }
    else
    {
        BaseType_t needs_yield = false;
        bool success = true;
        while (success && (update > 0))
        {
            success = xSemaphoreGiveFromISR(handle(), &needs_yield);
            update--;
        }
        portYIELD_FROM_ISR(needs_yield);
        return success;
    }
}

thread* semaphore::get_mutex_holder() const
{
    if (!this_cpu::is_in_isr())
    {
        return reinterpret_cast<thread*>(xSemaphoreGetMutexHolder(handle()));
    }
    else
    {
        return reinterpret_cast<thread*>(xSemaphoreGetMutexHolderFromISR(handle()));
    }
}

#if (configUSE_COUNTING_SEMAPHORES == 1)

semaphore::semaphore(count_type max, count_type desired)
{
    // construction not allowed in ISR
    configASSERT(!this_cpu::is_in_isr());

    (void)xSemaphoreCreateCountingStatic(max, desired, this);
}

#endif // (configUSE_COUNTING_SEMAPHORES == 1)

binary_semaphore::binary_semaphore(count_type desired)
{
    // construction not allowed in ISR
    configASSERT(!this_cpu::is_in_isr());

    (void)xSemaphoreCreateBinaryStatic(this);
    give(desired);
}

} // namespace freertos
