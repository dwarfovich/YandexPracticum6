#include <gtest/gtest.h>
#include <barrier>

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

TEST(BoundedQueueTest, ConcurrentPush) {
    constexpr int threads = 8;
    constexpr int tasks_per_thread = 1000;

    dispatcher::queue::BoundedQueue queue(threads * tasks_per_thread);

    std::barrier sync(threads);

    std::vector<std::jthread> workers;

    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&] {
            sync.arrive_and_wait();
            for (int i = 0; i < tasks_per_thread; ++i) {
                queue.push([] {});
            }
        });
    }

    workers.clear();  // join

    int count = 0;

    while (auto task = queue.try_pop()){
        ++count;
    }

    EXPECT_EQ(count, threads * tasks_per_thread);
    EXPECT_TRUE(queue.empty());
}

TEST(BoundedQueue, ConcurrentPop) {
    constexpr int tasks = 5000;
    constexpr int threads = 8;

    dispatcher::queue::BoundedQueue queue(tasks);

    std::atomic<int> popped = 0;

    for (int i = 0; i < tasks; ++i) {
        queue.push([&] { ++popped; });
    }

    std::barrier sync(threads);

    std::vector<std::jthread> workers;

    for (int i = 0; i < threads; ++i) {
        workers.emplace_back([&] {
            sync.arrive_and_wait();

            while (true) {
                auto task = queue.try_pop();

                if (!task)
                    break;

                (*task)();
            }
        });
    }

    workers.clear();

    EXPECT_EQ(popped.load(), tasks);
    EXPECT_TRUE(queue.empty());
}

TEST(BoundedQueue, ProducersConsumers) {
    constexpr int producers = 4;
    constexpr int consumers = 4;
    constexpr int tasks_per_producer = 1000;

    dispatcher::queue::BoundedQueue queue(100000);

    std::atomic<int> produced = 0;
    std::atomic<int> consumed = 0;

    std::barrier sync(producers + consumers);

    std::vector<std::jthread> threads;

    for (int i = 0; i < producers; ++i) {
        threads.emplace_back([&] {
            sync.arrive_and_wait();
            for (int j = 0; j < tasks_per_producer; ++j) {
                queue.push([&] { ++consumed; });

                ++produced;
            }
        });
    }

    for (int i = 0; i < consumers; ++i) {
        threads.emplace_back([&] {
            sync.arrive_and_wait();
            while (consumed.load() < producers * tasks_per_producer) {
                auto task = queue.try_pop();
                if (task){
                    (*task)();
                } else{
                    std::this_thread::yield();
                }
            }
        });
    }

    threads.clear();

    EXPECT_EQ(produced.load(), producers * tasks_per_producer);
    EXPECT_EQ(consumed.load(), producers * tasks_per_producer);
    EXPECT_TRUE(queue.empty());
}