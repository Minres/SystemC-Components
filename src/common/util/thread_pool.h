/*******************************************************************************
 * Copyright 2021-2022 MINRES Technologies GmbH
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

#ifndef _COMMON_UTIL_THREAD_POOL_H_
#define _COMMON_UTIL_THREAD_POOL_H_

#include <deque>
#include <future>
#include <thread>
#include <type_traits>
#include <vector>

/**
 * \ingroup scc-common
 */
/**@{*/
//! @brief SCC common utilities
namespace util {
//! @brief a simple thread pool
struct thread_pool {
    // the mutex, condition variable and deque form a single thread-safe triggered queue of tasks:
    std::mutex m;
    std::condition_variable v;
    // note that a packaged_task<void> can store a packaged_task<R>:
    std::deque<std::packaged_task<void()>> work;
    // this holds futures representing the worker threads being done:
    std::vector<std::future<void>> finished;
    // queue( lambda ) will enqueue the lambda into the tasks for the threads
    // to use.  A future of the type the lambda returns is given to let you get
    // the result out.
    //  template<class F, class R=typename std::result_of<F>::type>
    //  std::future<R> queue(F&& f) {
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;
        // wrap the function object into a packaged task, splitting
        // execution from the return value:
        std::packaged_task<return_type()> p(std::forward<F>(f));
        auto r = p.get_future(); // get the return value before we hand off the task
        {
            std::unique_lock<std::mutex> l(m);
            work.emplace_back(std::move(p)); // store the task<R()> as a task<void()>
        }
        v.notify_one(); // wake a thread to work on the task
        return r;       // return the future result of the task
    }
    // start N threads in the thread pool.
    void start(std::size_t N = 1) {
        for(std::size_t i = 0; i < N; ++i) {
            // each thread is a std::async running this->thread_task():
            finished.push_back(std::async(std::launch::async, [this] { thread_task(); }));
        }
    }
    // abort() cancels all non-started tasks, and tells every working thread
    // stop running, and waits for them to finish up.
    void abort() {
        cancel_pending();
        finish();
    }
    // cancel_pending() merely cancels all non-started tasks:
    void cancel_pending() {
        std::unique_lock<std::mutex> l(m);
        work.clear();
    }
    // finish enques a "stop the thread" message for every thread, then waits for them:
    void finish() {
        {
            std::unique_lock<std::mutex> l(m);
            for(auto&& unused : finished) {
                work.push_back({});
            }
        }
        v.notify_all();
        finished.clear();
    }
    ~thread_pool() { finish(); }

private:
    void thread_task() {
        while(true) {
            std::packaged_task<void()> f;
            {
                std::unique_lock<std::mutex> l(m);
                if(work.empty()) {
                    v.wait(l, [&] { return !work.empty(); });
                }
                f = std::move(work.front());
                work.pop_front();
            }
            if(!f.valid())
                return;
            f();
        }
    }
};
} // namespace util
/**@}*/
#endif /* _COMMON_UTIL_THREAD_POOL_H_ */
