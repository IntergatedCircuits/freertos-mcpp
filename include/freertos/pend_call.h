/**
 * @file      pend_call.h
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
#ifndef __FREERTOS_PEND_CALL_H_
#define __FREERTOS_PEND_CALL_H_

#include "freertos/tick_timer.h"

namespace freertos
{
    #if (configUSE_TIMERS == 1)

        using pend_function_2 = void (*)(void*, std::uint32_t);

        /// @brief  Schedules a single function call to be executed in the timer service thread.
        /// @param  func:      the function to execute in the timer service thread context
        /// @param  arg1:      opaque parameter to pass to the function
        /// @param  arg2:      opaque parameter to pass to the function
        /// @param  waittime:  duration to wait for the timer queue to accept this request
        /// @return true if the request is accepted, false if the timer queue is full
        bool pend_call(pend_function_2 func, void *arg1, std::uint32_t arg2,
                tick_timer::duration waittime = tick_timer::duration(0));

        template<typename T1, typename T2>
        inline typename std::enable_if<(sizeof(T1) == sizeof(void*)) && (sizeof(T2) == sizeof(std::uint32_t)),
        bool>::type pend_call(void (*func)(T1, T2), T1 arg1, T2 arg2,
                tick_timer::duration waittime = tick_timer::duration(0))
        {
            const auto pend_caller = static_cast<bool (*)(pend_function_2, void *, std::uint32_t, tick_timer::duration)>(&pend_call);
            return pend_caller(reinterpret_cast<pend_function_2>(func),
                    reinterpret_cast<void*>(*reinterpret_cast<std::uintptr_t*>(&arg1)),
                    *reinterpret_cast<std::uint32_t*>(&arg2),
                    waittime);
        }

        template<typename T1, typename T2>
        inline typename std::enable_if<(sizeof(T2) == sizeof(std::uint32_t)),
        bool>::type pend_call(T1& obj, void (T1::*member_func)(T2), T2 arg2,
                tick_timer::duration waittime = tick_timer::duration(0))
        {
            const auto pend_caller = static_cast<bool (*)(pend_function_2, void *, std::uint32_t, tick_timer::duration)>(&pend_call);
            return pend_caller(reinterpret_cast<pend_function_2>(member_func),
                    reinterpret_cast<void*>(&obj),
                    *reinterpret_cast<std::uint32_t*>(&arg2),
                    waittime);
        }

        using pend_function_1 = void (*)(std::uint32_t);

        /// @brief  Schedules a single function call to be executed in the timer service thread.
        /// @param  func:      the function to execute in the timer service thread context
        /// @param  arg1:      opaque parameter to pass to the function
        /// @param  waittime:  duration to wait for the timer queue to accept this request
        /// @return true if the request is accepted, false if the timer queue is full
        bool pend_call(pend_function_1 func, std::uint32_t arg1,
                tick_timer::duration waittime = tick_timer::duration(0));

        template<typename T1>
        inline typename std::enable_if<(sizeof(T1) == sizeof(std::uint32_t)),
        bool>::type pend_call(void (*func)(T1), T1 arg1,
                tick_timer::duration waittime = tick_timer::duration(0))
        {
            const auto pend_caller = static_cast<bool (*)(pend_function_1, std::uint32_t, tick_timer::duration)>(&pend_call);
            return pend_caller(reinterpret_cast<pend_function_1>(func),
                    *reinterpret_cast<std::uintptr_t*>(&arg1),
                    waittime);
        }

        template<typename T1>
        inline typename std::enable_if<(sizeof(T1*) == sizeof(std::uint32_t)),
        bool>::type pend_call(T1& obj, void (T1::*member_func)(),
                tick_timer::duration waittime = tick_timer::duration(0))
        {
            const auto pend_caller = static_cast<bool (*)(pend_function_1, std::uint32_t, tick_timer::duration)>(&pend_call);
            return pend_caller(reinterpret_cast<pend_function_1>(member_func),
                    reinterpret_cast<std::uintptr_t>(&obj),
                    waittime);
        }

        using pend_function_0 = void (*)();

        /// @brief  Schedules a single function call to be executed in the timer service thread.
        /// @param  func:      the function to execute in the timer service thread context
        /// @param  waittime:  duration to wait for the timer queue to accept this request
        /// @return true if the request is accepted, false if the timer queue is full
        bool pend_call(pend_function_0 func, tick_timer::duration waittime = tick_timer::duration(0));

    #endif // (configUSE_TIMERS == 1)
}

#endif // __FREERTOS_PEND_CALL_H_
