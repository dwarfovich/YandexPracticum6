#include <gtest/gtest.h>
#include <barrier>

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

TEST(UnboundedQueueTest, ConcurrentPush) {
    constexpr int threads = 8;
    constexpr int tasks_per_thread = 1000;

    dispatcher::queue::UnboundedQueue queue;

    std::barrier sync(threads);

    std::vector<std::jthread> workers;

    for (int i = 0; i < threads; ++i) {
        workers.emplace_back([&] {
            sync.arrive_and_wait();
            for (int j = 0; j < tasks_per_thread; ++j) {
                queue.push([] {});
            }
        });
    }

    workers.clear();

    int count = 0;

    while (queue.try_pop()){
        ++count;
    }

    EXPECT_EQ(count, threads * tasks_per_thread);
    EXPECT_TRUE(queue.empty());
}

TEST(UnboundedQueueTest, ConcurrentPop) {
    constexpr int tasks = 10000;
    constexpr int threads = 8;

    dispatcher::queue::UnboundedQueue queue;

    std::atomic<int> executed = 0;

    for (int i = 0; i < tasks; ++i) {
        queue.push([&] { ++executed; });
    }

    std::barrier sync(threads);

    std::vector<std::jthread> workers;

    for (int i = 0; i < threads; ++i) {
        workers.emplace_back([&] {
            sync.arrive_and_wait();
            while (true) {
                auto task = queue.try_pop();

                if (!task){
                    break;
                }

                (*task)();
            }
        });
    }

    workers.clear();

    EXPECT_EQ(executed.load(), tasks);
    EXPECT_TRUE(queue.empty());
}

TEST(UnboundedQueueTest, ProducersConsumers) {
    constexpr int producers = 4;
    constexpr int consumers = 4;
    constexpr int tasks_per_producer = 2000;

    dispatcher::queue::UnboundedQueue queue;

    std::atomic<int> executed = 0;
    std::atomic<int> produced = 0;

    std::barrier sync(producers + consumers);

    std::vector<std::jthread> threads;

    for (int i = 0; i < producers; ++i) {
        threads.emplace_back([&] {
            sync.arrive_and_wait();
            for (int j = 0; j < tasks_per_producer; ++j) {
                queue.push([&] { ++executed; });
                ++produced;
            }
        });
    }

    for (int i = 0; i < consumers; ++i) {
        threads.emplace_back([&] {
            sync.arrive_and_wait();

            while (executed.load() < producers * tasks_per_producer) {
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
    EXPECT_EQ(executed.load(), producers * tasks_per_producer);
    EXPECT_TRUE(queue.empty());
}

TEST(UnboundedQueue, EveryTaskExecutedExactlyOnce) {
    constexpr int tasks = 10000;
    constexpr int consumers = 8;

    dispatcher::queue::UnboundedQueue queue;

    std::vector<std::atomic<int>> visits(tasks);

    for (auto &v : visits){
        v.store(0);
    }

    for (int i = 0; i < tasks; ++i) {
        queue.push([&, i] { ++visits[i]; });
    }

    std::barrier sync(consumers);

    std::vector<std::jthread> workers;

    for (int i = 0; i < consumers; ++i) {
        workers.emplace_back([&] {
            sync.arrive_and_wait();
            while (true) {
                auto task = queue.try_pop();
                if (!task){
                    break;
                }
                (*task)();
            }
        });
    }

    workers.clear();

    for (const auto &v : visits) {
        EXPECT_EQ(v.load(), 1);
    }

    EXPECT_TRUE(queue.empty());
}