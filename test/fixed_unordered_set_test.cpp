#include "fixed_containers/fixed_unordered_set.hpp"

#include "mock_testing_types.hpp"
#include "test_utilities_common.hpp"

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"

#include <gtest/gtest.h>
#include <range/v3/iterator/concepts.hpp>
#include <range/v3/view/filter.hpp>

#include <algorithm>
#include <cmath>

namespace fixed_containers
{
namespace
{
using ES_1 = FixedUnorderedSet<int, 10>;
static_assert(TriviallyCopyable<ES_1>);
static_assert(NotTrivial<ES_1>);
static_assert(StandardLayout<ES_1>);
static_assert(TriviallyCopyAssignable<ES_1>);
static_assert(TriviallyMoveAssignable<ES_1>);
static_assert(IsStructuralType<ES_1>);

static_assert(std::bidirectional_iterator<ES_1::iterator>);
static_assert(std::bidirectional_iterator<ES_1::const_iterator>);
static_assert(!std::random_access_iterator<ES_1::iterator>);
static_assert(!std::random_access_iterator<ES_1::const_iterator>);

static_assert(std::is_same_v<std::iter_value_t<ES_1::iterator>, int>);
static_assert(std::is_same_v<std::iter_reference_t<ES_1::iterator>, const int&>);
static_assert(std::is_same_v<std::iter_difference_t<ES_1::iterator>, std::ptrdiff_t>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::iterator>::pointer, const int*>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::iterator>::iterator_category,
                             std::bidirectional_iterator_tag>);

static_assert(std::is_same_v<std::iter_value_t<ES_1::const_iterator>, int>);
static_assert(std::is_same_v<std::iter_reference_t<ES_1::const_iterator>, const int&>);
static_assert(std::is_same_v<std::iter_difference_t<ES_1::const_iterator>, std::ptrdiff_t>);
static_assert(
    std::is_same_v<typename std::iterator_traits<ES_1::const_iterator>::pointer, const int*>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::const_iterator>::iterator_category,
                             std::bidirectional_iterator_tag>);

}  // namespace

TEST(FixedUnorderedSet, DefaultConstructor)
{
    constexpr FixedUnorderedSet<int, 10> s1{};
    static_assert(s1.empty());
}

TEST(FixedUnorderedSet, IteratorConstructor)
{
    constexpr std::array INPUT{2, 4};
    constexpr FixedUnorderedSet<int, 10> s2{INPUT.begin(), INPUT.end()};

    static_assert(s2.size() == 2);
    static_assert(s2.contains(2));
    static_assert(s2.contains(4));
}

TEST(FixedUnorderedSet, Initializer)
{
    constexpr FixedUnorderedSet<int, 10> s1{2, 4};
    static_assert(s1.size() == 2);

    constexpr FixedUnorderedSet<int, 10> s2{3};
    static_assert(s2.size() == 1);
}

// TEST(FixedUnorderedSet, Find_TransparentComparator)
// {
//     constexpr FixedUnorderedSet<MockAComparableToB, 3, std::less<>> s{};
//     constexpr MockBComparableToA b{5};
//     static_assert(s.find(b) == s.end());
// }

// TEST(FixedUnorderedSet, Contains)
// {
//     constexpr FixedUnorderedSet<int, 10> s1{2, 4};
//     static_assert(s1.size() == 2);

//     static_assert(!s1.contains(1));
//     static_assert(s1.contains(2));
//     static_assert(!s1.contains(3));
//     static_assert(s1.contains(4));
// }

// TEST(FixedUnorderedSet, Contains_TransparentComparator)
// {
//     constexpr FixedUnorderedSet<MockAComparableToB, 5, std::less<>> s{
//         MockAComparableToB{1}, MockAComparableToB{3}, MockAComparableToB{5}};
//     constexpr MockBComparableToA b{5};
//     static_assert(s.contains(b));
// }

// TEST(FixedUnorderedSet, Count_TransparentComparator)
// {
//     constexpr FixedUnorderedSet<MockAComparableToB, 5, std::less<>> s{
//         MockAComparableToB{1}, MockAComparableToB{3}, MockAComparableToB{5}};
//     constexpr MockBComparableToA b{5};
//     static_assert(s.count(b) == 1);
// }

// TEST(FixedUnorderedSet, LowerBound)
// {
//     constexpr FixedUnorderedSet<int, 10> s1{2, 4};
//     static_assert(s1.size() == 2);

//     static_assert(*s1.lower_bound(1) == 2);
//     static_assert(*s1.lower_bound(2) == 2);
//     static_assert(*s1.lower_bound(3) == 4);
//     static_assert(*s1.lower_bound(4) == 4);
//     static_assert(s1.lower_bound(5) == s1.cend());
// }

// TEST(FixedUnorderedSet, LowerBound_TransparentComparator)
// {
//     constexpr FixedUnorderedSet<MockAComparableToB, 5, std::less<>> s{
//         MockAComparableToB{1}, MockAComparableToB{3}, MockAComparableToB{5}};
//     constexpr MockBComparableToA b{3};
//     static_assert(*s.lower_bound(b) == MockAComparableToB{3});
// }

// TEST(FixedUnorderedSet, UpperBound)
// {
//     constexpr FixedUnorderedSet<int, 10> s1{2, 4};
//     static_assert(s1.size() == 2);

//     static_assert(*s1.upper_bound(1) == 2);
//     static_assert(*s1.upper_bound(2) == 4);
//     static_assert(*s1.upper_bound(3) == 4);
//     static_assert(s1.upper_bound(4) == s1.cend());
//     static_assert(s1.upper_bound(5) == s1.cend());
// }

// TEST(FixedUnorderedSet, UpperBound_TransparentComparator)
// {
//     constexpr FixedUnorderedSet<MockAComparableToB, 5, std::less<>> s{
//         MockAComparableToB{1}, MockAComparableToB{3}, MockAComparableToB{5}};
//     constexpr MockBComparableToA b{3};
//     static_assert(*s.upper_bound(b) == MockAComparableToB{5});
// }

// TEST(FixedUnorderedSet, EqualRange)
// {
//     constexpr FixedUnorderedSet<int, 10> s1{2, 4};
//     static_assert(s1.size() == 2);

//     static_assert(s1.equal_range(1).first == s1.lower_bound(1));
//     static_assert(s1.equal_range(1).second == s1.upper_bound(1));

//     static_assert(s1.equal_range(2).first == s1.lower_bound(2));
//     static_assert(s1.equal_range(2).second == s1.upper_bound(2));

//     static_assert(s1.equal_range(3).first == s1.lower_bound(3));
//     static_assert(s1.equal_range(3).second == s1.upper_bound(3));

//     static_assert(s1.equal_range(4).first == s1.lower_bound(4));
//     static_assert(s1.equal_range(4).second == s1.upper_bound(4));

//     static_assert(s1.equal_range(5).first == s1.lower_bound(5));
//     static_assert(s1.equal_range(5).second == s1.upper_bound(5));
// }

// TEST(FixedUnorderedSet, EqualRange_TransparentComparator)
// {
//     constexpr FixedUnorderedSet<MockAComparableToB, 5, std::less<>> s{
//         MockAComparableToB{1}, MockAComparableToB{3}, MockAComparableToB{5}};
//     constexpr MockBComparableToA b{3};
//     static_assert(s.equal_range(b).first == s.lower_bound(b));
//     static_assert(s.equal_range(b).second == s.upper_bound(b));
// }

TEST(FixedUnorderedSet, MaxSize)
{
    constexpr FixedUnorderedSet<int, 10> s1{2, 4};
    static_assert(s1.max_size() == 10);

    constexpr FixedUnorderedSet<int, 4> s2{};
    static_assert(s2.max_size() == 4);

    static_assert(FixedUnorderedSet<int, 4>::static_max_size() == 4);
    EXPECT_EQ(4, (FixedUnorderedSet<int, 4>::static_max_size()));
    static_assert(max_size_v<FixedUnorderedSet<int, 4>> == 4);
    EXPECT_EQ(4, (max_size_v<FixedUnorderedSet<int, 4>>));
}

TEST(FixedUnorderedSet, EmptySizeFull)
{
    constexpr FixedUnorderedSet<int, 10> s1{2, 4};
    static_assert(s1.size() == 2);
    static_assert(!s1.empty());

    constexpr FixedUnorderedSet<int, 10> s2{};
    static_assert(s2.size() == 0);
    static_assert(s2.empty());

    constexpr FixedUnorderedSet<int, 2> s3{2, 4};
    static_assert(s3.size() == 2);
    static_assert(is_full(s3));

    constexpr FixedUnorderedSet<int, 5> s4{2, 4};
    static_assert(s4.size() == 2);
    static_assert(!is_full(s4));
}

TEST(FixedUnorderedSet, MaxSizeDeduction)
{
    constexpr auto s1 = make_fixed_unordered_set({30, 31});
    static_assert(s1.size() == 2);
    static_assert(s1.max_size() == 2);
    static_assert(s1.contains(30));
    static_assert(s1.contains(31));
    static_assert(!s1.contains(32));
}

TEST(FixedUnorderedSet, Insert)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedSet<int, 10> s{};
        s.insert(2);
        s.insert(4);
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedUnorderedSet, Insert_ExceedsCapacity)
{
    {
        FixedUnorderedSet<int, 2> s1{};
        s1.insert(2);
        s1.insert(4);
        s1.insert(4);
        s1.insert(4);
        EXPECT_DEATH(s1.insert(6), "");
    }
    {
        FixedUnorderedSet<int, 2> s1{};
        s1.insert(2);
        s1.insert(4);
        s1.insert(4);
        s1.insert(4);
        int key = 6;
        EXPECT_DEATH(s1.insert(key), "");
    }
}

TEST(FixedUnorderedSet, InsertMultipleTimes)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedSet<int, 10> s{};
        {
            auto [it, was_inserted] = s.insert(2);
            assert_or_abort(was_inserted);
            assert_or_abort(2 == *it);
        }
        {
            auto [it, was_inserted] = s.insert(4);
            assert_or_abort(was_inserted);
            assert_or_abort(4 == *it);
        }
        {
            auto [it, was_inserted] = s.insert(2);
            assert_or_abort(!was_inserted);
            assert_or_abort(2 == *it);
        }
        {
            auto [it, was_inserted] = s.insert(4);
            assert_or_abort(!was_inserted);
            assert_or_abort(4 == *it);
        }
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedUnorderedSet, InsertInitializer)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedSet<int, 10> s{};
        s.insert({2, 4});
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedUnorderedSet, Insert_Iterators)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedSet<int, 10> s{};
        std::array<int, 2> a{2, 4};
        s.insert(a.begin(), a.end());
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));

    static_assert(std::is_same_v<decltype(*s1.begin()), const int&>);

    FixedUnorderedSet<int, 10> s_non_const{};
    static_assert(std::is_same_v<decltype(*s_non_const.begin()), const int&>);
}

TEST(FixedUnorderedSet, Emplace)
{
    {
        constexpr FixedUnorderedSet<int, 10> s = []()
        {
            FixedUnorderedSet<int, 10> s1{};
            s1.emplace(2);
            const int key = 2;
            s1.emplace(key);
            return s1;
        }();

        static_assert(consteval_compare::equal<1, s.size()>);
        static_assert(s.contains(2));
    }

    {
        FixedUnorderedSet<int, 10> s1{};

        {
            auto [it, was_inserted] = s1.emplace(2);

            ASSERT_EQ(1, s1.size());
            ASSERT_TRUE(!s1.contains(1));
            ASSERT_TRUE(s1.contains(2));
            ASSERT_TRUE(!s1.contains(3));
            ASSERT_TRUE(!s1.contains(4));
            ASSERT_TRUE(s1.contains(2));
            ASSERT_TRUE(was_inserted);
            ASSERT_EQ(2, *it);
        }

        {
            auto [it, was_inserted] = s1.emplace(2);
            ASSERT_EQ(1, s1.size());
            ASSERT_TRUE(!s1.contains(1));
            ASSERT_TRUE(s1.contains(2));
            ASSERT_TRUE(!s1.contains(3));
            ASSERT_TRUE(!s1.contains(4));
            ASSERT_TRUE(s1.contains(2));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(2, *it);
        }
    }
}

TEST(FixedUnorderedSet, Emplace_ExceedsCapacity)
{
    {
        FixedUnorderedSet<int, 2> s1{};
        s1.emplace(2);
        s1.emplace(4);
        s1.emplace(4);
        s1.emplace(4);
        EXPECT_DEATH(s1.emplace(6), "");
    }
    {
        FixedUnorderedSet<int, 2> s1{};
        s1.emplace(2);
        s1.emplace(4);
        s1.emplace(4);
        s1.emplace(4);
        int key = 6;
        EXPECT_DEATH(s1.emplace(key), "");
    }
}

TEST(FixedUnorderedSet, Clear)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedSet<int, 10> s{2, 4};
        s.clear();
        return s;
    }();

    static_assert(s1.empty());
}

TEST(FixedUnorderedSet, Erase)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedSet<int, 10> s{2, 4};
        auto removed_count = s.erase(2);
        assert_or_abort(removed_count == 1);
        removed_count = s.erase(3);
        assert_or_abort(removed_count == 0);
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(!s1.contains(1));
    static_assert(!s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedUnorderedSet, EraseIterator)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedSet<int, 10> s{2, 3, 4};
        {
            auto it = s.begin();
            auto next = s.erase(it);
            assert_or_abort(*next == 3);
        }

        {
            auto it = s.cbegin();
            auto next = s.erase(it);
            assert_or_abort(*next == 4);
        }
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(!s1.contains(1));
    static_assert(!s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedUnorderedSet, EraseIterator_Ambiguity)
{
    // If the iterator has extraneous auto-conversions, it might cause ambiguity between the various
    // overloads
    FixedUnorderedSet<std::string, 5> s1{};
    s1.erase("");
}

TEST(FixedUnorderedSet, EraseIterator_InvalidIterator)
{
    FixedUnorderedSet<int, 10> s{2, 4};
    {
        auto it = s.begin();
        std::advance(it, 2);
        EXPECT_DEATH(s.erase(it), "");
    }
}

TEST(FixedUnorderedSet, EraseRange)
{
    {
        constexpr auto s1 = []()
        {
            FixedUnorderedSet<int, 10> s{2, 3, 4};
            auto from = s.begin();
            std::advance(from, 1);
            auto to = s.begin();
            std::advance(to, 2);
            auto next = s.erase(from, to);
            assert_or_abort(*next == 4);
            return s;
        }();

        static_assert(consteval_compare::equal<2, s1.size()>);
        static_assert(!s1.contains(1));
        static_assert(s1.contains(2));
        static_assert(!s1.contains(3));
        static_assert(s1.contains(4));
    }
    {
        constexpr auto s1 = []()
        {
            FixedUnorderedSet<int, 10> s{2, 4};
            auto from = s.begin();
            auto to = s.begin();
            auto next = s.erase(from, to);
            assert_or_abort(*next == 2);
            return s;
        }();

        static_assert(consteval_compare::equal<2, s1.size()>);
        static_assert(!s1.contains(1));
        static_assert(s1.contains(2));
        static_assert(!s1.contains(3));
        static_assert(s1.contains(4));
    }
    {
        constexpr auto s1 = []()
        {
            FixedUnorderedSet<int, 10> s{1, 4};
            auto from = s.begin();
            auto to = s.end();
            auto next = s.erase(from, to);
            assert_or_abort(next == s.end());
            return s;
        }();

        static_assert(consteval_compare::equal<0, s1.size()>);
        static_assert(!s1.contains(1));
        static_assert(!s1.contains(2));
        static_assert(!s1.contains(3));
        static_assert(!s1.contains(4));
    }
}

TEST(FixedUnorderedSet, EraseIf)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedSet<int, 10> s{2, 3, 4};
        std::size_t removed_count =
            fixed_containers::erase_if(s, [](const auto& key) { return key == 2 or key == 4; });
        assert_or_abort(2 == removed_count);
        return s;
    }();

    static_assert(consteval_compare::equal<1, s1.size()>);
    static_assert(!s1.contains(1));
    static_assert(!s1.contains(2));
    static_assert(s1.contains(3));
    static_assert(!s1.contains(4));
}

TEST(FixedUnorderedSet, IteratorBasic)
{
    constexpr FixedUnorderedSet<int, 10> s1{1, 2, 3, 4};

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 4);

    static_assert(*s1.begin() == 1);
    static_assert(*std::next(s1.begin(), 1) == 2);
    static_assert(*std::next(s1.begin(), 2) == 3);
    static_assert(*std::next(s1.begin(), 3) == 4);

    static_assert(*std::prev(s1.end(), 1) == 4);
    static_assert(*std::prev(s1.end(), 2) == 3);
    static_assert(*std::prev(s1.end(), 3) == 2);
    static_assert(*std::prev(s1.end(), 4) == 1);
}

TEST(FixedUnorderedSet, Iterator_OffByOneIssues)
{
    constexpr FixedUnorderedSet<int, 10> s1{{1, 4}};

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 2);

    static_assert(*s1.begin() == 1);
    static_assert(*std::next(s1.begin(), 1) == 4);

    static_assert(*std::prev(s1.end(), 1) == 4);
    static_assert(*std::prev(s1.end(), 2) == 1);
}

TEST(FixedUnorderedSet, Iterator_EnsureOrder)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedSet<int, 10> s{};
        s.insert(3);
        s.insert(4);
        s.insert(1);
        return s;
    }();

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 3);

    static_assert(*s1.begin() == 3);
    static_assert(*std::next(s1.begin(), 1) == 4);
    static_assert(*std::next(s1.begin(), 2) == 1);

    static_assert(*std::prev(s1.end(), 1) == 1);
    static_assert(*std::prev(s1.end(), 2) == 4);
    static_assert(*std::prev(s1.end(), 3) == 3);
}

TEST(FixedUnorderedSet, ReverseIteratorBasic)
{
    constexpr FixedUnorderedSet<int, 10> s1{1, 2, 3, 4};

    static_assert(consteval_compare::equal<4, std::distance(s1.crbegin(), s1.crend())>);

    static_assert(*s1.rbegin() == 4);
    static_assert(*std::next(s1.rbegin(), 1) == 3);
    static_assert(*std::next(s1.crbegin(), 2) == 2);
    static_assert(*std::next(s1.rbegin(), 3) == 1);

    static_assert(*std::prev(s1.rend(), 1) == 1);
    static_assert(*std::prev(s1.crend(), 2) == 2);
    static_assert(*std::prev(s1.rend(), 3) == 3);
    static_assert(*std::prev(s1.rend(), 4) == 4);
}

TEST(FixedUnorderedSet, ReverseIteratorBase)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedSet<int, 7> s{1, 2, 3};
        auto it = s.rbegin();  // points to 3
        std::advance(it, 1);   // points to 2
        // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        s.erase(std::next(it).base());
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(s1.contains(1));
    static_assert(s1.contains(3));
}

TEST(FixedUnorderedSet, Equality)
{
    constexpr FixedUnorderedSet<int, 10> s1{{1, 4}};
    constexpr FixedUnorderedSet<int, 10> s2{{4, 1}};
    constexpr FixedUnorderedSet<int, 10> s3{{1, 3}};
    constexpr FixedUnorderedSet<int, 10> s4{1};

    static_assert(s1 == s2);
    static_assert(s2 == s1);

    static_assert(s1 != s3);
    static_assert(s3 != s1);

    static_assert(s1 != s4);
    static_assert(s4 != s1);
}

TEST(FixedUnorderedSet, Ranges)
{
    FixedUnorderedSet<int, 10> s1{1, 4};
    auto f = s1 | ranges::views::filter([](const auto& v) -> bool { return v == 4; });

    EXPECT_EQ(1, ranges::distance(f));
    EXPECT_EQ(4, *f.begin());
}

TEST(FixedUnorderedSet, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    FixedUnorderedSet a = FixedUnorderedSet<int, 5>{};
    (void)a;
}

TEST(FixedUnorderedSet, SetIntersection)
{
    constexpr FixedUnorderedSet<int, 10> s1 = []()
    {
        FixedUnorderedSet<int, 10> v1{1, 4};
        FixedUnorderedSet<int, 10> v2{1};

        FixedUnorderedSet<int, 10> v_intersection;
        std::set_intersection(v1.begin(),
                              v1.end(),
                              v2.begin(),
                              v2.end(),
                              std::inserter(v_intersection, v_intersection.begin()));
        return v_intersection;
    }();

    static_assert(consteval_compare::equal<1, s1.size()>);
    static_assert(s1.contains(1));
    static_assert(!s1.contains(4));
}

namespace
{
template <FixedUnorderedSet<int, 5> /*INSTANCE*/>
struct FixedUnorderedSetInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedUnorderedSet<int, 5> /*INSTANCE*/>
constexpr void fixed_unordered_set_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedUnorderedSet, UsageAsTemplateParameter)
{
    static constexpr FixedUnorderedSet<int, 5> INSTANCE1{};
    fixed_unordered_set_instance_can_be_used_as_a_template_parameter<INSTANCE1>();
    FixedUnorderedSetInstanceCanBeUsedAsATemplateParameter<INSTANCE1> my_struct{};
    static_cast<void>(my_struct);
}

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedUnorderedSet, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::FixedUnorderedSet<int, 5> a{};
    erase_if(a, [](int) { return true; });
    (void)is_full(a);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
