/**
 * @file      cpu.h
 * @brief     FreeRTOS CPU related API abstraction
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
#ifndef __FREERTOS_CPU_H_
#define __FREERTOS_CPU_H_

#include "freertos/stdlib.h"

namespace freertos
{
    class cpu
    {
    public:
        /// @brief  A @ref BasicLockable class that prevents task and interrupt
        ///         context switches while locked.
        class critical_section
        {
        public:
            /// @brief  Locks the CPU, preventing thread and interrupt switches.
            void lock();

            /// @brief  Unlocks the CPU, allowing other interrupts and threads
            ///         to preempt the current execution context.
            void unlock();

            constexpr critical_section()
            {
            }

        private:
            std::uintptr_t restore_ = 0;
        };
    };

    namespace this_cpu
    {
        /// @brief  Determines if the current execution context is inside
        ///         an interrupt service routine.
        /// @note   The underlying port function (@ref xPortIsInsideInterrupt)
        ///         is only available for a subset of ports
        ///         but it could be extended to many targets
        ///         e.g. by checking if the current task's stack is being used
        /// @return true if the current execution context is ISR, false otherwise
        bool is_in_isr();
    }
}

#endif // __FREERTOS_CPU_H_
