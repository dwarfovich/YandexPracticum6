#pragma once

#include "queue/priority_queue.hpp"

#include <memory>
#include <thread>
#include <vector>
#include <print>
#include <exception>

namespace dispatcher::thread_pool {

class ThreadPool {
public:
    ThreadPool(const std::shared_ptr<dispatcher::queue::PriorityQueue>& queue, std::size_t threads_count) : queue_{queue} {
        if(!queue){
            throw std::runtime_error("No queue given");
        }

        threads_.reserve(threads_count);
        for(std::size_t i = 0; i < threads_count; ++i){
            threads_.emplace_back([this]{Worker();});
        }
    }

    ~ThreadPool(){
        queue_->shutdown();
    }

    private:
        void Worker(){
            while (auto task = queue_->pop()) {
                std::invoke(*task);
            }
        }

    private:
        std::shared_ptr<dispatcher::queue::PriorityQueue> queue_;
        std::vector<std::jthread> threads_;
};

} // namespace dispatcher::thread_pool
