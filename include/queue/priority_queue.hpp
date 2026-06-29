#pragma once

#include "queue/bounded_queue.hpp"
#include "queue/unbounded_queue.hpp"
#include "types.hpp"

#include <atomic>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <print>
#include <stdexcept>
#include <unordered_map>

namespace dispatcher::queue {

class PriorityQueue {
public:
    explicit PriorityQueue(const std::unordered_map<TaskPriority, QueueOptions> &queues_options) {
        std::lock_guard lock{mutex_};
        auto iter = queues_options.find(TaskPriority::High);
        if (iter == queues_options.cend()) {
            throw std::runtime_error("No options provided for high-priority queue");
        }
        if (iter->second.bounded) {
            high_priority_queue_ = std::make_unique<BoundedQueue>(iter->second.capacity.value());
        } else {
            high_priority_queue_ = std::make_unique<UnboundedQueue>();
        }
        iter = queues_options.find(TaskPriority::Normal);
        if (iter == queues_options.cend()) {
            throw std::runtime_error("No options provided for normal-priority queue");
        }
        if (iter->second.bounded) {
            normal_priority_queue_ = std::make_unique<BoundedQueue>(iter->second.capacity.value());
        } else {
            normal_priority_queue_ = std::make_unique<UnboundedQueue>();
        }
    }

    ~PriorityQueue() { shutdown(); }

    void push(TaskPriority priority, std::function<void()> task) {
        std::unique_lock lock{mutex_};
        if (priority == TaskPriority::High) {
            high_priority_queue_->push(std::move(task));

        } else {
            normal_priority_queue_->push(std::move(task));
        }
        lock.unlock();
        has_tasks_.notify_one();
    }

    std::optional<std::function<void()>> pop() {
        std::unique_lock lock(mutex_);

        std::println("Thread {} waits.", std::this_thread::get_id());
        has_tasks_.wait(
            lock, [this] { return !is_active_ || !high_priority_queue_->empty() || !normal_priority_queue_->empty(); });

        if (!is_active_)
            return std::nullopt;

        if (auto task = high_priority_queue_->try_pop()) {
            return task;
        } else {
            return normal_priority_queue_->try_pop();
        }
    }

    void shutdown() {
        {
            std::lock_guard lock{mutex_};
            is_active_ = false;
        }
        has_tasks_.notify_all();
    }

private:
    std::unique_ptr<IQueue> high_priority_queue_;
    std::unique_ptr<IQueue> normal_priority_queue_;
    std::mutex mutex_;
    std::condition_variable has_tasks_;
    bool is_active_ = true;
};

}  // namespace dispatcher::queue