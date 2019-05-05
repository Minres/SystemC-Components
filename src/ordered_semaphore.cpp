/*
 * ordered_semaphore.cpp
 *
 *  Created on: May 4, 2019
 *      Author: eyck
 */

#include <scc/ordered_semaphore.h>
#include "sysc/communication/sc_communication_ids.h"
#include "sysc/utils/sc_report.h"
#include "sysc/kernel/sc_wait.h"

namespace scc {

// ----------------------------------------------------------------------------
//  CLASS : sc_semaphore
//
//  The sc_semaphore primitive channel class.
// ----------------------------------------------------------------------------

// error reporting

void
ordered_semaphore::report_error( const char* id, const char* add_msg ) const
{
	char msg[BUFSIZ];
	if( add_msg != 0 ) {
		std::sprintf( msg, "%s: semaphore '%s'", add_msg, name() );
	} else {
		std::sprintf( msg, "semaphore '%s'", name() );
	}
	SC_REPORT_ERROR( id, msg );
}


// constructors

ordered_semaphore::ordered_semaphore( int init_value_ )
: sc_core::sc_object( sc_core::sc_gen_unique_name( "semaphore" ) ),
  m_free( (std::string(SC_KERNEL_EVENT_PREFIX)+"_free_event").c_str() ),
  m_value( init_value_ )
{
	if( m_value < 0 ) {
		report_error( sc_core::SC_ID_INVALID_SEMAPHORE_VALUE_ );
	}
}

ordered_semaphore::ordered_semaphore( const char* name_, int init_value_ )
: sc_object( name_ ),
  m_free( (std::string(SC_KERNEL_EVENT_PREFIX)+"_free_event").c_str() ),
  m_value( init_value_ )
{
	if( m_value < 0 ) {
		report_error( sc_core::SC_ID_INVALID_SEMAPHORE_VALUE_ );
	}
}


// interface methods

// lock (take) the semaphore, block if not available

int
ordered_semaphore::wait()
{
	queue.push_back(sc_core::sc_get_current_process_handle());
	while( in_use() ) {
		sc_core::wait( m_free);
	}
	-- m_value;
	return 0;
}


// lock (take) the semaphore, return -1 if not available

int
ordered_semaphore::trywait()
{
	if( in_use() ) {
		return -1;
	}
	-- m_value;
	return 0;
}


// unlock (give) the semaphore

int
ordered_semaphore::post()
{
	++m_value;
	m_free.notify();
	return 0;
}

} // namespace sc_core



