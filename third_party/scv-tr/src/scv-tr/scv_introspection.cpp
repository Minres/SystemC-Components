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

  scv_introspection.cpp -- The static data for the introspection facility.

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/

/*
 * Fixes ambiguous operator error in SystemC 2.2.0 when building SCV with
 * SC_INCLUDE_FX defined
 */
#if defined(SC_INCLUDE_FX) && !defined(SC_FX_EXCLUDE_OTHER)
#define SC_FX_EXCLUDE_OTHER
#endif

#include "scv_introspection.h"
#include <array>

namespace scv_tr {

class scv_debug {
public:
    enum dummy { INITIAL_DEBUG_LEVEL, INTROSPECTION };
    static void set_facility_level(dummy, int i = 0) {}
};
#define _SCV_DEFERR(code, number, string, severity)                                                                    \
    static _scv_message_desc* code##_base = 0;                                                                         \
    static _scv_message_desc** code;
#include "scv_messages.h"
#undef _SCV_DEFERR

// ----------------------------------------
// mimic _scv_message for _SCV_INTROSPECTION_ONLY
// ----------------------------------------
struct scv_error_data {
    const char* error_name;
    const int error_number;
    const char* error_string;
};

// ----------------------------------------
// _scv_extension_util
// ----------------------------------------

const char* _scv_extension_util::get_name() const { return _name.c_str(); }
const char* _scv_extension_util::kind() const {
    static const std::string s = "scv_extensions_if";
    return s.c_str();
}
void _scv_extension_util::print(std::ostream& o, int details, int indent) const {
    std::string space = "";
    for(int i = 0; i < indent; ++i)
        space += " ";

    switch(get_type()) {
    case BOOLEAN:
        if(get_bool()) {
            if(details == 0)
                o << space << "true" << std::endl;
            else
                o << space << get_short_name() << ":"
                  << "true" << std::endl;
        } else {
            if(details == 0)
                o << space << "false" << std::endl;
            else
                o << space << get_short_name() << ":"
                  << "false" << std::endl;
        }
        break;
    case ENUMERATION:
        if(details == 0)
            o << space << get_enum_string((int)get_integer()) << std::endl;
        else
            o << space << get_short_name() << ":" << get_enum_string((int)get_integer()) << std::endl;
        break;
    case INTEGER:
#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
        if(get_bitwidth() > 64) {
            sc_bv_base v(get_bitwidth());
            get_value(v);
            if(details == 0)
                o << space << v << std::endl;
            else
                o << space << get_short_name() << ":" << v << std::endl;
            return;
        }
#endif
        if(details == 0)
            o << space << get_integer() << std::endl;
        else
            o << space << get_short_name() << ":" << get_integer() << std::endl;
        break;
    case UNSIGNED:
#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
        if(get_bitwidth() > 64) {
            sc_bv_base v(get_bitwidth());
            get_value(v);
            if(details == 0)
                o << space << v << std::endl;
            else
                o << space << get_short_name() << ":" << v << std::endl;
            return;
        }
#endif
        if(details == 0)
            o << space << get_unsigned() << std::endl;
        else
            o << space << get_short_name() << ":" << get_unsigned() << std::endl;
        break;
    case FLOATING_POINT_NUMBER:
        if(details == 0)
            o << space << get_double() << std::endl;
        else
            o << space << get_short_name() << ":" << get_double() << std::endl;
        break;
    case BIT_VECTOR:
#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
    {
        sc_bv_base v(get_bitwidth());
        get_value(v);
        if(details == 0)
            o << space << v << std::endl;
        else
            o << space << get_short_name() << ":" << v << std::endl;
    }
#endif
    break;
    case LOGIC_VECTOR:
#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
    {
        sc_lv_base v(get_bitwidth());
        get_value(v);
        if(details == 0)
            o << space << v << std::endl;
        else
            o << space << get_short_name() << ":" << v << std::endl;
    }
#endif
    break;
    case FIXED_POINT_INTEGER:
        _scv_message::message(_scv_message::NOT_IMPLEMENTED_YET, "data introspection for fixed pointer number");
        break;
    case UNSIGNED_FIXED_POINT_INTEGER:
        _scv_message::message(_scv_message::NOT_IMPLEMENTED_YET, "data introspection for fixed pointer number");
        break;
    case RECORD: {
        int size = get_num_fields();
        if(get_short_name() != "")
            o << space << get_short_name() << " {" << std::endl;
        else
            o << space << "{" << std::endl;
        if(size) {
            for(int i = 0; i < size; ++i)
                get_field(i)->print(o, 1, indent + 2);
        }
        o << space << "}" << std::endl;
        break;
    }
    case POINTER:
        if(details == 0)
            o << space << "(pointer)" << std::endl;
        else
            o << space << get_short_name() << ": (pointer)" << std::endl;
        break;
    case ARRAY: {
        if(get_short_name() != "")
            o << space << get_short_name() << " {" << std::endl;
        else
            o << space << "{" << std::endl;
        int size = get_array_size();
        for(int i = 0; i < size; ++i)
            get_array_elt(i)->print(o, 1, indent + 2);
        o << space << "}" << std::endl;
        break;
    }
    case STRING:
        if(details == 0)
            o << space << get_string() << std::endl;
        else
            o << space << get_short_name() << ":" << get_string() << std::endl;
        break;
    default:
        _scv_message::message(_scv_message::INTERNAL_ERROR, "Unrecognized data introspection type.");
        break;
    }
}

bool _scv_extension_util::has_valid_extensions() const { return true; }
bool _scv_extension_util::is_dynamic() const { return _is_dynamic(); }

std::string _scv_extension_util::get_short_name() const { return std::string(_short_name.c_str()); }
void _scv_extension_util::set_name(const char* s) { _name = s; }
void _scv_extension_util::_set_name(const std::string& s) { _name = s; }

void _scv_extension_util::_set_parent(_scv_extension_util* p, const std::string& name) {
    if(!_parent) {
        _parent = p;
        _short_name = name;
        _set_name(name);
    }
}
void _scv_extension_util::_set_dynamic() {}

// ----------------------------------------
// _scv_extension_util_enum
// ----------------------------------------

void _scv_extension_util_enum::_get_enum_details(std::list<const char*>& names, std::list<int>& values) const {
    names = _get_names();
    values = _get_values();
}
const char* _scv_extension_util_enum::_get_enum_string(int e) const {
    std::list<const char*>& names = _get_names();
    std::list<int>& values = _get_values();
    std::list<const char*>::iterator i;
    std::list<int>::iterator j;
    for(i = names.begin(), j = values.begin(); i != names.end(); ++i, ++j) {
        if(*j == e)
            return *i;
    }
    _scv_message::message(_scv_message::INTROSPECTION_INVALID_ENUM_VALUE, get_type_name(), e);
    return "_error";
}

// ----------------------------------------
// _scv_extension_type_enum
// ----------------------------------------

scv_extensions_if::data_type _scv_extension_type_enum::get_type() const { return scv_extensions_if::ENUMERATION; }

int _scv_extension_type_enum::get_enum_size() const { return _get_enum_size(); }
void _scv_extension_type_enum::get_enum_details(std::list<const char*>& names, std::list<int>& values) const {
    _get_enum_details(names, values);
}
const char* _scv_extension_type_enum::get_enum_string(int e) const { return _get_enum_string(e); }

int _scv_extension_type_enum::get_num_fields() const { return 0; }
scv_extensions_if* _scv_extension_type_enum::get_field(unsigned) { return nullptr; }
const scv_extensions_if* _scv_extension_type_enum::get_field(unsigned) const { return nullptr; }

scv_extensions_if* _scv_extension_type_enum::get_pointer() { return nullptr; }
const scv_extensions_if* _scv_extension_type_enum::get_pointer() const { return nullptr; }

int _scv_extension_type_enum::get_array_size() const { return 0; }
scv_extensions_if* _scv_extension_type_enum::get_array_elt(int) { return nullptr; }
const scv_extensions_if* _scv_extension_type_enum::get_array_elt(int) const { return nullptr; }

// ----------------------------------------
// _scv_extension_rw_enum
// ----------------------------------------

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
#define _SCV_DEFAULT_RW_SYSC(class_name, obj)                                                                          \
    void class_name::assign(const sc_bv_base& v) { _SCV_RW_ERROR(assign, sc_bv_base, obj); }                           \
    void class_name::get_value(sc_bv_base& v) const { _SCV_RW_ERROR(get_value, sc_bv_base, obj); }                     \
    void class_name::assign(const sc_lv_base& v) { _SCV_RW_ERROR(assign, sc_lv_base, obj); }                           \
    void class_name::get_value(sc_lv_base& v) const { _SCV_RW_ERROR(get_value, sc_lv_base, obj); }
#else
#define _SCV_DEFAULT_RW_SYSC(class_name, obj)
#endif

void _scv_extension_rw_enum::assign(bool v) {
    *_get_instance() = v;
    trigger_value_change_cb();
}
void _scv_extension_rw_enum::assign(char v) {
    *_get_instance() = v;
    trigger_value_change_cb();
}
void _scv_extension_rw_enum::assign(unsigned char v) {
    *_get_instance() = v;
    trigger_value_change_cb();
}
void _scv_extension_rw_enum::assign(short v) {
    *_get_instance() = v;
    trigger_value_change_cb();
}
void _scv_extension_rw_enum::assign(unsigned short v) {
    *_get_instance() = v;
    trigger_value_change_cb();
}
void _scv_extension_rw_enum::assign(int v) {
    *_get_instance() = v;
    trigger_value_change_cb();
}
void _scv_extension_rw_enum::assign(unsigned v) {
    *_get_instance() = v;
    trigger_value_change_cb();
}
void _scv_extension_rw_enum::assign(long v) {
    *_get_instance() = v;
    trigger_value_change_cb();
}
void _scv_extension_rw_enum::assign(unsigned long v) {
    *_get_instance() = v;
    trigger_value_change_cb();
}
void _scv_extension_rw_enum::assign(long long v) {
    *_get_instance() = (int)v;
    trigger_value_change_cb();
}
void _scv_extension_rw_enum::assign(unsigned long long v) {
    *_get_instance() = (int)v;
    trigger_value_change_cb();
}
void _scv_extension_rw_enum::assign(float) { _SCV_RW_ERROR(assign, float, enum); }
void _scv_extension_rw_enum::assign(double) { _SCV_RW_ERROR(assign, double, enum); }
void _scv_extension_rw_enum::assign(const std::string& s) { assert(s.c_str()); }
void _scv_extension_rw_enum::assign(const char* s) {
    std::list<int>& values = _get_values();
    std::list<const char*>& names = _get_names();
    auto i = values.begin();
    auto j = names.begin();
    while(j != names.end() && 0 != strcmp(*j, s)) {
        ++i;
        ++j;
    }
    if(j != names.end())
        assign(*j);
    else
        _scv_message::message(_scv_message::INTROSPECTION_INVALID_ENUM_STRING, s);
    trigger_value_change_cb();
}
bool _scv_extension_rw_enum::get_bool() const {
    _SCV_RW_ERROR(get_bool, bool, obj);
    return false;
}
long long _scv_extension_rw_enum::get_integer() const { return *_get_instance(); }
unsigned long long _scv_extension_rw_enum::get_unsigned() const {
    _SCV_RW_ERROR(get_unsigned, unsigned, obj);
    return 0;
}
double _scv_extension_rw_enum::get_double() const {
    _SCV_RW_ERROR(get_double, double, obj);
    return 0;
}
std::string _scv_extension_rw_enum::get_string() const { return get_enum_string((int)get_integer()); }

_SCV_DEFAULT_RW_SYSC(_scv_extension_rw_enum, enum)

// ----------------------------------------
// _scv_extension_rand_enum
// -> need to be fixed
// ----------------------------------------

// ----------------------------------------
// _scv_extension_callbacks_enum
// ----------------------------------------

#define _SCV_EXT_TYPE_FC_COMMON_I(type_id)                                                                             \
    int _scv_extension_type_##type_id::get_enum_size() const { return 0; }                                             \
    void _scv_extension_type_##type_id::get_enum_details(std::list<const char*>&, std::list<int>&) const {}            \
    const char* _scv_extension_type_##type_id::get_enum_string(int) const { return "_error"; }                         \
                                                                                                                       \
    int _scv_extension_type_##type_id::get_num_fields() const { return 0; }                                            \
    scv_extensions_if* _scv_extension_type_##type_id::get_field(unsigned) { return 0; }                                \
    const scv_extensions_if* _scv_extension_type_##type_id::get_field(unsigned) const { return 0; }                    \
                                                                                                                       \
    scv_extensions_if* _scv_extension_type_##type_id::get_pointer() { return 0; }                                      \
    const scv_extensions_if* _scv_extension_type_##type_id::get_pointer() const { return 0; }                          \
                                                                                                                       \
    int _scv_extension_type_##type_id::get_array_size() const { return 0; }                                            \
    scv_extensions_if* _scv_extension_type_##type_id::get_array_elt(int) { return 0; }                                 \
    const scv_extensions_if* _scv_extension_type_##type_id::get_array_elt(int) const { return 0; }                     \
                                                                                                                       \
    scv_extensions_if* _scv_extension_type_##type_id::get_parent() { return this->_parent; }                           \
    const scv_extensions_if* _scv_extension_type_##type_id::get_parent() const { return this->_parent; }

#define _SCV_EXT_TYPE_FC_COMMON_EXT_I(basic_type, type_id, id)                                                         \
    _SCV_EXT_TYPE_FC_COMMON_I(type_id)                                                                                 \
    const char* _scv_extension_type_##type_id::get_type_name() const {                                                 \
        static const char* s = strdup(#basic_type);                                                                    \
        return s;                                                                                                      \
    }                                                                                                                  \
    scv_extension_type_if::data_type _scv_extension_type_##type_id::get_type() const { return scv_extensions_if::id; }

// specialization for basic types
#define _SCV_EXT_TYPE_FC_I(basic_type, type_id, id)                                                                    \
    _SCV_EXT_TYPE_FC_COMMON_EXT_I(basic_type, type_id, id)                                                             \
    int _scv_extension_type_##type_id::get_bitwidth() const { return 8 * sizeof(basic_type); }

// specialization for single-bit types
#define _SCV_EXT_TYPE_1_FC_I(basic_type, type_id, id)                                                                  \
    _SCV_EXT_TYPE_FC_COMMON_EXT_I(basic_type, type_id, id)                                                             \
    int _scv_extension_type_##type_id::get_bitwidth() const { return 1; }

// specialization for dynamically-sized types
#define _SCV_EXT_TYPE_D_FC_I(basic_type, type_id, id)                                                                  \
    _SCV_EXT_TYPE_FC_COMMON_EXT_I(basic_type, type_id, id)                                                             \
    _scv_extension_type_##type_id::_scv_extension_type_##type_id()                                                     \
    : _bitwidth(0) {}                                                                                                  \
    _scv_extension_type_##type_id::~_scv_extension_type_##type_id() {}                                                 \
    int _scv_extension_type_##type_id::get_bitwidth() const { return this->_bitwidth; }

// Method Bodies for _scv_extension_type_bool class

_SCV_EXT_TYPE_1_FC_I(bool, bool, BOOLEAN);

// Method Bodies for _scv_extension_type_##type_id classes

_SCV_EXT_TYPE_FC_I(char, char, INTEGER);
_SCV_EXT_TYPE_FC_I(short, short, INTEGER);
_SCV_EXT_TYPE_FC_I(int, int, INTEGER);
_SCV_EXT_TYPE_FC_I(long, long, INTEGER);
_SCV_EXT_TYPE_FC_I(long long, long_long, INTEGER);
_SCV_EXT_TYPE_FC_I(unsigned char, unsigned_char, UNSIGNED);
_SCV_EXT_TYPE_FC_I(unsigned short, unsigned_short, UNSIGNED);
_SCV_EXT_TYPE_FC_I(unsigned int, unsigned_int, UNSIGNED);
_SCV_EXT_TYPE_FC_I(unsigned long, unsigned_long, UNSIGNED);
_SCV_EXT_TYPE_FC_I(unsigned long long, unsigned_long_long, UNSIGNED);
_SCV_EXT_TYPE_FC_I(float, float, FLOATING_POINT_NUMBER);
_SCV_EXT_TYPE_FC_I(double, double, FLOATING_POINT_NUMBER);
_SCV_EXT_TYPE_FC_I(std::string, string, STRING);

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
_SCV_EXT_TYPE_1_FC_I(sc_bit, sc_bit, BIT_VECTOR);
_SCV_EXT_TYPE_1_FC_I(sc_logic, sc_logic, LOGIC_VECTOR);
_SCV_EXT_TYPE_D_FC_I(sc_signed, sc_signed, INTEGER);
_SCV_EXT_TYPE_D_FC_I(sc_unsigned, sc_unsigned, UNSIGNED);
_SCV_EXT_TYPE_D_FC_I(sc_int_base, sc_int_base, INTEGER);
_SCV_EXT_TYPE_D_FC_I(sc_uint_base, sc_uint_base, UNSIGNED);
_SCV_EXT_TYPE_D_FC_I(sc_lv_base, sc_lv_base, LOGIC_VECTOR);
_SCV_EXT_TYPE_D_FC_I(sc_bv_base, sc_bv_base, BIT_VECTOR);
#endif

#undef _SCV_EXT_TYPE_FC_I
#undef _SCV_EXT_TYPE_1_FC_I
#undef _SCV_EXT_TYPE_D_FC_I

//////////////////////////////////////////////////////////////////////

// ----------------------------------------
// specialization for records
// ----------------------------------------

#define _SCV_EXT_RW_FC_BASE_I(T, type_id)                                                                              \
    _scv_extension_rw_##type_id::_scv_extension_rw_##type_id() {}                                                      \
    _scv_extension_rw_##type_id::~_scv_extension_rw_##type_id() {}                                                     \
                                                                                                                       \
    inline const T* _scv_extension_rw_##type_id::get_instance() const { return _instance; }                            \
    inline T* _scv_extension_rw_##type_id::get_instance() { return _instance; }                                        \
    T* _scv_extension_rw_##type_id::_get_instance() const { return _instance; }                                        \
                                                                                                                       \
    void _scv_extension_rw_##type_id::_set_instance(T* p) {                                                            \
        _instance = p;                                                                                                 \
        _set_instance_core_wrap(p);                                                                                    \
    }                                                                                                                  \
    void _scv_extension_rw_##type_id::_set_instance_core_wrap(void*) {}                                                \
    void _scv_extension_rw_##type_id::_set_as_field(_scv_extension_util_record* parent, T* p,                          \
                                                    const std::string& name) {                                         \
        if(p)                                                                                                          \
            _set_instance(p);                                                                                          \
        else if(!this->_get_parent()) {                                                                                \
            this->_set_parent(parent, name);                                                                           \
            parent->_add_field(this);                                                                                  \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    const T& _scv_extension_rw_##type_id::read() { return *get_instance(); }                                           \
    void _scv_extension_rw_##type_id::write(const T& rhs) {                                                            \
        *_get_instance() = rhs;                                                                                        \
        this->trigger_value_change_cb();                                                                               \
    }

// ----------------------------------------
// specialization for basic types
// ----------------------------------------

#define _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, arg_name)                                                          \
    void _scv_extension_rw_##type_id::assign(arg_name i) {                                                             \
        *(this->_get_instance()) = i;                                                                                  \
        this->trigger_value_change_cb();                                                                               \
    }

#define _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, arg_name)                                                      \
    void _scv_extension_rw_##type_id::assign(arg_name i) { _SCV_RW_ERROR(assign, arg_name, type_name); }

#define _SCV_EXT_RW_FC_GET_VALUE_I(type_name, type_id, arg_name)                                                       \
    void _scv_extension_rw_##type_id::get_value(arg_name& i) { i = *(this->_get_instance()); }

// ------------------------------------------------------------
// C/C++ Types
// ------------------------------------------------------------

// --------------
// C/C++ types (begin)
// --------------

#define _SCV_EXT_RW_FC_I(basic_type, type_id, bitwidth)                                                                \
    _SCV_EXT_RW_FC_BASE_I(basic_type, type_id)                                                                         \
    _SCV_EXT_RW_FC_ASSIGNS_I(basic_type, type_id, bitwidth);

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
#define _SCV_EXT_RW_FC_BAD_ASSIGNS_SYSC_I(type_name, type_id)                                                          \
    void _scv_extension_rw_##type_id::assign(const sc_bv_base& v) { _SCV_RW_ERROR(assign, sc_bv_base, type_name); }    \
    void _scv_extension_rw_##type_id::get_value(sc_bv_base& v) const {                                                 \
        _SCV_RW_ERROR(get_value, sc_bv_base, type_name);                                                               \
    }                                                                                                                  \
    void _scv_extension_rw_##type_id::assign(const sc_lv_base& v) { _SCV_RW_ERROR(assign, sc_lv_base, type_name); }    \
    void _scv_extension_rw_##type_id::get_value(sc_lv_base& v) const {                                                 \
        _SCV_RW_ERROR(get_value, sc_lv_base, type_name);                                                               \
    }

#define _SCV_EXT_RW_FC_ASSIGNS_SYSC_I(type_id, bitwidth)                                                               \
    void _scv_extension_rw_##type_id::assign(const sc_bv_base& v) {                                                    \
        if(this->get_bitwidth() != v.length())                                                                         \
            _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, "sc_bv_base", "assign");    \
        static sc_int<bitwidth> tmp;                                                                                   \
        tmp = v;                                                                                                       \
        *(this->_get_instance()) = tmp;                                                                                \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    void _scv_extension_rw_##type_id::get_value(sc_bv_base& v) const {                                                 \
        if(this->get_bitwidth() != v.length())                                                                         \
            _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, "sc_bv_base", "get_value"); \
        v = *(this->_get_instance());                                                                                  \
    }                                                                                                                  \
    void _scv_extension_rw_##type_id::assign(const sc_lv_base& v) {                                                    \
        if(this->get_bitwidth() != v.length())                                                                         \
            _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, "sc_lv_base", "assign");    \
        static sc_int<bitwidth> tmp;                                                                                   \
        tmp = v;                                                                                                       \
        *(this->_get_instance()) = tmp;                                                                                \
        this->trigger_value_change_cb();                                                                               \
    }                                                                                                                  \
    void _scv_extension_rw_##type_id::get_value(sc_lv_base& v) const {                                                 \
        if(this->get_bitwidth() != v.length())                                                                         \
            _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, "sc_lv_base", "get_value"); \
        v = *(this->_get_instance());                                                                                  \
    }

#else
#define _SCV_EXT_RW_FC_ASSIGNS_SYSC_I(type_id, bitwidth)
#define _SCV_EXT_RW_FC_BAD_ASSIGNS_SYSC_I(type_name, type_id)
#endif

// --------------
// integer types
// --------------

#define _SCV_EXT_RW_FC_ASSIGNS_I(type_name, type_id, bitwidth)                                                         \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, bool);                                                                 \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, char);                                                                 \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned char);                                                        \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, short);                                                                \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned short);                                                       \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, int);                                                                  \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned int);                                                         \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, long);                                                                 \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned long);                                                        \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, long long);                                                            \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned long long);                                                   \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, float);                                                            \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, double);                                                           \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, const std::string&);                                               \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, const char*);                                                      \
    bool _scv_extension_rw_##type_id::get_bool() const { return *(this->_get_instance()) != 0; }                       \
    long long _scv_extension_rw_##type_id::get_integer() const { return *(this->_get_instance()); }                    \
    unsigned long long _scv_extension_rw_##type_id::get_unsigned() const { return *(this->_get_instance()); }          \
    double _scv_extension_rw_##type_id::get_double() const { return *(this->_get_instance()); }                        \
    std::string _scv_extension_rw_##type_id::get_string() const {                                                      \
        assert(0);                                                                                                     \
        return std::string("");                                                                                        \
    }                                                                                                                  \
    _SCV_EXT_RW_FC_ASSIGNS_SYSC_I(type_id, bitwidth)

_SCV_EXT_RW_FC_I(bool, bool, 1);
_SCV_EXT_RW_FC_I(char, char, 8);
_SCV_EXT_RW_FC_I(unsigned char, unsigned_char, 8);
_SCV_EXT_RW_FC_I(short, short, 16);
_SCV_EXT_RW_FC_I(unsigned short, unsigned_short, 16);
_SCV_EXT_RW_FC_I(int, int, 32);
_SCV_EXT_RW_FC_I(unsigned int, unsigned_int, 32);
_SCV_EXT_RW_FC_I(long, long, 32);
_SCV_EXT_RW_FC_I(unsigned long, unsigned_long, 32);
_SCV_EXT_RW_FC_I(long long, long_long, 64);
_SCV_EXT_RW_FC_I(unsigned long long, unsigned_long_long, 64);

#undef _SCV_EXT_RW_FC_ASSIGNS_I

// --------------
// floating pointer types
// --------------

#define _SCV_EXT_RW_FC_ASSIGNS_I(type_name, type_id, dummy)                                                            \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, bool);                                                                 \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, char);                                                                 \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned char);                                                        \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, short);                                                                \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned short);                                                       \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, int);                                                                  \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned int);                                                         \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, long);                                                                 \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned long);                                                        \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, long long);                                                            \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned long long);                                                   \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, float);                                                                \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, double);                                                               \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, const std::string&);                                               \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, const char*);                                                      \
    bool _scv_extension_rw_##type_id::get_bool() const {                                                               \
        _SCV_RW_ERROR(get_bool, bool, type_name);                                                                      \
        return false;                                                                                                  \
    }                                                                                                                  \
    long long _scv_extension_rw_##type_id::get_integer() const {                                                       \
        _SCV_RW_ERROR(get_integer, integer, type_name);                                                                \
        return 0;                                                                                                      \
    }                                                                                                                  \
    unsigned long long _scv_extension_rw_##type_id::get_unsigned() const {                                             \
        _SCV_RW_ERROR(get_unsigned, unsigned, type_name);                                                              \
        return 0;                                                                                                      \
    }                                                                                                                  \
    double _scv_extension_rw_##type_id::get_double() const { return *(this->_get_instance()); }                        \
    std::string _scv_extension_rw_##type_id::get_string() const {                                                      \
        assert(0);                                                                                                     \
        return std::string("");                                                                                        \
    }                                                                                                                  \
    _SCV_EXT_RW_FC_BAD_ASSIGNS_SYSC_I(type_name, type_id)

_SCV_EXT_RW_FC_I(float, float, dummy);
_SCV_EXT_RW_FC_I(double, double, dummy);

#undef _SCV_EXT_RW_FC_ASSIGNS_I

// --------------
// string type
// --------------
#define _SCV_EXT_RW_FC_ASSIGNS_I(type_name, type_id, dummy)                                                            \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, bool);                                                             \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, char);                                                             \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, unsigned char);                                                    \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, short);                                                            \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, unsigned short);                                                   \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, int);                                                              \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, unsigned int);                                                     \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, long);                                                             \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, unsigned long);                                                    \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, long long);                                                        \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, unsigned long long);                                               \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, float);                                                            \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, double);                                                           \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, const std::string&);                                                   \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, const char*);                                                          \
    bool _scv_extension_rw_##type_id::get_bool() const { return *(this->_get_instance()) != ""; }                      \
    long long _scv_extension_rw_##type_id::get_integer() const {                                                       \
        _SCV_RW_ERROR(get_integer, integer, type_name);                                                                \
        return 0;                                                                                                      \
    }                                                                                                                  \
    unsigned long long _scv_extension_rw_##type_id::get_unsigned() const {                                             \
        _SCV_RW_ERROR(get_unsigned, unsigned, type_name);                                                              \
        return 0;                                                                                                      \
    }                                                                                                                  \
    double _scv_extension_rw_##type_id::get_double() const {                                                           \
        _SCV_RW_ERROR(get_double, double, type_name);                                                                  \
        return 0;                                                                                                      \
    }                                                                                                                  \
    std::string _scv_extension_rw_##type_id::get_string() const {                                                      \
        return std::string(this->_get_instance()->c_str());                                                            \
    }                                                                                                                  \
    _SCV_EXT_RW_FC_BAD_ASSIGNS_SYSC_I(type_name, type_id)

_SCV_EXT_RW_FC_I(std::string, string, dummy);

#undef _SCV_EXT_RW_FC_ASSIGNS_I

// --------------
// C/C++ types (end)
// --------------

#undef _SCV_EXT_RW_FC_I

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
#undef _SCV_EXT_RW_FC_ASSIGNS_SYSC_I
#undef _SCV_EXT_RW_FC_BAD_ASSIGNS_SYSC_I
#endif

// ------------------------------------------------------------
// SystemC Types
// ------------------------------------------------------------

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)

// --------------
// sc_bit and sc_logic (begin)
// --------------

#define _SCV_EXT_RW_FC_BOOL_ASSIGN_I(type_name, type_id, arg_name)                                                     \
    void _scv_extension_rw_##type_id::assign(arg_name i) {                                                             \
        if(i)                                                                                                          \
            *this->_get_instance() = 1;                                                                                \
        else                                                                                                           \
            *this->_get_instance() = 0;                                                                                \
        this->trigger_value_change_cb();                                                                               \
    }

#undef _SCV_EXT_RW_FC_ASSIGNS_I
#define _SCV_EXT_RW_FC_ASSIGNS_I(type_name, type_id)                                                                   \
    _SCV_EXT_RW_FC_BOOL_ASSIGN_I(type_name, type_id, bool);                                                            \
    _SCV_EXT_RW_FC_BOOL_ASSIGN_I(type_name, type_id, char);                                                            \
    _SCV_EXT_RW_FC_BOOL_ASSIGN_I(type_name, type_id, unsigned char);                                                   \
    _SCV_EXT_RW_FC_BOOL_ASSIGN_I(type_name, type_id, short);                                                           \
    _SCV_EXT_RW_FC_BOOL_ASSIGN_I(type_name, type_id, unsigned short);                                                  \
    _SCV_EXT_RW_FC_BOOL_ASSIGN_I(type_name, type_id, int);                                                             \
    _SCV_EXT_RW_FC_BOOL_ASSIGN_I(type_name, type_id, unsigned int);                                                    \
    _SCV_EXT_RW_FC_BOOL_ASSIGN_I(type_name, type_id, long);                                                            \
    _SCV_EXT_RW_FC_BOOL_ASSIGN_I(type_name, type_id, unsigned long);                                                   \
    _SCV_EXT_RW_FC_BOOL_ASSIGN_I(type_name, type_id, long long);                                                       \
    _SCV_EXT_RW_FC_BOOL_ASSIGN_I(type_name, type_id, unsigned long long);                                              \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, float);                                                            \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, double);                                                           \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, const std::string&);                                               \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, const char*);                                                      \
    bool _scv_extension_rw_##type_id::get_bool() const { return this->_get_instance()->to_bool(); }                    \
    long long _scv_extension_rw_##type_id::get_integer() const { return this->get_bool(); }                            \
    unsigned long long _scv_extension_rw_##type_id::get_unsigned() const { return this->get_bool(); }                  \
    double _scv_extension_rw_##type_id::get_double() const { return this->get_bool(); }

// --------------
// sc_bit
// --------------

_SCV_EXT_RW_FC_BASE_I(sc_bit, sc_bit);
_SCV_EXT_RW_FC_ASSIGNS_I(sc_bit, sc_bit);
std::string _scv_extension_rw_sc_bit::get_string() const {
    if(get_integer())
        return std::string("1");
    else
        return std::string("0");
}
void _scv_extension_rw_sc_bit::assign(const sc_bv_base& v) {
    if(v.length() != 1)
        _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, "sc_bv_base", "assign");
    *(this->_get_instance()) = v.get_bit(0);
    this->trigger_value_change_cb();
}
void _scv_extension_rw_sc_bit::get_value(sc_bv_base& v) const {
    if(v.length() != 1)
        _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, "sc_bv_base", "get_value");

    if(this->_get_instance()->to_bool())
        v.set_bit(0, sc_dt::Log_1);
    else
        v.set_bit(0, sc_dt::Log_0);
}
void _scv_extension_rw_sc_bit::assign(const sc_lv_base& v) { _SCV_RW_ERROR(assign, sc_lv_base, sc_bit); }
void _scv_extension_rw_sc_bit::get_value(sc_lv_base& v) const { _SCV_RW_ERROR(get_value, sc_lv_base, sc_bit); }

// --------------
// sc_logic
// --------------

_SCV_EXT_RW_FC_BASE_I(sc_logic, sc_logic);
_SCV_EXT_RW_FC_ASSIGNS_I(sc_logic, sc_logic);
std::string _scv_extension_rw_sc_logic::get_string() const {
    std::array<char, 2> str_val;
    sprintf(str_val.data(), "%c", this->_get_instance()->to_char());
    return str_val.data();
}
void _scv_extension_rw_sc_logic::assign(const sc_bv_base& v) { _SCV_RW_ERROR(assign, sc_bv_base, sc_logic); }
void _scv_extension_rw_sc_logic::get_value(sc_bv_base& v) const { _SCV_RW_ERROR(get_value, sc_lv_base, sc_logic); }
void _scv_extension_rw_sc_logic::assign(const sc_lv_base& v) {
    if(v.length() != 1)
        _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, "sc_bv_base", "assign");
    *(this->_get_instance()) = v.get_bit(0);
    this->trigger_value_change_cb();
}
void _scv_extension_rw_sc_logic::get_value(sc_lv_base& v) const {
    if(v.length() != 1)
        _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, "sc_bv_base", "get_value");
    v.set_bit(0, this->_get_instance()->value());
}

// --------------
// sc_bit and sc_logic (end)
// --------------

// --------------
// SystemC Dynamic Types
// --------------

#define _SCV_EXT_RW_FC_BASE_D_I(T, type_id)                                                                            \
    _scv_extension_rw_##type_id::_scv_extension_rw_##type_id() {}                                                      \
    _scv_extension_rw_##type_id::~_scv_extension_rw_##type_id() {}                                                     \
                                                                                                                       \
    inline const T* _scv_extension_rw_##type_id::get_instance() const { return _instance; }                            \
    inline T* _scv_extension_rw_##type_id::get_instance() { return _instance; }                                        \
    T* _scv_extension_rw_##type_id::_get_instance() const { return _instance; }                                        \
                                                                                                                       \
    void _scv_extension_rw_##type_id::_set_instance(T* p) {                                                            \
        _instance = p;                                                                                                 \
        _set_instance_core_wrap(p);                                                                                    \
        _bitwidth = p ? p->length() : 0;                                                                               \
    }                                                                                                                  \
    void _scv_extension_rw_##type_id::_set_instance_core_wrap(void*) {}                                                \
    void _scv_extension_rw_##type_id::_set_as_field(_scv_extension_util_record* parent, T* p,                          \
                                                    const std::string& name) {                                         \
        if(p)                                                                                                          \
            _set_instance(p);                                                                                          \
        else if(!this->_get_parent()) {                                                                                \
            this->_set_parent(parent, name);                                                                           \
            parent->_add_field(this);                                                                                  \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    const T& _scv_extension_rw_##type_id::read() { return *get_instance(); }                                           \
    void _scv_extension_rw_##type_id::write(const T& rhs) {                                                            \
        *_get_instance() = rhs;                                                                                        \
        this->trigger_value_change_cb();                                                                               \
    }

#define _SCV_EXT_RW_FC_GET_VALUE_D_I(type_name, type_id, arg_name)                                                     \
    void _scv_extension_rw_##type_id::get_value(arg_name& i) const { i = *(this->_get_instance()); }

#define _SCV_EXT_RW_FC_ASSIGNS_D(type_name, type_id)                                                                   \
    _SCV_EXT_RW_FC_BASE_D_I(type_name, type_id);                                                                       \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, bool);                                                                 \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, char);                                                                 \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned char);                                                        \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, short);                                                                \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned short);                                                       \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, int);                                                                  \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned int);                                                         \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, long);                                                                 \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned long);                                                        \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, long long);                                                            \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned long long);                                                   \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, float);                                                            \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, double);                                                           \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, const std::string&);                                               \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, const char*);                                                      \
    bool _scv_extension_rw_##type_id::get_bool() const { return *(this->_get_instance()) != 0; }                       \
    long long _scv_extension_rw_##type_id::get_integer() const { return this->_get_instance()->to_int64(); }           \
    unsigned long long _scv_extension_rw_##type_id::get_unsigned() const {                                             \
        return this->_get_instance()->to_uint64();                                                                     \
    }                                                                                                                  \
    double _scv_extension_rw_##type_id::get_double() const { return this->_get_instance()->to_double(); }              \
    std::string _scv_extension_rw_##type_id::get_string() const {                                                      \
        assert(0);                                                                                                     \
        return std::string("");                                                                                        \
    }                                                                                                                  \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, const sc_bv_base&);                                                    \
    _SCV_EXT_RW_FC_GET_VALUE_D_I(type_name, type_id, sc_bv_base);                                                      \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, const sc_lv_base&);                                                    \
    _SCV_EXT_RW_FC_GET_VALUE_D_I(type_name, type_id, sc_lv_base);

#define _SCV_EXT_RW_FC_ASSIGNS_V(type_name, type_id)                                                                   \
    _SCV_EXT_RW_FC_BASE_D_I(type_name, type_id);                                                                       \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, bool);                                                                 \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, char);                                                                 \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned char);                                                        \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, short);                                                                \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned short);                                                       \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, int);                                                                  \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned int);                                                         \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, long);                                                                 \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned long);                                                        \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, long long);                                                            \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, unsigned long long);                                                   \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, float);                                                            \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, double);                                                           \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, const std::string&);                                               \
    _SCV_EXT_RW_FC_BAD_ASSIGN_I(type_name, type_id, const char*);                                                      \
    bool _scv_extension_rw_##type_id::get_bool() const { return *(this->_get_instance()) != 0; }                       \
    long long _scv_extension_rw_##type_id::get_integer() const {                                                       \
        _SCV_RW_ERROR(get_integer, integer, type_name);                                                                \
        return 0;                                                                                                      \
    }                                                                                                                  \
    unsigned long long _scv_extension_rw_##type_id::get_unsigned() const {                                             \
        _SCV_RW_ERROR(get_unsigned, unsigned, type_name);                                                              \
        return 0;                                                                                                      \
    }                                                                                                                  \
    double _scv_extension_rw_##type_id::get_double() const {                                                           \
        _SCV_RW_ERROR(get_double, double, type_name);                                                                  \
        return 0;                                                                                                      \
    }                                                                                                                  \
    std::string _scv_extension_rw_##type_id::get_string() const {                                                      \
        assert(0);                                                                                                     \
        return std::string("");                                                                                        \
    }                                                                                                                  \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, const sc_bv_base&);                                                    \
    _SCV_EXT_RW_FC_GET_VALUE_D_I(type_name, type_id, sc_bv_base);                                                      \
    _SCV_EXT_RW_FC_ASSIGN_I(type_name, type_id, const sc_lv_base&);                                                    \
    _SCV_EXT_RW_FC_GET_VALUE_D_I(type_name, type_id, sc_lv_base);

_SCV_EXT_RW_FC_ASSIGNS_D(sc_signed, sc_signed)
_SCV_EXT_RW_FC_ASSIGNS_D(sc_unsigned, sc_unsigned)
_SCV_EXT_RW_FC_ASSIGNS_D(sc_int_base, sc_int_base)
_SCV_EXT_RW_FC_ASSIGNS_D(sc_uint_base, sc_uint_base)
_SCV_EXT_RW_FC_ASSIGNS_V(sc_lv_base, sc_lv_base)
_SCV_EXT_RW_FC_ASSIGNS_V(sc_bv_base, sc_bv_base)

// --------------
// SystemC Dynamic Types (end)
// --------------

#undef _SCV_EXT_RW_FC_BOOL_ASSIGN_I
#undef _SCV_EXT_RW_FC_ASSIGNS_I

#undef _SCV_EXT_RW_FC_ASSIGNS_STRING_I
#undef _SCV_EXT_RW_FC_ASSIGNS_SYSC_I

#endif

// ----------------------------------------
// wrap up this component
// ----------------------------------------
#undef _SCV_EXT_RW_FC_ASSIGN_I
#undef _SCV_EXT_RW_FC_BAD_ASSIGN_I

#undef _SCV_EXT_RW_FC_COMMON_SYSC_I
#undef _SCV_EXT_RW_FC_COMMON_I

//////////////////////////////////////////////////////////////////////
} // namespace scv_tr
//////////////////////////////////////////////////////////////////////
