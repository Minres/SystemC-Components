/*
 * watchdog.h
 *
 *  Created on: 06.03.2020
 *      Author: eyckj
 */

#pragma one
#ifdef __clang__
// quirks for gcc 4.8.5 to get clang-tidy going
#include <bits/c++config.h>
#if __GLIBCXX__ == 20150623
#define noexcept
#endif
#endif
#include <atomic>
#ifdef noexcept
#undef noexcept
#endif
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace util {
/**
 * a watch dog based on https://github.com/didenko/TimeoutGuard
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
