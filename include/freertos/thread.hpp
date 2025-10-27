// SPDX-License-Identifier: MIT
#ifndef __FREERTOS_THREAD_HPP_
#define __FREERTOS_THREAD_HPP_

#include "freertos/stdlib.hpp"
#include "freertos/tick_timer.hpp"

// opaque thread definition
struct tskTaskControlBlock;

namespace freertos
{
namespace detail
{
constexpr ::UBaseType_t TOP_PRIORITY = configMAX_PRIORITIES - 1;
constexpr uint32_t MIN_STACK_SIZE = configMINIMAL_STACK_SIZE;
} // namespace detail

class event_group;

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
class thread : private ::StaticTask_t
{
  public:
    static constexpr std::size_t NAME_MAX_SIZE =
        configMAX_TASK_NAME_LEN - 1; // exclude the terminating '\0'

    using function = ::TaskFunction_t;

#if (configUSE_TRACE_FACILITY == 1)

    using id = ::UBaseType_t;

#else

    using id = std::uintptr_t;

#endif // (configUSE_TRACE_FACILITY == 1)

    /// @brief  Number of concurrent threads supported.
    /// @return Fixed to 0 since FreeRTOS doesn't have such limitation by design,
    ///         only resource constraints apply
    static constexpr unsigned int hardware_concurrency() { return 0; }

    /// @brief  Signals to the thread's observer that it's being terminated,
    ///         and destroys the thread, stopping its execution and freeing
    ///         its dynamically allocated memory.
    /// @remark Thread context callable
    ~thread();

#ifdef configTHREAD_EXIT_CONDITION_INDEX

  private:
    condition_flags* get_exit_condition() const;
    void set_exit_condition(condition_flags* cond);

  public:
    /// @brief  Waits for the thread to finish execution.
    /// @note   May only be called when the thread is joinable, and not from the owned thread's
    /// context
    void join();

    /// @brief  Checks if the thread is joinable (potentially executing).
    /// @return true if the thread is valid and hasn't been joined, false otherwise
    /// @remark Thread and ISR context callable
    bool joinable() const;

#endif // configTHREAD_EXIT_CONDITION_INDEX

    // detach is not supported.
    // if the thread is create()-d with dynamic allocation then the thread is already detached
    // if the thread is statically allocated, detach is not possible

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

    /// @brief  Reads the thread's friendly name (the string's size is limited to @ref
    /// NAME_MAX_SIZE).
    /// @return Pointer to the thread's name
    /// @remark Thread and ISR context callable
    const char* get_name();

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
        using value_type = ::UBaseType_t;

        /// @brief  Default priority is 1, to preempt the built-in IDLE thread of the RTOS.
        constexpr priority() : value_(1) {}
        constexpr priority(value_type value) : value_(value) {}
        operator value_type&() { return value_; }
        constexpr operator value_type() const { return value_; }

        static constexpr priority max() { return detail::TOP_PRIORITY; }
        static constexpr priority min() { return 0; }

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

    // use make_thread() instead
    void* operator new(size_t size) = delete;
    void* operator new[](size_t size) = delete;

    /// @brief  Empty delete operator, since the destructor does the memory freeing
    ///         if the object was dynamically allocated
    void operator delete(void* p) {}
    void operator delete[](void* p) {}

#if (configUSE_TASK_NOTIFICATIONS == 1)

    using notify_value = std::uint32_t;

    /// @brief A lightweight synchronization and inter-process communication mechanism
    /// for a single thread.
    class notifier
    {
      public:
        constexpr notifier(thread& t = *thread::get_current()) : thread_(&t) {}

        using index_type = ::UBaseType_t;

#if (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)

        constexpr notifier(thread& t, index_type index) : thread_(&t), index_(index) {}

#endif // (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)

        inline notify_value get_last_value() const { return last_value_; }

        inline notify_value get_value() { return clear(0); }

        void signal();
        bool cancel_signal();

        void increment();

        void set_flags(notify_value flags);
        inline void clear_flags(notify_value flags) { last_value_ = clear(flags); }

        bool try_set_value(notify_value new_value);

        void set_value(notify_value new_value);

        inline void reset_value() { last_value_ = clear(~0); }

        thread* get_thread() const { return thread_; }

      private:
        thread* const thread_;
        notify_value last_value_ = 0;

#if (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)

        const index_type index_ = 0;

#endif // (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)

        inline ::tskTaskControlBlock* handle() const
        {
            return const_cast<thread*>(thread_)->handle();
        }

        bool notify(unsigned action, notify_value value);
        notify_value clear(notify_value flags);
    };

#endif // (configUSE_TASK_NOTIFICATIONS == 1)

    static constexpr const char* DEFAULT_NAME = "anonym";
    static constexpr size_t DEFAULT_STACK_SIZE = detail::MIN_STACK_SIZE * sizeof(::StackType_t);

  protected:
    thread(::StackType_t* pstack, std::uint32_t stack_size, function func, void* param,
           priority prio, const char* name);

    ::tskTaskControlBlock* handle() const
    {
        return reinterpret_cast<::tskTaskControlBlock*>(const_cast<thread*>(this));
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
thread* make_thread(thread::function func, void* param,
                    size_t stacksize = thread::DEFAULT_STACK_SIZE,
                    thread::priority prio = thread::priority(),
                    const char* name = thread::DEFAULT_NAME);

template <typename T>
static typename std::enable_if<(sizeof(T) == sizeof(std::uintptr_t)), thread*>::type
make_thread(void (*func)(T), T arg, size_t stacksize = thread::DEFAULT_STACK_SIZE,
            thread::priority prio = thread::priority(), const char* name = thread::DEFAULT_NAME)
{
    return make_thread(reinterpret_cast<thread::function>(func), bit_cast<void*>(arg), stacksize,
                       prio, name);
}

template <typename T>
thread* make_thread(void (*func)(T*), T* arg, size_t stacksize = thread::DEFAULT_STACK_SIZE,
                    thread::priority prio = thread::priority(),
                    const char* name = thread::DEFAULT_NAME)
{
    return make_thread(reinterpret_cast<thread::function>(func), static_cast<void*>(arg), stacksize,
                       prio, name);
}

template <typename T>
thread* make_thread(void (*func)(T*), T& arg, size_t stacksize = thread::DEFAULT_STACK_SIZE,
                    thread::priority prio = thread::priority(),
                    const char* name = thread::DEFAULT_NAME)
{
    return make_thread(reinterpret_cast<thread::function>(func), static_cast<void*>(&arg),
                       stacksize, prio, name);
}

template <class T>
thread* make_thread(T& obj, void (T::*member_func)(), size_t stacksize = thread::DEFAULT_STACK_SIZE,
                    thread::priority prio = thread::priority(),
                    const char* name = thread::DEFAULT_NAME)
{
    return make_thread(reinterpret_cast<thread::function>(member_func), static_cast<void*>(&obj),
                       stacksize, prio, name);
}

#endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)

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
    static_thread(function func, void* param, priority prio = priority(),
                  const char* name = DEFAULT_NAME)
        : thread(stack_, sizeof(stack_) / sizeof(stack_[0]), func, param, prio, name)
    {}

    template <typename T>
    static_thread(
        typename std::enable_if<(sizeof(T) == sizeof(std::uintptr_t)), void (*)(T)>::type func,
        T arg, priority prio = priority(), const char* name = DEFAULT_NAME)
        : static_thread(reinterpret_cast<function>(func), bit_cast<void*>(arg), prio, name)
    {}

    template <typename T>
    static_thread(void (*func)(T*), T* arg, priority prio = priority(),
                  const char* name = DEFAULT_NAME)
        : static_thread(reinterpret_cast<function>(func), static_cast<void*>(arg), prio, name)
    {}

    template <typename T>
    static_thread(void (*func)(T*), T& arg, priority prio = priority(),
                  const char* name = DEFAULT_NAME)
        : static_thread(reinterpret_cast<function>(func), static_cast<void*>(arg), prio, name)
    {}

    template <class T>
    static_thread(T& obj, void (T::*member_func)(), priority prio = priority(),
                  const char* name = DEFAULT_NAME)
        : static_thread(reinterpret_cast<function>(member_func), static_cast<void*>(&obj), prio,
                        name)
    {}

  private:
    ::StackType_t stack_[STACK_SIZE_BYTES / sizeof(::StackType_t)];
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
template <class Rep, class Period>
inline void sleep_for(const std::chrono::duration<Rep, Period>& rel_time)
{
    // workaround to prevent this function calling itself
    const auto ticks_sleep_for = static_cast<void (*)(tick_timer::duration)>(&sleep_for);
    ticks_sleep_for(std::chrono::duration_cast<tick_timer::duration>(rel_time));
}

/// @brief  Blocks the current thread's execution until the given deadline.
/// @param  abs_time: deadline to block the current thread
template <class Clock, class Duration>
inline void sleep_until(const std::chrono::time_point<Clock, Duration>& abs_time)
{
    sleep_for(abs_time - Clock::now());
}

#if (configUSE_TASK_NOTIFICATIONS == 1)

/// @brief  Wait for a notifier to signal the current thread.
/// @param  rel_time: maximum duration to wait for the notification
/// @param  value: it is set to the value of the notification as it is received
/// @param  clear_flags_before: these flags are cleared from the notification value
///         before the waiting begins
/// @param  clear_flags_after: these flags are cleared from the notification value
///         after the signal was received (only if it was received)
/// @return true if a notification was received, false if timed out
bool wait_notification_for(const tick_timer::duration& rel_time, thread::notify_value* value,
                           thread::notify_value clear_flags_before = 0,
                           thread::notify_value clear_flags_after = 0);

inline void wait_notification(thread::notify_value* value,
                              thread::notify_value clear_flags_before = 0,
                              thread::notify_value clear_flags_after = 0)
{
    wait_notification_for(infinity, value, clear_flags_before, clear_flags_after);
}

/// @brief  Wait for a notifier to signal the current thread.
/// @param  rel_time: maximum duration to wait for the notification
/// @return true if a notification was received, false if timed out
inline bool wait_signal_for(const tick_timer::duration& rel_time)
{
    return wait_notification_for(rel_time, nullptr);
}

inline void wait_signal()
{
    wait_signal_for(infinity);
}

notify_value try_acquire_notification_for(const tick_timer::duration& rel_time,
                                          bool acquire_single = false);

inline notify_value acquire_notification(bool acquire_single = false)
{
    return try_acquire_notification_for(infinity, acquire_single);
}

#if (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)

/// @brief  Wait for a notifier to signal the current thread.
/// @param  index: notification selector index
/// @param  rel_time: maximum duration to wait for the notification
/// @param  value: it is set to the value of the notification as it is received
/// @param  clear_flags_before: these flags are cleared from the notification value
///         before the waiting begins
/// @param  clear_flags_after: these flags are cleared from the notification value
///         after the signal was received (only if it was received)
/// @return true if a notification was received, false if timed out
bool wait_notification_for(thread::notifier::index_type index, const tick_timer::duration& rel_time,
                           thread::notify_value* value, thread::notify_value clear_flags_before = 0,
                           thread::notify_value clear_flags_after = 0);

inline void wait_notification(thread::notifier::index_type index,
                              const tick_timer::duration& rel_time, thread::notify_value* value,
                              thread::notify_value clear_flags_before = 0,
                              thread::notify_value clear_flags_after = 0)
{
    wait_notification_for(index, infinity, value, clear_flags_before, clear_flags_after);
}

/// @brief  Wait for a notifier to signal the current thread.
/// @param  index: notification selector index
/// @param  rel_time: maximum duration to wait for the notification
/// @return true if a notification was received, false if timed out
inline bool wait_signal_for(thread::notifier::index_type index,
                            const tick_timer::duration& rel_time)
{
    return wait_notification_for(index, rel_time, nullptr);
}

inline void wait_signal(thread::notifier::index_type index)
{
    return wait_signal_for(index, infinity);
}

notify_value try_acquire_notification_for(thread::notifier::index_type index,
                                          const tick_timer::duration& rel_time,
                                          bool acquire_single = false);

inline notify_value acquire_notification(thread::notifier::index_type index,
                                         bool acquire_single = false)
{
    return try_acquire_notification_for(index, infinity, acquire_single);
}

#endif // (configTASK_NOTIFICATION_ARRAY_ENTRIES > 1)

#endif // (configUSE_TASK_NOTIFICATIONS == 1)
} // namespace this_thread
} // namespace freertos

#endif // __FREERTOS_THREAD_HPP_
