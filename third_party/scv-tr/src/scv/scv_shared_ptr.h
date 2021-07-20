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

  scv_shared_ptr.h -- 
  A wrapper class/template to provide a safe handle to an underlying object.

  A safe handle is one with automatic reference counting so that
  even when multiple C++ threads are sharing the underlying object,
  the system knows when to destroy the underlying object and reclaim
  the memory.

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

#ifndef INCLUDED_SCV_SHARED_PTR_H
#define INCLUDED_SCV_SHARED_PTR_H

#include <cassert>
#include <iostream>

#define LOCAL_MEMORY_MANAGEMENT \
  static int *_createCountP() { return new int(); } \
  static void _freeCountP(int *countP) { delete countP; }

template<typename T>
class scv_shared_ptr {
public:
  //
  // constructor (user-provided underlying object)
  //
  // It is the user's responsibility to make sure that no other code
  // is accessing the underlying object "core" except through safe handles.
  //
  // Typical usage:
  //   scv_shared_ptr<my_packet> packet(new my_packet());
  //
  scv_shared_ptr(T *coreP=NULL) : _coreP(coreP), _countP(NULL) {
    if (_coreP) { _countP = _createCountP(); *_countP = 1; }
  }

  scv_shared_ptr(T *coreP, int *countP) : _coreP(coreP), _countP(countP) {
    if (_countP) { ++(*_countP); }
  }

  //
  // constructor (copy)
  //
  // The new safe handle refers to the same underlying object as the
  // safe handle in the argument.
  //
  // If the argument is NULL, it is a place holder for future assignments.
  //
  scv_shared_ptr(const scv_shared_ptr<T>& t) 
    : _coreP(t._coreP), _countP(t._countP) { if (_countP) ++(*_countP); }

  //
  // destructor
  //
  ~scv_shared_ptr() {
    if (_coreP) { --(*_countP); if (*_countP == 0) { delete _coreP; _freeCountP(_countP); } }
  }

  template<typename T2>
  operator scv_shared_ptr<T2> () { return scv_shared_ptr<T2>(_coreP,_countP); }

  //
  // assignment
  //
  // The current safe handle becomes a wrapper to the underlying object 
  // of the safe handle in the argument. The reference count of the 
  // original underlying object is decrement, and the original underlying object
  // is destroyed if the reference count become zero.
  //
  scv_shared_ptr& operator=(const scv_shared_ptr& t) {
    if (t._countP != _countP) {
      if (t._coreP) ++(*t._countP);
      if (_coreP) {
	--(*_countP);
        if (*_countP == 0) { delete _coreP; _freeCountP(_countP); }
      }
      _coreP = t._coreP;
      _countP = t._countP;
    }
    return *this;
  }

  bool compare(const scv_shared_ptr& other) const {
    return this->_coreP == other._coreP;
  }

  bool isNull() const {
    return this->_coreP == NULL;
  }

  //
  // access to the underlying object
  //
  T& operator*() { 
    if (_coreP) return *_coreP;
    assert(0);
    return *(T*)0; // will cause segmentation fault when this error occurs.
  }
  const T& operator*() const {
    if (_coreP) return *_coreP;
    assert(0);
    return *(T*)0; // will cause segmentation fault when this error occurs.
  }
  T *operator->() {
    if (_coreP) return _coreP;
    assert(0);
    return NULL ;
  }
  const T *operator->() const {
    if (_coreP) return _coreP;
    assert(0);
    return NULL ;
  }
#if 0
  friend ostream& operator<<(ostream& os, const scv_shared_ptr& a) {
    os << *a._coreP << endl;
    return os;
  }
#endif

private:
  T *_coreP;
  int *_countP;

  LOCAL_MEMORY_MANAGEMENT
};

#undef LOCAL_MEMORY_MANAGEMENT

template <class T>
bool operator==(const scv_shared_ptr<T>& a, const scv_shared_ptr<T>& b) {
  return a.compare(b);
}
template <class T>
bool operator!=(const scv_shared_ptr<T>& a, const scv_shared_ptr<T>& b) {
  return !(a==b);
}
template <class T>
ostream& operator<<(ostream& os, const scv_shared_ptr<T>& a) {
    //os << *a._coreP << endl;
    os << *a << endl;
    return os;
}

#endif // INCLUDED_SCV_SHARED_PTR_H
