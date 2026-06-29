#include "task_dispatcher.hpp"
#include <future>
#include <gtest/gtest.h>
#include <latch>

using namespace dispatcher;
using namespace dispatcher::queue;

TEST(TaskDispatcher, ExecutesTask) {
    std::unordered_map<TaskPriority, QueueOptions> parameters{{TaskPriority::High, QueueOptions{true, 100}},
                                                              {TaskPriority::Normal, QueueOptions{false}}};
    TaskDispatcher dispatcher{2, parameters};
    std::promise<void> promise;
    auto future = promise.get_future();

    dispatcher.schedule(TaskPriority::Normal, [&] { promise.set_value(); });

    EXPECT_EQ(future.wait_for(std::chrono::milliseconds(100)), std::future_status::ready);

    std::promise<void> promise2;
    auto future2 = promise2.get_future();

    dispatcher.schedule(TaskPriority::High, [&] { promise2.set_value(); });

    EXPECT_EQ(future2.wait_for(std::chrono::milliseconds(100)), std::future_status::ready);
}

TEST(TaskDispatcher, ExecutesMultipleTasks) {
    std::unordered_map<TaskPriority, QueueOptions> parameters{{TaskPriority::High, QueueOptions{true, 100}},
                                                              {TaskPriority::Normal, QueueOptions{false}}};
    TaskDispatcher dispatcher{2, parameters};

    std::atomic<int> normal_counter = 0;
    std::atomic<int> high_counter = 0;
    constexpr int task_count = 100;
    std::latch done(task_count);

    for (int i = 0; i < task_count; ++i) {
        if (i & 1) {
            dispatcher.schedule(TaskPriority::Normal, [&] {
                ++normal_counter;
                done.count_down();
            });
        } else {
            dispatcher.schedule(TaskPriority::High, [&] {
                ++high_counter;
                done.count_down();
            });
        }
    }

    done.wait();
    EXPECT_EQ(normal_counter, 50);
    EXPECT_EQ(high_counter, 50);
}
// здесь ваш код