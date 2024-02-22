#pragma once

#include "fixed_containers/pair.hpp"
#include "fixed_containers/fixed_doubly_linked_list.hpp"
#include "fixed_containers/value_or_reference_storage.hpp"

#include <cstring>
#include <functional>
#include <memory>
#include <array>
#include <iostream>

// This is a modified version of the dense hashmap from https://github.com/martinus/unordered_dense, reimplmeneted to exist nicely in the fixed-containers universe.

// original license:
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2022-2023 Martin Leitner-Ankerl <martin.ankerl@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

namespace fixed_containers::fixed_robinhood_hashtable_detail
{

struct Bucket {
    // control how many bits to use for the hash fingerprint. The rest are used as the distance between this element and its "ideal" location in the table
    static constexpr std::uint32_t FINGERPRINT_BITS = 8;

    static constexpr std::uint32_t DIST_INC = 1U << FINGERPRINT_BITS;
    static constexpr std::uint32_t FINGERPRINT_MASK = DIST_INC - 1;

    std::uint32_t dist_and_fingerprint_;
    std::uint32_t value_index_;

    constexpr std::uint32_t dist() const {
        return dist_and_fingerprint_ >> FINGERPRINT_BITS;
    }

    constexpr std::uint32_t fingerprint() const {
        return dist_and_fingerprint_ & FINGERPRINT_MASK;
    }

    [[nodiscard]] static constexpr std::uint32_t dist_and_fingerprint_from_hash(std::uint64_t hash) {
        return DIST_INC | (static_cast<std::uint32_t>(hash) & FINGERPRINT_MASK);
    }

    [[nodiscard]] static constexpr std::uint32_t increment_dist(std::uint32_t dist_and_fingerprint) {
        return dist_and_fingerprint + DIST_INC;
    }
    
    [[nodiscard]] static constexpr std::uint32_t decrement_dist(std::uint32_t dist_and_fingerprint) {
        return dist_and_fingerprint - DIST_INC;
    }

    [[nodiscard]] constexpr Bucket plus_dist() const {
        return {increment_dist(dist_and_fingerprint_), value_index_};
    }

    [[nodiscard]] constexpr Bucket minus_dist() const {
        return {decrement_dist(dist_and_fingerprint_), value_index_};
    }
};

template <typename K,
          typename V,
          std::size_t MAXIMUM_VALUE_COUNT,
          std::size_t BUCKET_COUNT,
          class Hash,
          class KeyEqual>
class FixedRobinhoodHashtable
{
public:
    using PairType = Pair<K, value_or_reference_storage_detail::ValueOrReferenceStorage<V>>;
    using HashType = Hash;
    using KeyEqualType = KeyEqual;
    // TODO: sanify the size types...
    using SizeType = std::size_t;

    Hash IMPLEMENTATION_DETAIL_DO_NOT_USE_hash_{};
    KeyEqual IMPLEMENTATION_DETAIL_DO_NOT_USE_key_equal_{};

    static_assert(MAXIMUM_VALUE_COUNT <= BUCKET_COUNT, "need at least enough buckets to point to every value in array");
    static constexpr std::size_t MAXIMUM_NUM_ENTRIES = MAXIMUM_VALUE_COUNT;
    static constexpr std::size_t INTERNAL_TABLE_SIZE = BUCKET_COUNT;

    fixed_doubly_linked_list_detail::FixedDoublyLinkedList<PairType, MAXIMUM_NUM_ENTRIES> IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_{};
    
    std::array<Bucket, INTERNAL_TABLE_SIZE> IMPLEMENTATION_DETAIL_DO_NOT_USE_bucket_array_{};

    static constexpr std::size_t CAPACITY = MAXIMUM_NUM_ENTRIES;

    struct OpaqueIndexType
    {
        SizeType bucket_index;
        // we need a dist_and_fingerprint for emplace(), but not for checks where the value exists.
        // We make this field pull double duty by setting it to 0 for keys that exist, but the valid dist_and_fingerprint for those that don't.
        std::uint32_t dist_and_fingerprint;
    };
    
    using OpaqueIteratedType = SizeType;

////////////////////// helper functions
public:
    [[nodiscard]] constexpr Bucket& bucket_at(SizeType idx) {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_bucket_array_[idx];
    }
    [[nodiscard]] constexpr const Bucket& bucket_at(SizeType idx) const {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_bucket_array_[idx];
    }

    template <typename Key>
    [[nodiscard]] constexpr std::uint64_t hash(const Key& k) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_hash_(k);
    }

    template <typename K1, typename K2>
    [[nodiscard]] constexpr bool key_equal(const K1& k1, const K2& k2) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_equal_(k1, k2);
    }

    [[nodiscard]] static constexpr SizeType bucket_index_from_hash(std::uint64_t hash) {
        // Shift the hash right so that the bits of the hash used to compute the bucket index are totally distinct from the bits used in the fingerprint.
        // Without this, the fingerprint would tend to be totally useless as it encodes information that the resident index of the bucket also encodes.
        // This does not restrict the size of the table because we store the value_index in 32 bits, so the 56 left in this hash are plenty for our needs.
        std::uint64_t shifted_hash = hash >> Bucket::FINGERPRINT_BITS;
        return static_cast<SizeType>(shifted_hash % INTERNAL_TABLE_SIZE);
    }

    [[nodiscard]] static constexpr SizeType next_bucket_index(SizeType bucket_index) {
        if(bucket_index + 1 < INTERNAL_TABLE_SIZE)
        {
            return bucket_index + 1;
        }
        else {
            return 0;
        }
    }

    constexpr void place_and_shift_up(Bucket bucket, SizeType table_loc) {
        // replace the current bucket at the location with the given bucket, bubbling up elements until we hit an empty one
        while (0 != bucket_at(table_loc).dist_and_fingerprint_)
        {
            bucket = std::exchange(bucket_at(table_loc), bucket);
            bucket = bucket.plus_dist();
            table_loc = next_bucket_index(table_loc);
        }
        bucket_at(table_loc) = bucket;
    }

    constexpr void erase_bucket(const OpaqueIndexType& i)
    {
        SizeType table_loc = i.bucket_index;
        SizeType value_index = bucket_at(table_loc).value_index_;
 
        // shift down until either empty or an element with correct spot is found
        SizeType next_loc = next_bucket_index(table_loc);
        while(bucket_at(next_loc).dist_and_fingerprint_ >= Bucket::DIST_INC * 2)
        {
            bucket_at(table_loc) = bucket_at(next_loc).minus_dist();
            table_loc = std::exchange(next_loc, next_bucket_index(next_loc));
        }
        bucket_at(table_loc) = {};
    }

    constexpr SizeType erase_value(SizeType value_index)
    {
        SizeType next = IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.delete_at_and_return_next_index(value_index);

        return next;
    }

//////////////////////// Common Interface Impl
public:
    [[nodiscard]] constexpr SizeType size() const {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.size();
    }

    constexpr OpaqueIteratedType begin_index() const {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.front_index();
    }

    static constexpr OpaqueIteratedType invalid_index() {
        return decltype(IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_)::NULL_INDEX;
    }

    constexpr OpaqueIteratedType end_index() const {
        return invalid_index();
    }

    constexpr OpaqueIteratedType advance(const OpaqueIteratedType& value_index) const {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.next_of(value_index);
    }

    constexpr OpaqueIteratedType recede(const OpaqueIteratedType& value_index) const {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.prev_of(value_index);
    }

    constexpr const K& key_at(const OpaqueIteratedType& value_index) const {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.at(value_index).first;
    }

    constexpr const V& value_at(const OpaqueIteratedType& value_index) const
        requires IsNotEmpty<V>
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.at(value_index).second.get();
    }

    constexpr V& value_at(const OpaqueIteratedType& value_index)
        requires IsNotEmpty<V>
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.at(value_index).second.get();
    }

    constexpr OpaqueIteratedType iterated_index_from(const OpaqueIndexType& i) const {
        return bucket_at(i.bucket_index).value_index_;
    }

    constexpr OpaqueIndexType opaque_index_of(const K& k) const
    {
        std::uint64_t h = hash(k);
        std::uint32_t dist_and_fingerprint = Bucket::dist_and_fingerprint_from_hash(h);
        SizeType table_loc= bucket_index_from_hash(h);
        Bucket bucket = bucket_at(table_loc);

        while (true)
        {
            if(bucket.dist_and_fingerprint_ == dist_and_fingerprint && key_equal(k, key_at(bucket.value_index_)))
            {
                return {table_loc, 0};
            }
            // If we found a bucket that is closer to its "ideal" location than we would be if we matched, then it is impossible that the key will show up.
            // This check also triggers when we find an empty bucket.
            // Note that this is also the location that we will insert the key if it ends up getting inserted.
            else if (dist_and_fingerprint > bucket.dist_and_fingerprint_)
            {
                return {table_loc, dist_and_fingerprint};
            }
            dist_and_fingerprint = Bucket::increment_dist(dist_and_fingerprint);
            table_loc = next_bucket_index(table_loc);
            bucket = bucket_at(table_loc);
        }
    }

    constexpr bool exists(const OpaqueIndexType& i) const
    {
        // TODO: should we check if the index makes sense/points to a real place?
        return i.dist_and_fingerprint == 0;
    }

    constexpr const V& value(const OpaqueIndexType& i) const
        requires IsNotEmpty<V>
    {
        // no safety checks
        return value_at(bucket_at(i.bucket_index).value_index_);
    }

    constexpr V& value(const OpaqueIndexType& i)
        requires IsNotEmpty<V>
    {
        // no safety checks
        return value_at(bucket_at(i.bucket_index).value_index_);
    }

    template <typename... Args>
    constexpr OpaqueIndexType emplace(const OpaqueIndexType& i, Args&&... args)
    {
        SizeType value_loc = IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.emplace_back_and_return_index(std::forward<Args>(args)...);

        // place the bucket at the correct location
        place_and_shift_up(Bucket{i.dist_and_fingerprint, value_loc}, i.bucket_index);
        return {i.bucket_index, 0};
    }

    constexpr OpaqueIteratedType erase(const OpaqueIndexType& i)
    {
        SizeType value_index = bucket_at(i.bucket_index).value_index_;
 
        erase_bucket(i);
        SizeType next_index = erase_value(value_index);

        return next_index;
    }

    constexpr OpaqueIteratedType erase_range(const OpaqueIteratedType& start_value_index, const OpaqueIteratedType& end_value_index)
    {
        SizeType cur_index = start_value_index;
        while(cur_index != end_value_index)
        {
            cur_index = erase(opaque_index_of(key_at(cur_index)));
        }

        return end_value_index;
    }

    constexpr void clear()
    {
        erase_range(begin_index(), end_index());
    }
public:
    constexpr FixedRobinhoodHashtable() = default;

    constexpr FixedRobinhoodHashtable(const Hash& hash, const KeyEqual& equal = KeyEqual())
    : IMPLEMENTATION_DETAIL_DO_NOT_USE_hash_(hash)
    , IMPLEMENTATION_DETAIL_DO_NOT_USE_key_equal_(equal)
    {
    }

    // disable trivial copyability when using reference value types
    // this is an artificial limitation needed beacause `std::reference_wrapper` is trivially copyable
    constexpr FixedRobinhoodHashtable(const FixedRobinhoodHashtable& other)
        requires IsReference<V>
    : IMPLEMENTATION_DETAIL_DO_NOT_USE_hash_(other.IMPLEMENTATION_DETAIL_DO_NOT_USE_hash_)
    , IMPLEMENTATION_DETAIL_DO_NOT_USE_key_equal_(other.IMPLEMENTATION_DETAIL_DO_NOT_USE_key_equal_)
    , IMPLEMENTATION_DETAIL_DO_NOT_USE_bucket_array_(other.IMPLEMENTATION_DETAIL_DO_NOT_USE_bucket_array_)
    , IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_(other.IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_)
    {
    }
    constexpr FixedRobinhoodHashtable(const FixedRobinhoodHashtable& other) requires (!IsReference<V>) = default;
};

} // namespace fixed_containers::fixed_robinhood_hashtable_detail
