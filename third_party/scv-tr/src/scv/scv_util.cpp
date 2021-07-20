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

  scv_util.cpp -- The implementation of various small facilities.

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Stephan Gerth, Fraunhofer IIS-EAS, 2017-11-01
  Description of Modification: Added header and namespaces for C string library
                               calls

 *****************************************************************************/

#include "scv/scv_util.h"
#include "scv/_scv_associative_array.h"
#include "scv/scv_report.h"
#include <cstring>

/* ************************************************************************** */

// For cdsIdent:

#include "scv/scv_kit_date.h"
#include "scv/scv_ver.h"

#ifndef SCV_VERSION
#define SCV_VERSION "<SCV_VERSION undefined>"
#endif

#ifndef SCV_KIT_DATE
#define SCV_KIT_DATE "<SCV_KIT_DATE undefined>"
#endif


static struct {
  const char* strP;
  const char** strPP;
} scvVersionIdent = { "@(#)$CDS: libscv.so " SCV_VERSION " " SCV_KIT_DATE " $",
		&scvVersionIdent.strP };

/* ************************************************************************** */


//
// scv_startup
//


bool _scv_startup_called = false;

extern void scv_constraint_startup();

bool scv_startup()
{
  static bool first = true;
  if ( first ) {
    scv_constraint_startup();
    first = false;
  }
  return true;
}


//
// making unique names
//


int _scv_make_unique_id(const std::string& name, const std::string& kind)
{
  typedef _scv_associative_array<std::string,int> xref;
  static xref table("NameList",0);
  static const std::string delim = ":::";
  return table[kind+delim+name]++;
}

const std::string _scv_make_unique_name(const std::string& name, int id)
{
  static char *image = 0;
  static std::size_t len = 0;
  if ( id == 0 ) return name;
  std::size_t tmp = std::strlen(name.c_str()) + 36;
  if ( tmp > len ) {
    delete [] image;
    image = new char[tmp];
    len = tmp;
  }
  sprintf(image,"%s(%d)",name.c_str(),id);
  return image;
}


//
// Determine process name
//


_scv_process_name_server_t *_scv_process_name_server = 0;

void _scv_set_process_name_server(_scv_process_name_server_t *server)
{ _scv_process_name_server = server; }

const char *_scv_get_process_name(const sc_core::sc_process_handle proc_p)
// Enhance later to return unique name that's as stable as
// possible despite changes in order of execution.
{ return proc_p.name(); }

const char *scv_get_process_name(sc_core::sc_process_handle proc_p)
{
  if ( proc_p.valid() ) return "<main>";
  if ( _scv_process_name_server ) {
    return _scv_process_name_server(proc_p);
  }
  return _scv_get_process_name(proc_p);
}


//
// Class and associated methods for scv_out
//

class _scv_out_buf_t : public std::streambuf {
 public:
  int sync();
  int overflow(int ch); // Called with just one character
  int flush();
};

_scv_out_buf_t *_scv_out_buf_p = new _scv_out_buf_t;
std::ostream *_scv_out_p = new std::ostream(_scv_out_buf_p);
static int _scv_out_buffer_index = 0;
static char _scv_out_buffer[3000];
static bool _add_scv_prefix = true;
static char *_scv_prefix = getenv("SCV_REG");

int _scv_out_buf_t::sync() {
  if (_scv_out_buffer_index == 0) {
    return 0;
  }
  _scv_out_buffer[_scv_out_buffer_index] = '\0';
  if (_scv_prefix == NULL || std::strlen(_scv_prefix) == 0) {
    _scv_out_buffer_index = 0;
    std::cout << _scv_out_buffer;
    return 0;
  }
  char scv_prefix[10];
  if (_add_scv_prefix) {
    std::strcpy(scv_prefix, _scv_prefix);
  } else {
    scv_prefix[0] = '\0';
  }
  char scv_spare_buffer[3000];
  char *begin_line = &_scv_out_buffer[0];
  char *spare_buffer = &scv_spare_buffer[0];
  char *chr;
  for (chr = &_scv_out_buffer[0]; *chr != '\0'; chr++) {
    if (*chr == '\n') {
      std::strcpy(spare_buffer, scv_prefix);
      spare_buffer += std::strlen(scv_prefix);
      while (begin_line != chr) {
        *spare_buffer++ = *begin_line++;
      }
    }
  }
  if (begin_line < chr) {
    while (begin_line <= chr) {
      *spare_buffer++ = *begin_line++;
    }
  }
  if (*(chr-1) == '\n') {
    *(spare_buffer-2) = '\0';
  }
  std::strcpy(_scv_out_buffer, scv_spare_buffer);
  _scv_out_buffer_index = 0;
  std::cout << _scv_out_buffer << std::endl;
  return 0;
}

int _scv_out_buf_t::flush() {
  _scv_out_buf_p->sync();
  return 0;
}

int _scv_out_buf_t::overflow(int ch) {
  // Called with one char
  char c;
  c = (char) ch;
  _scv_out_buffer[_scv_out_buffer_index] = c;
  _scv_out_buffer_index++;
  if ( (_scv_out_buffer_index > 2000) || (c == '\n') ) {
    _scv_out_buf_p->sync();
  }
  return ch;
}

//
// Data structure initialization

int _scv_data_structure::_debug = -1;

const char *_scv_data_structure::_kind = "_scv_data_structure";

void scv_object_if::set_debug_level(const char *facility, int level)
{  scv_debug::set_level(facility,level); }
