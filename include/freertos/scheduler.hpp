// SPDX-License-Identifier: MIT
#ifndef __FREERTOS_SCHEDULER_HPP_
#define __FREERTOS_SCHEDULER_HPP_

#include "freertos/stdlib.hpp"
#include "freertos/thread.hpp"

namespace freertos
{
/// @brief  Static class that allows FreeRTOS scheduler control.
class scheduler
{
  public:
    /// @brief  Possible operating states of the scheduler.
    enum class state : ::BaseType_t
    {
        suspended = 0,
        uninitialized = 1,
        running = 2
    };

    /// @brief  Starts the scheduler. This function doesn't return.
    /// @remark Thread context callable
    [[noreturn]] static void start();

    /// @brief  Reads the current state of the scheduler.
    /// @return The current state of the scheduler
    /// @remark Thread context callable
    static state get_state();

    /// @brief  Reads the total number of existing threads.
    /// @return The number of threads
    /// @remark Thread context callable
    static size_t get_threads_count();

    /// @brief  @ref Lockable critical section that manipulates the scheduler's state.
    class critical_section
    {
      public:
        /// @brief  Suspends the scheduler, allowing the caller thread to operate without
        ///         the possibility of context switches. Blocking operations are forbidden
        ///         while the scheduler is suspended.
        /// @remark Thread context callable
        void lock();

        /// @brief  Resumes the scheduler, allowing thread context switches.
        /// @remark Thread context callable
        void unlock();

        constexpr critical_section() {}
    };

  private:
    scheduler();
};

} // namespace freertos

#endif // __FREERTOS_SCHEDULER_HPP_
