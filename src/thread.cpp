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
#include "freertos/condition_flags.h"

namespace freertos
{
    namespace native
    {
        #include "task.h"
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
    #ifdef configTHREAD_EXIT_CONDITION_INDEX

        cpu::critical_section cs;
        const lock_guard<decltype(cs)> lock(cs);

        auto cond = get_exit_condition();
        // at task creation, this pointer is set to NULL
        if (cond != nullptr)
        {
            // if there is, signal it
            cond->set(cflag::max());
        }

    #endif // configTHREAD_EXIT_CONDITION_INDEX
}

#ifdef configTHREAD_EXIT_CONDITION_INDEX

    #if (configNUM_THREAD_LOCAL_STORAGE_POINTERS <= configTHREAD_EXIT_CONDITION_INDEX)
    #error "Thread exit condition storage must be allowed."
    #endif

    condition_flags *thread::get_exit_condition()
    {
        return reinterpret_cast<condition_flags*>(
                pvTaskGetThreadLocalStoragePointer(handle(),
                        configTHREAD_EXIT_CONDITION_INDEX));
    }

    bool thread::set_exit_condition(condition_flags *cond)
    {
        cpu::critical_section cs;
        const lock_guard<decltype(cs)> lock(cs);

        if (get_exit_condition() == nullptr)
        {
            // no prior semaphores registered, register this one
            vTaskSetThreadLocalStoragePointer(handle(),
                    configTHREAD_EXIT_CONDITION_INDEX, reinterpret_cast<void*>(cond));
            return true;
        }
        else
        {
            // thread has other observer already
            return false;
        }
    }

    bool thread::clear_exit_condition(condition_flags *cond)
    {
        cpu::critical_section cs;
        const lock_guard<decltype(cs)> lock(cs);

        if (get_exit_condition() == cond)
        {
            // this semaphore is registered, clear it
            vTaskSetThreadLocalStoragePointer(handle(),
                    configTHREAD_EXIT_CONDITION_INDEX, nullptr);
            return true;
        }
        else
        {
            // thread has other observer already
            return false;
        }
    }

#endif // configTHREAD_EXIT_CONDITION_INDEX

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
        state s;
        switch (eTaskGetState(handle()))
        {
            case eTaskState::eRunning:
                s = state::running;
                break;
            case eTaskState::eReady:
                s = state::ready;
                break;
            case eTaskState::eBlocked:
            case eTaskState::eSuspended:
                s = state::suspended;
                break;
            case eTaskState::eDeleted:
            case eTaskState::eInvalid:
            default:
                s = state::terminated;
                break;
        }
        return s;
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

#if (configUSE_TASK_NOTIFICATIONS == 1)

    bool thread::notifier::notify(unsigned action, notify_value value)
    {
        if (!this_cpu::is_in_isr())
        {
            return xTaskGenericNotify(handle(), value, static_cast<eNotifyAction>(action), &last_value_);
        }
        else
        {
            BaseType_t needs_yield = false;
            bool success = xTaskGenericNotifyFromISR(handle(), value, static_cast<eNotifyAction>(action), &last_value_, &needs_yield);
            portYIELD_FROM_ISR(needs_yield);
            return success;
        }
    }

    void thread::notifier::signal()
    {
        notify(eNoAction, 0);
    }

    bool thread::notifier::cancel_signal()
    {
        configASSERT(!this_cpu::is_in_isr());
        return xTaskNotifyStateClear(handle());
    }

    void thread::notifier::increment()
    {
        notify(eIncrement, 0);
    }

    void thread::notifier::set_flags(notify_value flags)
    {
        notify(eSetBits, flags);
    }

    notify_value thread::notifier::clear(notify_value flags)
    {
        configASSERT(!this_cpu::is_in_isr());
        return ulTaskNotifyValueClear(handle(), flags);
    }

    bool thread::notifier::try_set_value(notify_value new_value)
    {
        return notify(eSetValueWithoutOverwrite, new_value);
    }

    void thread::notifier::set_value(notify_value new_value)
    {
        notify(eSetValueWithOverwrite, new_value);
    }

    bool this_thread::wait_notification_for(const tick_timer::duration& rel_time,
            thread::notify_value *value,
            thread::notify_value clear_flags_before, thread::notify_value clear_flags_after)
    {
        configASSERT(!this_cpu::is_in_isr());
        return xTaskNotifyWait(clear_flags_before, clear_flags_after, value, to_ticks(rel_time));
    }

    thread::notify_value this_thread::try_acquire_notification_for(const tick_timer::duration& rel_time,
            bool acquire_single)
    {
        configASSERT(!this_cpu::is_in_isr());
        return ulTaskNotifyTake(!acquire_single, to_ticks(rel_time));
    }

#endif // (configUSE_TASK_NOTIFICATIONS == 1)

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

void this_thread::sleep_for(tick_timer::duration rel_time)
{
    configASSERT(!this_cpu::is_in_isr());
    configASSERT(scheduler::get_state() == scheduler::state::running);

    native::vTaskDelay(to_ticks(rel_time));
}
