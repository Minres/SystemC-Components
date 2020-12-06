/*******************************************************************************
 * Copyright 2018 MINRES Technologies GmbH
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

#ifndef _SCC_PEQ_H_
#define _SCC_PEQ_H_

#include <boost/optional.hpp>
#include <map>
#include <systemc>
#include <type_traits>

namespace scc {
/**
 * @struct peq
 * @brief priority event queue
 *
 * A simple priority event queue with a copy of the original value
 *
 * @tparam TYPE the type name of the object to keep in th equeue
 */
template <class TYPE> struct peq : public sc_core::sc_object {

    static_assert(std::is_copy_constructible<TYPE>::value, "TYPE needs to be copy-constructible");

    using pair_type = std::pair<const sc_core::sc_time, TYPE>;
    /**
      * @fn  peq()
     * @brief default constructor creating a unnamed peq
     *
     */
    peq()
    : sc_core::sc_object(sc_core::sc_gen_unique_name("peq")) {}
/**
 * @fn  peq(const char*)
 * @brief named peq constructor
 *
 * @param name
 */
    explicit peq(const char* name)
    : sc_core::sc_object(name) {}
    /**
     * @fn void notify(const TYPE&, const sc_core::sc_time&)
     * @brief non-blocking push.
     *
     * Inserts entry into the queue with time based notification
     *
     * @param entry the value to insert
     * @param t the delay for calling get
     */
    void notify(const TYPE& entry, const sc_core::sc_time& t) {
        m_scheduled_events.insert(pair_type(t + sc_core::sc_time_stamp(), entry));
        m_event.notify(t);
    }
    /**
     * @fn void notify(const TYPE&)
     * @brief non-blocking push
     *
     * Inserts entry into the queue with immediate notification
     *
     * @param entry the value to insert
     */
    void notify(const TYPE& entry) {
        m_scheduled_events.insert(pair_type(sc_core::sc_time_stamp(), entry));
        m_event.notify(); // immediate notification
    }
    /**
     * @fn boost::optional<TYPE> get_next()
     * @brief non-blocking get
     *
     * @return optional copy of the head element
     */
    boost::optional<TYPE> get_next() {
        if(m_scheduled_events.empty())
            return boost::none;
        sc_core::sc_time now = sc_core::sc_time_stamp();
        if(m_scheduled_events.begin()->first > now) {
            m_event.notify(m_scheduled_events.begin()->first - now);
            return boost::none;
        } else {
            auto entry = m_scheduled_events.begin()->second;
            m_scheduled_events.erase(m_scheduled_events.begin());
            return entry;
        }
    }
    /**
     * @fn TYPE get()
     * @brief blocking get
     *
     * @return copy of the next entry.
     */
    TYPE get() {
        while(!has_next()) {
            sc_core::wait(event());
        }
        auto entry = m_scheduled_events.begin()->second;
        m_scheduled_events.erase(m_scheduled_events.begin());
        return entry;
    }
    /**
     * @fn sc_core::sc_event& event()
     * @brief get the available event
     *
     * @return reference to the event
     */
    sc_core::sc_event& event() { return m_event; }
    /**
     * @fn void cancel_all()
     * @brief cancel all events from the event queue
     *
     */
    void cancel_all() {
        m_scheduled_events.clear();
        m_event.cancel();
    }
    /**
     * @fn bool has_next()
     * @brief check if value is available at current time
     *
     * @return true if data is available for \ref get()
     */
    bool has_next() {
        if(m_scheduled_events.empty())
            return false;
        sc_core::sc_time now = sc_core::sc_time_stamp();
        if(m_scheduled_events.begin()->first > now) {
            m_event.notify(m_scheduled_events.begin()->first - now);
            return false;
        } else {
            return true;
        }
    }

private:
    std::multimap<const sc_core::sc_time, TYPE> m_scheduled_events;
    sc_core::sc_event m_event;
};
} // namespace scc

#endif /* _SCC_PEQ_H_ */
