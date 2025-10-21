// SPDX-License-Identifier: MIT
#include "freertos/queue.hpp"
#include "freertos/cpu.hpp"
#if ESP_PLATFORM
#include <freertos/queue.h>
#else
#include <queue.h>
#endif

namespace freertos
{
queue::size_type queue::size() const
{
    if (!this_cpu::is_in_isr())
    {
        return uxQueueMessagesWaiting(handle());
    }
    else
    {
        return uxQueueMessagesWaitingFromISR(handle());
    }
}

queue::size_type queue::available() const
{
    // no ISR API available
    configASSERT(!this_cpu::is_in_isr());
    {
        return uxQueueSpacesAvailable(handle());
    }
}

bool queue::full() const
{
    if (!this_cpu::is_in_isr())
    {
        return uxQueueMessagesWaiting(handle()) == 0;
    }
    else
    {
        return xQueueIsQueueFullFromISR(handle());
    }
}

bool queue::empty() const
{
    if (!this_cpu::is_in_isr())
    {
        return uxQueueSpacesAvailable(handle()) == 0;
    }
    else
    {
        return xQueueIsQueueEmptyFromISR(handle());
    }
}

#if 0
queue::size_type queue::max_size() const
{
    // no public API available to read the queue length
    // doing size() + available() could lead to races
    return base_.uxDummy4[1];
}
#endif

void queue::reset()
{
    // no ISR API available
    configASSERT(!this_cpu::is_in_isr());
    {
        // call always succeeds
        (void)xQueueReset(handle());
    }
}

queue::~queue()
{
    // destruction not allowed in ISR
    configASSERT(!this_cpu::is_in_isr());

    vQueueDelete(handle());
}

bool queue::push_front(void* data, tick_timer::duration waittime)
{
    if (!this_cpu::is_in_isr())
    {
        return xQueueSendToFront(handle(), data, to_ticks(waittime));
    }
    else
    {
        // cannot wait in ISR
        configASSERT(to_ticks(waittime) == 0);

        BaseType_t needs_yield = false;
        bool success = xQueueSendToFrontFromISR(handle(), data, &needs_yield);
        portYIELD_FROM_ISR(needs_yield);
        return success;
    }
}

bool queue::push_back(void* data, tick_timer::duration waittime)
{
    if (!this_cpu::is_in_isr())
    {
        return xQueueSendToBack(handle(), data, to_ticks(waittime));
    }
    else
    {
        // cannot wait in ISR
        configASSERT(to_ticks(waittime) == 0);

        BaseType_t needs_yield = false;
        bool success = xQueueSendToBackFromISR(handle(), data, &needs_yield);
        portYIELD_FROM_ISR(needs_yield);
        return success;
    }
}

void queue::replace(void* data)
{
    if (!this_cpu::is_in_isr())
    {
        bool success = xQueueOverwrite(handle(), data);
        configASSERT(success);
    }
    else
    {
        BaseType_t needs_yield = false;
        bool success = xQueueOverwriteFromISR(handle(), data, &needs_yield);
        configASSERT(success);
        portYIELD_FROM_ISR(needs_yield);
    }
}

bool queue::peek_front(void* data, tick_timer::duration waittime) const
{
    if (!this_cpu::is_in_isr())
    {
        return xQueuePeek(handle(), data, to_ticks(waittime));
    }
    else
    {
        return xQueuePeekFromISR(handle(), data);
    }
}

bool queue::pop_front(void* data, tick_timer::duration waittime)
{
    if (!this_cpu::is_in_isr())
    {
        return xQueueReceive(handle(), data, to_ticks(waittime));
    }
    else
    {
        // cannot wait in ISR
        configASSERT(to_ticks(waittime) == 0);

        BaseType_t needs_yield = false;
        bool success = xQueueReceiveFromISR(handle(), data, &needs_yield);
        portYIELD_FROM_ISR(needs_yield);
        return success;
    }
}

queue::queue(size_type size, size_type elem_size, unsigned char* elem_buffer)
{
    // construction not allowed in ISR
    configASSERT(!this_cpu::is_in_isr());

    (void)xQueueCreateStatic(size, elem_size, elem_buffer, this);
}

#if 0 && (configSUPPORT_DYNAMIC_ALLOCATION == 1)

    queue* queue::create(size_type size, size_type elem_size)
    {
        // construction not allowed in ISR
        configASSERT(!this_cpu::is_in_isr());

        return reinterpret_cast<queue*>(xQueueCreate(size, elem_size));
    }

#endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)
} // namespace freertos
