/**
 * @file      thread.h
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
#ifndef __FREERTOS_THREAD_H_
#define __FREERTOS_THREAD_H_

#include "freertos/tick_timer.h"

namespace freertos
{
    namespace native
    {
        // opaque thread definition
        struct tskTaskControlBlock;

        // these macros may use native type casts, so need some redirection
        constexpr UBaseType_t TOP_PRIORITY = configMAX_PRIORITIES - 1;
        constexpr uint32_t MIN_STACK_SIZE = configMINIMAL_STACK_SIZE;
    }

    class condition_flags;

    using notify_value = std::uint32_t;

    /// @brief A class representing a thread of execution (equals to a FreeRTOS task).
    ///
    /// @attention The default FreeRTOS thread termination behavior is modified to allow
    /// threads to terminate by returning. The destructor will signal the exit semaphore,
    /// then delete the thread.
    ///
    /// It is the responsibility of the implementer do decide
    /// how the lifetime of the thread ends:
    ///
    /// a) through destruction of the thread object (externally controlled termination)
    /// b) by allowing the thread function to return (internally controlled termination)
    ///
    /// If externally controlled termination is desired, then it must be ensured
    /// that the thread function never returns. For this use-case a @ref static_thread
    /// is recommended.
    ///
    /// If internally controlled termination is desired, then the thread must be
    /// dynamically allocated, so the working memory is freed as a result of the
    /// thread's termination. In this case no smart pointer mechanism should be
    /// used to manage the allocated thread pointer. (This is doubly important since
    /// the thread's memory is allocated in two chunks.)
    ///
    class thread : protected native::StaticTask_t
    {
    public:
        static constexpr std::size_t NAME_MAX_SIZE = configMAX_TASK_NAME_LEN - 1; // exclude the terminating '\0'

        using function = native::TaskFunction_t;

        #if (configUSE_TRACE_FACILITY == 1)

            using id = native::UBaseType_t;

        #else

            using id = std::uintptr_t;

        #endif // (configUSE_TRACE_FACILITY == 1)

        /// @brief  Signals to the thread's observer that it's being terminated,
        ///         and destroys the thread, stopping its execution and freeing
        ///         its dynamically allocated memory.
        /// @remark Thread context callable
        ~thread();

        #ifdef configTHREAD_EXIT_CONDITION_INDEX

            /// @brief  Returns the currently stored exit condition of the thread.
            ///         Each thread may store a condition reference that it will signal
            ///         at the time of termination.
            /// @return Pointer to the exit condition, or nullptr if not set
            condition_flags *get_exit_condition();

            /// @brief  Tries to set a new exit condition for the thread.
            /// @param  sem: pointer to the condition to set
            /// @return true if the condition is set, false if the slot is already occupied
            bool set_exit_condition(condition_flags *cond);

            /// @brief  Tries to clear the exit condition for the thread.
            /// @param  sem: pointer to the condition to clear
            /// @return true if the condition is cleared, false if it wasn't set to begin with
            bool clear_exit_condition(condition_flags *cond);

        #endif // configTHREAD_EXIT_CONDITION_INDEX

        /// @brief  Suspends the execution of the thread, until @ref resume is called.
        /// @remark Thread context callable
        void suspend();

        /// @brief  Resumes the execution of the suspended thread.
        /// @remark Thread and ISR context callable
        void resume();

        /// @brief  Provides a unique identifier of the thread.
        /// @return The thread's unique identifier (0 is reserved as invalid)
        /// @remark Thread and ISR context callable
        id get_id() const;

        /// @brief  Reads the thread's friendly name (the string's size is limited to @ref NAME_MAX_SIZE).
        /// @return Pointer to the thread's name
        /// @remark Thread and ISR context callable
        const char *get_name();

        /// @brief  Possible operating states of a thread.
        enum class state
        {
            running = 0,
            ready,
            suspended,
            terminated,
        };

        /// @brief  Reads the current state of the thread.
        /// @return The current state of the thread
        /// @remark Thread context callable
        state get_state() const;

        /// @brief  Returns the currently executing thread.
        /// @return Pointer to the currently executing thread
        /// @remark Thread context callable (obviously)
        static thread* get_current();

        /// @brief  Thin type wrapper for thread priority.
        class priority
        {
        public:
            using value_type = native::UBaseType_t;

            /// @brief  Default priority is 1, to preempt the built-in IDLE thread of the RTOS.
            constexpr priority()
                    : value_(1)
            { }
            constexpr priority(value_type value)
                    : value_(value)
            { }
            operator value_type&()
            {
                return value_;
            }
            constexpr operator value_type() const
            {
                return value_;
            }

            static constexpr priority max()
            {
                return native::TOP_PRIORITY;
            }
            static constexpr priority min()
            {
                return 0;
            }

        private:
            value_type value_;
        };

        /// @brief  Returns the thread's current priority level.
        /// @return The priority of the thread
        /// @remark Thread and ISR context callable
        priority get_priority() const;

        /// @return Pointer to the currently executing thread
        /// @remark Thread context callable
        void set_priority(priority prio);


        #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

            /// @brief  Creates a new thread by allocating memory on the heap, and initializing it.
            ///         The thread becomes ready to execute within this call, meaning that it might
            ///         have started running by the time this call returns.
            /// @param  func:      the function to execute in the thread context
            /// @param  param:     opaque parameter to pass to the thread function
            /// @param  stacksize: size of the thread's stack in bytes
            /// @param  prio:      thread priority level
            /// @param  name:      short label for identifying the thread
            /// @return pointer to the newly created thread, or nullptr if allocation failed
            static thread* create(function func, void *param,
                    size_t stacksize = DEFAULT_STACK_SIZE,
                    priority prio    = priority(),
                    const char *name = DEFAULT_NAME);

            template<typename T>
            static typename std::enable_if<(sizeof(T) <= sizeof(std::uintptr_t)), thread*>::type
            create(void (*func)(T), T arg,
                    size_t stacksize = DEFAULT_STACK_SIZE,
                    priority prio    = priority(),
                    const char *name = DEFAULT_NAME)
            {
                return create(reinterpret_cast<function>(func),
                        reinterpret_cast<void*>(static_cast<std::uintptr_t>(arg)),
                        stacksize, prio, name);
            }

            template<typename T>
            static thread*
            create(void (*func)(T*), T* arg,
                    size_t stacksize = DEFAULT_STACK_SIZE,
                    priority prio    = priority(),
                    const char *name = DEFAULT_NAME)
            {
                return create(reinterpret_cast<function>(func),
                        reinterpret_cast<void*>(arg),
                        stacksize, prio, name);
            }

            template<typename T>
            static thread*
            create(void (*func)(T*), T& arg,
                    size_t stacksize = DEFAULT_STACK_SIZE,
                    priority prio    = priority(),
                    const char *name = DEFAULT_NAME)
            {
                return create(reinterpret_cast<function>(func),
                        reinterpret_cast<void*>(&arg),
                        stacksize, prio, name);
            }

            template<class T>
            static thread* create(T& obj, void (T::*member_func)(),
                    size_t stacksize = DEFAULT_STACK_SIZE,
                    priority prio    = priority(),
                    const char *name = DEFAULT_NAME)
            {
                return create(reinterpret_cast<function>(member_func),
                        reinterpret_cast<void*>(&obj),
                        stacksize, prio, name);
            }

            /// @brief  Empty delete operator, since the destructor does the memory freeing
            ///         if the object was dynamically allocated
            void operator delete(void *p)
            {
            }

        #endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)

    protected:
        static constexpr const char* DEFAULT_NAME = "anonym";
        static constexpr size_t DEFAULT_STACK_SIZE = native::MIN_STACK_SIZE * sizeof(native::StackType_t);

        thread(native::StackType_t *pstack, std::uint32_t stack_size,
                function func, void *param,
                priority prio, const char *name);

        native::tskTaskControlBlock* handle() const
        {
            return reinterpret_cast<native::tskTaskControlBlock*>(const_cast<thread*>(this));
        }

    private:
        // non-copyable
        thread(const thread&) = delete;
        thread& operator=(const thread&) = delete;
        // non-movable
        thread(const thread&&) = delete;
        thread& operator=(const thread&&) = delete;

        void signal_exit();
    };


    /// @brief  A thread with statically allocated stack.
    template <const std::size_t STACK_SIZE_BYTES>
    class static_thread : public thread
    {
    public:
        static constexpr std::size_t STACK_SIZE = STACK_SIZE_BYTES;

        /// @brief  Constructs a static thread. The thread becomes ready to execute
        ///         within this call, meaning that it might have started running
        ///         by the time this call returns.
        /// @param  func:      the function to execute in the thread context
        /// @param  param:     opaque parameter to pass to the thread function
        /// @param  prio:      thread priority level
        /// @param  name:      short label for identifying the thread
        static_thread(function func, void *param,
                priority prio = priority(), const char *name = DEFAULT_NAME)
            : thread(stack_, sizeof(stack_) / sizeof(stack_[0]),
                    func, param, prio, name)
        {
        }

        template<typename T>
        static_thread(typename std::enable_if<(sizeof(T) <= sizeof(std::uintptr_t)),
                void (*)(T)>::type func, T arg,
                priority prio = priority(), const char *name = DEFAULT_NAME)
            : static_thread(reinterpret_cast<function>(func),
                    reinterpret_cast<void*>(static_cast<std::uintptr_t>(arg)),
                    prio, name)
        {
        }

        template<typename T>
        static_thread(void (*func)(T*), T* arg,
                priority prio = priority(), const char *name = DEFAULT_NAME)
            : static_thread(reinterpret_cast<function>(func),
                    reinterpret_cast<void*>(arg),
                    prio, name)
        {
        }

        template<typename T>
        static_thread(void (*func)(T*), T& arg,
                priority prio = priority(), const char *name = DEFAULT_NAME)
            : static_thread(reinterpret_cast<function>(func),
                    reinterpret_cast<void*>(arg),
                    prio, name)
        {
        }

        template<class T>
        static_thread(T& obj, void (T::*member_func)(),
                priority prio = priority(), const char *name = DEFAULT_NAME)
            : static_thread(reinterpret_cast<function>(member_func),
                    reinterpret_cast<void*>(&obj),
                    prio, name)
        {
        }

        /// @brief  Static threads aren't allocated by the OS, so new and delete
        ///         may use the default operators.
        void operator delete(void *p)
        {
            ::delete p;
        }

    private:
        native::StackType_t stack_[STACK_SIZE_BYTES / sizeof(native::StackType_t)];
    };


    /// @brief  Namespace offering control on the current thread of execution.
    namespace this_thread
    {
        /// @brief  Yields execution of the current thread so the OS can schedule
        ///         other thread(s) for the remainder of the time slice.
        void yield();

        /// @brief  Provides a unique identifier of the current thread.
        /// @return The current thread's unique identifier
        thread::id get_id();

        void sleep_for(tick_timer::duration rel_time);

        /// @brief  Blocks the current thread's execution for a given duration.
        /// @param  rel_time: duration to block the current thread
        template<class Rep, class Period>
        void sleep_for(const std::chrono::duration<Rep, Period>& rel_time)
        {
            // workaround to prevent this function calling itself
            const auto ticks_sleep_for = static_cast<void (*)(tick_timer::duration)>(&sleep_for);
            ticks_sleep_for(std::chrono::duration_cast<tick_timer::duration>(rel_time));
        }

        /// @brief  Blocks the current thread's execution until the given deadline.
        /// @param  abs_time: deadline to block the current thread
        template<class Clock, class Duration>
        void sleep_until(const std::chrono::time_point<Clock, Duration>& abs_time)
        {
            sleep_for(abs_time - Clock::now());
        }

        #if 0 && (configUSE_TASK_NOTIFICATIONS == 1)

            bool notify_wait_for(const tick_timer::duration& rel_time,
                    thread::notify_flag clr_at_entry = 0, thread::notify_flag clr_at_exit = 0,
                    thread::notify_flag *received = nullptr);

            notify_value notify_value_wait_for(const tick_timer::duration& rel_time,
                    thread::notify_flag clr_at_entry = 0, thread::notify_flag clr_at_exit = 0,
                    thread::notify_flag *received = nullptr);

        #endif // (configUSE_TASK_NOTIFICATIONS == 1)
    }
}

#endif // __FREERTOS_THREAD_H_
