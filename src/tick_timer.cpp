// SPDX-License-Identifier: MIT
#include "freertos/tick_timer.hpp"
#include "freertos/cpu.hpp"
#if ESP_PLATFORM
#include <freertos/task.h>
#else
#include <task.h>
#endif

namespace freertos
{
tick_timer::time_point tick_timer::now()
{
    rep ticks;
    if (!this_cpu::is_in_isr())
    {
        ticks = xTaskGetTickCount();
    }
    else
    {
        ticks = xTaskGetTickCountFromISR();
    }

    return time_point(duration(ticks));
}
} // namespace freertos
