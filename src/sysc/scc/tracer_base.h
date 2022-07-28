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

#ifndef _SCC_TRACER_BASE_H_
#define _SCC_TRACER_BASE_H_

#include "utilities.h"
#include <sysc/tracing/sc_trace.h>
#include <type_traits>

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
/**
 * @enum trace_types
 * @brief identifies the various type to be traced
 *
 */
enum class trace_types : unsigned {
    NONE = 0x0,      /**< NONE */
    SIGNALS = 0x1,   /**< SIGNALS */
    PORTS = 0x2,     /**< PORTS */
    SOCKETS = 0x4,   /**< SOCKETS */
    VARIABLES = 0x8, /**< VARIABLES */
    ALL = 0xff       /**< ALL */
};
/**
 * @fn trace_types operator |(trace_types, trace_types)
 * @brief operator overload to allow boolean or operations on \ref trace_types
 *
 * @param lhs left hand side
 * @param rhs right hand side
 * @return result
 */
inline trace_types operator|(trace_types lhs, trace_types rhs) {
    return static_cast<trace_types>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));
}
/**
 * @fn trace_types operator &(trace_types, trace_types)
 * @brief operator overload to allow boolean and operations on \ref trace_types
 *
 * @param lhs left hand side
 * @param rhs right hand side
 * @return result
 */
inline trace_types operator&(trace_types lhs, trace_types rhs) {
    return static_cast<trace_types>(static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs));
}
/**
 * @class tracer_base
 *
 * it provide the basic infrastructure to automagically trace ports, signals, tlm sockets and
 * sc_variables. It comes with some limitations:
 * - arbitrary sized data types are only traced if
 *   - size is less or equal 1024,
 *   - if size is less than or equal 128 or a multiple of 8
 * - sc_variables holding std::array or std::vector are only supported for native C++ datatypes
 *
 * @brief base class for automatic tracer
 *
 */
class tracer_base : public sc_core::sc_module {
public:
    /**
     * @fn  tracer_base(const sc_core::sc_module_name&)
     * @brief named constructor
     *
     * @param nm the instance name
     */
    tracer_base(const sc_core::sc_module_name& nm)
    : sc_core::sc_module(nm) {}
    /**
     * @fn  tracer_base(const sc_core::sc_module_name&, sc_core::sc_trace_file*, bool=true)
     * @brief named constructor with trace file
     *
     * @param nm the instance name of the tracer
     * @param tf the trace file
     * @param owned if true the tracefile is owned by the tracer and closed upon simulation end
     */
    tracer_base(const sc_core::sc_module_name& nm, sc_core::sc_trace_file* tf, bool owned = true)
    : sc_core::sc_module(nm)
    , trf(tf) {}
    /**
     * @fn  ~tracer_base()
     * @brief destructor
     *
     */
    ~tracer_base() {}
    /**
     * @fn void set_trace_types(trace_types)
     * @brief set the types to trace
     *
     * @param t
     */
    void set_trace_types(trace_types t) { types_to_trace = t; }
    /**
     * @fn const sc_core::sc_trace_file* get_trace_file()const
     * @brief get the tracefile used by this tracer
     *
     * @return the tracefile
     */
    const sc_core::sc_trace_file* get_trace_file() const { return trf; }
    /**
     * @fn const sc_core::sc_trace_file* get_trace_file()const
     * @brief get the tracefile used by this tracer
     *
     * @return the tracefile
     */
    sc_core::sc_trace_file* get_trace_file() { return trf; }
    /**
     * @fn void set_trace_file(sc_core::sc_trace_file*)
     * @brief set the trace file of this tracer
     *
     * The provided file is not owned by the tracer. Hence the caller is responsible for closing the tracefile
     * @param trf
     */
    void set_trace_file(sc_core::sc_trace_file* trf) { this->trf = trf; }

protected:
    //! the default for tracing if no attribute is configured
    bool default_trace_enable{true};

    virtual void descend(const sc_core::sc_object*, bool trace_all);

    static void try_trace(sc_core::sc_trace_file* trace_file, const sc_core::sc_object* object, trace_types t);

    sc_core::sc_trace_file* trf{nullptr};

    trace_types types_to_trace{trace_types::ALL};
};

} // namespace scc
/** @} */ // end of scc-sysc
#endif /* _SCC_TRACER_BASE_H_ */
