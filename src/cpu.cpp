// SPDX-License-Identifier: MIT
#include "freertos/cpu.hpp"
#if ESP_PLATFORM
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#else
#include <FreeRTOS.h>
#include <task.h>
#endif

namespace freertos
{
void cpu::critical_section::lock()
{
    if (!this_cpu::is_in_isr())
    {
#if ESP_PLATFORM
        taskENTER_CRITICAL(&restore_lock_);
#else
        taskENTER_CRITICAL();
#endif
    }
    else
    {
        restore_var() = std::uintptr_t(taskENTER_CRITICAL_FROM_ISR());
    }
}

void cpu::critical_section::unlock()
{
    if (!this_cpu::is_in_isr())
    {
#if ESP_PLATFORM
        taskEXIT_CRITICAL(&restore_lock_);
#else
        taskEXIT_CRITICAL();
#endif
    }
    else
    {
        taskEXIT_CRITICAL_FROM_ISR(restore_var());
    }
}

bool this_cpu::is_in_isr()
{
#if ESP_PLATFORM
    return xPortInIsrContext();
#else
    return xPortIsInsideInterrupt();
#endif
}

} // namespace freertos
