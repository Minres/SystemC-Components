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

#pragma once
#include <atomic>
#ifdef noexcept
#undef noexcept
#endif
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

/**
 * \ingroup scc-common
 */
/**@{*/
//! @brief SCC common utilities
namespace util {
/**
 * @brief a watch dog based on https://github.com/didenko/TimeoutGuard
 */
class watchdog {
public:
    /**
     * constructor
     * @param timeout until the watch dog is going to expire
     * @param alarm_cb the function to be called once the watch dog expires, upon expiration the watch dog goes into
     * idle state
     * @param sleep_duration the granularity to check expiration
     */
    watchdog(std::chrono::system_clock::duration timeout, std::function<void(void)> alarm_cb,
             std::chrono::system_clock::duration sleep_duration);
    /**
     *
     * @param timeout timeout until the watch dog is going to expire
     * @param alarm the function to be called once the watch dog expires, upon expiration the watch dog goes into idle
     * state
     */
    watchdog(std::chrono::system_clock::duration timeout, std::function<void(void)> alarm);
    /**
     * destructor
     */
    ~watchdog();

    watchdog(const watchdog&) = delete;

    watchdog& operator=(const watchdog&) = delete;

    watchdog(watchdog&&) = delete;

    watchdog& operator=(watchdog&&) = delete;

    /**
     * arms the watch dog
     */
    void arm();
    /**
     * re-arms the watch dog
     */
    void re_arm();

private:
    void guard();

    std::chrono::system_clock::duration timeout;
    std::chrono::system_clock::duration sleep_duration;
    std::function<void(void)> alarm_cb;
    std::atomic_bool idle{true};
    std::atomic_bool live{true};
    std::atomic<std::chrono::system_clock::time_point> touched{std::chrono::system_clock::now()};
    std::thread guard_thread;
    std::mutex guard_mutex;
    std::condition_variable wakeup;
};
} // namespace util
/** @} */
