/*
 * ordered_semaphore.h
 *
 *  Created on: May 4, 2019
 *      Author: eyck
 */

#pragma once

#include "sysc/kernel/sc_event.h"
#include "sysc/kernel/sc_object.h"
#include "sysc/communication/sc_semaphore_if.h"
#include <sysc/kernel/sc_process_handle.h>
#include <deque>

namespace scc {
// ----------------------------------------------------------------------------
//  CLASS : sc_semaphore
//
//  The sc_semaphore primitive channel class.
// ----------------------------------------------------------------------------

class ordered_semaphore
: public sc_core::sc_semaphore_if,
  public sc_core::sc_object
{
public:

    // constructors

    explicit ordered_semaphore( int init_value_ );
    ordered_semaphore( const char* name_, int init_value_ );


    // interface methods

    // lock (take) the semaphore, block if not available
    virtual int wait();

    // lock (take) the semaphore, return -1 if not available
    virtual int trywait();

    // unlock (give) the semaphore
    virtual int post();

    // get the value of the semaphore
    virtual int get_value() const
	{ return m_value; }

    virtual const char* kind() const
        { return "sc_semaphore_ordered"; }

protected:

    // support methods

    bool in_use()
	{
    	bool avail =  m_value > 0 && queue.front()==sc_core::sc_get_current_process_handle();
    	if(avail)
    		queue.pop_front();
    	return ( !avail );
	}


    // error reporting
    void report_error( const char* id, const char* add_msg = 0 ) const;

protected:

    sc_core::sc_event m_free;        // event to block on when m_value is negative
    int      m_value;       // current value of the semaphore
    std::deque<sc_core::sc_process_handle> queue;
private:

    // disabled
    ordered_semaphore( const ordered_semaphore& );
    ordered_semaphore& operator = ( const ordered_semaphore& );
};

}


