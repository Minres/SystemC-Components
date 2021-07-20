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

  scv_expression.h -- 
  The public interface for the expression facility.
  - scv_expression to create boolean expressions to used internally
    for specifying constraints and event expressions
  - Expressions can be created on objects of type scv_smart_ptr
    or sc_signal along with constants . 
  - Provides interface to traverse the expression tree and evaluate
    boolean value for an expression.
  - Logical operators    [ &&  ||  ! ]
  - Relational operators [ >  >=  < <= == != ]
  - Arithmetic operators [ +  -  * ]

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

#ifndef SCV_EXPRESSION_H
#define SCV_EXPRESSION_H

#include "scv/scv_object_if.h"
#include "scv/scv_shared_ptr.h"
#include <list>

namespace sc_core {

class sc_interface;
template <class T> class sc_signal_in_if;

} // namespace sc_core

class scv_expression_core_base;

class scv_expression : public scv_object_if {
public:
  enum operatorT {
    EMPTY,
    EXTENSION,
    INT_CONSTANT,
    BOOLEAN_CONSTANT,
    UNSIGNED_CONSTANT,
    DOUBLE_CONSTANT,
    SC_BIGINT_CONSTANT,
    SC_BIGUINT_CONSTANT,
    SC_BV_CONSTANT,
    STRING_CONSTANT,
    SC_STRING_CONSTANT,
    SC_SIGNAL,
    EQUAL,
    NOT_EQUAL,
    GREATER_THAN,
    LESS_THAN,
    GREATER_OR_EQUAL,
    LESS_OR_EQUAL,
    AND, 
    OR,
    NOT,
    PLUS,
    MINUS,
    MULTIPLY
  };
public: // constructing an expression from a constant or a variable
  scv_expression(scv_expression_core_base * core = NULL);
  scv_expression(const scv_expression& rhs);
  scv_expression(bool b);
  scv_expression(int i);
  scv_expression(long long i);
  scv_expression(unsigned u);
  scv_expression(unsigned long long i);
  scv_expression(double d);
  scv_expression(std::string);
  template <int W>
  static scv_expression create_constant(const sc_int<W>& v) {
    return _scv_create_expression(v);
  }
  template <int W>
  static scv_expression create_constant(const sc_uint<W>& v) {
    return _scv_create_expression(v);
  }
  template <int W>
  static scv_expression create_constant(const sc_bigint<W>& v) {
    return _scv_create_expression(v);
  }
  template <int W>
  static scv_expression create_constant(const sc_biguint<W>& v) {
    return _scv_create_expression(v);
  }
  template <int W>
  static scv_expression create_constant(const sc_bv<W>& v) {
    return _scv_create_expression(v);
  }
  template <class T>
  static scv_expression create_reference(sc_core::sc_signal_in_if<T>& s){
    return _scv_create_expression(s);
  }

  virtual ~scv_expression();

  // constructing a complex expression from simpler expressions
  friend scv_expression operator==(const scv_expression& a,
				  const scv_expression& b);
  friend scv_expression operator!=(const scv_expression& a,
				  const scv_expression& b);
  friend scv_expression operator>(const scv_expression& a,
				  const scv_expression& b);
  friend scv_expression operator<(const scv_expression& a,
				  const scv_expression& b);
  friend scv_expression operator>=(const scv_expression& a,
				  const scv_expression& b);
  friend scv_expression operator<=(const scv_expression& a,
				  const scv_expression& b);
  friend scv_expression operator&&(const scv_expression& a,
				  const scv_expression& b);
  friend scv_expression operator||(const scv_expression& a,
				  const scv_expression& b);
  friend scv_expression operator!(const scv_expression& a);
  friend scv_expression operator+(const scv_expression& a,
				 const scv_expression& b);
  friend scv_expression operator-(const scv_expression& a,
				 const scv_expression& b);
  friend scv_expression operator*(const scv_expression& a,
				 const scv_expression& b);
public: // convenient methods
  scv_expression& operator=(bool);
  scv_expression& operator&=(const scv_expression& e); 
public: // basic operation
  bool evaluate(void) const;
public: // expression tree traversal 
  long long get_int_value(void) const ;
  bool get_bool_value(void) const ;
  unsigned long long get_unsigned_value(void) const;
  double get_double_value(void) const;
  int get_bit_width(void) const;
  scv_extensions_if* get_extension(void) const;
  sc_core::sc_interface* get_signal(void) const;
  void get_extension_list(std::list<scv_extensions_if*>& ext_list);
  void get_signal_list(std::list<sc_core::sc_interface*>& sig_list);
public: // get_value methods to access value of constants
  void get_value(bool&) const;
  void get_value(char&) const;
  void get_value(short&) const;
  void get_value(unsigned short&) const;
  void get_value(int&) const;
  void get_value(unsigned int&) const;
  void get_value(long&) const;
  void get_value(unsigned long&) const;
  void get_value(long long&) const;
  void get_value(unsigned long long&) const;
  void get_value(float&) const;
  void get_value(double&) const;
  void get_value(std::string&) const;
  void get_value(sc_bv_base&) const;
  void get_value(sc_lv_base&) const;
public:
  const scv_expression& get_left(void) const;
  const scv_expression& get_right(void) const;
  scv_expression::operatorT get_operator(void) const;
  const char* get_expression_string(void) const;
public: // debugging interface
  const char *get_name() const;
  const char *kind() const;
  void print(ostream& o=scv_out, int details=0, int indent=0) const;
  void show(int details=0, int indent=0) const; 
  static int get_debug();
  static void set_debug(int i);
public: // method for backward compatibility (create_reference)
  template <class T>
  static scv_expression create(sc_signal_in_if<T>& s){
    return _scv_create_expression(s);
  }
private: // private methods for updating sc_signal values
  void update_signal_value(void) const; 
  friend void _scv_update_signal_value(const scv_expression& e);
private:
  scv_shared_ptr<scv_expression_core_base> core_;
  static int _debug;
};

class _scv_expression_error {
public:
  static void illegalAccess(const char * msgP) {
    _scv_message::message(_scv_message::EXPRESSION_ILLEGAL_EXTRACTION,msgP);
  }
};

class scv_expression_core_base {
public:
  virtual ~scv_expression_core_base() {};
  virtual const char *get_name(void) const = 0;
  virtual const scv_expression& get_left(void) const = 0;
  virtual const scv_expression& get_right(void) const = 0;
  virtual long long get_int_value(void) const = 0;
  virtual bool get_bool_value(void) const = 0;
  virtual unsigned long long get_unsigned_value(void) const = 0;
  virtual double get_double_value(void) const = 0;
  virtual int get_bit_width(void) const = 0; 
  virtual scv_extensions_if * get_extension(void) const = 0;
  virtual sc_core::sc_interface * get_signal(void) const = 0;
  virtual scv_expression::operatorT get_operator(void) const = 0;
  virtual void update_signal_value(void) const = 0;
  virtual void get_value(bool&) const = 0;
  virtual void get_value(char&) const = 0;
  virtual void get_value(short&) const = 0;
  virtual void get_value(unsigned short&) const = 0;
  virtual void get_value(int&) const = 0;
  virtual void get_value(unsigned int&) const = 0;
  virtual void get_value(long&) const = 0;
  virtual void get_value(unsigned long&) const = 0;
  virtual void get_value(long long&) const = 0;
  virtual void get_value(unsigned long long&) const = 0;
  virtual void get_value(float&) const = 0;
  virtual void get_value(double&) const = 0;
  virtual void get_value(std::string&) const = 0;
  virtual void get_value(sc_bv_base&) const = 0;
  virtual void get_value(sc_lv_base&) const = 0;
};

template <typename T>
scv_extensions<T> scv_get_extensions(T& d);

template <typename T>
const scv_extensions<T> scv_get_const_extensions(const T& d);



class scv_expression_core : public scv_expression_core_base {
protected:
  mutable scv_extensions_if * core_;
  union {
    long long _intValue;
    int _boolValue;
    unsigned long long _unsignedValue;
    double _doubleValue;
    std::string* _str;
    std::string* _sc_str;
  }_value;
  sc_bv_base *_data;
  int _bit_width;
  scv_expression::operatorT _operator;
  scv_expression _left;
  scv_expression _right;
public:
  scv_expression_core(scv_extensions_if * core);
  scv_expression_core(int i);
  scv_expression_core(long long i);
  scv_expression_core(bool i);
  scv_expression_core(unsigned u);
  scv_expression_core(unsigned long long u);
  scv_expression_core(double d);
  scv_expression_core(std::string s);
  template <int W>
  scv_expression_core(sc_int<W> v) : core_(NULL), _data(NULL) {
    _value._intValue = v;
    _bit_width = W;
    _operator = scv_expression::INT_CONSTANT;
  }
  template <int W>
  scv_expression_core(sc_uint<W> v) : core_(NULL), _data(NULL) {
    _value._unsignedValue = v;
    _bit_width = W;
    _operator = scv_expression::UNSIGNED_CONSTANT;
  }
  template <int W>
  scv_expression_core(sc_bigint<W> v) : core_(NULL) {
    _data = new sc_bv_base(W);
    _bit_width = W;
    *_data = v;
    _operator = scv_expression::SC_BIGINT_CONSTANT;
  }
  template <int W>
  scv_expression_core(sc_biguint<W> v) : core_(NULL) {
    _data = new sc_bv_base(W);
    *_data = v;
    _bit_width = W;
    _operator = scv_expression::SC_BIGUINT_CONSTANT;
  }
  template <int W>
  scv_expression_core(sc_bv<W> v) : core_(NULL) {
    _data = new sc_bv_base(W);
    _bit_width = W;
    *_data = v;
    _operator = scv_expression::SC_BV_CONSTANT;
  }
  virtual ~scv_expression_core();
public:
  const char *get_name(void) const;
  virtual void update_signal_value(void) const;
  long long get_int_value(void) const;
  bool get_bool_value(void) const;
  unsigned long long get_unsigned_value(void) const;
  double get_double_value(void) const;
  int get_bit_width(void) const {
    return _bit_width;
  }
  scv_extensions_if * get_extension(void) const;
  virtual sc_core::sc_interface * get_signal(void) const;
  const scv_expression& get_left(void) const {
    return _left;
  }
  const scv_expression& get_right(void) const {
    return _right;
  }
  scv_expression::operatorT get_operator(void) const {
    return _operator;
  }
  scv_expression_core(scv_expression::operatorT op,
    const scv_expression& a , const scv_expression& b) :
   _operator(op), _left(a), _right(b) {}
public: // definition of get_value method 
  void get_value(bool&) const;
  void get_value(char&) const;
  void get_value(short&) const;
  void get_value(unsigned short&) const;
  void get_value(int&) const;
  void get_value(unsigned int&) const;
  void get_value(long&) const;
  void get_value(unsigned long&) const;
  void get_value(long long&) const;
  void get_value(unsigned long long&) const;
  void get_value(float&) const;
  void get_value(double&) const;
  void get_value(std::string&) const;
  void get_value(sc_bv_base&) const;
  void get_value(sc_lv_base&) const;
protected:
  scv_expression_core(scv_expression::operatorT op) : _data(NULL), _operator(op) {
  }
};

template<typename T>
class scv_expression_core_signal : public scv_expression_core {
  sc_signal_in_if<T>*  sig_;
public:
  scv_expression_core_signal(sc_signal_in_if<T>& s) : scv_expression_core(scv_expression::SC_SIGNAL) {
    const scv_extensions<T> e = scv_get_const_extensions(s.read());
    core_ = new scv_extensions<T>(e);
    sig_ = &s;
  }

  virtual void update_signal_value(void) const {
    assert(_operator == scv_expression::SC_SIGNAL);
    const scv_extensions<T> e = scv_get_const_extensions(sig_->read());
    *((scv_extensions<T>*) core_) = e;
  }

  virtual sc_core::sc_interface * get_signal(void) const {
      return sig_;
  }
};



#define _SCV_GET_CONSTANT_VALUE()                                   \
  if (_operator == scv_expression::BOOLEAN_CONSTANT) {             \
    val =  _value._boolValue;                                      \
  } else if (_operator == scv_expression::INT_CONSTANT) {          \
    val = _value._intValue;                                        \
  } else if (_operator == scv_expression::UNSIGNED_CONSTANT) {     \
    val = _value._unsignedValue;                                   \
  } else if (_operator == scv_expression::SC_BIGINT_CONSTANT ||    \
    _operator == scv_expression::SC_BIGUINT_CONSTANT ||            \
    _operator == scv_expression::SC_BV_CONSTANT ) {                \
    sc_signed value(_bit_width);                                   \
    value = (*_data);

#define _SCV_GET_CONSTANT_ERROR(type_name)                          \
  } else {                                                         \
    _scv_expression_error::illegalAccess("get_value(#type_name&)");\
    return ;                                                       \
  }                                                                

#define _SCV_GET_SC_VAL(type_name, method)                          \
  _SCV_GET_CONSTANT_VALUE();                                        \
  val = (type_name)value.method();                                 \
  _SCV_GET_CONSTANT_ERROR(type_name);                               \



template <class T>
inline scv_expression _scv_create_expression(sc_signal_in_if<T>& s)
{
  return scv_expression(new scv_expression_core_signal<T>(s));
}

template <int W>
inline scv_expression _scv_create_expression(const sc_int<W>& v)
{
  return scv_expression(new scv_expression_core(v));
}

template <int W>
inline scv_expression _scv_create_expression(const sc_uint<W>& v)
{
  return scv_expression(new scv_expression_core(v));
}

template <int W>
inline scv_expression _scv_create_expression(const sc_bigint<W>& v)
{
  return scv_expression(new scv_expression_core(v));
}

template <int W>
inline scv_expression _scv_create_expression(const sc_biguint<W>& v)
{
  return scv_expression(new scv_expression_core(v)); 
}

template <int W>
inline scv_expression _scv_create_expression(const sc_bv<W>& v)
{
  return scv_expression(new scv_expression_core(v));
}


#endif
