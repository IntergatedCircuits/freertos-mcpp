/**
 * @file      thread_owner.h
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
#ifndef __FREERTOS_THREAD_OWNER_H_
#define __FREERTOS_THREAD_OWNER_H_

#include "freertos/thread.h"
#include "freertos/semaphore.h"
#include <utility>

namespace freertos
{
    /// @brief  A class that approximates the std::thread API.
    class thread_owner
    {
    public:
        using id = thread::id;
        using native_handle_type = thread*;

        /// @brief  Constructs an empty thread owner.
        thread_owner()
        {
        }

        #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

            /// @brief  Constructs a dynamically allocated thread with the provided arguments.
            /// @note   FreeRTOS thread functions take only one argument, the following arguments
            ///         specify the thread's properties (stack size, priority, friendly name)
            template<class Function, class ... Args>
            thread_owner(Function&& f, Args&&... args)
                    : pthread_(thread::create(f, std::forward<Args>(args)...)), exit_sem_()
            {
                if (pthread_ != nullptr)
                {
                    // call will always succeed with a freshly created thread
                    (void) pthread_->set_exit_semaphore(&exit_sem_);
                }
            }

        #endif // (configSUPPORT_DYNAMIC_ALLOCATION == 1)

        /// @brief  Provides a unique identifier of the thread.
        /// @return The thread's unique identifier (0 is reserved as invalid)
        id get_id() const;

        /// @brief  Checks if the thread is joinable (potentially executing).
        /// @return true if the thread is valid and hasn't been joined, false otherwise
        bool joinable() const;

        /// @brief  Waits for the owned thread to finish execution.
        /// @note   May only be called when the thread is joinable, and not from the owned thread's context
        void join();

        /// @brief  Waits for the owned thread to finish execution.
        void detach();

        /// @brief  Number of concurrent threads supported.
        /// @return Fixed to 0 since FreeRTOS doesn't have such limitation by design,
        ///         only resource constraints apply
        static constexpr unsigned int hardware_concurrency()
        {
            return 0;
        }

        /// @brief  Provides access to the underlying thread's handle.
        /// @return Pointer to the owned thread
        native_handle_type native_handle() const
        {
            return pthread_;
        }

    private:
        thread *pthread_ = nullptr;
        binary_semaphore exit_sem_;

        // non-copyable
        thread_owner(const thread_owner&) = delete;
        thread_owner& operator=(const thread_owner&) = delete;
    };
}

#endif // __FREERTOS_THREAD_OWNER_H_
