// SPDX-License-Identifier: MIT
#ifndef __FREERTOS_EVENT_GROUP_HPP_
#define __FREERTOS_EVENT_GROUP_HPP_

#include "freertos/tick_timer.hpp"

struct EventGroupDef_t;

namespace freertos
{
/// @brief  Thin type wrapper for condition flag value type.
class events
{
  public:
    using value_type = ::TickType_t;

    constexpr explicit events() : value_(0) {}
    constexpr explicit events(value_type value) : value_(value) {}
    operator value_type&() { return value_; }
    constexpr operator value_type() const { return value_; }
    constexpr events operator&(events other) const { return events(value_ & other.value_); }

    static constexpr events max()
    {
        // the highest byte is reserved for OS flags
        return events((1 << ((sizeof(value_type) - 1) * 8)) - 1);
    }
    static constexpr events min() { return events(); }

    /// @brief  The value returned by blocking function calls when the wait time expired
    ///         without any relevant flags being set.
    static constexpr events timeout() { return events(); }

  private:
    value_type value_;
};

/// @brief  This class is a lightweight condition variable, allows threads to block
///         until a combination of flags has been set. The key difference to @ref condition_variable
///         is that here the waiting side chooses the wait strategy:
///          1. whether to wait for a combination of flags (all) or one of many (any)
///          2. whether to consume the flag when receiving it (default) or not (shared)
class event_group : private ::StaticEventGroup_t
{
  public:
    /// @brief  Constructs a event_group statically.
    /// @remark Thread context callable
    event_group();

    /// @brief  Destroys the event_group, including freeing its dynamically allocated memory.
    /// @remark Thread context callable
    ~event_group();

    /// @brief  Reads the current flags status.
    /// @return The currently active flags
    /// @remark Thread and ISR context callable
    events get() const;

    /// @brief  Sets the provided flags in the condition.
    /// @param  flags: the flags to activate
    /// @remark Thread and ISR context callable
    void set(events flags);

    /// @brief  Removes the provided flags from the condition.
    /// @param  flags: the flags to deactivate
    /// @remark Thread and ISR context callable
    void clear(events flags);

    /// @brief  Blocks the current thread until any of the provided flags is raised.
    ///         When a flag unblocks the thread, it will be cleared.
    /// @param  flags: selection of flags to wait on
    /// @param  rel_time: duration to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Rep, class Period>
    events wait_any_for(events flags, const std::chrono::duration<Rep, Period>& rel_time)
    {
        return wait(flags, std::chrono::duration_cast<tick_timer::duration>(rel_time), true, false);
    }

    /// @brief  Blocks the current thread until any of the provided flags is raised.
    ///         When a flag unblocks the thread, it will be cleared.
    /// @param  flags: selection of flags to wait on
    /// @param  abs_time: deadline to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Clock, class Duration>
    events wait_any_until(events flags, const std::chrono::time_point<Clock, Duration>& abs_time)
    {
        return wait_any_for(flags, abs_time - Clock::now());
    }

    /// @brief  Blocks the current thread until all of the provided flags are raised.
    ///         When the thread is unblocked, the required flags will be cleared.
    /// @param  flags: combination of flags to wait on
    /// @param  rel_time: duration to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Rep, class Period>
    events wait_all_for(events flags, const std::chrono::duration<Rep, Period>& rel_time)
    {
        return wait(flags, std::chrono::duration_cast<tick_timer::duration>(rel_time), true, true);
    }

    /// @brief  Blocks the current thread until all of the provided flags are raised.
    ///         When the thread is unblocked, the required flags will be cleared.
    /// @param  flags: combination of flags to wait on
    /// @param  abs_time: deadline to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Clock, class Duration>
    events wait_all_until(events flags, const std::chrono::time_point<Clock, Duration>& abs_time)
    {
        return wait_all_for(flags, abs_time - Clock::now());
    }

    /// @brief  Blocks the current thread until any of the provided flags is raised.
    ///         Doesn't modify the flags upon activation.
    /// @param  flags: selection of flags to wait on
    /// @param  rel_time: duration to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Rep, class Period>
    events shared_wait_any_for(events flags, const std::chrono::duration<Rep, Period>& rel_time)
    {
        return wait(flags, std::chrono::duration_cast<tick_timer::duration>(rel_time), false,
                    false);
    }

    /// @brief  Blocks the current thread until any of the provided flags is raised.
    ///         Doesn't modify the flags upon activation.
    /// @param  flags: selection of flags to wait on
    /// @param  abs_time: deadline to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Clock, class Duration>
    events shared_wait_any_until(events flags,
                                 const std::chrono::time_point<Clock, Duration>& abs_time)
    {
        return shared_wait_any_for(flags, abs_time - Clock::now());
    }

    /// @brief  Blocks the current thread until all of the provided flags are raised.
    ///         Doesn't modify the flags upon activation.
    /// @param  flags: combination of flags to wait on
    /// @param  rel_time: duration to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Rep, class Period>
    events shared_wait_all_for(events flags, const std::chrono::duration<Rep, Period>& rel_time)
    {
        return wait(flags, std::chrono::duration_cast<tick_timer::duration>(rel_time), false, true);
    }

    /// @brief  Blocks the current thread until all of the provided flags are raised.
    ///         Doesn't modify the flags upon activation.
    /// @param  flags: combination of flags to wait on
    /// @param  abs_time: deadline to wait for the activation
    /// @return the raised flag(s) that caused the activation, or 0 if timed out
    /// @remark Thread context callable
    template <class Clock, class Duration>
    events shared_wait_all_until(events flags,
                                 const std::chrono::time_point<Clock, Duration>& abs_time)
    {
        return shared_wait_all_for(flags, abs_time - Clock::now());
    }

  protected:
    inline ::EventGroupDef_t* handle() const
    {
        return reinterpret_cast<::EventGroupDef_t*>(const_cast<event_group*>(this));
    }

    events wait(events flags, const tick_timer::duration& rel_time, bool exclusive, bool match_all);
};
} // namespace freertos

#endif // __FREERTOS_EVENT_GROUP_HPP_
