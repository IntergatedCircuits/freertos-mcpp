/**
 * @file      condition_variable.cpp
 * @brief     FreeRTOS condition_variable implementation
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
#include "freertos/condition_variable.h"
#include "freertos/cpu.h"

namespace freertos
{
    namespace native
    {
        #include "task.h"
        #include "event_groups.h"
    }
}
using namespace freertos;
using namespace freertos::native;

condition_flags::condition_flags()
{
    configASSERT(!this_cpu::is_in_isr());
    xEventGroupCreateStatic(this);
}

condition_flags::~condition_flags()
{
    configASSERT(!this_cpu::is_in_isr());
    vEventGroupDelete(handle());
}

cflag condition_flags::get() const
{
    if (!this_cpu::is_in_isr())
    {
        return xEventGroupGetBits(handle());
    }
    else
    {
        return xEventGroupGetBitsFromISR(handle());
    }
}

void condition_flags::set(cflag flags)
{
    if (!this_cpu::is_in_isr())
    {
        (void)xEventGroupSetBits(handle(), flags);
    }
    else
    {
        #if (configUSE_TIMERS == 1)

            BaseType_t needs_yield = false;
            (void)xEventGroupSetBitsFromISR(handle(), flags, &needs_yield);
            portYIELD_FROM_ISR(needs_yield);

        #else

            configASSERT(false);

        #endif // (configUSE_TIMERS == 1)
    }
}

void condition_flags::clear(cflag flags)
{
    if (!this_cpu::is_in_isr())
    {
        (void)xEventGroupClearBits(handle(), flags);
    }
    else
    {
        #if (configUSE_TIMERS == 1)

            (void)xEventGroupClearBitsFromISR(handle(), flags);

        #else

            configASSERT(false);

        #endif // (configUSE_TIMERS == 1)
    }
}

cflag condition_flags::wait(cflag flags, const tick_timer::duration& rel_time, bool exclusive, bool match_all)
{
    configASSERT(!this_cpu::is_in_isr());
    cflag setflags = xEventGroupWaitBits(handle(), flags, exclusive, match_all, to_ticks(rel_time));
    // only return the flags that are relevant to the wait operation
    return flags & setflags;
}

#if (configUSE_MUTEXES == 1)

    condition_variable::condition_variable()
        : queue_(), waiters_(0)
    {
        // construction not allowed in ISR
        configASSERT(!this_cpu::is_in_isr());
    }

    condition_variable::~condition_variable()
    {
        configASSERT(waiters_ == 0);
    }

    void condition_variable::notify(waiter_count_t waiters)
    {
        // leaves a previous unconsumed message in the queue,
        // possibly blocking the effect of the current call
        queue_.push_front(waiters);
    }

    void condition_variable::notify_one()
    {
        auto waiters = waiters_;
        if (waiters > 0)
        {
            notify(1);
        }
    }

    void condition_variable::notify_all()
    {
        auto waiters = waiters_;
        if (waiters > 0)
        {
            notify(waiters);
        }
    }

    cv_status condition_variable::wait_for(unique_lock<mutex>& lock, const tick_timer::duration& rel_time)
    {
        configASSERT(!this_cpu::is_in_isr());
        configASSERT(lock.owns_lock());

        // add thread to waiting list (while still locking)
        waiters_++;

        lock.unlock();

        // only the wait for the CV signal happens while unlocked
        waiter_count_t rx_waiters;
        bool success = queue_.pop_front(&rx_waiters, rel_time);

        lock.lock();

        // remove thread from waiting list (when again blocking)
        waiters_--;

        if (success)
        {
            auto rem_waiters = rx_waiters - 1;
            rem_waiters = std::min(rem_waiters, waiters_);

            // chain the notification if necessary (from notify_all)
            if (rem_waiters > 0)
            {
                // overwrite the previous message
                // in case a notify arrived in the meantime
                queue_.replace(rem_waiters);
            }
        }

        return success ? cv_status::no_timeout : cv_status::timeout;
    }

#endif // (configUSE_MUTEXES == 1)
