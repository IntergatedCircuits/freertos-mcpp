/**
 * @file      thread_owner.cpp
 * @brief     std::thread approximating API
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
#include "freertos/thread_owner.h"
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

thread_owner::id thread_owner::get_id() const
{
    if (pthread_ != nullptr)
    {
        return pthread_->get_id();
    }
    else
    {
        return id(0);
    }
}

bool thread_owner::joinable() const
{
    return pthread_ != nullptr;
}

void thread_owner::join()
{
    configASSERT(joinable()); // else invalid_argument
    configASSERT(this->get_id() != freertos::this_thread::get_id()); // else resource_deadlock_would_occur

    // wait for signal from thread exit
    exit_sem_.acquire();

    // signal received, thread is deleted, mark it so
    pthread_ = nullptr;
}

void thread_owner::detach()
{
    if (joinable())
    {
        // if getting the semaphore fails, the thread is still alive
        if (!exit_sem_.try_acquire())
        {
            // unlink exit semaphore
            (void)pthread_->clear_exit_semaphore(&exit_sem_);
        }
        // release the thread itself
        pthread_ = nullptr;
    }
}
