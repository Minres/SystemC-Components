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

  scv_object_if.h --
  Common base class for SCV objects.  All SCV API classes
  should inherit from this class, directly or indirectly.

  Until set_level() is called, get_level() should return -1.

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

#ifndef SCV_OBJECT_IF_H
#define SCV_OBJECT_IF_H

#include <iostream>

class scv_object_if {
public:

  // return the instance name of the data structure
  virtual const char *get_name() const = 0;

  // return a string unique to each class
  virtual const char *kind() const = 0;

  // print current values on output stream
  virtual void print(std::ostream& o=scv_out, int details=0, int indent=0) const = 0;

  // print current values on output stream
  virtual void show(int details=0, int indent=0) const { print(scv_out,-1,0); }

  // control debug messages by facility (do not override)
  static void set_debug_level(const char * facility, int level = -1);

  // are debug messages on for this class (write for each class)?
  // static int get_debug() { ... }

  // control debug messages by class (write for each class)
  // static void set_debug(int) { ... }

  //destructor (does nothing)
  virtual ~scv_object_if() {};

};

#endif
