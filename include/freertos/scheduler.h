/**
 * @file      scheduler.h
 * @brief     FreeRTOS scheduler control API
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
#ifndef __FREERTOS_SCHEDULER_H_
#define __FREERTOS_SCHEDULER_H_

#include "freertos/thread.h"
#include "freertos/stdlib.h"

namespace freertos
{
    /// @brief  Static class that allows FreeRTOS scheduler control.
    class scheduler
    {
    public:
        /// @brief  Possible operating states of the scheduler.
        enum class state : native::BaseType_t
        {
            suspended = 0,
            uninitialized = 1,
            running = 2
        };

        /// @brief  Starts the scheduler. This function doesn't return.
        /// @remark Thread context callable
        [[noreturn]] static void start();

        /// @brief  Reads the current state of the scheduler.
        /// @return The current state of the scheduler
        /// @remark Thread context callable
        static state get_state();

        /// @brief  Reads the total number of existing threads.
        /// @return The number of threads
        /// @remark Thread context callable
        static size_t get_threads_count();

        /// @brief  @ref Lockable critical section that manipulates the scheduler's state.
        class critical_section
        {
        public:
            /// @brief  Suspends the scheduler, allowing the caller thread to operate without
            ///         the possibility of context switches. Blocking operations are forbidden
            ///         while the scheduler is suspended.
            /// @remark Thread context callable
            void lock();

            /// @brief  Resumes the scheduler, allowing thread context switches.
            /// @remark Thread context callable
            void unlock();

            constexpr critical_section()
            {
            }
        };

    private:
        scheduler();
    };
}

#endif // __FREERTOS_SCHEDULER_H_
