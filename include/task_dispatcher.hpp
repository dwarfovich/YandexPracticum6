#pragma once

#include <memory>

#include "queue/priority_queue.hpp"
#include "thread_pool/thread_pool.hpp"
#include "types.hpp"

namespace dispatcher {

class TaskDispatcher {
public:
    TaskDispatcher(size_t thread_count, const std::unordered_map<TaskPriority, queue::QueueOptions> &queue_options)
        : queue_{std::make_shared<queue::PriorityQueue>(queue_options)} 
        , thread_pool_{queue_, thread_count}
    {}
    ~TaskDispatcher() {}

    void schedule(TaskPriority priority, std::function<void()> task) {
        queue_->push(priority, task);
    }

private:
    std::shared_ptr<queue::PriorityQueue> queue_;
    thread_pool::ThreadPool thread_pool_;
};

}  // namespace dispatcher