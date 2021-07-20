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

  _scv_associative_array.h -- 
  Associative array is an efficient implementation of a large array indexed
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

#ifndef SCV_ASSOCIATIVE_ARRAY_H
#define SCV_ASSOCIATIVE_ARRAY_H

#include "scv/_scv_data_structure.h"
#include "scv/scv_util.h"
#include "scv/scv_random.h"
#include <map>

template <class Data>   // needed for scv_random.cpp
std::ostream& operator<<(std::ostream&os, const std::list<Data> & t) {
  typename std::list<Data>::const_iterator temp;
  for (temp=t.begin(); temp != t.end(); ++temp) {
    os << (*temp) << std::endl;
  }
  return os; 
}

template <class Key, class Data, typename container_type = std::map<Key,  Data, std::less<Key> > > 
class _scv_associative_array : public _scv_data_structure {
public:
  typedef typename container_type::iterator iteratorT;  
  typedef typename container_type::const_iterator constIteratorT;  
  typedef typename container_type::const_iterator const_iterator;  
private:
  Data _defaultValue;
  container_type _map;  

public:

  _scv_associative_array(const char *name,
			       const Data& defaultValue)
   : _scv_data_structure(name), _defaultValue(defaultValue), _map() { }
    
  //
  // parameter access (corresponds to the argument of the constructor arguments
  // 
  const Data& defaultValue() const { return _defaultValue; } 
  const Data& default_value() const { return _defaultValue; } 

  //
  // copy constructor (create a new Associative array with the same values as the argument)
  // 
  _scv_associative_array(const _scv_associative_array& other, const char *nameP="<anonymous>") 
    : _scv_data_structure(nameP), _defaultValue(other._defaultValue), _map(other._map) { } 

  //
  // virtual destructor
  //
  virtual ~_scv_associative_array() { } 

  virtual const char *kind() const
    { static const char name[]="_scv_associative_array"; return name; }

 
  //
  // accessing an element
  // 
  virtual Data& operator[](const Key& i) { 
    if (_map.count(i)) return _map[i]; 
    else { 
      std::pair<const Key, Data> tmp(i, _defaultValue);  _map.insert(tmp);  // modified for hp
      return _map[i];
    }
  } 
  virtual const Data& operator[](const Key& i) const { 
    if (_map.count(i)) {
      container_type * myMap = (container_type *) &_map;
      return (*myMap)[i]; 
    }
    else { 
      return _defaultValue;
    } 
  } 

  void insert(const Key& k, const Data& data){
    _map[k] = data ;
  }

  // This procedure is written to provide the user a read only procedure.
  // the [] operator can be write or read for non-const objects. In such
  // a situation if [] is used on non-const object to query the value
  // [i] , if there is no value stored at index i, it creates a copy of
  // _defaultValue by allocating new memory and storing it in the associative array.
  // This is because when [] is used we do not know if it is for reading or writing.
  // If it is for reading we do not need to create a copy of _defaultValue.
  // The getValue procedure does this.


  const Data& getValue(const Key& i) const {
    return (*this)[i] ;
  }

  //
  // freeing the storage allocated for an element, and
  // resetting its value to the default value.
  //
  void erase(const Key& i) { _map.erase((const Key&)i); } 

  //
  // freeing all storages, and resetting all elements to the default value.
  //
  void clear() { _map.erase(_map.begin(), _map.end()); } 

  //
  // methods involving iteratorTs (to be specified once the iteratorT
  // interface is finalized)
  //
  iteratorT begin() { return _map.begin(); } 
  iteratorT end() { return _map.end(); } 
  constIteratorT begin() const { return _map.begin(); } 
  constIteratorT end() const { return _map.end(); } 

  bool empty() const { return _map.empty(); }

  iteratorT find(const Key& i) { return _map.find(i); }
  constIteratorT find(const Key& i) const { return _map.find(i); }

  //
  // assignment and equality comparison
  // 
  _scv_associative_array& operator=(const _scv_associative_array& other) { 
    if (this != &other) {
      _map = other._map;
    } 
    return *this;
  }
  bool is_equal(const _scv_associative_array<Key, Data, container_type> & other)  const ;

  //
  // return the number of explicit element allocated through []
  // but not yet erased through erase().
  //
  int numExplicitElt() const { return _map.size(); } 
  int num_explicit_elt() const { return _map.size(); } 
 
  //
  // return TRUE if there is at least one explicit element.
  // allocated through [] but not yet erased through erase().
  //
  bool hasExplicitElt() const { return numExplicitElt() > 0; } 
  bool has_explicit_elt() const { return numExplicitElt() > 0; } 
 
  //
  // return the number of elements with values differ from 
  // the default value. 
  // We just count them! (nkhalil)
  //
  int numNonDefaultElt() const ;
  int num_non_default_elt() const ;

  //
  // return TRUE if there is any element with values differ from 
  // the default value 
  //
  int hasNonDefaultElt() const;
  int has_non_default_elt() const;

  virtual void print(std::ostream& os=scv_out, int details=0, int indent=0) const {
  typename container_type::const_iterator temp;
  os << kind() << " Name: " << get_name() << std::endl;
  if ( ! details ) return;
  for (temp = _map.begin(); temp != _map.end(); ++temp) {
    os << "\t[ " << (*temp).first << " ] = ";
    os << (*temp).second << std::endl;
  }
  }

  virtual void show(int details=0, int indent=0) const {
    print(scv_out, details, indent);
  }

  //int get_debug() const {return -1;}
  //void set_debug(int) {}
};

template <class Key, class Data, typename container_type>
std::ostream& operator<<(std::ostream&os, const _scv_associative_array<Key, Data, container_type>& t){
  t.print(os, 1);
  return(os);
}

template <class Key, class Data, typename container_type> 
bool _scv_associative_array<Key, Data, container_type>::is_equal(const _scv_associative_array<Key, Data, container_type>& other) const {
  return this->_map == other._map;
}

template <class Key, class Data, typename container_type> 
bool operator==(const _scv_associative_array<Key, Data, container_type>& a,
		const _scv_associative_array<Key, Data, container_type>& b){ 
  return a.is_equal(b);
}

template <class Key, class Data, typename container_type> 
int _scv_associative_array<Key, Data, container_type>::numNonDefaultElt() const { 
  int nelements = 0;
  constIteratorT temp;
  for(temp = _map.begin(); temp != _map.end(); ++temp) { 
    if ((*temp).second != _defaultValue) nelements++;
  }
  return nelements;
}
 
template <class Key, class Data, typename container_type> 
int _scv_associative_array<Key, Data, container_type>::num_non_default_elt() const { 
return _scv_associative_array<Key, Data, container_type>::numNonDefaultElt();
}

template <class Key, class Data, typename container_type> 
int _scv_associative_array<Key, Data, container_type>::hasNonDefaultElt() const { 
  constIteratorT temp;
  for(temp = _map.begin(); temp != _map.end(); ++temp) { 
    if ((*temp).second != _defaultValue) return 1;
  }
  return 0;
} 

template <class Key, class Data, typename container_type> 
int _scv_associative_array<Key, Data, container_type>::has_non_default_elt() const { 
return _scv_associative_array<Key, Data, container_type>::hasNonDefaultElt();
}

#endif // SCV_ASSOCIATIVE_ARRAY_H
