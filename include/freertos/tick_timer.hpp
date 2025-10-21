// SPDX-License-Identifier: MIT
#ifndef __FREERTOS_TICK_TIMER_HPP_
#define __FREERTOS_TICK_TIMER_HPP_

#include <chrono>
#include <cstddef>
#include <cstdint>
#if ESP_PLATFORM
#include <freertos/FreeRTOS.h>
#else
#include <FreeRTOS.h>
#endif

namespace freertos
{
namespace detail
{
constexpr ::TickType_t infinite_delay = portMAX_DELAY;
constexpr ::TickType_t tick_rate_Hz = configTICK_RATE_HZ;
} // namespace detail

/// @brief  A @ref TrivialClock class that wraps the FreeRTOS tick timer.
class tick_timer
{
  public:
    using rep = ::TickType_t;
    using period = std::ratio<1, detail::tick_rate_Hz>;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<tick_timer>;
    static constexpr bool is_steady = true;

    /// @brief  Wraps the current OS tick count into a clock time point.
    /// @return The current tick count as time_point
    /// @remark Thread and ISR context callable
    static time_point now();
};

/// @brief  Converts a duration to the underlying tick count.
/// @param  rel_time: time duration
/// @return Tick count
template <class Rep, class Period>
constexpr tick_timer::rep to_ticks(const std::chrono::duration<Rep, Period>& rel_time)
{
    return std::chrono::duration_cast<tick_timer::duration>(rel_time).count();
}

/// @brief  Converts @ref tick_timer::time_point to the underlying tick count.
/// @param  time: time point from the start of the tick_timer
/// @return Tick count
constexpr tick_timer::rep to_ticks(const tick_timer::time_point& time)
{
    return to_ticks(time.time_since_epoch());
}

/// @brief  Dedicated @ref tick_timer::duration expression that ensures
///         infinite wait time on an operation
constexpr tick_timer::duration infinity{detail::infinite_delay};
} // namespace freertos

#endif // __FREERTOS_TICK_TIMER_HPP_
