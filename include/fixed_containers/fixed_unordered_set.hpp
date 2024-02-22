#pragma once

#include "fixed_containers/wyhash.hpp"
#include "fixed_containers/fixed_robinhood_hashtable.hpp"
#include "fixed_containers/fixed_set_adapter.hpp"
#include "fixed_containers/set_checking.hpp"
#include "fixed_containers/concepts.hpp"

namespace fixed_containers
{

static constexpr std::size_t default_bucket_count(std::size_t value_count)
{
    // oversize the bucket array by 30%
    // TODO: think about the oversize percentage
    // TODO: round to a nearby power of 2 to improve modulus performance
    return (value_count * 130) / 100;
}

template <typename K,
          std::size_t MAXIMUM_VALUE_COUNT,
          std::size_t BUCKET_COUNT=default_bucket_count(MAXIMUM_VALUE_COUNT),
          class Hash=wyhash::hash<K>,
          class KeyEqual=std::equal_to<K>,
          customize::SetChecking<K> CheckingType = customize::SetAbortChecking<K, MAXIMUM_VALUE_COUNT>>
class FixedUnorderedSet : public FixedSetAdapter<K, fixed_robinhood_hashtable_detail::FixedRobinhoodHashtable<K, EmptyValue, MAXIMUM_VALUE_COUNT, BUCKET_COUNT, Hash, KeyEqual>, CheckingType>
{
    using FSA = FixedSetAdapter<K, fixed_robinhood_hashtable_detail::FixedRobinhoodHashtable<K, EmptyValue, MAXIMUM_VALUE_COUNT, BUCKET_COUNT, Hash, KeyEqual>, CheckingType>;
public:
    constexpr FixedUnorderedSet(const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual()) noexcept
    : FSA{hash, equal}
    {}

    template <InputIterator InputIt>
    constexpr FixedUnorderedSet(
        InputIt first,
        InputIt last,
        const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual(),
        const std_transition::source_location& loc = std_transition::source_location::current())
        : FixedUnorderedSet{hash, equal}
    {
        this->insert(first, last, loc);
    }

    constexpr FixedUnorderedSet(std::initializer_list<typename FixedUnorderedSet::value_type> list,
        const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual(),
        const std_transition::source_location& loc = std_transition::source_location::current())
        : FixedUnorderedSet{hash, equal}
    {
        this->insert(list, loc);
    }
};

/**
 * Construct a FixedUnorderedSet with its capacity being deduced from the number of key-value pairs being
 * passed.
 */
template <typename K,
          class Hash=wyhash::hash<K>,
          class KeyEqual=std::equal_to<K>,
          customize::SetChecking<K> CheckingType,
          std::size_t MAXIMUM_SIZE,
          std::size_t BUCKET_COUNT=default_bucket_count(MAXIMUM_SIZE),
          // Exposing this as a template parameter is useful for customization (for example with
          // child classes that set the CheckingType)
          typename FixedSetType =
              FixedUnorderedSet<K, MAXIMUM_SIZE, BUCKET_COUNT, Hash, KeyEqual, CheckingType>>
[[nodiscard]] constexpr FixedSetType make_fixed_unordered_set(
    const K (&list)[MAXIMUM_SIZE],
    const Hash& hash = Hash{},
    const KeyEqual& key_equal = KeyEqual{},
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    return {std::begin(list), std::end(list), hash, key_equal, loc};
}

template <typename K,
          class Hash=wyhash::hash<K>,
          class KeyEqual=std::equal_to<K>,
          std::size_t MAXIMUM_SIZE,
          std::size_t BUCKET_COUNT=default_bucket_count(MAXIMUM_SIZE)>
[[nodiscard]] constexpr auto make_fixed_unordered_set(const K (&list)[MAXIMUM_SIZE],
                                            const Hash& hash = Hash{},
                                            const KeyEqual& key_equal = KeyEqual{},
                                            const std_transition::source_location& loc =
                                                std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::SetAbortChecking<K, MAXIMUM_SIZE>;
    using FixedSetType =
        FixedUnorderedSet<K, MAXIMUM_SIZE, BUCKET_COUNT, Hash, KeyEqual, CheckingType>;
    return make_fixed_unordered_set<K,
                          Hash,
                          KeyEqual,
                          CheckingType,
                          MAXIMUM_SIZE,
                          BUCKET_COUNT,
                          FixedSetType>(list, hash, key_equal, loc);
}

}
