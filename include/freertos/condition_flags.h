/**
 * @file      condition_flags.h
 * @brief     FreeRTOS event group API abstraction
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
#ifndef __FREERTOS_CONDITION_FLAGS_H_
#define __FREERTOS_CONDITION_FLAGS_H_

#include "freertos/tick_timer.h"

namespace freertos
{
    namespace native
    {
        struct EventGroupDef_t;
    }

    /// @brief  Thin type wrapper for condition flag value type.
    class cflag
    {
    public:
        using value_type = native::TickType_t;

        constexpr cflag()
                : value_(0)
        { }
        constexpr cflag(value_type value)
                : value_(value)
        { }
        operator value_type&()
        {
            return value_;
        }
        constexpr operator value_type() const
        {
            return value_;
        }

        static constexpr cflag max()
        {
            // the highest byte is reserved for OS flags
            return (1 << ((sizeof(value_type) - 1) * 8)) - 1;
        }
        static constexpr cflag min()
        {
            return 0;
        }

        /// @brief  The value returned by blocking function calls when the wait time expired
        ///         without any relevant flags being set.
        static constexpr cflag timeout()
        {
            return 0;
        }

    private:
        value_type value_;
    };

    /// @brief  This class is a lightweight condition variable, allows threads to block
    ///         until a combination of flags has been set. The key difference to @ref condition_variable
    ///         is that here the waiting side chooses the wait strategy:
    ///          1. whether to wait for a combination of flags (all) or one of many (any)
    ///          2. whether to consume the flag when receiving it (default) or not (shared)
    class condition_flags : private native::StaticEventGroup_t
    {
    public:
        /// @brief  Constructs a condition_flags statically.
        /// @remark Thread context callable
        condition_flags();

        /// @brief  Destroys the condition_flags, including freeing its dynamically allocated memory.
        /// @remark Thread context callable
        ~condition_flags();

        /// @brief  Reads the current flags status.
        /// @return The currently active flags
        /// @remark Thread and ISR context callable
        cflag get() const;

        /// @brief  Sets the provided flags in the condition.
        /// @param  flags: the flags to activate
        /// @remark Thread and ISR context callable
        void set(cflag flags);

        /// @brief  Removes the provided flags from the condition.
        /// @param  flags: the flags to deactivate
        /// @remark Thread and ISR context callable
        void clear(cflag flags);

        /// @brief  Blocks the current thread until any of the provided flags is raised.
        ///         When a flag unblocks the thread, it will be cleared.
        /// @param  flags: selection of flags to wait on
        /// @param  rel_time: duration to wait for the activation
        /// @return the raised flag(s) that caused the activation, or 0 if timed out
        /// @remark Thread context callable
        template<class Rep, class Period>
        cflag wait_any_for(cflag flags, const std::chrono::duration<Rep, Period>& rel_time)
        {
            return wait(flags, std::chrono::duration_cast<tick_timer::duration>(rel_time), true, false);
        }

        /// @brief  Blocks the current thread until any of the provided flags is raised.
        ///         When a flag unblocks the thread, it will be cleared.
        /// @param  flags: selection of flags to wait on
        /// @param  abs_time: deadline to wait for the activation
        /// @return the raised flag(s) that caused the activation, or 0 if timed out
        /// @remark Thread context callable
        template<class Clock, class Duration>
        cflag wait_any_until(cflag flags, const std::chrono::time_point<Clock, Duration>& abs_time)
        {
            return wait_any_for(flags, abs_time - Clock::now());
        }

        /// @brief  Blocks the current thread until all of the provided flags are raised.
        ///         When the thread is unblocked, the required flags will be cleared.
        /// @param  flags: combination of flags to wait on
        /// @param  rel_time: duration to wait for the activation
        /// @return the raised flag(s) that caused the activation, or 0 if timed out
        /// @remark Thread context callable
        template<class Rep, class Period>
        cflag wait_all_for(cflag flags, const std::chrono::duration<Rep, Period>& rel_time)
        {
            return wait(flags, std::chrono::duration_cast<tick_timer::duration>(rel_time), true, true);
        }

        /// @brief  Blocks the current thread until all of the provided flags are raised.
        ///         When the thread is unblocked, the required flags will be cleared.
        /// @param  flags: combination of flags to wait on
        /// @param  abs_time: deadline to wait for the activation
        /// @return the raised flag(s) that caused the activation, or 0 if timed out
        /// @remark Thread context callable
        template<class Clock, class Duration>
        cflag wait_all_until(cflag flags, const std::chrono::time_point<Clock, Duration>& abs_time)
        {
            return wait_all_for(flags, abs_time - Clock::now());
        }

        /// @brief  Blocks the current thread until any of the provided flags is raised.
        ///         Doesn't modify the flags upon activation.
        /// @param  flags: selection of flags to wait on
        /// @param  rel_time: duration to wait for the activation
        /// @return the raised flag(s) that caused the activation, or 0 if timed out
        /// @remark Thread context callable
        template<class Rep, class Period>
        cflag shared_wait_any_for(cflag flags, const std::chrono::duration<Rep, Period>& rel_time)
        {
            return wait(flags, std::chrono::duration_cast<tick_timer::duration>(rel_time), false, false);
        }

        /// @brief  Blocks the current thread until any of the provided flags is raised.
        ///         Doesn't modify the flags upon activation.
        /// @param  flags: selection of flags to wait on
        /// @param  abs_time: deadline to wait for the activation
        /// @return the raised flag(s) that caused the activation, or 0 if timed out
        /// @remark Thread context callable
        template<class Clock, class Duration>
        cflag shared_wait_any_until(cflag flags, const std::chrono::time_point<Clock, Duration>& abs_time)
        {
            return shared_wait_any_for(flags, abs_time - Clock::now());
        }

        /// @brief  Blocks the current thread until all of the provided flags are raised.
        ///         Doesn't modify the flags upon activation.
        /// @param  flags: combination of flags to wait on
        /// @param  rel_time: duration to wait for the activation
        /// @return the raised flag(s) that caused the activation, or 0 if timed out
        /// @remark Thread context callable
        template<class Rep, class Period>
        cflag shared_wait_all_for(cflag flags, const std::chrono::duration<Rep, Period>& rel_time)
        {
            return wait(flags, std::chrono::duration_cast<tick_timer::duration>(rel_time), false, true);
        }

        /// @brief  Blocks the current thread until all of the provided flags are raised.
        ///         Doesn't modify the flags upon activation.
        /// @param  flags: combination of flags to wait on
        /// @param  abs_time: deadline to wait for the activation
        /// @return the raised flag(s) that caused the activation, or 0 if timed out
        /// @remark Thread context callable
        template<class Clock, class Duration>
        cflag shared_wait_all_until(cflag flags, const std::chrono::time_point<Clock, Duration>& abs_time)
        {
            return shared_wait_all_for(flags, abs_time - Clock::now());
        }

    protected:
        inline native::EventGroupDef_t* handle() const
        {
            return reinterpret_cast<native::EventGroupDef_t*>(const_cast<condition_flags*>(this));
        }

        cflag wait(cflag flags, const tick_timer::duration& rel_time, bool exclusive, bool match_all);
    };
}

#endif // __FREERTOS_CONDITION_FLAGS_H_
