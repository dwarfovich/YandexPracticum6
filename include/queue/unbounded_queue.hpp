#pragma once

#include "queue/queue.hpp"

#include <mutex>
#include <queue>

namespace dispatcher::queue {

class UnboundedQueue : public IQueue {
public:
    ~UnboundedQueue() override = default;

    void push(std::function<void()> task) override {
            queue_.push(std::move(task));
    }

    std::optional<std::function<void()>> try_pop() noexcept override {
        if (queue_.empty()) {
            return {};
        } else {
            auto task = std::make_optional(std::move(queue_.front()));
            queue_.pop();

            return task;
        }
    }

    bool empty() const noexcept override { return queue_.empty(); }

private:
    std::queue<std::function<void()>> queue_;
};

}  // namespace dispatcher::queue