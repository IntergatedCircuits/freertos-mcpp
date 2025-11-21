// SPDX-License-Identifier: MIT
#include "freertos/thread.hpp"
#include "freertos/cpu.hpp"
#include "freertos/event_group.hpp"
#include "freertos/scheduler.hpp"
#if ESP_PLATFORM
#include <freertos/task.h>
#else
#include <task.h>
#endif

#if configTASK_RETURN_ADDRESS != vTaskExitHandler
#error "Ensure that FreeRTOSConfig.h contains: #define configTASK_RETURN_ADDRESS vTaskExitHandler"
#endif

/// @brief  The thread's execution enters this function when the thread function returns.
extern "C" void vTaskExitHandler(void)
{
    freertos::thread* t = freertos::thread::get_current();
    // call destructor
    t->~thread();
}

namespace freertos
{
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

condition_flags* thread::get_exit_condition() const
{
    return reinterpret_cast<condition_flags*>(
        pvTaskGetThreadLocalStoragePointer(handle(), configTHREAD_EXIT_CONDITION_INDEX));
}

void thread::set_exit_condition(condition_flags* cond)
{
    configASSERT(nullptr == get_exit_condition()); // cannot have multiple threads joining
    vTaskSetThreadLocalStoragePointer(handle(), configTHREAD_EXIT_CONDITION_INDEX,
                                      reinterpret_cast<void*>(cond));
}

bool thread::joinable() const
{
    return (get_state() != state::terminated) && (nullptr == get_exit_condition());
}

void thread::join()
{
    configASSERT(joinable()); // else invalid_argument
    configASSERT(this->get_id() !=
                 freertos::this_thread::get_id()); // else resource_deadlock_would_occur

    condition_flags exit_cond;
    set_exit_condition(&exit_cond);

    // wait for signal from thread exit
    exit_cond.shared_wait_any_for(cflag::max(), infinity);

    // signal received, thread is deleted, return
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
    return uxTaskGetTaskNumber(handle());
#else
    return id(this);
#endif // (configUSE_TRACE_FACILITY == 1)
}

const char* thread::get_name()
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

thread* make_thread(thread::function func, void* param, size_t stacksize, thread::priority prio,
                    const char* name)
{
    thread* t = nullptr;
    bool res = xTaskCreate(func, name, stacksize / sizeof(StackType_t), param, prio,
                           reinterpret_cast<TaskHandle_t*>(&t));
    configASSERT(res);
    return t;
}

#endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)

thread::thread(StackType_t* pstack, std::uint32_t stack_size, function func, void* param,
               priority prio, const char* name)
{
    (void)xTaskCreateStatic(func, name, stack_size, param, prio, pstack, this);
}

#if (configUSE_TASK_NOTIFICATIONS == 1)

bool thread::notifier::notify(unsigned action, notify_value value)
{
    if (!this_cpu::is_in_isr())
    {
        return
#if (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)
            xTaskNotifyAndQueryIndexed(handle(), index_
#else
            xTaskNotifyAndQuery(handle()
#endif // (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)
                                       ,
                                       value, static_cast<eNotifyAction>(action), &last_value_);
    }
    else
    {
        BaseType_t needs_yield = false;
        bool success =
#if (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)
            xTaskNotifyAndQueryIndexedFromISR(handle(), index_
#else
            xTaskNotifyAndQueryFromISR(handle()
#endif // (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)
                                              ,
                                              value, static_cast<eNotifyAction>(action),
                                              &last_value_, &needs_yield);
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
    return
#if (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)
        xTaskNotifyStateClearIndexed(handle(), index_
#else
        xTaskNotifyStateClear(handle()
#endif // (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)
        );
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
    return
#if (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)
        ulTaskNotifyValueClearIndexed(handle(), index_
#else
        ulTaskNotifyValueClear(handle()
#endif // (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)
                                      ,
                                      flags);
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
                                        thread::notify_value* value,
                                        thread::notify_value clear_flags_before,
                                        thread::notify_value clear_flags_after)
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

#if (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)

bool this_thread::wait_notification_for(thread::notifier::index_type index,
                                        const tick_timer::duration& rel_time,
                                        thread::notify_value* value,
                                        thread::notify_value clear_flags_before,
                                        thread::notify_value clear_flags_after)
{
    configASSERT(!this_cpu::is_in_isr());
    return xTaskNotifyWaitIndexed(index, clear_flags_before, clear_flags_after, value,
                                  to_ticks(rel_time));
}

thread::notify_value this_thread::try_acquire_notification_for(thread::notifier::index_type index,
                                                               const tick_timer::duration& rel_time,
                                                               bool acquire_single)
{
    configASSERT(!this_cpu::is_in_isr());
    return ulTaskNotifyTakeIndexed(index, !acquire_single, to_ticks(rel_time));
}

#endif // (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)

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

    vTaskDelay(to_ticks(rel_time));
}

void this_thread::terminate()
{
    thread::get_current()->~thread();
}

} // namespace freertos
