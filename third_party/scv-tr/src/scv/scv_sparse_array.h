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

  scv_sparse_array.h -- 
  A sparse array is an efficient implementation of a large array indexed
  by a range of integers, when most of the elements have the same
  default value (e.g. 0 or "undefined").

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

#ifndef SCV_SPARSE_ARRAY_H
#define SCV_SPARSE_ARRAY_H

#include "scv/_scv_associative_array.h"
#include "scv/scv_report.h"

#include <climits>
#include <cmath>
#include <map>

// scv_sparse_array (with integer indexing) (using map)

template <class Key, class T>
class scv_sparse_array : public _scv_associative_array<Key, T, std::map<Key, T, std::less<Key> > > {
  T _dummy;     // only for the purpose of error recovery
  Key _indexLB;
  Key _indexUB;
public:

typedef std::map<Key, T, std::less<Key> > container_type;

  // constructor
  scv_sparse_array(const char *nameP, const T& defaultValue,
		  const Key& indexLB=0 , const Key& indexUB=INT_MAX ):

    _scv_associative_array<Key, T, container_type>(nameP, defaultValue),
    _dummy(defaultValue),
    _indexLB(indexLB), 
    _indexUB(indexUB)
  { };

  //
  // copy constructor (create new sparse array with same values as argument)
  // 
  scv_sparse_array(const scv_sparse_array& other, const char *nameP="<anonymous>") 
    : _scv_associative_array<Key, T, container_type>(other, nameP),
    _dummy(other._dummy),
    _indexLB(other._indexLB), _indexUB(other._indexUB){}

  //
  // virtual destructor
  //
  virtual ~scv_sparse_array() {}

  virtual const char* kind() const
    { static const char name[]="scv_sparse_array"; return name; }

  //
  // parameter access (corresponds to the argument of the constructor arguments
  // 
  const Key& indexLB() const { return _indexLB; };
  const Key& indexUB() const { return _indexUB; };

  const Key& lower_bound() const { return _indexLB; };
  const Key& upper_bound() const { return _indexUB; };

  //
  // accessing an element
  //
  // If "i" is out-of-bound, issues warning and return default value;
  // modifying T& returned from operator[] will have no effect on the array.
  // 
  // 
  T& operator[](const Key& i) { 
    if ((i >= _indexLB) &&  (i <= _indexUB)) 
      return _scv_associative_array<Key, T, container_type>::operator[](i);
    else {
      // _scv_message do not handle yet systemc objects.
      //_scv_message::message(_scv_message::SPARSE_ARRAY_ILLEGAL_INDEX,
      //		i, this->nameP(), _indexLB, _indexUB);
      scv_out << "*** SCV_ERROR: SPARSE_ARRAY_ILLEGAL_INDEX: " << i << endl ;
      scv_out << "In scv_sparse_array '" << this->nameP() << "'." << endl;
      scv_out << "The valid range is lower = " << _indexLB ;
      scv_out << ", upper = " << _indexUB << endl; 
      // return dummy so that if the user changes its value,
      // it won't affect the other operations
      return _dummy; 
    }
  }
  const T& operator[](const Key& i) const { 
    if (!((i >= _indexLB) &&  (i <= _indexUB))) { 
      //_scv_message::message(_scv_message::SPARSE_ARRAY_ILLEGAL_INDEX,
      //		i, this->nameP(), _indexLB, _indexUB);
      scv_out << "*** SCV_ERROR: SPARSE_ARRAY_ILLEGAL_INDEX: " << i << endl ;
      scv_out << "In scv_sparse_array '" << this->nameP() << "'." << endl;
      scv_out << "The valid range is lower = " << _indexLB ;
      scv_out << ", upper = " << _indexUB << endl; 
      return _dummy;
    } else 
      return _scv_associative_array<Key, T, container_type>::operator[](i);
  } 

  //
  // assignment and equality comparison
  // 
  scv_sparse_array& operator=(const scv_sparse_array<Key, T>& other) { 
    if (this != &other) {
      *((_scv_associative_array<Key, T, container_type> *) this) =
	(_scv_associative_array<Key, T, container_type> ) other;
      _indexUB = other._indexUB;
      _indexLB = other._indexLB;
    } 
    return *this;
  }

  virtual void print(ostream& os=scv_out, int details=0, int indent=0) const {
    //typename container_type::const_iterator temp;
    //os << kind() << " Name: " << get_name() << endl;
    _scv_associative_array<Key, T,container_type>::print(os,details,indent);
  }

  virtual void show(int details=0, int indent=0) const {
    print(scv_out, details, indent);
  }

};

#endif // SCV_SPARSE_ARRAY_H
