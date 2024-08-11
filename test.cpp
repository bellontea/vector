#include <gtest/gtest.h>

#include <deque>
#include <vector>
#include <random>
#include "vector.h"

template<class T>
class TestAllocator
{
    using stdAlloc = std::allocator<T>;

public:
    using value_type = stdAlloc::value_type;
    using size_type = stdAlloc::size_type;
    using propagate_on_container_move_assignment = stdAlloc::propagate_on_container_move_assignment;

    constexpr TestAllocator() noexcept : m_allocator(){}
    constexpr TestAllocator(const TestAllocator &other) noexcept : m_allocator(other.m_allocator){}

    template<class U>
    constexpr explicit TestAllocator(const TestAllocator<U> &other) noexcept : m_allocator(other.m_allocator){}

    constexpr ~TestAllocator(){
        EXPECT_EQ(0 , counter);
    }

    constexpr auto allocate(size_type n) {counter += n; return m_allocator.allocate(n);}
    constexpr auto deallocate(value_type *p, size_type n) {counter -= n; return m_allocator.deallocate(p, n);}

private:
    stdAlloc m_allocator;
    size_t counter = 0;
};

template <typename Vector>
class TestsVector : public testing::Test
{
    using BaseClass = testing::Test;
};

class TestVar
{
public:
    explicit TestVar(bool isRandom = false)
    {
        if (!isRandom)
            return;

        auto seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::mt19937 rng(seed);

        using random_bytes_engine = std::independent_bits_engine<
                std::mt19937, CHAR_BIT, char8_t>;

        random_bytes_engine rbe(rng);
        _intVal = rbe();
        std::generate(_stringVal.begin(), _stringVal.end(), rbe);
    }

    TestVar(const TestVar& other) = default;
    TestVar(TestVar&& other) noexcept
    {operator=(std::move(other));}

    TestVar& operator=(const TestVar& other) noexcept = default;
    TestVar& operator=(TestVar&& other) noexcept
    {
        _intVal = other._intVal;
        _stringVal = std::move(other._stringVal);
        return *this;
    }

    bool operator==(const TestVar &other) const
    {
        return _intVal == other._intVal && _stringVal == other._stringVal;
    }
    int _intVal{};
    std::string _stringVal;
};

using TestTypes = ::testing::Types<std::vector<int, TestAllocator<int>>, const std::vector<bool>, std::vector<TestVar>,
        containers::vector<int>>;
TYPED_TEST_SUITE(TestsVector, TestTypes);

TYPED_TEST(TestsVector, AddingValueTest)
{
    using value_type = typename TypeParam::value_type;
    if constexpr (!std::is_const_v<TypeParam>)
    {
        {
            TypeParam testVector;
            value_type test_type = value_type();
            if constexpr (std::is_same_v<value_type, TestVar>)
                test_type = std::move(value_type(true));

            testVector.push_back(test_type);
            ASSERT_EQ(test_type, testVector[0]);
            EXPECT_EQ(1, testVector.size());
        }

        {
            TypeParam testVector;
            testVector.emplace_back();
            ASSERT_EQ(value_type(), testVector[0]);
            EXPECT_EQ(1, testVector.size());
        }
    }
}

TYPED_TEST(TestsVector, CtorTests)
{
    using value_type = typename TypeParam::value_type;
    {
        TypeParam testVector(3);
        ASSERT_EQ(3, testVector.size());
        for (const auto &val : testVector)
            ASSERT_EQ(value_type(), val);
        auto testFun = [](const auto &val){EXPECT_EQ(value_type(), val);};\
        // iterator tests
        std::for_each(testVector.begin(), testVector.end(), testFun);
        std::for_each(testVector.rbegin(), testVector.rend(), testFun);
        std::for_each(testVector.cbegin(), testVector.cend(), testFun);
        std::for_each(testVector.crbegin(), testVector.crend(), testFun);
    }

    if constexpr (std::is_same_v<value_type, TestVar>)
    {
        TypeParam testVector(10, value_type(true));
        ASSERT_FALSE(std::equal(testVector.begin(), testVector.end(), TypeParam(10).begin()));

        TypeParam testVectorRevert(testVector.rbegin(), testVector.rend());
        EXPECT_TRUE(std::equal(testVectorRevert.begin(), testVectorRevert.end(), testVector.rbegin()));

        {
            TypeParam testVectorCopy = testVector;
            EXPECT_TRUE(std::equal(testVector.begin(), testVector.end(), testVectorCopy.begin()));
        }

        {
            TypeParam testVectorCopy(testVector);
            EXPECT_TRUE(std::equal(testVector.begin(), testVector.end(), testVectorCopy.begin()));
        }

        {
            TypeParam testVectorCopy(testVector);
            TypeParam testVectorCopyMove(std::move(testVector));
            EXPECT_TRUE(std::equal(testVectorCopyMove.begin(), testVectorCopyMove.end(), testVectorCopy.begin()));
            EXPECT_TRUE(testVector.empty());
        }
    }

    {
        TypeParam testVector = {value_type(), value_type(), value_type()};
        EXPECT_EQ(3, testVector.size());
    }
}