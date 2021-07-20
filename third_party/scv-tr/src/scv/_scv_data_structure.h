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

  _scv_data_structure.h -- 
  The base class of any data structure, specifying the interface
  common to them.

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Torsten Maehne
                               Universite Pierre et Marie Curie, 2013-12-04
  Description of Modification: Add basic documentation comment for
                               _scv_CstructMethods<T> and enable taking into
                               account differing memory layouts for C structs
                               using this class as a base class on different
                               platforms.

      Name, Affiliation, Date: Torsten Maehne
                               Universite Pierre et Marie Curie, 2013-12-04
  Description of Modification: Exclude class template _scv_CstructMethods<T>
                               from compilation, as its implementation is
                               questionable and not used at all in the SCV
                               implementation or accompanying examples.

 *****************************************************************************/

#ifndef SCV_DATA_STRUCTURE_H
#define SCV_DATA_STRUCTURE_H

#include <string>

#include "scv/scv_object_if.h"
#include "scv/scv_util.h"
#include "scv/scv_debug.h"


// Don't compile _scv_CstructMethods<T> due to its questionable implementation.
#if 0

// Default offset for a C struct of type T with respect to the this
// pointer used by _scv_CstructMethods<T> to implement its constructor
// and comparison operators using memset() and memcmp(). Usually, it
// should be 0, as a C struct and _scv_CstructMethods<T> don't contain
// virtual member functions (which presence would add a
// vtable). However, this also assumes that the compiler is doing the
// empty base class optimization, which may not always be the
// case. So, the offset is compiler, optimization, and
// platform-dependent.
#ifndef SCV_CSTRUCT_METHODS_OFFSET
#  define SCV_CSTRUCT_METHODS_OFFSET 0
#endif


// The template _scv_CstructMethods<T> serves to instrument a C struct
// containing only member variables of plain-old data types and C
// structs fulfilling the same constraint with a default constructor
// and basic comparison and output operator implementations. The
// constructor will zero-initialize the whole occupied memory by T
// using memset(). Implementations of the operators ==, <, !=, >, <=,
// >= are provided using memcmp(). A basic output operator<<
// implementation is provided to identify the structure as a C-struct.
//
// To this end, _scv_CstructMethods<T> uses the Curiously Recurring
// Template Pattern (CRTP). To use the above-described functionality,
// a C struct T simply has to inherit publicly from
// _scv_CstructMethods<T>.
//
// Attention, the memory layout of C++ structures and classes is
// ABI-specific and thus varies in function of the processor and OS
// platform, used compiler version, etc. It is also dependent on the
// optimizations carried out by the compiler (especially the empty
// base class optimization) and whether a vtable is present.
template<class T>
struct _scv_CstructMethods {
  _scv_CstructMethods() {
    memset(this+SCV_CSTRUCT_METHODS_OFFSET, 0, sizeof(T)-SCV_CSTRUCT_METHODS_OFFSET);
  }
  friend bool operator==(const _scv_CstructMethods<T>& a,
                         const _scv_CstructMethods<T>& b) {
    return 0==memcmp(&a+SCV_CSTRUCT_METHODS_OFFSET, &b+4,
                     sizeof(T)-SCV_CSTRUCT_METHODS_OFFSET);
  }
  friend bool operator<(const _scv_CstructMethods<T>& a,
                         const _scv_CstructMethods<T>& b) {
    return memcmp(&a+SCV_CSTRUCT_METHODS_OFFSET, &b+SCV_CSTRUCT_METHODS_OFFSET,
                  sizeof(T)-SCV_CSTRUCT_METHODS_OFFSET) < 0;
  }
  friend ostream& operator<<(ostream& os, const _scv_CstructMethods<T>&) {
    os << "< C-struct >";
    return os;
  }
  friend bool operator!=(const _scv_CstructMethods<T>& a,
                         const _scv_CstructMethods<T>& b) {
    return !(a==b);
  }
  friend bool operator>(const _scv_CstructMethods<T>& a,
                        const _scv_CstructMethods<T>& b) {
    return b<a;
  }
  friend bool operator<=(const _scv_CstructMethods<T>& a,
                         const _scv_CstructMethods<T>& b) {
    return a<b || a==b;
  }
  friend bool operator>=(const _scv_CstructMethods<T>& a,
                         const _scv_CstructMethods<T>& b) {
    return a>b || a==b;
  }
};

#endif // 0


class _scv_data_structure : public scv_object_if {
public:

  //
  // virtual destructor
  //
  virtual ~_scv_data_structure() {};

  //
  // return the instance name of the data structure 
  //
  const char *get_name() const { return _name.c_str(); }

  virtual const char *kind() const { return _kind; }

  // 
  // print current values on output stream or on current screen
  //
  virtual void print(std::ostream& o, int details, int indent) const {};

  virtual void show(int details=0, int indent=0) const {
      print(scv_out, details, indent);
  }

  const char *nameP() const {  return _name.c_str(); }

  static int get_debug() {return _debug;} 

  static void set_debug(int debug)  {
    scv_debug::set_facility_level(scv_debug::DATA_STRUCTURES, debug);
    _debug = debug;
  }

//protected:
  //
  // constructor (all data structures have optional name)
  //
  _scv_data_structure(const char *name = "<anonymous>") : _name(name) {};
  std::string _name;
  static int _debug;
  static const char *_kind;

  static void set_class_debug(int debug) { _debug = debug; }
};

#endif // SCV_DATA_STRUCTURE_H
