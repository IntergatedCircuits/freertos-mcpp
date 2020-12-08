/**
 * @file      tick_timer.h
 * @brief     FreeRTOS tick timer related API abstraction
 * @author    Benedek Kupper
 *
 * Copyright (c) 2020 Benedek Kupper
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
#ifndef __FREERTOS_TICK_TIMER_H_
#define __FREERTOS_TICK_TIMER_H_

#include <cstdint>
#include <cstddef>
#include <chrono>

namespace freertos
{
    namespace native
    {
        #include "FreeRTOS.h"

        // these macros use native type casts, so need some redirection
        constexpr TickType_t INFINITE_DELAY = portMAX_DELAY;
        constexpr TickType_t TICK_RATE_HZ = configTICK_RATE_HZ;
    }

    /// @brief  A @ref TrivialClock class that wraps the FreeRTOS tick timer.
    class tick_timer
    {
    public:
        using rep                       = native::TickType_t;
        using period                    = std::ratio<1, native::TICK_RATE_HZ>;
        using duration                  = std::chrono::duration<rep, period>;
        using time_point                = std::chrono::time_point<tick_timer>;
        static constexpr bool is_steady = true;

        /// @brief  Wraps the current OS tick count into a clock time point.
        /// @return The current tick count as time_point
        /// @remark Thread and ISR context callable
        static time_point now();
    };

    /// @brief  Converts @ref tick_timer::duration to the underlying tick count.
    /// @param  duration: time duration in tick_timer scale
    /// @return Tick count
    constexpr tick_timer::rep to_ticks(const tick_timer::duration& duration)
    {
        return duration.count();
    }

    /// @brief  Converts @ref tick_timer::time_point to the underlying tick count.
    /// @param  time: time point from the start of the tick_timer
    /// @return Tick count
    constexpr tick_timer::rep to_ticks(const tick_timer::time_point& time)
    {
        return to_ticks(time.time_since_epoch());
    }

    /// @brief  Dedicated @ref tick_timer::duration expression that ensures
    ///         infinite wait time on an operation
    constexpr tick_timer::duration INFINITY { native::INFINITE_DELAY };
}

#endif // __FREERTOS_TICK_TIMER_H_
