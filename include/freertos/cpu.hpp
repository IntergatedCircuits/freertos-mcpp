// SPDX-License-Identifier: MIT
#ifndef __FREERTOS_CPU_HPP_
#define __FREERTOS_CPU_HPP_

#include "freertos/stdlib.hpp"
#include "freertos/tick_timer.hpp"

namespace freertos
{
class cpu
{
  public:
    /// @brief  A @ref BasicLockable class that prevents task and interrupt
    ///         context switches while locked.
    class critical_section
    {
      public:
        /// @brief  Locks the CPU, preventing thread and interrupt switches.
        void lock();

        /// @brief  Unlocks the CPU, allowing other interrupts and threads
        ///         to preempt the current execution context.
        void unlock();

        constexpr critical_section()
        {
#if ESP_PLATFORM
            spinlock_initialize(&restore_lock_);
#endif
        }

      private:
#if ESP_PLATFORM
        spinlock_t restore_lock_;
        auto& restore_var() { return restore_lock_.count; }
#else
        std::uintptr_t restore_{};
        auto& restore_var() { return restore_; }
#endif
    };
};

namespace this_cpu
{
/// @brief  Determines if the current execution context is inside
///         an interrupt service routine.
/// @note   The underlying port function (@ref xPortIsInsideInterrupt)
///         is only available for a subset of ports
///         but it could be extended to many targets
///         e.g. by checking if the current task's stack is being used
/// @return true if the current execution context is ISR, false otherwise
bool is_in_isr();

} // namespace this_cpu
} // namespace freertos

#endif // __FREERTOS_CPU_HPP_
