// SPDX-License-Identifier: MIT
#include "freertos/event_group.hpp"
#include "freertos/cpu.hpp"
#if ESP_PLATFORM
#include <freertos/event_groups.h>
#include <freertos/task.h>
#else
#include <event_groups.h>
#include <task.h>
#endif

namespace freertos
{

event_group::event_group()
{
    configASSERT(!this_cpu::is_in_isr());
    xEventGroupCreateStatic(this);
}

event_group::~event_group()
{
    configASSERT(!this_cpu::is_in_isr());
    vEventGroupDelete(handle());
}

events event_group::get() const
{
    if (!this_cpu::is_in_isr())
    {
        return events(xEventGroupGetBits(handle()));
    }
    else
    {
        return events(xEventGroupGetBitsFromISR(handle()));
    }
}

void event_group::set(events flags)
{
    if (!this_cpu::is_in_isr())
    {
        (void)xEventGroupSetBits(handle(), flags);
    }
    else
    {
#if (configUSE_TIMERS == 1)

        BaseType_t needs_yield = false;
        (void)xEventGroupSetBitsFromISR(handle(), flags, &needs_yield);
        portYIELD_FROM_ISR(needs_yield);

#else

        configASSERT(false);

#endif // (configUSE_TIMERS == 1)
    }
}

void event_group::clear(events flags)
{
    if (!this_cpu::is_in_isr())
    {
        (void)xEventGroupClearBits(handle(), flags);
    }
    else
    {
#if (configUSE_TIMERS == 1)

        (void)xEventGroupClearBitsFromISR(handle(), flags);

#else

        configASSERT(false);

#endif // (configUSE_TIMERS == 1)
    }
}

events event_group::wait(events flags, const tick_timer::duration& rel_time, bool exclusive,
                         bool match_all)
{
    configASSERT(!this_cpu::is_in_isr());
    events setflags{xEventGroupWaitBits(handle(), flags, exclusive, match_all, to_ticks(rel_time))};
    // only return the flags that are relevant to the wait operation
    return flags & setflags;
}
} // namespace freertos
