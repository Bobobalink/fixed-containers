#pragma once

#include "fixed_containers/concepts.hpp"

#include <functional>
#include <type_traits>

namespace fixed_containers::reference_storage_detail
{
template <IsReference T>
struct ReferenceStorage
{
    using T0 = std::remove_reference_t<T>;
    std::reference_wrapper<T0> value;

    explicit constexpr ReferenceStorage(T v)
      : value{v}
    {
    }

    constexpr const T& get() const { return value.get(); }
    constexpr T& get() { return value.get(); }

    template <typename U>
    constexpr bool operator==(const ReferenceStorage<U>& rhs)
    {
      return get() == rhs.get();
    }

    // TODO: should we disable this being TriviallyCopyable to match unwrapped references? It's an artificial limitation
    // this doesn't work with existing `constexpr` uses of it inside of `FixedIndexBasedPoolStorage` (because of union access)

    // constexpr ReferenceStorage(const ReferenceStorage& other)
    // : value{other.value}
    // {
    // }
};
}  // namespace fixed_containers::value_storage_detail
