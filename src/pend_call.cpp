// SPDX-License-Identifier: MIT
#include "freertos/pend_call.hpp"
#include "freertos/cpu.hpp"
#if ESP_PLATFORM
#include <freertos/timers.h>
#else
#include <timers.h>
#endif

namespace freertos
{
#if (configUSE_TIMERS == 1)

bool pend_call(pend_function_2 func, void* arg1, std::uint32_t arg2, tick_timer::duration waittime)
{
    if (!this_cpu::is_in_isr())
    {
        return xTimerPendFunctionCall(func, arg1, arg2, to_ticks(waittime));
    }
    else
    {
        // cannot wait in ISR
        configASSERT(to_ticks(waittime) == 0);

        BaseType_t needs_yield = false;
        bool success = xTimerPendFunctionCallFromISR(func, arg1, arg2, &needs_yield);
        portYIELD_FROM_ISR(needs_yield);
        return success;
    }
}

static void pend_redirect_1(void* arg1, std::uint32_t arg2)
{
    auto func = reinterpret_cast<pend_function_1>(arg1);
    func(arg2);
}

bool pend_call(pend_function_1 func, std::uint32_t arg1, tick_timer::duration waittime)
{
    return pend_call(&pend_redirect_1, reinterpret_cast<void*>(func), arg1, waittime);
}

static void pend_redirect_0(void* arg1, std::uint32_t arg2)
{
    auto func = reinterpret_cast<pend_function_0>(arg1);
    func();
}

bool pend_call(pend_function_0 func, tick_timer::duration waittime)
{
    return pend_call(&pend_redirect_0, reinterpret_cast<void*>(func), 0, waittime);
}

#endif // (configUSE_TIMERS == 1)
} // namespace freertos
