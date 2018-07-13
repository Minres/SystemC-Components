/*
 * peq.h
 *
 *  Created on: 13.07.2018
 *      Author: eyck
 */

#ifndef _SCC_PEQ_H_
#define _SCC_PEQ_H_

#include <systemc>
#include <boost/optional.hpp>
#include <map>
#include <type_traits>

namespace scc {
/**
 * a simple priority event queue with a copy of the original value
 */
template <class TYPE>
struct peq : public sc_core::sc_object {

  static_assert(std::is_copy_constructible<TYPE>::value, "TYPE needs to be copy-constructible");

  using pair_type = std::pair<const sc_core::sc_time, TYPE>;

  peq() : sc_core::sc_object(sc_core::sc_gen_unique_name("peq")){ }

  explicit
  peq(const char* name) : sc_core::sc_object(name){ }

  void notify(const TYPE& trans, const sc_core::sc_time& t){
    m_scheduled_events.insert(pair_type(t + sc_core::sc_time_stamp(), trans));
    m_event.notify(t);
  }

  void notify(const TYPE& trans) {
    m_scheduled_events.insert(pair_type(sc_core::sc_time_stamp(), trans));
    m_event.notify(); // immediate notification
  }

  boost::optional<TYPE> get_next() {
    if (m_scheduled_events.empty())
      return boost::none;;
    sc_core::sc_time now = sc_core::sc_time_stamp();
    if (m_scheduled_events.begin()->first <= now) {
      auto trans = m_scheduled_events.begin()->second;
      m_scheduled_events.erase(m_scheduled_events.begin());
      return trans;
    }
    m_event.notify(m_scheduled_events.begin()->first - now);
    return boost::none;;
  }

  sc_core::sc_event& event(){
    return m_event;
  }

  // Cancel all events from the event queue
  void cancel_all() {
    m_scheduled_events.clear();
    m_event.cancel();
  }

private:
  std::map<const sc_core::sc_time, TYPE> m_scheduled_events;
  sc_core::sc_event m_event;
};
}

#endif /* _SCC_PEQ_H_ */
