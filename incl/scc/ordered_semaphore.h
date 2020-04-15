/*******************************************************************************
 * Copyright 2019 MINRES Technologies GmbH
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

#include "sysc/communication/sc_semaphore_if.h"
#include "sysc/kernel/sc_event.h"
#include "sysc/kernel/sc_object.h"
#include <deque>
#include <sysc/kernel/sc_process_handle.h>

namespace scc {
// ----------------------------------------------------------------------------
//  CLASS : sc_semaphore
//
//  The sc_semaphore primitive channel class.
// ----------------------------------------------------------------------------

class ordered_semaphore : public sc_core::sc_semaphore_if, public sc_core::sc_object {
public:
    // constructors

    explicit ordered_semaphore(int init_value_);
    ordered_semaphore(const char* name_, int init_value_);

    // interface methods

    // lock (take) the semaphore, block if not available
    virtual int wait();

    // lock (take) the semaphore, return -1 if not available
    virtual int trywait();

    // unlock (give) the semaphore
    virtual int post();

    // get the value of the semaphore
    virtual int get_value() const { return m_value; }

    virtual const char* kind() const { return "sc_semaphore_ordered"; }

    struct lock {
        lock(scc::ordered_semaphore& sem) :sem(sem){ sem.wait();}
        ~lock(){sem.post();}
    private:
        scc::ordered_semaphore& sem;
    };

protected:
    // support methods

    bool in_use() {
        bool avail = m_value > 0 && queue.front() == sc_core::sc_get_current_process_handle();
        if(avail)
            queue.pop_front();
        return (!avail);
    }

    // error reporting
    void report_error(const char* id, const char* add_msg = 0) const;

protected:
    sc_core::sc_event m_free; // event to block on when m_value is negative
    int m_value;              // current value of the semaphore
    std::deque<sc_core::sc_process_handle> queue;

private:
    // disabled
    ordered_semaphore(const ordered_semaphore&);
    ordered_semaphore& operator=(const ordered_semaphore&);
};

template<unsigned CAPACITY>
struct ordered_semaphore_t : public ordered_semaphore {
    explicit ordered_semaphore_t():ordered_semaphore(CAPACITY){}
    ordered_semaphore_t(const char* name_):ordered_semaphore(name_, CAPACITY){}

};

} // namespace scc
