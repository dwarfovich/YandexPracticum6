#pragma once

#include "queue/queue.hpp"

#include <mutex>
#include <queue>

namespace dispatcher::queue {

class BoundedQueue : public IQueue {
public:
    explicit BoundedQueue(std::size_t capacity) : capacity_{capacity} {}
    ~BoundedQueue() override {
        is_active_ = false;
    }

    void push(std::function<void()> task) override {
        std::unique_lock lock{mutex_};
        has_free_slots_.wait(lock, [this] { return queue_.size() < capacity_ || !is_active_; });
        if(!is_active_){
            return;
        }
        queue_.push(std::move(task));
        has_tasks_.notify_one();
    }

    std::optional<std::function<void()>> try_pop() noexcept override { 
        std::unique_lock lock{mutex_};
        has_tasks_.wait(lock, [this]{ return !queue_.empty() || !is_active_; });
        if (!is_active_) {
            return {};
        }
        auto task_option = std::make_optional(std::move(queue_.front()));
        queue_.pop();
        has_free_slots_.notify_one();

        return task_option;
    }

private:
    const std::size_t capacity_;
    std::queue<std::function<void()>> queue_;
    mutable std::mutex mutex_;
    std::condition_variable has_free_slots_;
    std::condition_variable has_tasks_;
    bool is_active_ = true;
};

}  // namespace dispatcher::queue