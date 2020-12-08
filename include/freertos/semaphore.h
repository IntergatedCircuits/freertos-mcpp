/**
 * @file      semaphore.h
 * @brief     FreeRTOS semaphore API abstraction
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
#ifndef __FREERTOS_SEMAPHORE_H_
#define __FREERTOS_SEMAPHORE_H_

#include "freertos/queue.h"

namespace freertos
{
    class thread;

    /// @brief  An abstract base class for semaphores. Implements std::counting_semaphore API.
    ///         An important distinction is that this class is non-copyable and non-movable,
    ///         since FreeRTOS mutexes inherit from semaphores (that is, they're both null-size queues).
    class semaphore : protected queue
    {
    public:
        using count_type = queue::size_type;

        /// @brief  Waits indefinitely until the semaphore is available, then takes it.
        /// @remark Thread context callable
        inline void acquire()
        {
            (void)take(INFINITY);
        }

        /// @brief  Tries to take the semaphore if it is available.
        /// @return true if successful, false if the semaphore is unavailable
        /// @remark Thread and ISR context callable
        inline bool try_acquire()
        {
            return take(tick_timer::duration(0));
        }

        /// @brief  Tries to take the semaphore within the given time duration.
        /// @param  rel_time: duration to wait for the semaphore to become available
        /// @return true if successful, false if the semaphore is unavailable
        /// @remark Thread context callable
        template<class Rep, class Period>
        inline bool try_acquire_for(const std::chrono::duration<Rep, Period>& rel_time)
        {
            return take(std::chrono::duration_cast<tick_timer::duration>(rel_time));
        }

        /// @brief  Tries to take the semaphore until the given deadline.
        /// @param  abs_time: deadline to wait until the semaphore becomes available
        /// @return true if successful, false if the semaphore is unavailable
        /// @remark Thread context callable
        template<class Clock, class Duration>
        inline bool try_acquire_until(const std::chrono::time_point<Clock, Duration>& abs_time)
        {
            return try_acquire_for(abs_time - Clock::now());
        }

        /// @brief  Makes the semaphore available a given number of times.
        /// @param  update: the number of available signals to send
        /// @remark Thread and ISR context callable
        inline void release(count_type update = 1)
        {
            (void)give(update);
        }

        /// @brief  Function to observe the semaphore's current acquirable count.
        /// @return The semaphore's acquirable count
        /// @remark Thread and ISR context callable
        inline count_type get_count() const
        {
            return queue::size();
        }

        #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

            /// @brief  Empty delete operator, since the destructor does the memory freeing
            ///         if the object was dynamically allocated
            void operator delete(void * p)
            {
            }

        #endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)

    protected:
        // actual FreeRTOS operations
        bool take(tick_timer::duration timeout);
        bool give(count_type update);

        // empty constructor is used by mutexes, since the underlying API create calls differ
        semaphore()
        {
        }

        #if (configUSE_COUNTING_SEMAPHORES == 1)

            // used to create counting_semaphore
            semaphore(count_type max, count_type desired);

            #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

                static semaphore* create_counting(count_type max, count_type desired);

            #endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)

        #endif // (configUSE_COUNTING_SEMAPHORES == 1)

        // only for mutexes
        thread *get_mutex_holder() const;
    };

    #if (configUSE_COUNTING_SEMAPHORES == 1)

        /// @brief  Counting semaphore class.
        template<semaphore::count_type MAX_VALUE = 1>
        class counting_semaphore : public semaphore
        {
        public:
            /// @brief  Maximum value of the internal counter.
            static constexpr count_type max()
            {
                return MAX_VALUE;
            }

            /// @brief  Constructs a counting semaphore statically.
            counting_semaphore(count_type desired = 0)
                : semaphore(MAX_VALUE, desired)
            {
            }

            #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

                /// @brief  Creates a counting semaphore by allocating memory on the heap, and initializing it.
                /// @return Dynamically allocated counting semaphore, or nullptr if allocation failed
                static counting_semaphore *create(count_type desired = 0)
                {
                    return reinterpret_cast<counting_semaphore*>(create_counting(MAX_VALUE, desired));
                }

            #endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)
        };

    #endif // (configUSE_COUNTING_SEMAPHORES == 1)


    /// @brief  Binary semaphore class.
    //using binary_semaphore = std::counting_semaphore<1>;
    class binary_semaphore : public semaphore
    {
    public:
        /// @brief  Maximum value of the internal counter.
        static constexpr count_type max()
        {
            return 1;
        }

        /// @brief  Constructs a binary semaphore statically.
        binary_semaphore(count_type desired = 0);

        #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

            /// @brief  Creates a binary semaphore by allocating memory on the heap, and initializing it.
            /// @return Dynamically allocated binary semaphore, or nullptr if allocation failed
            static binary_semaphore *create(count_type desired = 0);

        #endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)
    };
}

#endif // __FREERTOS_SEMAPHORE_H_
