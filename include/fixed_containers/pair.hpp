#pragma once

#include "fixed_containers/concepts.hpp"

#include <utility>
#include <memory>

namespace fixed_containers
{
// std::pair is not std::trivially_copyable and/or not a structural type (see tests).
// This is used for certain cases where we need a pair but std::pair won't work,
// for example in having instances of pairs as template parameters.
// Fixed containers do not have std::pair in their storage for this reason.
template <typename T1, typename T2>
struct Pair
{
    using first_type = T1;
    using second_type = T2;

    T1 first;
    T2 second;

    constexpr Pair() = default;
    
    template <class U1, class... Args>
    constexpr Pair(U1&& u1, Args&&... args)
        : first{std::forward<U1>(u1)}
        , second{std::forward<Args>(args)...}
    {
    }

    template <class... Args1, class... Args2>
    constexpr Pair(std::piecewise_construct_t, std::tuple<Args1...> first_args, std::tuple<Args2...> second_args)
    {
        std::apply([&](Args1&&... args1) { std::construct_at(&first, std::forward<Args1>(args1)...); },
                  std::move(first_args));
        std::apply([&](Args2&&... args2) { std::construct_at(&second, std::forward<Args2>(args2)...); },
                  std::move(second_args));
    }

    constexpr operator std::pair<T1, T2>() const { return {first, second}; }

    template <class U1, class U2>
    constexpr operator std::pair<U1, U2>() const
    {
        return {first, second};
    }
    template <class U1, class U2>
    constexpr operator std::pair<U1, U2>()
    {
        return {first, second};
    }

    template <typename U1, typename U2>
    constexpr bool operator==(const Pair<U1, U2>& rhs) const
    {
        return first == rhs.first && second == rhs.second;
    }
};

template <typename T1, IsEmpty T2>
struct Pair<T1, T2>
{
    using first_type = T1;

    T1 first;

    constexpr Pair() = default;
    
    template <class U1>
    constexpr Pair(U1&& u1)
        : first{std::forward<U1>(u1)}
    {
    }

    template <class... Args>
    constexpr Pair(Args&&... args)
    : first{std::forward<Args>(args)...}
    {
    }

    constexpr operator T1() const { return first; }

    template <typename U1>
    constexpr operator U1() const
    {
        return first;
    }

    template <typename U1>
    constexpr operator U1()
    {
        return first;
    }

    template <typename U1>
    constexpr bool operator==(const Pair<U1, void>& rhs) const
    {
        return first == rhs.first;
    }
};

template <typename T1, typename T2>
Pair(T1 t1, T2 t2) -> Pair<T1, T2>;

}  // namespace fixed_containers
