// SPDX-License-Identifier: MIT
#ifndef __FREERTOS_STDLIB_HPP_
#define __FREERTOS_STDLIB_HPP_

#include <mutex>
#include <condition_variable>

namespace freertos
{
template <typename _Mutex>
using lock_guard = std::lock_guard<_Mutex>;

template <typename _Mutex>
using unique_lock = std::unique_lock<_Mutex>;

enum class cv_status
{
    no_timeout,
    timeout
};
} // namespace freertos

#endif // __FREERTOS_STDLIB_HPP_
