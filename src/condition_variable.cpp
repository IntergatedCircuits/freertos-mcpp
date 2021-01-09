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
    }
}
using namespace freertos;
using namespace freertos::native;

condition_variable_any::condition_variable_any()
    : queue_(), waiters_(0)
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

bool condition_variable_any::do_wait(const tick_timer::duration& rel_time, waiter_count_t *rx_waiters)
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
