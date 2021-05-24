/*
 * watchdog.cpp
 *
 *  Created on: 06.03.2020
 *      Author: eyckj
 */

#include <util/watchdog.h>

#include <utility>

using namespace util;
using namespace std::chrono;

watchdog::watchdog(system_clock::duration timeout, std::function<void(void)> alarm_cb,
                   system_clock::duration sleep_duration)
: timeout(timeout)
, sleep_duration(sleep_duration)
, alarm_cb(std::move(alarm_cb)) {
    idle.store(true);
    live.store(true);
    guard_thread = std::thread([this] { guard(); });
}

watchdog::watchdog(system_clock::duration timeout, std::function<void(void)> alarm)
: watchdog(timeout, alarm, timeout / 10){};

watchdog::~watchdog() {
    live.store(false);
    wakeup.notify_all();
    guard_thread.join();
}

void watchdog::guard() {
    while(live.load()) {
        if(idle.load()) {
            // Sleep indefinitely until either told to become active or destruct
            std::unique_lock<std::mutex> live_lock(guard_mutex);
            wakeup.wait(live_lock, [this]() { return !this->idle.load() || !this->live.load(); });
        };
        if(!live.load())
            break;
        // the actual timeout checking
        auto now = system_clock::now();
        if((now - touched.load()) > timeout) {
            idle.store(true);
            alarm_cb();
            continue; // skip waiting for next timeout
        }
        {
            // sleep until next timeout check or destruction
            std::unique_lock<std::mutex> live_lock(guard_mutex);
            wakeup.wait_for(live_lock, sleep_duration, [this]() { return !this->live.load(); });
        }
    };
}

void watchdog::arm() {
    re_arm();
    idle.store(false);
    wakeup.notify_all();
}

void watchdog::re_arm() { touched.store(system_clock::now()); }
