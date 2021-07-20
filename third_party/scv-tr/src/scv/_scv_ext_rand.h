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

  _scv_ext_rand.h -- The implementation for the extension component "rand"

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

const std::string& _scv_get_name(scv_constraint_base*);

#ifndef _SCV_INTROSPECTION_ONLY
#include "scv/_scv_randomization.h"
#else
#define GET_RANDOM() 
class _scv_constraint_data {
public:
  void set_random(scv_shared_ptr<scv_random>) {}
  scv_shared_ptr<scv_random> get_random() const { return scv_shared_ptr<scv_random>(); }
};
template<typename T> class _scv_distribution {};
#endif

void _scv_constraint_wrapup(scv_extensions_if* e);

// ----------------------------------------
// specialization for enum
// ----------------------------------------
class _scv_extension_rand_enum
  : public _SCV_INTROSPECTION_BASE_ENUM {
public:
  _scv_extension_rand_enum() {}
  virtual ~_scv_extension_rand_enum() {
    if (this->_has_dynamic_data() && this->_get_dynamic_data()->cdata_) {
      if (this->_get_dynamic_data()->cdata_->get_constraint()) {
        _scv_constraint_wrapup(this);
      }
      delete this->_get_dynamic_data()->cdata_;
    } 
  }
  virtual void next();
  virtual void disable_randomization();
  virtual void enable_randomization();
  virtual bool is_randomization_enabled();
  virtual void set_random(scv_shared_ptr<scv_random> gen);
  virtual scv_shared_ptr<scv_random> get_random(void);
  virtual scv_expression form_expression() const;
  virtual void use_constraint(scv_smart_ptr_if& s);
  virtual void use_constraint(scv_extensions_if* e);
  void reset_distribution();
public: // internal methods
  virtual void uninitialize();
  virtual void initialize() const;
  virtual void updated();
  virtual void set_constraint(scv_constraint_base* c);
  virtual void set_constraint(bool mode); 
  virtual void set_extension(scv_extensions_if * e = NULL);
  virtual bool is_initialized() const;
  virtual void generate_value_() = 0;
  virtual void _reset_bag_distribution() = 0;
  virtual _scv_constraint_data* get_constraint_data();
  virtual void get_generator(void);
#ifndef _SCV_INTROSPECTION_ONLY
  void set_value_mode(_scv_constraint_data::gen_mode m); 
#endif
protected:
  void _reset_keep_only_distribution();
};

#ifndef _SCV_INTROSPECTION_ONLY
inline _scv_constraint_data* _get_constraint_data_enum(_scv_extension_rand_enum* data) 
{
  return data->get_constraint_data();
}

inline void _set_mode_enum(_scv_extension_rand_enum* data, _scv_constraint_data::gen_mode m) 
{
  data->set_value_mode(m);
}

inline scv_shared_ptr<scv_random> _get_random_enum(_scv_extension_rand_enum* data)
{
  return data->get_constraint_data()->get_random(data);
}

bool _scv_has_complex_constraint(scv_extensions_if*);
void _scv_set_constraint(scv_extensions_if* s, bool mode);
#endif

#ifndef _SCV_INTROSPECTION_ONLY
void _scv_use_constraint(scv_extensions_if* to, scv_extensions_if* e);
scv_extensions_if* _scv_get_extension(scv_smart_ptr_if& s);
#else
inline void _scv_use_constraint(scv_extensions_if* to, scv_extensions_if* e) {}
inline scv_extensions_if* _scv_get_extension(scv_smart_ptr_if& s) { return 0; }
#endif

// ----------------------------------------
// specialization for records
// ----------------------------------------
template<typename T>
class scv_extension_rand
 : public _SCV_INTROSPECTION_BASE {
public:
  virtual ~scv_extension_rand() {
#ifndef _SCV_INTROSPECTION_ONLY
    if (this->_has_dynamic_data() && 
	this->_get_dynamic_data()->cdata_) {
      delete this->_get_dynamic_data()->cdata_;
    }
#endif
  }
  virtual void set_distribution_from(scv_extensions_if* e) {
    int size = this->_get_num_fields();
    for (int i=0; i<size; ++i) {
      this->_get_field(i)->set_distribution_from(e);
    }
  }
  virtual void next() { GET_RANDOM(); uninitialize(); initialize(); }
  virtual void uninitialize() { 
    if (this->_get_dynamic_data()->disable_randomization_ == true) return ;
    int size = this->_get_num_fields();
    for (int i=0; i<size; ++i) {
      this->_get_field(i)->uninitialize();
    }
#ifndef _SCV_INTROSPECTION_ONLY
    if (get_constraint_data()->get_mode() == _scv_constraint_data::EXTENSION) {
      get_constraint_data()->get_extension()->uninitialize();
    }
#endif
  }
  virtual void initialize() const {
    if (this->_is_dynamic()) {
      int size = this->_get_num_fields();
      for (int i=0; i<size; ++i) {
	this->_get_field(i)->initialize();
      }
    }
  };
  virtual void disable_randomization() {
    _SCV_CHECK_DYNAMIC(enable_randomization,return);
    this->_get_dynamic_data()->disable_randomization_ = true;
    int size = this->_get_num_fields();
    for (int i=0; i<size; ++i) {
      this->_get_field(i)->disable_randomization();
    }
  }
  virtual void enable_randomization() {
    _SCV_CHECK_DYNAMIC(enable_randomization,return);
    this->_get_dynamic_data()->disable_randomization_ = false;
    int size = this->_get_num_fields();
    for (int i=0; i<size; ++i) {
      this->_get_field(i)->enable_randomization();
    }
    uninitialize();
  }
  virtual bool is_randomization_enabled() {
    if (!this->_is_dynamic()) return false;
    int size = this->_get_num_fields();
    for (int i=0; i<size; ++i) {
      if (this->_get_field(i)->is_randomization_enabled())
	return true;
    }
    if (this->_has_dynamic_data()) {
      return !this->_get_dynamic_data()->disable_randomization_;
    }
    return false;
  }
  virtual void updated() {
    int size = this->_get_num_fields();
    for (int i=0; i<size; ++i) {
      this->_get_field(i)->updated();
    }
  }
  virtual void set_constraint(scv_constraint_base* c) {
    int size = this->_get_num_fields();
    for (int i=0; i<size; ++i) {
      this->_get_field(i)->set_constraint(c);
    }
#ifndef _SCV_INTROSPECTION_ONLY
    get_constraint_data()->set_constraint(c);
#endif
  }
  virtual void set_constraint(bool mode) {
#ifndef _SCV_INTROSPECTION_ONLY
    _scv_set_constraint(this, mode);
#endif
  }
  virtual void set_extension(scv_extensions_if * e = NULL) { 
#ifndef _SCV_INTROSPECTION_ONLY
    get_constraint_data()->set_extension(e);
    get_constraint_data()->set_mode(_scv_constraint_data::EXTENSION);
    int size = this->_get_num_fields();
    for (int i=0; i<size; ++i) {
      this->_get_field(i)->set_extension(e->get_field(i));
    }
#endif
  }
  virtual void set_random(scv_shared_ptr<scv_random> gen) {
    get_constraint_data()->set_random(gen);
    //cout << "set_random : record  : " << this->get_name() << endl;
    int size = this->_get_num_fields();
    for (int i=0; i<size; ++i) {
      this->_get_field(i)->set_random(gen);
    }
  }
  virtual scv_shared_ptr<scv_random> get_random(void) {
#ifndef _SCV_INTROSPECTION_ONLY
    return get_constraint_data()->get_random(this);
#else
    return scv_shared_ptr<scv_random>();
#endif
  }
  virtual bool is_initialized() const {
    if (this->_has_dynamic_data()) return !this->_get_dynamic_data()->undefined_;
    return !this->_is_dynamic();
  }
  virtual scv_expression form_expression() const { 
    _scv_message::message(_scv_message::INTROSPECTION_INVALID_EXPRESSION,"composite object",this->get_name());
    return scv_expression();
  }
  void use_constraint(scv_smart_ptr_if& s) { 
    use_constraint(_scv_get_extension(s)); 
  }
  virtual void use_constraint(scv_extensions_if* e) { 
    _scv_use_constraint(this, e);
    return;
  }
  virtual _scv_constraint_data * get_constraint_data() {
    if (!this->_get_dynamic_data()->cdata_)
      this->_get_dynamic_data()->cdata_ = new _scv_constraint_data;
    return this->_get_dynamic_data()->cdata_;
  }
  virtual void get_generator(void) {
    _scv_message::message(_scv_message::INTERNAL_ERROR,"cannot call get_generator for composite object");
    return;
  }
};

// ----------------------------------------
// specialization for arrays
// (added cast of N to "int" since some compilers automatically
// regard it as unsigned even though I have declard it as int)
// ----------------------------------------
template<typename T, int N>
class scv_extension_rand<T[N]>
  : public _SCV_INTROSPECTION_BASE2 {
public:
  scv_extension_rand() {}
  virtual ~scv_extension_rand() {
    if (this->_has_dynamic_data() && this->_get_dynamic_data()->cdata_)
      delete this->_get_dynamic_data()->cdata_;
  }

public:
  virtual void next() { GET_RANDOM(); uninitialize(); initialize(); }
  virtual void set_distribution_from(scv_extensions_if* e) {
    for (int i=0; i<(int)N; ++i) {
      this->_get_array_elt(i)->set_distribution_from(e);
    }
  }
  virtual void disable_randomization() {
    for (int i=0; i<(int)N; ++i) {
      this->_get_array_elt(i)->disable_randomization();
    }
  }
  virtual void enable_randomization() {
    for (int i=0; i<(int)N; ++i) {
      this->_get_array_elt(i)->enable_randomization();
    }
  }
  virtual bool is_randomization_enabled() {
    for (int i=0; i<(int)N; ++i) {
      if (this->_get_array_elt(i)->is_randomization_enabled())
	return true;
    }
    return false;
  }
  virtual void set_random(scv_shared_ptr<scv_random> r) {
    _SCV_CHECK_DYNAMIC(set_random,return);
    get_constraint_data()->set_random(r);
    for (int i=0; i<(int)N; ++i) {
      this->_get_array_elt(i)->set_random(r);
    }
  }
  virtual scv_shared_ptr<scv_random> get_random(void){
    _SCV_CHECK_DYNAMIC(set_random,return scv_shared_ptr<scv_random>());
    return get_constraint_data()->get_random(this);
  }
  virtual void get_generator(void) {
    _scv_message::message(_scv_message::INTERNAL_ERROR,"Cannot do get_generator for array type.");
    return;
  }
  virtual _scv_constraint_data * get_constraint_data() {
    if (!this->_get_dynamic_data()->cdata_)
      this->_get_dynamic_data()->cdata_ = new _scv_constraint_data;
    return this->_get_dynamic_data()->cdata_;
  }
  virtual scv_expression form_expression() const {
    _scv_message::message(_scv_message::INTROSPECTION_INVALID_EXPRESSION,"array",this->get_name());
    return scv_expression();
  }
  virtual void use_constraint(scv_smart_ptr_if& s) {
    _scv_message::message(_scv_message::INTERNAL_ERROR,"Cannot set constraint to array type.");
  }
  virtual void use_constraint(scv_extensions_if*) {
    _scv_message::message(_scv_message::INTERNAL_ERROR,"Cannot set constraint to array type.");
  }

public: // internal methods
  virtual void uninitialize() {
    for (int i=0; i<(int)N; ++i) {
      this->_get_array_elt(i)->uninitialize();
    }
  }
  virtual void initialize() const {
    for (int i=0; i<(int)N; ++i) {
      this->_get_array_elt(i)->initialize();
    }
  }
  virtual bool is_initialized() const {
    for (int i=0; i<(int)N; ++i) {
      if (!this->_get_array_elt(i)->is_initialized()) return false;
    }
    return true;
  }
  virtual void updated() {
    for (int i=0; i<(int)N; ++i) {
      this->_get_array_elt(i)->updated();
    }
  }
  virtual void set_constraint(scv_constraint_base* c) {
    for (int i=0; i<(int)N; ++i) {
      this->_get_array_elt(i)->set_constraint(c);
    }
    get_constraint_data()->set_constraint(c);
  }
  virtual void set_constraint(bool mode) {
#ifndef _SCV_INTROSPECTION_ONLY
    _scv_set_constraint(this, mode);
#endif
  }
  virtual void set_extension(scv_extensions_if* e) {
    get_constraint_data()->set_extension(e);
    get_constraint_data()->set_mode(_scv_constraint_data::EXTENSION);
    for (int i=0; i<(int)N; ++i) {
      this->_get_array_elt(i)->set_extension(e->get_array_elt(i));
    }
  }
};

// ----------------------------------------
// specialization for pointers
// ----------------------------------------
template<typename T>
class scv_extension_rand<T*>
 : public _SCV_INTROSPECTION_BASE1 {
public:
  virtual scv_shared_ptr<scv_random> get_random(void){
#ifndef _SCV_INTROSPECTION_ONLY
    _SCV_CHECK_DYNAMIC(get_random,return scv_shared_ptr<scv_random>());
    return get_constraint_data()->get_random(this);
#endif
  }
  virtual bool is_randomization_enabled() {
    return false;
  }
  virtual scv_expression form_expression() const { 
    _scv_message::message(_scv_message::INTROSPECTION_INVALID_EXPRESSION,"pointer",this->get_name());
    return scv_expression();
  }
  virtual void set_constraint(scv_constraint_base*) {
    //_scv_message::message(_scv_message::INTERNAL_ERROR,"Cannot set constraint to pointer type.");
  }
  virtual void set_constraint(bool mode) {
  }
  virtual void set_extension(scv_extensions_if*) {
    _scv_message::message(_scv_message::INTERNAL_ERROR,"Cannot set constraint to pointer type.");
  }
  void use_constraint(scv_smart_ptr_if& s) {
    _scv_message::message(_scv_message::INTERNAL_ERROR,"Cannot use constraint from pointer type.");
  }
  virtual void use_constraint(scv_extensions_if*) {
    _scv_message::message(_scv_message::INTERNAL_ERROR,"Cannot use constraint from pointer type.");
  }
  virtual void set_random(scv_shared_ptr<scv_random>r) {
    _SCV_CHECK_DYNAMIC(set_random,return);
    get_constraint_data()->set_random(r);
  }

  virtual _scv_constraint_data * get_constraint_data() {
    if (!this->_get_dynamic_data()->cdata_)
      this->_get_dynamic_data()->cdata_ = new _scv_constraint_data;
    return this->_get_dynamic_data()->cdata_;
  }

  virtual void get_generator(void) {
    _scv_message::message(_scv_message::INTERNAL_ERROR, "cannot call get_generator for pointer types");
    return;
  }
  virtual void set_distribution_from(scv_extensions_if* e) {}
public:
  virtual void disable_randomization() {} 
  virtual void enable_randomization() {}
  virtual void next() { uninitialize(); initialize(); }
  virtual void uninitialize() {}
  virtual void initialize() const {}
  virtual bool is_initialized() const { return true; }
  virtual void updated() {}
};



// ----------------------------------------
// specialization for basis types
// ----------------------------------------

#ifndef _SCV_INTROSPECTION_ONLY
#define _SCV_EXT_RAND_FC_EXTRA_D(T, type_id) \
  void set_value_mode(_scv_constraint_data::gen_mode m); \
  void keep_only(const T& value); \
  void keep_only(const T& lb, const T& ub); \
  void keep_only(const std::list<T>& vlist); \
  \
  void keep_out(const T& value); \
  void keep_out(const T& lb, const T& ub); \
  void keep_out(const std::list<T>& vlist); \
  _scv_distribution<T> * _get_distribution(); \
  void _set_distribution(_scv_distribution<T>*); \
  virtual ~_scv_extension_rand_ ## type_id(); \
  void set_mode(scv_extensions_if::mode_t t); \
  void set_mode(scv_bag<std::pair<T, T> >& d); \
  void set_mode(scv_bag<T>& d); \
  void reset_distribution(); \
  scv_extensions_if::mode_t get_mode(void); \
protected:  \
  void _reset_bag_distribution(); \
  void _reset_keep_only_distribution(); \

#else
#define _SCV_EXT_RAND_FC_EXTRA_D(T, type_id)
#endif


#define _SCV_EXT_RAND_FC_D(T, type_id) \
class _scv_extension_rand_ ## type_id \
 : public scv_extension_rw<T> { \
public: \
  virtual void next(); \
  virtual void uninitialize(); \
  virtual void initialize() const; \
public: \
  virtual void disable_randomization(); \
  virtual void enable_randomization(); \
  virtual bool is_randomization_enabled(); \
  virtual scv_expression form_expression() const; \
  virtual void updated(); \
  virtual bool is_initialized() const; \
  virtual void set_random(scv_shared_ptr<scv_random> gen); \
  virtual scv_shared_ptr<scv_random> get_random(void); \
  void use_constraint(scv_smart_ptr_if& s); \
  virtual void use_constraint(scv_extensions_if* e); \
  virtual void set_constraint(scv_constraint_base* c); \
  virtual void set_constraint(bool mode); \
  virtual void set_extension(scv_extensions_if * e = NULL); \
  \
  virtual _scv_constraint_data * get_constraint_data(); \
  virtual void set_distribution_from(scv_extensions_if* e); \
  virtual void get_generator(void); \
  _scv_extension_rand_ ## type_id(); \
  virtual void generate_value_(); \
  _SCV_EXT_RAND_FC_EXTRA_D(T, type_id) \
}; \
\
template<> \
class scv_extension_rand<T> \
  : public _scv_extension_rand_ ## type_id {}; \


#define _SCV_EXT_RAND_FC_1_D(basic_type,type_id) \
  _SCV_EXT_RAND_FC_D(basic_type,type_id) \


#define _SCV_EXT_RAND_FC_D_D(T, type_id) \
class _scv_extension_rand_ ## type_id \
 : public scv_extension_rw<T> { \
public: \
  virtual void next(); \
  virtual void uninitialize(); \
  virtual void initialize() const; \
public: \
  virtual void disable_randomization(); \
  virtual void enable_randomization(); \
  virtual bool is_randomization_enabled(); \
  virtual scv_expression form_expression() const; \
  virtual void updated(); \
  virtual bool is_initialized() const; \
  virtual void set_random(scv_shared_ptr<scv_random> gen); \
  virtual scv_shared_ptr<scv_random> get_random(void); \
  void use_constraint(scv_smart_ptr_if& s); \
  virtual void use_constraint(scv_extensions_if* e); \
  virtual void set_constraint(scv_constraint_base* c); \
  virtual void set_constraint(bool mode); \
  virtual void set_extension(scv_extensions_if * e = NULL); \
  \
  virtual _scv_constraint_data * get_constraint_data(); \
  virtual void get_generator(void); \
  _scv_extension_rand_ ## type_id(); \
  virtual void generate_value_(); \
  virtual void set_distribution_from(scv_extensions_if* e); \
  _SCV_EXT_RAND_FC_EXTRA_D(T, type_id) \
}; \
\
template<> \
class scv_extension_rand<T> \
  : public _scv_extension_rand_ ## type_id {}; \


// ----------------------------------------
// specialization for basis types
// ----------------------------------------
template<typename T>
class _scv_extension_rand_N
 : public scv_extension_rw<T> {
public:
  virtual void next() { uninitialize(); initialize(); }
  virtual void uninitialize() { 
    assert(this->_is_dynamic());
    if (this->_get_dynamic_data()->disable_randomization_ == true) return ;
    this->_get_dynamic_data()->undefined_ = true;
#ifndef _SCV_INTROSPECTION_ONLY
    if (get_constraint_data()->get_mode() == _scv_constraint_data::EXTENSION) {
      get_constraint_data()->get_extension()->uninitialize();
    }
#endif
  }
  virtual void initialize() const {
    if (this->_is_dynamic() && this->_get_dynamic_data()->undefined_) {
      const_cast<_scv_extension_rand_N*>(this)->generate_value_();
      const_cast<_scv_extension_rand_N*>(this)->_get_dynamic_data()->undefined_ = false;
    }
  };
public:
  virtual void disable_randomization() {
    assert(this->_is_dynamic());
    this->_get_dynamic_data()->disable_randomization_ = true;
  }
  virtual void enable_randomization() {
    assert(this->_is_dynamic());
    this->_get_dynamic_data()->disable_randomization_ = false;
    uninitialize();
  }
  virtual bool is_randomization_enabled() {
    if (!this->_is_dynamic()) return false;
    if (this->_has_dynamic_data()) return !this->_get_dynamic_data()->disable_randomization_;
    return true;
  }
  virtual scv_expression form_expression() const { 
    return scv_expression(new scv_expression_core((scv_extensions_if*)(this))); 
  }
  virtual void updated() { 
    this->_get_dynamic_data()->undefined_ = false;
  }
  virtual bool is_initialized() const { 
    return !this->_get_dynamic_data()->undefined_;
  }
  virtual void set_random(scv_shared_ptr<scv_random> gen) {
    get_constraint_data()->set_random(gen);
  }
  virtual scv_shared_ptr<scv_random> get_random(void) {
#ifndef _SCV_INTROSPECTION_ONLY
    return get_constraint_data()->get_random(this);
#endif
  }
  void use_constraint(scv_smart_ptr_if& s) { 
    use_constraint(_scv_get_extension(s)); 
  }
  virtual void use_constraint(scv_extensions_if* e) {
    _scv_use_constraint(this, e);
    return;
  }
  virtual void set_constraint(scv_constraint_base* c) { 
#ifndef _SCV_INTROSPECTION_ONLY
    get_constraint_data()->set_constraint(c);
#endif
  }
  virtual void set_constraint(bool mode) { 
#ifndef _SCV_INTROSPECTION_ONLY
    _scv_set_constraint(this, mode);
#endif
  }
  virtual void set_extension(scv_extensions_if * e = NULL) { 
#ifndef _SCV_INTROSPECTION_ONLY
    get_constraint_data()->set_extension(e);
    get_constraint_data()->set_mode(_scv_constraint_data::EXTENSION);
#endif
  }

#ifndef _SCV_INTROSPECTION_ONLY
  void set_value_mode(_scv_constraint_data::gen_mode m) {
    get_constraint_data()->set_mode(m);
    if (m == _scv_constraint_data::DISTRIBUTION ||
        m == _scv_constraint_data::DISTRIBUTION_RANGE) { 
      get_constraint_data()->set_ext_mode(scv_extensions_if::DISTRIBUTION);
    }
  }
#endif
  virtual _scv_constraint_data * get_constraint_data() {
#ifndef _SCV_INTROSPECTION_ONLY
    assert(this->_is_dynamic());
    if (!this->_get_dynamic_data()->cdata_) {
      this->_get_dynamic_data()->cdata_ = new _scv_constraint_data;
    }
    return this->_get_dynamic_data()->cdata_;
#endif
  }
  virtual void get_generator(void) {
#ifndef _SCV_INTROSPECTION_ONLY
    if (get_constraint_data()->get_gen_type() == _scv_constraint_data::EMPTY) {
      T* dummy_value = new T;
      _scv_keep_range(this, *dummy_value, *dummy_value, false, true);
      get_constraint_data()->set_mode(_scv_constraint_data::NO_CONSTRAINT);
      delete dummy_value;
    }
    return;
#endif
  }
#ifndef _SCV_INTROSPECTION_ONLY
  void keep_only(const T& value) {
    _reset_bag_distribution();
    _scv_keep_range(this, value, value, false);
  }
  void keep_only(const T& lb, const T& ub) {
    _reset_bag_distribution();
    _scv_keep_range(this, lb, ub, false);
  }
  void keep_only(const std::list<T>& vlist) {
    _reset_bag_distribution();
    _scv_keep_range(this, vlist);
  }

  void keep_out(const T& value) {
    _reset_bag_distribution();
    _scv_keep_range(this, value, value, true); 
  }
  void keep_out(const T& lb, const T& ub) {
    _reset_bag_distribution();
    _scv_keep_range(this, lb, ub, true);
  }
  void keep_out(const std::list<T>& vlist) {
    _reset_bag_distribution();
    typename std::list<T>::const_iterator i;
    for (i = vlist.begin(); i != vlist.end(); i++) {
      _scv_keep_range(this, *i, *i, true);
    }
  }
#endif
public:
  _scv_extension_rand_N() {}

#ifndef _SCV_INTROSPECTION_ONLY
  _scv_distribution<T> * _get_distribution() {
    return (_scv_distribution<T> *) this->_get_dynamic_data()->dist_;
  }
  void _set_distribution(_scv_distribution<T>* d) {
    _scv_distribution<T> * dist = _get_distribution();
    _scv_constraint_data * cdata = get_constraint_data();
    if (!dist) {
      this->_get_dynamic_data()->dist_ = new _scv_distribution<T>;
      dist = _get_distribution();
    } else {
      dist->reset_distribution();
    }
    if (d->dist_) {
      dist->set_mode(*d->dist_, cdata, this);
    } else if (d->dist_r_) {
      dist->set_mode(*d->dist_r_, cdata, this);
    } else {
      _scv_message::message(_scv_message::INTERNAL_ERROR, "_set_distribution(basis_types)");
    }
  }
  virtual ~_scv_extension_rand_N() { 
    if (this->_has_dynamic_data() && this->_get_dynamic_data()->dist_)
      delete _get_distribution();
    if (this->_has_dynamic_data() && this->_get_dynamic_data()->cdata_) {
      if (this->_get_dynamic_data()->cdata_->get_constraint()) {
        _scv_constraint_wrapup(this);
      }
      delete this->_get_dynamic_data()->cdata_;
    }
  }
#endif
  virtual void generate_value_() { 
#ifndef _SCV_INTROSPECTION_ONLY
    if (!_get_distribution()) { this->_get_dynamic_data()->dist_ = new _scv_distribution<T>; }
    _get_distribution()->generate_value_(this,this->get_constraint_data());
    return;
#endif
  }
#ifndef _SCV_INTROSPECTION_ONLY
  void set_mode(scv_extensions_if::mode_t t) {
    int lb, ub; lb =0; ub =0;
    if (!_get_distribution()) { 
      this->_get_dynamic_data()->dist_ = new _scv_distribution<T>; 
    }
    if (!check_mode(t, this, this->get_name(), _get_distribution()))
      return;
    else 
      this->get_constraint_data()->set_ext_mode(t, lb, ub);
  }
  void set_mode(scv_bag<std::pair<T, T> >& d) {
    _reset_keep_only_distribution();
    if (!_get_distribution()) { this->_get_dynamic_data()->dist_ = new _scv_distribution<T>; }
    _get_distribution()->set_mode(d,this->get_constraint_data(),this);
  }
  void set_mode(scv_bag<T>& d) {
    _reset_keep_only_distribution();
    if (!_get_distribution()) { this->_get_dynamic_data()->dist_ = new _scv_distribution<T>; }
    _get_distribution()->set_mode(d,this->get_constraint_data(),this);
  }
  void reset_distribution() {
    _reset_bag_distribution();
    _reset_keep_only_distribution();
  }
  scv_extensions_if::mode_t get_mode(void) {
    return this->get_constraint_data()->get_ext_mode();
  }
  virtual void set_distribution_from(scv_extensions_if* e) {
    _scv_distribution<T> *dist = (_scv_distribution<T>*)
      e->get_dynamic_data()->dist_;
    _set_distribution(dist);
  }
protected:
  void _reset_bag_distribution() {
    if (_get_distribution()) {
      _get_distribution()->reset_distribution();
    }
  }
  void _reset_keep_only_distribution() {
    this->get_constraint_data()->reset_distribution(this);
  }
#endif
};


#define _SCV_EXT_RAND_FC_N_D(basic_type)                        \
template<int N>  \
class scv_extension_rand<basic_type >  \
 : public _scv_extension_rand_N<basic_type > {}; \


_SCV_EXT_RAND_FC_D(bool,bool);                   
_SCV_EXT_RAND_FC_D(char,char);                   
_SCV_EXT_RAND_FC_D(unsigned char,unsigned_char);		
_SCV_EXT_RAND_FC_D(short,short);			
_SCV_EXT_RAND_FC_D(unsigned short,unsigned_short);		
_SCV_EXT_RAND_FC_D(int,int);			
_SCV_EXT_RAND_FC_D(unsigned int,unsigned_int);		
_SCV_EXT_RAND_FC_D(long,long);			
_SCV_EXT_RAND_FC_D(unsigned long,unsigned_long);		
_SCV_EXT_RAND_FC_D(long long,long_long);		
_SCV_EXT_RAND_FC_D(unsigned long long,unsigned_long_long);	
_SCV_EXT_RAND_FC_D(float,float);			
_SCV_EXT_RAND_FC_D(double,double);			
_SCV_EXT_RAND_FC_D(std::string,string);			
_SCV_EXT_RAND_FC_N_D(test_uint<N>);		


#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
_SCV_EXT_RAND_FC_1_D(sc_bit,sc_bit);		
_SCV_EXT_RAND_FC_1_D(sc_logic,sc_logic);		
_SCV_EXT_RAND_FC_N_D(sc_int<N>);		
_SCV_EXT_RAND_FC_N_D(sc_uint<N>);		
_SCV_EXT_RAND_FC_N_D(sc_bigint<N>);		
_SCV_EXT_RAND_FC_N_D(sc_biguint<N>);		
_SCV_EXT_RAND_FC_N_D(sc_bv<N>);		
_SCV_EXT_RAND_FC_N_D(sc_lv<N>);		
// SCV_EXT_RAND_FC_N_D(tag,sc_fixed<N>);		       
// SCV_EXT_RAND_FC_N_D(tag,sc_ufixed<N>);		       
_SCV_EXT_RAND_FC_D_D(sc_signed,sc_signed);		
_SCV_EXT_RAND_FC_D_D(sc_unsigned,sc_unsigned);		
_SCV_EXT_RAND_FC_D_D(sc_int_base,sc_int_base);		
_SCV_EXT_RAND_FC_D_D(sc_uint_base,sc_uint_base);		
_SCV_EXT_RAND_FC_D_D(sc_lv_base,sc_lv_base);		
_SCV_EXT_RAND_FC_D_D(sc_bv_base,sc_bv_base);		
#endif


#undef _SCV_EXT_RAND_FC_D
#undef _SCV_EXT_RAND_FC_1_D
#undef _SCV_EXT_RAND_FC_N_D



// ----------------------------------------
// wrap up this component
// ----------------------------------------
#undef _SCV_INTROSPECTION_BASE
#define _SCV_INTROSPECTION_BASE scv_extension_rand<T>
#undef _SCV_INTROSPECTION_BASE1
#define _SCV_INTROSPECTION_BASE1 scv_extension_rand<T*>
#undef _SCV_INTROSPECTION_BASE2
#define _SCV_INTROSPECTION_BASE2 scv_extension_rand<T[N]>

#undef _SCV_INTROSPECTION_BASE_ENUM
#define _SCV_INTROSPECTION_BASE_ENUM _scv_extension_rand_enum
