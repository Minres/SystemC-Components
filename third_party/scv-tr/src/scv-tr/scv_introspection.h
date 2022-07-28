//  -*- C++ -*- <this line is for emacs to recognize it as C++ code>
/*****************************************************************************
  Licensed to Accellera Systems Initiative Inc. (Accellera)
  under one or more contributor license agreements.  See the
  NOTICE file distributed with this work for additional
  information regarding copyright ownership. Accellera licenses
  this file to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing,
  software distributed under the License is distributed on an
  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
  KIND, either express or implied.  See the License for the
  specific language governing permissions and limitations
  under the License.

 *****************************************************************************/

/*****************************************************************************

  scv_introspection.h --
  The public interface for the introspection facility.


  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey, Samir Agrawal
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Stephan Schulz, Fraunhofer IIS-EAS, 2013-02-21
  Description of Modification: undefined DELETE macro from winnt.h to support
                               mingw32

 *****************************************************************************/

#ifndef SCV_TR_INTROSPECTION_H
#define SCV_TR_INTROSPECTION_H

#include <systemc>

#include "scv_report.h"
#include <cassert>
#include <cstring>
#include <list>
#include <memory>
#include <string>

//! @brief SystemC Verification Library (SCV) Transaction Recording
namespace scv_tr {

// specific stuff for randomization extensions
template <typename T> class scv_extensions;
class scv_constraint_base;
class scv_extensions_if;
class scv_smart_ptr_if;
class _scv_constraint_data;

template <typename T> class scv_bag {};
class scv_random {};

// ----------------------------------------
// test sc_uint<N> without SystemC
// ----------------------------------------
#define TEST_NEST_TEMPLATE
template <int N> class test_uint {
    int i;

public:
    test_uint(int i = 0)
    : i(i) {}
    test_uint(const test_uint& j)
    : i(j) {}
    test_uint& operator=(const test_uint& j) {
        i = j.i;
        return *this;
    }
    test_uint& operator=(int j) {
        i = j;
        return *this;
    }
    operator int() const { return i; }
    int to_int64() const { return i; }
    int to_uint64() const { return i; }
};

// ----------------------------------------
// core introspection infrastructure
// ----------------------------------------

// overall interface for the extensions
class scv_extensions_if;
class scv_object_if {
public:
    // return the instance name of the data structure
    virtual const char* get_name() const = 0;

    // return a string unique to each class
    virtual const char* kind() const = 0;

    // print current values on output stream
    virtual void print(std::ostream& o = std::cout, int details = 0, int indent = 0) const = 0;

    // print current values on output stream
    virtual void show(int details = 0, int indent = 0) const { print(std::cout, -1, 0); }

    // control debug messages by facility (do not override)
    static void set_debug_level(const char* facility, int level = -1);

    // are debug messages on for this class (write for each class)?
    // static int get_debug() { ... }

    // control debug messages by class (write for each class)
    // static void set_debug(int) { ... }

    // destructor (does nothing)
    virtual ~scv_object_if(){};
};

#define _SCV_INTROSPECTION_BASE scv_object_if

// ----------------------------------------
// utilities supporting all extensions
// ----------------------------------------
class scv_extension_util_if : public _SCV_INTROSPECTION_BASE {
public:
    // scv_object_if's interface is also available
    //
    virtual const char* get_name() const = 0;
    virtual const char* kind() const = 0;
    virtual void print(std::ostream& o = std::cout, int details = 0, int indent = 0) const = 0;
    virtual bool has_valid_extensions() const = 0;
    virtual bool is_dynamic() const = 0;
    virtual std::string get_short_name() const = 0;
    virtual void set_name(const char*) = 0; // error if performed on fields/array-elts
};

#undef _SCV_INTROSPECTION_BASE
#define _SCV_INTROSPECTION_BASE scv_extension_util_if

// ----------------------------------------
// static extension to extract type information
// ----------------------------------------
class scv_extension_type_if : public _SCV_INTROSPECTION_BASE {
public:
    virtual const char* get_type_name() const = 0;

    enum data_type {
        BOOLEAN,                      // bool
        ENUMERATION,                  // enum
        INTEGER,                      // char, short, int, long, long long, sc_int, sc_bigint
        UNSIGNED,                     // unsigned { char, short, int, long, long long }, sc_uint, sc_biguint
        FLOATING_POINT_NUMBER,        // float, double
        BIT_VECTOR,                   // sc_bit, sc_bv
        LOGIC_VECTOR,                 // sc_logic, sc_lv
        FIXED_POINT_INTEGER,          // sc_fixed
        UNSIGNED_FIXED_POINT_INTEGER, // sc_ufixed
        RECORD,                       // struct/class
        POINTER,                      // T*
        ARRAY,                        // T[N]
        STRING                        // string, std::string
    };
    virtual data_type get_type() const = 0;

    bool is_bool() const { return get_type() == BOOLEAN; }

    bool is_enum() const { return get_type() == ENUMERATION; }
    virtual int get_enum_size() const = 0;
    virtual void get_enum_details(std::list<const char*>&, std::list<int>&) const = 0;
    virtual const char* get_enum_string(int) const = 0;

    bool is_integer() const { return get_type() == INTEGER; }
    bool is_unsigned() const { return get_type() == UNSIGNED; }
    bool is_bit_vector() const { return get_type() == BIT_VECTOR; }
    bool is_logic_vector() const { return get_type() == LOGIC_VECTOR; }
    bool is_fixed() const { return get_type() == FIXED_POINT_INTEGER; }
    bool is_unsigned_fixed() const { return get_type() == UNSIGNED_FIXED_POINT_INTEGER; }
    bool is_floating_point_number() const { return get_type() == FLOATING_POINT_NUMBER; }

    bool is_record() const { return get_type() == RECORD; }
    virtual int get_num_fields() const = 0;
    virtual scv_extensions_if* get_field(unsigned) = 0;
    virtual const scv_extensions_if* get_field(unsigned) const = 0;

    bool is_pointer() const { return get_type() == POINTER; }
    virtual scv_extensions_if* get_pointer() = 0;
    virtual const scv_extensions_if* get_pointer() const = 0;

    bool is_array() const { return get_type() == ARRAY; }
    virtual int get_array_size() const = 0;
    virtual scv_extensions_if* get_array_elt(int) = 0;
    virtual const scv_extensions_if* get_array_elt(int) const = 0;

    bool is_string() const { return get_type() == STRING; }

    virtual int get_bitwidth() const = 0;

    // return non-null if this is a field in a record
    // or an element in an array.
    virtual scv_extensions_if* get_parent() = 0;
    virtual const scv_extensions_if* get_parent() const = 0;

    // ... more to be added.
};

#undef _SCV_INTROSPECTION_BASE
#define _SCV_INTROSPECTION_BASE scv_extension_type_if

// ----------------------------------------
// static extension to read and write to an object, its field, and its array element
// ----------------------------------------
class scv_extension_rw_if : public _SCV_INTROSPECTION_BASE {
public:
    virtual void assign(bool) = 0;
    virtual void assign(char) = 0;
    virtual void assign(unsigned char) = 0;
    virtual void assign(short) = 0;
    virtual void assign(unsigned short) = 0;
    virtual void assign(int) = 0;
    virtual void assign(unsigned) = 0;
    virtual void assign(long) = 0;
    virtual void assign(unsigned long) = 0;
    virtual void assign(long long) = 0;
    virtual void assign(unsigned long long) = 0;
    virtual void assign(float) = 0;
    virtual void assign(double) = 0;
    virtual void assign(const std::string&) = 0;
    virtual void assign(const char*) = 0;

    virtual bool get_bool() const = 0;
    virtual long long get_integer() const = 0;
    virtual unsigned long long get_unsigned() const = 0;
    virtual double get_double() const = 0;
    virtual std::string get_string() const = 0;

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
    virtual void assign(const sc_dt::sc_bv_base& v) = 0;
    virtual void get_value(sc_dt::sc_bv_base& v) const = 0;
    virtual void assign(const sc_dt::sc_lv_base& v) = 0;
    virtual void get_value(sc_dt::sc_lv_base& v) const = 0;
#endif
};

#undef _SCV_INTROSPECTION_BASE
#define _SCV_INTROSPECTION_BASE scv_extension_rw_if

class _scv_dynamic_data;

// ----------------------------------------
// dynamic extension to perform value change callback
// ----------------------------------------
#if defined(_MSC_VER) || defined(_WIN32)
#ifdef DELETE
#undef DELETE // defined in winnt.h
#endif
#endif
// ----------------------------------------
// a thin coordination layer to collect all the extensions
// -- to be updated when a new extension is made available
// ----------------------------------------

// interface for the overall extensions
class scv_extensions_if : public _SCV_INTROSPECTION_BASE {};

#undef _SCV_INTROSPECTION_BASE
#define _SCV_INTROSPECTION_BASE scv_extensions_if

// implementation details
#include "_scv_ext_comp.h"

// to be used as base class of your composite type
template <typename T> class scv_extensions_base : public _SCV_INTROSPECTION_BASE {
public:
    virtual ~scv_extensions_base() {}
};

// to be used as base class of your enum type
template <typename T> class scv_enum_base : public _SCV_INTROSPECTION_BASE_ENUM {
public:
    scv_enum_base() {}
    virtual ~scv_enum_base() {}
    virtual std::list<const char*>& _get_names() const {
        static std::list<const char*> _names;
        return _names;
    }
    virtual std::list<int>& _get_values() const {
        static std::list<int> _values;
        return _values;
    }
    virtual int get_bitwidth() const { return 8 * sizeof(T); }
    T read() const { return (T)_SCV_INTROSPECTION_BASE_ENUM::read(); }
    void write(const T rhs) { _SCV_INTROSPECTION_BASE_ENUM::write((int)rhs); }
    void _set_instance(T* p) { _SCV_INTROSPECTION_BASE_ENUM::_set_instance((int*)p); }
    void _set_as_field(_scv_extension_util_record* parent, T* p, const std::string& name) {
        _SCV_INTROSPECTION_BASE_ENUM::_set_as_field(parent, (int*)p, name);
    }
    T* _get_instance() const { return (T*)_instance; }
    T* get_instance() {
        _scv_message::message(_scv_message::INTROSPECTION_GET_INSTANCE_USAGE);
        return (T*)_instance;
    }
    const T* get_instance() const { return (T*)_instance; }
};

// to be specialized for user-specified datatype
template <typename T> class scv_extensions : public scv_extensions_base<T> {
public:
    scv_extensions() {
        // this class should never be instantiated because
        // only specializations of this template is instantiated
        _scv_message::message(_scv_message::INTROSPECTION_INVALID_EXTENSIONS);
    }
};

// supporting macros
#define SCV_EXTENSIONS(type_name) template <> class scv_extensions<type_name> : public scv_extensions_base<type_name>

#define SCV_EXTENSIONS_CTOR(type_name)                                                                                 \
    virtual const char* get_type_name() const {                                                                        \
        static const char* s = strdup(#type_name);                                                                     \
        return s;                                                                                                      \
    }                                                                                                                  \
    scv_extensions() { _set_instance(NULL); }                                                                          \
    scv_extensions(const scv_extensions& rhs) {                                                                        \
        _set_instance(NULL);                                                                                           \
        _set_instance((type_name*)rhs._get_instance());                                                                \
    }                                                                                                                  \
    virtual ~scv_extensions() {}                                                                                       \
    scv_extensions& operator=(const scv_extensions& rhs) {                                                             \
        write(*rhs._get_instance());                                                                                   \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions& operator=(const type_name& rhs) {                                                                  \
        write(rhs);                                                                                                    \
        return *this;                                                                                                  \
    }                                                                                                                  \
    operator const type_name&() const { return *(type_name*)_get_instance(); }                                         \
    virtual void _set_instance_core_wrap(void* p) { _set_instance_core((type_name*)p); }                               \
    void _set_instance_core(type_name* _scv_object_with_introspection)

#define SCV_FIELD(field_name)                                                                                          \
    std::string field_name##_name = #field_name;                                                                       \
    field_name._set_as_field(this, _scv_object_with_introspection ? (&_scv_object_with_introspection->field_name) : 0, \
                             field_name##_name)

#define SCV_EXTENSIONS_BASE_CLASS(class_name)                                                                          \
    scv_extensions<class_name>::_set_instance_core(_scv_object_with_introspection)

#define SCV_ENUM_EXTENSIONS(type_name) template <> class scv_extensions<type_name> : public scv_enum_base<type_name>

#define SCV_ENUM_CTOR(type_name)                                                                                       \
    virtual const char* get_type_name() const {                                                                        \
        static const char* s = strdup(#type_name);                                                                     \
        return s;                                                                                                      \
    }                                                                                                                  \
    scv_extensions() {                                                                                                 \
        static bool dummy = _init();                                                                                   \
        if(0)                                                                                                          \
            std::cout << dummy;                                                                                        \
    }                                                                                                                  \
    scv_extensions(const scv_extensions& rhs) { _set_instance(rhs._get_instance()); }                                  \
    virtual ~scv_extensions() {}                                                                                       \
    scv_extensions& operator=(const scv_extensions& rhs) {                                                             \
        write(*rhs._get_instance());                                                                                   \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions& operator=(type_name rhs) {                                                                         \
        write(rhs);                                                                                                    \
        return *this;                                                                                                  \
    }                                                                                                                  \
    operator type_name() const { return *(type_name*)_get_instance(); }                                                \
    bool _init() {                                                                                                     \
        __init();                                                                                                      \
        return true;                                                                                                   \
    }                                                                                                                  \
    void __init()

#define SCV_ENUM(element_name) _set_enum((int)element_name, #element_name)

// convenient functions for extensions
#if !defined(__HP_aCC)
template <typename T> std::ostream& operator<<(std::ostream& os, const scv_extensions<T>& data) {
    os << *data._get_instance();
    return os;
}
#endif

// implementation details
#include "_scv_introspection.h"

// ----------------------------------------
// various access interface to access an extension object
//
// scv_get_const_extensions is there to get around HP ambituous
// overloaded function call problem.
// ----------------------------------------
template <typename T> scv_extensions<T> scv_get_extensions(T& d) {
    scv_extensions<T> e;
    e._set_instance(&d);
    return e;
};

template <typename T> const scv_extensions<T> scv_get_const_extensions(const T& d) {
    scv_extensions<T> e;
    e._set_instance((T*)&d);
    return e;
};
} // namespace scv_tr
#endif
