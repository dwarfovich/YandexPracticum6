#include <chrono>
#include <climits>
#include <iostream>
#include <print>
#include <thread>

#include "logger.hpp"
#include "queue/bounded_queue.hpp"
#include "queue/unbounded_queue.hpp"
#include "task_dispatcher.hpp"
using namespace dispatcher;

int main() {
    //dispatcher::queue::BoundedQueue q{5};
    //dispatcher::queue::UnboundedQueue uq;
    std::unordered_map<TaskPriority, queue::QueueOptions> parameters{
        {TaskPriority::High, queue::QueueOptions{true, 100}}, {TaskPriority::Normal, queue::QueueOptions{false}}
    };
    //dispatcher::queue::PriorityQueue pq {parameters};

    dispatcher::thread_pool::ThreadPool tp{std::make_shared<dispatcher::queue::PriorityQueue>(parameters), 5};

    Logger::Get().Log("ThreadPool was created.\n");

    //TaskDispatcher td(std::thread::hardware_concurrency());
    //std::vector<std::jthread> threads;

    //for (int i = 0; i < 5; ++i) {
    //    threads.emplace_back([&, i]() {
    //        for (int j = 0; j < 10; j++) {
    //            td.schedule(TaskPriority::Normal,
    //                        [=]() { Logger::Get().Log("Normal priority message №" + std::to_string(10 * i + j)); });
    //            td.schedule(TaskPriority::High,
    //                        [=]() { Logger::Get().Log("High priority message №" + std::to_string(10 * i + j)); });
    //        }
    //    });
    //}
    
    Logger::Get().Log("Return\n");

    return 0;
}
