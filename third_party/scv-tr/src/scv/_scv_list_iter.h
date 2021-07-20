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

  _scv_list_iter.h -- 

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Torsten Maehne
                               Universite Pierre et Marie Curie, 2013-12-02
  Description of Modification: Fix the equality operator implementation of the
                               different iterators.

 *****************************************************************************/

#include <list>
#include <iostream>

//
// scv_bag_iter class: STL-like iteratorT class to iterate on the bag 
// elements. Used by scv_bag. Should not be used directly.
//
template <class T>
class scv_bag_iter { 
        typedef typename std::list<_scv_bag_record<T> >::const_iterator literBRCT ;
        typedef typename std::list<_scv_bag_record<T> >::iterator literBRT ;
private:
  literBRT  _current;
  int _objectCount; // Keeps track of no. of object copies.
public:
  // Constructors...
  scv_bag_iter() {}
  //
  scv_bag_iter(literBRT iter) :
    _current(iter), _objectCount(1) {}
  // 
  scv_bag_iter(const scv_bag_iter& other) : 
    _current(other._current), _objectCount(other._objectCount)  { }

  // 
  scv_bag_iter& operator=(const scv_bag_iter& other) { 
    if (this != &other) { 
      _current = other._current;
      _objectCount = other._objectCount;
    } 
    return *this;
  } 

  // Assign listIterator to BagIterator leaving the count field untouched
  scv_bag_iter& operator=(const literBRT& other) { 
    if ((*this)._current != other) { 
      _current = other;
    } 
    return *this;
  } 

  //Assign count field to BagIterator
  scv_bag_iter&  operator=(int count){
    (*this)._objectCount = count;
    return *this;
  }

  // Dereference
  _scv_bag_record<T>& operator*() { return (*_current); } 
  // Test for equality
  friend bool operator==<>(const scv_bag_iter<T>& first,const scv_bag_iter<T>& second);
  // prefix ++
  scv_bag_iter& operator++() { 
    if (++_objectCount > (*_current).count()) {  //should advance
      ++_current;
      _objectCount = 1;
    } 
    return (*this);
  }
  // postfix ++
  scv_bag_iter operator++(int) { 
    scv_bag_iter tmp = *this;
    ++(*this);
    return tmp;
  }

  scv_bag_iter& dInc() { 
    ++_current;
    _objectCount = 1;
    return (*this);
  }

  scv_bag_iter& operator--() { 
    if (--_objectCount == 0) {  //should decrement main iteratorT
      --_current;
      _objectCount = (*_current).count();
    } 
    return (*this);
  }
  // postfix ++
  scv_bag_iter operator--(int) { 
    scv_bag_iter tmp = *this;
    --(*this);
    return tmp;
  }

  scv_bag_iter& dDec() { 
      --_current;
      _objectCount = (*_current).count();
    return (*this);
  }

  // pointer dereference
  const T* operator->() { 
    return &(*_current).element();
  } 

  bool operator==(literBRT& l) const {
    return (*this)._current == l;
  }

  literBRT& current() {return _current; };
  
  int& objCount() {
    return _objectCount ;
  };

};

template <class T>
bool operator==(const scv_bag_iter<T>& first, const scv_bag_iter<T>& second) {
  return (first._current == second._current) 
    &&   (first._objectCount == second._objectCount);
}

//
// Const version of the above....
// 
template <class T>
class scv_bag_const_iter { 
        typedef typename std::list<_scv_bag_record<T> >::const_iterator literBRCT ;
        typedef typename std::list<_scv_bag_record<T> >::iterator literBRT ;
private:
  literBRCT _current;
  int _objectCount;
public:
  scv_bag_const_iter() {}
  scv_bag_const_iter(literBRCT iter):
    _current(iter), _objectCount(1)  { }
  //
  scv_bag_const_iter(const scv_bag_const_iter& other) : 
    _current(other._current), _objectCount(other._objectCount)  { }
  // 


  scv_bag_const_iter& operator=(const scv_bag_const_iter& other) { 
    if (this != &other) { 
      _current = other._current;
      _objectCount = other._objectCount;
    } 
    return *this;
  } 

  // Assign list const_iterator to Bag const iterator leaving count unassigned
  scv_bag_const_iter& operator=(const literBRCT& other) { 
    if ((*this)._current != other) { 
      _current = other;
    } 
    return *this;
  } 

  // Assign an integerto objectCount leave the const_iterator untouched
  scv_bag_const_iter&  operator=(int count){
    (*this)._objectCount = count;
    return *this;
  }


  // Dereference
  const _scv_bag_record<T>& operator*() { return (*_current); } 

  // Test for equality
  friend bool operator==<>(const scv_bag_const_iter<T>& first,const scv_bag_const_iter<T>& second);
  // prefix ++
  scv_bag_const_iter& operator++() { 
    if (++_objectCount > (*_current).count()) {  //should advance
      ++_current;
      _objectCount = 1;
    } 
    return (*this);
  }
  // postfix ++
  scv_bag_const_iter operator++(int) { 
    scv_bag_const_iter tmp = *this;
    ++(*this);
    return tmp;
  }

  // Distinct increment
  scv_bag_const_iter& dInc() { 
    ++_current;
    _objectCount = 1;
    return (*this);
  }


  scv_bag_const_iter& operator--() { 
    if (--_objectCount == 0) {  //should decrement main iteratorT
      --_current;
      _objectCount = (*_current).count();
    } 
    return (*this);
  }
  // 
  scv_bag_const_iter operator--(int) { 
    scv_bag_const_iter tmp = *this;
    --(*this);
    return tmp;
  }

  scv_bag_const_iter& dDec() { 
      --_current;
      _objectCount = (*_current).count();
    return (*this);
  }

  // pointer dereference
  const T* operator->() { 
    return &(*_current).element();
  } 

  // compare list constant iterator with bag constant iterator
  bool operator==(literBRCT& l) const {
    return (*this)._current == l;
  }

  literBRCT& current() { return _current ;} 

  int& objCount() {
    return _objectCount ;
  }
};

template <class T>
bool operator==(const scv_bag_const_iter<T>& first, const scv_bag_const_iter<T>& second) {
  return (first._current == second._current) 
    &&   (first._objectCount == second._objectCount);
}


// Peek Iterator

template <class T>
class scv_peek_bag_iter { 
        typedef typename std::list<_scv_bag_record<T> >::const_iterator lbrIterCT ;
	typedef typename std::list<_scv_bag_record<T> >::iterator lbrIterT ;
private:
	/*  lbrIterCT& _peek;
  lbrIterT& _modify;
  literBRCT _lpeek;
  literBRT _lmodify; */
  lbrIterT _peek ;
  int _mObjectCount, _uObjectCount;
  bool _mflag;
public:
  
  scv_peek_bag_iter() {} ;
  scv_peek_bag_iter(lbrIterT iter):
    _peek(iter), _mObjectCount(0), _uObjectCount(1), _mflag(false)  { } ;
  //

  scv_peek_bag_iter(const scv_peek_bag_iter& other) : 
    _peek(other._peek)
    , _mObjectCount(other._mObjectCount), _uObjectCount(other._uObjectCount)  { }


  scv_peek_bag_iter& operator=(const scv_peek_bag_iter& other) { 
    if (this != &other) { 
      _peek = other._peek;
      _mObjectCount = other._mObjectCount;
      _uObjectCount = other._uObjectCount;
      _mflag = other._mflag;
    } 
    return *this;
  }

  // Assign list const_iterator to Bag const iterator leaving count unassigned
  scv_peek_bag_iter& operator=(const lbrIterT& other) { 
    _peek = other;
    return *this;
  } 


  // Dereference
  const _scv_bag_record<T>& operator*() { return (*_peek); } 
  /*
  // Test for equality
  friend bool operator==<>(const scv_peek_bag_iter<T>& first,const scv_peek_bag_iter<T>& second);
  */ //commented by dsb Apr 05, 2000 As it is not used and seems to be giving
  //  hp 2.95 compile problems
  // init counters during increment and decrement

  inline  void lInitCount() {
      _uObjectCount = 0 ; 
      _mObjectCount = 0 ;
  }

  inline void mInitCount() {
    _mObjectCount = (*_peek).mCount();
    _uObjectCount = (*_peek).uCount();
    _mflag = ((*_peek).mCount() ? true : false) ;
  }

  // prefix ++

  bool incCountAny(){
    if (_uObjectCount < (*_peek).uCount()) {
      _uObjectCount++ ;
      _mflag = false;
      return true;
    }
    if (_mObjectCount < (*_peek).mCount()) {
      _mObjectCount++ ;
      _mflag = true ;
      return true ;
    }
    return false;
  }

  scv_peek_bag_iter& operator++() { 
    if (!incCountAny()) {  //should advance
      ++_peek;
      lInitCount();
      incCountAny();
    }
    return (*this);
  }

  // postfix ++
  scv_peek_bag_iter operator++(int) { 
    scv_peek_bag_iter tmp = *this;
    ++(*this);
    return tmp;
  }

  // Distinct increment

  scv_peek_bag_iter& dInc() { 
    ++_peek;
    lInitCount();
    return (*this);
  }


  scv_peek_bag_iter& operator--() { 
    if (_mObjectCount > 0 ) {
      --_mObjectCount ;
	_mflag = true ;
    }
    else if  (_uObjectCount > 0 ) {
      --_uObjectCount ;
      	_mflag = false ;
    }
    else {  //should decrement main iteratorT
      --_peek;
       mInitCount();
    } ;
    return (*this);
  }
  // 
  scv_peek_bag_iter operator--(int) { 
    scv_peek_bag_iter tmp = *this;
    --(*this);
    return tmp;
  }

  scv_peek_bag_iter& dDec() { 
      --_peek;
      mInitCount();
    return (*this);
  }

  // pointer dereference
  const T* operator->() { 
    return &(*_peek).element();
  } 

  // compare list iterator with bag peek iterator
  bool operator==(lbrIterT& l){
    return (*this)._peek == l;
  }

  lbrIterT& modify() { return _peek ;  } 
  lbrIterT& peek() { return _peek ;} 

  int objCount() {
    return _uObjectCount + _mObjectCount ;
  }

  int& uObjCount() {
    return _uObjectCount ;
  }

  int& mObjCount() {
    return _mObjectCount ;
  }

  bool mflag() {
    return _mflag ;
  }
};
