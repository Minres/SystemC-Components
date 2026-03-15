/*******************************************************************************
 * Copyright 2017 MINRES Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#ifndef _THREAD_SYNCRONIZER_H_
#define _THREAD_SYNCRONIZER_H_

#include <atomic>
#include <functional>
#include <future>
#include <rigtorp/SPSCQueue.h>
#include <stdexcept>
#include <thread>

//! @brief SCC common utilities
/**
 * \ingroup scc-common
 */
/**@{*/
namespace util {
/**
 * @brief executes a function syncronized in another thread
 */
class thread_syncronizer {
private:
    static constexpr size_t default_queue_capacity = 1024;

    using task_type = std::function<void()>;

    rigtorp::SPSCQueue<task_type> tasks_;
    std::atomic<bool> ready{false};
    std::atomic<bool> running{true};

public:
    /**
     * the constructor.
     */
    explicit thread_syncronizer(size_t queue_capacity = default_queue_capacity)
    : tasks_(queue_capacity) {}
    /**
     * the destructor
     */
    ~thread_syncronizer() {
        ready.store(false, std::memory_order_release);
        running.store(false, std::memory_order_release);
    }
    /**
     * check if the synchronizer can handle functions
     *
     * @return true if it can handle a new request
     */
    bool is_ready() { return ready.load(std::memory_order_acquire); }
    /**
     * enqueue a function to be executed in the other thread and wait for completion
     *
     * @param f the functor to execute
     * @param args the arguments to pass to the functor
     * @return the result of the function
     */
    template <class F, class... Args> typename std::result_of<F(Args...)>::type enqueue_and_wait(F&& f, Args&&... args) {
        auto res = enqueue(f, args...);
        res.wait();
        return res.get();
    }
    /**
     * enqueue a function to be executed in the other thread
     *
     * @param f the functor to execute
     * @param args the arguments to pass to the functor
     * @return the future holding the result of the execution
     */
    template <class F, class... Args> auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        while(running.load(std::memory_order_acquire)) {
            if(tasks_.try_emplace([task]() { (*task)(); })) {
                return res;
            }
            std::this_thread::yield();
        }
        throw std::runtime_error("thread_syncronizer is shutting down");
    }
    /**
     * execute the next task in queue but do not wait for the next one
     */
    void execute() {
        if(auto* pending = tasks_.front()) {
            // Move the task out before popping so the queue slot can be released
            // immediately after the callback has been taken over locally.
            task_type functor = std::move(*pending);
            tasks_.pop();
            try {
                functor();
            } catch(...) {
            } // Suppress all exceptions.
        }
    }
    /**
     *  execute the next task in queue or wait for the next one
     */
    void executeNext() {
        ready.store(true, std::memory_order_release);
        while(running.load(std::memory_order_acquire) && ready.load(std::memory_order_acquire)) {
            if(tasks_.front()) {
                execute();
                break;
            }
            std::this_thread::yield();
        }
        ready.store(false, std::memory_order_release);
    }
};
} // namespace util
/**@}*/
#endif /* _THREAD_SYNCRONIZER_H_ */
