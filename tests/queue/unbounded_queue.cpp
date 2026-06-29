#include <gtest/gtest.h>

#include "queue/unbounded_queue.hpp"

using namespace dispatcher::queue;

TEST(UnboundedQueueTest, EmptyQueue) {
    UnboundedQueue q;
    ASSERT_TRUE(q.empty());
    ASSERT_FALSE(q.try_pop().has_value());
}

TEST(UnboundedQueueTest, TestSingleElement) {
    UnboundedQueue q;
    q.push({});
    ASSERT_FALSE(q.empty());
    auto r = q.try_pop();
    ASSERT_TRUE(r.has_value());
}

TEST(UnboundedQueueTest, InsertingElements) {
    UnboundedQueue q;
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

TEST(UnboundedQueueTest, ReturnsEmpty) {
    UnboundedQueue q;
    auto r = q.try_pop();
    ASSERT_FALSE(r.has_value());
    q.push({});
    q.try_pop();
    r = q.try_pop();
    ASSERT_FALSE(r.has_value());

}

