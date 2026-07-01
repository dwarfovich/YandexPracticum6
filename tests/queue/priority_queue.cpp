#include <gtest/gtest.h>

#include "queue/priority_queue.hpp"
#include "types.hpp"

using namespace dispatcher;
using namespace dispatcher::queue;

TEST(PriorityQueueTest, EmptyQueue) {
    std::unordered_map<TaskPriority, QueueOptions> parameters{
        {TaskPriority::High, QueueOptions{true, 100}}, {TaskPriority::Normal, QueueOptions{false}}};
    PriorityQueue q {parameters};
    q.shutdown();
    ASSERT_FALSE(q.pop().has_value());
}

TEST(PriorityQueueTest, TestSingleElement) {
    std::unordered_map<TaskPriority, QueueOptions> parameters{{TaskPriority::High, QueueOptions{true, 1}},
                                                              {TaskPriority::Normal, QueueOptions{false}}};
    PriorityQueue q{parameters};
    q.push(TaskPriority::Normal, {});
    ASSERT_TRUE(q.pop().has_value());
    q.push(TaskPriority::High, {});
    ASSERT_TRUE(q.pop().has_value());
}

TEST(PriorityQueueTest, TestShutdown) {
    std::unordered_map<TaskPriority, QueueOptions> parameters{{TaskPriority::High, QueueOptions{true, 1}},
                                                              {TaskPriority::Normal, QueueOptions{false}}};
    PriorityQueue q{parameters};
    q.push(TaskPriority::Normal, {});
    q.push(TaskPriority::High, {});
    q.shutdown();
    ASSERT_FALSE(q.pop().has_value());
    ASSERT_FALSE(q.pop().has_value());
    ASSERT_FALSE(q.pop().has_value());
}

TEST(PriorityQueueTest, TestPriority) {
    std::unordered_map<TaskPriority, QueueOptions> parameters{{TaskPriority::High, QueueOptions{true, 1}},
                                                              {TaskPriority::Normal, QueueOptions{false}}};
    PriorityQueue q{parameters};
    auto f1 = []{};
    auto f2 = []{};
    q.push(TaskPriority::Normal, std::ref(f1));
    q.push(TaskPriority::High, std::ref(f2));

    auto r = q.pop();
    auto p = r.value().target<std::reference_wrapper<decltype(f2)>>();
    ASSERT_NE(p, nullptr);
    ASSERT_EQ(&p->get(), &f2);

    r = q.pop();
    auto p2 = r.value().target<std::reference_wrapper<decltype(f1)>>();
    ASSERT_NE(p2, nullptr);
    ASSERT_EQ(&p2->get(), &f1);
}

// TEST(UnboundedQueueTest, TestSingleElement) {
//     UnboundedQueue q;
//     q.push({});
//     ASSERT_FALSE(q.empty());
//     auto r = q.try_pop();
//     ASSERT_TRUE(r.has_value());
// }
//
// TEST(UnboundedQueueTest, InsertingElements) {
//     UnboundedQueue q;
//     q.push({});
//     ASSERT_FALSE(q.empty());
//     auto r = q.try_pop();
//     ASSERT_TRUE(r.has_value());
//
//     q.push({});
//     q.push({});
//     ASSERT_FALSE(q.empty());
//     r = q.try_pop();
//     ASSERT_TRUE(r.has_value());
//     r = q.try_pop();
//     ASSERT_TRUE(r.has_value());
// }
//
// TEST(UnboundedQueueTest, ReturnsEmpty) {
//     UnboundedQueue q;
//     auto r = q.try_pop();
//     ASSERT_FALSE(r.has_value());
//     q.push({});
//     q.try_pop();
//     r = q.try_pop();
//     ASSERT_FALSE(r.has_value());
// }
