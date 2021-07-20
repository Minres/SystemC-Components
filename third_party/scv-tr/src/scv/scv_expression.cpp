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

  scv_expression.cpp -- The public interface for the expression facility.

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

#include "scv/scv_util.h"
#include "scv/scv_introspection.h"
#include "scv/scv_debug.h"

int scv_expression::_debug = scv_debug::INITIAL_DEBUG_LEVEL;

/////////////////////////////////////////////////////////////////////
// Class : scv_expression
//   - Implementation of scv_expression interface
/////////////////////////////////////////////////////////////////////

scv_expression::scv_expression(scv_expression_core_base * core) : core_(core) {}

scv_expression::scv_expression(const scv_expression& rhs) : core_(rhs.core_) {}
 
scv_expression::scv_expression(int i) : core_(new scv_expression_core(i) ) {}

scv_expression::scv_expression(long long i) : core_(new scv_expression_core(i)) {}

scv_expression::scv_expression(bool b) : core_(new scv_expression_core(b) ) {}

scv_expression::scv_expression(unsigned u) : core_(new scv_expression_core(u) ) {}

scv_expression::scv_expression(unsigned long long u) : core_(new scv_expression_core(u)) {}

scv_expression::scv_expression(double d) : core_(new scv_expression_core(d)) {}

scv_expression::scv_expression(std::string s) : core_(new scv_expression_core(s)) {}

scv_expression::~scv_expression() {}

scv_expression operator==(const scv_expression& a,
                         const scv_expression& b) 
{
  return scv_expression(new scv_expression_core(scv_expression::EQUAL, a, b)); 
}

scv_expression operator!=(const scv_expression& a,
                         const scv_expression& b) 
{
  return scv_expression(new scv_expression_core(scv_expression::NOT_EQUAL, a, b)); 
}

scv_expression operator>(const scv_expression& a,
                        const scv_expression& b) 
{
  return scv_expression(new scv_expression_core(scv_expression::GREATER_THAN, a, b)); 
}

scv_expression operator<(const scv_expression& a,
                        const scv_expression& b) 
{
  return scv_expression(new scv_expression_core(scv_expression::LESS_THAN, a, b)); 
}

scv_expression operator>=(const scv_expression& a,
                         const scv_expression& b) 
{
  return scv_expression(new scv_expression_core(scv_expression::GREATER_OR_EQUAL, a, b)); 
}

scv_expression operator<=(const scv_expression& a,
                         const scv_expression& b) 
{
  return scv_expression(new scv_expression_core(scv_expression::LESS_OR_EQUAL, a, b)); 
}

scv_expression operator&&(const scv_expression& a,
                         const scv_expression& b) 
{
  return scv_expression(new scv_expression_core(scv_expression::AND, a, b)); 
}

scv_expression operator||(const scv_expression& a,
                         const scv_expression& b) 
{
  return scv_expression(new scv_expression_core(scv_expression::OR, a, b)); 
}

scv_expression operator!(const scv_expression& a)
{
  return scv_expression(new scv_expression_core(scv_expression::NOT, a, a)); 
}

scv_expression operator+(const scv_expression& a,
                        const scv_expression& b) 
{
  return scv_expression(new scv_expression_core(scv_expression::PLUS, a, b)); 
}

scv_expression operator-(const scv_expression& a,
                        const scv_expression& b) 
{
  return scv_expression(new scv_expression_core(scv_expression::MINUS, a, b)); 
}

scv_expression operator*(const scv_expression& a,
                        const scv_expression& b) 
{
  return scv_expression(new scv_expression_core(scv_expression::MULTIPLY, a, b)); 
}

scv_expression& scv_expression::operator=(bool) { static scv_expression e = scv_expression(new scv_expression_core(1));
  *this = e; 
  return *this;
}

scv_expression& scv_expression::operator&=(const scv_expression& e) { 
  *this = *this && e;
  return *this; 
}

template <typename T>
static T _evaluateOperation(const scv_expression& e, T& left, T& right ); 
static const scv_extensions_if * _evaluateExpression(const scv_expression& e); 
static bool _getBool(const scv_extensions_if * valueP); 

bool scv_expression::evaluate(void) const {
  const scv_extensions_if * valueP = _evaluateExpression(*this);
  if (!valueP) return false;
  switch (valueP->get_type()) {
  case scv_extensions_if::RECORD:
  case scv_extensions_if::ARRAY:
    assert(0); return false;
  case scv_extensions_if::INTEGER:
    return valueP->get_integer() > 0; 
  case scv_extensions_if::BOOLEAN:
    return valueP->get_bool(); 
  case scv_extensions_if::UNSIGNED:
    return valueP->get_unsigned() > 0; 
  case scv_extensions_if::FLOATING_POINT_NUMBER:
    return valueP->get_double() > 0; 
  default: {
    std::string msg = "Cannot recognize extensions of type " +
                 std::string(valueP->get_type_name()) + "in scv_expression";
    _scv_message::message(_scv_message::INTERNAL_ERROR, msg.c_str());
    return false;
  }
  }
}

long long scv_expression::get_int_value() const 
{
  return (core_->get_int_value());
}

bool scv_expression::get_bool_value() const 
{
  return (core_->get_bool_value());
}

unsigned long long scv_expression::get_unsigned_value() const 
{
  return (core_->get_unsigned_value());
}

double scv_expression::get_double_value() const 
{
  return (core_->get_double_value());
}

int scv_expression::get_bit_width(void) const {
  return core_->get_bit_width();
}

scv_extensions_if* scv_expression::get_extension(void) const 
{
  return (core_->get_extension());
}

sc_interface* scv_expression::get_signal(void) const 
{
  return (core_->get_signal());
}

static void _get_extension_list(const scv_expression& e, 
  std::list<scv_extensions_if*>&ext_list);

void scv_expression::get_extension_list(std::list<scv_extensions_if*>& ext_list)
{
  ext_list.clear();
  _get_extension_list(*this, ext_list);
}

static void _get_signal_list(const scv_expression& e, 
  std::list<sc_interface*>&sig_list);

void scv_expression::get_signal_list(std::list<sc_interface*>& sig_list)
{
  sig_list.clear();
  _get_signal_list(*this, sig_list);
}

const scv_expression& scv_expression::get_left(void) const
{
  return (core_->get_left());
}

const scv_expression& scv_expression::get_right(void) const
{
  return (core_->get_right());
}

scv_expression::operatorT scv_expression::get_operator(void) const
{
  return (core_->get_operator());
}

void _scv_update_signal_value(const scv_expression& e)
{
  e.update_signal_value();
}

static std::string _get_expression_string(const scv_expression& e);

const char *scv_expression::get_expression_string(void) const 
{
  static std::string text;
  text = _get_expression_string(*this);
  return text.c_str();
}

int scv_expression::get_debug()
{
  return _debug;
}

const char *scv_expression::get_name() const
{
  return core_->get_name();
}

const char *scv_expression::kind() const
{
  static const char *name = "scv_expression"; 
  return name;
}

void scv_expression::print(ostream& o, int details, int indent) const
{
  o << get_expression_string() << endl; 
}

void scv_expression::set_debug(int debug)
{
  if ( _debug == debug ) return;
  _debug = debug;
  scv_debug::set_facility_level(scv_debug::RANDOMIZATION, debug);
}

void scv_expression::show(int details, int indent) const
{
  print(scv_out, details, indent);
}

void scv_expression::update_signal_value(void) const
{
  core_->update_signal_value();
}

void scv_expression::get_value(bool& v) const{ 
  core_->get_value(v);
}
void scv_expression::get_value(char& v) const{ 
  core_->get_value(v);
}
void scv_expression::get_value(short& v) const{ 
  core_->get_value(v);
}
void scv_expression::get_value(unsigned short& v) const{ 
  core_->get_value(v);
}
void scv_expression::get_value(int& v) const{ 
  core_->get_value(v);
}
void scv_expression::get_value(unsigned int& v) const{ 
  core_->get_value(v);
}
void scv_expression::get_value(long& v) const{ 
  core_->get_value(v);
}
void scv_expression::get_value(unsigned long& v) const{ 
  core_->get_value(v);
}
void scv_expression::get_value(long long& v) const{ 
  core_->get_value(v);
}
void scv_expression::get_value(unsigned long long& v) const{ 
  core_->get_value(v);
}
void scv_expression::get_value(float& v) const{ 
  core_->get_value(v);
}
void scv_expression::get_value(double& v) const{ 
  core_->get_value(v);
}
void scv_expression::get_value(std::string& v) const{ 
  core_->get_value(v);
}
void scv_expression::get_value(sc_bv_base& v) const{ 
  core_->get_value(v);
}
void scv_expression::get_value(sc_lv_base& v) const{ 
  core_->get_value(v);
}

static std::string _get_expression_string(const scv_expression& e) {
  char tmpString[1024];
  switch (e.get_operator()) {
  case scv_expression::EMPTY:
    return "(empty)";
  case scv_expression::INT_CONSTANT:
    sprintf(tmpString,"%lld",e.get_int_value()); return tmpString;
  case scv_expression::BOOLEAN_CONSTANT:
    if (e.get_bool_value()) {
      sprintf(tmpString,"true"); return tmpString;
    } else {
      sprintf(tmpString,"false"); return tmpString;
    }
  case scv_expression::UNSIGNED_CONSTANT:
    sprintf(tmpString,"%llu",e.get_unsigned_value()); return tmpString;
  case scv_expression::SC_BIGINT_CONSTANT:
  case scv_expression::SC_BIGUINT_CONSTANT:
  case scv_expression::SC_BV_CONSTANT: {
    sc_bv_base val(e.get_bit_width());
    e.get_value(val);
    return val.to_string();
  }
  case scv_expression::DOUBLE_CONSTANT:
    sprintf(tmpString,"%f",e.get_double_value()); return tmpString;
  case scv_expression::EXTENSION:
    return e.get_name();
  case scv_expression::PLUS:
    return "(" + _get_expression_string(e.get_left()) + "+" + _get_expression_string(e.get_right()) + ")";
  case scv_expression::MINUS:
    return "(" + _get_expression_string(e.get_left()) + "-" + _get_expression_string(e.get_right()) + ")";
  case scv_expression::MULTIPLY:
    return "(" + _get_expression_string(e.get_left()) + "*" + _get_expression_string(e.get_right()) + ")";
  case scv_expression::EQUAL:
    return "(" + _get_expression_string(e.get_left()) + "==" + _get_expression_string(e.get_right()) + ")";
  case scv_expression::NOT_EQUAL:
    return "(" + _get_expression_string(e.get_left()) + "!=" + _get_expression_string(e.get_right()) + ")";
  case scv_expression::GREATER_THAN:
    return "(" + _get_expression_string(e.get_left()) + ">" + _get_expression_string(e.get_right()) + ")";
  case scv_expression::LESS_THAN:
    return "(" + _get_expression_string(e.get_left()) + "<" + _get_expression_string(e.get_right()) + ")";

  case scv_expression::GREATER_OR_EQUAL:
    return "(" + _get_expression_string(e.get_left()) + ">=" + _get_expression_string(e.get_right()) + ")";
  case scv_expression::LESS_OR_EQUAL:
    return "(" + _get_expression_string(e.get_left()) + "<=" + _get_expression_string(e.get_right()) + ")";
  case scv_expression::AND:
    return "(" + _get_expression_string(e.get_left()) + "&&" + _get_expression_string(e.get_right()) + ")";
  case scv_expression::OR:
    return "(" + _get_expression_string(e.get_left()) + "||" + _get_expression_string(e.get_right()) + ")";
  case scv_expression::NOT:
    return "!" + _get_expression_string(e.get_left());
  case scv_expression::SC_SIGNAL:
    return e.get_name();
  default:
    return "<error>";
  }
}

static void _get_extension_list(const scv_expression& e, std::list<scv_extensions_if*>&ext_list)
{
  switch (e.get_operator()) {
  case scv_expression::EMPTY:
  case scv_expression::INT_CONSTANT:
  case scv_expression::BOOLEAN_CONSTANT:
  case scv_expression::UNSIGNED_CONSTANT:
  case scv_expression::DOUBLE_CONSTANT:
  case scv_expression::SC_SIGNAL:
    return;
  case scv_expression::EXTENSION:
    ext_list.push_back(e.get_extension());
    return;
  case scv_expression::PLUS:
  case scv_expression::MINUS:
  case scv_expression::MULTIPLY:
  case scv_expression::EQUAL:
  case scv_expression::NOT_EQUAL:
  case scv_expression::GREATER_THAN:
  case scv_expression::LESS_THAN:
  case scv_expression::GREATER_OR_EQUAL:
  case scv_expression::LESS_OR_EQUAL:
  case scv_expression::AND:
  case scv_expression::OR:
    _get_extension_list(e.get_left(),ext_list);
    _get_extension_list(e.get_right(),ext_list);
    return;
  case scv_expression::NOT:
    _get_extension_list(e.get_left(),ext_list);
    return;
  default:
    return;
  }
}

static void _get_signal_list(const scv_expression& e, std::list<sc_interface*>&sig_list)
{
  switch (e.get_operator()) {
  case scv_expression::EMPTY:
  case scv_expression::INT_CONSTANT:
  case scv_expression::BOOLEAN_CONSTANT:
  case scv_expression::UNSIGNED_CONSTANT:
  case scv_expression::DOUBLE_CONSTANT:
  case scv_expression::EXTENSION:
    return;
  case scv_expression::SC_SIGNAL:
    sig_list.push_back(e.get_signal());
    return;
  case scv_expression::PLUS:
  case scv_expression::MINUS:
  case scv_expression::MULTIPLY:
  case scv_expression::EQUAL:
  case scv_expression::NOT_EQUAL:
  case scv_expression::GREATER_THAN:
  case scv_expression::LESS_THAN:
  case scv_expression::GREATER_OR_EQUAL:
  case scv_expression::LESS_OR_EQUAL:
  case scv_expression::AND:
  case scv_expression::OR:
    _get_signal_list(e.get_left(),sig_list);
    _get_signal_list(e.get_right(),sig_list);
    return;
  case scv_expression::NOT:
    _get_signal_list(e.get_left(),sig_list);
    return;
  default:
    return;
  }
}

static bool _getBool(const scv_extensions_if * valueP) {
  switch (valueP->get_type()) {
  case scv_extensions_if::INTEGER:
    return valueP->get_integer() > 0;
  case scv_extensions_if::BOOLEAN:
    return valueP->get_bool();
  case scv_extensions_if::UNSIGNED:
    return valueP->get_unsigned() > 0;
  case scv_extensions_if::FLOATING_POINT_NUMBER:
    return valueP->get_double() > 0;
  default:
    assert(0); return false;
  }
} 

template <typename T>
static T _evaluateOperation(const scv_expression& e, T& left, T& right ) {
  switch (e.get_operator()) {
  case scv_expression::PLUS:
    return left + right; 
  case scv_expression::MINUS:
    return left - right; 
  case scv_expression::MULTIPLY:
    return left * right;
  case scv_expression::EQUAL:
    return left == right;
  case scv_expression::NOT_EQUAL:
    return left != right;
  case scv_expression::GREATER_THAN:
    return left > right;
  case scv_expression::LESS_THAN:
    return left < right;
  case scv_expression::GREATER_OR_EQUAL:
    return left >= right;
  case scv_expression::LESS_OR_EQUAL:
    return left <= right;
  default:
    assert(0); T val=0; return val;
  }
}

static const scv_extensions_if * 
_evaluateExpression(const scv_expression& e) {
  static scv_smart_ptr<long long> s_intValue;
  static scv_smart_ptr<unsigned long long> s_unsignedValue;
  static scv_smart_ptr<double> s_doubleValue;
  static scv_smart_ptr<bool> s_boolValue;

  switch (e.get_operator()) {
  case scv_expression::EMPTY:
    *s_intValue = 0; return &*s_intValue;
  case scv_expression::INT_CONSTANT:
    *s_intValue = e.get_int_value(); return &*s_intValue;
  case scv_expression::BOOLEAN_CONSTANT:
    *s_boolValue = e.get_bool_value(); return &*s_boolValue;
  case scv_expression::UNSIGNED_CONSTANT:
    *s_unsignedValue = e.get_unsigned_value(); return &*s_unsignedValue;
  case scv_expression::DOUBLE_CONSTANT:
    *s_doubleValue = e.get_double_value(); return &*s_doubleValue;
  case scv_expression::EXTENSION:
    return e.get_extension();
  case scv_expression::SC_SIGNAL: 
    _scv_update_signal_value(e);
    return e.get_extension();
  case scv_expression::PLUS:
  case scv_expression::MINUS:
  case scv_expression::MULTIPLY:
  case scv_expression::EQUAL:
  case scv_expression::NOT_EQUAL:
  case scv_expression::GREATER_THAN:
  case scv_expression::LESS_THAN:
  case scv_expression::GREATER_OR_EQUAL:
  case scv_expression::LESS_OR_EQUAL:
    {
      const scv_extensions_if * valueP;

      valueP = _evaluateExpression(e.get_left());
      if (!valueP) return NULL;

      switch(valueP->get_type()) {
      case scv_extensions_if::INTEGER: {
        int left = (int)valueP->get_integer();
        valueP = _evaluateExpression(e.get_right());
        if (!valueP) return NULL;
        int right = (int)valueP->get_integer();
        *s_intValue = _evaluateOperation(e, left, right);
        return &*s_intValue;
        break;
      }
      case scv_extensions_if::BOOLEAN: {
        bool left = (bool)valueP->get_bool();
        valueP = _evaluateExpression(e.get_right());
        if (!valueP) return NULL;
        bool right = (bool)valueP->get_bool();
        *s_boolValue = _evaluateOperation(e, left, right);
        return &*s_boolValue;
        break;
      }
      case scv_extensions_if::UNSIGNED: {
        unsigned left = (unsigned)valueP->get_unsigned();
        valueP = _evaluateExpression(e.get_right());
        if (!valueP) return NULL;
        unsigned right = (unsigned)valueP->get_unsigned();
        *s_unsignedValue = _evaluateOperation(e, left, right);
        return &*s_unsignedValue;
        break;
      }
      case scv_extensions_if::FLOATING_POINT_NUMBER: {
        double left = valueP->get_double();
        valueP = _evaluateExpression(e.get_right());
        if (!valueP) return NULL;
        double right = valueP->get_double();
        *s_doubleValue = _evaluateOperation(e, left, right);
        return &*s_doubleValue;
        break;
      }
      default: {
        std::string msg = "Cannot recognize extensions of type " +
                 std::string(valueP->get_type_name()) + "in scv_expression";
        _scv_message::message(_scv_message::INTERNAL_ERROR, msg.c_str());
	*s_intValue = 0; return &*s_intValue;
      }
      }
    }
  case scv_expression::AND:
    {
      const scv_extensions_if * valueP;

      valueP = _evaluateExpression(e.get_left());
      if (!valueP) return NULL;
      bool left = _getBool(valueP);

      valueP = _evaluateExpression(e.get_right());
      if (!valueP) return NULL;
      bool right = _getBool(valueP);

      *s_intValue = left && right;
      return &*s_intValue;
    }
  case scv_expression::OR:
    {
      const scv_extensions_if * valueP;

      valueP = _evaluateExpression(e.get_left());
      if (!valueP) return NULL;
      bool left = _getBool(valueP);

      valueP = _evaluateExpression(e.get_right());
      if (!valueP) return NULL;
      bool right = _getBool(valueP);

      *s_intValue = left || right;
      return &*s_intValue;
    }
  case scv_expression::NOT:
    {
      const scv_extensions_if * valueP;

      valueP = _evaluateExpression(e.get_left());
      if (!valueP) return NULL;
      bool left = _getBool(valueP);

      *s_intValue = !left;
      return &*s_intValue;
    }
  default: {
    char msg[1024];
    sprintf(msg, "Cannot recognize extensions of type %d in scv_expression.",
      e.get_operator()); 
    _scv_message::message(_scv_message::INTERNAL_ERROR, msg);
    return NULL;
  }
  }
}



scv_expression_core::scv_expression_core(scv_extensions_if * core) : core_(core), _data(NULL) {
  _operator = scv_expression::EXTENSION;
}
scv_expression_core::scv_expression_core(int i) : core_(NULL), _data(NULL) {
  _value._intValue = i;
  _bit_width = sizeof(int) * 8;
  _operator = scv_expression::INT_CONSTANT;
} 
scv_expression_core::scv_expression_core(long long i) : core_(NULL), _data(NULL) {
  _value._intValue = i; 
  _bit_width = sizeof(long long) * 8;
  _operator = scv_expression::INT_CONSTANT;
} 
scv_expression_core::scv_expression_core(bool i) : core_(NULL) , _data(NULL){
  _value._boolValue = i;
  _bit_width = sizeof(bool) * 8;
  _operator = scv_expression::BOOLEAN_CONSTANT;
} 
scv_expression_core::scv_expression_core(unsigned u) : core_(NULL) , _data(NULL){
  _value._unsignedValue = u;
  _bit_width = sizeof(unsigned) * 8;
  _operator = scv_expression::UNSIGNED_CONSTANT;
}
scv_expression_core::scv_expression_core(unsigned long long u) : core_(NULL) , _data(NULL){
  _value._unsignedValue = u;
  _bit_width = sizeof(unsigned long long) * 8;
  _operator = scv_expression::UNSIGNED_CONSTANT;
}
scv_expression_core::scv_expression_core(double d) : core_(NULL) , _data(NULL){
  _value._doubleValue = d;
  _bit_width = sizeof(double) * 8;
  _operator = scv_expression::DOUBLE_CONSTANT;
}
scv_expression_core::scv_expression_core(std::string s) : core_(NULL) , _data(NULL){
  _value._str = new std::string;
  _bit_width = s.length();
  *(_value._str) = s;
  _operator = scv_expression::STRING_CONSTANT;
}
scv_expression_core::~scv_expression_core() {
  if (_operator == scv_expression::SC_BIGINT_CONSTANT ||
    _operator == scv_expression::SC_BIGUINT_CONSTANT ||       
    _operator == scv_expression::SC_BV_CONSTANT ) {          
    if (_data) delete _data;
  } else if (_operator == scv_expression::STRING_CONSTANT) {
    if (_value._str) delete _value._str;
  } else if (_operator == scv_expression::SC_STRING_CONSTANT) {
    if (_value._str) delete _value._sc_str;
  }
}
const char *scv_expression_core::get_name(void) const {
  const static char *def_name = "";
  const static char *def_signal_name = "signal";
  const static char *def_ext_name = "default";

  if (_operator == scv_expression::EXTENSION) {
    if ( ! strcmp(core_->get_name(),"") ) {
      return def_ext_name;
    } else {
      return core_->get_name();
    }
  } else if (_operator == scv_expression::SC_SIGNAL) {
      return def_signal_name;
  } else {
    _scv_expression_error::illegalAccess("get_name");
    return def_name;
  }
}
void scv_expression_core::update_signal_value(void) const {
    assert(0);
}
long long scv_expression_core::get_int_value(void) const {
  if (_operator == scv_expression::INT_CONSTANT) {
    return _value._intValue;
  } else {
    _scv_expression_error::illegalAccess("get_int_value");
    return 0;
  }
}
bool scv_expression_core::get_bool_value(void) const {
  if (_operator == scv_expression::BOOLEAN_CONSTANT) {
    return _value._boolValue;
  } else {
    _scv_expression_error::illegalAccess("get_bool_value");
    return 0;
  }
}
unsigned long long scv_expression_core::get_unsigned_value(void) const {
  if (_operator == scv_expression::UNSIGNED_CONSTANT) {
    return _value._unsignedValue;
  } else {
    _scv_expression_error::illegalAccess("get_unsigned_value");
    return 0;
  }
}
double scv_expression_core::get_double_value(void) const {
  if (_operator == scv_expression::DOUBLE_CONSTANT) {
    return _value._doubleValue;
  } else {
    _scv_expression_error::illegalAccess("get_double_value");
    return 0.0;
  }
}
scv_extensions_if * scv_expression_core::get_extension(void) const {
  if (_operator == scv_expression::EXTENSION ||
      _operator == scv_expression::SC_SIGNAL ) {
    return core_;
  } else {
    _scv_expression_error::illegalAccess("get_extension");
    return NULL;
  }
}
sc_interface * scv_expression_core::get_signal(void) const {
    _scv_expression_error::illegalAccess("get_signal");
    return NULL;
}

void scv_expression_core::get_value(char& val) const {
  _SCV_GET_SC_VAL(char, to_int);
}

void scv_expression_core::get_value(short& val) const {
 _SCV_GET_SC_VAL(short, to_int);
}

void scv_expression_core::get_value(unsigned short& val) const {
  _SCV_GET_SC_VAL(int, to_uint);
}

void scv_expression_core::get_value(int& val) const {
  _SCV_GET_SC_VAL(int, to_int);
}

void scv_expression_core::get_value(unsigned int& val) const {
  _SCV_GET_SC_VAL(int, to_uint);
}

void scv_expression_core::get_value(long& val) const {
  _SCV_GET_SC_VAL(long, to_long);
}

void scv_expression_core::get_value(unsigned long& val) const {
  _SCV_GET_SC_VAL(unsigned long, to_ulong);
}

void scv_expression_core::get_value(long long& val) const {
  _SCV_GET_SC_VAL(long long, to_int64);
}

void scv_expression_core::get_value(unsigned long long& val) const {
  _SCV_GET_SC_VAL(unsigned long long, to_uint64);
}

void scv_expression_core::get_value(float& val) const {
  _SCV_GET_SC_VAL(float, to_double);
}

void scv_expression_core::get_value(double& val) const {
  _SCV_GET_SC_VAL(double, to_double);
}

void scv_expression_core::get_value(std::string& val) const {
  if (_operator == scv_expression::STRING_CONSTANT) { 
    val = *(_value._str);
  } else if (_operator == scv_expression::SC_STRING_CONSTANT) {
    val = (_value._sc_str)->c_str();
  } else {
    _scv_expression_error::illegalAccess("get_value(std::string&)");
    return ;                                               
  }
}

void scv_expression_core::get_value(sc_bv_base& val) const {
  _SCV_GET_CONSTANT_VALUE();                                        
  val = value;
  _SCV_GET_CONSTANT_ERROR(sc_bv_base);                         
}

void scv_expression_core::get_value(sc_lv_base& val) const {
  _SCV_GET_CONSTANT_VALUE();                                        
  val = value;
  _SCV_GET_CONSTANT_ERROR(sc_lv_base);                         
}

void scv_expression_core::get_value(bool& val) const {
  if (_operator == scv_expression::BOOLEAN_CONSTANT) {  
    val =  _value._boolValue;                          
  } else if (_operator == scv_expression::INT_CONSTANT) { 
    val = (_value._intValue != 0);                              
  } else if (_operator == scv_expression::UNSIGNED_CONSTANT) {
    val = (_value._unsignedValue != 0);                             
  } else if (_operator == scv_expression::SC_BIGINT_CONSTANT ||
    _operator == scv_expression::SC_BIGUINT_CONSTANT ||       
    _operator == scv_expression::SC_BV_CONSTANT ) {          
    val = ((*_data) != 0);
  } else {
    _scv_expression_error::illegalAccess("get_value(bool&)");
    return ;                                                  
  }
}
