#include <gtest/gtest.h>

#include "queue/bounded_queue.hpp"

using namespace dispatcher::queue;

TEST(BoundedQueueTest, EmptyQueue) {
    BoundedQueue q(0);
    ASSERT_TRUE(q.empty());
    ASSERT_FALSE(q.try_pop().has_value());

    BoundedQueue q2(1);
    ASSERT_TRUE(q2.empty());
    ASSERT_FALSE(q2.try_pop().has_value());

    BoundedQueue q3(2100);
    ASSERT_TRUE(q3.empty());
    ASSERT_FALSE(q3.try_pop().has_value());
}

TEST(BoundedQueueTest, TestSingleElement) {
    BoundedQueue q(1);
    q.push({});
    ASSERT_FALSE(q.empty());
    auto r = q.try_pop();
    ASSERT_TRUE(r.has_value());
}

TEST(BoundedQueueTest, InsertingElements) {
    BoundedQueue q(5);
    q.push({});
    ASSERT_FALSE(q.empty());
    auto r = q.try_pop();
    ASSERT_TRUE(r.has_value());

    q.push({});
    q.push({});
    ASSERT_FALSE(q.empty());
    r = q.try_pop();
    ASSERT_TRUE(r.has_value());
    r = q.try_pop();
    ASSERT_TRUE(r.has_value());
}

TEST(BoundedQueueTest, ThrowsWhenEmptyCapacityExceeded) {
    BoundedQueue q(0);
    ASSERT_THROW(q.push({}), std::exception);
}

TEST(BoundedQueueTest, ThrowsWhenCapacityExceeded) {
    BoundedQueue q(2);
    q.push({});
    q.push({});
    ASSERT_THROW(q.push({}), std::exception);
}

TEST(BoundedQueueTest, ThrowsWhenCapacityExceeded2) {
    BoundedQueue q(2);
    q.push({});
    q.push({});
    q.try_pop();
    q.try_pop();
    q.push({});
    q.push({});
    ASSERT_THROW(q.push({}), std::exception);
}
