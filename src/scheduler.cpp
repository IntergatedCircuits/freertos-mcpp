/**
 * @file      scheduler.cpp
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
#include "freertos/scheduler.h"

namespace freertos
{
    namespace native
    {
        #include "task.h"

        static_assert(taskSCHEDULER_SUSPENDED   == static_cast<BaseType_t>(scheduler::state::suspended),
                "These values must match!");
        static_assert(taskSCHEDULER_NOT_STARTED == static_cast<BaseType_t>(scheduler::state::uninitialized),
                "These values must match!");
        static_assert(taskSCHEDULER_RUNNING     == static_cast<BaseType_t>(scheduler::state::running),
                "These values must match!");
    }
}
using namespace freertos;
using namespace freertos::native;

void scheduler::start()
{
    // this call doesn't return
    vTaskStartScheduler();
}

size_t scheduler::get_threads_count()
{
    return uxTaskGetNumberOfTasks();
}

scheduler::state scheduler::get_state()
{
    return static_cast<scheduler::state>(xTaskGetSchedulerState());
}

void scheduler::critical_section::lock()
{
    vTaskSuspendAll();
}

void scheduler::critical_section::unlock()
{
    if (xTaskResumeAll())
    {
        // a context switch had already happened
    }
}
