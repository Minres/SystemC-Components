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

// ----------------------------------------
// utilities supporting all extensions
// ----------------------------------------
class scv_extension_util_if : public scv_object_if {
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

// ----------------------------------------
// static extension to extract type information
// ----------------------------------------
class scv_extension_type_if : public scv_extension_util_if {
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

// ----------------------------------------
// static extension to read and write to an object, its field, and its array element
// ----------------------------------------
class scv_extension_rw_if : public scv_extension_type_if {
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
class scv_extensions_if : public scv_extension_rw_if {};

// implementation details
// ----------------------------------------
// macro for common inlined check for _is_dynamic()
// ----------------------------------------
#define _SCV_CHECK_DYNAMIC(feature, return_statement)                                                                  \
    if(!this->_is_dynamic()) {                                                                                         \
        _scv_message::message(_scv_message::INTROSPECTION_INVALID_DYNAMIC_EXTENSIONS, #feature);                       \
        return_statement;                                                                                              \
    }

// ----------------------------------------
// error message
// ----------------------------------------
#define _SCV_RW_ERROR(feature, type, obj)                                                                              \
    _scv_message::message(_scv_message::INTROSPECTION_INVALID_READ_WRITE, #feature, #type, #obj, this->get_name())

// ----------------------------------------
// common data objects for dynamic extensions
// ----------------------------------------
#include <cstdio>

inline std::string _scv_ext_util_get_string(int i) {
    char tmp[128];
    std::sprintf(tmp, "%d", i);
    return tmp;
}

inline const char* _scv_ext_util_get_name(const char* format, const char* name, int N) {
    static char tmp[1024];
    std::sprintf(tmp, format, name, N);
    return strdup(tmp);
}

inline const char* _scv_ext_util_get_name(const char* format, const char* name) {
    static char tmp[1024];
    std::sprintf(tmp, format, name);
    return strdup(tmp);
}

// ----------------------------------------
// others
// ----------------------------------------
#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
#define _SCV_SYSTEMC_BASIC_TYPE_SPECIALIZATION()                                                                       \
    _SCV_COMPONENT_1(sc_dt::sc_bit);                                                                                   \
    _SCV_COMPONENT_1(sc_dt::sc_logic);                                                                                 \
    _SCV_COMPONENT_N(sc_dt::sc_int);                                                                                   \
    _SCV_COMPONENT_N(sc_dt::sc_uint);                                                                                  \
    _SCV_COMPONENT_N(sc_dt::sc_bigint);                                                                                \
    _SCV_COMPONENT_N(sc_dt::sc_biguint);                                                                               \
    _SCV_COMPONENT_N(sc_dt::sc_bv);                                                                                    \
    _SCV_COMPONENT_N(sc_dt::sc_lv);                                                                                    \
    _SCV_COMPONENT(sc_dt::sc_signed);                                                                                  \
    _SCV_COMPONENT(sc_dt::sc_unsigned);                                                                                \
    _SCV_COMPONENT(sc_dt::sc_int_base);                                                                                \
    _SCV_COMPONENT(sc_dt::sc_uint_base);                                                                               \
    _SCV_COMPONENT(sc_dt::sc_lv_base);                                                                                 \
    _SCV_COMPONENT(sc_dt::sc_bv_base);

// SCV_COMPONENT_N(tag,sc_fixed);
// SCV_COMPONENT_N(tag,sc_ufixed);

#else
#define _SCV_SYSTEMC_BASIC_TYPE_SPECIALIZATION()
#endif

#define _SCV_BASIC_TYPE_SPECIALIZATION()                                                                               \
    _SCV_COMPONENT(bool);                                                                                              \
    _SCV_COMPONENT(char);                                                                                              \
    _SCV_COMPONENT(unsigned char);                                                                                     \
    _SCV_COMPONENT(short);                                                                                             \
    _SCV_COMPONENT(unsigned short);                                                                                    \
    _SCV_COMPONENT(int);                                                                                               \
    _SCV_COMPONENT(unsigned int);                                                                                      \
    _SCV_COMPONENT(long);                                                                                              \
    _SCV_COMPONENT(unsigned long);                                                                                     \
    _SCV_COMPONENT(long long);                                                                                         \
    _SCV_COMPONENT(unsigned long long);                                                                                \
    _SCV_COMPONENT(float);                                                                                             \
    _SCV_COMPONENT(double);                                                                                            \
    _SCV_COMPONENT(std::string);                                                                                       \
    _SCV_COMPONENT_N(test_uint);                                                                                       \
    _SCV_SYSTEMC_BASIC_TYPE_SPECIALIZATION()

// implementation details
// clang-format off
#include "scv_report.h"
// ----------------------------------------
// a first class in the hierarchy that contains class variables.
// ----------------------------------------
class _scv_extension_util : public scv_extensions_if {
public:
    _scv_extension_util()
    : _name("")
    , _short_name("")
    , _parent(NULL) {}
    virtual ~_scv_extension_util() {}

    // scv_object_if
    const char* get_name() const override;
    const char* kind() const override;
    void print(std::ostream& o = std::cout, int details = 0, int indent = 0) const override;

    // extension "util"
    bool has_valid_extensions() const override;
    ;
    bool is_dynamic() const override;
    ;
    std::string get_short_name() const override;
    ;
    void set_name(const char* s) override;
    ;
    virtual void _set_name(const std::string& s);

public: // non-virtual for fast execution
    inline void trigger_value_change_cb() {}

protected: // fast version of the introspection interface (non-virtual)
    bool _is_dynamic() const { return false; }

public: // internal methods (non-virtual for efficiency)
    bool _has_dynamic_data() const { return false; }
    void _set_parent(_scv_extension_util* p, const std::string& name);
    const scv_extensions_if* _get_parent() const { return _parent; }

public: // internal methods (virtual to distinguish basic/record/array)
    virtual void _set_dynamic();

public:
    std::string _name;
    std::string _short_name;
    scv_extensions_if* _parent;
};

// ----------------------------------------
// specialization for record
// ----------------------------------------
class _scv_extension_util_record : public _scv_extension_util {
public:
    virtual ~_scv_extension_util_record() {}

    // extension "util"
    virtual bool has_valid_extensions() const { return false; }
    virtual void set_name(const char* s) {
        _name = s;
        std::list<_scv_extension_util*>::iterator f;
        for(f = _fields.begin(); f != _fields.end(); ++f) {
            (*f)->_set_name(_name + "." + (*f)->get_short_name().c_str());
        }
    }
    virtual void _set_name(const std::string& s) {
        _name = s;
        std::list<_scv_extension_util*>::iterator f;
        for(f = _fields.begin(); f != _fields.end(); ++f) {
            (*f)->_set_name(_name + "." + (*f)->get_short_name().c_str());
        }
    }

protected: // fast version of the introspection interface (non-virtual)
    int _get_num_fields() const { return static_cast<int>(_fields.size()); }
    _scv_extension_util* _get_field(unsigned i) {
        if(i >= _fields.size()) {
            _scv_message::message(_scv_message::INTROSPECTION_INVALID_INDEX, i, "composite object", get_name());
            return NULL;
        }
        std::list<_scv_extension_util*>::iterator f = _fields.begin();
        while(i--) {
            ++f;
        }
        return *f;
    }
    const _scv_extension_util* _get_field(unsigned i) const {
        if(i >= _fields.size()) {
            _scv_message::message(_scv_message::INTROSPECTION_INVALID_INDEX, i, "composite object", get_name());
            return NULL;
        }
        std::list<_scv_extension_util*>::const_iterator f = _fields.begin();
        while(i--) {
            ++f;
        }
        return *f;
    }

public: // internal methods (non-virtual for efficiency)
    void _add_field(_scv_extension_util* f) { _fields.push_back(f); }

public: // internal methods (virtual to distinguish basic/record/array)
    virtual void _set_dynamic() {
        _scv_extension_util::_set_dynamic();
        int size = _get_num_fields();
        for(int i = 0; i < size; ++i) {
            _get_field(i)->_set_dynamic();
        }
    }

protected:
    std::list<_scv_extension_util*> _fields;
};

template <typename T> class scv_extension_util : public _scv_extension_util_record {
public:
    virtual ~scv_extension_util() {}
};

// ----------------------------------------
// specialization for array
// (added cast of N to "int" since some compilers automatically
// regard it as unsigned even though I have declard it as int)
// ----------------------------------------
template <typename T, int N> class scv_extension_util<T[N]> : public _scv_extension_util {
public:
    scv_extension_util()
    : _elts(0) {}
    virtual ~scv_extension_util() {
        if(_elts)
            delete[] _elts;
    }

    // extension "util"
    virtual void set_name(const char* s) {
        _name = s;
        for(int i = 0; i < (int)N; ++i) {
            _elts[i]->_set_name(_name + "[" + _scv_ext_util_get_string(i) + "]");
        }
    }
    virtual void _set_name(const std::string& s) {
        _name = s;
        for(int i = 0; i < (int)N; ++i) {
            _elts[i]->_set_name(_name + "[" + _scv_ext_util_get_string(i) + "]");
        }
    }

protected: // fast version of the introspection interface (non-virtual)
    inline int _get_array_size() { return N; }
    inline _scv_extension_util* _get_array_elt(unsigned i) { return _elts[i]; }
    inline const _scv_extension_util* _get_array_elt(unsigned i) const { return _elts[i]; }

public: // internal methods (non-virtual for efficiency)
    inline void _set_up_array(_scv_extension_util** elts) { _elts = elts; }

public: // internal methods (virtual to distinguish basic/record/array)
    virtual void _set_dynamic() {
        _scv_extension_util::_set_dynamic();
        for(int i = 0; i < (int)N; ++i) {
            _elts[i]->_set_dynamic();
        }
    }

protected:
    _scv_extension_util** _elts;
};

// ----------------------------------------
// specialization for pointers
// ----------------------------------------
class _scv_extension_util_ptr : public _scv_extension_util {
public:
    _scv_extension_util_ptr()
    : _ptr(0) {}
    virtual ~_scv_extension_util_ptr() {}

protected: // fast version of the introspection interface (non-virtual)
    scv_extensions_if* _get_pointer() { return _ptr; }
    const scv_extensions_if* _get_pointer() const { return _ptr; }

protected:
    mutable _scv_extension_util* _ptr;
};

template <typename T> class scv_extension_util<T*> : public _scv_extension_util_ptr {
public:
    virtual ~scv_extension_util() {}
};

// ----------------------------------------
// specialization for enums
// ----------------------------------------
class _scv_extension_util_enum : public _scv_extension_util {
public:
    _scv_extension_util_enum() {}
    virtual ~_scv_extension_util_enum() {}

protected: // fast version of the introspection interface (non-virtual)
    inline int _get_enum_size() const { return static_cast<int>(_get_names().size()); }
    void _get_enum_details(std::list<const char*>&, std::list<int>&) const;
    const char* _get_enum_string(int) const;

    virtual std::list<const char*>& _get_names() const = 0;
    virtual std::list<int>& _get_values() const = 0;
};

// ----------------------------------------
// specialization for basic types
// ----------------------------------------
#define _SCV_COMPONENT(basic_type)                                                                                     \
    template <> class scv_extension_util<basic_type> : public _scv_extension_util {                                    \
    public:                                                                                                            \
        virtual ~scv_extension_util() {}                                                                               \
    };

#define _SCV_COMPONENT_1(basic_type) _SCV_COMPONENT(basic_type)

#define _SCV_COMPONENT_N(basic_type)                                                                                   \
    template <int N> class scv_extension_util<basic_type<N>> : public _scv_extension_util {                            \
    public:                                                                                                            \
        virtual ~scv_extension_util() {}                                                                               \
    };

_SCV_BASIC_TYPE_SPECIALIZATION();

#undef _SCV_COMPONENT
#undef _SCV_COMPONENT_1
#undef _SCV_COMPONENT_N

// ----------------------------------------
// specialization for records
// ----------------------------------------
template <typename T> class scv_extension_type : public scv_extension_util<T> {
public:
    scv_extension_type() {}
    virtual ~scv_extension_type() {}

    virtual scv_extension_type_if::data_type get_type() const { return scv_extension_type_if::RECORD; }

    virtual int get_enum_size() const { return 0; }
    virtual void get_enum_details(std::list<const char*>&, std::list<int>&) const {}
    virtual const char* get_enum_string(int) const { return "_error"; }

    virtual int get_num_fields() const { return this->_get_num_fields(); }
    virtual scv_extensions_if* get_field(unsigned i) { return this->_get_field(i); }
    virtual const scv_extensions_if* get_field(unsigned i) const { return this->_get_field(i); }

    virtual scv_extensions_if* get_pointer() { return 0; }
    virtual const scv_extensions_if* get_pointer() const { return 0; }

    virtual int get_array_size() const { return 0; }
    virtual scv_extensions_if* get_array_elt(int) { return 0; }
    virtual const scv_extensions_if* get_array_elt(int) const { return 0; }

    virtual int get_bitwidth() const {
        std::list<_scv_extension_util*>::const_iterator f;
        int size = 0;
        for(f = this->_fields.begin(); f != this->_fields.end(); ++f) {
            size += (*f)->get_bitwidth();
        }
        return size;
    }

    virtual scv_extensions_if* get_parent() { return this->_parent; }
    virtual const scv_extensions_if* get_parent() const { return this->_parent; }
};

// ----------------------------------------
// specialization for array
// ----------------------------------------
template <typename T, int N> class scv_extension_type<T[N]> : public scv_extension_util<T[N]> {
public:
    scv_extension_type() {}
    virtual ~scv_extension_type() {}

    virtual const char* get_type_name() const {
        static const char* s = _scv_ext_util_get_name("%s[%d]", scv_extensions<T>().get_type_name(), N);
        return s;
    }

    virtual scv_extension_type_if::data_type get_type() const { return scv_extension_type_if::ARRAY; }

    virtual int get_enum_size() const { return 0; }
    virtual void get_enum_details(std::list<const char*>&, std::list<int>&) const {}
    virtual const char* get_enum_string(int) const { return "_error"; }

    virtual int get_num_fields() const { return 0; }
    virtual scv_extensions_if* get_field(unsigned i) { return 0; }
    virtual const scv_extensions_if* get_field(unsigned i) const { return 0; }

    virtual scv_extensions_if* get_pointer() { return 0; }
    virtual const scv_extensions_if* get_pointer() const { return 0; }

    virtual int get_array_size() const { return N; }
    virtual scv_extensions_if* get_array_elt(int i) { return this->_get_array_elt(i); }
    virtual const scv_extensions_if* get_array_elt(int i) const { return this->_get_array_elt(i); }

    virtual int get_bitwidth() const { return get_array_elt(0)->get_bitwidth() * get_array_size(); }

    virtual scv_extensions_if* get_parent() { return this->_parent; }
    virtual const scv_extensions_if* get_parent() const { return this->_parent; }
};

// ----------------------------------------
// specialization for pointers
// ----------------------------------------
template <typename T> class scv_extension_type<T*> : public scv_extension_util<T*> {
public:
    scv_extension_type() {}
    virtual ~scv_extension_type() {}

    virtual const char* get_type_name() const {
        static const char* s = _scv_ext_util_get_name("%s*", scv_extensions<T>().get_type_name());
        return s;
    }

    virtual scv_extension_type_if::data_type get_type() const { return scv_extension_type_if::POINTER; }

    virtual int get_enum_size() const { return 0; }
    virtual void get_enum_details(std::list<const char*>&, std::list<int>&) const {}
    virtual const char* get_enum_string(int) const { return "_error"; }

    virtual int get_num_fields() const { return 0; }
    virtual scv_extensions_if* get_field(unsigned i) { return 0; }
    virtual const scv_extensions_if* get_field(unsigned i) const { return 0; }

    virtual scv_extensions_if* get_pointer() { return this->_get_pointer(); }
    virtual const scv_extensions_if* get_pointer() const { return this->_get_pointer(); }

    virtual int get_array_size() const { return 0; }
    virtual scv_extensions_if* get_array_elt(int) { return 0; }
    virtual const scv_extensions_if* get_array_elt(int) const { return 0; }

    virtual int get_bitwidth() const { return sizeof(T*); }

    virtual scv_extensions_if* get_parent() { return this->_parent; }
    virtual const scv_extensions_if* get_parent() const { return this->_parent; }
};

// ----------------------------------------
// specialization for enums
// ----------------------------------------
class _scv_extension_type_enum : public _scv_extension_util_enum {
public:
    _scv_extension_type_enum() {}
    virtual ~_scv_extension_type_enum() {}

    // implemented in leaf classes
    // virtual const char *get_type_name() const;

    virtual scv_extension_type_if::data_type get_type() const;

    virtual int get_enum_size() const;
    virtual void get_enum_details(std::list<const char*>&, std::list<int>&) const;
    virtual const char* get_enum_string(int) const;

    virtual int get_num_fields() const;
    virtual scv_extensions_if* get_field(unsigned);
    virtual const scv_extensions_if* get_field(unsigned) const;

    virtual scv_extensions_if* get_pointer();
    virtual const scv_extensions_if* get_pointer() const;

    virtual int get_array_size() const;
    virtual scv_extensions_if* get_array_elt(int);
    virtual const scv_extensions_if* get_array_elt(int) const;

    // implemented in leaf classes
    // virtual int get_bitwidth() const;

    virtual scv_extensions_if* get_parent() { return this->_parent; }
    virtual const scv_extensions_if* get_parent() const { return this->_parent; }

public:
    void _set_enum(int e, const char* name) {
        _get_names().push_back(name);
        _get_values().push_back(e);
    }
};

// ----------------------------------------
// specialization for basic types
// ----------------------------------------

#define _SCV_EXT_TYPE_FC_D(type_name, type_id)                                                                         \
    class _scv_extension_type_##type_id : public scv_extension_util<type_name> {                                       \
    public:                                                                                                            \
        _scv_extension_type_##type_id() {}                                                                             \
        virtual ~_scv_extension_type_##type_id() {}                                                                    \
                                                                                                                       \
        virtual int get_enum_size() const;                                                                             \
        virtual void get_enum_details(std::list<const char*>&, std::list<int>&) const;                                 \
        virtual const char* get_enum_string(int) const;                                                                \
        virtual int get_num_fields() const;                                                                            \
        virtual scv_extensions_if* get_field(unsigned);                                                                \
        virtual const scv_extensions_if* get_field(unsigned) const;                                                    \
        virtual scv_extensions_if* get_pointer();                                                                      \
        virtual const scv_extensions_if* get_pointer() const;                                                          \
        virtual int get_array_size() const;                                                                            \
        virtual scv_extensions_if* get_array_elt(int);                                                                 \
        virtual const scv_extensions_if* get_array_elt(int) const;                                                     \
        virtual scv_extensions_if* get_parent();                                                                       \
        virtual const scv_extensions_if* get_parent() const;                                                           \
                                                                                                                       \
        virtual const char* get_type_name() const;                                                                     \
        virtual scv_extension_type_if::data_type get_type() const;                                                     \
        virtual int get_bitwidth() const;                                                                              \
    };                                                                                                                 \
                                                                                                                       \
    template <> class scv_extension_type<type_name> : public _scv_extension_type_##type_id {                           \
    public:                                                                                                            \
        scv_extension_type() {}                                                                                        \
        virtual ~scv_extension_type() {}                                                                               \
    };

#define _SCV_EXT_TYPE_1_FC_D(type_name, type_id) _SCV_EXT_TYPE_FC_D(type_name, type_id)

#define _SCV_EXT_TYPE_N_FC_D(type_name, id)                                                                            \
    template <int N> class scv_extension_type<type_name> : public scv_extension_util<type_name> {                      \
    public:                                                                                                            \
        scv_extension_type() {}                                                                                        \
        virtual ~scv_extension_type() {}                                                                               \
                                                                                                                       \
        virtual int get_enum_size() const { return 0; }                                                                \
        virtual void get_enum_details(std::list<const char*>&, std::list<int>&) const {}                               \
        virtual const char* get_enum_string(int) const { return "_error"; }                                            \
        virtual int get_num_fields() const { return 0; }                                                               \
        virtual scv_extensions_if* get_field(unsigned) { return 0; }                                                   \
        virtual const scv_extensions_if* get_field(unsigned) const { return 0; }                                       \
        virtual scv_extensions_if* get_pointer() { return 0; }                                                         \
        virtual const scv_extensions_if* get_pointer() const { return 0; }                                             \
        virtual int get_array_size() const { return 0; }                                                               \
        virtual scv_extensions_if* get_array_elt(int) { return 0; }                                                    \
        virtual const scv_extensions_if* get_array_elt(int) const { return 0; }                                        \
        virtual scv_extensions_if* get_parent() { return this->_parent; }                                              \
        virtual const scv_extensions_if* get_parent() const { return this->_parent; }                                  \
                                                                                                                       \
        virtual const char* get_type_name() const {                                                                    \
            static const char* s = _scv_ext_util_get_name("%s<%d>", #type_name, N);                                    \
            return s;                                                                                                  \
        }                                                                                                              \
        virtual scv_extension_type_if::data_type get_type() const { return scv_extensions_if::id; }                    \
        virtual int get_bitwidth() const { return N; }                                                                 \
    };

#define _SCV_EXT_TYPE_D_FC_D(type_name, type_id)                                                                       \
    class _scv_extension_type_##type_id : public scv_extension_util<type_name> {                                       \
    public:                                                                                                            \
        _scv_extension_type_##type_id();                                                                               \
        virtual ~_scv_extension_type_##type_id();                                                                      \
                                                                                                                       \
        virtual int get_enum_size() const;                                                                             \
        virtual void get_enum_details(std::list<const char*>&, std::list<int>&) const;                                 \
        virtual const char* get_enum_string(int) const;                                                                \
        virtual int get_num_fields() const;                                                                            \
        virtual scv_extensions_if* get_field(unsigned);                                                                \
        virtual const scv_extensions_if* get_field(unsigned) const;                                                    \
        virtual scv_extensions_if* get_pointer();                                                                      \
        virtual const scv_extensions_if* get_pointer() const;                                                          \
        virtual int get_array_size() const;                                                                            \
        virtual scv_extensions_if* get_array_elt(int);                                                                 \
        virtual const scv_extensions_if* get_array_elt(int) const;                                                     \
        virtual scv_extensions_if* get_parent();                                                                       \
        virtual const scv_extensions_if* get_parent() const;                                                           \
                                                                                                                       \
        virtual const char* get_type_name() const;                                                                     \
        virtual scv_extension_type_if::data_type get_type() const;                                                     \
        virtual int get_bitwidth() const;                                                                              \
        int _bitwidth;                                                                                                 \
    };                                                                                                                 \
                                                                                                                       \
    template <> class scv_extension_type<type_name> : public _scv_extension_type_##type_id {                           \
    public:                                                                                                            \
        scv_extension_type() {}                                                                                        \
        virtual ~scv_extension_type() {}                                                                               \
    };

_SCV_EXT_TYPE_FC_D(bool, bool);
_SCV_EXT_TYPE_FC_D(char, char);
_SCV_EXT_TYPE_FC_D(short, short);
_SCV_EXT_TYPE_FC_D(int, int);
_SCV_EXT_TYPE_FC_D(long, long);
_SCV_EXT_TYPE_FC_D(long long, long_long);
_SCV_EXT_TYPE_FC_D(unsigned char, unsigned_char);
_SCV_EXT_TYPE_FC_D(unsigned short, unsigned_short);
_SCV_EXT_TYPE_FC_D(unsigned int, unsigned_int);
_SCV_EXT_TYPE_FC_D(unsigned long, unsigned_long);
_SCV_EXT_TYPE_FC_D(unsigned long long, unsigned_long_long);
_SCV_EXT_TYPE_FC_D(float, float);
_SCV_EXT_TYPE_FC_D(double, double);
_SCV_EXT_TYPE_FC_D(std::string, string);

#ifdef TEST_NEST_TEMPLATE
_SCV_EXT_TYPE_N_FC_D(test_uint<N>, UNSIGNED);
#endif

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
_SCV_EXT_TYPE_N_FC_D(sc_dt::sc_int<N>, INTEGER);
_SCV_EXT_TYPE_N_FC_D(sc_dt::sc_bigint<N>, INTEGER);
_SCV_EXT_TYPE_N_FC_D(sc_dt::sc_uint<N>, UNSIGNED);
_SCV_EXT_TYPE_N_FC_D(sc_dt::sc_biguint<N>, UNSIGNED);
_SCV_EXT_TYPE_1_FC_D(sc_dt::sc_bit, sc_bit);
_SCV_EXT_TYPE_N_FC_D(sc_dt::sc_bv<N>, BIT_VECTOR);
_SCV_EXT_TYPE_1_FC_D(sc_dt::sc_logic, sc_logic);
_SCV_EXT_TYPE_N_FC_D(sc_dt::sc_lv<N>, LOGIC_VECTOR);
// SCV_EXT_TYPE_N_FC_D(sc_fixed,FIXED_POINTER_INTEGER);
// SCV_EXT_TYPE_N_FC_D(sc_ufixed,UNSIGNED_FIXED_POINTER_INTEGER);
_SCV_EXT_TYPE_D_FC_D(sc_dt::sc_signed, sc_signed);
_SCV_EXT_TYPE_D_FC_D(sc_dt::sc_unsigned, sc_unsigned);
_SCV_EXT_TYPE_D_FC_D(sc_dt::sc_int_base, sc_int_base);
_SCV_EXT_TYPE_D_FC_D(sc_dt::sc_uint_base, sc_uint_base);
_SCV_EXT_TYPE_D_FC_D(sc_dt::sc_lv_base, sc_lv_base);
_SCV_EXT_TYPE_D_FC_D(sc_dt::sc_bv_base, sc_bv_base);
#endif

#undef _SCV_EXT_TYPE_FC_D
#undef _SCV_EXT_TYPE_N_FC_D
#undef _SCV_EXT_TYPE_1_FC_D

// ----------------------------------------
// this extension component introduces
// several public methods for access when
// type information is known.
//
// const T& scv_extensions<T>::read() const;
// void scv_extensions<T>::write(const T&);
//
// ----------------------------------------

// ----------------------------------------
// default implementation of various interfaces
// ----------------------------------------

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
#define _SCV_DEFAULT_RW_SYSC                                                                                           \
    virtual void assign(const sc_dt::sc_bv_base& v) { assert(0); }                                                     \
    virtual void get_value(sc_dt::sc_bv_base& v) const { assert(0); }                                                  \
    virtual void assign(const sc_dt::sc_lv_base& v) { assert(0); }                                                     \
    virtual void get_value(sc_dt::sc_lv_base& v) const { assert(0); }
#else
#define _SCV_DEFAULT_RW_SYSC
#endif
#define _SCV_DEFAULT_RW                                                                                                \
    virtual void assign(bool) { assert(0); }                                                                           \
    virtual void assign(char) { assert(0); }                                                                           \
    virtual void assign(unsigned char) { assert(0); }                                                                  \
    virtual void assign(short) { assert(0); }                                                                          \
    virtual void assign(unsigned short) { assert(0); }                                                                 \
    virtual void assign(int) { assert(0); }                                                                            \
    virtual void assign(unsigned) { assert(0); }                                                                       \
    virtual void assign(long) { assert(0); }                                                                           \
    virtual void assign(unsigned long) { assert(0); }                                                                  \
    virtual void assign(long long) { assert(0); }                                                                      \
    virtual void assign(unsigned long long) { assert(0); }                                                             \
    virtual void assign(float) { assert(0); }                                                                          \
    virtual void assign(double) { assert(0); }                                                                         \
    virtual void assign(const std::string&) { assert(0); }                                                             \
    virtual void assign(const char*) { assert(0); }                                                                    \
                                                                                                                       \
    virtual bool get_bool() const {                                                                                    \
        assert(0);                                                                                                     \
        return false;                                                                                                  \
    }                                                                                                                  \
    virtual long long get_integer() const {                                                                            \
        assert(0);                                                                                                     \
        return 0;                                                                                                      \
    }                                                                                                                  \
    virtual unsigned long long get_unsigned() const {                                                                  \
        assert(0);                                                                                                     \
        return 0;                                                                                                      \
    }                                                                                                                  \
    virtual double get_double() const {                                                                                \
        assert(0);                                                                                                     \
        return 0;                                                                                                      \
    }                                                                                                                  \
    virtual std::string get_string() const {                                                                           \
        assert(0);                                                                                                     \
        return std::string("");                                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    _SCV_DEFAULT_RW_SYSC

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
#define _SCV_INTROSPECTION_RW_FC_D_SYSC                                                                                \
    virtual void assign(const sc_dt::sc_bv_base& v);                                                                   \
    virtual void get_value(sc_dt::sc_bv_base& v) const;                                                                \
    virtual void assign(const sc_dt::sc_lv_base& v);                                                                   \
    virtual void get_value(sc_dt::sc_lv_base& v) const;
#else
#define _SCV_INTROSPECTION_RW_FC_D_SYSC
#endif
#define _SCV_INTROSPECTION_RW_FC_D                                                                                     \
    virtual void assign(bool);                                                                                         \
    virtual void assign(char);                                                                                         \
    virtual void assign(unsigned char);                                                                                \
    virtual void assign(short);                                                                                        \
    virtual void assign(unsigned short);                                                                               \
    virtual void assign(int);                                                                                          \
    virtual void assign(unsigned);                                                                                     \
    virtual void assign(long);                                                                                         \
    virtual void assign(unsigned long);                                                                                \
    virtual void assign(long long);                                                                                    \
    virtual void assign(unsigned long long);                                                                           \
    virtual void assign(float);                                                                                        \
    virtual void assign(double);                                                                                       \
    virtual void assign(const std::string&);                                                                           \
    virtual void assign(const char*);                                                                                  \
                                                                                                                       \
    virtual bool get_bool() const;                                                                                     \
    virtual long long get_integer() const;                                                                             \
    virtual unsigned long long get_unsigned() const;                                                                   \
    virtual double get_double() const;                                                                                 \
    virtual std::string get_string() const;                                                                            \
                                                                                                                       \
    _SCV_INTROSPECTION_RW_FC_D_SYSC

#define _SCV_IMPLEMENT_RW(type_id)                                                                                     \
    const type_id& read() const { return *_get_instance(); }                                                           \
    void write(const type_id& rhs) {                                                                                   \
        *_get_instance() = rhs;                                                                                        \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    void _set_instance(type_id* p) {                                                                                   \
        _instance = p;                                                                                                 \
        _set_instance_core_wrap(p);                                                                                    \
    }                                                                                                                  \
    void _set_as_field(_scv_extension_util_record* parent, type_id* p, const std::string& name) {                      \
        if(p)                                                                                                          \
            _set_instance(p);                                                                                          \
        else if(!this->_get_parent()) {                                                                                \
            this->_set_parent(parent, name);                                                                           \
            parent->_add_field(this);                                                                                  \
        }                                                                                                              \
    }                                                                                                                  \
    type_id* _get_instance() const { return _instance; }                                                               \
    type_id* get_instance() {                                                                                          \
        _scv_message::message(_scv_message::INTROSPECTION_GET_INSTANCE_USAGE);                                         \
        return _instance;                                                                                              \
    }                                                                                                                  \
    const type_id* get_instance() const { return _instance; }

#define _SCV_IMPLEMENT_RW_FULL(type_id)                                                                                \
    _SCV_IMPLEMENT_RW(type_id);                                                                                        \
    virtual void _set_instance_core_wrap(void* p) {}                                                                   \
    type_id* _instance{nullptr};

// ----------------------------------------
// specialization for records
// ----------------------------------------
template <typename T> class _scv_extension_rw_base : public scv_extension_type<T> {
public:
    _scv_extension_rw_base() {}
    virtual ~_scv_extension_rw_base() {}

public: // public API for use only when full type information is available
    const T* get_instance() const { return _instance; }
    T* get_instance() { return _instance; }
    T* _get_instance() const { return _instance; }

public: // internal API for implementation only
    void _set_instance(T* p) {
        _instance = p;
        _set_instance_core_wrap(p);
    }
    virtual void _set_instance_core_wrap(void* p) {}
    void _set_as_field(_scv_extension_util_record* parent, T* p, const std::string& name) {
        if(p)
            _set_instance(p);
        else if(!this->_get_parent()) {
            this->_set_parent(parent, name);
            parent->_add_field(this);
        }
    }

public:
    _SCV_DEFAULT_RW
    const T& read() { return *get_instance(); }
    void write(const T& rhs) {
        *_get_instance() = rhs;
        this->trigger_value_change_cb();
    }

public:
    T* _instance{nullptr};
};

template <typename T> class scv_extension_rw : public _scv_extension_rw_base<T> {};

// Rachida
// ----------------------------------------
// specialization for array
// ----------------------------------------
template <typename T, int N> class scv_extension_rw<T[N]> : public scv_extension_type<T[N]> {
    typedef T my_type[N];

public:
    scv_extension_rw() {}
    virtual ~scv_extension_rw() {}

public:
    _SCV_IMPLEMENT_RW_FULL(my_type)
    _SCV_INTROSPECTION_RW_FC_D
};

template <typename T, int N> void scv_extension_rw<T[N]>::assign(bool) { _SCV_RW_ERROR(assign, bool, array); }
template <typename T, int N> void scv_extension_rw<T[N]>::assign(char) { _SCV_RW_ERROR(assign, char, array); }
template <typename T, int N> void scv_extension_rw<T[N]>::assign(unsigned char) {
    _SCV_RW_ERROR(assign, unsigned char, array);
}
template <typename T, int N> void scv_extension_rw<T[N]>::assign(short) { _SCV_RW_ERROR(assign, short, array); }
template <typename T, int N> void scv_extension_rw<T[N]>::assign(unsigned short) {
    _SCV_RW_ERROR(assign, unsigned short, array);
}
template <typename T, int N> void scv_extension_rw<T[N]>::assign(int) { _SCV_RW_ERROR(assign, int, array); }
template <typename T, int N> void scv_extension_rw<T[N]>::assign(unsigned) { _SCV_RW_ERROR(assign, unsigned, array); }
template <typename T, int N> void scv_extension_rw<T[N]>::assign(long) { _SCV_RW_ERROR(assign, long, array); }
template <typename T, int N> void scv_extension_rw<T[N]>::assign(unsigned long) {
    _SCV_RW_ERROR(assign, unsigned long, array);
}
template <typename T, int N> void scv_extension_rw<T[N]>::assign(long long) { _SCV_RW_ERROR(assign, long long, array); }
template <typename T, int N> void scv_extension_rw<T[N]>::assign(unsigned long long) {
    _SCV_RW_ERROR(assign, unsigned long long, array);
}
template <typename T, int N> void scv_extension_rw<T[N]>::assign(float) { _SCV_RW_ERROR(assign, float, array); }
template <typename T, int N> void scv_extension_rw<T[N]>::assign(double) { _SCV_RW_ERROR(assign, double, array); }
template <typename T, int N> void scv_extension_rw<T[N]>::assign(const std::string&) {
    _SCV_RW_ERROR(assign, std::string, array);
}
template <typename T, int N> void scv_extension_rw<T[N]>::assign(const char*) {
    _SCV_RW_ERROR(assign, const char*, array);
}

template <typename T, int N> bool scv_extension_rw<T[N]>::get_bool() const {
    _SCV_RW_ERROR(get_bool, bool, array);
    return false;
}
template <typename T, int N> long long scv_extension_rw<T[N]>::get_integer() const {
    _SCV_RW_ERROR(get_integer, integer, array);
    return 0;
}
template <typename T, int N> unsigned long long scv_extension_rw<T[N]>::get_unsigned() const {
    _SCV_RW_ERROR(get_unsigned, unsigned, array);
    return 0;
}
template <typename T, int N> double scv_extension_rw<T[N]>::get_double() const {
    _SCV_RW_ERROR(get_double, double, array);
    return 0;
}
template <typename T, int N> std::string scv_extension_rw<T[N]>::get_string() const {
    _SCV_RW_ERROR(get_string, string, array);
    return std::string("");
}

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
template <typename T, int N> void scv_extension_rw<T[N]>::assign(const sc_dt::sc_bv_base& v) {
    _SCV_RW_ERROR(assign, sc_dt::sc_bv_base, array);
}
template <typename T, int N> void scv_extension_rw<T[N]>::get_value(sc_dt::sc_bv_base& v) const {
    _SCV_RW_ERROR(get_value, sc_dt::sc_bv_base, array);
}
template <typename T, int N> void scv_extension_rw<T[N]>::assign(const sc_dt::sc_lv_base& v) {
    _SCV_RW_ERROR(assign, sc_dt::sc_lv_base, array);
}
template <typename T, int N> void scv_extension_rw<T[N]>::get_value(sc_dt::sc_lv_base& v) const {
    _SCV_RW_ERROR(get_value, sc_dt::sc_lv_base, array);
}
#endif
// ----------------------------------------
// specialization for pointers
// ----------------------------------------
template <typename T> class scv_extension_rw<T*> : public scv_extension_type<T*> {
public: // public API for use only when full type information is available
    T** const get_instance() const { return _instance; }
    T** get_instance() { return _instance; }
    T** _get_instance() const { return _instance; }

public:
    void _set_instance(T** p) { _instance = p; }
    void _set_as_field(_scv_extension_util_record* parent, T** p, const std::string& name) {
        if(p)
            _set_instance(p);
        else if(!this->_get_parent()) {
            this->_set_parent(parent, name);
            parent->_add_field(this);
        }
    }

public:
    _SCV_DEFAULT_RW
    const T& read() { return *get_instance(); }
    void write(const T& rhs) {
        *_get_instance() = rhs;
        this->trigger_value_change_cb();
    }

public:
    T** _instance;
};

// ----------------------------------------
// specialization for enums
// ----------------------------------------
class _scv_extension_rw_enum : public _scv_extension_type_enum {
public:
    _scv_extension_rw_enum() {}
    virtual ~_scv_extension_rw_enum() {}

    _SCV_INTROSPECTION_RW_FC_D

    int read() const { return *_get_instance(); }
    void write(int rhs) {
        *_get_instance() = rhs;
        this->trigger_value_change_cb();
    }
    void _set_instance(int* p) { _instance = p; }
    void _set_as_field(_scv_extension_util_record* parent, int* p, const std::string& name) {
        if(p)
            _set_instance(p);
        else if(!this->_get_parent()) {
            _set_parent(parent, name);
            parent->_add_field(this);
        }
    }
    int* _get_instance() const { return _instance; }
    int* get_instance() {
        _scv_message::message(_scv_message::INTROSPECTION_GET_INSTANCE_USAGE);
        return _instance;
    }
    const int* get_instance() const { return _instance; }

    int* _instance{nullptr};
};

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
#define _SCV_EXT_RW_FC_COMMON_SYSC_D                                                                                   \
    virtual void assign(const sc_dt::sc_bv_base& v);                                                                   \
    virtual void get_value(sc_dt::sc_bv_base& v) const;                                                                \
    virtual void assign(const sc_dt::sc_lv_base& v);                                                                   \
    virtual void get_value(sc_dt::sc_lv_base& v) const;
#else
#define _SCV_EXT_RW_FC_COMMON_SYSC_D
#endif

// ----------------------------------------
// specialization for basic types
// ----------------------------------------

#define _SCV_EXT_RW_FC_D(basic_type, type_id)                                                                          \
    class _scv_extension_rw_##type_id : public scv_extension_type<basic_type> {                                        \
    public:                                                                                                            \
        _scv_extension_rw_##type_id();                                                                                 \
        virtual ~_scv_extension_rw_##type_id();                                                                        \
                                                                                                                       \
    public: /* public API for use only when full type information is available */                                      \
        const basic_type* get_instance() const;                                                                        \
        basic_type* get_instance();                                                                                    \
        basic_type* _get_instance() const;                                                                             \
                                                                                                                       \
    public: /* internal API for implementation only */                                                                 \
        void _set_instance(basic_type* p);                                                                             \
        virtual void _set_instance_core_wrap(void* p);                                                                 \
        void _set_as_field(_scv_extension_util_record* parent, basic_type* p, const std::string& name);                \
                                                                                                                       \
    public:                                                                                                            \
        virtual void assign(bool);                                                                                     \
        virtual void assign(char);                                                                                     \
        virtual void assign(unsigned char);                                                                            \
        virtual void assign(short);                                                                                    \
        virtual void assign(unsigned short);                                                                           \
        virtual void assign(int);                                                                                      \
        virtual void assign(unsigned);                                                                                 \
        virtual void assign(long);                                                                                     \
        virtual void assign(unsigned long);                                                                            \
        virtual void assign(long long);                                                                                \
        virtual void assign(unsigned long long);                                                                       \
        virtual void assign(float);                                                                                    \
        virtual void assign(double);                                                                                   \
        virtual void assign(const std::string&);                                                                       \
        virtual void assign(const char*);                                                                              \
                                                                                                                       \
        virtual bool get_bool() const;                                                                                 \
        virtual long long get_integer() const;                                                                         \
        virtual unsigned long long get_unsigned() const;                                                               \
        virtual double get_double() const;                                                                             \
        virtual std::string get_string() const;                                                                        \
                                                                                                                       \
        _SCV_EXT_RW_FC_COMMON_SYSC_D                                                                                   \
        const basic_type& read();                                                                                      \
        void write(const basic_type& rhs);                                                                             \
                                                                                                                       \
    public:                                                                                                            \
        basic_type* _instance;                                                                                         \
    };                                                                                                                 \
                                                                                                                       \
    template <> class scv_extension_rw<basic_type> : public _scv_extension_rw_##type_id {                              \
    public:                                                                                                            \
        scv_extension_rw() {}                                                                                          \
        virtual ~scv_extension_rw() {}                                                                                 \
    };

// ------------------------------------------------------------
// C/C++ Types
// ------------------------------------------------------------

// --------------
// integer types
// --------------

_SCV_EXT_RW_FC_D(bool, bool)
_SCV_EXT_RW_FC_D(char, char)
_SCV_EXT_RW_FC_D(unsigned char, unsigned_char)
_SCV_EXT_RW_FC_D(short, short)
_SCV_EXT_RW_FC_D(unsigned short, unsigned_short)
_SCV_EXT_RW_FC_D(int, int)
_SCV_EXT_RW_FC_D(unsigned int, unsigned_int)
_SCV_EXT_RW_FC_D(long, long)
_SCV_EXT_RW_FC_D(unsigned long, unsigned_long)
_SCV_EXT_RW_FC_D(long long, long_long)
_SCV_EXT_RW_FC_D(unsigned long long, unsigned_long_long)

// --------------
// floating pointer types
// --------------

_SCV_EXT_RW_FC_D(float, float)
_SCV_EXT_RW_FC_D(double, double)

// --------------
// string type
// --------------

_SCV_EXT_RW_FC_D(std::string, string)

// ------------------------------------------------------------
// SystemC Types
// ------------------------------------------------------------

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)

// --------------
// sc_bit
// --------------

_SCV_EXT_RW_FC_D(sc_dt::sc_bit, sc_bit)

// --------------
// sc_logic
// --------------

_SCV_EXT_RW_FC_D(sc_dt::sc_logic, sc_logic)

_SCV_EXT_RW_FC_D(sc_dt::sc_signed, sc_signed)
_SCV_EXT_RW_FC_D(sc_dt::sc_unsigned, sc_unsigned)
_SCV_EXT_RW_FC_D(sc_dt::sc_int_base, sc_int_base)
_SCV_EXT_RW_FC_D(sc_dt::sc_uint_base, sc_uint_base)
_SCV_EXT_RW_FC_D(sc_dt::sc_lv_base, sc_lv_base)
_SCV_EXT_RW_FC_D(sc_dt::sc_bv_base, sc_bv_base)

// --------------
// sc_int and sc_uint (begin)
// sc_bigint and sc_biguint (begin)
// sc_bv and sc_lv (begin)
// --------------

#define _SCV_EXT_RW_FC_N_BASE(T)                                                                                       \
public: /* public API for use only when full type information is available */                                          \
    const T* get_instance() const { return _instance; }                                                                \
    T* get_instance() { return _instance; }                                                                            \
    T* _get_instance() const { return _instance; }                                                                     \
                                                                                                                       \
public: /* internal API for implementation only */                                                                     \
    void _set_instance(T* p) {                                                                                         \
        _instance = p;                                                                                                 \
        _set_instance_core_wrap(p);                                                                                    \
    }                                                                                                                  \
    virtual void _set_instance_core_wrap(void* p) {}                                                                   \
    void _set_as_field(_scv_extension_util_record* parent, T* p, const std::string& name) {                            \
        if(p)                                                                                                          \
            _set_instance(p);                                                                                          \
        else if(!this->_get_parent()) {                                                                                \
            this->_set_parent(parent, name);                                                                           \
            parent->_add_field(this);                                                                                  \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
public:                                                                                                                \
    const T& read() { return *get_instance(); }                                                                        \
    void write(const T& rhs) {                                                                                         \
        *_get_instance() = rhs;                                                                                        \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
                                                                                                                       \
public:                                                                                                                \
    T* _instance{nullptr};

#define _SCV_EXT_RW_FC_N_ASSIGN(type_name, arg_name)                                                                   \
    virtual void assign(arg_name i) {                                                                                  \
        *(this->_get_instance()) = i;                                                                                  \
        this->trigger_value_change_cb();                                                                               \
    }

#define _SCV_EXT_RW_FC_N_BAD_ASSIGN(type_name, arg_name)                                                               \
    virtual void assign(arg_name i) { _SCV_RW_ERROR(assign, arg_name, type_name); }

#define _SCV_EXT_RW_FC_N_ASSIGNS(type_name)                                                                            \
    _SCV_EXT_RW_FC_N_ASSIGN(type_name, bool)                                                                           \
    _SCV_EXT_RW_FC_N_ASSIGN(type_name, char)                                                                           \
    _SCV_EXT_RW_FC_N_ASSIGN(type_name, unsigned char)                                                                  \
    _SCV_EXT_RW_FC_N_ASSIGN(type_name, short)                                                                          \
    _SCV_EXT_RW_FC_N_ASSIGN(type_name, unsigned short)                                                                 \
    _SCV_EXT_RW_FC_N_ASSIGN(type_name, int)                                                                            \
    _SCV_EXT_RW_FC_N_ASSIGN(type_name, unsigned int)                                                                   \
    _SCV_EXT_RW_FC_N_ASSIGN(type_name, long)                                                                           \
    _SCV_EXT_RW_FC_N_ASSIGN(type_name, unsigned long)                                                                  \
    _SCV_EXT_RW_FC_N_ASSIGN(type_name, long long)                                                                      \
    _SCV_EXT_RW_FC_N_ASSIGN(type_name, unsigned long long)                                                             \
    _SCV_EXT_RW_FC_N_BAD_ASSIGN(type_name, float)                                                                      \
    _SCV_EXT_RW_FC_N_BAD_ASSIGN(type_name, double)

#define _SCV_EXT_RW_FC_N_ASSIGNS_STRING(type_name)                                                                     \
    virtual void assign(const std::string& s) {                                                                        \
        *(this->get_instance()) = s.c_str();                                                                           \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    virtual void assign(const char* s) {                                                                               \
        *(this->get_instance()) = s;                                                                                   \
        this->trigger_value_change_cb();                                                                               \
    }

#define _SCV_EXT_RW_FC_N_ASSIGNS_GET(type_name)                                                                        \
    virtual bool get_bool() const { return *(this->_get_instance()) != (type_name)0; }                                 \
    virtual long long get_integer() const { return this->_get_instance()->to_int64(); }                                \
    virtual unsigned long long get_unsigned() const { return this->_get_instance()->to_uint64(); }                     \
    virtual double get_double() const { return this->_get_instance()->to_double(); }                                   \
    virtual std::string get_string() const { return this->get_instance()->to_string(); }

#define _SCV_EXT_RW_FC_N_ASSIGNS_SYSC(type_name)                                                                       \
    virtual void assign(const sc_dt::sc_bv_base& v) {                                                                  \
        if(this->get_bitwidth() != v.length())                                                                         \
            _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, "sc_bv_base", "assign");    \
        *(this->_get_instance()) = v;                                                                                  \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    virtual void get_value(sc_dt::sc_bv_base& v) const {                                                               \
        if(this->get_bitwidth() != v.length())                                                                         \
            _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, "sc_bv_base", "get_value"); \
        /*this->initialize();*/                                                                                            \
        v = *(this->_get_instance());                                                                                  \
    }                                                                                                                  \
    virtual void assign(const sc_dt::sc_lv_base& v) {                                                                  \
        if(this->get_bitwidth() != v.length())                                                                         \
            _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, "sc_lv_base", "assign");    \
        *(this->_get_instance()) = v;                                                                                  \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    virtual void get_value(sc_dt::sc_lv_base& v) const {                                                               \
        if(this->get_bitwidth() != v.length())                                                                         \
            _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, "sc_lv_base", "get_value"); \
        /*this->initialize();*/                                                                                            \
        v = *(this->_get_instance());                                                                                  \
    }

// --------------
// sc_int
// --------------
template <int N> class scv_extension_rw<sc_dt::sc_int<N>> : public scv_extension_type<sc_dt::sc_int<N>> {
public:
    scv_extension_rw() {}
    virtual ~scv_extension_rw() {}

public:
    _SCV_EXT_RW_FC_N_BASE(sc_dt::sc_int<N>)
    _SCV_EXT_RW_FC_N_ASSIGNS(sc_dt::sc_int)
    _SCV_EXT_RW_FC_N_BAD_ASSIGN(sc_dt::sc_int, const std::string&)
    _SCV_EXT_RW_FC_N_BAD_ASSIGN(sc_dt::sc_int, const char*)
    _SCV_EXT_RW_FC_N_ASSIGNS_GET(sc_dt::sc_int<N>)
    _SCV_EXT_RW_FC_N_ASSIGNS_SYSC(sc_dt::sc_int)
};

// --------------
// sc_uint
// --------------
template <int N> class scv_extension_rw<sc_dt::sc_uint<N>> : public scv_extension_type<sc_dt::sc_uint<N>> {
public:
    scv_extension_rw() {}
    virtual ~scv_extension_rw() {}

public:
    _SCV_EXT_RW_FC_N_BASE(sc_dt::sc_uint<N>)
    _SCV_EXT_RW_FC_N_ASSIGNS(sc_dt::sc_uint)
    _SCV_EXT_RW_FC_N_BAD_ASSIGN(sc_dt::sc_uint, const std::string&)
    _SCV_EXT_RW_FC_N_BAD_ASSIGN(sc_dt::sc_uint, const char*)
    _SCV_EXT_RW_FC_N_ASSIGNS_GET(sc_dt::sc_uint<N>)
    _SCV_EXT_RW_FC_N_ASSIGNS_SYSC(sc_dt::sc_uint)
};

// --------------
// sc_bigint
// --------------
template <int N> class scv_extension_rw<sc_dt::sc_bigint<N>> : public scv_extension_type<sc_dt::sc_bigint<N>> {
public:
    scv_extension_rw() {}
    virtual ~scv_extension_rw() {}

public:
    _SCV_EXT_RW_FC_N_BASE(sc_dt::sc_bigint<N>)
    _SCV_EXT_RW_FC_N_ASSIGNS(sc_dt::sc_bigint)
    _SCV_EXT_RW_FC_N_ASSIGNS_STRING(sc_dt::sc_bigint)
    _SCV_EXT_RW_FC_N_ASSIGNS_GET(sc_dt::sc_bigint<N>)
    _SCV_EXT_RW_FC_N_ASSIGNS_SYSC(sc_dt::sc_bigint)
};

// --------------
// sc_biguint
// --------------
template <int N> class scv_extension_rw<sc_dt::sc_biguint<N>> : public scv_extension_type<sc_dt::sc_biguint<N>> {
public:
    scv_extension_rw() {}
    virtual ~scv_extension_rw() {}

public:
    _SCV_EXT_RW_FC_N_BASE(sc_dt::sc_biguint<N>)
    _SCV_EXT_RW_FC_N_ASSIGNS(sc_dt::sc_biguint)
    _SCV_EXT_RW_FC_N_ASSIGNS_STRING(sc_dt::sc_biguint)
    _SCV_EXT_RW_FC_N_ASSIGNS_GET(sc_dt::sc_biguint<N>)
    _SCV_EXT_RW_FC_N_ASSIGNS_SYSC(sc_dt::sc_biguint)
};

// --------------
// sc_bv
// --------------
template <int N> class scv_extension_rw<sc_dt::sc_bv<N>> : public scv_extension_type<sc_dt::sc_bv<N>> {
public:
    scv_extension_rw() {}
    virtual ~scv_extension_rw() {}

public:
    _SCV_EXT_RW_FC_N_BASE(sc_dt::sc_bv<N>)
    _SCV_EXT_RW_FC_N_ASSIGNS(sc_dt::sc_bv)
    _SCV_EXT_RW_FC_N_ASSIGNS_STRING(sc_bv)
    virtual bool get_bool() const { return *(this->_get_instance()) != 0; }
    virtual long long get_integer() const {
        static sc_dt::sc_bigint<N> tmp;
        tmp = *this->_get_instance();
        return tmp.to_int64();
    }
    virtual unsigned long long get_unsigned() const {
        static sc_dt::sc_bigint<N> tmp;
        tmp = *this->_get_instance();
        return tmp.to_uint64();
    }
    virtual double get_double() const {
        static sc_dt::sc_bigint<N> tmp;
        tmp = *this->_get_instance();
        return tmp.to_double();
    }
    virtual std::string get_string() const { return this->get_instance()->to_string(); }
    _SCV_EXT_RW_FC_N_ASSIGNS_SYSC(sc_dt::sc_bv);
};

// --------------
// sc_lv
// --------------
template <int N> class scv_extension_rw<sc_dt::sc_lv<N>> : public scv_extension_type<sc_dt::sc_lv<N>> {
public:
    scv_extension_rw() {}
    virtual ~scv_extension_rw() {}

public:
    _SCV_EXT_RW_FC_N_BASE(sc_dt::sc_lv<N>)
    _SCV_EXT_RW_FC_N_ASSIGNS(sc_dt::sc_lv)
    _SCV_EXT_RW_FC_N_ASSIGNS_STRING(sc_lv)
    virtual bool get_bool() const { return *(this->_get_instance()) != 0; }
    virtual long long get_integer() const {
        static sc_dt::sc_bigint<N> tmp;
        tmp = *this->_get_instance();
        return tmp.to_int64();
    }
    virtual unsigned long long get_unsigned() const {
        static sc_dt::sc_bigint<N> tmp;
        tmp = *this->_get_instance();
        return tmp.to_uint64();
    }
    virtual double get_double() const {
        static sc_dt::sc_bigint<N> tmp;
        tmp = *this->_get_instance();
        return tmp.to_double();
    }
    virtual std::string get_string() const { return this->get_instance()->to_string(); }
    _SCV_EXT_RW_FC_N_ASSIGNS_SYSC(sc_dt::sc_lv);
};

// --------------
// sc_int and sc_uint (end)
// sc_bigint and sc_biguint (end)
// sc_bv and sc_lv (end)
// --------------
#undef _SCV_EXT_RW_FC_N_BASE
#undef _SCV_EXT_RW_FC_N_ASSIGN
#undef _SCV_EXT_RW_FC_N_BAD_ASSIGN
#undef _SCV_EXT_RW_FC_N_ASSIGNS
#undef _SCV_EXT_RW_FC_N_ASSIGNS_STRING
#undef _SCV_EXT_RW_FC_N_ASSIGNS_GET
#undef _SCV_EXT_RW_FC_N_ASSIGNS_SYSC

#endif

// ----------------------------------------
// wrap up this component
// ----------------------------------------
#undef _SCV_ASSIGN
#undef _SCV_BAD_ASSIGN

#undef _SCV_DEFAULT_RW_SYSC
#undef _SCV_DEFAULT_RW

#undef _SCV_SYSTEMC_BASIC_TYPE_SPECIALIZATION
#undef _SCV_BASIC_TYPE_SPECIALIZATION

// to be used as base class of your composite type
template <typename T> class scv_extensions_base : public scv_extension_rw<T> {
public:
    virtual ~scv_extensions_base() {}
};

// to be used as base class of your enum type
template <typename T> class scv_enum_base : public _scv_extension_rw_enum {
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
    T read() const { return (T)_scv_extension_rw_enum::read(); }
    void write(const T rhs) { _scv_extension_rw_enum::write((int)rhs); }
    void _set_instance(T* p) { _scv_extension_rw_enum::_set_instance((int*)p); }
    void _set_as_field(_scv_extension_util_record* parent, T* p, const std::string& name) {
        _scv_extension_rw_enum::_set_as_field(parent, (int*)p, name);
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

// ----------------------------------------
// final definition of the extension classes for builtin C/C++ types
// ----------------------------------------

#define _SCV_OSTREAM(type_name)                                                                                        \
    friend std::ostream& operator<<(std::ostream& os, const scv_extensions<type_name>& e) {                            \
        os << *e._get_instance();                                                                                      \
        return os;                                                                                                     \
    }

template <> class scv_extensions<bool> : public scv_extensions_base<bool> {
public:
    scv_extensions<bool>& operator=(bool b) {
        *_get_instance() = b;
        trigger_value_change_cb();
        return *this;
    }
    operator bool() const { return *_get_instance(); }
    _SCV_OSTREAM(bool)
};

#define _SCV_INTEGER_INTERFACE(type_name)                                                                              \
public:                                                                                                                \
    scv_extensions<type_name>& operator=(const scv_extensions<type_name>& i) {                                         \
        *_get_instance() = *(i._get_instance());                                                                       \
        trigger_value_change_cb();                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator=(type_name i) {                                                                \
        *_get_instance() = i;                                                                                          \
        trigger_value_change_cb();                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator+=(type_name i) {                                                               \
        *_get_instance() += i;                                                                                         \
        trigger_value_change_cb();                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator-=(type_name i) {                                                               \
        *_get_instance() -= i;                                                                                         \
        trigger_value_change_cb();                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator*=(type_name i) {                                                               \
        *_get_instance() *= i;                                                                                         \
        trigger_value_change_cb();                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator/=(type_name i) {                                                               \
        *_get_instance() /= i;                                                                                         \
        trigger_value_change_cb();                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator%=(type_name i) {                                                               \
        *_get_instance() %= i;                                                                                         \
        trigger_value_change_cb();                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator&=(type_name i) {                                                               \
        *_get_instance() &= i;                                                                                         \
        trigger_value_change_cb();                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator|=(type_name i) {                                                               \
        *_get_instance() |= i;                                                                                         \
        trigger_value_change_cb();                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator^=(type_name i) {                                                               \
        *_get_instance() ^= i;                                                                                         \
        trigger_value_change_cb();                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator<<=(type_name i) {                                                              \
        *_get_instance() <<= i;                                                                                        \
        trigger_value_change_cb();                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator>>=(type_name i) {                                                              \
        *_get_instance() >>= i;                                                                                        \
        trigger_value_change_cb();                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator++() {                                                                          \
        ++*_get_instance();                                                                                            \
        trigger_value_change_cb();                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
    type_name operator++(int) {                                                                                        \
        type_name tmp = *_get_instance();                                                                              \
        ++*_get_instance();                                                                                            \
        trigger_value_change_cb();                                                                                     \
        return tmp;                                                                                                    \
    }                                                                                                                  \
    scv_extensions<type_name>& operator--() {                                                                          \
        --*_get_instance();                                                                                            \
        trigger_value_change_cb();                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
    type_name operator--(int) {                                                                                        \
        type_name tmp = *_get_instance();                                                                              \
        --*_get_instance();                                                                                            \
        trigger_value_change_cb();                                                                                     \
        return tmp;                                                                                                    \
    }                                                                                                                  \
    operator type_name() const { return *_get_instance(); }                                                            \
    _SCV_OSTREAM(type_name)

// for all C/C++ builtin integer types
#define _SCV_TAG_FINAL_COMPONENT(type_name)                                                                            \
    template <> class scv_extensions<type_name> : public scv_extensions_base<type_name> {                              \
        _SCV_INTEGER_INTERFACE(type_name)                                                                              \
    }

_SCV_TAG_FINAL_COMPONENT(char);
_SCV_TAG_FINAL_COMPONENT(unsigned char);
_SCV_TAG_FINAL_COMPONENT(short);
_SCV_TAG_FINAL_COMPONENT(unsigned short);
_SCV_TAG_FINAL_COMPONENT(int);
_SCV_TAG_FINAL_COMPONENT(unsigned int);
_SCV_TAG_FINAL_COMPONENT(long);
_SCV_TAG_FINAL_COMPONENT(unsigned long);
_SCV_TAG_FINAL_COMPONENT(long long);
_SCV_TAG_FINAL_COMPONENT(unsigned long long);

#undef _SCV_TAG_FINAL_COMPONENT

// for all C/C++ builtin floating point type types
#define _SCV_TAG_FINAL_COMPONENT(type_name)                                                                            \
    template <> class scv_extensions<type_name> : public scv_extensions_base<type_name> {                              \
    public:                                                                                                            \
        scv_extensions<type_name>& operator=(const scv_extensions<type_name>& i) {                                     \
            *_get_instance() = *(i._get_instance());                                                                   \
            trigger_value_change_cb();                                                                                 \
            return *this;                                                                                              \
        }                                                                                                              \
        scv_extensions<type_name>& operator=(type_name i) {                                                            \
            *_get_instance() = i;                                                                                      \
            trigger_value_change_cb();                                                                                 \
            return *this;                                                                                              \
        }                                                                                                              \
        scv_extensions<type_name>& operator+=(type_name i) {                                                           \
            *_get_instance() += i;                                                                                     \
            trigger_value_change_cb();                                                                                 \
            return *this;                                                                                              \
        }                                                                                                              \
        scv_extensions<type_name>& operator-=(type_name i) {                                                           \
            *_get_instance() -= i;                                                                                     \
            trigger_value_change_cb();                                                                                 \
            return *this;                                                                                              \
        }                                                                                                              \
        scv_extensions<type_name>& operator*=(type_name i) {                                                           \
            *_get_instance() *= i;                                                                                     \
            trigger_value_change_cb();                                                                                 \
            return *this;                                                                                              \
        }                                                                                                              \
        scv_extensions<type_name>& operator/=(type_name i) {                                                           \
            *_get_instance() /= i;                                                                                     \
            trigger_value_change_cb();                                                                                 \
            return *this;                                                                                              \
        }                                                                                                              \
        operator type_name() const { return *_get_instance(); }                                                        \
        _SCV_OSTREAM(type_name)                                                                                        \
    }

_SCV_TAG_FINAL_COMPONENT(float);
_SCV_TAG_FINAL_COMPONENT(double);

template <> class scv_extensions<std::string> : public scv_extensions_base<std::string> {
public:
    scv_extensions<std::string>& operator=(const scv_extensions<std::string>& i) {
        *_get_instance() = *(i._get_instance());
        trigger_value_change_cb();
        return *this;
    }
    scv_extensions<std::string>& operator=(const std::string& s) {
        *_get_instance() = s;
        trigger_value_change_cb();
        return *this;
    }
    scv_extensions<std::string>& operator=(const char* s) {
        *_get_instance() = s;
        trigger_value_change_cb();
        return *this;
    }
    _SCV_OSTREAM(std::string);
};

#undef _SCV_TAG_FINAL_COMPONENT

#undef _SCV_INTEGER_INTERFACE

#define _SCV_INTEGER_INTERFACE(type_name)                                                                              \
public:                                                                                                                \
    scv_extensions<type_name>& operator=(const scv_extensions<type_name>& i) {                                         \
        *this->_get_instance() = *(i._get_instance());                                                                 \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator=(type_name i) {                                                                \
        *this->_get_instance() = i;                                                                                    \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator+=(type_name i) {                                                               \
        *this->_get_instance() += i;                                                                                   \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator-=(type_name i) {                                                               \
        *this->_get_instance() -= i;                                                                                   \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator*=(type_name i) {                                                               \
        *this->_get_instance() *= i;                                                                                   \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator/=(type_name i) {                                                               \
        *this->_get_instance() /= i;                                                                                   \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator%=(type_name i) {                                                               \
        *this->_get_instance() %= i;                                                                                   \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator&=(type_name i) {                                                               \
        *this->_get_instance() &= i;                                                                                   \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator|=(type_name i) {                                                               \
        *this->_get_instance() |= i;                                                                                   \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator^=(type_name i) {                                                               \
        *this->_get_instance() ^= i;                                                                                   \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator<<=(type_name i) {                                                              \
        *this->_get_instance() <<= i;                                                                                  \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator>>=(type_name i) {                                                              \
        *this->_get_instance() >>= i;                                                                                  \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    scv_extensions<type_name>& operator++() {                                                                          \
        ++*this->_get_instance();                                                                                      \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    type_name operator++(int) {                                                                                        \
        type_name tmp = *this->_get_instance();                                                                        \
        ++*this->_get_instance();                                                                                      \
        this->trigger_value_change_cb();                                                                               \
        return tmp;                                                                                                    \
    }                                                                                                                  \
    scv_extensions<type_name>& operator--() {                                                                          \
        --*this->_get_instance();                                                                                      \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    type_name operator--(int) {                                                                                        \
        type_name tmp = *this->_get_instance();                                                                        \
        --*this->_get_instance();                                                                                      \
        this->trigger_value_change_cb();                                                                               \
        return tmp;                                                                                                    \
    }                                                                                                                  \
    operator type_name() const {                                                                                       \
        const_cast<scv_extensions<type_name>*>(this)->initialize();                                                    \
        return *this->_get_instance();                                                                                 \
    }                                                                                                                  \
    _SCV_OSTREAM(type_name)

// for all SystemC templated types
#define _SCV_TAG_FINAL_COMPONENT(type_name)                                                                            \
    template <int N> class scv_extensions<type_name> : public scv_extensions_base<type_name> {                         \
        _SCV_INTEGER_INTERFACE(type_name);                                                                             \
    }

#ifdef TEST_NEST_TEMPLATE
_SCV_TAG_FINAL_COMPONENT(test_uint<N>);
#endif

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)

// _SCV_TAG_FINAL_COMPONENT(sc_fixed);
// _SCV_TAG_FINAL_COMPONENT(sc_ufixed);

#undef _SCV_TAG_FINAL_COMPONENT

#define _SCV_IMPL                                                                                                      \
    {                                                                                                                  \
        *this->_get_instance() = v;                                                                                    \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }
#define _SCV_IMPL1                                                                                                     \
    {                                                                                                                  \
        *this->_get_instance() = *(v._get_instance());                                                                 \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }
#define _SCV_IMPL2(op)                                                                                                 \
    {                                                                                                                  \
        *this->_get_instance() op v;                                                                                   \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }
#define _SCV_IMPL3(op)                                                                                                 \
    {                                                                                                                  \
        u.initialize();                                                                                                \
        v.initialize();                                                                                                \
        return *u._get_instance() op * v._get_instance();                                                              \
    }
#define _SCV_IMPL4(op)                                                                                                 \
    {                                                                                                                  \
        u.initialize();                                                                                                \
        return *u._get_instance() op v;                                                                                \
    }
#define _SCV_IMPL5(op)                                                                                                 \
    {                                                                                                                  \
        v.initialize();                                                                                                \
        return u op * v._get_instance();                                                                               \
    }
#define _SCV_MAP(return_type, method)                                                                                  \
    return_type method() const { return this->_get_instance()->method(); }

using namespace sc_dt;

#define _SCV_BASE_ASSIGN(src_type)                                                                                     \
    return_type& operator=(src_type i) {                                                                               \
        *this->_get_instance() = i;                                                                                    \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }

#define _SCV_SIGNED_SELFOP(op, src_type)                                                                               \
    return_type& operator op(src_type i) {                                                                             \
        *this->_get_instance() += i;                                                                                   \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }

#define _SCV_SIGNED_SELFOPS(op)                                                                                        \
    _SCV_SIGNED_SELFOP(op, const sc_signed&)                                                                           \
    _SCV_SIGNED_SELFOP(op, const sc_unsigned&)                                                                         \
    _SCV_SIGNED_SELFOP(op, int64)                                                                                      \
    _SCV_SIGNED_SELFOP(op, uint64)                                                                                     \
    _SCV_SIGNED_SELFOP(op, long)                                                                                       \
    _SCV_SIGNED_SELFOP(op, unsigned long)                                                                              \
    _SCV_SIGNED_SELFOP(op, int)                                                                                        \
    _SCV_SIGNED_SELFOP(op, unsigned int)                                                                               \
    _SCV_SIGNED_SELFOP(op, const sc_int_base&)                                                                         \
    _SCV_SIGNED_SELFOP(op, const sc_uint_base&)

#ifdef SC_INCLUDE_FX
#define _SCV_INT_FX_ASSIGN()                                                                                           \
    _SCV_BASE_ASSIGN(const sc_fxval&)                                                                                  \
    _SCV_BASE_ASSIGN(const sc_fxval_fast&)                                                                             \
    _SCV_BASE_ASSIGN(const sc_fxnum&)                                                                                  \
    _SCV_BASE_ASSIGN(const sc_fxnum_fast&)
#else
#define _SCV_INT_FX_ASSIGN()
#endif

#ifdef SC_DT_DEPRECATED
#define _SCV_INT_DEPRECATED(type_name)                                                                                 \
    _SCV_MAP(int, to_signed)                                                                                           \
    _SCV_MAP(unsigned, to_unsigned)
#else
#define _SCV_INT_DEPRECATED(type_name)
#endif

#define _SCV_SIGNED_INTERFACE(type_name)                                                                               \
public:                                                                                                                \
    operator const type_name&() const { return *this->_get_instance(); }                                               \
    typedef scv_extensions<type_name> return_type;                                                                     \
    return_type& operator=(const return_type& i) {                                                                     \
        *this->_get_instance() = *(i._get_instance());                                                                 \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    _SCV_BASE_ASSIGN(const sc_dt::sc_signed&)                                                                          \
    _SCV_BASE_ASSIGN(const sc_dt::sc_signed_subref&)                                                                   \
    _SCV_BASE_ASSIGN(const sc_dt::sc_unsigned&)                                                                        \
    _SCV_BASE_ASSIGN(const sc_dt::sc_unsigned_subref&)                                                                 \
    _SCV_BASE_ASSIGN(const char*)                                                                                      \
    _SCV_BASE_ASSIGN(int64)                                                                                            \
    _SCV_BASE_ASSIGN(uint64)                                                                                           \
    _SCV_BASE_ASSIGN(long)                                                                                             \
    _SCV_BASE_ASSIGN(unsigned long)                                                                                    \
    _SCV_BASE_ASSIGN(int)                                                                                              \
    _SCV_BASE_ASSIGN(unsigned int)                                                                                     \
    _SCV_BASE_ASSIGN(double)                                                                                           \
    _SCV_BASE_ASSIGN(const sc_dt::sc_int_base&)                                                                        \
    _SCV_BASE_ASSIGN(const sc_dt::sc_uint_base&)                                                                       \
    _SCV_BASE_ASSIGN(const sc_dt::sc_bv_base&)                                                                         \
    _SCV_BASE_ASSIGN(const sc_dt::sc_lv_base&)                                                                         \
    _SCV_INT_FX_ASSIGN()                                                                                               \
    return_type& operator++() {                                                                                        \
        ++*this->_get_instance();                                                                                      \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    type_name operator++(int) {                                                                                        \
        type_name tmp = *this->_get_instance();                                                                        \
        ++*this->_get_instance();                                                                                      \
        this->trigger_value_change_cb();                                                                               \
        return tmp;                                                                                                    \
    }                                                                                                                  \
    return_type& operator--() {                                                                                        \
        --*this->_get_instance();                                                                                      \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    type_name operator--(int) {                                                                                        \
        type_name tmp = *this->_get_instance();                                                                        \
        --*this->_get_instance();                                                                                      \
        this->trigger_value_change_cb();                                                                               \
        return tmp;                                                                                                    \
    }                                                                                                                  \
    _SCV_MAP(int, to_int)                                                                                              \
    _SCV_MAP(unsigned int, to_uint)                                                                                    \
    _SCV_MAP(long, to_long)                                                                                            \
    _SCV_MAP(unsigned long, to_ulong)                                                                                  \
    _SCV_MAP(int64, to_int64)                                                                                          \
    _SCV_MAP(uint64, to_uint64)                                                                                        \
    _SCV_MAP(double, to_double)                                                                                        \
    _SCV_INT_DEPRECATED(type_name)                                                                                     \
    const std::string to_string(sc_numrep numrep = SC_DEC) const { return this->_get_instance()->to_string(numrep); }  \
    const std::string to_string(sc_numrep numrep, bool w_prefix) const {                                               \
        return this->_get_instance()->to_string(numrep, w_prefix);                                                     \
    }                                                                                                                  \
    void scan(std::istream& is = std::cin) {                                                                           \
        this->_get_instance()->scan(is);                                                                               \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    void dump(std::ostream& os = std::cout) const { this->_get_instance()->dump(os); }                                 \
    _SCV_MAP(int, length)                                                                                              \
    _SCV_MAP(bool, iszero)                                                                                             \
    _SCV_MAP(bool, sign)                                                                                               \
    bool test(int i) const { return this->_get_instance()->test(i); }                                                  \
    void set(int i) {                                                                                                  \
        this->_get_instance()->set(i);                                                                                 \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    void clear(int i) {                                                                                                \
        this->_get_instance()->clear(i);                                                                               \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    void set(int i, bool v) {                                                                                          \
        this->_get_instance()->set(i, v);                                                                              \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    void invert(int i) {                                                                                               \
        this->_get_instance()->invert(i);                                                                              \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    void reverse() {                                                                                                   \
        this->_get_instance()->reverse();                                                                              \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    void get_packed_rep(sc_dt::sc_digit* buf) const { this->_get_instance()->get_packed_rep(buf); }                    \
    void set_packed_rep(sc_dt::sc_digit* buf) {                                                                        \
        this->_get_instance()->set_packed_rep(buf);                                                                    \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    _SCV_SIGNED_SELFOPS(+=)                                                                                            \
    _SCV_SIGNED_SELFOPS(-=)                                                                                            \
    _SCV_SIGNED_SELFOPS(*=)                                                                                            \
    _SCV_SIGNED_SELFOPS(/=)                                                                                            \
    _SCV_SIGNED_SELFOPS(%=)                                                                                            \
    _SCV_SIGNED_SELFOPS(&=)                                                                                            \
    _SCV_SIGNED_SELFOPS(|=)                                                                                            \
    _SCV_SIGNED_SELFOPS(^=)                                                                                            \
    _SCV_SIGNED_SELFOPS(<<=)                                                                                           \
    _SCV_SIGNED_SELFOPS(>>=)                                                                                           \
    _SCV_OSTREAM(type_name)

#define _SCV_TAG_FINAL_COMPONENT(type_name)                                                                            \
    template <> class scv_extensions<type_name> : public scv_extensions_base<type_name> {                              \
        _SCV_SIGNED_INTERFACE(type_name)                                                                               \
    }

_SCV_TAG_FINAL_COMPONENT(sc_dt::sc_signed);
_SCV_TAG_FINAL_COMPONENT(sc_dt::sc_unsigned);

#undef _SCV_TAG_FINAL_COMPONENT

#define _SCV_INT_BASE_SELFOPS(op)                                                                                      \
    _SCV_SIGNED_SELFOP(op, int64)                                                                                      \
    _SCV_SIGNED_SELFOP(op, uint64)                                                                                     \
    _SCV_SIGNED_SELFOP(op, long)                                                                                       \
    _SCV_SIGNED_SELFOP(op, unsigned long)                                                                              \
    _SCV_SIGNED_SELFOP(op, int)                                                                                        \
    _SCV_SIGNED_SELFOP(op, unsigned int)                                                                               \
    _SCV_SIGNED_SELFOP(op, const sc_int_base&)                                                                         \
    _SCV_SIGNED_SELFOP(op, const sc_uint_base&)

#define _SCV_INT_BASE_INTERFACE(type_name)                                                                             \
public:                                                                                                                \
    operator const type_name&() const { return *this->_get_instance(); }                                               \
    typedef scv_extensions<type_name> return_type;                                                                     \
    void invalid_length() const { this->invalid_length(); }                                                            \
    void invalid_index(int i) const { this->invalid_index(i); }                                                        \
    void invalid_range(int l, int r) const { this->invalid_range(l, r); }                                              \
    void check_length() const { this->check_length(); }                                                                \
    void check_index(int i) const { this->check_index(i); }                                                            \
    void check_range(int l, int r) const { this->check_range(l, r); }                                                  \
    void extend_sign() { this->extend_sign(); }                                                                        \
    return_type& operator=(const return_type& i) {                                                                     \
        *this->_get_instance() = *(i._get_instance());                                                                 \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    _SCV_BASE_ASSIGN(int_type)                                                                                         \
    _SCV_BASE_ASSIGN(const sc_int_base&)                                                                               \
    _SCV_BASE_ASSIGN(const sc_int_subref&)                                                                             \
    _SCV_BASE_ASSIGN(const sc_signed&)                                                                                 \
    _SCV_BASE_ASSIGN(const sc_unsigned&)                                                                               \
    _SCV_INT_FX_ASSIGN()                                                                                               \
    _SCV_BASE_ASSIGN(const sc_bv_base&)                                                                                \
    _SCV_BASE_ASSIGN(const sc_lv_base&)                                                                                \
    _SCV_BASE_ASSIGN(const char*)                                                                                      \
    _SCV_BASE_ASSIGN(unsigned long)                                                                                    \
    _SCV_BASE_ASSIGN(long)                                                                                             \
    _SCV_BASE_ASSIGN(unsigned int)                                                                                     \
    _SCV_BASE_ASSIGN(int)                                                                                              \
    _SCV_BASE_ASSIGN(uint64)                                                                                           \
    _SCV_BASE_ASSIGN(double)                                                                                           \
    _SCV_SIGNED_SELFOP(+=, int_type)                                                                                   \
    _SCV_SIGNED_SELFOP(-=, int_type)                                                                                   \
    _SCV_SIGNED_SELFOP(*=, int_type)                                                                                   \
    _SCV_SIGNED_SELFOP(/=, int_type)                                                                                   \
    _SCV_SIGNED_SELFOP(%=, int_type)                                                                                   \
    _SCV_SIGNED_SELFOP(&=, int_type)                                                                                   \
    _SCV_SIGNED_SELFOP(|=, int_type)                                                                                   \
    _SCV_SIGNED_SELFOP(^=, int_type)                                                                                   \
    _SCV_SIGNED_SELFOP(<<=, int_type)                                                                                  \
    _SCV_SIGNED_SELFOP(>>=, int_type)                                                                                  \
    return_type& operator++() {                                                                                        \
        ++*this->_get_instance();                                                                                      \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    type_name operator++(int) {                                                                                        \
        type_name tmp = *this->_get_instance();                                                                        \
        ++*this->_get_instance();                                                                                      \
        this->trigger_value_change_cb();                                                                               \
        return tmp;                                                                                                    \
    }                                                                                                                  \
    return_type& operator--() {                                                                                        \
        --*this->_get_instance();                                                                                      \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    type_name operator--(int) {                                                                                        \
        type_name tmp = *this->_get_instance();                                                                        \
        --*this->_get_instance();                                                                                      \
        this->trigger_value_change_cb();                                                                               \
        return tmp;                                                                                                    \
    }                                                                                                                  \
    bool test(int i) const { return this->_get_instance()->test(i); }                                                  \
    void set(int i) {                                                                                                  \
        this->_get_instance()->set(i);                                                                                 \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    void set(int i, bool v) {                                                                                          \
        this->_get_instance()->set(i, v);                                                                              \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    _SCV_MAP(int, length)                                                                                              \
    _SCV_MAP(bool, and_reduce)                                                                                         \
    _SCV_MAP(bool, nand_reduce)                                                                                        \
    _SCV_MAP(bool, or_reduce)                                                                                          \
    _SCV_MAP(bool, nor_reduce)                                                                                         \
    _SCV_MAP(bool, xor_reduce)                                                                                         \
    _SCV_MAP(bool, xnor_reduce)                                                                                        \
    operator int_type() const { return this->value(); }                                                                \
    _SCV_MAP(int_type, value)                                                                                          \
    _SCV_MAP(int, to_int)                                                                                              \
    _SCV_MAP(unsigned int, to_uint)                                                                                    \
    _SCV_MAP(long, to_long)                                                                                            \
    _SCV_MAP(unsigned long, to_ulong)                                                                                  \
    _SCV_MAP(int64, to_int64)                                                                                          \
    _SCV_MAP(uint64, to_uint64)                                                                                        \
    _SCV_MAP(double, to_double)                                                                                        \
    const std::string to_string(sc_numrep numrep = SC_DEC) const { return this->_get_instance()->to_string(numrep); }  \
    const std::string to_string(sc_numrep numrep, bool w_prefix) const {                                               \
        return this->_get_instance()->to_string(numrep, w_prefix);                                                     \
    }                                                                                                                  \
    void scan(std::istream& is = std::cin) {                                                                           \
        this->_get_instance()->scan(is);                                                                               \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    _SCV_OSTREAM(type_name)

#define _SCV_TAG_FINAL_COMPONENT(type_name)                                                                            \
    template <> class scv_extensions<type_name> : public scv_extensions_base<type_name> {                              \
        _SCV_INT_BASE_INTERFACE(type_name)                                                                             \
    }

_SCV_TAG_FINAL_COMPONENT(sc_dt::sc_int_base);
_SCV_TAG_FINAL_COMPONENT(sc_dt::sc_uint_base);

#undef _SCV_TAG_FINAL_COMPONENT

#define _SCV_BIT_BASE_INTERFACE(type_name)                                                                             \
public:                                                                                                                \
    operator const type_name&() const { return *this->_get_instance(); }                                               \
    typedef scv_extensions<type_name> return_type;                                                                     \
    return_type& operator=(const return_type& i) {                                                                     \
        *this->_get_instance() = *(i._get_instance());                                                                 \
        this->trigger_value_change_cb();                                                                               \
        return *this;                                                                                                  \
    }                                                                                                                  \
    _SCV_BASE_ASSIGN(const type_name&)                                                                                 \
    template <class X>                                                                                                 \
    _SCV_BASE_ASSIGN(const sc_proxy<X>&)                                                                               \
    _SCV_BASE_ASSIGN(const char*) _SCV_BASE_ASSIGN(const bool*) _SCV_BASE_ASSIGN(const sc_logic*)                      \
        _SCV_BASE_ASSIGN(const sc_unsigned&) _SCV_BASE_ASSIGN(const sc_signed&) _SCV_BASE_ASSIGN(const sc_uint_base&)  \
            _SCV_BASE_ASSIGN(const sc_int_base&) _SCV_BASE_ASSIGN(unsigned long) _SCV_BASE_ASSIGN(long)                \
                _SCV_BASE_ASSIGN(unsigned int) _SCV_BASE_ASSIGN(int) _SCV_BASE_ASSIGN(uint64) _SCV_BASE_ASSIGN(int64)  \
                    _SCV_MAP(int, length) _SCV_MAP(int, size) sc_logic_value_t                                         \
                    get_bit(int i) const {                                                                             \
        return sc_dt::sc_logic_value_t(this->_get_instance()->get_bit(i));                                             \
    }                                                                                                                  \
    void set_bit(int i, sc_logic_value_t v) {                                                                          \
        this->_get_instance()->set_bit(i, v);                                                                          \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    unsigned long get_word(int i) const { return this->_get_instance()->get_word(i); }                                 \
    void set_word(int i, unsigned long w) {                                                                            \
        this->_get_instance()->set_word(i, w);                                                                         \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    unsigned long get_cword(int i) const { return this->_get_instance()->get_cword(i); }                               \
    void set_cword(int i, unsigned long w) {                                                                           \
        this->_get_instance()->set_cword(i, w);                                                                        \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    void clean_tail() {                                                                                                \
        this->_get_instance()->clean_tail();                                                                           \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    _SCV_MAP(bool, is_01)                                                                                              \
    _SCV_OSTREAM(type_name)

#define _SCV_TAG_FINAL_COMPONENT(type_name)                                                                            \
    template <> class scv_extensions<type_name> : public scv_extensions_base<type_name> {                              \
        _SCV_BIT_BASE_INTERFACE(type_name)                                                                             \
    }

_SCV_TAG_FINAL_COMPONENT(sc_lv_base);
_SCV_TAG_FINAL_COMPONENT(sc_bv_base);

#undef _SCV_TAG_FINAL_COMPONENT

// sc_uint and sc_int are exactly the same as
template <int W> class scv_extensions<sc_uint<W>> : public scv_extensions_base<sc_uint<W>> {
public:
    typedef scv_extensions<sc_uint<W>> return_type;

    return_type& operator=(const return_type& v) _SCV_IMPL1
        // from class sc_uint
        return_type&
        operator=(const sc_uint_base& v) _SCV_IMPL return_type&
        operator=(const sc_signed& v) _SCV_IMPL return_type&
        operator=(const sc_unsigned& v) _SCV_IMPL
#ifdef SC_INCLUDE_FX
        return_type&
        operator=(const sc_fxval& v) _SCV_IMPL return_type&
        operator=(const sc_fxval_fast& v) _SCV_IMPL return_type&
        operator=(const sc_fxnum& v) _SCV_IMPL return_type&
        operator=(const sc_fxnum_fast& v) _SCV_IMPL
#endif
        return_type&
        operator=(const sc_bv_base& v) _SCV_IMPL return_type&
        operator=(const sc_lv_base& v) _SCV_IMPL

        return_type&
        operator++() // prefix
    {
        ++*this->_get_instance();
        this->trigger_value_change_cb();
        return *this;
    }
    const return_type operator++(int) // postfix
    {
        sc_uint<W> tmp = *this->_get_instance()++;
        this->trigger_value_change_cb();
        return tmp;
    }
    return_type& operator--() // prefix
    {
        --*this->_get_instance();
        this->trigger_value_change_cb();
        return *this;
    }
    const return_type operator--(int) // postfix
    {
        sc_uint<W> tmp = *this->_get_instance()--;
        this->trigger_value_change_cb();
        return tmp;
    }

    // from class sc_uint_base

    _SCV_MAP(int, bitwidth);
    _SCV_MAP(int, length);
    _SCV_MAP(unsigned int, to_uint);
    _SCV_MAP(int, to_int);
    _SCV_MAP(uint64, to_uint64);
    _SCV_MAP(int64, to_int64);
#ifndef _32BIT_
    _SCV_MAP(long, long_low);
    _SCV_MAP(long, long_high);
#endif
    bool test(int i) const { return this->_get_instance()->test(i); }
    void set(int i) {
        this->_get_instance()->set(i);
        this->trigger_value_change_cb();
    }
    void set(int i, bool v) {
        this->_get_instance()->set(i, v);
        this->trigger_value_change_cb();
    }
    // sc_uint_bitref operator [] (int i)
    bool operator[](int i) const { return this->_get_instance()->operator[](i); }
    //  sc_uint_subref range(int left, int right);

    // operator ==, !=, <, <=, >, >= should be handled by uint_type();
    // operator +, -, etc. as well.

    //  void print( ostream& os ) const {  this->_get_instance()->print(os); }
};

template <int W> class scv_extensions<sc_int<W>> : public scv_extensions_base<sc_int<W>> {
public:
    typedef scv_extensions<sc_int<W>> return_type;

    return_type& operator=(const return_type& v) _SCV_IMPL1
        // from class sc_int
        return_type&
        operator=(const sc_int_base& v) _SCV_IMPL return_type&
        operator=(const sc_signed& v) _SCV_IMPL return_type&
        operator=(const sc_unsigned& v) _SCV_IMPL
#ifdef SC_INCLUDE_FX
        return_type&
        operator=(const sc_fxval& v) _SCV_IMPL return_type&
        operator=(const sc_fxval_fast& v) _SCV_IMPL return_type&
        operator=(const sc_fxnum& v) _SCV_IMPL return_type&
        operator=(const sc_fxnum_fast& v) _SCV_IMPL
#endif
        return_type&
        operator=(const sc_bv_base& v) _SCV_IMPL return_type&
        operator=(const sc_lv_base& v) _SCV_IMPL

        return_type&
        operator++() // prefix
    {
        ++*this->_get_instance();
        this->trigger_value_change_cb();
        return *this;
    }
    const return_type operator++(int) // postfix
    {
        sc_int<W> tmp = *this->_get_instance()++;
        this->trigger_value_change_cb();
        return tmp;
    }
    return_type& operator--() // prefix
    {
        --*this->_get_instance();
        this->trigger_value_change_cb();
        return *this;
    }
    const return_type operator--(int) // postfix
    {
        sc_int<W> tmp = *this->_get_instance()--;
        this->trigger_value_change_cb();
        return tmp;
    }

    // from class sc_int_base

    _SCV_MAP(int, bitwidth);
    _SCV_MAP(int, length);
    _SCV_MAP(unsigned int, to_uint);
    _SCV_MAP(int, to_int);
    _SCV_MAP(uint64, to_uint64);
    _SCV_MAP(int64, to_int64);
#ifndef _32BIT_
    _SCV_MAP(long, long_low);
    _SCV_MAP(long, long_high);
#endif
    bool test(int i) const { return this->_get_instance()->test(i); }
    void set(int i) {
        this->_get_instance()->set(i);
        this->trigger_value_change_cb();
    }
    void set(int i, bool v) {
        this->_get_instance()->set(i, v);
        this->trigger_value_change_cb();
    }
    // sc_int_bitref operator [] (int i)
    bool operator[](int i) const { return this->_get_instance()->operator[](i); }
    //  sc_int_subref range(int left, int right);

    // operator ==, !=, <, <=, >, >= should be handled by int_type();
    // operator +, -, etc. as well.

    //  void print( ostream& os ) const {  this->_get_instance()->print(os); }
};

// sc_biguint and sc_bigint are exactly the same.
// need to add &=, etc.
template <int W> class scv_extensions<sc_biguint<W>> : public scv_extensions_base<sc_biguint<W>> {
public:
    typedef scv_extensions<sc_biguint<W>> return_type;

    return_type& operator=(const return_type& v) _SCV_IMPL1 return_type&
    operator=(const sc_biguint<W>& v) _SCV_IMPL return_type&
    operator=(const sc_unsigned& v) _SCV_IMPL return_type&
    operator=(const sc_signed& v) _SCV_IMPL return_type&
    operator=(const char* v) _SCV_IMPL return_type&
    operator=(int64 v) _SCV_IMPL return_type&
    operator=(uint64 v) _SCV_IMPL return_type&
    operator=(long v) _SCV_IMPL return_type&
    operator=(unsigned long v) _SCV_IMPL return_type&
    operator=(int v) _SCV_IMPL return_type&
    operator=(unsigned int v) _SCV_IMPL return_type&
    operator=(double v) _SCV_IMPL return_type&
    operator=(const sc_bv_base& v) _SCV_IMPL return_type&
    operator=(const sc_lv_base& v) _SCV_IMPL return_type&
    operator=(const sc_int_base& v) _SCV_IMPL return_type&
    operator=(const sc_uint_base& v) _SCV_IMPL
#ifdef SC_INCLUDE_FX
        return_type&
        operator=(const sc_fxval& v) _SCV_IMPL return_type&
        operator=(const sc_fxval_fast& v) _SCV_IMPL return_type&
        operator=(const sc_fxnum& v) _SCV_IMPL return_type&
        operator=(const sc_fxnum_fast& v) _SCV_IMPL
#endif
        return_type&
        operator+=(const sc_signed& v) _SCV_IMPL2(+=) return_type&
        operator+=(const sc_unsigned& v) _SCV_IMPL2(+=) return_type&
        operator+=(int64 v) _SCV_IMPL2(+=) return_type&
        operator+=(uint64 v) _SCV_IMPL2(+=) return_type&
        operator+=(long v) _SCV_IMPL2(+=) return_type&
        operator+=(unsigned long v) _SCV_IMPL2(+=) return_type&
        operator+=(int v) _SCV_IMPL2(+=) return_type&
        operator+=(unsigned int v) _SCV_IMPL2(+=) return_type&
        operator+=(const sc_int_base& v) _SCV_IMPL2(+=) return_type&
        operator+=(const sc_uint_base& v) _SCV_IMPL2(+=)

            return_type&
            operator-=(const sc_signed& v) _SCV_IMPL2(-=) return_type&
            operator-=(const sc_unsigned& v) _SCV_IMPL2(-=) return_type&
            operator-=(int64 v) _SCV_IMPL2(-=) return_type&
            operator-=(uint64 v) _SCV_IMPL2(-=) return_type&
            operator-=(long v) _SCV_IMPL2(-=) return_type&
            operator-=(unsigned long v) _SCV_IMPL2(-=) return_type&
            operator-=(int v) _SCV_IMPL2(-=) return_type&
            operator-=(unsigned int v) _SCV_IMPL2(-=) return_type&
            operator-=(const sc_int_base& v) _SCV_IMPL2(-=) return_type&
            operator-=(const sc_uint_base& v) _SCV_IMPL2(-=)

                return_type&
                operator*=(const sc_signed& v) _SCV_IMPL2(*=) return_type&
                operator*=(const sc_unsigned& v) _SCV_IMPL2(*=) return_type&
                operator*=(int64 v) _SCV_IMPL2(*=) return_type&
                operator*=(uint64 v) _SCV_IMPL2(*=) return_type&
                operator*=(long v) _SCV_IMPL2(*=) return_type&
                operator*=(unsigned long v) _SCV_IMPL2(*=) return_type&
                operator*=(int v) _SCV_IMPL2(*=) return_type&
                operator*=(unsigned int v) _SCV_IMPL2(*=) return_type&
                operator*=(const sc_int_base& v) _SCV_IMPL2(*=) return_type&
                operator*=(const sc_uint_base& v) _SCV_IMPL2(*=)

                    return_type&
                    operator/=(const sc_signed& v) _SCV_IMPL2(/=) return_type&
                    operator/=(const sc_unsigned& v) _SCV_IMPL2(/=) return_type&
                    operator/=(int64 v) _SCV_IMPL2(/=) return_type&
                    operator/=(uint64 v) _SCV_IMPL2(/=) return_type&
                    operator/=(long v) _SCV_IMPL2(/=) return_type&
                    operator/=(unsigned long v) _SCV_IMPL2(/=) return_type&
                    operator/=(int v) _SCV_IMPL2(/=) return_type&
                    operator/=(unsigned int v) _SCV_IMPL2(/=) return_type&
                    operator/=(const sc_int_base& v) _SCV_IMPL2(/=) return_type&
                    operator/=(const sc_uint_base& v) _SCV_IMPL2(/=)

                        return_type&
                        operator%=(const sc_signed& v) _SCV_IMPL2(%=) return_type&
                        operator%=(const sc_unsigned& v) _SCV_IMPL2(%=) return_type&
                        operator%=(int64 v) _SCV_IMPL2(%=) return_type&
                        operator%=(uint64 v) _SCV_IMPL2(%=) return_type&
                        operator%=(long v) _SCV_IMPL2(%=) return_type&
                        operator%=(unsigned long v) _SCV_IMPL2(%=) return_type&
                        operator%=(int v) _SCV_IMPL2(%=) return_type&
                        operator%=(unsigned int v) _SCV_IMPL2(%=) return_type&
                        operator%=(const sc_int_base& v) _SCV_IMPL2(%=) return_type&
                        operator%=(const sc_uint_base& v) _SCV_IMPL2(%=)

                            return_type&
                            operator&=(const sc_signed& v) _SCV_IMPL2(&=) return_type&
                            operator&=(const sc_unsigned& v) _SCV_IMPL2(&=) return_type&
                            operator&=(int64 v) _SCV_IMPL2(&=) return_type&
                            operator&=(uint64 v) _SCV_IMPL2(&=) return_type&
                            operator&=(long v) _SCV_IMPL2(&=) return_type&
                            operator&=(unsigned long v) _SCV_IMPL2(&=) return_type&
                            operator&=(int v) _SCV_IMPL2(&=) return_type&
                            operator&=(unsigned int v) _SCV_IMPL2(&=) return_type&
                            operator&=(const sc_int_base& v) _SCV_IMPL2(&=) return_type&
                            operator&=(const sc_uint_base& v) _SCV_IMPL2(&=)

                                return_type&
                                operator|=(const sc_signed& v) _SCV_IMPL2(|=) return_type&
                                operator|=(const sc_unsigned& v) _SCV_IMPL2(|=) return_type&
                                operator|=(int64 v) _SCV_IMPL2(|=) return_type&
                                operator|=(uint64 v) _SCV_IMPL2(|=) return_type&
                                operator|=(long v) _SCV_IMPL2(|=) return_type&
                                operator|=(unsigned long v) _SCV_IMPL2(|=) return_type&
                                operator|=(int v) _SCV_IMPL2(|=) return_type&
                                operator|=(unsigned int v) _SCV_IMPL2(|=) return_type&
                                operator|=(const sc_int_base& v) _SCV_IMPL2(|=) return_type&
                                operator|=(const sc_uint_base& v) _SCV_IMPL2(|=)

                                    return_type&
                                    operator^=(const sc_signed& v) _SCV_IMPL2(^=) return_type&
                                    operator^=(const sc_unsigned& v) _SCV_IMPL2(^=) return_type&
                                    operator^=(int64 v) _SCV_IMPL2(^=) return_type&
                                    operator^=(uint64 v) _SCV_IMPL2(^=) return_type&
                                    operator^=(long v) _SCV_IMPL2(^=) return_type&
                                    operator^=(unsigned long v) _SCV_IMPL2(^=) return_type&
                                    operator^=(int v) _SCV_IMPL2(^=) return_type&
                                    operator^=(unsigned int v) _SCV_IMPL2(^=) return_type&
                                    operator^=(const sc_int_base& v) _SCV_IMPL2(^=) return_type&
                                    operator^=(const sc_uint_base& v) _SCV_IMPL2(^=)

                                        return_type&
                                        operator<<=(const sc_signed& v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(const sc_unsigned& v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(int64 v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(uint64 v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(long v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(unsigned long v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(int v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(unsigned int v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(const sc_int_base& v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(const sc_uint_base& v) _SCV_IMPL2(<<=)

                                            return_type&
                                            operator>>=(const sc_signed& v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(const sc_unsigned& v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(int64 v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(uint64 v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(long v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(unsigned long v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(int v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(unsigned int v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(const sc_int_base& v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(const sc_uint_base& v) _SCV_IMPL2(>>=)

                                                return_type&
                                                operator++() {
        ++*this->_get_instance();
        this->trigger_value_change_cb();
        return *this;
    }
    const sc_unsigned operator++(int) {
        sc_biguint<W> tmp = *this->_get_instance()++;
        this->trigger_value_change_cb();
        return tmp;
    }
    return_type& operator--() {
        --*this->_get_instance();
        this->trigger_value_change_cb();
        return *this;
    }
    const sc_unsigned operator--(int) {
        sc_biguint<W> tmp = *this->_get_instance()--;
        this->trigger_value_change_cb();
        return tmp;
    }
    //  sc_unsigned_bitref operator [] (int i)
    const bool operator[](int i) const { return this->_get_instance()->operator[](i); }
    const sc_unsigned range(int i, int j) const { return this->_get_instance()->range(i, j); }
    //  sc_unsigned_subref operator () (int i, int j)
    const sc_unsigned operator()(int i, int j) const { return this->_get_instance()->operator()(i, j); }

    std::string to_string(sc_numrep base = SC_DEC, bool formatted = false) const {
        return this->_get_instance()->to_string(base, formatted);
    }
    std::string to_string(int base, bool formatted = false) const {
        return this->_get_instance()->to_string(base, formatted);
    }

    _SCV_MAP(int64, to_int64);
    _SCV_MAP(uint64, to_uint64);
    _SCV_MAP(long, to_long);
    _SCV_MAP(unsigned long, to_ulong);
    _SCV_MAP(unsigned long, to_unsigned_long);
    _SCV_MAP(int, to_int);
    _SCV_MAP(int, to_signed);
    _SCV_MAP(unsigned int, to_uint);
    _SCV_MAP(unsigned int, to_unsigned);
    _SCV_MAP(unsigned int, to_unsigned_int);
    _SCV_MAP(double, to_double);
    //  void print() const {  this->_get_instance()->print(); }
    //  void print(ostream &os) const {  this->_get_instance()->print(os); }
    void dump() const { this->_get_instance()->dump(); };
    void dump(std::ostream& os) const { this->_get_instance()->dump(os); };
    _SCV_MAP(int, length);
    _SCV_MAP(bool, iszero);
    _SCV_MAP(bool, sign);
    bool test(int i) const { return this->_get_instance()->test(i); }
    void set(int i) {
        this->_get_instance()->set(i);
        this->trigger_value_change_cb();
    }
    void clear(int i) {
        this->_get_instance()->clear(i);
        this->trigger_value_change_cb();
    }
    void set(int i, bool v) {
        this->_get_instance()->set(i, v);
        this->trigger_value_change_cb();
    }
    void invert(int i) {
        this->_get_instance()->invert(i);
        this->trigger_value_change_cb();
    }
    void reverse() {
        this->_get_instance()->reverse();
        this->trigger_value_change_cb();
    }
    void get_packed_rep(sc_dt::sc_digit* buf) const { this->_get_instance()->get_packet_ref(buf); }
    void set_packed_rep(sc_dt::sc_digit* buf) {
        this->_get_instance()->get_packet_ref(buf);
        this->trigger_value_change_cb();
    }

    operator const sc_unsigned&() const { return *this->_get_instance(); }
};

template <int W> class scv_extensions<sc_bigint<W>> : public scv_extensions_base<sc_bigint<W>> {
public:
    typedef scv_extensions<sc_bigint<W>> return_type;

    return_type& operator=(const return_type& v) _SCV_IMPL1 return_type&
    operator=(const sc_bigint<W>& v) _SCV_IMPL return_type&
    operator=(const sc_unsigned& v) _SCV_IMPL return_type&
    operator=(const sc_signed& v) _SCV_IMPL return_type&
    operator=(const char* v) _SCV_IMPL return_type&
    operator=(int64 v) _SCV_IMPL return_type&
    operator=(uint64 v) _SCV_IMPL return_type&
    operator=(long v) _SCV_IMPL return_type&
    operator=(unsigned long v) _SCV_IMPL return_type&
    operator=(int v) _SCV_IMPL return_type&
    operator=(unsigned int v) _SCV_IMPL return_type&
    operator=(double v) _SCV_IMPL return_type&
    operator=(const sc_bv_base& v) _SCV_IMPL return_type&
    operator=(const sc_lv_base& v) _SCV_IMPL return_type&
    operator=(const sc_int_base& v) _SCV_IMPL return_type&
    operator=(const sc_uint_base& v) _SCV_IMPL
#ifdef SC_INCLUDE_FX
        return_type&
        operator=(const sc_fxval& v) _SCV_IMPL return_type&
        operator=(const sc_fxval_fast& v) _SCV_IMPL return_type&
        operator=(const sc_fxnum& v) _SCV_IMPL return_type&
        operator=(const sc_fxnum_fast& v) _SCV_IMPL
#endif

        return_type&
        operator+=(const sc_signed& v) _SCV_IMPL2(+=) return_type&
        operator+=(const sc_unsigned& v) _SCV_IMPL2(+=) return_type&
        operator+=(int64 v) _SCV_IMPL2(+=) return_type&
        operator+=(uint64 v) _SCV_IMPL2(+=) return_type&
        operator+=(long v) _SCV_IMPL2(+=) return_type&
        operator+=(unsigned long v) _SCV_IMPL2(+=) return_type&
        operator+=(int v) _SCV_IMPL2(+=) return_type&
        operator+=(unsigned int v) _SCV_IMPL2(+=) return_type&
        operator+=(const sc_int_base& v) _SCV_IMPL2(+=) return_type&
        operator+=(const sc_uint_base& v) _SCV_IMPL2(+=)

            return_type&
            operator-=(const sc_signed& v) _SCV_IMPL2(-=) return_type&
            operator-=(const sc_unsigned& v) _SCV_IMPL2(-=) return_type&
            operator-=(int64 v) _SCV_IMPL2(-=) return_type&
            operator-=(uint64 v) _SCV_IMPL2(-=) return_type&
            operator-=(long v) _SCV_IMPL2(-=) return_type&
            operator-=(unsigned long v) _SCV_IMPL2(-=) return_type&
            operator-=(int v) _SCV_IMPL2(-=) return_type&
            operator-=(unsigned int v) _SCV_IMPL2(-=) return_type&
            operator-=(const sc_int_base& v) _SCV_IMPL2(-=) return_type&
            operator-=(const sc_uint_base& v) _SCV_IMPL2(-=)

                return_type&
                operator*=(const sc_signed& v) _SCV_IMPL2(*=) return_type&
                operator*=(const sc_unsigned& v) _SCV_IMPL2(*=) return_type&
                operator*=(int64 v) _SCV_IMPL2(*=) return_type&
                operator*=(uint64 v) _SCV_IMPL2(*=) return_type&
                operator*=(long v) _SCV_IMPL2(*=) return_type&
                operator*=(unsigned long v) _SCV_IMPL2(*=) return_type&
                operator*=(int v) _SCV_IMPL2(*=) return_type&
                operator*=(unsigned int v) _SCV_IMPL2(*=) return_type&
                operator*=(const sc_int_base& v) _SCV_IMPL2(*=) return_type&
                operator*=(const sc_uint_base& v) _SCV_IMPL2(*=)

                    return_type&
                    operator/=(const sc_signed& v) _SCV_IMPL2(/=) return_type&
                    operator/=(const sc_unsigned& v) _SCV_IMPL2(/=) return_type&
                    operator/=(int64 v) _SCV_IMPL2(/=) return_type&
                    operator/=(uint64 v) _SCV_IMPL2(/=) return_type&
                    operator/=(long v) _SCV_IMPL2(/=) return_type&
                    operator/=(unsigned long v) _SCV_IMPL2(/=) return_type&
                    operator/=(int v) _SCV_IMPL2(/=) return_type&
                    operator/=(unsigned int v) _SCV_IMPL2(/=) return_type&
                    operator/=(const sc_int_base& v) _SCV_IMPL2(/=) return_type&
                    operator/=(const sc_uint_base& v) _SCV_IMPL2(/=)

                        return_type&
                        operator%=(const sc_signed& v) _SCV_IMPL2(%=) return_type&
                        operator%=(const sc_unsigned& v) _SCV_IMPL2(%=) return_type&
                        operator%=(int64 v) _SCV_IMPL2(%=) return_type&
                        operator%=(uint64 v) _SCV_IMPL2(%=) return_type&
                        operator%=(long v) _SCV_IMPL2(%=) return_type&
                        operator%=(unsigned long v) _SCV_IMPL2(%=) return_type&
                        operator%=(int v) _SCV_IMPL2(%=) return_type&
                        operator%=(unsigned int v) _SCV_IMPL2(%=) return_type&
                        operator%=(const sc_int_base& v) _SCV_IMPL2(%=) return_type&
                        operator%=(const sc_uint_base& v) _SCV_IMPL2(%=)

                            return_type&
                            operator&=(const sc_signed& v) _SCV_IMPL2(&=) return_type&
                            operator&=(const sc_unsigned& v) _SCV_IMPL2(&=) return_type&
                            operator&=(int64 v) _SCV_IMPL2(&=) return_type&
                            operator&=(uint64 v) _SCV_IMPL2(&=) return_type&
                            operator&=(long v) _SCV_IMPL2(&=) return_type&
                            operator&=(unsigned long v) _SCV_IMPL2(&=) return_type&
                            operator&=(int v) _SCV_IMPL2(&=) return_type&
                            operator&=(unsigned int v) _SCV_IMPL2(&=) return_type&
                            operator&=(const sc_int_base& v) _SCV_IMPL2(&=) return_type&
                            operator&=(const sc_uint_base& v) _SCV_IMPL2(&=)

                                return_type&
                                operator|=(const sc_signed& v) _SCV_IMPL2(|=) return_type&
                                operator|=(const sc_unsigned& v) _SCV_IMPL2(|=) return_type&
                                operator|=(int64 v) _SCV_IMPL2(|=) return_type&
                                operator|=(uint64 v) _SCV_IMPL2(|=) return_type&
                                operator|=(long v) _SCV_IMPL2(|=) return_type&
                                operator|=(unsigned long v) _SCV_IMPL2(|=) return_type&
                                operator|=(int v) _SCV_IMPL2(|=) return_type&
                                operator|=(unsigned int v) _SCV_IMPL2(|=) return_type&
                                operator|=(const sc_int_base& v) _SCV_IMPL2(|=) return_type&
                                operator|=(const sc_uint_base& v) _SCV_IMPL2(|=)

                                    return_type&
                                    operator^=(const sc_signed& v) _SCV_IMPL2(^=) return_type&
                                    operator^=(const sc_unsigned& v) _SCV_IMPL2(^=) return_type&
                                    operator^=(int64 v) _SCV_IMPL2(^=) return_type&
                                    operator^=(uint64 v) _SCV_IMPL2(^=) return_type&
                                    operator^=(long v) _SCV_IMPL2(^=) return_type&
                                    operator^=(unsigned long v) _SCV_IMPL2(^=) return_type&
                                    operator^=(int v) _SCV_IMPL2(^=) return_type&
                                    operator^=(unsigned int v) _SCV_IMPL2(^=) return_type&
                                    operator^=(const sc_int_base& v) _SCV_IMPL2(^=) return_type&
                                    operator^=(const sc_uint_base& v) _SCV_IMPL2(^=)

                                        return_type&
                                        operator<<=(const sc_signed& v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(const sc_unsigned& v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(int64 v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(uint64 v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(long v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(unsigned long v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(int v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(unsigned int v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(const sc_int_base& v) _SCV_IMPL2(<<=) return_type&
                                        operator<<=(const sc_uint_base& v) _SCV_IMPL2(<<=)

                                            return_type&
                                            operator>>=(const sc_signed& v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(const sc_unsigned& v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(int64 v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(uint64 v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(long v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(unsigned long v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(int v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(unsigned int v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(const sc_int_base& v) _SCV_IMPL2(>>=) return_type&
                                            operator>>=(const sc_uint_base& v) _SCV_IMPL2(>>=)

                                                return_type&
                                                operator++() {
        ++*this->_get_instance();
        this->trigger_value_change_cb();
        return *this;
    }
    const sc_unsigned operator++(int) {
        sc_bigint<W> tmp = *this->_get_instance()++;
        this->trigger_value_change_cb();
        return tmp;
    }
    return_type& operator--() {
        --*this->_get_instance();
        this->trigger_value_change_cb();
        return *this;
    }
    const sc_unsigned operator--(int) {
        sc_bigint<W> tmp = *this->_get_instance()--;
        this->trigger_value_change_cb();
        return tmp;
    }
    //  sc_unsigned_bitref operator [] (int i)
    const bool operator[](int i) const { return this->_get_instance()->operator[](i); }
    const sc_unsigned range(int i, int j) const { return this->_get_instance()->range(i, j); }
    //  sc_unsigned_subref operator () (int i, int j)
    const sc_unsigned operator()(int i, int j) const { return this->_get_instance()->operator()(i, j); }

    std::string to_string(sc_numrep base = SC_DEC, bool formatted = false) const {
        return this->_get_instance()->to_string(base, formatted);
    }
    std::string to_string(int base, bool formatted = false) const {
        return this->_get_instance()->to_string(base, formatted);
    }

    _SCV_MAP(int64, to_int64);
    _SCV_MAP(uint64, to_uint64);
    _SCV_MAP(long, to_long);
    _SCV_MAP(unsigned long, to_ulong);
    _SCV_MAP(unsigned long, to_unsigned_long);
    _SCV_MAP(int, to_int);
    _SCV_MAP(int, to_signed);
    _SCV_MAP(unsigned int, to_uint);
    _SCV_MAP(unsigned int, to_unsigned);
    _SCV_MAP(unsigned int, to_unsigned_int);
    _SCV_MAP(double, to_double);
    //  void print() const {  this->_get_instance()->print(); }
    //  void print(ostream &os) const {  this->_get_instance()->print(os); }
    void dump() const { this->_get_instance()->dump(); };
    void dump(std::ostream& os) const { this->_get_instance()->dump(os); };
    _SCV_MAP(int, length);
    _SCV_MAP(bool, iszero);
    _SCV_MAP(bool, sign);
    bool test(int i) const { return this->_get_instance()->test(i); }
    void set(int i) {
        this->_get_instance()->set(i);
        this->trigger_value_change_cb();
    }
    void clear(int i) {
        this->_get_instance()->clear(i);
        this->trigger_value_change_cb();
    }
    void set(int i, bool v) {
        this->_get_instance()->set(i, v);
        this->trigger_value_change_cb();
    }
    void invert(int i) {
        this->_get_instance()->invert(i);
        this->trigger_value_change_cb();
    }
    void reverse() {
        this->_get_instance()->reverse();
        this->trigger_value_change_cb();
    }
    void get_packed_rep(sc_dt::sc_digit* buf) const { this->_get_instance()->get_packet_ref(buf); }
    void set_packed_rep(sc_dt::sc_digit* buf) {
        this->_get_instance()->get_packet_ref(buf);
        this->trigger_value_change_cb();
    }

    operator const sc_signed&() const { return *this->_get_instance(); }
};

template <> class scv_extensions<sc_bit> : public scv_extensions_base<sc_bit> {
public:
    typedef scv_extensions<sc_bit> return_type;

    return_type& operator=(const return_type& v) _SCV_IMPL1 return_type&
    operator=(const sc_bit& v) _SCV_IMPL return_type&
    operator=(int v) _SCV_IMPL return_type&
    operator=(bool v) _SCV_IMPL return_type&
    operator=(char v) _SCV_IMPL return_type&
    operator&=(const sc_bit& v) _SCV_IMPL2(&=) return_type&
    operator&=(int v) _SCV_IMPL2(&=) return_type&
    operator&=(bool v) _SCV_IMPL2(&=) return_type&
    operator&=(char v) _SCV_IMPL2(&=) return_type&
    operator|=(const sc_bit& v) _SCV_IMPL2(|=) return_type&
    operator|=(int v) _SCV_IMPL2(|=) return_type&
    operator|=(bool v) _SCV_IMPL2(|=) return_type&
    operator|=(char v) _SCV_IMPL2(|=) return_type&
    operator^=(const sc_bit& v) _SCV_IMPL2(^=) return_type&
    operator^=(int v) _SCV_IMPL2(^=) return_type&
    operator^=(bool v) _SCV_IMPL2(^=) return_type&
    operator^=(char v) _SCV_IMPL2(^=) _SCV_MAP(bool, to_bool);
    _SCV_MAP(char, to_char);
    //  void print( ostream& os) const {  return this->_get_instance()->print(os); }
    operator const sc_bit&() const { return *this->_get_instance(); }
};

template <> class scv_extensions<sc_logic> : public scv_extensions_base<sc_logic> {
public:
    typedef scv_extensions<sc_logic> return_type;

    return_type& operator=(const return_type& v) _SCV_IMPL1 return_type&
    operator=(sc_dt::sc_logic_value_t v) _SCV_IMPL return_type&
    operator=(const sc_logic& v) _SCV_IMPL return_type&
    operator=(char v) _SCV_IMPL return_type&
    operator=(int v) _SCV_IMPL return_type&
    operator=(bool v) _SCV_IMPL return_type&
    operator&=(const sc_logic& v) _SCV_IMPL2(&=) return_type&
    operator|=(const sc_logic& v) _SCV_IMPL2(|=) return_type&
    operator^=(const sc_logic& v) _SCV_IMPL2(^=) bool
    operator==(const sc_logic& r) const {
        return *this->_get_instance() == r;
    }
    bool operator==(char r) const { return *this->_get_instance() == r; }
    bool operator!=(const sc_logic& r) const { return *this->_get_instance() != r; }
    bool operator!=(char r) const { return *this->_get_instance() != r; }
    _SCV_MAP(char, to_char);
    _SCV_MAP(bool, is_01);
    _SCV_MAP(bool, to_bool);
    //  void print( ostream& os ) const {  this->_get_instance()->print(os); }

    operator const sc_logic&() const { return *this->_get_instance(); }
};

template <int W> class scv_extensions<sc_bv<W>> : public scv_extensions_base<sc_bv<W>> {
public:
    operator const sc_bv<W>&() const { return *this->_get_instance(); }

    typedef scv_extensions<sc_bv<W>> return_type;

    sc_bv_base* clone() { return this->_get_instance()->clone(); /* don't clone randomization status */ }
    return_type& operator=(const return_type& v) _SCV_IMPL1
        //  template<class T> return_type& operator=(const sc_proxy<T>& v) _SCV_IMPL
        return_type&
        operator=(const sc_bv<W>& v) _SCV_IMPL return_type&
        operator=(const char* v) _SCV_IMPL return_type&
        operator=(const bool* v) _SCV_IMPL return_type&
        operator=(const sc_unsigned& v) _SCV_IMPL return_type&
        operator=(const sc_signed& v) _SCV_IMPL return_type&
        operator=(const sc_uint_base& v) _SCV_IMPL return_type&
        operator=(const sc_int_base& v) _SCV_IMPL return_type&
        operator=(long v) _SCV_IMPL return_type&
        operator=(unsigned long v) _SCV_IMPL return_type&
        operator=(int v) _SCV_IMPL return_type&
        operator=(unsigned v) _SCV_IMPL return_type&
        operator=(char v) _SCV_IMPL return_type&
        operator=(const sc_bit& v) _SCV_IMPL return_type&
        operator=(int64 v) _SCV_IMPL return_type&
        operator=(uint64 v) _SCV_IMPL return_type&
        operator=(const sc_int<W>& v) _SCV_IMPL return_type&
        operator=(const sc_uint<W>& v) _SCV_IMPL

        void resize(unsigned long new_size) {
        this->_get_instance()->resize(new_size);
        this->trigger_value_change_cb();
    }

    // from sc_bv_base
    long get_bit(unsigned n) const { return this->_get_instance()->get_bit(n); }
    void set_bit(unsigned bit_number, long value) {
        this->_get_instance()->set_bit(bit_number, value);
        this->trigger_value_change_cb();
    }
    unsigned long get_word(unsigned i) const { return this->_get_instance()->get_word(i); }
    void set_word(unsigned i, unsigned long w) {
        this->_get_instance()->set_word(i, w);
        this->trigger_value_change_cb();
    }
    unsigned long get_cword(unsigned i) const { return this->_get_instance()->get_cword(i); }
    void set_cword(unsigned i, unsigned long w) {
        this->_get_instance()->set_cword(i, w);
        this->trigger_value_change_cb();
    }
    _SCV_MAP(int, length);

    return_type& operator&=(const sc_unsigned& v) _SCV_IMPL2(&=) return_type& operator|=(const sc_unsigned& v)
        _SCV_IMPL2(|=) return_type&
        operator^=(const sc_unsigned& v) _SCV_IMPL2(^=)

            return_type&
            operator&=(const sc_signed& v) _SCV_IMPL2(&=) return_type&
            operator|=(const sc_signed& v) _SCV_IMPL2(|=) return_type&
            operator^=(const sc_signed& v) _SCV_IMPL2(^=)

                return_type&
                operator&=(unsigned int v) _SCV_IMPL2(&=) return_type&
                operator|=(unsigned int v) _SCV_IMPL2(|=) return_type&
                operator^=(unsigned int v) _SCV_IMPL2(^=)

                    return_type&
                    operator&=(int v) _SCV_IMPL2(&=) return_type&
                    operator|=(int v) _SCV_IMPL2(|=) return_type&
                    operator^=(int v) _SCV_IMPL2(^=)

                        return_type&
                        operator&=(unsigned long v) _SCV_IMPL2(&=) return_type&
                        operator|=(unsigned long v) _SCV_IMPL2(|=) return_type&
                        operator^=(unsigned long v) _SCV_IMPL2(^=)

                            return_type&
                            operator&=(long v) _SCV_IMPL2(&=) return_type&
                            operator|=(long v) _SCV_IMPL2(|=) return_type&
                            operator^=(long v) _SCV_IMPL2(^=)

                                return_type&
                                operator&=(const char* v) _SCV_IMPL2(&=) return_type&
                                operator|=(const char* v) _SCV_IMPL2(|=) return_type&
                                operator^=(const char* v) _SCV_IMPL2(^=)

                                    sc_bv_base
                                    operator&(const char* s) const {
        return *this->_get_instance() & s;
    }
    sc_bv_base operator|(const char* s) const { return *this->_get_instance() | s; }
    sc_bv_base operator^(const char* s) const { return *this->_get_instance() ^ s; }

    friend return_type operator&(const char* s, const return_type& b) {
        b.initialize();
        return *b._get_instance() & s;
    }
    friend return_type operator|(const char* s, const return_type& b) {
        b.initialize();
        return *b._get_instance() | s;
    }
    friend return_type operator^(const char* s, const return_type& b) {
        b.initialize();
        return *b._get_instance() ^ s;
    }

    void set(unsigned long v = 0) {
        this->_get_instance()->set(v);
        this->trigger_value_change_cb();
    }
};

template <int W> class scv_extensions<sc_lv<W>> : public scv_extensions_base<sc_lv<W>> {
public:
    operator const sc_lv<W>&() const { return *this->_get_instance(); }

    typedef scv_extensions<sc_lv<W>> return_type;

    sc_bv_base* clone() { return this->_get_instance()->clone(); /* don't clone randomization status */ }
    return_type& operator=(const return_type& v) _SCV_IMPL1
        //  template<class T> return_type& operator=(const sc_proxy<T>& v) _SCV_IMPL
        return_type&
        operator=(const sc_lv<W>& v) _SCV_IMPL return_type&
        operator=(const char* v) _SCV_IMPL return_type&
        operator=(const bool* v) _SCV_IMPL return_type&
        operator=(const sc_logic* v) _SCV_IMPL // this is the only difference from sc_bv.
        return_type&
        operator=(const sc_unsigned& v) _SCV_IMPL return_type&
        operator=(const sc_signed& v) _SCV_IMPL return_type&
        operator=(const sc_uint_base& v) _SCV_IMPL return_type&
        operator=(const sc_int_base& v) _SCV_IMPL return_type&
        operator=(long v) _SCV_IMPL return_type&
        operator=(unsigned long v) _SCV_IMPL return_type&
        operator=(int v) _SCV_IMPL return_type&
        operator=(unsigned v) _SCV_IMPL return_type&
        operator=(char v) _SCV_IMPL return_type&
        operator=(const sc_bit& v) _SCV_IMPL return_type&
        operator=(int64 v) _SCV_IMPL return_type&
        operator=(uint64 v) _SCV_IMPL return_type&
        operator=(const sc_int<W>& v) _SCV_IMPL return_type&
        operator=(const sc_uint<W>& v) _SCV_IMPL

        void resize(unsigned long new_size) {
        this->_get_instance()->resize(new_size);
        this->trigger_value_change_cb();
    }

    // from sc_bv_base
    long get_bit(unsigned n) const { return this->_get_instance()->get_bit(n); }
    void set_bit(unsigned bit_number, long value) {
        this->_get_instance()->set_bit(bit_number, value);
        this->trigger_value_change_cb();
    }
    unsigned long get_word(unsigned i) const { return this->_get_instance()->get_word(i); }
    void set_word(unsigned i, unsigned long w) {
        this->_get_instance()->set_word(i, w);
        this->trigger_value_change_cb();
    }
    unsigned long get_cword(unsigned i) const { return this->_get_instance()->get_cword(i); }
    void set_cword(unsigned i, unsigned long w) {
        this->_get_instance()->set_cword(i, w);
        this->trigger_value_change_cb();
    }
    _SCV_MAP(int, length);

    return_type& operator&=(const sc_unsigned& v) _SCV_IMPL2(&=) return_type& operator|=(const sc_unsigned& v)
        _SCV_IMPL2(|=) return_type&
        operator^=(const sc_unsigned& v) _SCV_IMPL2(^=)

            return_type&
            operator&=(const sc_signed& v) _SCV_IMPL2(&=) return_type&
            operator|=(const sc_signed& v) _SCV_IMPL2(|=) return_type&
            operator^=(const sc_signed& v) _SCV_IMPL2(^=)

                return_type&
                operator&=(unsigned int v) _SCV_IMPL2(&=) return_type&
                operator|=(unsigned int v) _SCV_IMPL2(|=) return_type&
                operator^=(unsigned int v) _SCV_IMPL2(^=)

                    return_type&
                    operator&=(int v) _SCV_IMPL2(&=) return_type&
                    operator|=(int v) _SCV_IMPL2(|=) return_type&
                    operator^=(int v) _SCV_IMPL2(^=)

                        return_type&
                        operator&=(unsigned long v) _SCV_IMPL2(&=) return_type&
                        operator|=(unsigned long v) _SCV_IMPL2(|=) return_type&
                        operator^=(unsigned long v) _SCV_IMPL2(^=)

                            return_type&
                            operator&=(long v) _SCV_IMPL2(&=) return_type&
                            operator|=(long v) _SCV_IMPL2(|=) return_type&
                            operator^=(long v) _SCV_IMPL2(^=)

                                return_type&
                                operator&=(const char* v) _SCV_IMPL2(&=) return_type&
                                operator|=(const char* v) _SCV_IMPL2(|=) return_type&
                                operator^=(const char* v) _SCV_IMPL2(^=)

                                    sc_bv_base
                                    operator&(const char* s) const {
        return *this->_get_instance() & s;
    }
    sc_bv_base operator|(const char* s) const { return *this->_get_instance() | s; }
    sc_bv_base operator^(const char* s) const { return *this->_get_instance() ^ s; }

    friend return_type operator&(const char* s, const return_type& b) {
        b.initialize();
        return *b._get_instance() & s;
    }
    friend return_type operator|(const char* s, const return_type& b) {
        b.initialize();
        return *b._get_instance() | s;
    }
    friend return_type operator^(const char* s, const return_type& b) {
        b.initialize();
        return *b._get_instance() ^ s;
    }

    bool is_01() { return this->_get_instance()->is_01(); } // this should have been "const"
};

#endif // SystemC

#undef _SCV_INTEGER_INTERFACE

// ----------------------------------------
// special extension class to handle getting an extension from an extension
// ----------------------------------------
template <typename T> class scv_extensions<scv_extensions<T>> : public scv_extensions<T> {
public:
    scv_extensions() {}
    scv_extensions(const scv_extensions<T>& rhs)
    : scv_extensions<T>(rhs) {}
    virtual ~scv_extensions() {}
    scv_extensions& operator=(const scv_extensions<T>& rhs) { return scv_extensions<T>::operator=(rhs); }
    scv_extensions& operator=(const T& rhs) { return scv_extensions<T>::operator=(rhs); }
    operator const T&() const { return *scv_extensions<T>::_get_instance(); }

    virtual void _set_instance(T* i) { scv_extensions<T>::_set_instance(i); }
    virtual void _set_instance(scv_extensions<T>* i) { scv_extensions<T>::_set_instance(i->_get_instance()); }
};

// ----------------------------------------
// specialization for array
// ----------------------------------------
template <typename T, int N> class scv_extensions<T[N]> : public scv_extension_rw<T[N]> {

    typedef T my_type[N];

public:
    // ----------------------------------------
    // implementation of the specialization for array
    // (added cast of N to "int" since some compilers automatically
    // regard it as unsigned even though I have declard it as int)
    // ----------------------------------------
    scv_extensions();
    virtual ~scv_extensions(){};

public:
    scv_extensions<T>& operator[](int i);
    const scv_extensions<T>& operator[](int i) const;

public:
    virtual void _set_instance_core_wrap(void* p);

public:
    scv_extensions& operator=(const scv_extensions& rhs);
    scv_extensions& operator=(const T* rhs);

private:
    scv_extensions<T> _array[N];
};

// ----------------------------------------
// specialization for pointers
// ----------------------------------------
template <typename T> class scv_smart_ptr;

template <typename T> class scv_extensions<T*> : public scv_extension_rw<T*> {
public:
    // (can only be used with pointer to a single object)
    // (cannot only be used with pointer to an array)
    scv_extensions() {}
    virtual ~scv_extensions() {}

public:
    scv_extensions<T>& operator*();
    const scv_extensions<T>& operator*() const;
    scv_extensions<T>* operator->();
    const scv_extensions<T>* operator->() const;

public:
    scv_extensions<T*>& operator=(const scv_extensions<T*>& rhs);
    scv_extensions<T*>& operator=(scv_extensions<T>* rhs);
    scv_extensions<T*>& operator=(const scv_smart_ptr<T>& rhs);
    scv_extensions<T*>& operator=(T* rhs);
    scv_extensions<T*>& operator=(int);

public:
    virtual void _set_instance_core_wrap(void* p);
    const char* get_type_name() const {
        static const char* s = _scv_ext_util_get_name("%s*", scv_extensions<T>().get_type_name());
        return s;
    }

private:
    mutable bool _own_typed_ptr;
    mutable scv_extensions<T>* _typed_ptr;
    const scv_extensions<T>* _get_ptr() const;
    const scv_extensions<T>* _set_ptr() const;
};

// ----------------------------------------
// implementation of the specialization for array
// (added cast of N to "int" since some compilers automatically
// regard it as unsigned even though I have declard it as int)
// ----------------------------------------
template <typename T, int N> scv_extensions<T[N]>::scv_extensions() {
    std::string tmp;
    _scv_extension_util** a = new _scv_extension_util*[N];
    for(int i = 0; i < (int)N; ++i) {
        a[i] = &_array[i];
        tmp = "[" + _scv_ext_util_get_string(i) + "]";
        _array[i]._set_parent(this, tmp);
    }
    this->_set_up_array(a);
}

template <typename T, int N> scv_extensions<T>& scv_extensions<T[N]>::operator[](int i) {
    if(i < 0 || i >= (int)N) {
        _scv_message::message(_scv_message::INTROSPECTION_INVALID_INDEX, i, "array", this->get_name());
        return _array[0];
    }
    return _array[i];
}

template <typename T, int N> const scv_extensions<T>& scv_extensions<T[N]>::operator[](int i) const {
    if(i < 0 || i >= (int)N) {
        _scv_message::message(_scv_message::INTROSPECTION_INVALID_INDEX, i, "array", this->get_name());
        return _array[0];
    }
    return _array[i];
}

template <typename T, int N> void scv_extensions<T[N]>::_set_instance_core_wrap(void* p) {
    if(p) {
        my_type* tp = (my_type*)p;
        for(int i = 0; i < (int)N; ++i)
            _array[i]._set_instance(&(*tp)[i]);
    }
}

/*
template<typename T, int N>
scv_extensions<T[N]>& scv_extensions<T[N]>::operator=(const scv_extensions& rhs) {
  for (int i=0; i<(int)N; ++i) { _array[i] = rhs[i]; }
  this->trigger_value_change_cb();
}
*/

template <typename T, int N> scv_extensions<T[N]>& scv_extensions<T[N]>::operator=(const T* rhs) {
    for(int i = 0; i < (int)N; ++i) {
        _array[i] = rhs[i];
    }
    this->trigger_value_change_cb();
    return *this;
}

// ----------------------------------------
// implementation of the specialization for pointers
// ----------------------------------------
template <typename T> const scv_extensions<T>* scv_extensions<T*>::_get_ptr() const {
    if(*(this->_instance)) {
        if(!this->_ptr) {
            this->_own_typed_ptr = true;
            this->_typed_ptr = new scv_extensions<T>();
            this->_ptr = this->_typed_ptr;
            this->_typed_ptr->_set_instance(*this->_instance);
        }
    } else {
        if(this->_ptr) {
            if(this->_own_typed_ptr)
                delete this->_typed_ptr;
            this->_typed_ptr = NULL;
            this->_ptr = NULL;
        }
    }
    return this->_typed_ptr;
}

template <typename T> const scv_extensions<T>* scv_extensions<T*>::_set_ptr() const {
    if(*this->_get_instance()) {
        if(!this->_ptr) {
            this->_own_typed_ptr = true;
            this->_typed_ptr = new scv_extensions<T>();
            this->_ptr = this->_typed_ptr;
        }
        this->_typed_ptr->_set_instance(*this->_get_instance());
    } else {
        if(this->_ptr) {
            if(this->_own_typed_ptr)
                delete this->_typed_ptr;
            this->_typed_ptr = NULL;
            this->_ptr = NULL;
        }
    }
    return this->_typed_ptr;
}

template <typename T> scv_extensions<T>& scv_extensions<T*>::operator*() {
    const scv_extensions<T>* ptr = _get_ptr();
    if(!ptr) {
        static scv_extensions<T> e;
        _scv_message::message(_scv_message::INTROSPECTION_NULL_POINTER, this->get_name());
        return e;
    }
    return *(scv_extensions<T>*)ptr;
}

template <typename T> const scv_extensions<T>& scv_extensions<T*>::operator*() const {
    const scv_extensions<T>* ptr = _get_ptr();
    if(!ptr) {
        static scv_extensions<T> e;
        _scv_message::message(_scv_message::INTROSPECTION_NULL_POINTER, this->get_name());
        return &e; // cppcheck-suppress returnTempReference
    }
    return *ptr;
}

template <typename T> scv_extensions<T>* scv_extensions<T*>::operator->() { return (scv_extensions<T>*)_get_ptr(); }

template <typename T> const scv_extensions<T>* scv_extensions<T*>::operator->() const {
    return (scv_extensions<T>*)_get_ptr();
}

template <typename T> scv_extensions<T*>& scv_extensions<T*>::operator=(const scv_extensions<T*>& rhs) {
    *this->_get_instance() = *rhs._instance;
    if(rhs._ptr->is_dynamic()) {
        // share the same extension until the object disapear
        // (in practise (but slow), probably need to register
        // a deletion callback.
        this->_own_typed_ptr = false;
        this->_ptr = rhs._ptr;
        this->_typed_ptr = rhs._typed_ptr;
    }
    _set_ptr();
    this->trigger_value_change_cb();
    return *this;
}

template <typename T> scv_extensions<T*>& scv_extensions<T*>::operator=(scv_extensions<T>* rhs) {
    *this->_get_instance() = rhs->_instance;
    if(rhs->is_dynamic()) {
        // share the same extension until the object disapear
        // (in practise (but slow), probably need to register
        // a deletion callback.
        this->_own_typed_ptr = false;
        this->_ptr = rhs;
        this->_typed_ptr = rhs;
    }
    _set_ptr();
    this->trigger_value_change_cb();
    return *this;
}

/* this is in _scv_smart_ptr.h
template<typename T> scv_extensions<T*>&
scv_extensions<T*>::operator=(const scv_smart_ptr<T>& rhs) { ... }
*/

template <typename T> scv_extensions<T*>& scv_extensions<T*>::operator=(T* rhs) {
    *this->_get_instance() = rhs;
    _set_ptr();
    this->trigger_value_change_cb();
    return *this;
}

template <typename T> scv_extensions<T*>& scv_extensions<T*>::operator=(int rhs) {
    *this->_get_instance() = (T*)rhs;
    _set_ptr();
    this->trigger_value_change_cb();
    return *this;
}

template <typename T> void scv_extensions<T*>::_set_instance_core_wrap(void*) { _set_ptr(); }

// ----------------------------------------
// various access interface to access an extension object
//
// scv_get_const_extensions is there to get around HP ambituous
// overloaded function call problem.
// ----------------------------------------
template <typename T, typename std::enable_if<std::is_pointer<T>::value, T>::type* = nullptr>
scv_extensions<T> scv_get_extensions(T& d) {
    scv_extensions<T> e;
    e._set_instance((T*)&d);
    e=d;
    return e;
};

template <typename T, typename std::enable_if<!std::is_pointer<T>::value, T>::type* = nullptr>
scv_extensions<T> scv_get_extensions(T& d) {
    scv_extensions<T> e;
    e._set_instance(&d);
    return e;
};

template <typename T, typename std::enable_if<std::is_pointer<T>::value, T>::type* = nullptr>
const scv_extensions<T> scv_get_const_extensions(const T& d) {
    scv_extensions<T> e;
    e._set_instance((T*)&d);
    e._set_instance_core_wrap(nullptr);
    return e;
};

template <typename T, typename std::enable_if<!std::is_pointer<T>::value, T>::type* = nullptr>
const scv_extensions<T> scv_get_const_extensions(const T& d) {
    scv_extensions<T> e;
    e._set_instance((T*)&d);
    return e;
};
} // namespace scv_tr
#endif
