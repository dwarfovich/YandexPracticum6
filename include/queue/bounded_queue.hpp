#pragma once

#include "queue/queue.hpp"

#include <mutex>
#include <queue>

namespace dispatcher::queue {

class BoundedQueue : public IQueue {
public:
    explicit BoundedQueue(std::size_t capacity) : capacity_{capacity}{
    }
    ~BoundedQueue() override = default;

    void push(std::function<void()> task) override {
        std::lock_guard lock {mutex_};
        if (queue_.size() < capacity_){
            queue_.push(std::move(task));
        } else {
            throw std::runtime_error("The queue is out of capacity");
        }
    }

    std::optional<std::function<void()>> try_pop() noexcept override {
        std::lock_guard lock{mutex_};
        if (queue_.empty()) {
            return {};
        } else {
            auto task_option = std::make_optional(std::move(queue_.front()));
            queue_.pop();

            return task_option;
        }
    }

    bool empty() const noexcept override {
        std::lock_guard lock{mutex_};
        return queue_.empty();
    }
private:
    mutable std::mutex mutex_;
    const std::size_t capacity_;
    std::queue<std::function<void()>> queue_;
};

}  // namespace dispatcher::queue