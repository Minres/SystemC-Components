/*******************************************************************************
 * Copyright 2020-2022 MINRES Technologies GmbH
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
