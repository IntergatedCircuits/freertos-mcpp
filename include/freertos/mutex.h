/**
 * @file      mutex.h
 * @brief     FreeRTOS mutex API abstraction
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
#ifndef __FREERTOS_MUTEX_H_
#define __FREERTOS_MUTEX_H_

#include "freertos/semaphore.h"
#include "freertos/stdlib.h"

namespace freertos
{
    #if (configUSE_MUTEXES == 1)

        /// @brief  A class implementing std::mutex and std::timed_mutex API.
        class mutex : protected semaphore
        {
        public:
            /// @brief  Locks the mutex, blocks until the mutex is lockable.
            /// @remark Thread context callable
            inline void lock()
            {
                semaphore::acquire();
            }

            /// @brief  Attempts to lock the mutex.
            /// @return true if the mutex got locked, false if it's already locked
            /// @remark Thread context callable
            inline bool try_lock()
            {
                return semaphore::try_acquire();
            }

            /// @brief  Unlocks the mutex.
            /// @remark Thread context callable
            void unlock();

            /// @brief  Tries to lock the mutex within the given time duration.
            /// @param  rel_time: duration to wait for the mutex to become unlocked
            /// @return true if successful, false if the mutex is locked
            /// @remark Thread context callable
            template<class Rep, class Period>
            inline bool try_acquire_for(const std::chrono::duration<Rep, Period>& rel_time)
            {
                return semaphore::try_acquire_for(rel_time);
            }

            /// @brief  Tries to lock the mutex until the given deadline.
            /// @param  abs_time: deadline to wait until the mutex becomes unlocked
            /// @return true if successful, false if the mutex is locked
            /// @remark Thread context callable
            template<class Clock, class Duration>
            inline bool try_acquire_until(const std::chrono::time_point<Clock, Duration>& abs_time)
            {
                return semaphore::try_acquire_for(abs_time);
            }

            /// @brief  Function to observe the mutex's current locking thread.
            /// @return The mutex's current holder thread, or nullptr if the mutex is unlocked
            /// @remark Thread and ISR context callable
            inline thread *get_locking_thread() const
            {
                return semaphore::get_mutex_holder();
            }

            /// @brief  Constructs a mutex statically.
            mutex();

            #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

                /// @brief  Creates a mutex by allocating memory on the heap, and initializing it.
                /// @return Dynamically allocated mutex, or nullptr if allocation failed
                static mutex *create();

            #endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)

            // non-movable
            mutex(const mutex&&) = delete;
            mutex& operator=(const mutex&&) = delete;
        };


        /// @brief  A type alias since @ref mutex already implements std::timed_mutex API.
        using timed_mutex = mutex;


        #if (configUSE_RECURSIVE_MUTEXES == 1)

            /// @brief  A class implementing std::recursive_mutex and std::recursive_timed_mutex API.
            class recursive_mutex : protected semaphore
            {
            public:
                /// @brief  Locks the mutex if the current thread isn't locking it already,
                ///         blocks until the mutex is lockable.
                /// @remark Thread context callable
                inline void lock()
                {
                    semaphore::acquire();
                }

                /// @brief  Attempts to lock the mutex if the current thread isn't locking it already.
                /// @return true if the mutex got locked, false if it's already locked by another thread
                /// @remark Thread context callable
                inline bool try_lock()
                {
                    return semaphore::try_acquire();
                }

                /// @brief  Reduces the thread's lock count and unlocks the mutex when the lock count reaches zero.
                /// @remark Thread context callable
                void unlock();

                /// @brief  Tries to lock the mutex within the given time duration.
                /// @param  rel_time: duration to wait for the mutex to become unlocked
                /// @return true if successful, false if the mutex is locked
                /// @remark Thread context callable
                template<class Rep, class Period>
                inline bool try_acquire_for(const std::chrono::duration<Rep, Period>& rel_time)
                {
                    return semaphore::try_acquire_for(rel_time);
                }

                /// @brief  Tries to lock the mutex until the given deadline.
                /// @param  abs_time: deadline to wait until the mutex becomes unlocked
                /// @return true if successful, false if the mutex is locked
                /// @remark Thread context callable
                template<class Clock, class Duration>
                inline bool try_acquire_until(const std::chrono::time_point<Clock, Duration>& abs_time)
                {
                    return semaphore::try_acquire_for(abs_time);
                }

                /// @brief  Function to observe the mutex's current locking thread.
                /// @return The mutex's current holder thread, or nullptr if the mutex is unlocked
                /// @remark Thread and ISR context callable
                inline thread *get_locking_thread() const
                {
                    return semaphore::get_mutex_holder();
                }

                /// @brief  Constructs a recursive mutex statically.
                recursive_mutex();

                #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

                    /// @brief  Creates a mutex by allocating memory on the heap, and initializing it.
                    /// @return Dynamically allocated mutex, or nullptr if allocation failed
                    static recursive_mutex *create();

                #endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)

                // non-movable
                recursive_mutex(const recursive_mutex&&) = delete;
                recursive_mutex& operator=(const recursive_mutex&&) = delete;
            };


            /// @brief  A type alias since @ref recursive_mutex already implements std::recursive_timed_mutex API.
            using recursive_timed_mutex = recursive_mutex;


        #endif // (configUSE_RECURSIVE_MUTEXES == 1)

    #endif // (configUSE_MUTEXES == 1)
}

#endif // __FREERTOS_MUTEX_H_
