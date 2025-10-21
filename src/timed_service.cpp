// SPDX-License-Identifier: MIT
#include "freertos/timed_service.hpp"
#include "freertos/cpu.hpp"
#include "freertos/thread.hpp"
#if ESP_PLATFORM
#include <freertos/timers.h>
#else
#include <timers.h>
#endif

namespace freertos
{
#if (configUSE_TIMERS == 1)

timed_service::~timed_service()
{
    configASSERT(!this_cpu::is_in_isr());
    xTimerDelete(handle(), to_ticks(infinity));
}

timed_service::timed_service(function func, void* arg, tick_timer::duration period, bool periodic,
                             const char* name)
{
    configASSERT(!this_cpu::is_in_isr());
    xTimerCreateStatic(DEFAULT_NAME, to_ticks(period), periodic, arg,
                       reinterpret_cast<TimerCallbackFunction_t>(func), this);
}

const char* timed_service::get_name() const
{
    return pcTimerGetName(handle());
}

bool timed_service::is_reloading() const
{
    configASSERT(!this_cpu::is_in_isr());
    return uxTimerGetReloadMode(handle());
}

void timed_service::set_reloading(bool reloading)
{
    vTimerSetReloadMode(handle(), reloading);
}

tick_timer::time_point timed_service::get_trigger_time() const
{
    return tick_timer::time_point(tick_timer::duration(xTimerGetExpiryTime(handle())));
}

void* timed_service::get_owner() const
{
    configASSERT(!this_cpu::is_in_isr());
    return pvTimerGetTimerID(handle());
}

void timed_service::set_owner(void* owner)
{
    configASSERT(!this_cpu::is_in_isr());
    vTimerSetTimerID(handle(), owner);
}

bool timed_service::is_active() const
{
    configASSERT(!this_cpu::is_in_isr());
    return xTimerIsTimerActive(handle());
}

bool timed_service::start(tick_timer::duration waittime)
{
    if (!this_cpu::is_in_isr())
    {
        return xTimerStart(handle(), to_ticks(waittime));
    }
    else
    {
        // cannot wait in ISR
        configASSERT(to_ticks(waittime) == 0);

        BaseType_t needs_yield = false;
        bool success = xTimerStartFromISR(handle(), &needs_yield);
        portYIELD_FROM_ISR(needs_yield);
        return success;
    }
}

bool timed_service::stop(tick_timer::duration waittime)
{
    if (!this_cpu::is_in_isr())
    {
        return xTimerStop(handle(), to_ticks(waittime));
    }
    else
    {
        // cannot wait in ISR
        configASSERT(to_ticks(waittime) == 0);

        BaseType_t needs_yield = false;
        bool success = xTimerStopFromISR(handle(), &needs_yield);
        portYIELD_FROM_ISR(needs_yield);
        return success;
    }
}

bool timed_service::reset(tick_timer::duration waittime)
{
    if (!this_cpu::is_in_isr())
    {
        return xTimerReset(handle(), to_ticks(waittime));
    }
    else
    {
        // cannot wait in ISR
        configASSERT(to_ticks(waittime) == 0);

        BaseType_t needs_yield = false;
        bool success = xTimerResetFromISR(handle(), &needs_yield);
        portYIELD_FROM_ISR(needs_yield);
        return success;
    }
}

tick_timer::duration timed_service::get_period() const
{
    configASSERT(!this_cpu::is_in_isr());
    return tick_timer::duration(xTimerGetPeriod(handle()));
}

bool timed_service::set_period(tick_timer::duration new_period, tick_timer::duration waittime)
{
    if (!this_cpu::is_in_isr())
    {
        return xTimerChangePeriod(handle(), to_ticks(new_period), to_ticks(waittime));
    }
    else
    {
        // cannot wait in ISR
        configASSERT(to_ticks(waittime) == 0);

        BaseType_t needs_yield = false;
        bool success = xTimerChangePeriodFromISR(handle(), to_ticks(new_period), &needs_yield);
        portYIELD_FROM_ISR(needs_yield);
        return success;
    }
}

thread* timed_service::get_service_thread()
{
    return reinterpret_cast<thread*>(xTimerGetTimerDaemonTaskHandle());
}

#endif // (configUSE_TIMERS == 1)
} // namespace freertos
