/**
 * @file      queue.h
 * @brief     FreeRTOS queue API abstraction
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
#ifndef __FREERTOS_QUEUE_H_
#define __FREERTOS_QUEUE_H_

#include "freertos/tick_timer.h"

namespace freertos
{
    namespace native
    {
        // opaque queue definition
        struct QueueDefinition;
    }

    /// @brief  An abstract base class for all queues and derivatives.
    class queue : protected native::StaticQueue_t
    {
    public:
        using size_type = native::UBaseType_t;

        /// @brief  The current occupied size of the queue.
        /// @return The number of elements in the queue
        /// @remark Thread and ISR context callable
        size_type size() const;

        /// @brief  The current free size of the queue.
        /// @return The number of available spaces in the queue
        /// @remark Thread context callable
        size_type available() const;

        /// @brief  Determines if the queue is currently full.
        /// @return True if the queue is full, false otherwise
        /// @remark Thread and ISR context callable
        bool full() const;

        /// @brief  Determines if the queue is currently empty.
        /// @return True if the queue is empty, false otherwise
        /// @remark Thread and ISR context callable
        bool empty() const;

        /// @brief  Flushes the queue, resetting it to it's initial empty state.
        /// @remark Thread context callable
        void reset();

        /// @brief  Destroys the queue, including freeing its dynamically allocated memory.
        /// @remark Thread context callable
        ~queue();

    protected:
        inline native::QueueDefinition* handle() const
        {
            return reinterpret_cast<native::QueueDefinition*>(const_cast<queue*>(this));
        }

        bool push_front(void *data, tick_timer::duration waittime);
        bool push_back(void *data, tick_timer::duration waittime);
        void replace(void *data);
        bool peek_front(void *data, tick_timer::duration waittime) const;
        bool pop_front(void *data, tick_timer::duration waittime);

        // empty constructor is used by semaphores, since the underlying API create calls differ
        queue()
        {
        }

        // used to create shallow_copy_queue
        queue(size_type size, size_type elem_size, unsigned char *elem_buffer);

    private:
        // non-copyable
        queue(const queue&) = delete;
        queue& operator=(const queue&) = delete;
        // non-movable
        queue(const queue&&) = delete;
        queue& operator=(const queue&&) = delete;
    };

    template<typename T>
    class ishallow_copy_queue : public queue
    {
    public:
        using value_type = T;

        /// @brief  Size of the elements stored in the queue.
        static constexpr size_type elem_size()
        {
            return sizeof(value_type);
        }

        /// @brief  Pushes a new value to the front of the queue.
        /// @param  value: the new value to copy
        /// @param  waittime: duration to wait for the queue to have available space
        /// @return true if successful, false if the queue is full
        /// @remark Thread and ISR context callable (ISR only with no waittime)
        bool push_front(const value_type &value, tick_timer::duration waittime = tick_timer::duration(0))
        {
            return queue::push_front(reinterpret_cast<void*>(const_cast<value_type*>(&value)), waittime);
        }

        /// @brief  Pushes a new value to the back of the queue.
        /// @param  value: the new value to copy
        /// @param  waittime: duration to wait for the queue to have available space
        /// @return true if successful, false if the queue is full
        /// @remark Thread and ISR context callable (ISR only with no waittime)
        bool push_back(const value_type &value, tick_timer::duration waittime = tick_timer::duration(0))
        {
            return queue::push_back(reinterpret_cast<void*>(const_cast<value_type*>(&value)), waittime);
        }

        /// @brief  Replaces the current queue element value to a new one.
        ///         This call is meant to be used by single length queues only.
        /// @param  value: the new value to copy
        /// @remark Thread and ISR context callable
        void replace(const value_type &value)
        {
            queue::replace(reinterpret_cast<void*>(const_cast<value_type*>(&value)));
        }

        /// @brief  Copies the front value of the queue without consuming it.
        /// @param  value: the destination pointer to copy to
        /// @param  waittime: duration to wait for the queue to have an available element
        /// @return true if successful, false if the queue is empty
        /// @remark Thread and ISR context callable (ISR only with no waittime)
        bool peek_front(value_type *value, tick_timer::duration waittime = tick_timer::duration(0)) const
        {
            return queue::peek_front(reinterpret_cast<void*>(value), waittime);
        }

        /// @brief  Copies the front value of the queue and removes it from the queue.
        /// @param  value: the destination pointer to copy to
        /// @param  waittime: duration to wait for the queue to have an available element
        /// @return true if successful, false if the queue is empty
        /// @remark Thread and ISR context callable (ISR only with no waittime)
        bool pop_front(value_type *value, tick_timer::duration waittime = tick_timer::duration(0))
        {
            return queue::pop_front(reinterpret_cast<void*>(value), waittime);
        }

    protected:
        ishallow_copy_queue(size_type size, size_type elem_size, unsigned char *elem_buffer)
            : queue(size, elem_size, elem_buffer)
        {
        }
    };

    /// @brief  A thread/ISR-safe queue that stores shallow copies (via memcpy) of pushed elements.
    template<typename T, const queue::size_type MAX_SIZE>
    class shallow_copy_queue : public ishallow_copy_queue<T>
    {
    public:
        using value_type = T;
        using size_type = queue::size_type;

        /// @brief  Maximum size of the queue.
        static constexpr size_type max_size()
        {
            return MAX_SIZE;
        }

        /// @brief  Constructs a shallow-copy queue statically.
        shallow_copy_queue()
            : ishallow_copy_queue<value_type>(max_size(), sizeof(value_type), elem_buffer_)
        {
        }

    private:
        unsigned char elem_buffer_[max_size() * sizeof(value_type)];
    };
}

#endif // __FREERTOS_QUEUE_H_
