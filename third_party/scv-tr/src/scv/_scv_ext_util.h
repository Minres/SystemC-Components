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

  _scv_ext_util -- The implementation for the extension component "util".

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
// a first class in the hierarchy that contains class variables.
// ----------------------------------------
class _scv_extension_util : public _SCV_INTROSPECTION_BASE {
public:
  _scv_extension_util() 
    : _data(0), _name(""), _short_name(""), _parent(NULL) {}
  virtual ~_scv_extension_util() {
    if (_has_dynamic_data()) {
      delete _data;
    }
  }

  // scv_object_if
  virtual const char *get_name() const;
  virtual const char *kind() const;
  virtual void print(ostream& o= scv_out, int details=0, int indent=0) const;
  virtual void show(int details=0, int indent=0) const;

  // extension "util"
  virtual bool has_valid_extensions() const;
  virtual bool is_dynamic() const;
  virtual std::string get_short_name() const;
  virtual void set_name(const char * s);
  virtual void _set_name(const std::string& s);

public: // non-virtual for fast execution
  inline void trigger_value_change_cb() {
    if (_is_dynamic()) {
      this->updated();
      this->_get_dynamic_data()
	->execute_callbacks(this, scv_extensions_if::VALUE_CHANGE);
    }
  }

protected: // fast version of the introspection interface (non-virtual)
  bool _is_dynamic() const { return _data != 0; }

public: // internal methods (non-virtual for efficiency)
  bool _has_dynamic_data() const {
    return _data != 0 && _data != (_scv_dynamic_data*) 1;
  }
  _scv_dynamic_data * _get_dynamic_data();
  _scv_dynamic_data * get_dynamic_data();
  const _scv_dynamic_data *_get_dynamic_data() const;
  void _set_parent(_scv_extension_util *p, const std::string& name);
  const scv_extensions_if *_get_parent() const { return _parent; }

public: // internal methods (virtual to distinguish basic/record/array)
  virtual void _set_dynamic();

public:
  // overloaded 
  //   0 : not dynamic
  //   1 : capable of dynamic
  //   valid-ptr: has dynamic
  mutable _scv_dynamic_data *_data; 
  std::string _name;
  std::string _short_name;
  scv_extensions_if *_parent;
};

// ----------------------------------------
// specialization for record
// ----------------------------------------
class _scv_extension_util_record : public _scv_extension_util {
public:
  virtual ~_scv_extension_util_record() {}

  // extension "util"
  virtual bool has_valid_extensions() const { return false; }
  virtual void set_name(const char * s) {
    _name = s;
    std::list<_scv_extension_util*>::iterator f;
    for (f = _fields.begin(); f != _fields.end(); ++f) {
      (*f)->_set_name(_name + "." + (*f)->get_short_name().c_str());
    }
  }
  virtual void _set_name(const std::string& s) {
    _name = s;
    std::list<_scv_extension_util*>::iterator f;
    for (f = _fields.begin(); f != _fields.end(); ++f) {
      (*f)->_set_name(_name + "." + (*f)->get_short_name().c_str());
    }
  }

protected: // fast version of the introspection interface (non-virtual)
  int _get_num_fields() const { return static_cast<int>(_fields.size()); }
  _scv_extension_util * _get_field(unsigned i) {
  if (i>=_fields.size()) {
    _scv_message::message(_scv_message::INTROSPECTION_INVALID_INDEX,i,"composite object",get_name());
    return NULL;
  }
    std::list<_scv_extension_util*>::iterator f = _fields.begin();	
    while (i--) { ++f; }
    return *f;	
  }
  const _scv_extension_util * _get_field(unsigned i) const { 
  if (i>=_fields.size()) {
    _scv_message::message(_scv_message::INTROSPECTION_INVALID_INDEX,i,"composite object",get_name());
    return NULL;
  }
    std::list<_scv_extension_util*>::const_iterator f = _fields.begin();	
    while (i--) { ++f; }
    return *f;	
  }

public: // internal methods (non-virtual for efficiency)
  void _add_field(_scv_extension_util * f) { _fields.push_back(f); }

public: // internal methods (virtual to distinguish basic/record/array)
  virtual void _set_dynamic() { 
    _scv_extension_util::_set_dynamic();
    int size = _get_num_fields();
    for (int i=0; i<size; ++i) {
      _get_field(i)->_set_dynamic();
    }
  }

protected:
  std::list<_scv_extension_util*> _fields;
};

template<typename T>
class scv_extension_util
  : public _scv_extension_util_record {
public:
  virtual ~scv_extension_util() {}
};

// ----------------------------------------
// specialization for array
// (added cast of N to "int" since some compilers automatically
// regard it as unsigned even though I have declard it as int)
// ----------------------------------------
template<typename T, int N>
class scv_extension_util<T[N]>
  : public _scv_extension_util {
public:
  scv_extension_util() : _elts(0) {}
  virtual ~scv_extension_util() {
    if (_elts) delete[] _elts; 
  }

  // extension "util"
  virtual void set_name(const char * s) {
    _name = s;
    for (int i=0; i<(int)N; ++i) {
      _elts[i]->_set_name(_name + "[" + _scv_ext_util_get_string(i) + "]");
    }
  }
  virtual void _set_name(const std::string& s) {
    _name = s;
    for (int i=0; i<(int)N; ++i) {
      _elts[i]->_set_name(_name + "[" + _scv_ext_util_get_string(i) + "]");
    }
  }

protected: // fast version of the introspection interface (non-virtual)
  inline int _get_array_size() { return N; }
  inline _scv_extension_util * _get_array_elt(unsigned i) {
    return _elts[i];
  }
  inline const _scv_extension_util * _get_array_elt(unsigned i) const {
    return _elts[i];
  }

public: // internal methods (non-virtual for efficiency)
  inline void _set_up_array(_scv_extension_util** elts) { _elts = elts; }

public: // internal methods (virtual to distinguish basic/record/array)
  virtual void _set_dynamic() {
    _scv_extension_util::_set_dynamic();
    for (int i=0; i<(int)N; ++i) {
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
  _scv_extension_util_ptr() : _ptr(0) {}
  virtual ~_scv_extension_util_ptr() {}

protected: // fast version of the introspection interface (non-virtual)
  scv_extensions_if * _get_pointer() { return _ptr; }
  const scv_extensions_if * _get_pointer() const { return _ptr; }

protected:
  mutable _scv_extension_util* _ptr;
};

template<typename T>
class scv_extension_util<T*>
  : public _scv_extension_util_ptr {
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
  void _get_enum_details(std::list<const char *>&, std::list<int>&) const;
  const char * _get_enum_string(int) const;

  virtual std::list<const char *>& _get_names() const = 0;
  virtual std::list<int>& _get_values() const = 0;
};

// ----------------------------------------
// specialization for basic types
// ----------------------------------------
#define _SCV_COMPONENT(basic_type)                          \
template<>                                                  \
class scv_extension_util<basic_type>                        \
  : public _scv_extension_util {                            \
public:                                                     \
  virtual ~scv_extension_util() {}                          \
};                                                          \

#define _SCV_COMPONENT_1(basic_type)                        \
_SCV_COMPONENT(basic_type)                                  \

#define _SCV_COMPONENT_N(basic_type)                        \
template<int N>                                             \
class scv_extension_util<basic_type<N> >                    \
  : public _scv_extension_util {                            \
public:                                                     \
  virtual ~scv_extension_util() {}                          \
};                                                          \

_SCV_BASIC_TYPE_SPECIALIZATION();

#undef _SCV_COMPONENT
#undef _SCV_COMPONENT_1
#undef _SCV_COMPONENT_N

// ----------------------------------------
// wrap up this component
// ----------------------------------------
#undef _SCV_INTROSPECTION_BASE
#define _SCV_INTROSPECTION_BASE scv_extension_util<T>
#undef _SCV_INTROSPECTION_BASE1
#define _SCV_INTROSPECTION_BASE1 scv_extension_util<T*>
#undef _SCV_INTROSPECTION_BASE2
#define _SCV_INTROSPECTION_BASE2 scv_extension_util<T[N]>

#define _SCV_INTROSPECTION_BASE_ENUM _scv_extension_util_enum
