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

  _scv_ext_rw.h -- The implementation for the extension component "scv_tag_rw".

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey, Samir Agrawal
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/

#include <cassert>
#include <string>

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
#define _SCV_DEFAULT_RW_SYSC \
  virtual void assign(const sc_bv_base& v) { assert(0); } \
  virtual void get_value(sc_bv_base& v) const { assert(0); } \
  virtual void assign(const sc_lv_base& v) { assert(0); } \
  virtual void get_value(sc_lv_base& v) const { assert(0); }
#else
#define _SCV_DEFAULT_RW_SYSC
#endif
#define _SCV_DEFAULT_RW \
  virtual void assign(bool) { assert(0); } \
  virtual void assign(char) { assert(0); } \
  virtual void assign(unsigned char) { assert(0); } \
  virtual void assign(short) { assert(0); }		       \
  virtual void assign(unsigned short) { assert(0); }	       \
  virtual void assign(int) { assert(0); }		       \
  virtual void assign(unsigned) { assert(0); }		       \
  virtual void assign(long) { assert(0); }		       \
  virtual void assign(unsigned long) { assert(0); }	       \
  virtual void assign(long long) { assert(0); }		       \
  virtual void assign(unsigned long long) { assert(0); } \
  virtual void assign(float) { assert(0); }		       \
  virtual void assign(double) { assert(0); }		       \
  virtual void assign(const std::string&) { assert(0); }	       \
  virtual void assign(const char *) { assert(0); }	       \
							       \
  virtual bool get_bool() const  { assert(0); return false; } \
  virtual long long get_integer() const  { assert(0); return 0; }	   \
  virtual unsigned long long get_unsigned() const { assert(0); return 0; } \
  virtual double get_double() const { assert(0); return 0; }		   \
  virtual std::string get_string() const { assert(0); return std::string(""); } \
							       \
  _SCV_DEFAULT_RW_SYSC					       \

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
#define _SCV_INTROSPECTION_RW_FC_D_SYSC           \
  virtual void assign(const sc_bv_base& v); \
  virtual void get_value(sc_bv_base& v) const; \
  virtual void assign(const sc_lv_base& v); \
  virtual void get_value(sc_lv_base& v) const;
#else
#define _SCV_INTROSPECTION_RW_FC_D_SYSC
#endif
#define _SCV_INTROSPECTION_RW_FC_D \
  virtual void assign(bool); \
  virtual void assign(char); \
  virtual void assign(unsigned char); \
  virtual void assign(short); \
  virtual void assign(unsigned short); \
  virtual void assign(int); \
  virtual void assign(unsigned); \
  virtual void assign(long); \
  virtual void assign(unsigned long); \
  virtual void assign(long long); \
  virtual void assign(unsigned long long); \
  virtual void assign(float); \
  virtual void assign(double); \
  virtual void assign(const std::string&); \
  virtual void assign(const char *); \
  \
  virtual bool get_bool() const; \
  virtual long long get_integer() const; \
  virtual unsigned long long get_unsigned() const; \
  virtual double get_double() const; \
  virtual std::string get_string() const; \
  \
  _SCV_INTROSPECTION_RW_FC_D_SYSC \

#define _SCV_IMPLEMENT_RW(type_id) \
  const type_id& read() const { return *_get_instance(); } \
  void write(const type_id& rhs) { *_get_instance() = rhs; this->trigger_value_change_cb(); } \
  void _set_instance(type_id * p) { _instance = p; _set_instance_core_wrap(p); } \
  void _set_as_field(_scv_extension_util_record * parent,              \
			    type_id * p, const std::string& name) { \
    if (p) _set_instance(p); \
    else if ( ! this->_get_parent() ) { this->_set_parent(parent,name); parent->_add_field(this); } \
  } \
  type_id* _get_instance() const { return _instance; } \
  type_id* get_instance() { \
    _scv_message::message(_scv_message::INTROSPECTION_GET_INSTANCE_USAGE); \
    return _instance; \
  } \
  const type_id* get_instance() const { return _instance; } \

#define _SCV_IMPLEMENT_RW_FULL(type_id) \
  _SCV_IMPLEMENT_RW(type_id); \
  virtual void _set_instance_core_wrap(void * p) {} \
  type_id * _instance; \

// ----------------------------------------
// specialization for records
// ----------------------------------------
template<typename T>
class _scv_extension_rw_base
 : public _SCV_INTROSPECTION_BASE {
public:
  _scv_extension_rw_base() {}
  virtual ~_scv_extension_rw_base() {}

public: // public API for use only when full type information is available
  const T* get_instance() const { return _instance; }
  T* get_instance() { return _instance; }
  T* _get_instance() const { return _instance; }

public: // internal API for implementation only
  void _set_instance(T* p) { _instance = p; _set_instance_core_wrap(p); }
  virtual void _set_instance_core_wrap(void* p) {}
  void _set_as_field(_scv_extension_util_record * parent, T* p,
		    const std::string& name) {
    if (p) _set_instance(p);
    else if ( ! this->_get_parent() ) { this->_set_parent(parent,name); parent->_add_field(this); }
  }

public:
  _SCV_DEFAULT_RW
  const T& read() { return *get_instance(); }
  void write(const T& rhs) { *_get_instance() = rhs; this->trigger_value_change_cb(); }

public:
  T * _instance;
};

template<typename T>
class scv_extension_rw
  : public _scv_extension_rw_base<T>
{};
 
//Rachida
// ----------------------------------------
// specialization for array
// ----------------------------------------
template<typename T, int N>
class scv_extension_rw<T[N]>
  : public _SCV_INTROSPECTION_BASE2 {
  typedef T my_type[N];
public:
  scv_extension_rw() {}
  virtual ~scv_extension_rw() {}

public:
  _SCV_IMPLEMENT_RW_FULL(my_type)
  _SCV_INTROSPECTION_RW_FC_D
};

template<typename T, int N> void scv_extension_rw<T[N]>::assign(bool) {
  _SCV_RW_ERROR(assign,bool,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(char) {
  _SCV_RW_ERROR(assign,char,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(unsigned char) {
  _SCV_RW_ERROR(assign,unsigned char,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(short) {
  _SCV_RW_ERROR(assign,short,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(unsigned short) {
  _SCV_RW_ERROR(assign,unsigned short,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(int) {
  _SCV_RW_ERROR(assign,int,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(unsigned) {
  _SCV_RW_ERROR(assign,unsigned,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(long) {
  _SCV_RW_ERROR(assign,long,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(unsigned long) {
  _SCV_RW_ERROR(assign,unsigned long,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(long long) {
  _SCV_RW_ERROR(assign,long long,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(unsigned long long) {
  _SCV_RW_ERROR(assign,unsigned long long,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(float) {
  _SCV_RW_ERROR(assign,float,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(double) {
  _SCV_RW_ERROR(assign,double,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(const std::string&) {
  _SCV_RW_ERROR(assign,std::string,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(const char *) {
  _SCV_RW_ERROR(assign,const char *,array);
}

template<typename T, int N> bool scv_extension_rw<T[N]>::get_bool() const  {
  _SCV_RW_ERROR(get_bool,bool,array); return false;
}
template<typename T, int N> long long scv_extension_rw<T[N]>::get_integer() const  {
  _SCV_RW_ERROR(get_integer,integer,array); return 0;
}
template<typename T, int N> unsigned long long scv_extension_rw<T[N]>::get_unsigned() const {
  _SCV_RW_ERROR(get_unsigned,unsigned,array); return 0;
}
template<typename T, int N> double scv_extension_rw<T[N]>::get_double() const {
  _SCV_RW_ERROR(get_double,double,array); return 0;
}
template<typename T, int N> std::string scv_extension_rw<T[N]>::get_string() const {
  _SCV_RW_ERROR(get_string,string,array); return std::string("");
}

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
template<typename T, int N> void scv_extension_rw<T[N]>::assign(const sc_bv_base& v) {
  _SCV_RW_ERROR(assign,sc_bv_base,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::get_value(sc_bv_base& v) const {
  _SCV_RW_ERROR(get_value,sc_bv_base,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::assign(const sc_lv_base& v) {
  _SCV_RW_ERROR(assign,sc_lv_base,array);
}
template<typename T, int N> void scv_extension_rw<T[N]>::get_value(sc_lv_base& v) const {
  _SCV_RW_ERROR(get_value,sc_lv_base,array);
}
#endif
// ----------------------------------------
// specialization for pointers
// ----------------------------------------
template<typename T>
class scv_extension_rw<T*>
 : public _SCV_INTROSPECTION_BASE1 {
public: // public API for use only when full type information is available
  T** const get_instance() const { return _instance; }
  T** get_instance() { return _instance; }
  T** _get_instance() const { return _instance; }

public:
  void _set_instance(T** p) { _instance = p; }
  void _set_as_field(_scv_extension_util_record * parent, T** p,
		    const std::string& name) {
    if (p) _set_instance(p); 
    else if ( ! this->_get_parent() ) { this->_set_parent(parent,name); parent->_add_field(this); }
  }

public:
  _SCV_DEFAULT_RW
  const T& read() { return *get_instance(); }
  void write(const T& rhs) {
    *_get_instance() = rhs; this->trigger_value_change_cb();
  }
public:
  T ** _instance;
};

// ----------------------------------------
// specialization for enums
// ----------------------------------------
class _scv_extension_rw_enum
  : public _SCV_INTROSPECTION_BASE_ENUM {
public:
  _scv_extension_rw_enum() {}
  virtual ~_scv_extension_rw_enum() {}

  _SCV_INTROSPECTION_RW_FC_D

  int read() const { return *_get_instance(); }
  void write(int rhs) {
    *_get_instance() = rhs; this->trigger_value_change_cb();
  }
  void _set_instance(int * p) { _instance = p; }
  void _set_as_field(_scv_extension_util_record * parent, int * p,
		     const std::string& name) {
    if (p) _set_instance(p);
    else if ( ! this->_get_parent() ) { _set_parent(parent,name); parent->_add_field(this); }
  }
  int * _get_instance() const { return _instance; }
  int * get_instance() {
    _scv_message::message(_scv_message::INTROSPECTION_GET_INSTANCE_USAGE);
    return _instance;
  }
  const int * get_instance() const { return _instance; }

  int * _instance;
};


#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
#define _SCV_EXT_RW_FC_COMMON_SYSC_D           \
  virtual void assign(const sc_bv_base& v); \
  virtual void get_value(sc_bv_base& v) const; \
  virtual void assign(const sc_lv_base& v); \
  virtual void get_value(sc_lv_base& v) const;
#else
#define _SCV_EXT_RW_FC_COMMON_SYSC_D
#endif


// ----------------------------------------
// specialization for basic types
// ----------------------------------------

#define _SCV_EXT_RW_FC_D(basic_type,type_id) \
class _scv_extension_rw_ ## type_id \
 : public scv_extension_type<basic_type> { \
public: \
  _scv_extension_rw_ ## type_id(); \
  virtual ~_scv_extension_rw_ ## type_id(); \
  \
public: /* public API for use only when full type information is available */ \
  const basic_type* get_instance() const; \
  basic_type* get_instance(); \
  basic_type* _get_instance() const; \
  \
public: /* internal API for implementation only */ \
  void _set_instance(basic_type* p); \
  virtual void _set_instance_core_wrap(void* p); \
  void _set_as_field(_scv_extension_util_record * parent, basic_type* p, \
		     const std::string& name); \
  \
public: \
  virtual void assign(bool); \
  virtual void assign(char); \
  virtual void assign(unsigned char); \
  virtual void assign(short); \
  virtual void assign(unsigned short);\
  virtual void assign(int); \
  virtual void assign(unsigned); \
  virtual void assign(long); \
  virtual void assign(unsigned long); \
  virtual void assign(long long); \
  virtual void assign(unsigned long long); \
  virtual void assign(float); \
  virtual void assign(double); \
  virtual void assign(const std::string&); \
  virtual void assign(const char *); \
  \
  virtual bool get_bool() const; \
  virtual long long get_integer() const; \
  virtual unsigned long long get_unsigned() const; \
  virtual double get_double() const; \
  virtual std::string get_string() const; \
  \
  _SCV_EXT_RW_FC_COMMON_SYSC_D \
  const basic_type& read(); \
  void write(const basic_type& rhs); \
  \
public: \
  basic_type * _instance; \
}; \
\
template<> \
class scv_extension_rw<basic_type> \
  : public _scv_extension_rw_ ## type_id { \
public: \
  scv_extension_rw() {} \
  virtual ~scv_extension_rw() {} \
}; \


// ------------------------------------------------------------
// C/C++ Types
// ------------------------------------------------------------

// --------------
// integer types
// --------------

_SCV_EXT_RW_FC_D(bool,bool)
_SCV_EXT_RW_FC_D(char,char)
_SCV_EXT_RW_FC_D(unsigned char,unsigned_char)
_SCV_EXT_RW_FC_D(short,short)
_SCV_EXT_RW_FC_D(unsigned short,unsigned_short)
_SCV_EXT_RW_FC_D(int,int)
_SCV_EXT_RW_FC_D(unsigned int,unsigned_int)
_SCV_EXT_RW_FC_D(long,long)
_SCV_EXT_RW_FC_D(unsigned long,unsigned_long)
_SCV_EXT_RW_FC_D(long long,long_long)
_SCV_EXT_RW_FC_D(unsigned long long,unsigned_long_long)

// --------------
// floating pointer types
// --------------

_SCV_EXT_RW_FC_D(float,float)
_SCV_EXT_RW_FC_D(double,double)

// --------------
// string type
// --------------

_SCV_EXT_RW_FC_D(std::string,string)


// ------------------------------------------------------------
// SystemC Types
// ------------------------------------------------------------

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)

// --------------
// sc_bit
// --------------

_SCV_EXT_RW_FC_D(sc_bit,sc_bit)

// --------------
// sc_logic
// --------------

_SCV_EXT_RW_FC_D(sc_logic,sc_logic)

_SCV_EXT_RW_FC_D(sc_signed,sc_signed)
_SCV_EXT_RW_FC_D(sc_unsigned,sc_unsigned)
_SCV_EXT_RW_FC_D(sc_int_base,sc_int_base)
_SCV_EXT_RW_FC_D(sc_uint_base,sc_uint_base)
_SCV_EXT_RW_FC_D(sc_lv_base,sc_lv_base)
_SCV_EXT_RW_FC_D(sc_bv_base,sc_bv_base)


// --------------
// sc_int and sc_uint (begin)
// sc_bigint and sc_biguint (begin)
// sc_bv and sc_lv (begin)
// --------------


#define _SCV_EXT_RW_FC_N_BASE(T) \
public: /* public API for use only when full type information is available */ \
  const T* get_instance() const { return _instance; } \
  T* get_instance() { return _instance; } \
  T* _get_instance() const { return _instance; } \
  \
public: /* internal API for implementation only */ \
  void _set_instance(T* p) { _instance = p; _set_instance_core_wrap(p); } \
  virtual void _set_instance_core_wrap(void* p) {} \
  void _set_as_field(_scv_extension_util_record * parent, T* p,  \
		    const std::string& name) { \
    if (p) _set_instance(p); \
    else if ( ! this->_get_parent() ) { this->_set_parent(parent,name); parent->_add_field(this); } \
  } \
  \
public: \
  const T& read() { return *get_instance(); } \
  void write(const T& rhs) { *_get_instance() = rhs; this->trigger_value_change_cb(); } \
  \
public: \
  T * _instance; \


#define _SCV_EXT_RW_FC_N_ASSIGN(type_name,arg_name) \
  virtual void assign(arg_name i) { \
    *(this->_get_instance()) = i; \
    this->trigger_value_change_cb(); \
  }


#define _SCV_EXT_RW_FC_N_BAD_ASSIGN(type_name,arg_name) \
  virtual void assign(arg_name i) { \
    _SCV_RW_ERROR(assign,arg_name,type_name); \
  }


#define _SCV_EXT_RW_FC_N_ASSIGNS(type_name) \
  _SCV_EXT_RW_FC_N_ASSIGN(type_name,bool) \
  _SCV_EXT_RW_FC_N_ASSIGN(type_name,char) \
  _SCV_EXT_RW_FC_N_ASSIGN(type_name,unsigned char) \
  _SCV_EXT_RW_FC_N_ASSIGN(type_name,short) \
  _SCV_EXT_RW_FC_N_ASSIGN(type_name,unsigned short) \
  _SCV_EXT_RW_FC_N_ASSIGN(type_name,int) \
  _SCV_EXT_RW_FC_N_ASSIGN(type_name,unsigned int) \
  _SCV_EXT_RW_FC_N_ASSIGN(type_name,long) \
  _SCV_EXT_RW_FC_N_ASSIGN(type_name,unsigned long) \
  _SCV_EXT_RW_FC_N_ASSIGN(type_name,long long) \
  _SCV_EXT_RW_FC_N_ASSIGN(type_name,unsigned long long) \
  _SCV_EXT_RW_FC_N_BAD_ASSIGN(type_name,float) \
  _SCV_EXT_RW_FC_N_BAD_ASSIGN(type_name,double) \


#define _SCV_EXT_RW_FC_N_ASSIGNS_STRING(type_name) \
  virtual void assign(const std::string& s) { \
    *(this->get_instance()) = s.c_str();	 \
    this->trigger_value_change_cb();		 \
  }						 \
  virtual void assign(const char *s) {		 \
    *(this->get_instance()) = s;		 \
    this->trigger_value_change_cb();		 \
  }						 \


#define _SCV_EXT_RW_FC_N_ASSIGNS_GET(type_name) \
  virtual bool get_bool() const {		     \
    return *(this->_get_instance()) != (type_name)0; \
  }						     \
  virtual long long get_integer() const {	     \
    return this->_get_instance()->to_int64();	     \
  }						     \
  virtual unsigned long long get_unsigned() const { \
    return this->_get_instance()->to_uint64();	     \
  }						     \
  virtual double get_double() const  {		     \
    return this->_get_instance()->to_double();	     \
  }						     \
  virtual std::string get_string() const {	     \
    return this->get_instance()->to_string(); \
  }						     \


#define _SCV_EXT_RW_FC_N_ASSIGNS_SYSC(type_name) \
  virtual void assign(const sc_bv_base& v) { \
    if (this->get_bitwidth() != v.length()) \
      _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, \
			  "sc_bv_base","assign"); \
    *(this->_get_instance()) = v; this->trigger_value_change_cb(); \
  }								   \
  virtual void get_value(sc_bv_base& v) const { \
    if (this->get_bitwidth() != v.length()) \
      _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, \
			  "sc_bv_base","get_value"); \
    this->initialize(); v = *(this->_get_instance()); \
  }							           \
  virtual void assign(const sc_lv_base& v) { \
    if (this->get_bitwidth() != v.length()) \
      _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, \
			  "sc_lv_base","assign"); \
    *(this->_get_instance()) = v; this->trigger_value_change_cb(); \
  }							           \
  virtual void get_value(sc_lv_base& v) const { \
    if (this->get_bitwidth() != v.length()) \
      _scv_message::message(_scv_message::INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, \
			  "sc_lv_base","get_value"); \
    this->initialize(); v = *(this->_get_instance()); \
  }								   \


// --------------
// sc_int
// --------------
template<int N>
class scv_extension_rw<sc_int<N> >
  : public scv_extension_type<sc_int<N> > {
public:
  scv_extension_rw() {}
  virtual ~scv_extension_rw() {}
public:
  _SCV_EXT_RW_FC_N_BASE(sc_int<N>)
  _SCV_EXT_RW_FC_N_ASSIGNS(sc_int)
  _SCV_EXT_RW_FC_N_BAD_ASSIGN(sc_int,const std::string&)
  _SCV_EXT_RW_FC_N_BAD_ASSIGN(sc_int,const char *)
  _SCV_EXT_RW_FC_N_ASSIGNS_GET(sc_int<N>)
  _SCV_EXT_RW_FC_N_ASSIGNS_SYSC(sc_int)
};

// --------------
// sc_uint
// --------------
template<int N>
class scv_extension_rw<sc_uint<N> >
  : public scv_extension_type<sc_uint<N> > {
public:
  scv_extension_rw() {}
  virtual ~scv_extension_rw() {}
public:
  _SCV_EXT_RW_FC_N_BASE(sc_uint<N>)
  _SCV_EXT_RW_FC_N_ASSIGNS(sc_uint)
  _SCV_EXT_RW_FC_N_BAD_ASSIGN(sc_uint,const std::string&)
  _SCV_EXT_RW_FC_N_BAD_ASSIGN(sc_uint,const char *)
  _SCV_EXT_RW_FC_N_ASSIGNS_GET(sc_uint<N>)
  _SCV_EXT_RW_FC_N_ASSIGNS_SYSC(sc_uint)
};

// --------------
// sc_bigint
// --------------
template<int N>
class scv_extension_rw<sc_bigint<N> >
  : public scv_extension_type<sc_bigint<N> > {
public:
  scv_extension_rw() {}
  virtual ~scv_extension_rw() {}
public:
  _SCV_EXT_RW_FC_N_BASE(sc_bigint<N>)
  _SCV_EXT_RW_FC_N_ASSIGNS(sc_bigint)
  _SCV_EXT_RW_FC_N_ASSIGNS_STRING(sc_bigint)
  _SCV_EXT_RW_FC_N_ASSIGNS_GET(sc_bigint<N>)
  _SCV_EXT_RW_FC_N_ASSIGNS_SYSC(sc_bigint)
};

// --------------
// sc_biguint
// --------------
template<int N>
class scv_extension_rw<sc_biguint<N> >
  : public scv_extension_type<sc_biguint<N> > {
public:
  scv_extension_rw() {}
  virtual ~scv_extension_rw() {}
public:
  _SCV_EXT_RW_FC_N_BASE(sc_biguint<N>)
  _SCV_EXT_RW_FC_N_ASSIGNS(sc_biguint)
  _SCV_EXT_RW_FC_N_ASSIGNS_STRING(sc_biguint)
  _SCV_EXT_RW_FC_N_ASSIGNS_GET(sc_biguint<N>)
  _SCV_EXT_RW_FC_N_ASSIGNS_SYSC(sc_biguint)
};

// --------------
// sc_bv
// --------------
template<int N>
class scv_extension_rw<sc_bv<N> >
  : public scv_extension_type<sc_bv<N> > {
public:
  scv_extension_rw() {}
  virtual ~scv_extension_rw() {}
public:
  _SCV_EXT_RW_FC_N_BASE(sc_bv<N>)
  _SCV_EXT_RW_FC_N_ASSIGNS(sc_bv)
  _SCV_EXT_RW_FC_N_ASSIGNS_STRING(sc_bv)
  virtual bool get_bool() const {
    return *(this->_get_instance()) != 0;
  }
  virtual long long get_integer() const {
    static sc_bigint<N> tmp;
    tmp = *this->_get_instance();
    return tmp.to_int64();
  }
  virtual unsigned long long get_unsigned() const {
    static sc_bigint<N> tmp;
    tmp = *this->_get_instance();
    return tmp.to_uint64();
  }
  virtual double get_double() const {
    static sc_bigint<N> tmp;
    tmp = *this->_get_instance();
    return tmp.to_double();
  }
  virtual std::string get_string() const {
    return this->get_instance()->to_string();
  }
  _SCV_EXT_RW_FC_N_ASSIGNS_SYSC(sc_bv);
};

// --------------
// sc_lv
// --------------
template<int N>
class scv_extension_rw<sc_lv<N> >
  : public scv_extension_type<sc_lv<N> > {
public:
  scv_extension_rw() {}
  virtual ~scv_extension_rw() {}
public:
  _SCV_EXT_RW_FC_N_BASE(sc_lv<N>)
  _SCV_EXT_RW_FC_N_ASSIGNS(sc_lv)
  _SCV_EXT_RW_FC_N_ASSIGNS_STRING(sc_lv)
  virtual bool get_bool() const {
    return *(this->_get_instance()) != 0;
  }
  virtual long long get_integer() const {
    static sc_bigint<N> tmp;
    tmp = *this->_get_instance();
    return tmp.to_int64();
  }
  virtual unsigned long long get_unsigned() const {
    static sc_bigint<N> tmp;
    tmp = *this->_get_instance();
    return tmp.to_uint64();
  }
  virtual double get_double() const {
    static sc_bigint<N> tmp;
    tmp = *this->_get_instance();
    return tmp.to_double();
  }
  virtual std::string get_string() const {
    return this->get_instance()->to_string();
  }
  _SCV_EXT_RW_FC_N_ASSIGNS_SYSC(sc_lv);
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

#undef _SCV_INTROSPECTION_BASE
#define _SCV_INTROSPECTION_BASE scv_extension_rw<T>
#undef _SCV_INTROSPECTION_BASE1
#define _SCV_INTROSPECTION_BASE1 scv_extension_rw<T*>
#undef _SCV_INTROSPECTION_BASE2
#define _SCV_INTROSPECTION_BASE2 scv_extension_rw<T[N]>

#undef _SCV_INTROSPECTION_BASE_ENUM
#define _SCV_INTROSPECTION_BASE_ENUM _scv_extension_rw_enum
