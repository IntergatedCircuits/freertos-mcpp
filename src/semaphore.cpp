/**
 * @file      semaphore.cpp
 * @brief     FreeRTOS semaphore API abstraction
 * @author    Benedek Kupper
 *
 * Copyright (c) 2020 Benedek Kupper
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
#include "freertos/semaphore.h"
#include "freertos/cpu.h"
#include <cstring>

namespace freertos
{
    namespace native
    {
        #include "semphr.h"
    }
}
using namespace freertos;
using namespace freertos::native;

#if 0 // doesn't seem like a good idea
semaphore::semaphore(semaphore&& other)
{
    std::memmove(this, &other, sizeof(semaphore));
    std::memset(&other, 0, sizeof(semaphore));
}

semaphore& semaphore::operator=(semaphore&& other)
{
    if (this != &other)
    {
        std::memmove(this, &other, sizeof(semaphore));
        std::memset(&other, 0, sizeof(semaphore));
    }
    return *this;
}
#endif

bool semaphore::take(tick_timer::duration timeout)
{
    if (!this_cpu::is_in_isr())
    {
        return xSemaphoreTake(handle(), to_ticks(timeout));
    }
    else
    {
        // cannot wait in ISR
        configASSERT(to_ticks(timeout) == 0);

        BaseType_t needs_yield = false;
        bool success = xSemaphoreTakeFromISR(handle(), &needs_yield);
        portYIELD_FROM_ISR(needs_yield);
        return success;
    }
}

bool semaphore::give(count_type update)
{
    // the API only allows giving a single count
    if (!this_cpu::is_in_isr())
    {
        bool success = true;
        while (success && (update > 0))
        {
            success = xSemaphoreGive(handle());
            update--;
        }
        return success;
    }
    else
    {
        BaseType_t needs_yield = false;
        bool success = true;
        while (success && (update > 0))
        {
            success = xSemaphoreGiveFromISR(handle(), &needs_yield);
            update--;
        }
        portYIELD_FROM_ISR(needs_yield);
        return success;
    }
}

thread *semaphore::get_mutex_holder() const
{
    if (!this_cpu::is_in_isr())
    {
        return reinterpret_cast<thread*>(xSemaphoreGetMutexHolder(handle()));
    }
    else
    {
        return reinterpret_cast<thread*>(xSemaphoreGetMutexHolderFromISR(handle()));
    }
}

#if (configUSE_COUNTING_SEMAPHORES == 1)

    semaphore::semaphore(count_type max, count_type desired)
    {
        // construction not allowed in ISR
        configASSERT(!this_cpu::is_in_isr());

        (void)xSemaphoreCreateCountingStatic(max, desired, this);
    }

    #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

        semaphore* semaphore::create_counting(count_type max, count_type desired)
        {
            // construction not allowed in ISR
            configASSERT(!this_cpu::is_in_isr());

            return reinterpret_cast<semaphore*>(xSemaphoreCreateCounting(max, desired));
        }

    #endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)

#endif // (configUSE_COUNTING_SEMAPHORES == 1)

binary_semaphore::binary_semaphore(count_type desired)
{
    // construction not allowed in ISR
    configASSERT(!this_cpu::is_in_isr());

    (void)xSemaphoreCreateBinaryStatic(this);
    give(desired);
}

#if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

    binary_semaphore *binary_semaphore::create(count_type desired)
    {
        // construction not allowed in ISR
        configASSERT(!this_cpu::is_in_isr());

        binary_semaphore *s = reinterpret_cast<binary_semaphore*>(xSemaphoreCreateBinary());
        if (s != nullptr)
        {
            s->give(desired);
        }
        return s;
    }

#endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)
