/**
 * @file      pend_call.cpp
 * @brief     FreeRTOS pend call API abstraction
 * @author    Benedek Kupper
 *
 * Copyright (c) 2021 Benedek Kupper
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "freertos/pend_call.h"
#include "freertos/cpu.h"

namespace freertos
{
    namespace native
    {
        #include "timers.h"
    }
}
using namespace freertos;
using namespace freertos::native;

#if (configUSE_TIMERS == 1)

    bool freertos::pend_call(pend_function_2 func, void *arg1, std::uint32_t arg2, tick_timer::duration waittime)
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

    static void pend_redirect_1(void *arg1, std::uint32_t arg2)
    {
        auto func = reinterpret_cast<pend_function_1>(arg1);
        func(arg2);
    }

    bool freertos::pend_call(pend_function_1 func, std::uint32_t arg1, tick_timer::duration waittime)
    {
        return pend_call(&pend_redirect_1, reinterpret_cast<void*>(func), arg1, waittime);
    }

    static void pend_redirect_0(void *arg1, std::uint32_t arg2)
    {
        auto func = reinterpret_cast<pend_function_0>(arg1);
        func();
    }

    bool freertos::pend_call(pend_function_0 func, tick_timer::duration waittime)
    {
        return pend_call(&pend_redirect_0, reinterpret_cast<void*>(func), 0, waittime);
    }

#endif // (configUSE_TIMERS == 1)
