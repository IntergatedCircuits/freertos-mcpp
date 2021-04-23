/**
 * @file      timed_service.h
 * @brief     FreeRTOS timer service API
 * @author    Benedek Kupper
 *
 * Copyright (c) 2021 Benedek Kupper
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
#ifndef __FREERTOS_TIMED_SERVICE_H_
#define __FREERTOS_TIMED_SERVICE_H_

#include "freertos/tick_timer.h"

namespace freertos
{
    namespace native
    {
        struct tmrTimerControl;
    }

    class thread;

    #if (configUSE_TIMERS == 1)

        class timed_service : protected native::StaticTimer_t
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
            bool set_period(tick_timer::duration new_period, tick_timer::duration waittime = tick_timer::duration(0));

            void *get_owner() const;
            void set_owner(void *owner);

            tick_timer::time_point get_trigger_time() const;

            const char* get_name() const;

            timed_service(function func, void *owner, tick_timer::duration period,
                    bool reloading, const char *name = DEFAULT_NAME);

        private:
            static constexpr const char* DEFAULT_NAME = "anonym";

            native::tmrTimerControl* handle() const
            {
                return reinterpret_cast<native::tmrTimerControl*>(const_cast<timed_service*>(this));
            }

            static thread *get_service_thread();

            // non-copyable
            timed_service(const timed_service&) = delete;
            timed_service& operator=(const timed_service&) = delete;
            // non-movable
            timed_service(const timed_service&&) = delete;
            timed_service& operator=(const timed_service&&) = delete;
        };

    #endif // (configUSE_TIMERS == 1)
}

#endif // __FREERTOS_TIMED_SERVICE_H_
