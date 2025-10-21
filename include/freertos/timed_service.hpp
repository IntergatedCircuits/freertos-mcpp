// SPDX-License-Identifier: MIT
#ifndef __FREERTOS_TIMED_SERVICE_HPP_
#define __FREERTOS_TIMED_SERVICE_HPP_

#include "freertos/tick_timer.hpp"

struct tmrTimerControl;

namespace freertos
{
class thread;

#if (configUSE_TIMERS == 1)

class timed_service : protected ::StaticTimer_t
{
  public:
    using function = void (*)(timed_service*);

    ~timed_service();

    bool is_active() const;

    bool start(tick_timer::duration waittime = tick_timer::duration(0));
    bool stop(tick_timer::duration waittime = tick_timer::duration(0));
    bool reset(tick_timer::duration waittime = tick_timer::duration(0));

    bool is_reloading() const;
    void set_reloading(bool reloading);

    tick_timer::duration get_period() const;
    bool set_period(tick_timer::duration new_period,
                    tick_timer::duration waittime = tick_timer::duration(0));

    void* get_owner() const;
    void set_owner(void* owner);

    tick_timer::time_point get_trigger_time() const;

    const char* get_name() const;

    timed_service(function func, void* owner, tick_timer::duration period, bool reloading,
                  const char* name = DEFAULT_NAME);

  private:
    static constexpr const char* DEFAULT_NAME = "anonym";

    ::tmrTimerControl* handle() const
    {
        return reinterpret_cast<::tmrTimerControl*>(const_cast<timed_service*>(this));
    }

    static thread* get_service_thread();

    // non-copyable
    timed_service(const timed_service&) = delete;
    timed_service& operator=(const timed_service&) = delete;
    // non-movable
    timed_service(const timed_service&&) = delete;
    timed_service& operator=(const timed_service&&) = delete;
};

#endif // (configUSE_TIMERS == 1)
} // namespace freertos

#endif // __FREERTOS_TIMED_SERVICE_H_
