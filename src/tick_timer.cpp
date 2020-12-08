/**
 * @file      tick_timer.cpp
 * @brief     FreeRTOS tick timer related API abstraction
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
#include "freertos/tick_timer.h"
#include "freertos/cpu.h"

namespace freertos
{
    namespace native
    {
        #include "task.h"
    }
}
using namespace freertos;
using namespace freertos::native;

tick_timer::time_point tick_timer::now()
{
    rep ticks;
    if (!this_cpu::is_in_isr())
    {
        ticks = xTaskGetTickCount();
    }
    else
    {
        ticks = xTaskGetTickCountFromISR();
    }

    return time_point(duration(ticks));
}
