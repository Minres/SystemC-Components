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

  _scv_introspection.h -- The main implementation for the introspection
  facility.

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

// ----------------------------------------
// final definition of the extension classes for builtin C/C++ types
// ----------------------------------------

#define _SCV_OSTREAM(type_name)                                      \
  friend std::ostream& operator<<(std::ostream& os,                  \
                                  const scv_extensions< type_name >& e) { \
    const_cast<scv_extensions< type_name >& >(e).initialize();       \
    os << *e._get_instance(); return os;                             \
  }

template<>
class scv_extensions<bool>
  : public scv_extensions_base<bool> {
public:
  scv_extensions<bool>& operator=(bool b)
    { *_get_instance() = b; trigger_value_change_cb(); return *this; }
  operator bool() const {
    const_cast<scv_extensions<bool> * >(this)->initialize();
    return *_get_instance();
  }
  _SCV_OSTREAM(bool)
  _SCV_PAREN_OPERATOR(bool)
};

#define _SCV_INTEGER_INTERFACE(type_name) \
public:                                                             \
  scv_extensions< type_name >& operator=(const scv_extensions<type_name>& i) { \
    *_get_instance() = *(i._get_instance()); trigger_value_change_cb(); return *this; \
  } \
  scv_extensions< type_name >& operator=(type_name i) { \
    *_get_instance() = i; trigger_value_change_cb(); return *this; \
  } \
  scv_extensions< type_name >& operator+=(type_name i) { \
    *_get_instance() += i; trigger_value_change_cb(); return *this; \
  } \
  scv_extensions< type_name >& operator-=(type_name i) { \
    *_get_instance() -= i; trigger_value_change_cb(); return *this; \
  } \
  scv_extensions< type_name >& operator*=(type_name i) { \
    *_get_instance() *= i; trigger_value_change_cb(); return *this; \
  } \
  scv_extensions< type_name >& operator/=(type_name i) { \
    *_get_instance() /= i; trigger_value_change_cb(); return *this; \
  } \
  scv_extensions< type_name >& operator%=(type_name i) { \
    *_get_instance() %= i; trigger_value_change_cb(); return *this; \
  } \
  scv_extensions< type_name >& operator&=(type_name i) { \
    *_get_instance() &= i; trigger_value_change_cb(); return *this; \
  } \
  scv_extensions< type_name >& operator|=(type_name i) { \
    *_get_instance() |= i; trigger_value_change_cb(); return *this; \
  } \
  scv_extensions< type_name >& operator^=(type_name i) { \
    *_get_instance() ^= i; trigger_value_change_cb(); return *this; \
  } \
  scv_extensions< type_name >& operator<<=(type_name i) { \
    *_get_instance() <<= i; trigger_value_change_cb(); return *this; \
  } \
  scv_extensions< type_name >& operator>>=(type_name i) { \
    *_get_instance() >>= i; trigger_value_change_cb(); return *this; \
  } \
  scv_extensions< type_name >& operator++() { \
    ++*_get_instance(); trigger_value_change_cb(); return *this; \
  } \
  type_name operator++(int) { \
    type_name tmp = *_get_instance(); \
    ++*_get_instance(); trigger_value_change_cb(); return tmp; \
  } \
  scv_extensions< type_name >& operator--() { \
    --*_get_instance(); trigger_value_change_cb(); return *this; \
  } \
  type_name operator--(int) { \
    type_name tmp = *_get_instance(); \
    --*_get_instance(); trigger_value_change_cb(); return tmp; \
  } \
  operator type_name() const { \
    const_cast<scv_extensions< type_name > * >(this)->initialize(); \
    return *_get_instance(); \
  } \
  _SCV_OSTREAM(type_name) \
  _SCV_PAREN_OPERATOR(type_name)

// for all C/C++ builtin integer types
#define _SCV_TAG_FINAL_COMPONENT(type_name) \
template<> \
class scv_extensions< type_name > \
  : public scv_extensions_base< type_name > { \
  _SCV_INTEGER_INTERFACE(type_name) \
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
#define _SCV_TAG_FINAL_COMPONENT(type_name) \
template<> \
class scv_extensions< type_name > \
  : public scv_extensions_base< type_name > { \
public:                                                            \
  scv_extensions< type_name >& operator=(const scv_extensions<type_name>& i) { \
    *_get_instance() = *(i._get_instance()); trigger_value_change_cb(); return *this; \
  }                                                                 \
  scv_extensions< type_name >& operator=(type_name i) { \
    *_get_instance() = i; trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator+=(type_name i) { \
    *_get_instance() += i; trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator-=(type_name i) { \
    *_get_instance() -= i; trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator*=(type_name i) { \
    *_get_instance() *= i; trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator/=(type_name i) { \
    *_get_instance() /= i; trigger_value_change_cb(); return *this; \
  }                                                                \
  operator type_name() const { \
    const_cast<scv_extensions< type_name > * >(this)->initialize(); \
    return *_get_instance(); \
  }                                                                \
  _SCV_OSTREAM(type_name) \
  _SCV_PAREN_OPERATOR(type_name) \
}

_SCV_TAG_FINAL_COMPONENT(float);
_SCV_TAG_FINAL_COMPONENT(double);

template<>
class scv_extensions<std::string>
  : public scv_extensions_base<std::string> {
public:
  scv_extensions<std::string>& operator=(const scv_extensions<std::string>& i) { \
    *_get_instance() = *(i._get_instance()); trigger_value_change_cb(); return *this; \
  }                                                                 \
  scv_extensions<std::string>& operator=(const std::string& s) {
    *_get_instance() = s; trigger_value_change_cb(); return *this;
  }
  scv_extensions<std::string>& operator=(const char * s) {
    *_get_instance() = s; trigger_value_change_cb(); return *this;
  }
  _SCV_OSTREAM(std::string);
};


#undef _SCV_TAG_FINAL_COMPONENT

#undef _SCV_INTEGER_INTERFACE

#define _SCV_INTEGER_INTERFACE(type_name) \
public:                                                            \
  scv_extensions< type_name >& operator=(const scv_extensions< type_name >& i) { \
    *this->_get_instance() = *(i._get_instance()); this->trigger_value_change_cb(); return *this; \
  }                                                                 \
  scv_extensions< type_name >& operator=(type_name i) { \
    *this->_get_instance() = i; this->trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator+=(type_name i) { \
    *this->_get_instance() += i; this->trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator-=(type_name i) { \
    *this->_get_instance() -= i; this->trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator*=(type_name i) { \
    *this->_get_instance() *= i; this->trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator/=(type_name i) { \
    *this->_get_instance() /= i; this->trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator%=(type_name i) { \
    *this->_get_instance() %= i; this->trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator&=(type_name i) { \
    *this->_get_instance() &= i; this->trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator|=(type_name i) { \
    *this->_get_instance() |= i; this->trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator^=(type_name i) { \
    *this->_get_instance() ^= i; this->trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator<<=(type_name i) { \
    *this->_get_instance() <<= i; this->trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator>>=(type_name i) { \
    *this->_get_instance() >>= i; this->trigger_value_change_cb(); return *this; \
  }                                                                \
  scv_extensions< type_name >& operator++() { \
    ++*this->_get_instance(); this->trigger_value_change_cb(); return *this; \
  }                                                                \
  type_name operator++(int) { \
    type_name tmp = *this->_get_instance(); \
    ++*this->_get_instance(); this->trigger_value_change_cb(); return tmp; \
  }                                                                \
  scv_extensions< type_name >& operator--() { \
    --*this->_get_instance(); this->trigger_value_change_cb(); return *this; \
  }                                                                \
  type_name operator--(int) { \
    type_name tmp = *this->_get_instance(); \
    --*this->_get_instance(); this->trigger_value_change_cb(); return tmp; \
  }                                                                \
  operator type_name() const { \
    const_cast<scv_extensions< type_name > * >(this)->initialize(); \
    return *this->_get_instance(); \
  }                                                                \
  _SCV_OSTREAM(type_name) \
  _SCV_PAREN_OPERATOR(type_name)


// for all SystemC templated types
#define _SCV_TAG_FINAL_COMPONENT(type_name) \
template<int N> \
class scv_extensions< type_name > \
  : public scv_extensions_base< type_name > { \
  _SCV_INTEGER_INTERFACE(type_name); \
}

#ifdef TEST_NEST_TEMPLATE
_SCV_TAG_FINAL_COMPONENT(test_uint<N> );
#endif

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)

// _SCV_TAG_FINAL_COMPONENT(sc_fixed);
// _SCV_TAG_FINAL_COMPONENT(sc_ufixed);

#undef _SCV_TAG_FINAL_COMPONENT

#define _SCV_IMPL { *this->_get_instance() = v; this->trigger_value_change_cb(); return *this; }
#define _SCV_IMPL1 { *this->_get_instance() = *(v._get_instance()); this->trigger_value_change_cb(); return *this; }
#define _SCV_IMPL2(op) { this->initialize(); *this->_get_instance() op v; this->trigger_value_change_cb(); return *this; }
#define _SCV_IMPL3(op) { u.initialize(); v.initialize(); return *u._get_instance() op *v._get_instance(); }
#define _SCV_IMPL4(op) { u.initialize(); return *u._get_instance() op v; }
#define _SCV_IMPL5(op) { v.initialize(); return u op *v._get_instance(); }
#define _SCV_MAP(return_type,method) return_type method() const { this->initialize(); return this->_get_instance()->method(); }

using namespace sc_dt;

#define _SCV_BASE_ASSIGN(src_type) \
  return_type& operator=(src_type i) \
    { *this->_get_instance() = i; this->trigger_value_change_cb(); return *this; } \

#define _SCV_SIGNED_SELFOP(op,src_type) \
  return_type& operator op (src_type i) \
    { this->initialize(); *this->_get_instance() += i; this->trigger_value_change_cb(); return *this; } \

#define _SCV_SIGNED_SELFOPS(op) \
  _SCV_SIGNED_SELFOP(op,const sc_signed&) \
  _SCV_SIGNED_SELFOP(op,const sc_unsigned&) \
  _SCV_SIGNED_SELFOP(op,int64) \
  _SCV_SIGNED_SELFOP(op,uint64) \
  _SCV_SIGNED_SELFOP(op,long) \
  _SCV_SIGNED_SELFOP(op,unsigned long) \
  _SCV_SIGNED_SELFOP(op,int) \
  _SCV_SIGNED_SELFOP(op,unsigned int) \
  _SCV_SIGNED_SELFOP(op,const sc_int_base&) \
  _SCV_SIGNED_SELFOP(op,const sc_uint_base&) \

#ifdef SC_INCLUDE_FX
# define _SCV_INT_FX_ASSIGN() \
    _SCV_BASE_ASSIGN(const sc_fxval&) \
    _SCV_BASE_ASSIGN(const sc_fxval_fast&) \
    _SCV_BASE_ASSIGN(const sc_fxnum&) \
    _SCV_BASE_ASSIGN(const sc_fxnum_fast&)
#else
# define _SCV_INT_FX_ASSIGN()
#endif

#ifdef SC_DT_DEPRECATED
# define _SCV_INT_DEPRECATED(type_name) \
    _SCV_MAP(int,to_signed) \
    _SCV_MAP(unsigned,to_unsigned)
#else
# define _SCV_INT_DEPRECATED(type_name)
#endif

#define _SCV_SIGNED_INTERFACE(type_name) \
public: \
  operator const type_name&() const { this->initialize(); return *this->_get_instance(); } \
  typedef scv_extensions< type_name > return_type; \
  return_type& operator=(const return_type& i) \
    { *this->_get_instance() = *(i._get_instance()); this->trigger_value_change_cb(); return *this; } \
  _SCV_BASE_ASSIGN(const sc_signed&) \
  _SCV_BASE_ASSIGN(const sc_signed_subref&) \
  _SCV_BASE_ASSIGN(const sc_unsigned&) \
  _SCV_BASE_ASSIGN(const sc_unsigned_subref&) \
  _SCV_BASE_ASSIGN(const char*) \
  _SCV_BASE_ASSIGN(int64) \
  _SCV_BASE_ASSIGN(uint64) \
  _SCV_BASE_ASSIGN(long) \
  _SCV_BASE_ASSIGN(unsigned long) \
  _SCV_BASE_ASSIGN(int) \
  _SCV_BASE_ASSIGN(unsigned int) \
  _SCV_BASE_ASSIGN(double) \
  _SCV_BASE_ASSIGN(const sc_int_base&) \
  _SCV_BASE_ASSIGN(const sc_uint_base&) \
  _SCV_BASE_ASSIGN(const sc_bv_base&) \
  _SCV_BASE_ASSIGN(const sc_lv_base&) \
  _SCV_INT_FX_ASSIGN() \
  return_type& operator++() \
    { ++*this->_get_instance(); this->trigger_value_change_cb(); return *this; } \
  type_name operator++(int) { \
    type_name tmp = *this->_get_instance(); \
    ++*this->_get_instance(); this->trigger_value_change_cb(); return tmp; \
  } \
  return_type& operator--() \
    { --*this->_get_instance(); this->trigger_value_change_cb(); return *this; } \
  type_name operator--(int) { \
    type_name tmp = *this->_get_instance(); \
    --*this->_get_instance(); this->trigger_value_change_cb(); return tmp; \
  } \
  _SCV_MAP(int,to_int) \
  _SCV_MAP(unsigned int,to_uint) \
  _SCV_MAP(long,to_long) \
  _SCV_MAP(unsigned long,to_ulong) \
  _SCV_MAP(int64,to_int64) \
  _SCV_MAP(uint64,to_uint64) \
  _SCV_MAP(double,to_double) \
  _SCV_INT_DEPRECATED(type_name) \
  const std::string to_string(sc_numrep numrep=SC_DEC) const \
    { this->initialize(); return this->_get_instance()->to_string(numrep); } \
  const std::string to_string(sc_numrep numrep, bool w_prefix) const \
    { this->initialize(); return this->_get_instance()->to_string(numrep,w_prefix); } \
  void scan( istream& is = cin ) \
    { this->_get_instance()->scan(is); this->trigger_value_change_cb(); } \
  void dump( ostream& os = cout ) const \
    { this->initialize(); this->_get_instance()->dump(os); } \
  _SCV_MAP(int,length) \
  _SCV_MAP(bool,iszero) \
  _SCV_MAP(bool,sign) \
  bool test(int i) const \
    { this->initialize(); return this->_get_instance()->test(i); } \
  void set(int i) \
    { this->initialize(); this->_get_instance()->set(i); this->trigger_value_change_cb(); } \
  void clear(int i) \
    { this->initialize(); this->_get_instance()->clear(i); this->trigger_value_change_cb(); } \
  void set(int i, bool v) \
    { this->initialize(); this->_get_instance()->set(i,v); this->trigger_value_change_cb(); } \
  void invert(int i) \
    { this->initialize(); this->_get_instance()->invert(i); this->trigger_value_change_cb(); } \
  void reverse() \
    { this->initialize(); this->_get_instance()->reverse(); this->trigger_value_change_cb(); } \
  void get_packed_rep(sc_dt::sc_digit *buf) const \
    { this->initialize(); this->_get_instance()->get_packed_rep(buf); } \
  void set_packed_rep(sc_dt::sc_digit *buf) \
    { this->initialize(); this->_get_instance()->set_packed_rep(buf); this->trigger_value_change_cb(); } \
  _SCV_SIGNED_SELFOPS(+=) \
  _SCV_SIGNED_SELFOPS(-=) \
  _SCV_SIGNED_SELFOPS(*=) \
  _SCV_SIGNED_SELFOPS(/=) \
  _SCV_SIGNED_SELFOPS(%=) \
  _SCV_SIGNED_SELFOPS(&=) \
  _SCV_SIGNED_SELFOPS(|=) \
  _SCV_SIGNED_SELFOPS(^=) \
  _SCV_SIGNED_SELFOPS(<<=) \
  _SCV_SIGNED_SELFOPS(>>=) \
  _SCV_OSTREAM(type_name) \
  _SCV_PAREN_OPERATOR(type_name)

#define _SCV_TAG_FINAL_COMPONENT(type_name) \
template<> \
class scv_extensions< type_name > \
  : public scv_extensions_base< type_name > { \
  _SCV_SIGNED_INTERFACE(type_name) \
}

_SCV_TAG_FINAL_COMPONENT(sc_signed);
_SCV_TAG_FINAL_COMPONENT(sc_unsigned);

#undef _SCV_TAG_FINAL_COMPONENT

#define _SCV_INT_BASE_SELFOPS(op) \
  _SCV_SIGNED_SELFOP(op,int64) \
  _SCV_SIGNED_SELFOP(op,uint64) \
  _SCV_SIGNED_SELFOP(op,long) \
  _SCV_SIGNED_SELFOP(op,unsigned long) \
  _SCV_SIGNED_SELFOP(op,int) \
  _SCV_SIGNED_SELFOP(op,unsigned int) \
  _SCV_SIGNED_SELFOP(op,const sc_int_base&) \
  _SCV_SIGNED_SELFOP(op,const sc_uint_base&) \

#define _SCV_INT_BASE_INTERFACE(type_name) \
public: \
  operator const type_name&() const { this->initialize(); return *this->_get_instance(); } \
  typedef scv_extensions< type_name > return_type; \
  void invalid_length() const { this->initialize(); this->invalid_length(); } \
  void invalid_index(int i) const { this->initialize(); this->invalid_index(i); } \
  void invalid_range(int l, int r) const { this->initialize(); this->invalid_range(l,r); } \
  void check_length() const { this->initialize(); this->check_length(); } \
  void check_index(int i) const { this->initialize(); this->check_index(i); } \
  void check_range(int l, int r) const { this->initialize(); this->check_range(l,r); } \
  void extend_sign() { this->initialize(); this->extend_sign(); } \
  return_type& operator=(const return_type& i) \
    { *this->_get_instance() = *(i._get_instance()); this->trigger_value_change_cb(); return *this; } \
  _SCV_BASE_ASSIGN(int_type) \
  _SCV_BASE_ASSIGN(const sc_int_base&) \
  _SCV_BASE_ASSIGN(const sc_int_subref&) \
  _SCV_BASE_ASSIGN(const sc_signed&) \
  _SCV_BASE_ASSIGN(const sc_unsigned&) \
  _SCV_INT_FX_ASSIGN() \
  _SCV_BASE_ASSIGN(const sc_bv_base&) \
  _SCV_BASE_ASSIGN(const sc_lv_base&) \
  _SCV_BASE_ASSIGN(const char*) \
  _SCV_BASE_ASSIGN(unsigned long) \
  _SCV_BASE_ASSIGN(long) \
  _SCV_BASE_ASSIGN(unsigned int) \
  _SCV_BASE_ASSIGN(int) \
  _SCV_BASE_ASSIGN(uint64) \
  _SCV_BASE_ASSIGN(double) \
  _SCV_SIGNED_SELFOP(+=,int_type) \
  _SCV_SIGNED_SELFOP(-=,int_type) \
  _SCV_SIGNED_SELFOP(*=,int_type) \
  _SCV_SIGNED_SELFOP(/=,int_type) \
  _SCV_SIGNED_SELFOP(%=,int_type) \
  _SCV_SIGNED_SELFOP(&=,int_type) \
  _SCV_SIGNED_SELFOP(|=,int_type) \
  _SCV_SIGNED_SELFOP(^=,int_type) \
  _SCV_SIGNED_SELFOP(<<=,int_type) \
  _SCV_SIGNED_SELFOP(>>=,int_type) \
  return_type& operator++() \
    { ++*this->_get_instance(); this->trigger_value_change_cb(); return *this; } \
  type_name operator++(int) { \
    type_name tmp = *this->_get_instance(); \
    ++*this->_get_instance(); this->trigger_value_change_cb(); return tmp; \
  } \
  return_type& operator--() \
    { --*this->_get_instance(); this->trigger_value_change_cb(); return *this; } \
  type_name operator--(int) { \
    type_name tmp = *this->_get_instance(); \
    --*this->_get_instance(); this->trigger_value_change_cb(); return tmp; \
  } \
  bool test(int i) const \
    { this->initialize(); return this->_get_instance()->test(i); } \
  void set(int i) \
    { this->initialize(); this->_get_instance()->set(i); this->trigger_value_change_cb(); } \
  void set(int i, bool v) \
    { this->initialize(); this->_get_instance()->set(i,v); this->trigger_value_change_cb(); } \
  _SCV_MAP(int,length) \
  _SCV_MAP(bool,and_reduce) \
  _SCV_MAP(bool,nand_reduce) \
  _SCV_MAP(bool,or_reduce) \
  _SCV_MAP(bool,nor_reduce) \
  _SCV_MAP(bool,xor_reduce) \
  _SCV_MAP(bool,xnor_reduce) \
  operator int_type() const { return this->value(); } \
  _SCV_MAP(int_type,value) \
  _SCV_MAP(int,to_int) \
  _SCV_MAP(unsigned int,to_uint) \
  _SCV_MAP(long,to_long) \
  _SCV_MAP(unsigned long,to_ulong) \
  _SCV_MAP(int64,to_int64) \
  _SCV_MAP(uint64,to_uint64) \
  _SCV_MAP(double,to_double) \
  const std::string to_string(sc_numrep numrep=SC_DEC) const \
    { this->initialize(); return this->_get_instance()->to_string(numrep); } \
  const std::string to_string(sc_numrep numrep, bool w_prefix) const \
    { this->initialize(); return this->_get_instance()->to_string(numrep,w_prefix); } \
  void scan( istream& is = cin ) \
    { this->_get_instance()->scan(is); this->trigger_value_change_cb(); } \
  _SCV_OSTREAM(type_name) \
  _SCV_PAREN_OPERATOR(type_name)

#define _SCV_TAG_FINAL_COMPONENT(type_name) \
template<> \
class scv_extensions< type_name > \
  : public scv_extensions_base< type_name > { \
  _SCV_INT_BASE_INTERFACE(type_name) \
}

_SCV_TAG_FINAL_COMPONENT(sc_int_base);
_SCV_TAG_FINAL_COMPONENT(sc_uint_base);

#undef _SCV_TAG_FINAL_COMPONENT

#define _SCV_BIT_BASE_INTERFACE(type_name) \
public: \
  operator const type_name&() const { this->initialize(); return *this->_get_instance(); } \
  typedef scv_extensions< type_name > return_type; \
  return_type& operator=(const return_type& i) \
    { *this->_get_instance() = *(i._get_instance()); this->trigger_value_change_cb(); return *this; } \
  _SCV_BASE_ASSIGN(const type_name&) \
  template <class X> \
  _SCV_BASE_ASSIGN(const sc_proxy<X>&) \
  _SCV_BASE_ASSIGN(const char*) \
  _SCV_BASE_ASSIGN(const bool*) \
  _SCV_BASE_ASSIGN(const sc_logic*) \
  _SCV_BASE_ASSIGN(const sc_unsigned&) \
  _SCV_BASE_ASSIGN(const sc_signed&) \
  _SCV_BASE_ASSIGN(const sc_uint_base&) \
  _SCV_BASE_ASSIGN(const sc_int_base&) \
  _SCV_BASE_ASSIGN(unsigned long) \
  _SCV_BASE_ASSIGN(long) \
  _SCV_BASE_ASSIGN(unsigned int) \
  _SCV_BASE_ASSIGN(int) \
  _SCV_BASE_ASSIGN(uint64) \
  _SCV_BASE_ASSIGN(int64) \
  _SCV_MAP(int,length) \
  _SCV_MAP(int,size) \
  sc_logic_value_t get_bit(int i) const \
    { this->initialize(); return sc_dt::sc_logic_value_t(this->_get_instance()->get_bit(i)); } \
  void set_bit(int i, sc_logic_value_t v) \
    { this->initialize(); this->_get_instance()->set_bit(i,v); this->trigger_value_change_cb(); } \
  unsigned long get_word(int i) const \
    { this->initialize(); return this->_get_instance()->get_word(i); } \
  void set_word(int i, unsigned long w) \
    { this->initialize(); this->_get_instance()->set_word(i,w); this->trigger_value_change_cb(); } \
  unsigned long get_cword(int i) const \
    { this->initialize(); return this->_get_instance()->get_cword(i); } \
  void set_cword(int i, unsigned long w) \
    { this->initialize(); this->_get_instance()->set_cword(i,w); this->trigger_value_change_cb(); } \
  void clean_tail() \
    { this->initialize(); this->_get_instance()->clean_tail(); this->trigger_value_change_cb(); } \
  _SCV_MAP(bool,is_01) \
  _SCV_OSTREAM(type_name) \
  _SCV_PAREN_OPERATOR(type_name)

#define _SCV_TAG_FINAL_COMPONENT(type_name) \
template<> \
class scv_extensions< type_name > \
  : public scv_extensions_base< type_name > { \
  _SCV_BIT_BASE_INTERFACE(type_name) \
}

_SCV_TAG_FINAL_COMPONENT(sc_lv_base);
_SCV_TAG_FINAL_COMPONENT(sc_bv_base);

#undef _SCV_TAG_FINAL_COMPONENT


// sc_uint and sc_int are exactly the same as
template<int W>
class scv_extensions< sc_uint<W> >
  : public scv_extensions_base< sc_uint<W> > {
public:
  _SCV_PAREN_OPERATOR(sc_uint<W>)

  typedef scv_extensions< sc_uint<W> > return_type;

  return_type& operator=(const return_type& v) _SCV_IMPL1
  // from class sc_uint
#ifndef _SCV_INTROSPECTION_ONLY
  return_type& operator = (uint_type                    v) _SCV_IMPL
#endif
  return_type& operator = (const sc_uint_base&          v) _SCV_IMPL
#ifndef _SCV_INTROSPECTION_ONLY
  return_type& operator = (const sc_uint_subref&        v) _SCV_IMPL
#endif
  return_type& operator = (const sc_signed&             v) _SCV_IMPL
  return_type& operator = (const sc_unsigned&           v) _SCV_IMPL
#ifdef SC_INCLUDE_FX
  return_type& operator = ( const sc_fxval&             v) _SCV_IMPL
  return_type& operator = ( const sc_fxval_fast&        v) _SCV_IMPL
  return_type& operator = ( const sc_fxnum&             v) _SCV_IMPL
  return_type& operator = ( const sc_fxnum_fast&        v) _SCV_IMPL
#endif
  return_type& operator = ( const sc_bv_base&           v) _SCV_IMPL
  return_type& operator = ( const sc_lv_base&           v) _SCV_IMPL
#ifndef _SCV_INTROSPECTION_ONLY
  return_type& operator += (uint_type v) _SCV_IMPL2(+=)
  return_type& operator -= (uint_type v) _SCV_IMPL2(-=)
  return_type& operator *= (uint_type v) _SCV_IMPL2(*=)
  return_type& operator /= (uint_type v) _SCV_IMPL2(/=)
  return_type& operator %= (uint_type v) _SCV_IMPL2(%=)
  return_type& operator &= (uint_type v) _SCV_IMPL2(&=)
  return_type& operator |= (uint_type v) _SCV_IMPL2(|=)
  return_type& operator ^= (uint_type v) _SCV_IMPL2(^=)
  return_type& operator <<= (uint_type v) _SCV_IMPL2(<<=)
  return_type& operator >>= (uint_type v) _SCV_IMPL2(>>=)
#endif

  return_type& operator ++ () // prefix
  { this->initialize(); ++*this->_get_instance(); this->trigger_value_change_cb(); return *this; }
  const return_type operator ++ (int) // postfix
  { this->initialize(); sc_uint<W> tmp = *this->_get_instance()++; this->trigger_value_change_cb(); return tmp; }
  return_type& operator -- () // prefix
  { this->initialize(); --*this->_get_instance(); this->trigger_value_change_cb(); return *this; }
  const return_type operator -- (int) // postfix
  { this->initialize(); sc_uint<W> tmp = *this->_get_instance()--; this->trigger_value_change_cb(); return tmp; }

  // from class sc_uint_base
#ifndef _SCV_INTROSPECTION_ONLY
  operator uint_type() const { this->initialize(); return this->_get_instance()->operator uint_type(); }
#endif

  _SCV_MAP(int,bitwidth);
  _SCV_MAP(int,length);
  _SCV_MAP(unsigned int,to_uint);
  _SCV_MAP(int,to_int);
  _SCV_MAP(uint64,to_uint64);
  _SCV_MAP(int64,to_int64);
#ifndef _32BIT_
  _SCV_MAP(long,long_low);
  _SCV_MAP(long,long_high);
#endif
  bool test(int i) const { this->initialize(); return this->_get_instance()->test(i); }
  void set(int i) { this->initialize(); this->_get_instance()->set(i); this->trigger_value_change_cb(); }
  void set(int i, bool v) { this->initialize(); this->_get_instance()->set(i,v); this->trigger_value_change_cb(); }
  // sc_uint_bitref operator [] (int i)
  bool operator [] (int i) const { this->initialize(); return this->_get_instance()->operator [](i); }
  //  sc_uint_subref range(int left, int right);
#ifndef _SCV_INTROSPECTION_ONLY
  uint_type range(int left, int right) const { this->initialize(); return this->_get_instance()->range(left,right); }
#endif

  // operator ==, !=, <, <=, >, >= should be handled by uint_type();
  // operator +, -, etc. as well.

  //  void print( ostream& os ) const { this->initialize(); this->_get_instance()->print(os); }
};

template<int W>
class scv_extensions< sc_int<W> >
  : public scv_extensions_base< sc_int<W> > {
public:
  _SCV_PAREN_OPERATOR(sc_int<W>)

  typedef scv_extensions< sc_int<W> > return_type;

  return_type& operator=(const return_type& v) _SCV_IMPL1
  // from class sc_int
#ifndef _SCV_INTROSPECTION_ONLY
  return_type& operator = (int_type                     v) _SCV_IMPL
#endif
  return_type& operator = (const sc_int_base&           v) _SCV_IMPL
#ifndef _SCV_INTROSPECTION_ONLY
  return_type& operator = (const sc_int_subref&         v) _SCV_IMPL
#endif
  return_type& operator = (const sc_signed&             v) _SCV_IMPL
  return_type& operator = (const sc_unsigned&           v) _SCV_IMPL
#ifdef SC_INCLUDE_FX
  return_type& operator = ( const sc_fxval&             v) _SCV_IMPL
  return_type& operator = ( const sc_fxval_fast&        v) _SCV_IMPL
  return_type& operator = ( const sc_fxnum&             v) _SCV_IMPL
  return_type& operator = ( const sc_fxnum_fast&        v) _SCV_IMPL
#endif
  return_type& operator = ( const sc_bv_base&           v) _SCV_IMPL
  return_type& operator = ( const sc_lv_base&           v) _SCV_IMPL
#ifndef _SCV_INTROSPECTION_ONLY
  return_type& operator += (int_type v) _SCV_IMPL2(+=)
  return_type& operator -= (int_type v) _SCV_IMPL2(-=)
  return_type& operator *= (int_type v) _SCV_IMPL2(*=)
  return_type& operator /= (int_type v) _SCV_IMPL2(/=)
  return_type& operator %= (int_type v) _SCV_IMPL2(%=)
  return_type& operator &= (int_type v) _SCV_IMPL2(&=)
  return_type& operator |= (int_type v) _SCV_IMPL2(|=)
  return_type& operator ^= (int_type v) _SCV_IMPL2(^=)
  return_type& operator <<= (int_type v) _SCV_IMPL2(<<=)
  return_type& operator >>= (int_type v) _SCV_IMPL2(>>=)
#endif

  return_type& operator ++ () // prefix
  { this->initialize(); ++*this->_get_instance(); this->trigger_value_change_cb(); return *this; }
  const return_type operator ++ (int) // postfix
  { this->initialize(); sc_int<W> tmp = *this->_get_instance()++; this->trigger_value_change_cb(); return tmp; }
  return_type& operator -- () // prefix
  { this->initialize(); --*this->_get_instance(); this->trigger_value_change_cb(); return *this; }
  const return_type operator -- (int) // postfix
  { this->initialize(); sc_int<W> tmp = *this->_get_instance()--; this->trigger_value_change_cb(); return tmp; }

  // from class sc_int_base
#ifndef _SCV_INTROSPECTION_ONLY
  operator int_type() const { this->initialize(); return this->_get_instance()->operator int_type(); }
#endif

  _SCV_MAP(int,bitwidth);
  _SCV_MAP(int,length);
  _SCV_MAP(unsigned int,to_uint);
  _SCV_MAP(int,to_int);
  _SCV_MAP(uint64,to_uint64);
  _SCV_MAP(int64,to_int64);
#ifndef _32BIT_
  _SCV_MAP(long,long_low);
  _SCV_MAP(long,long_high);
#endif
  bool test(int i) const { this->initialize(); return this->_get_instance()->test(i); }
  void set(int i) { this->initialize(); this->_get_instance()->set(i); this->trigger_value_change_cb(); }
  void set(int i, bool v) { this->initialize(); this->_get_instance()->set(i,v); this->trigger_value_change_cb(); }
  // sc_int_bitref operator [] (int i)
  bool operator [] (int i) const { this->initialize(); return this->_get_instance()->operator [](i); }
  //  sc_int_subref range(int left, int right);
#ifndef _SCV_INTROSPECTION_ONLY
  int_type range(int left, int right) const { this->initialize(); return this->_get_instance()->range(left,right); }
#endif

  // operator ==, !=, <, <=, >, >= should be handled by int_type();
  // operator +, -, etc. as well.

  //  void print( ostream& os ) const { this->initialize(); this->_get_instance()->print(os); }
};

// sc_biguint and sc_bigint are exactly the same.
// need to add &=, etc.
template<int W>
class scv_extensions< sc_biguint<W> >
  : public scv_extensions_base< sc_biguint<W> > {
public:
  _SCV_PAREN_OPERATOR(sc_biguint<W>)

  typedef scv_extensions< sc_biguint<W> > return_type;

  return_type& operator=(const return_type& v) _SCV_IMPL1
  return_type& operator=(const sc_biguint<W>&      v) _SCV_IMPL
  return_type& operator=(const sc_unsigned&        v) _SCV_IMPL
#ifndef _SCV_INTROSPECTION_ONLY
  return_type& operator=(const sc_unsigned_subref& v) _SCV_IMPL
#endif
  return_type& operator=(const sc_signed&          v) _SCV_IMPL
#ifndef _SCV_INTROSPECTION_ONLY
  return_type& operator=(const sc_signed_subref&   v) _SCV_IMPL
#endif
  return_type& operator=(const char*               v) _SCV_IMPL
  return_type& operator=(int64                     v) _SCV_IMPL
  return_type& operator=(uint64                    v) _SCV_IMPL
  return_type& operator=(long                      v) _SCV_IMPL
  return_type& operator=(unsigned long             v) _SCV_IMPL
  return_type& operator=(int                       v) _SCV_IMPL
  return_type& operator=(unsigned int              v) _SCV_IMPL
  return_type& operator=(double                    v) _SCV_IMPL
  return_type& operator=( const sc_bv_base&        v) _SCV_IMPL
  return_type& operator=( const sc_lv_base&        v) _SCV_IMPL
  return_type& operator=( const sc_int_base&       v) _SCV_IMPL
  return_type& operator=( const sc_uint_base&      v) _SCV_IMPL
#ifdef SC_INCLUDE_FX
  return_type& operator = ( const sc_fxval& v ) _SCV_IMPL
  return_type& operator = ( const sc_fxval_fast& v ) _SCV_IMPL
  return_type& operator = ( const sc_fxnum& v ) _SCV_IMPL
  return_type& operator = ( const sc_fxnum_fast& v ) _SCV_IMPL
#endif
  return_type& operator += (const sc_signed&    v) _SCV_IMPL2(+=)
  return_type& operator += (const sc_unsigned&  v) _SCV_IMPL2(+=)
  return_type& operator += (int64               v) _SCV_IMPL2(+=)
  return_type& operator += (uint64              v) _SCV_IMPL2(+=)
  return_type& operator += (long                v) _SCV_IMPL2(+=)
  return_type& operator += (unsigned long       v) _SCV_IMPL2(+=)
  return_type& operator += (int                 v) _SCV_IMPL2(+=)
  return_type& operator += (unsigned int        v) _SCV_IMPL2(+=)
  return_type& operator += (const sc_int_base&  v) _SCV_IMPL2(+=)
  return_type& operator += (const sc_uint_base& v) _SCV_IMPL2(+=)

  return_type& operator -= (const sc_signed&    v) _SCV_IMPL2(-=)
  return_type& operator -= (const sc_unsigned&  v) _SCV_IMPL2(-=)
  return_type& operator -= (int64               v) _SCV_IMPL2(-=)
  return_type& operator -= (uint64              v) _SCV_IMPL2(-=)
  return_type& operator -= (long                v) _SCV_IMPL2(-=)
  return_type& operator -= (unsigned long       v) _SCV_IMPL2(-=)
  return_type& operator -= (int                 v) _SCV_IMPL2(-=)
  return_type& operator -= (unsigned int        v) _SCV_IMPL2(-=)
  return_type& operator -= (const sc_int_base&  v) _SCV_IMPL2(-=)
  return_type& operator -= (const sc_uint_base& v) _SCV_IMPL2(-=)

  return_type& operator *= (const sc_signed&    v) _SCV_IMPL2(*=)
  return_type& operator *= (const sc_unsigned&  v) _SCV_IMPL2(*=)
  return_type& operator *= (int64               v) _SCV_IMPL2(*=)
  return_type& operator *= (uint64              v) _SCV_IMPL2(*=)
  return_type& operator *= (long                v) _SCV_IMPL2(*=)
  return_type& operator *= (unsigned long       v) _SCV_IMPL2(*=)
  return_type& operator *= (int                 v) _SCV_IMPL2(*=)
  return_type& operator *= (unsigned int        v) _SCV_IMPL2(*=)
  return_type& operator *= (const sc_int_base&  v) _SCV_IMPL2(*=)
  return_type& operator *= (const sc_uint_base& v) _SCV_IMPL2(*=)

  return_type& operator /= (const sc_signed&    v) _SCV_IMPL2(/=)
  return_type& operator /= (const sc_unsigned&  v) _SCV_IMPL2(/=)
  return_type& operator /= (int64               v) _SCV_IMPL2(/=)
  return_type& operator /= (uint64              v) _SCV_IMPL2(/=)
  return_type& operator /= (long                v) _SCV_IMPL2(/=)
  return_type& operator /= (unsigned long       v) _SCV_IMPL2(/=)
  return_type& operator /= (int                 v) _SCV_IMPL2(/=)
  return_type& operator /= (unsigned int        v) _SCV_IMPL2(/=)
  return_type& operator /= (const sc_int_base&  v) _SCV_IMPL2(/=)
  return_type& operator /= (const sc_uint_base& v) _SCV_IMPL2(/=)

  return_type& operator %= (const sc_signed&    v) _SCV_IMPL2(%=)
  return_type& operator %= (const sc_unsigned&  v) _SCV_IMPL2(%=)
  return_type& operator %= (int64               v) _SCV_IMPL2(%=)
  return_type& operator %= (uint64              v) _SCV_IMPL2(%=)
  return_type& operator %= (long                v) _SCV_IMPL2(%=)
  return_type& operator %= (unsigned long       v) _SCV_IMPL2(%=)
  return_type& operator %= (int                 v) _SCV_IMPL2(%=)
  return_type& operator %= (unsigned int        v) _SCV_IMPL2(%=)
  return_type& operator %= (const sc_int_base&  v) _SCV_IMPL2(%=)
  return_type& operator %= (const sc_uint_base& v) _SCV_IMPL2(%=)

  return_type& operator &= (const sc_signed&    v) _SCV_IMPL2(&=)
  return_type& operator &= (const sc_unsigned&  v) _SCV_IMPL2(&=)
  return_type& operator &= (int64               v) _SCV_IMPL2(&=)
  return_type& operator &= (uint64              v) _SCV_IMPL2(&=)
  return_type& operator &= (long                v) _SCV_IMPL2(&=)
  return_type& operator &= (unsigned long       v) _SCV_IMPL2(&=)
  return_type& operator &= (int                 v) _SCV_IMPL2(&=)
  return_type& operator &= (unsigned int        v) _SCV_IMPL2(&=)
  return_type& operator &= (const sc_int_base&  v) _SCV_IMPL2(&=)
  return_type& operator &= (const sc_uint_base& v) _SCV_IMPL2(&=)

  return_type& operator |= (const sc_signed&    v) _SCV_IMPL2(|=)
  return_type& operator |= (const sc_unsigned&  v) _SCV_IMPL2(|=)
  return_type& operator |= (int64               v) _SCV_IMPL2(|=)
  return_type& operator |= (uint64              v) _SCV_IMPL2(|=)
  return_type& operator |= (long                v) _SCV_IMPL2(|=)
  return_type& operator |= (unsigned long       v) _SCV_IMPL2(|=)
  return_type& operator |= (int                 v) _SCV_IMPL2(|=)
  return_type& operator |= (unsigned int        v) _SCV_IMPL2(|=)
  return_type& operator |= (const sc_int_base&  v) _SCV_IMPL2(|=)
  return_type& operator |= (const sc_uint_base& v) _SCV_IMPL2(|=)

  return_type& operator ^= (const sc_signed&    v) _SCV_IMPL2(^=)
  return_type& operator ^= (const sc_unsigned&  v) _SCV_IMPL2(^=)
  return_type& operator ^= (int64               v) _SCV_IMPL2(^=)
  return_type& operator ^= (uint64              v) _SCV_IMPL2(^=)
  return_type& operator ^= (long                v) _SCV_IMPL2(^=)
  return_type& operator ^= (unsigned long       v) _SCV_IMPL2(^=)
  return_type& operator ^= (int                 v) _SCV_IMPL2(^=)
  return_type& operator ^= (unsigned int        v) _SCV_IMPL2(^=)
  return_type& operator ^= (const sc_int_base&  v) _SCV_IMPL2(^=)
  return_type& operator ^= (const sc_uint_base& v) _SCV_IMPL2(^=)

  return_type& operator <<= (const sc_signed&    v) _SCV_IMPL2(<<=)
  return_type& operator <<= (const sc_unsigned&  v) _SCV_IMPL2(<<=)
  return_type& operator <<= (int64               v) _SCV_IMPL2(<<=)
  return_type& operator <<= (uint64              v) _SCV_IMPL2(<<=)
  return_type& operator <<= (long                v) _SCV_IMPL2(<<=)
  return_type& operator <<= (unsigned long       v) _SCV_IMPL2(<<=)
  return_type& operator <<= (int                 v) _SCV_IMPL2(<<=)
  return_type& operator <<= (unsigned int        v) _SCV_IMPL2(<<=)
  return_type& operator <<= (const sc_int_base&  v) _SCV_IMPL2(<<=)
  return_type& operator <<= (const sc_uint_base& v) _SCV_IMPL2(<<=)

  return_type& operator >>= (const sc_signed&    v) _SCV_IMPL2(>>=)
  return_type& operator >>= (const sc_unsigned&  v) _SCV_IMPL2(>>=)
  return_type& operator >>= (int64               v) _SCV_IMPL2(>>=)
  return_type& operator >>= (uint64              v) _SCV_IMPL2(>>=)
  return_type& operator >>= (long                v) _SCV_IMPL2(>>=)
  return_type& operator >>= (unsigned long       v) _SCV_IMPL2(>>=)
  return_type& operator >>= (int                 v) _SCV_IMPL2(>>=)
  return_type& operator >>= (unsigned int        v) _SCV_IMPL2(>>=)
  return_type& operator >>= (const sc_int_base&  v) _SCV_IMPL2(>>=)
  return_type& operator >>= (const sc_uint_base& v) _SCV_IMPL2(>>=)

  return_type& operator ++ ()
  { this->initialize(); ++*this->_get_instance(); this->trigger_value_change_cb(); return *this; }
  const sc_unsigned operator ++ (int)
  { this->initialize(); sc_biguint<W> tmp = *this->_get_instance()++; this->trigger_value_change_cb(); return tmp; }
  return_type& operator -- ()
  { this->initialize(); --*this->_get_instance(); this->trigger_value_change_cb(); return *this; }
  const sc_unsigned operator -- (int)
  { this->initialize(); sc_biguint<W> tmp = *this->_get_instance()--; this->trigger_value_change_cb(); return tmp; }
  //  sc_unsigned_bitref operator [] (int i)
  const bool operator [] (int i) const
  { this->initialize(); return this->_get_instance()->operator [](i); }
  const sc_unsigned range(int i, int j) const
  { this->initialize(); return this->_get_instance()->range(i,j); }
  //  sc_unsigned_subref operator () (int i, int j)
  const sc_unsigned operator () (int i, int j) const
  { this->initialize(); return this->_get_instance()->operator ()(i,j); }

  std::string to_string(sc_numrep base = SC_DEC, bool formatted = false) const
  { this->initialize(); return this->_get_instance()->to_string(base,formatted); }
  std::string to_string(int base, bool formatted = false) const
  { this->initialize(); return this->_get_instance()->to_string(base,formatted); }

  _SCV_MAP(int64,to_int64);
  _SCV_MAP(uint64,to_uint64);
  _SCV_MAP(long,to_long);
  _SCV_MAP(unsigned long,to_ulong);
  _SCV_MAP(unsigned long,to_unsigned_long);
  _SCV_MAP(int,to_int);
  _SCV_MAP(int,to_signed);
  _SCV_MAP(unsigned int,to_uint);
  _SCV_MAP(unsigned int,to_unsigned);
  _SCV_MAP(unsigned int,to_unsigned_int);
  _SCV_MAP(double,to_double);
  //  void print() const { this->initialize(); this->_get_instance()->print(); }
  //  void print(ostream &os) const { this->initialize(); this->_get_instance()->print(os); }
  void dump() const { this->initialize(); this->_get_instance()->dump(); };
  void dump(ostream &os) const { this->initialize(); this->_get_instance()->dump(os); };
  _SCV_MAP(int,length);
  _SCV_MAP(bool,iszero);
  _SCV_MAP(bool,sign);
  bool test(int i) const { this->initialize(); return this->_get_instance()->test(i); }
  void set(int i) { this->initialize(); this->_get_instance()->set(i); this->trigger_value_change_cb(); }
  void clear(int i) { this->initialize(); this->_get_instance()->clear(i); this->trigger_value_change_cb(); }
  void set(int i, bool v) { this->initialize(); this->_get_instance()->set(i,v); this->trigger_value_change_cb(); }
  void invert(int i) { this->initialize(); this->_get_instance()->invert(i); this->trigger_value_change_cb(); }
  void reverse() { this->initialize(); this->_get_instance()->reverse(); this->trigger_value_change_cb(); }
  void get_packed_rep(sc_dt::sc_digit *buf) const { this->initialize(); this->_get_instance()->get_packet_ref(buf); }
  void set_packed_rep(sc_dt::sc_digit *buf) { this->_get_instance()->get_packet_ref(buf); this->trigger_value_change_cb(); }

  operator const sc_unsigned&() const { this->initialize(); return *this->_get_instance(); }
};

template<int W>
class scv_extensions< sc_bigint<W> >
  : public scv_extensions_base< sc_bigint<W> > {
public:
  _SCV_PAREN_OPERATOR(sc_bigint<W>)

  typedef scv_extensions< sc_bigint<W> > return_type;

  return_type& operator=(const return_type& v) _SCV_IMPL1
  return_type& operator=(const sc_bigint<W>&       v) _SCV_IMPL
  return_type& operator=(const sc_unsigned&        v) _SCV_IMPL
#ifndef _SCV_INTROSPECTION_ONLY
  return_type& operator=(const sc_unsigned_subref& v) _SCV_IMPL
#endif
  return_type& operator=(const sc_signed&          v) _SCV_IMPL
#ifndef _SCV_INTROSPECTION_ONLY
  return_type& operator=(const sc_signed_subref&   v) _SCV_IMPL
#endif
  return_type& operator=(const char*               v) _SCV_IMPL
  return_type& operator=(int64                     v) _SCV_IMPL
  return_type& operator=(uint64                    v) _SCV_IMPL
  return_type& operator=(long                      v) _SCV_IMPL
  return_type& operator=(unsigned long             v) _SCV_IMPL
  return_type& operator=(int                       v) _SCV_IMPL
  return_type& operator=(unsigned int              v) _SCV_IMPL
  return_type& operator=(double                    v) _SCV_IMPL
  return_type& operator=( const sc_bv_base&        v) _SCV_IMPL
  return_type& operator=( const sc_lv_base&        v) _SCV_IMPL
  return_type& operator=( const sc_int_base&       v) _SCV_IMPL
  return_type& operator=( const sc_uint_base&      v) _SCV_IMPL
#ifdef SC_INCLUDE_FX
  return_type& operator = ( const sc_fxval& v ) _SCV_IMPL
  return_type& operator = ( const sc_fxval_fast& v ) _SCV_IMPL
  return_type& operator = ( const sc_fxnum& v ) _SCV_IMPL
  return_type& operator = ( const sc_fxnum_fast& v ) _SCV_IMPL
#endif

  return_type& operator += (const sc_signed&    v) _SCV_IMPL2(+=)
  return_type& operator += (const sc_unsigned&  v) _SCV_IMPL2(+=)
  return_type& operator += (int64               v) _SCV_IMPL2(+=)
  return_type& operator += (uint64              v) _SCV_IMPL2(+=)
  return_type& operator += (long                v) _SCV_IMPL2(+=)
  return_type& operator += (unsigned long       v) _SCV_IMPL2(+=)
  return_type& operator += (int                 v) _SCV_IMPL2(+=)
  return_type& operator += (unsigned int        v) _SCV_IMPL2(+=)
  return_type& operator += (const sc_int_base&  v) _SCV_IMPL2(+=)
  return_type& operator += (const sc_uint_base& v) _SCV_IMPL2(+=)

  return_type& operator -= (const sc_signed&    v) _SCV_IMPL2(-=)
  return_type& operator -= (const sc_unsigned&  v) _SCV_IMPL2(-=)
  return_type& operator -= (int64               v) _SCV_IMPL2(-=)
  return_type& operator -= (uint64              v) _SCV_IMPL2(-=)
  return_type& operator -= (long                v) _SCV_IMPL2(-=)
  return_type& operator -= (unsigned long       v) _SCV_IMPL2(-=)
  return_type& operator -= (int                 v) _SCV_IMPL2(-=)
  return_type& operator -= (unsigned int        v) _SCV_IMPL2(-=)
  return_type& operator -= (const sc_int_base&  v) _SCV_IMPL2(-=)
  return_type& operator -= (const sc_uint_base& v) _SCV_IMPL2(-=)

  return_type& operator *= (const sc_signed&    v) _SCV_IMPL2(*=)
  return_type& operator *= (const sc_unsigned&  v) _SCV_IMPL2(*=)
  return_type& operator *= (int64               v) _SCV_IMPL2(*=)
  return_type& operator *= (uint64              v) _SCV_IMPL2(*=)
  return_type& operator *= (long                v) _SCV_IMPL2(*=)
  return_type& operator *= (unsigned long       v) _SCV_IMPL2(*=)
  return_type& operator *= (int                 v) _SCV_IMPL2(*=)
  return_type& operator *= (unsigned int        v) _SCV_IMPL2(*=)
  return_type& operator *= (const sc_int_base&  v) _SCV_IMPL2(*=)
  return_type& operator *= (const sc_uint_base& v) _SCV_IMPL2(*=)

  return_type& operator /= (const sc_signed&    v) _SCV_IMPL2(/=)
  return_type& operator /= (const sc_unsigned&  v) _SCV_IMPL2(/=)
  return_type& operator /= (int64               v) _SCV_IMPL2(/=)
  return_type& operator /= (uint64              v) _SCV_IMPL2(/=)
  return_type& operator /= (long                v) _SCV_IMPL2(/=)
  return_type& operator /= (unsigned long       v) _SCV_IMPL2(/=)
  return_type& operator /= (int                 v) _SCV_IMPL2(/=)
  return_type& operator /= (unsigned int        v) _SCV_IMPL2(/=)
  return_type& operator /= (const sc_int_base&  v) _SCV_IMPL2(/=)
  return_type& operator /= (const sc_uint_base& v) _SCV_IMPL2(/=)

  return_type& operator %= (const sc_signed&    v) _SCV_IMPL2(%=)
  return_type& operator %= (const sc_unsigned&  v) _SCV_IMPL2(%=)
  return_type& operator %= (int64               v) _SCV_IMPL2(%=)
  return_type& operator %= (uint64              v) _SCV_IMPL2(%=)
  return_type& operator %= (long                v) _SCV_IMPL2(%=)
  return_type& operator %= (unsigned long       v) _SCV_IMPL2(%=)
  return_type& operator %= (int                 v) _SCV_IMPL2(%=)
  return_type& operator %= (unsigned int        v) _SCV_IMPL2(%=)
  return_type& operator %= (const sc_int_base&  v) _SCV_IMPL2(%=)
  return_type& operator %= (const sc_uint_base& v) _SCV_IMPL2(%=)

  return_type& operator &= (const sc_signed&    v) _SCV_IMPL2(&=)
  return_type& operator &= (const sc_unsigned&  v) _SCV_IMPL2(&=)
  return_type& operator &= (int64               v) _SCV_IMPL2(&=)
  return_type& operator &= (uint64              v) _SCV_IMPL2(&=)
  return_type& operator &= (long                v) _SCV_IMPL2(&=)
  return_type& operator &= (unsigned long       v) _SCV_IMPL2(&=)
  return_type& operator &= (int                 v) _SCV_IMPL2(&=)
  return_type& operator &= (unsigned int        v) _SCV_IMPL2(&=)
  return_type& operator &= (const sc_int_base&  v) _SCV_IMPL2(&=)
  return_type& operator &= (const sc_uint_base& v) _SCV_IMPL2(&=)

  return_type& operator |= (const sc_signed&    v) _SCV_IMPL2(|=)
  return_type& operator |= (const sc_unsigned&  v) _SCV_IMPL2(|=)
  return_type& operator |= (int64               v) _SCV_IMPL2(|=)
  return_type& operator |= (uint64              v) _SCV_IMPL2(|=)
  return_type& operator |= (long                v) _SCV_IMPL2(|=)
  return_type& operator |= (unsigned long       v) _SCV_IMPL2(|=)
  return_type& operator |= (int                 v) _SCV_IMPL2(|=)
  return_type& operator |= (unsigned int        v) _SCV_IMPL2(|=)
  return_type& operator |= (const sc_int_base&  v) _SCV_IMPL2(|=)
  return_type& operator |= (const sc_uint_base& v) _SCV_IMPL2(|=)

  return_type& operator ^= (const sc_signed&    v) _SCV_IMPL2(^=)
  return_type& operator ^= (const sc_unsigned&  v) _SCV_IMPL2(^=)
  return_type& operator ^= (int64               v) _SCV_IMPL2(^=)
  return_type& operator ^= (uint64              v) _SCV_IMPL2(^=)
  return_type& operator ^= (long                v) _SCV_IMPL2(^=)
  return_type& operator ^= (unsigned long       v) _SCV_IMPL2(^=)
  return_type& operator ^= (int                 v) _SCV_IMPL2(^=)
  return_type& operator ^= (unsigned int        v) _SCV_IMPL2(^=)
  return_type& operator ^= (const sc_int_base&  v) _SCV_IMPL2(^=)
  return_type& operator ^= (const sc_uint_base& v) _SCV_IMPL2(^=)

  return_type& operator <<= (const sc_signed&    v) _SCV_IMPL2(<<=)
  return_type& operator <<= (const sc_unsigned&  v) _SCV_IMPL2(<<=)
  return_type& operator <<= (int64               v) _SCV_IMPL2(<<=)
  return_type& operator <<= (uint64              v) _SCV_IMPL2(<<=)
  return_type& operator <<= (long                v) _SCV_IMPL2(<<=)
  return_type& operator <<= (unsigned long       v) _SCV_IMPL2(<<=)
  return_type& operator <<= (int                 v) _SCV_IMPL2(<<=)
  return_type& operator <<= (unsigned int        v) _SCV_IMPL2(<<=)
  return_type& operator <<= (const sc_int_base&  v) _SCV_IMPL2(<<=)
  return_type& operator <<= (const sc_uint_base& v) _SCV_IMPL2(<<=)

  return_type& operator >>= (const sc_signed&    v) _SCV_IMPL2(>>=)
  return_type& operator >>= (const sc_unsigned&  v) _SCV_IMPL2(>>=)
  return_type& operator >>= (int64               v) _SCV_IMPL2(>>=)
  return_type& operator >>= (uint64              v) _SCV_IMPL2(>>=)
  return_type& operator >>= (long                v) _SCV_IMPL2(>>=)
  return_type& operator >>= (unsigned long       v) _SCV_IMPL2(>>=)
  return_type& operator >>= (int                 v) _SCV_IMPL2(>>=)
  return_type& operator >>= (unsigned int        v) _SCV_IMPL2(>>=)
  return_type& operator >>= (const sc_int_base&  v) _SCV_IMPL2(>>=)
  return_type& operator >>= (const sc_uint_base& v) _SCV_IMPL2(>>=)

  return_type& operator ++ ()
  { this->initialize(); ++*this->_get_instance(); this->trigger_value_change_cb(); return *this; }
  const sc_unsigned operator ++ (int)
  { this->initialize(); sc_bigint<W> tmp = *this->_get_instance()++; this->trigger_value_change_cb(); return tmp; }
  return_type& operator -- ()
  { this->initialize(); --*this->_get_instance(); this->trigger_value_change_cb(); return *this; }
  const sc_unsigned operator -- (int)
  { this->initialize(); sc_bigint<W> tmp = *this->_get_instance()--; this->trigger_value_change_cb(); return tmp; }
  //  sc_unsigned_bitref operator [] (int i)
  const bool operator [] (int i) const
  { this->initialize(); return this->_get_instance()->operator [](i); }
  const sc_unsigned range(int i, int j) const
  { this->initialize(); return this->_get_instance()->range(i,j); }
  //  sc_unsigned_subref operator () (int i, int j)
  const sc_unsigned operator () (int i, int j) const
  { this->initialize(); return this->_get_instance()->operator ()(i,j); }

  std::string to_string(sc_numrep base = SC_DEC, bool formatted = false) const
  { this->initialize(); return this->_get_instance()->to_string(base,formatted); }
  std::string to_string(int base, bool formatted = false) const
  { this->initialize(); return this->_get_instance()->to_string(base,formatted); }

  _SCV_MAP(int64,to_int64);
  _SCV_MAP(uint64,to_uint64);
  _SCV_MAP(long,to_long);
  _SCV_MAP(unsigned long,to_ulong);
  _SCV_MAP(unsigned long,to_unsigned_long);
  _SCV_MAP(int,to_int);
  _SCV_MAP(int,to_signed);
  _SCV_MAP(unsigned int,to_uint);
  _SCV_MAP(unsigned int,to_unsigned);
  _SCV_MAP(unsigned int,to_unsigned_int);
  _SCV_MAP(double,to_double);
  //  void print() const { this->initialize(); this->_get_instance()->print(); }
  //  void print(ostream &os) const { this->initialize(); this->_get_instance()->print(os); }
  void dump() const { this->initialize(); this->_get_instance()->dump(); };
  void dump(ostream &os) const { this->initialize(); this->_get_instance()->dump(os); };
  _SCV_MAP(int,length);
  _SCV_MAP(bool,iszero);
  _SCV_MAP(bool,sign);
  bool test(int i) const { this->initialize(); return this->_get_instance()->test(i); }
  void set(int i) { this->initialize(); this->_get_instance()->set(i); this->trigger_value_change_cb(); }
  void clear(int i) { this->initialize(); this->_get_instance()->clear(i); this->trigger_value_change_cb(); }
  void set(int i, bool v) { this->initialize(); this->_get_instance()->set(i,v); this->trigger_value_change_cb(); }
  void invert(int i) { this->initialize(); this->_get_instance()->invert(i); this->trigger_value_change_cb(); }
  void reverse() { this->initialize(); this->_get_instance()->reverse(); this->trigger_value_change_cb(); }
  void get_packed_rep(sc_dt::sc_digit *buf) const { this->initialize(); this->_get_instance()->get_packet_ref(buf); }
  void set_packed_rep(sc_dt::sc_digit *buf) { this->_get_instance()->get_packet_ref(buf); this->trigger_value_change_cb(); }

  operator const sc_signed&() const { this->initialize(); return *this->_get_instance(); }
};

template<>
class scv_extensions< sc_bit >
  : public scv_extensions_base< sc_bit > {
public:
  _SCV_PAREN_OPERATOR(sc_bit)

  typedef scv_extensions< sc_bit > return_type;

  return_type& operator=(const return_type& v) _SCV_IMPL1
  return_type& operator = ( const sc_bit& v ) _SCV_IMPL
  return_type& operator = ( int v ) _SCV_IMPL
  return_type& operator = ( bool v ) _SCV_IMPL
  return_type& operator = ( char v ) _SCV_IMPL
  return_type& operator &= ( const sc_bit& v ) _SCV_IMPL2(&=)
  return_type& operator &= ( int v ) _SCV_IMPL2(&=)
  return_type& operator &= ( bool v ) _SCV_IMPL2(&=)
  return_type& operator &= ( char v ) _SCV_IMPL2(&=)
  return_type& operator |= ( const sc_bit& v ) _SCV_IMPL2(|=)
  return_type& operator |= ( int v ) _SCV_IMPL2(|=)
  return_type& operator |= ( bool v ) _SCV_IMPL2(|=)
  return_type& operator |= ( char v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( const sc_bit& v ) _SCV_IMPL2(^=)
  return_type& operator ^= ( int v ) _SCV_IMPL2(^=)
  return_type& operator ^= ( bool v ) _SCV_IMPL2(^=)
  return_type& operator ^= ( char v ) _SCV_IMPL2(^=)
  _SCV_MAP(bool,to_bool);
  _SCV_MAP(char,to_char);
  //  void print( ostream& os) const { this->initialize(); return this->_get_instance()->print(os); }
  operator const sc_bit&() const { this->initialize(); return *this->_get_instance(); }
};

template<>
class scv_extensions< sc_logic >
  : public scv_extensions_base< sc_logic > {
public:
  _SCV_PAREN_OPERATOR(sc_logic)

  typedef scv_extensions< sc_logic > return_type;

  return_type& operator=(const return_type& v) _SCV_IMPL1
  return_type& operator = ( sc_dt::sc_logic_value_t v ) _SCV_IMPL
  return_type& operator = ( const sc_logic& v ) _SCV_IMPL
  return_type& operator = ( char v ) _SCV_IMPL
  return_type& operator = ( int v ) _SCV_IMPL
  return_type& operator = ( bool v ) _SCV_IMPL
  return_type& operator &= ( const sc_logic& v ) _SCV_IMPL2(&=)
  return_type& operator |= ( const sc_logic& v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( const sc_logic& v ) _SCV_IMPL2(^=)
  bool operator == ( const sc_logic& r ) const { this->initialize(); return *this->_get_instance() == r; }
  bool operator == ( char r ) const { this->initialize(); return *this->_get_instance() == r; }
  bool operator != ( const sc_logic& r ) const { this->initialize(); return *this->_get_instance() != r; }
  bool operator != ( char r ) const { this->initialize(); return *this->_get_instance() != r; }
  _SCV_MAP(char,to_char);
  _SCV_MAP(bool,is_01);
  _SCV_MAP(bool,to_bool);
  //  void print( ostream& os ) const { this->initialize(); this->_get_instance()->print(os); }

  operator const sc_logic&() const { this->initialize(); return *this->_get_instance(); }
};

template<int W>
class scv_extensions< sc_bv<W> >
  : public scv_extensions_base< sc_bv<W> > {
public:
  _SCV_PAREN_OPERATOR(sc_bv<W>)
  operator const sc_bv<W>&() const { this->initialize(); return *this->_get_instance(); }

  typedef scv_extensions< sc_bv<W> > return_type;

  sc_bv_base* clone() { return this->_get_instance()->clone(); /* don't clone randomization status */ }
  return_type& operator=(const return_type& v) _SCV_IMPL1
  //  template<class T> return_type& operator=(const sc_proxy<T>& v) _SCV_IMPL
  return_type& operator=(const sc_bv<W> & v) _SCV_IMPL
  return_type& operator=( const char* v) _SCV_IMPL
  return_type& operator=( const bool* v) _SCV_IMPL
  return_type& operator=( const sc_unsigned& v) _SCV_IMPL
  return_type& operator=( const sc_signed& v) _SCV_IMPL
  return_type& operator=( const sc_uint_base& v) _SCV_IMPL
  return_type& operator=( const sc_int_base& v) _SCV_IMPL
  return_type& operator=( long v) _SCV_IMPL
  return_type& operator=( unsigned long v) _SCV_IMPL
  return_type& operator=( int v) _SCV_IMPL
  return_type& operator=( unsigned v) _SCV_IMPL
  return_type& operator=( char v) _SCV_IMPL
  return_type& operator=( const sc_bit& v) _SCV_IMPL
  return_type& operator=(int64 v) _SCV_IMPL
  return_type& operator=(uint64 v) _SCV_IMPL
  return_type& operator=( const sc_int<W>& v) _SCV_IMPL
  return_type& operator=( const sc_uint<W>& v) _SCV_IMPL

  void resize(unsigned long new_size)
  { this->initialize(); this->_get_instance()->resize(new_size); this->trigger_value_change_cb(); }

  // from sc_bv_base
  long get_bit(unsigned n) const { this->initialize(); return this->_get_instance()->get_bit(n); }
  void set_bit(unsigned bit_number, long value)
  { this->initialize(); this->_get_instance()->set_bit(bit_number,value); this->trigger_value_change_cb(); }
  unsigned long get_word(unsigned i) const { this->initialize(); return this->_get_instance()->get_word(i); }
  void set_word(unsigned i, unsigned long w)
  { this->initialize(); this->_get_instance()->set_word(i,w); this->trigger_value_change_cb(); }
  unsigned long get_cword(unsigned i) const { this->initialize(); return this->_get_instance()->get_cword(i); }
  void set_cword(unsigned i, unsigned long w)
  { this->initialize(); this->_get_instance()->set_cword(i,w); this->trigger_value_change_cb(); }
  _SCV_MAP(int,length);

  return_type& operator &= ( const sc_unsigned& v ) _SCV_IMPL2(&=)
  return_type& operator |= ( const sc_unsigned& v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( const sc_unsigned& v ) _SCV_IMPL2(^=)

  return_type& operator &= ( const sc_signed& v ) _SCV_IMPL2(&=)
  return_type& operator |= ( const sc_signed& v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( const sc_signed& v ) _SCV_IMPL2(^=)

  return_type& operator &= ( unsigned int v ) _SCV_IMPL2(&=)
  return_type& operator |= ( unsigned int v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( unsigned int v ) _SCV_IMPL2(^=)

  return_type& operator &= ( int v ) _SCV_IMPL2(&=)
  return_type& operator |= ( int v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( int v ) _SCV_IMPL2(^=)

  return_type& operator &= ( unsigned long v ) _SCV_IMPL2(&=)
  return_type& operator |= ( unsigned long v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( unsigned long v ) _SCV_IMPL2(^=)

  return_type& operator &= ( long v ) _SCV_IMPL2(&=)
  return_type& operator |= ( long v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( long v ) _SCV_IMPL2(^=)

  return_type& operator &= ( const char* v ) _SCV_IMPL2(&=)
  return_type& operator |= ( const char* v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( const char* v ) _SCV_IMPL2(^=)

  sc_bv_base operator & ( const char* s ) const
  { this->initialize(); return *this->_get_instance() & s; }
  sc_bv_base operator | ( const char* s ) const
  { this->initialize(); return *this->_get_instance() | s; }
  sc_bv_base operator ^ ( const char* s ) const
  { this->initialize(); return *this->_get_instance() ^ s; }

  friend return_type operator & ( const char* s, const return_type& b )
  { b.initialize(); return *b._get_instance() & s; }
  friend return_type operator | ( const char* s, const return_type& b )
  { b.initialize(); return *b._get_instance() | s; }
  friend return_type operator ^ ( const char* s, const return_type& b )
  { b.initialize(); return *b._get_instance() ^ s; }

  void set(unsigned long v=0) { this->initialize(); this->_get_instance()->set(v); this->trigger_value_change_cb(); }

};

template<int W>
class scv_extensions< sc_lv<W> >
  : public scv_extensions_base< sc_lv<W> > {
public:
  _SCV_PAREN_OPERATOR(sc_lv<W>)
  operator const sc_lv<W>&() const { this->initialize(); return *this->_get_instance(); }

  typedef scv_extensions< sc_lv<W> > return_type;

  sc_bv_base* clone() { return this->_get_instance()->clone(); /* don't clone randomization status */ }
  return_type& operator=(const return_type& v) _SCV_IMPL1
  //  template<class T> return_type& operator=(const sc_proxy<T>& v) _SCV_IMPL
  return_type& operator=(const sc_lv<W>& v) _SCV_IMPL
  return_type& operator=( const char* v) _SCV_IMPL
  return_type& operator=( const bool* v) _SCV_IMPL
  return_type& operator=( const sc_logic* v) _SCV_IMPL // this is the only difference from sc_bv.
  return_type& operator=( const sc_unsigned& v) _SCV_IMPL
  return_type& operator=( const sc_signed& v) _SCV_IMPL
  return_type& operator=( const sc_uint_base& v) _SCV_IMPL
  return_type& operator=( const sc_int_base& v) _SCV_IMPL
  return_type& operator=( long v) _SCV_IMPL
  return_type& operator=( unsigned long v) _SCV_IMPL
  return_type& operator=( int v) _SCV_IMPL
  return_type& operator=( unsigned v) _SCV_IMPL
  return_type& operator=( char v) _SCV_IMPL
  return_type& operator=( const sc_bit& v) _SCV_IMPL
  return_type& operator=(int64 v) _SCV_IMPL
  return_type& operator=(uint64 v) _SCV_IMPL
  return_type& operator=( const sc_int<W>& v) _SCV_IMPL
  return_type& operator=( const sc_uint<W>& v) _SCV_IMPL

  void resize(unsigned long new_size)
  { this->initialize(); this->_get_instance()->resize(new_size); this->trigger_value_change_cb(); }

  // from sc_bv_base
  long get_bit(unsigned n) const { this->initialize(); return this->_get_instance()->get_bit(n); }
  void set_bit(unsigned bit_number, long value)
  { this->initialize(); this->_get_instance()->set_bit(bit_number,value); this->trigger_value_change_cb(); }
  unsigned long get_word(unsigned i) const { this->initialize(); return this->_get_instance()->get_word(i); }
  void set_word(unsigned i, unsigned long w)
  { this->initialize(); this->_get_instance()->set_word(i,w); this->trigger_value_change_cb(); }
  unsigned long get_cword(unsigned i) const { this->initialize(); return this->_get_instance()->get_cword(i); }
  void set_cword(unsigned i, unsigned long w)
  { this->initialize(); this->_get_instance()->set_cword(i,w); this->trigger_value_change_cb(); }
  _SCV_MAP(int,length);

  return_type& operator &= ( const sc_unsigned& v ) _SCV_IMPL2(&=)
  return_type& operator |= ( const sc_unsigned& v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( const sc_unsigned& v ) _SCV_IMPL2(^=)

  return_type& operator &= ( const sc_signed& v ) _SCV_IMPL2(&=)
  return_type& operator |= ( const sc_signed& v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( const sc_signed& v ) _SCV_IMPL2(^=)

  return_type& operator &= ( unsigned int v ) _SCV_IMPL2(&=)
  return_type& operator |= ( unsigned int v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( unsigned int v ) _SCV_IMPL2(^=)

  return_type& operator &= ( int v ) _SCV_IMPL2(&=)
  return_type& operator |= ( int v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( int v ) _SCV_IMPL2(^=)

  return_type& operator &= ( unsigned long v ) _SCV_IMPL2(&=)
  return_type& operator |= ( unsigned long v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( unsigned long v ) _SCV_IMPL2(^=)

  return_type& operator &= ( long v ) _SCV_IMPL2(&=)
  return_type& operator |= ( long v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( long v ) _SCV_IMPL2(^=)

  return_type& operator &= ( const char* v ) _SCV_IMPL2(&=)
  return_type& operator |= ( const char* v ) _SCV_IMPL2(|=)
  return_type& operator ^= ( const char* v ) _SCV_IMPL2(^=)

  sc_bv_base operator & ( const char* s ) const
  { this->initialize(); return *this->_get_instance() & s; }
  sc_bv_base operator | ( const char* s ) const
  { this->initialize(); return *this->_get_instance() | s; }
  sc_bv_base operator ^ ( const char* s ) const
  { this->initialize(); return *this->_get_instance() ^ s; }

  friend return_type operator & ( const char* s, const return_type& b )
  { b.initialize(); return *b._get_instance() & s; }
  friend return_type operator | ( const char* s, const return_type& b )
  { b.initialize(); return *b._get_instance() | s; }
  friend return_type operator ^ ( const char* s, const return_type& b )
  { b.initialize(); return *b._get_instance() ^ s; }

  bool is_01() { this->initialize(); return this->_get_instance()->is_01(); } // this should have been "const"

};

#endif // SystemC

#undef _SCV_INTEGER_INTERFACE

// ----------------------------------------
// special extension class to handle getting an extension from an extension
// ----------------------------------------
template<typename T>
class scv_extensions< scv_extensions<T> > : public scv_extensions<T> {
public:
  scv_extensions< scv_extensions<T> > () {}
  scv_extensions< scv_extensions<T> > (const scv_extensions<T>& rhs) : scv_extensions<T>(rhs) {}
  virtual ~scv_extensions() {}
  scv_extensions& operator=(const scv_extensions<T>& rhs) {
    return scv_extensions<T>::operator=(rhs);
  }
  scv_extensions& operator=(const T& rhs) {
    return scv_extensions<T>::operator=(rhs);
  }
  operator const T&() const { return *scv_extensions<T>::_get_instance(); }
  scv_expression operator()() { return scv_extensions<T>::form_expression(); }

  virtual void _set_instance(T *i) {
    scv_extensions<T>::_set_instance(i);
  }
  virtual void _set_instance(scv_extensions<T> *i) {
    scv_extensions<T>::_set_instance(i->_get_instance());
  }
};

// ----------------------------------------
// specialization for array
// ----------------------------------------
template<typename T, int N>
class scv_extensions<T[N]> : public _SCV_INTROSPECTION_BASE2 {

  typedef T my_type[N];
public:
// ----------------------------------------
// implementation of the specialization for array
// (added cast of N to "int" since some compilers automatically
// regard it as unsigned even though I have declard it as int)
// ----------------------------------------
  scv_extensions();
  virtual ~scv_extensions() {};

public:
  scv_extensions<T>& operator[](int i);
  const scv_extensions<T>& operator[](int i) const;

public:
  virtual void _set_instance_core_wrap(void *p);

public:
  scv_extensions& operator=(const scv_extensions& rhs);
  scv_extensions& operator=(const T * rhs);

private:
  scv_extensions<T> _array[N];
};

// ----------------------------------------
// specialization for pointers
// ----------------------------------------
template<typename T> class scv_smart_ptr;

template<typename T>
class scv_extensions<T*> : public _SCV_INTROSPECTION_BASE1 {
public:
  // (can only be used with pointer to a single object)
  // (cannot only be used with pointer to an array)
  scv_extensions() {}
  virtual ~scv_extensions() {}

public:
  scv_extensions<T>& operator*();
  const scv_extensions<T>& operator*() const;
  scv_extensions<T> * operator->();
  const scv_extensions<T> * operator->() const;

public:
  scv_extensions<T*>& operator=(const scv_extensions<T*>& rhs);
  scv_extensions<T*>& operator=(scv_extensions<T> * rhs);
  scv_extensions<T*>& operator=(const scv_smart_ptr<T>& rhs);
  scv_extensions<T*>& operator=(T * rhs);
  scv_extensions<T*>& operator=(int);

public:
  virtual void _set_instance_core_wrap(void *p);
  const char* get_type_name() const {
    static const char * s = _scv_ext_util_get_name("%s*", scv_extensions<T>().get_type_name());
    return s;
  }

private:
  mutable bool _own_typed_ptr;
  mutable scv_extensions<T> * _typed_ptr;
  const scv_extensions<T> * _get_ptr() const;
  const scv_extensions<T> * _set_ptr() const;
};

// ----------------------------------------
// implementation of the specialization for array
// (added cast of N to "int" since some compilers automatically
// regard it as unsigned even though I have declard it as int)
// ----------------------------------------
template<typename T, int N>
scv_extensions<T[N]>::scv_extensions() {
  std::string tmp;
  _scv_extension_util ** a = new _scv_extension_util*[N];
  for (int i=0; i<(int)N; ++i) {
    a[i] = &_array[i];
    tmp = "[" + _scv_ext_util_get_string(i) + "]";
    _array[i]._set_parent(this,tmp);
  }
  this->_set_up_array(a);
}

template<typename T, int N>
scv_extensions<T>& scv_extensions<T[N]>::operator[](int i) {
  if (i<0 || i>=(int)N) {
    _scv_message::message(_scv_message::INTROSPECTION_INVALID_INDEX,i,"array",this->get_name());
    return _array[0];
  }
  return _array[i];
}

template<typename T, int N>
const scv_extensions<T>& scv_extensions<T[N]>::operator[](int i) const {
  if (i<0 || i>=(int)N) {
    _scv_message::message(_scv_message::INTROSPECTION_INVALID_INDEX,i,"array",this->get_name());
    return _array[0];
  }
  return _array[i];
}

template<typename T, int N>
void scv_extensions<T[N]>::_set_instance_core_wrap(void *p) {
  if ( p ) {
    my_type *tp = (my_type*)p;
    for (int i=0; i<(int)N; ++i) _array[i]._set_instance(&(*tp)[i]);
  }
}

/*
template<typename T, int N>
scv_extensions<T[N]>& scv_extensions<T[N]>::operator=(const scv_extensions& rhs) {
  for (int i=0; i<(int)N; ++i) { _array[i] = rhs[i]; }
  this->trigger_value_change_cb();
}
*/

template<typename T, int N>
scv_extensions<T[N]>& scv_extensions<T[N]>::operator=(const T * rhs) {
  for (int i=0; i<(int)N; ++i) { _array[i] = rhs[i]; }
  this->trigger_value_change_cb();
  return *this;
}

// ----------------------------------------
// implementation of the specialization for pointers
// ----------------------------------------
template<typename T> const scv_extensions<T> *
scv_extensions<T*>::_get_ptr() const {
  if (*(this->_instance)) {
    if (!this->_ptr) {
      this->_own_typed_ptr = true;
      this->_typed_ptr = new scv_extensions<T>();
      this->_ptr = this->_typed_ptr;
      this->_typed_ptr->_set_instance(*this->_instance);
    }
  } else {
    if (this->_ptr) {
      if (this->_own_typed_ptr) delete this->_typed_ptr;
      this->_typed_ptr = NULL;
      this->_ptr = NULL;
    }
  }
  return this->_typed_ptr;
}

template<typename T> const scv_extensions<T> *
scv_extensions<T*>::_set_ptr() const {
  if (*this->_get_instance()) {
    if (!this->_ptr) {
      this->_own_typed_ptr = true;
      this->_typed_ptr = new scv_extensions<T>();
      this->_ptr = this->_typed_ptr;
    }
    this->_typed_ptr->_set_instance(*this->_get_instance());
  } else {
    if (this->_ptr) {
      if (this->_own_typed_ptr) delete this->_typed_ptr;
      this->_typed_ptr = NULL;
      this->_ptr = NULL;
    }
  }
  return this->_typed_ptr;
}

template<typename T> scv_extensions<T>&
scv_extensions<T*>::operator*() {
  const scv_extensions<T> * ptr = _get_ptr();
  if (!ptr) {
    static scv_extensions<T> e;
    _scv_message::message(_scv_message::INTROSPECTION_NULL_POINTER,this->get_name());
    return e;
  }
  return *(scv_extensions<T>*)ptr;
}

template<typename T> const scv_extensions<T>&
scv_extensions<T*>::operator*() const {
  const scv_extensions<T> * ptr = _get_ptr();
  if (!ptr) {
    static scv_extensions<T> e;
    _scv_message::message(_scv_message::INTROSPECTION_NULL_POINTER,this->get_name());
    return &e;
  }
  return *ptr;
}

template<typename T> scv_extensions<T> *
scv_extensions<T*>::operator->() {
  return (scv_extensions<T>*)_get_ptr();
}

template<typename T> const scv_extensions<T> *
scv_extensions<T*>::operator->()  const {
  return (scv_extensions<T>*)_get_ptr();
}

template<typename T> scv_extensions<T*>&
scv_extensions<T*>::operator=(const scv_extensions<T*>& rhs) {
  *this->_get_instance() = *rhs._instance;
  if (rhs._ptr->is_dynamic()) {
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

template<typename T> scv_extensions<T*>&
scv_extensions<T*>::operator=(scv_extensions<T> * rhs) {
  *this->_get_instance() = rhs->_instance;
  if (rhs->is_dynamic()) {
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

template<typename T> scv_extensions<T*>&
scv_extensions<T*>::operator=(T * rhs) {
  *this->_get_instance() = rhs;
  _set_ptr();
  this->trigger_value_change_cb();
  return *this;
}

template<typename T> scv_extensions<T*>&
scv_extensions<T*>::operator=(int rhs) {
  *this->_get_instance() = (T*) rhs;
  _set_ptr();
  this->trigger_value_change_cb();
  return *this;
}

template<typename T> void
scv_extensions<T*>::_set_instance_core_wrap(void *) {
  _set_ptr();
}

