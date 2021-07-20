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

  scv_bag.h -- 
  A bag is a collection of objects, where two objects may look
  exactly the same (cf. a set). It allows prioritized/random access to the
  objects. It also allows marking of elements so as to avoid repeated
  access to the same object.

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Torsten Maehne,
                               Universite Pierre et Marie Curie, 2013-05-28
  Description of Modification: Fix initialization of _randomP member variable
                               in the scv_bag copy constructor.
                               Fix memory leak due to forgotten delete on the
                               scv_bag::_randomP member variable in the
                               scv_bag destructor if it is owned by the
                               scv_bag instance.
                               Default initialize _lastPeekIsMarked and
                               _lastPeek member variables in the
                               scv_bag constructors to avoid any surprises.

 *****************************************************************************/

#ifndef SCV_BAG_H
#define SCV_BAG_H

#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <list>
#include "scv/scv_random.h"
#include "scv/scv_util.h"
#include "scv/scv_report.h" 

//
// A bag is implemented as a priority list of bagRecords. A _scv_bag_record class 
// implements the multiple copies and different marked/unmarked state of a bag
// element.
//
template <class T>
class _scv_bag_record { 
private:
  T   _element;
  int _totalCount;
  int _unmarkedCount;
public:
  // constructors
  _scv_bag_record() :  _element(T()), _totalCount(1), _unmarkedCount(1) { }
  _scv_bag_record(const T& arg, const int count=1) : _element(arg), 
    _totalCount(count), _unmarkedCount(count) { } 
  _scv_bag_record(const _scv_bag_record& other) : _element(other._element),
    _totalCount(other._totalCount), _unmarkedCount(other._unmarkedCount) {}

  // destructor
  ~_scv_bag_record() {} ; 

  // retreiving object and counts
  T&  element() { return _element; } 
  const T&  element() const { return _element; } 
  int count() const { return _totalCount; }
  int uCount() const { return _unmarkedCount; }
  int mCount() const { return _totalCount - _unmarkedCount; }


  // popCount decrements object counts and reset unmarkedCount for object 
  // if needed
  void popCount(bool marked, bool All, int& size, int& umarkSize, int& peekuCount, int& peekmCount) { 
    if (All) {
      size -= _totalCount ;
      umarkSize -= _unmarkedCount ;
      _totalCount = 0 ;
      _unmarkedCount = 0;
      peekuCount = 0;
      peekmCount = 0;
    } else {
      if (marked && mCount()) --peekmCount;
      else if (!marked && uCount()) {
	--_unmarkedCount ;
	--umarkSize;
	--peekuCount;
      } else if (!marked) {
	_scv_message::message(_scv_message::BAG_ZERO_UNMARKED_OBJECTS,"pop");
      } else if (marked) {
	_scv_message::message(_scv_message::BAG_ZERO_MARKED_OBJECTS,"pop");
      }
      --_totalCount; --size ; 
    }
  }

  // mark/unmark element
  int  markElement(bool All = false ) { 
    if (All) {
      int count = _unmarkedCount ;
      _unmarkedCount = 0 ; 
      return count ;
    }
    else if (_unmarkedCount > 0) {
      _unmarkedCount--; 
      return 1 ;
    } else return 0 ;
  } 

  int unmarkElement(bool All = false) { 
    if (All) {
      int mcount = _totalCount - _unmarkedCount ;
      _unmarkedCount = _totalCount; 
      return mcount ;
    }
    else if (_unmarkedCount < _totalCount) {
      _unmarkedCount++ ;
      return 1 ;
    } else return 0 ;
  } 

  bool is_equal(const _scv_bag_record& other) const ;
  //  friend std::ostream& operator<<(std::ostream& out, const _scv_bag_record<T>& b); 
  void print(std::ostream& o=scv_out, int details=0, int indent=0) const;
};

template <class T>
bool _scv_bag_record<T>::is_equal(const _scv_bag_record<T>& other)  const { 
  return  (this->_element == other._element) ;
} 

template <class T>
bool operator==(const _scv_bag_record<T>& a, const _scv_bag_record<T>& b){ 
  return  (a.is_equal(b));
} 


#include "scv/_scv_list_iter.h"


//
// the parameter to the template, "T", must be either builtin C++ type
// or a class with copy constructor, assignment, and equality
// comparison.
//
// Implementation uses a _scv_associative_array<T <no_of_copies, iref> > where 
// iref is an index-like for the object T. no_of_copies elements with value 
// of iref are pushed on the list<int>. Peeking is equivalent to
// retreiving the element at the front of the list. 
// To allow cross-referencing from irefs to the objects, a second 
// _scv_associative_array<int, T> is used. 
//
// New implementation uses a priority list of bag records. A bag record is 
// a structure of the element plus the number of copies plus the number of 
// marked elements. 
// 


template <class T>
class scv_bag : public _scv_data_structure {
  typedef int (*priorityFuncT)(const T&, const T&);
public:
  virtual const char *kind() const
    { static const char *name="scv_bag"; return name; }

private:
  typedef typename std::list<_scv_bag_record<T> >::iterator _bIterT;
  typedef typename std::list<_scv_bag_record<T> >::const_iterator _bCIterT;
  typedef scv_peek_bag_iter<T>  peekIterT;
  // Bag data
  std::list<_scv_bag_record<T> > _bag;
  int _size;
  int _dsize;
  int _unmarkedSize;
  // Peek iter data outside constness checking
  mutable bool _lastPeekIsMarked;
  mutable bool _lastPeekValid;
  mutable peekIterT _lastPeek;
  mutable bool _randMode;
  mutable bool _rlock;
  mutable bool _randomOwner;
  unsigned long _seed;
  scv_random* _randomP;

inline  void incPeek() const {
  ++_lastPeek;
  if (_lastPeek.peek() == ((scv_bag*) this)->_bag.end()) 
    _lastPeek.peek() = ((scv_bag*) this)->_bag.begin();	
}


inline  void dIncPeek()  const {
  _lastPeek.dInc();
  if (_lastPeek.peek() == ((scv_bag*) this)->_bag.end()) 
    _lastPeek.peek() = ((scv_bag*) this)->_bag.begin();	
}

  // if valid peek increment else reset to begin

  inline void nPeek0(bool distinct = false) const {
    if (!_lastPeekValid) {
      _lastPeek.peek() = ((scv_bag*) this)->_bag.begin();
      _lastPeek.lInitCount();
    } else if (distinct) dIncPeek();
  }


inline void nextPeek(bool distinct= false) const {
  nPeek0(distinct);
  incPeek();
}


  // increments _lastPeek until it points to an object with marked copies
inline  void markedPeek(bool distinct) const {
  nPeek0(distinct);
  while( _lastPeek.mObjCount() >= (*_lastPeek).mCount()) dIncPeek();

  (_lastPeek.mObjCount())++ ;

}

  // increments _lastPeek until it points to an object with unmarked copies
inline  void unmarkedPeek(bool distinct) const {
  nPeek0(distinct);

  while( _lastPeek.uObjCount() >= (*_lastPeek).uCount()) dIncPeek();
  
  (_lastPeek.uObjCount())++ ;

}

inline  void decPeek() const {
  if (_lastPeek.peek() == ((scv_bag*) this)->_bag.begin()) 
    _lastPeek.peek() = ((scv_bag*) this)->_bag.end();	
  --_lastPeek;
}

inline  void dDecPeek()  const {
  if (_lastPeek.peek() == ((scv_bag*) this)->_bag.begin()) 
  _lastPeek.peek() = ((scv_bag*) this)->_bag.end();	
  _lastPeek.dDec();
}

  // if valid peek decrement else reset to begin
  // The symmetry is violated here

inline void prevPeek(bool distinct=false)  const {
  if (_lastPeekValid) if (distinct) dDecPeek(); else decPeek();
  else {
    _lastPeek.peek() = ((scv_bag*) this)->_bag.begin();
    _lastPeek.lInitCount();
  }
}

  // remove the record from the underlying
  // container class implementing bag
  // and update all bookkeeping data members like _size/_dsize/_unmarkedSize

void _bagErase() { 
  //  cout << "DEBUG bag size before erase" << _bag.size() << std::endl ;
  if (_lastPeekValid) {

    _lastPeekIsMarked = false;
    _unmarkedSize -= ((*_lastPeek).uCount());
    _size -= ((*_lastPeek).count());
    --_dsize;
    
    if (_randMode || _size == 0 ||
	_lastPeek.peek() == _bag.begin()) {
      _bag.erase(_lastPeek.modify());
      _lastPeekValid = false;
    } else {
      _bIterT eraseIter = _lastPeek.modify() ;
      dDecPeek();
      _bag.erase(eraseIter);
    }
  } else { 
    _scv_message::message(_scv_message::BAG_INVALID_PEEK_ERASE,nameP());
  }
  //  cout << "DEBUG bag size after erase" << _bag.size() << std::endl ;
}



public:
  
  //  enum markStatusT { unMarked, marked, any} ;

  //
  // main constructor: Note that for the initial release we will not support 
  // custom priority bags. 
  //
  scv_bag(const char *nameP="<anonymous>",
	  unsigned long seed = 0) :
    _scv_data_structure(nameP),
    _bag(),
    _size(0), 
    _dsize(0),
    _unmarkedSize(0),
    _lastPeekIsMarked(),
    _lastPeekValid(false),
    _lastPeek(),
    _randMode(true),
    _rlock(true),
    _randomOwner(false),
    _seed(seed),
    _randomP(NULL) { 
  }
  // Copy Constructor
  scv_bag(const scv_bag& other, const char *nameP="<anonymous>") : 
    _scv_data_structure(nameP), 
    _bag(other._bag), 
    _size(other._size), 
    _dsize(other._dsize),
    _unmarkedSize(other._unmarkedSize),
    _lastPeekIsMarked(),
    _lastPeekValid(other._lastPeekValid),
    _lastPeek(other._lastPeek),
    _randMode(true),
    _rlock(true),
    _randomOwner(other._randomOwner),
    _seed(other._seed),
    _randomP(NULL) {
    if (_randomOwner)
      _randomP = new scv_random(*other._randomP);
  } 
  // Assignement
  scv_bag& operator=(const scv_bag& other) { 
    if (this != &other) { 
      _bag = other._bag;
      _size = other._size;
      _dsize = other._dsize;
      _unmarkedSize = other._unmarkedSize;
      _lastPeekValid = other._lastPeekValid;
      _lastPeek = other._lastPeek;
      _randMode = other._randMode; 
      _rlock = other._rlock;
    } 
    return *this;
  }
 
  //
  // virtual destructor
  //
  ~scv_bag() {
    if (_randomOwner) {
      delete _randomP;
    }
  }

  //
  // return the number of objects in the bag: We keep a tally instead of
  // traversing the map each call summing up the counts.
  //
  int size() const { return _size; } 
  
  // 
  // return true if there is nothing in the bag
  //
  bool empty() const { return _size == 0; } 

  //
  // return the number of unmarked objects: They all should be in the 
  // 
  //
  int unMarkedSize() const { return _unmarkedSize; } 
  int unmarked_size() const { return _unmarkedSize; }
  int uSize() const {return _unmarkedSize; }

  int mSize() const { return _size - _unmarkedSize; } 
  int markedSize() const { return _size - _unmarkedSize; } 
  int marked_size() const { return _size - _unmarkedSize; }

  int dSize() const { return  _dsize; } 
  int distinctSize() const { return  _dsize; } 
  int distinct_size() const { return _dsize; }


  void resetPeek () const {
    _lastPeekValid = false;
    _lastPeekIsMarked = false;
    _rlock = false ;
    _randMode = true ;
  }
  void reset_peek () const {
    resetPeek();
  }
  bool validPeek() const { return _lastPeekValid; }
  bool valid_peek() const { return _lastPeekValid; }
  
  const T& peek() const { 
    if (_lastPeekValid) return (*_lastPeek).element();
    else {
      _scv_message::message(_scv_message::BAG_INVALID_PEEK_RETURN,nameP());
      return (*_lastPeek).element();
    }
  };

  const int count() const { return (*_lastPeek).count(); }

  //
  // Add identical objects into the bag 
  // The number of objects added is specified by "num" >= 1.
  //

  void add(const T& arg, int num=1) { 
    if (num > 0) {
    _scv_bag_record<T> b(arg, num);
    _bag.push_back(b) ;
    _size += num; 
    ++_dsize;
    _unmarkedSize += num;
    return;
    } else {
      _scv_message::message(_scv_message::BAG_INVALID_ADD_ARGUMENT,num);
    }
  } 
      
  void push(const T& arg, int num = 1) { add(arg, num); }

  //
  // Look into bag and examine one object. Should be const as T object is 
  // a map Key!
  // 

  const T& peekRandom(bool mark = false) { 
    if (!_randomP) {
      _randomOwner = true;
      _randomP = new scv_random(nameP(),_seed);
    }
    if (_unmarkedSize == 0) {
      _scv_message::message(_scv_message::BAG_ZERO_UNMARKED_OBJECTS,"peekRandom");
      return (*_bag.end()).element(); // This is to keep the compiler happy
    }
    int i = abs((int)(_randomP->next()) % _unmarkedSize);
    _bIterT iter;
    for (iter = _bag.begin();  iter != _bag.end(); ++iter) { 
      if (i < (*iter).uCount()) { 
	_lastPeek = iter;
	_lastPeek.mObjCount() = 0;
	_lastPeek.uObjCount() = i + 1;
	_lastPeekValid = true;
	_lastPeekIsMarked = false ;
	_randMode = true;
	_rlock = false ;
	if (mark) scv_bag::mark();
	return (*iter).element();
      } else { 
	i -= (*iter).uCount();
      }
    }
    return (*_bag.end()).element();
  }
  const T& peek_random(bool mark = false) { 
    return peekRandom(mark);
  } 

  const T& peekRandom()  const { 
    if (!_randomP) {
      _randomOwner = true;
      _randomP = new scv_random(nameP(),_seed);
    }
    if (_unmarkedSize == 0) {
      _scv_message::message(_scv_message::BAG_ZERO_UNMARKED_OBJECTS,"peekRandom");
      return (*_bag.end()).element(); // This is to keep the compiler happy
    }
    int i = abs((int)(_randomP->next()) % _unmarkedSize);
    _bIterT iter;
    for (iter = ((scv_bag*) this)->_bag.begin();  
	 iter !=((scv_bag*) this)->_bag.end(); 
	 ++iter) { 
      if (i < (*iter).uCount()) { 
	_lastPeek = iter;
	_lastPeek.mObjCount() = 0;
	_lastPeek.uObjCount() = i + 1;
	_lastPeekValid = true;
	_lastPeekIsMarked = false ;
	_randMode = true;
	_rlock = false ;
	return (*iter).element();
      } else { 
	i -= (*iter).uCount();
      }
    }
    return (*_bag.end()).element(); 
  }

  const T& peek_random()  const { 
    return peekRandom();
  }

void print (std::ostream& o=scv_out, int details=0, int indent=0) const;
void show (int details=0, int indent=0) const;

private:
  // Next Unmarked Object

  const T& peekNextUnmarked(bool distinct= false) const {
    if (_unmarkedSize > 0){
      // Marked Objects exists
      //      nextPeek(distinct);
      unmarkedPeek(distinct) ;
      _lastPeekValid = true;
      _lastPeekIsMarked = false ;
      _randMode = false;
      _rlock = false ;
      return (*_lastPeek).element() ;

      // Marked objects do not exist
    } else {
      _scv_message::message(_scv_message::BAG_ZERO_UNMARKED_OBJECTS,"peakNextUnmarked");
      return (*_lastPeek).element() ; // returning garbage
    } // throw tbvInvalidObject();
  }

  // Next Marked Object 
  const T& peekNextMarked(bool distinct = false) const {
    if (_unmarkedSize < _size){
      // Marked Objects exists
      //      nextPeek(distinct);
      markedPeek(distinct) ;
      _lastPeekValid = true;
      _lastPeekIsMarked = true ;
      _randMode = false;
      _rlock = false;
      return (*_lastPeek).element() ;

      // Marked objects do not exist
    } else {
      _scv_message::message(_scv_message::BAG_ZERO_MARKED_OBJECTS,"peakNextMarked");
      return (*_lastPeek).element() ; // returning garbage
    } // throw tbvInvalidObject();
  }


  const T& peekNextAny(bool distinct= false) const {
    if (_size >0 ){
      nextPeek(distinct);
      _lastPeekValid = true;
      _lastPeekIsMarked = _lastPeek.mflag();
      //      cout << "lastPeekIsMarked " << _lastPeekIsMarked << std::endl ;
      //      cout << "peekuCount " << _lastPeek.uObjCount() << " uCount " << (*_lastPeek).uCount() << std::endl ;
      _randMode = false;
      _rlock = false ;
      return (*_lastPeek).element();
    } else {
      _scv_message::message(_scv_message::EMPTY_BAG,nameP());
      return (*_lastPeek).element() ; // returning garbage
    } 
  }

public:
  //  const T& peekNext(markStatusT peekAt = unMarked) { 
  const T& peekNext(char peekAt = 'a', bool distinct = false) const { 
    switch (peekAt) {
    case 'u': return peekNextUnmarked(distinct);
    case 'm': return peekNextMarked(distinct);
    default: return peekNextAny(distinct);
    }
  }
  const T& peek_next(char peekAt = 'a', bool distinct = false) const { 
     return peekNext(peekAt, distinct);
  }

  //
  // peek and mark the object so it would not be picked next; i.e. remove the
  // element from the _unmarked list. 
  // 
    void mark(bool AllCopies = false)  { 
      if (_lastPeekValid ) {
	if (_lastPeekIsMarked == false || AllCopies) {
	  //	  _unmarkedSize -= (*_lastPeek).markElement(AllCopies);
	  int nmark = (*(_lastPeek.modify())).markElement(AllCopies);
	  _unmarkedSize -= nmark;
	  _lastPeek.uObjCount() -=nmark;
	  _lastPeek.mObjCount() +=nmark;
	  _lastPeekIsMarked = true;
	}     
      } else { 
	_scv_message::message(_scv_message::BAG_INVALID_PEEK_MARK,nameP());
      }
    }

  void unMark(bool AllCopies = false) {
    if (_lastPeekValid) {
      if (_lastPeekIsMarked || AllCopies) {
	//	  _unmarkedSize += (*_lastPeek).unmarkElement(AllCopies);
	int nUmark = (*(_lastPeek.modify())).unmarkElement(AllCopies);
	  _unmarkedSize += nUmark;
	  _lastPeek.uObjCount() +=nUmark;
	  _lastPeek.mObjCount() -=nUmark;
	  _lastPeekIsMarked = false;
      }
    } else { 
      _scv_message::message(_scv_message::BAG_INVALID_PEEK_UNMARK,nameP());
    }
  }
  void unmark(bool AllCopies = false) {
    unMark(AllCopies);
  }

  //
  // Remove last object returned by peek(). 
  // 

  void remove(bool AllCopies = false) { 
    if (_lastPeekValid && !_rlock) { 
      (*(_lastPeek.modify())).popCount(_lastPeekIsMarked, AllCopies, _size, _unmarkedSize
				       , _lastPeek.uObjCount(), _lastPeek.mObjCount());
      //      cout << "DEBUG " << "AFter popCount is " << (*_lastPeek).count() << std::endl ;
      if ((*_lastPeek).count() == 0) _bagErase();
      if (_randMode) _lastPeekValid = false;
      _rlock = true ;
      return ;
    } else {
      _scv_message::message(_scv_message::BAG_INVALID_PEEK_ERASE,nameP());
      return;
    }
  } 

  //
  // reset to an empty bag
  //
  void clear() { 
    _bag.clear();
    _size = 0;
    _dsize = 0 ;
    _unmarkedSize = 0;
    _lastPeekIsMarked = false;
    _lastPeekValid = false;
    _randMode = true;
    return;
  } 
  // 
  // 

  void unmarkAll() { 
    _bIterT iter;
    for (iter = _bag.begin();
	 iter != _bag.end();
	 ++iter) (*iter).unmarkElement(true);
    _unmarkedSize = _size;
    _lastPeekIsMarked = false;
  }
  void unmark_all() {
    unmarkAll();
  }
  void markAll() { 
    _bIterT iter;
    for (iter = _bag.begin();
	 iter != _bag.end();
	 ++iter) (*iter).markElement(true);
    _unmarkedSize = 0;
    _lastPeekIsMarked = true;
  } 
  void mark_all() {
    markAll();
  }
  
  bool is_equal(const scv_bag<T>& other) const;

  bool operator!=(const scv_bag<T>& other);

  // 
  // virtual methods from the interface of _scv_data_structure;
  // see _scv_data_structure for details.
  //
  // virtual void traceOn();
  // virtual void traceOff();
  // 
  // Prints the elements in the bag. Multiple copies get multiple prints...
  //
  void read(std::istream& ) ;

public:
  void setRandom(scv_random& random) {
    if (_randomOwner) {
      delete _randomP;
      _randomOwner = false;
    }
    _randomP = &random;
  }
  void set_random(scv_random& random) {
    setRandom(random);
  } 
};

template <class T>
void _scv_bag_record<T>::print(std::ostream& out, int details, int indent) const {
    //out << element() ;
    out << " Count: " << count() << " unmarked: " << uCount() << std::endl; 
}

template <class T>
std::ostream& operator<<(std::ostream& out, const _scv_bag_record<T>& b) { 
  b.print(out, 1);
  return out;
}

// 
// Prints the elements in the bag. Multiple copies get multiple prints...
//

template <class T>
void scv_bag<T>::print(std::ostream& out, int details, int indent) const {
  out << kind() << " Name: " <<  nameP() << std::endl;
  for( typename std::list<_scv_bag_record<T> >::const_iterator iter = _bag.begin();
       iter != _bag.end();
       ++iter) (*iter).print(out,details,indent);
  out << std::endl << std::endl;  
}

template <class T>
void scv_bag<T>::show(int details, int indent) const {
  print(scv_out, details, indent);
}

template <class T>
std::ostream& operator<<(std::ostream& out, const scv_bag<T>& t) {
  t.print(out, 1);
  return (out);
}

template <class T>
bool scv_bag<T>::operator!=(const scv_bag& other) {
  return !(this->_bag == other._bag);
} 

template <class T>
bool scv_bag<T>::is_equal(const scv_bag<T>& other) const {
  return (this->_bag == other._bag);
} 

template <class T>
bool operator==(const scv_bag<T>& a, const scv_bag<T>& b) { 
  return (a.is_equal(b));
} 


template <class T>
void scv_bag<T>::read(std::istream& in) {

  int num = 0;  in >> num ;
  T ip ; 

  for (int i = 0 ; i < num ; i++) {
    in  >> ip ;
    this->push(ip);
  }

}

template <class T>
std::istream& operator>>(std::istream& in, scv_bag<T>& t) {
  t.read(in);
  return (in);
}


#endif // SCV_BAG_H
