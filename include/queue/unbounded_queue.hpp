#pragma once

#include "queue/queue.hpp"

#include <mutex>
#include <queue>

namespace dispatcher::queue {

class UnboundedQueue : public IQueue {
public:
    explicit UnboundedQueue(){}
    ~UnboundedQueue() override {
        is_active_ = false;
    }

    void push(std::function<void()> task) override {
        std::lock_guard lock {mutex_};
        queue_.push(std::move(task));
        has_tasks_.notify_one();
    }

    std::optional<std::function<void()>> try_pop() noexcept override {
        std::unique_lock lock{mutex_};
        has_tasks_.wait(lock, [this]{ return !queue_.empty() || !is_active_; });
        if(!is_active_){
            return {};
        }
        auto task = std::make_optional(std::move(queue_.front()));
        queue_.pop();

        return task;
    }

private:
    std::queue<std::function<void()>> queue_;
    mutable std::mutex mutex_;
    std::condition_variable has_tasks_;
    bool is_active_ = true;
};

}  // namespace dispatcher::queue