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

  _scv_smart_ptr -- The main implementation for the scv_smart_ptr class.

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
// implementation of the access methods to static extensions
// ----------------------------------------
template <typename T>
scv_extensions<T> scv_get_extensions(T& d) {
  scv_extensions<T> e;
  e._set_instance(&d);
  return e;
};

template <typename T>
const scv_extensions<T> scv_get_const_extensions(const T& d) {
  scv_extensions<T> e;
  e._set_instance((T*)&d);
  return e;
};

// ----------------------------------------
// implementation of scv_smart_ptr
// ----------------------------------------
extern void _scv_insert_smart_ptr(scv_smart_ptr_if *);

template <typename T> scv_smart_ptr<T>::scv_smart_ptr()
  : data_(new T()),
    ext_(new scv_extensions<T>()),
    tmp_(&*ext_) {
  ext_->_set_instance(&*data_);
  init();  
}

template <typename T>
scv_smart_ptr<T>::scv_smart_ptr(const std::string& name)
  : data_(new T()),
    ext_(new scv_extensions<T>()),
    tmp_(&*ext_) {
  ext_->_set_instance(&*data_);
  ext_->set_name(name.c_str());
  init();  
}

template <typename T>
scv_smart_ptr<T>::scv_smart_ptr(const char * name)
  : data_(new T()),
    ext_(new scv_extensions<T>()),
    tmp_(&*ext_) {
  ext_->_set_instance(&*data_);
  ext_->set_name(name);
  init();  
}

template <typename T> scv_smart_ptr<T>::scv_smart_ptr(T * data)
  : data_(data),
    ext_(new scv_extensions<T>()),
    tmp_(&*ext_) {
  ext_->_set_instance(data);
  init();  
}

template <typename T> scv_smart_ptr<T>::scv_smart_ptr(T * data, const std::string& name)
  : data_(data),
    ext_(new scv_extensions<T>()),
    tmp_(&*ext_) {
  ext_->_set_instance(data);
  ext_->set_name(name.c_str());
  init();  
}

template <typename T> scv_smart_ptr<T>::scv_smart_ptr(T * data, const char * name)
  : data_(data),
    ext_(new scv_extensions<T>()),
    tmp_(&*ext_) {
  ext_->_set_instance(data);
  ext_->set_name(name);
  init();  
}

template <typename T> scv_smart_ptr<T>::scv_smart_ptr(const scv_smart_ptr<T>& rhs) 
  : data_(rhs.data_), ext_(rhs.ext_), tmp_(&*ext_) {
}

template <typename T> scv_smart_ptr<T>::scv_smart_ptr(
  scv_shared_ptr<T> data, 
  scv_shared_ptr< scv_extensions<T> > ext,
  scv_extensions<T> *tmp
) : data_(data), ext_(ext), tmp_(tmp)
{ init(); }

void _scv_constraint_wrapup(scv_extensions_if* e);

template <typename T> scv_smart_ptr<T>::~scv_smart_ptr() {
#ifndef _SCV_INTROSPECTION_ONLY
  //_scv_constraint_wrapup(&*ext_);
#endif
}

template <typename T> void scv_smart_ptr<T>::init() {
  ext_->_set_dynamic();
#ifndef _SCV_INTROSPECTION_ONLY
  ::_scv_insert_smart_ptr(this);
#endif
}

template <typename T> scv_smart_ptr<T>& 
scv_smart_ptr<T>::operator=(const scv_smart_ptr<T>& rhs) {
  data_ = rhs.data_; // shared object
  ext_ = rhs.ext_;   // shared extension
  tmp_ = rhs.tmp_;
  return *this;
}

template<typename T> scv_extensions<T*>& 
scv_extensions<T*>::operator=(const scv_smart_ptr<T>& rhs) {
  *this->_get_instance() = (T*) rhs->_get_instance();
  if (rhs->is_dynamic()) {
    // share the same extension until the object disapear
    // (in practise (but slow), probably need to register
    // a deletion callback.
    this->_own_typed_ptr = false;
    this->_ptr = (scv_extensions<T>*) &*rhs;
    this->_typed_ptr = (scv_extensions<T>*) &*rhs;
  }
  _set_ptr();
  this->trigger_value_change_cb();
  return *this;
}

template <typename T> scv_smart_ptr<T>::scv_smart_ptr(scv_shared_ptr<T> data) 
  : data_(data), ext_(new scv_extensions<T>()), tmp_(&*ext_) {
  _scv_message::message(_scv_message::INTERNAL_ERROR,"scv_smart_ptr(scv_shared_ptr) should not be called.");
  ext_->_set_instance(&*data_);
  init();
}

// ----------------------------------------
// special extension class to handle getting an extension from a smart pointer
// ----------------------------------------
template<typename T>
class scv_extensions< scv_smart_ptr<T> > : public scv_extensions<T*> {
public:
  scv_extensions() {}
  scv_extensions(const scv_extensions<T*>& rhs) : scv_extensions<T*>(rhs) {}
  virtual ~scv_extensions() {}
  scv_extensions& operator=(const scv_extensions<T*>& rhs) {
    return scv_extensions<T*>::operator=(rhs);
  }
  scv_extensions& operator=(const T * rhs) {
    return scv_extensions<T*>::operator=(rhs);
  }
  operator const T *() const { return *scv_extensions<T*>::_get_instance(); }
  scv_expression operator()() { return scv_extensions<T*>::form_expression(); }

  virtual void _set_instance(T ** i) {
    scv_extensions<T*>::_set_instance(i);
  }  
  virtual void _set_instance(scv_smart_ptr<T> * i) {
    scv_extensions<T*>::_set_instance(i->get_extension_ptr()->_get_instance());
  }  
};
