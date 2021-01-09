/**
 * @file      condition_variable.h
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
#ifndef __FREERTOS_CONDITION_VARIABLE_H_
#define __FREERTOS_CONDITION_VARIABLE_H_

#include "freertos/mutex.h"

namespace freertos
{
    /// @brief  This object allows thread(s) to wait on a condition
    ///         until they are notified of its change. Matches the std::condition_variable_any API.
    class condition_variable_any
    {
    public:
        /// @brief  Constructs a condition_variable statically.
        /// @remark Thread context callable
        condition_variable_any();

        /// @brief  The destructor may only be called once no threads are waiting.
        /// @remark Thread context callable
        ~condition_variable_any();

        /// @brief  Unblocks the highest priority waiting thread (if any thread is waiting).
        /// @remark Thread and ISR context callable
        void notify_one();

        /// @brief  Unblocks all waiting threads.
        /// @remark Thread and ISR context callable
        void notify_all();

        /// @brief  Atomically unlocks @ref lock, and blocks the thread until a notification is received.
        ///         When unblocked, the @ref lock is reacquired again.
        /// @param  lock: the mutex to unlock while waiting on the condition_variable
        /// @remark Thread context callable
        template<class Lock>
        void wait(Lock& lock)
        {
            (void)wait_for(lock, infinity);
        }

        /// @brief  Atomically unlocks @ref lock, and blocks the thread
        ///         until the predicate is true after a notification is received.
        ///         When unblocked, the @ref lock is reacquired again.
        /// @param  lock: the mutex to unlock while waiting on the condition_variable
        /// @param  pred: the condition to wait on
        /// @remark Thread context callable
        template<class Lock, class Predicate>
        void wait(Lock& lock, Predicate pred)
        {
            while (!pred())
            {
                wait(lock);
            }
        }

        /// @brief  Atomically unlocks @ref lock, and blocks the thread
        ///         until a notification is received or until times out.
        ///         When unblocked, the @ref lock is reacquired again.
        /// @param  lock: the mutex to unlock while waiting on the condition_variable
        /// @param  rel_time: duration to wait for the notification
        /// @return timeout if timed out without any notification, or no_timeout otherwise
        /// @remark Thread context callable
        template<class Lock, class Rep, class Period>
        cv_status wait_for(Lock& lock, const std::chrono::duration<Rep, Period>& rel_time)
        {
            pre_wait();

            lock.unlock();

            waiter_count_t rx_waiters;
            bool success = do_wait(std::chrono::duration_cast<tick_timer::duration>(rel_time), &rx_waiters);

            lock.lock();

            return post_wait(success, rx_waiters);
        }

        /// @brief  Atomically unlocks @ref lock, and blocks the thread
        ///         until a notification is received or until times out.
        ///         When unblocked, the @ref lock is reacquired again.
        /// @param  lock: the mutex to unlock while waiting on the condition_variable
        /// @param  abs_time: deadline to wait for the notification
        /// @return timeout if timed out without any notification, or no_timeout otherwise
        /// @remark Thread context callable
        template<class Lock, class Clock, class Duration>
        cv_status wait_until(Lock& lock, const std::chrono::time_point<Clock, Duration>& abs_time)
        {
            return wait_for(lock, abs_time - Clock::now());
        }

        /// @brief  Atomically unlocks @ref lock, and blocks the thread
        ///         until the predicate is true after a notification is received,
        ///         or until times out. When unblocked, the @ref lock is reacquired again.
        /// @param  lock: the mutex to unlock while waiting on the condition_variable
        /// @param  abs_time: deadline to wait for the notification
        /// @param  pred: the condition to wait on
        /// @return false if the predicate still evaluates to false after the timeout expired,
        ///         otherwise true.
        /// @remark Thread context callable
        template<class Lock, class Clock, class Duration, class Pred>
        bool wait_until(Lock& lock, const std::chrono::time_point<Clock, Duration>& abs_time, Pred pred)
        {
            while (!pred())
            {
                if (wait_until(lock, abs_time) == cv_status::timeout)
                {
                    return pred();
                }
            }
            return true;
        }

        /// @brief  Atomically unlocks @ref lock, and blocks the thread
        ///         until the predicate is true after a notification is received,
        ///         or until times out. When unblocked, the @ref lock is reacquired again.
        /// @param  lock: the mutex to unlock while waiting on the condition_variable
        /// @param  rel_time: duration to wait for the notification
        /// @param  pred: the condition to wait on
        /// @return false if the predicate still evaluates to false after the timeout expired,
        ///         otherwise true.
        /// @remark Thread context callable
        template<class Lock, class Rep, class Period, class Pred>
        bool wait_for(Lock& lock, const std::chrono::duration<Rep, Period>& rel_time, Pred pred)
        {
            return wait_until(lock, tick_timer::now() + rel_time, std::move(pred));
        }

    private:
        using waiter_count_t = native::UBaseType_t;

        shallow_copy_queue<waiter_count_t, 1> queue_;
        waiter_count_t waiters_;

        // non-copyable
        condition_variable_any(const condition_variable_any&) = delete;
        condition_variable_any& operator=(const condition_variable_any&) = delete;

        void notify(waiter_count_t waiters);

        void pre_wait();
        bool do_wait(const tick_timer::duration& rel_time, waiter_count_t *rx_waiters);
        cv_status post_wait(bool wait_success, waiter_count_t rx_waiters);
    };
}

#endif // __FREERTOS_CONDITION_VARIABLE_H_
