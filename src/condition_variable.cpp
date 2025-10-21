// SPDX-License-Identifier: MIT
#include "freertos/condition_variable.hpp"
#include "freertos/cpu.hpp"
#if ESP_PLATFORM
#include <freertos/task.h>
#else
#include <task.h>
#endif

namespace freertos
{

condition_variable_any::condition_variable_any() : queue_(), waiters_(0)
{
    // construction not allowed in ISR
    configASSERT(!this_cpu::is_in_isr());
}

condition_variable_any::~condition_variable_any()
{
    configASSERT(waiters_ == 0);
}

void condition_variable_any::notify(waiter_count_t waiters)
{
    // leaves a previous unconsumed message in the queue,
    // possibly blocking the effect of the current call
    queue_.push_front(waiters);
}

void condition_variable_any::notify_one()
{
    auto waiters = waiters_;
    if (waiters > 0)
    {
        notify(1);
    }
}

void condition_variable_any::notify_all()
{
    auto waiters = waiters_;
    if (waiters > 0)
    {
        notify(waiters);
    }
}

void condition_variable_any::pre_wait()
{
    configASSERT(!this_cpu::is_in_isr());

    // add thread to waiting list (while still locking)
    waiters_++;
}

bool condition_variable_any::do_wait(const tick_timer::duration& rel_time,
                                     waiter_count_t* rx_waiters)
{
    return queue_.pop_front(rx_waiters, rel_time);
}

cv_status condition_variable_any::post_wait(bool wait_success, waiter_count_t rx_waiters)
{
    // remove thread from waiting list (when again blocking)
    waiters_--;

    if (wait_success)
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

    return wait_success ? cv_status::no_timeout : cv_status::timeout;
}
} // namespace freertos
