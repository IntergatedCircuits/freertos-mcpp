/**
 * @file      mutex.cpp
 * @brief     FreeRTOS mutex API abstraction
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
#include "freertos/mutex.h"
#include "freertos/cpu.h"
#include "freertos/thread.h"

namespace freertos
{
    namespace native
    {
        #include "semphr.h"
    }
}
using namespace freertos;
using namespace freertos::native;

#if (configUSE_MUTEXES == 1)

    void mutex::unlock()
    {
        // the same thread must unlock the mutex that has locked it
        configASSERT(get_locking_thread()->get_id() == this_thread::get_id());

        semaphore::release();
    }

    mutex::mutex()
    {
        // construction not allowed in ISR
        configASSERT(!this_cpu::is_in_isr());

        xSemaphoreCreateMutexStatic(this);
    }

    #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

        mutex *mutex::create()
        {
            // construction not allowed in ISR
            configASSERT(!this_cpu::is_in_isr());

            return reinterpret_cast<mutex*>(xSemaphoreCreateMutex());
        }

    #endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)

    #if (configUSE_RECURSIVE_MUTEXES == 1)

        void recursive_mutex::unlock()
        {
            // the same thread must unlock the mutex that has locked it
            configASSERT(get_locking_thread()->get_id() == this_thread::get_id());

            semaphore::release();
        }

        recursive_mutex::recursive_mutex()
        {
            // construction not allowed in ISR
            configASSERT(!this_cpu::is_in_isr());

            xSemaphoreCreateRecursiveMutexStatic(this);
        }

        #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

            recursive_mutex *recursive_mutex::create()
            {
                // construction not allowed in ISR
                configASSERT(!this_cpu::is_in_isr());

                return reinterpret_cast<recursive_mutex*>(xSemaphoreCreateRecursiveMutex());
            }

        #endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)

    #endif // (configUSE_RECURSIVE_MUTEXES == 1)

#endif // (configUSE_MUTEXES == 1)
