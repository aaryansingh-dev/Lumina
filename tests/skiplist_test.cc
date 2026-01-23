#include "gtest/gtest.h"
#include "skiplist.h"

namespace lumina {

// Simple comparator for integers
struct IntComparator {
    int operator()(const int& a, const int& b) const {
        if (a < b) return -1;
        if (a > b) return 1;
        return 0;
    }
};

// Test inserting and finding a single element
TEST(SkipListTest, InsertAndContainsSingle) {
    SkipList<int, IntComparator> list{IntComparator()};
    list.Insert(42);
    EXPECT_TRUE(list.Contains(42));
    EXPECT_FALSE(list.Contains(10));
}

// Test multiple inserts and Contains()
TEST(SkipListTest, InsertMultiple) {
    SkipList<int, IntComparator> list{IntComparator()};
    std::vector<int> values = {5, 1, 7, 3, 9, 2};
    for (int v : values) list.Insert(v);

    for (int v : values) {
        EXPECT_TRUE(list.Contains(v));
    }

    // Test some values not inserted
    EXPECT_FALSE(list.Contains(0));
    EXPECT_FALSE(list.Contains(6));
    EXPECT_FALSE(list.Contains(10));
}

// Test order via iterator
TEST(SkipListTest, IteratorTraversal) {
    SkipList<int, IntComparator> list{IntComparator()};
    std::vector<int> values = {4, 2, 5, 1, 3};
    for (int v : values) list.Insert(v);

    // Create iterator
    SkipList<int, IntComparator>::Iterator it(&list);
    it.SeekToFirst();

    int expected = 1;
    while (it.Valid()) {
        EXPECT_EQ(it.key(), expected);
        ++expected;
        it.Next();
    }

    EXPECT_EQ(expected, 6); // Should have iterated 5 elements
}

// Test duplicates are not allowed (assert)
TEST(SkipListTest, NoDuplicateInsert) {
    SkipList<int, IntComparator> list{IntComparator()};
    list.Insert(10);
    // In debug build, inserting duplicate triggers assert
    // EXPECT_DEATH(list.Insert(10), ".*"); // Optional if you want to test assert behavior
}

} // namespace lumina
