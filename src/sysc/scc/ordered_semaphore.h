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
#include <array>
#include <deque>
#include <sysc/kernel/sc_process_handle.h>

#ifndef SC_API
#define SC_API
#endif

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
/**
 * @class ordered_semaphore
 * @brief The ordered_semaphore primitive channel class.
 *
 * The ordered semaphore acts like an ordinary semaphore. It gives the guarantee that access is granted in the order of
 * arrival (FCFS)
 */
class SC_API ordered_semaphore : public sc_core::sc_semaphore_if, public sc_core::sc_object {
public:
    /**
     * @fn  ordered_semaphore(unsigned=1)
     * @brief constructor of an un-named semaphore
     *
     * If the initial value is 0 the semaphore has an unlimited capacity but is initially empty
     *
     * @param init_value initial capacity of the semaphore
     */
    explicit ordered_semaphore(unsigned init_value = 1);
    /**
     * @fn  ordered_semaphore(const char*, unsigned=1)
     * @brief constructor of an un-named semaphore
     *
     * @param name the SystemC name
     * @param init_value initial capacity of the semaphore
     */
    ordered_semaphore(const char* name, unsigned init_value = 1);

    ordered_semaphore(const ordered_semaphore&) = delete;

    ordered_semaphore& operator=(const ordered_semaphore&) = delete;
    /**
     * @fn int wait()
     * @brief lock (take) the semaphore, block if not available
     *
     * @return value after locking
     */
    int wait() override;
    /**
     * @fn int wait()
     * @brief lock (take) the semaphore, block if not available
     *
     * @return value after locking
     */
    int wait(unsigned priority);
    /**
     * @fn int trywait()
     * @brief lock (take) the semaphore, return -1 if not available
     *
     * @return value after locking or -1 if the semaphore could not be locked
     */
    int trywait() override;
    /**
     * @fn int post()
     * @brief unlock (give) the semaphore
     *
     * @return value after posting
     */
    int post() override;
    /**
     * @fn unsigned get_capacity()
     * @brief retrieve the initial capacity of the semaphore
     *
     */
    unsigned get_capacity() { return capacity; }
    /**
     * @fn void set_capacity(unsigned)
     * @brief change the capacity
     *
     * @param capacity the new capacity
     */
    void set_capacity(unsigned capacity);
    /**
     * @fn int get_value()const
     * @brief get the value of the semaphore
     *
     * @return current value
     */
    int get_value() const override { return value; }
    /**
     * @fn const char* kind()const
     * @brief kind of this SastemC object
     *
     * @return
     */
    const char* kind() const override { return "sc_semaphore_ordered"; }
    /**
     * @struct lock
     * @brief a lock for the semaphore
     *
     * it allows to use lock/unlock a semaphore in RTTI style
     *
     */
    struct lock {
        /**
         * @fn  lock(scc::ordered_semaphore&)
         * @brief lock the given semahore, wait if not free
         *
         * @param sem the semaphore
         */
        lock(scc::ordered_semaphore& sem)
        : sem(sem) {
            sem.wait();
        }
        lock(scc::ordered_semaphore& sem, unsigned prio)
        : sem(sem) {
            sem.wait(prio);
        }
        /**
         * @fn  ~lock()
         * @brief destructor, unlock the semaphore if still locked
         *
         */
        ~lock() { release(); }
        /**
         * @fn  release()
         * @brief unlock the semaphore
         *
         */
        void release() {
            if(owned)
                sem.post();
            owned = false;
        }

    private:
        scc::ordered_semaphore& sem;
        bool owned{true};
    };

protected:
    // support methods
    bool in_use() {
        if(value > 0) {
            if(queue[1].size()) {
                if(queue[1].front() == sc_core::sc_get_current_process_handle()) {
                    queue[1].pop_front();
                    return false;
                }
            } else {
                if(queue[0].front() == sc_core::sc_get_current_process_handle()) {
                    queue[0].pop_front();
                    return false;
                }
            }
        }
        return true;
    }

    // error reporting
    void report_error(const char* id, const char* add_msg = 0) const;

protected:
    sc_core::sc_event free_evt; // event to block on when m_value is negative
    int value;                  // current value of the semaphore
    unsigned capacity;
    std::array<std::deque<sc_core::sc_process_handle>, 2> queue;
};

template <unsigned CAPACITY> struct ordered_semaphore_t : public ordered_semaphore {
    explicit ordered_semaphore_t()
    : ordered_semaphore(CAPACITY) {}
    ordered_semaphore_t(const char* name_)
    : ordered_semaphore(name_, CAPACITY) {}
};

} // namespace scc
/** @} */ // end of scc-sysc
