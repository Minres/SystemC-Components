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
#include <queue>
#include <stdexcept>
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
    std::queue<std::function<void()>> tasks_;
    std::atomic<bool> ready;
    std::mutex mutex_;
    std::condition_variable condition_;

public:
    /**
     * the constructor.
     */
    thread_syncronizer() = default;
    /**
     * the destructor
     */
    ~thread_syncronizer() {
        // Set running flag to false then notify all threads.
        condition_.notify_all();
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
    template <class F, class... Args>
    typename std::result_of<F(Args...)>::type enqueue_and_wait(F&& f, Args&&... args) {
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
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mutex_);
            tasks_.emplace([task]() { (*task)(); });
        }
        condition_.notify_one();
        return res;
    }
    /**
     * execute the next task in queue but do not wait for the next one
     */
    void execute() {
        if(tasks_.empty())
            return;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            // Copy task locally and remove from the queue. This is done within
            // its own scope so that the task object is destructed immediately
            // after running the task.  This is useful in the event that the
            // function contains shared_ptr arguments bound via bind.
            std::function<void()> functor = tasks_.front();
            tasks_.pop();
            lock.unlock();
            // Run the task.
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
        // Wait on condition variable while the task is empty
        std::unique_lock<std::mutex> lock(mutex_);
        while(tasks_.empty() && ready.load(std::memory_order_acquire)) {
            condition_.wait_for(lock, std::chrono::milliseconds(10));
        }
        lock.unlock();
        execute();
        ready.store(false, std::memory_order_release);
    }
};
} // namespace util
/**@}*/
#endif /* _THREAD_SYNCRONIZER_H_ */
