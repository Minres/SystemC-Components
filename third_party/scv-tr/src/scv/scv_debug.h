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
/*
 * Author:  Dean Shea
 *
 * Description:
 *
 * The public interface for debug messages.
 *
 * This is scv_debug.h, the header file for the Debug/Tracing facility
 * in Test Builder.
 *
 * To add a new facility, and an enum item to scv_debug::debug_facilities,
 * and also add the string name to scv_debug::facility_names[].  Also,
 * modify scv_debug::set_level_for_classes() to modify the debug levels for
 * appropriate classes when a level is set for the new facility.
 *
 * Here's a summary of what scv_debug can do:
 *
 *   A macro called SCV_DEBUG is used to put strings into a trace file and
 *   to stdout.  The argument to the macro are: object, level, and a string.
 *   object is a pointer to any member of a class derived from scv_object_if.
 *
 *   A macro called SCV_DEBUG_CHECK is used to check if tracing is turned on.
 *
 *   Each facility that can be traced has an enum item and a string name.
 *   (A facility can also be a developer name, for special cases.)
 *
 *   Each SCV_DEBUG call specifies a level, where 0 is highest, increasing
 *   numbers are lower levels.
 *
 *   You can set a level for tracing - all traces up to that level number
 *   are set.  Exception: 0 means all, and -1 means none.  You can use
 *   scv_debug::set_level() to control tracing for all classes in a
 *   particular facility, or use <class>::set_debug() to control tracing 
 *   for each class separately.
 *
 *   You can suspend/resume tracing to the log file and to stdout.
 *
 *
 * Here's a summary of how to use scv_debug:
 *
 *   Open a trace file, usually at the beginning of execution.  If you don't
 *   make this call, the default file is: "scv_debug.log"
 *
 *   Turn on tracing for facilities.  This could be done at any time, but
 *   usually at the beginning of execution, while parsing the command line:
 *
 *   options:
 *     scv_debug::set_level("threads");  // Default is level 0 tracing.
 *   or:
 *     scv_debug::set_level("threads", 2);  // Level 2 tracing.
 *
 *   Add a trace, for the tb_thread class, at level 0.  First argument is
 *   any member of the class.  Third argument is a string:
 *
 *     SCV_DEBUG(this, 0, "Hello 1"); 
 *
 *   To check if tracing for a class is turned on:
 *
 *     if (SCV_DEBUG_CHECK(this, 0)) { 
 *       ...
 *     }
 *
 */


#ifndef SCV_DEBUG_H
#define SCV_DEBUG_H

#include <cstdio>

#include "scv/scv_object_if.h"
#include "scv/scv_util.h"

#define _scv_trace(arg) if ((arg)) \
scv_out << "_scv_trace at file " << __FILE__ << " line " << __LINE__ << " "


// Comment out the following for builds without tracing capability.
#define SCV_DEBUG_ON


#ifdef SCV_DEBUG_ON 

// Use this macro to add traces to the trace file - arg1 is the string
// that's put into the file.
#define SCV_DEBUG(object_p, level, arg1) \
  if ( scv_debug::check((object_p)->get_debug(),(level)) ) \
    scv_debug::record_data((__FILE__), (__LINE__), (object_p), (level), (arg1))

// This macro will check if tracing is turned on for a facility/level.
// If set, then return 1, else return 0.
#define SCV_DEBUG_CHECK(item, level) _scv_debug_check((item),(level))

#else
// Macros do nothing if SCV_DEBUG_ON is not defined.
#define SCV_DEBUG(object_p, level, arg1)
#define SCV_DEBUG_CHECK(object_p, level) 0
#endif


class scv_debug : public scv_object_if {
public:
  // Modify this enum to add a new facility; you also need to add a 
  // string for the facility name to scv_debug::facility_names[] in 
  // scv_debug.cpp.
  enum debug_facilities {
    ALL = 0,
    DATA_STRUCTURES,
    INTROSPECTION,
    MESSAGES,
    RANDOMIZATION,
    RECORDING,
    SIGNALS,
    TRANSACTORS,
    LAST		// Add new ones above this
  };
  static const int INITIAL_DEBUG_LEVEL;
  static const int SUSPENDED_DEBUG_LEVEL;

private:
  scv_debug(const char *filename = 0);

public:
  virtual ~scv_debug();

public:

  // This is called by the SCV_DEBUG_CHECK macro, not by users:
  static int check(int actual_level, int target_level);

  // This is called by the SCV_DEBUG macro, not by users:
  static void record_data(
    const char *filename,
    int lineno,
    scv_object_if *object_p,
    int target_level,
    const char *data);

  // Open a new trace file, default is tbvdbg.log.  Also close the old file.
  static void open_trace_file(const char *filename);
  static void close_trace_file();  // Traces will now only go to stdout.

  // Turns on/off stdout:
  static void open_stdout();
  static void close_stdout();

  // Set tracing level:
  static void set_level(debug_facilities facility, int level = 0);
  static void set_level(const char *facility_name, int level = 0);

  // Set tracing level by call from facility
  static void set_facility_level(debug_facilities facility, int level = 0);

  // Temporarily suspend/resume tracing:
  static void resume(int facility);
  static void suspend(int facility);

  static void indent(std::ostream& os, int indent);

  // implement scv_object_if methods
  const char *get_name() const { return kind(); }
  const char *kind() const { return _kind; }
  void print(std::ostream& o=scv_out, int details=0, int indent=0) const;
  void show(int details=0, int indent=0) const { print(scv_out,details,indent); }
  static int get_debug() { return _debug; }
  static void set_debug(int);

private:
  static void set_level_for_classes(int, int);

private:
  std::FILE *file_p;  // Trace log file.
  int facility_levels[LAST];
  int send_to_stdout;  // Toggle stdout tracing.
  static int _debug;
  static int local_origination;
  static scv_debug *scv_debug_p; // the only instance of scv_debug
  static const char* facility_names[];
  static const char *_kind;

  void send_to_log(const char *theString);
};

template<typename T>
bool _scv_debug_check(T *object_p, int target_level)
{ return scv_debug::check(object_p->get_debug(),target_level); }

inline bool _scv_debug_check(int actual_level, int target_level)
{ return scv_debug::check(actual_level,target_level); }

#endif
