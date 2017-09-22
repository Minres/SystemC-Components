/*******************************************************************************
 * Copyright (C) 2017, MINRES Technologies GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Contributors:
 *       eyck@minres.com - initial API and implementation
 ******************************************************************************/
#ifndef _THREAD_SYNCRONIZER_H_
#define _THREAD_SYNCRONIZER_H_

#include <atomic>
#include <functional>
#include <future>
#include <queue>
#include <stdexcept>

class thread_syncronizer {
private:
    std::queue<std::function<void()>> tasks_;
    std::atomic<bool> ready;
    std::mutex mutex_;
    std::condition_variable condition_;

public:
    /// @brief Constructor.
    thread_syncronizer() = default;

    /// @brief Destructor.
    ~thread_syncronizer() {
        // Set running flag to false then notify all threads.
        condition_.notify_all();
    }

    bool is_ready() { return ready.load(std::memory_order_acquire); }

    template <class F, class... Args>
    typename std::result_of<F(Args...)>::type enqueue_and_wait(F &&f, Args &&... args) {
        auto res = enqueue(f, args...);
        res.wait();
        return res.get();
    }

    template <class F, class... Args>
    auto enqueue(F &&f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
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
    /// @brief execute the next task in queue but do not wait for the next one
    void execute() {
        if (tasks_.empty()) return;
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
            } catch (...) {
            } // Suppress all exceptions.
        }
    }

    /// @brief execute the next task in queue or wait for the next one
    void executeNext() {
        ready.store(true, std::memory_order_release);
        // Wait on condition variable while the task is empty
        std::unique_lock<std::mutex> lock(mutex_);
        while (tasks_.empty() && ready.load(std::memory_order_acquire)) {
            condition_.wait_for(lock, std::chrono::milliseconds(10));
        }
        lock.unlock();
        execute();
        ready.store(false, std::memory_order_release);
    }
};
#endif /* _THREAD_SYNCRONIZER_H_ */
