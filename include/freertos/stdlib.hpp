// SPDX-License-Identifier: MIT
#ifndef __FREERTOS_STDLIB_HPP_
#define __FREERTOS_STDLIB_HPP_

#include <mutex>
#include <condition_variable>
#if __has_include(<bit>)
#include <bit>
#else
#include <type_traits>
#endif

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

template <typename To, typename From>
#ifdef __cpp_lib_bit_cast
constexpr To bit_cast(const From& src)
{
    return std::bit_cast<To>(src);
}
#else
// https://en.cppreference.com/w/cpp/numeric/bit_cast.html
std::enable_if_t<sizeof(To) == sizeof(From) && std::is_trivially_copyable_v<From> &&
                     std::is_trivially_copyable_v<To>,
                 To>
bit_cast(const From& src) noexcept
{
    static_assert(std::is_trivially_constructible_v<To>,
                  "This implementation additionally requires "
                  "destination type to be trivially constructible");

    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}
#endif

} // namespace freertos

#endif // __FREERTOS_STDLIB_HPP_
