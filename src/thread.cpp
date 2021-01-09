/**
 * @file      thread.cpp
 * @brief     FreeRTOS thread (task in FreeRTOS naming) API abstraction
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
#include "freertos/thread.h"
#include "freertos/cpu.h"
#include "freertos/scheduler.h"
#include "freertos/semaphore.h"

namespace freertos
{
    namespace native
    {
        #include "task.h"

        static_assert(eTaskState::eRunning   == static_cast<eTaskState>(thread::state::running),
                "These values must match!");
        static_assert(eTaskState::eReady     == static_cast<eTaskState>(thread::state::ready),
                "These values must match!");
        static_assert(eTaskState::eBlocked   == static_cast<eTaskState>(thread::state::blocked),
                "These values must match!");
        static_assert(eTaskState::eSuspended == static_cast<eTaskState>(thread::state::suspended),
                "These values must match!");
        static_assert(eTaskState::eDeleted   == static_cast<eTaskState>(thread::state::deleted),
                "These values must match!");
        static_assert(eTaskState::eInvalid   == static_cast<eTaskState>(thread::state::invalid),
                "These values must match!");
    }
}
using namespace freertos;
using namespace freertos::native;

#if configTASK_RETURN_ADDRESS != vTaskExitHandler
#error "Ensure that FreeRTOSConfig.h contains: #define configTASK_RETURN_ADDRESS vTaskExitHandler"
#endif

/// @brief  The thread's execution enters this function when the thread function returns.
extern "C" void vTaskExitHandler(void)
{
    thread *t = thread::get_current();
    // call destructor
    t->~thread();
}

thread::~thread()
{
    signal_exit();

    vTaskDelete(handle());
}

void thread::signal_exit()
{
    #ifdef configTHREAD_EXIT_SEMAPHORE_INDEX

        cpu::critical_section cs;
        const lock_guard<decltype(cs)> lock(cs);

        auto sem = get_exit_semaphore();
        // at task creation, this pointer is set to NULL
        if (sem != nullptr)
        {
            // if there is, signal it
            sem->release();
        }

    #endif // configTHREAD_EXIT_SEMAPHORE_INDEX
}

#ifdef configTHREAD_EXIT_SEMAPHORE_INDEX

    #if (configNUM_THREAD_LOCAL_STORAGE_POINTERS <= configTHREAD_EXIT_SEMAPHORE_INDEX)
    #error "Thread exit semaphore storage must be allowed."
    #endif

    binary_semaphore *thread::get_exit_semaphore()
    {
        return reinterpret_cast<binary_semaphore*>(
                pvTaskGetThreadLocalStoragePointer(handle(),
                        configTHREAD_EXIT_SEMAPHORE_INDEX));
    }

    bool thread::set_exit_semaphore(binary_semaphore *sem)
    {
        cpu::critical_section cs;
        const lock_guard<decltype(cs)> lock(cs);

        if (get_exit_semaphore() == nullptr)
        {
            // no prior semaphores registered, register this one
            vTaskSetThreadLocalStoragePointer(handle(),
                    configTHREAD_EXIT_SEMAPHORE_INDEX, reinterpret_cast<void*>(sem));
            return true;
        }
        else
        {
            // thread has other observer already
            return false;
        }
    }

    bool thread::clear_exit_semaphore(binary_semaphore *sem)
    {
        cpu::critical_section cs;
        const lock_guard<decltype(cs)> lock(cs);

        if (get_exit_semaphore() == sem)
        {
            // this semaphore is registered, clear it
            vTaskSetThreadLocalStoragePointer(handle(),
                    configTHREAD_EXIT_SEMAPHORE_INDEX, nullptr);
            return true;
        }
        else
        {
            // thread has other observer already
            return false;
        }
    }

#endif // configTHREAD_EXIT_SEMAPHORE_INDEX

void thread::suspend()
{
    configASSERT(!this_cpu::is_in_isr());
    {
        vTaskSuspend(handle());
    }
}

void thread::resume()
{
    if (!this_cpu::is_in_isr())
    {
        vTaskResume(handle());
    }
    else
    {
        BaseType_t needs_yield = xTaskResumeFromISR(handle());
        portYIELD_FROM_ISR(needs_yield);
    }
}

thread::priority thread::get_priority() const
{
    if (!this_cpu::is_in_isr())
    {
        return uxTaskPriorityGet(handle());
    }
    else
    {
        return uxTaskPriorityGetFromISR(handle());
    }
}

void thread::set_priority(priority prio)
{
    configASSERT(!this_cpu::is_in_isr());
    {
        vTaskPrioritySet(handle(), prio);
    }
}

thread::id thread::get_id() const
{
#if (configUSE_TRACE_FACILITY == 1)
    return priv::uxTaskGetTaskNumber(handle());
#else
    return id(this);
#endif // (configUSE_TRACE_FACILITY == 1)
}

const char *thread::get_name()
{
    return const_cast<const char*>(pcTaskGetName(handle()));
}

thread::state thread::get_state() const
{
    configASSERT(!this_cpu::is_in_isr());
    {
        return static_cast<state>(eTaskGetState(handle()));
    }
}

thread* thread::get_current()
{
    configASSERT(!this_cpu::is_in_isr());
    configASSERT(scheduler::get_state() != scheduler::state::uninitialized);

    return reinterpret_cast<thread*>(xTaskGetCurrentTaskHandle());
}

#if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

thread* thread::create(function func, void *param,
        size_t stacksize, priority prio, const char *name)
{
    thread* t = nullptr;
    bool res = xTaskCreate(func, name,
            stacksize / sizeof(StackType_t),
            param, prio, reinterpret_cast<TaskHandle_t*>(&t));
    configASSERT(res);
    return t;
}

#endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)

thread::thread(StackType_t *pstack ,std::uint32_t stack_size,
        function func, void *param, priority prio, const char *name)
{
    (void)xTaskCreateStatic(func, name, stack_size,
            param, prio, pstack, this);
}

void this_thread::yield()
{
    configASSERT(!this_cpu::is_in_isr());
    configASSERT(scheduler::get_state() != scheduler::state::uninitialized);

    taskYIELD();
}

thread::id this_thread::get_id()
{
    return thread::get_current()->get_id();
}

void this_thread::sleep_for(const tick_timer::duration& rel_time)
{
    configASSERT(!this_cpu::is_in_isr());
    configASSERT(scheduler::get_state() == scheduler::state::running);

    native::vTaskDelay(to_ticks(rel_time));
}
