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

  scv_random.cpp -- The public interface for unsigned random stream and seed
  file manipulation

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Stephan Schulz, Fraunhofer IIS-EAS, 2013-02-21
  Description of Modification: Added check for _WIN32 macro to support mingw32

      Name, Affiliation, Date: Stephan Gerth, Fraunhofer IIS-EAS, 2017-11-01
  Description of Modification: Added header and namespaces for C string library
                               calls

 *****************************************************************************/


#include "scv/scv_util.h"
#include "scv/scv_random.h"
#include "scv/scv_report.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>


#if (((defined _MSC_VER) || (defined _WIN32)) && !defined __MINGW32__)
inline int rand_r(unsigned int *) { return rand(); }
#endif


//#define OLD_SEED_GENERATION_SEMANTICS
//#define DONT_UNIQUIFY_NAMES

/////////////////////////////////////////////////////////////////////////
// Class : scv_random_error
//   - Error message interface for the scv_random class
//   - static methods to print respective messages
/////////////////////////////////////////////////////////////////////////

class scv_random_error {
public:
  static void missing_algorithm(const std::string algorithm_name) {
    _scv_message::message(_scv_message::RANDOM_NULL_ALGORITHM,algorithm_name.c_str());
  }

  static void out_of_order_seed(const std::string instance_name_p,
    const std::string fileNameP)
  {
    if (instance_name_p =="") {
      _scv_message::message(_scv_message::RANDOM_OUT_OF_ORDER_SEED,
        "<anonymous>",fileNameP.c_str());
    } else  {
      _scv_message::message(_scv_message::RANDOM_OUT_OF_ORDER_SEED,
        instance_name_p.c_str(),fileNameP.c_str());
    }
  }
  static void cannot_match_seed(const std::string instance_name,
    const std::string fileName)
  {
    if (instance_name == "") {
      _scv_message::message(_scv_message::RANDOM_CANNOT_MATCH_SEED,
        "<anonymous>",fileName.c_str());
    } else  {
      _scv_message::message(_scv_message::RANDOM_CANNOT_MATCH_SEED,
        instance_name.c_str(),fileName.c_str());
    }
  }
  static void retrieving_with_same_name(const std::string instance_name,
    const std::string fileName)
  {
    if (instance_name == "") {
      _scv_message::message(_scv_message::RANDOM_RETRIEVING_SEED_WITH_SAME_NAME,
			       "<anonymous>",fileName.c_str());
    } else {
      _scv_message::message(_scv_message::RANDOM_RETRIEVING_SEED_WITH_SAME_NAME,
			       instance_name.c_str(), fileName.c_str());
    }
  }
  static void storing_with_same_name(const std::string instance_name,
    const std::string fileName)
  {
    if (instance_name == "") {
      _scv_message::message(_scv_message::RANDOM_STORING_SEED_WITH_SAME_NAME,
			       "<anonymous>",fileName.c_str());
    } else  {
      _scv_message::message(_scv_message::RANDOM_STORING_SEED_WITH_SAME_NAME,
			       instance_name.c_str(), fileName.c_str());
    }
  }
  static void seed_monitor_not_off(const std::string fileName)
  {
    _scv_message::message(_scv_message::RANDOM_SEED_MONITOR_NOT_OFF,
			     fileName.c_str());
  }
  static void seed_not_exhausted(const std::string fileName) {
    _scv_message::message(_scv_message::RANDOM_SEED_NOT_EXHAUSTED,
			     fileName.c_str());
  }
  static void cannot_open_seed_file(const std::string fileName) {
    _scv_message::message(_scv_message::RANDOM_CANNOT_OPEN_SEED_FILE,
			      fileName.c_str());
  }
};

/////////////////////////////////////////////////////////////////////
// Class : _scv_random_impl
//   - class encapsulating implementation details of scv_random
//   -
/////////////////////////////////////////////////////////////////////

#ifdef OLD_SEED_GENERATION_SEMANTICS
static scv_random * s_seed_generator = NULL;
#endif

static bool s_retrieve = false;
static void retrieve_seed(const std::string & name, unsigned long long * seed);
static unsigned long long _scv_generate_seed(const std::string& name);
static void _scv_update_current_thread_info(const std::string& name);
static inline unsigned int _scv_jrand48(unsigned short next[3]);

static std::string s_current_thread_name;
static char s_current_inst_num[64];
static int s_inst_num;

static std::string _scv_get_unique_name(const std::string & name);
static std::string _scv_extract_name(const char * str);


class _scv_random_impl {
public:
  static int debug;
  scv_random::value_generation_algorithm _alg_type;
  unsigned long long _seed;
  scv_random::alg_func _algorithm;
  union {
    unsigned long long _next;
    unsigned int _rand_next;
    unsigned short _next48[3];
  } u ;

  _scv_random_impl(const std::string name,
       unsigned long long seed,
       scv_random::alg_func algorithm,
       scv_random::value_generation_algorithm alg_type) :
         _alg_type(alg_type) {

    // generate a seed regardless of whether a seed is
    // to be retrieved or not,
    // for better predictability even when there is problem
    // in the seed file.


    _scv_update_current_thread_info(name);

    if (seed == 0) {
#ifdef OLD_SEED_GENERATION_SEMANTICS
      if (!s_seed_generator) scv_random::set_global_seed();
#endif
      _seed = _scv_generate_seed(name);
    } else {
      _seed = seed;
    }

    if (s_retrieve) retrieve_seed(_scv_get_unique_name(name), &_seed);

    _algorithm = algorithm;
    if (_algorithm) {
      u._next = _seed;
    } else if (_alg_type == scv_random::RAND48) {
      u._next48[0] = (unsigned short)(_seed >> 16);
      u._next48[1] = (unsigned short)(_seed & 0xffff);
      u._next48[2] = 0x330E;
    } else if (_alg_type == scv_random::RAND ||
               _alg_type == scv_random::RAND32) {
      u._rand_next = (unsigned int)_seed;
    } else {
      _scv_message::message(_scv_message::INTERNAL_ERROR, " unknown _alg_type ");
    }
  }
  ~_scv_random_impl() {}

  unsigned int next() {
    if (_algorithm) {
      return _algorithm(u._next);
    } else if (_alg_type == scv_random::RAND48) {
      return _scv_jrand48(u._next48);
    } else if (_alg_type == scv_random::RAND) {
      return rand_r(&u._rand_next);
    } else if (_alg_type == scv_random::RAND32) {
#ifdef LINUX_SOURCE
      return rand_r(&u._rand_next);
#else
      // RAND_MAX on SUN/HP is 32767, so still will not be generating
      // negative values for signed types and also the 16th and 32nd
      // bits will be 0.
      unsigned int val = rand_r(&u._rand_next);
      val |= (rand_r(&u._rand_next) << 16);
      return val;
#endif
    } else {
      // should never happen
      return 0;
    }
  }

  unsigned int testNext() {
    if (_algorithm) {
      unsigned long long tmp = u._next;
      return _algorithm(tmp);
    } else if (_alg_type == scv_random::RAND48) {
      unsigned short tmp[3];
      tmp[0] = u._next48[0];
      tmp[1] = u._next48[1];
      tmp[2] = u._next48[2];
      return _scv_jrand48(tmp);
    } else if (_alg_type == scv_random::RAND) {
      return rand_r(&u._rand_next);
    } else {
      // should never happen
      return 0;
    }
  }
  static int get_debug(void) {
    return debug;
  }
  static void set_debug(int dbg) {
    if ( debug == dbg ) return;
    debug = dbg;
    scv_debug::set_facility_level(scv_debug::RANDOMIZATION, dbg);
  }
};

int _scv_random_impl::debug = scv_debug::INITIAL_DEBUG_LEVEL;

///////////////////////////////////////////////////////////////
// Class: scv_random
//   - Implementation of scv_random interface
//   - Generate multiple independent unsigned random streams
//   - seed management for maintaining reproducibility
//     across various simulation runs and even kernel
//     implementations. Thread ordering is one of the
//   - Explicit seed management with storage and retrieval
//     from seed registry
///////////////////////////////////////////////////////////////

unsigned long long scv_random::global_seed = 1;

scv_random::value_generation_algorithm scv_random::global_alg_type = scv_random::RAND48;

//---------------------------------------------
// global configuration on algorithm and seed
//---------------------------------------------
static scv_random::alg_func s_algorithm = NULL;
extern unsigned long long _scv_default_global_init_seed(
  unsigned long job_number = 0);
extern unsigned long long _scv_get_seed_from_name(const char *, unsigned);

static void _scv_set_algorithm(scv_random::value_generation_algorithm alg,
  scv_random::alg_func custom_alg, scv_random::alg_func * scv_be_set,
  const std::string & algorithm_name) ;

void scv_random::set_global_seed(unsigned long long seed)
{
#ifdef OLD_SEED_GENERATION_SEMANTICS
  if (s_seed_generator) {
    delete s_seed_generator;
    s_seed_generator = NULL;
  }
  if (seed==0) seed = _scv_default_global_init_seed();
#endif

  global_seed = seed;

#ifdef OLD_SEED_GENERATION_SEMANTICS
  s_seed_generator = new scv_random("internal seed generator",seed);
#endif

}

unsigned long long scv_random::get_global_seed(void) {
  return global_seed;
}

unsigned long long scv_random::pick_random_seed(unsigned long job_number) {
  return _scv_default_global_init_seed(job_number);
}

void scv_random::set_default_algorithm(value_generation_algorithm alg,
			       alg_func custom_alg) {
  ::_scv_set_algorithm(alg,custom_alg,&s_algorithm,
    "the global algorithm for scv_random");
  global_alg_type = alg;
}

static std::list<scv_random *>& s_list_of_generators();

void scv_random::get_generators(std::list<scv_random *>& genList)
{
  std::list<scv_random*>::iterator iter;
  for (iter = s_list_of_generators().begin();
     !(iter == s_list_of_generators().end());
       iter++ ) {
    genList.push_back(*iter);
  }
}

//---------------------------------------------
// constructors for independent random streams
//---------------------------------------------
static void addSelf(scv_random * self);

scv_random::scv_random(const char* name)
  : _scv_data_structure(_scv_extract_name(name).c_str()),
    _coreP(new _scv_random_impl(_name,0,s_algorithm, global_alg_type))
{
  _name = _scv_get_unique_name(_name);
  addSelf(this);
}

scv_random::scv_random(unsigned long long seed)
  : _scv_data_structure("<anonymous>"),
    _coreP(new _scv_random_impl(_name,seed,s_algorithm, global_alg_type))
{
  _name = _scv_get_unique_name(_name);
  addSelf(this);
}

scv_random::scv_random(const char* name, unsigned long long seed)
  : _scv_data_structure(_scv_extract_name(name).c_str()),
    _coreP(new _scv_random_impl(_name,seed,s_algorithm, global_alg_type))
{
  _name = _scv_get_unique_name(_name);
  addSelf(this);
}

scv_random::scv_random(const scv_random& other,
		     const char* name, unsigned long long seed)
  : _scv_data_structure(_scv_extract_name(name).c_str()),
    _coreP(new _scv_random_impl(_name,seed,other._coreP->_algorithm, other._coreP->_alg_type))
{
  _name = _scv_get_unique_name(_name);
  addSelf(this);
}

scv_random::~scv_random()
{
  s_list_of_generators().remove(this);
  delete _coreP;
}

//---------------------------------------------
// random value generation and configuration of
// independent random streams
//---------------------------------------------
unsigned int scv_random::next()
{
  return _coreP->next();
}

void scv_random::set_algorithm(value_generation_algorithm m,
			 alg_func algorithm)
{
  _coreP->_alg_type = m;
  ::_scv_set_algorithm(m,algorithm,&_coreP->_algorithm,get_name());
}

unsigned long long scv_random::get_initial_seed() const
{
  return _coreP->_seed;
}

unsigned long long scv_random::get_current_seed() const
{
  if (_coreP->_algorithm) {
    return _coreP->u._next;
  } else if (_coreP->_alg_type == scv_random::RAND48) {
    unsigned long long seed = 0;
    unsigned long long tmp = 0;

    tmp = _coreP->u._next48[2];
    seed = tmp << 32;

    tmp = _coreP->u._next48[1];
    seed = seed | tmp << 16;

    tmp = _coreP->u._next48[0];
    seed = seed | tmp;

    return seed;
  } else if (_coreP->_alg_type == scv_random::RAND ||
             _coreP->_alg_type == scv_random::RAND32) {
    return _coreP->u._rand_next;
  } else {
    // should not have reached here
    return 0;
  }
  // return to make compiler happy
  return 0;
}

void scv_random::set_current_seed(unsigned long long seed)
{
  if (_coreP->_algorithm) {
    _coreP->u._next = seed;
  } else if (_coreP->_alg_type == scv_random::RAND48) {
    _coreP->u._next48[0] = (unsigned short ) (seed & 0xffff);
    _coreP->u._next48[1] = (unsigned short ) (seed >> 16) & 0xffff;
    _coreP->u._next48[2] = (unsigned short ) (seed >> 32) & 0xffff;
  } else if (_coreP->_alg_type == scv_random::RAND ||
             _coreP->_alg_type == scv_random::RAND32) {
    _coreP->u._rand_next = (unsigned int)seed;
  } else {
    // should not have reached here
  }
  return;
}

//----------------------------------------------------------------------
// -initial seed is the seed assigned to this object when it is created
// -current seed is the current seed value that will be used to generate
// -the next unsigned integer value
// -these methods print the seeds of all current scv_random objects
//----------------------------------------------------------------------

void scv_random::print_initial_seeds(const char* fileName)
{
  if (fileName != NULL) {
    FILE * filePtr = fopen(fileName,"wb");

    std::list<scv_random*>::iterator iter;
    for (iter = s_list_of_generators().begin();
	 !(iter == s_list_of_generators().end());
	 ++iter) {

      std::string s = (*iter)->get_name();
      if (s!="") {
	fprintf(filePtr,"\"%s\" :: %llu\n",s.c_str(),
          (*iter)->get_initial_seed());
      } else {
	fprintf(filePtr,"\"<anonymous>\" :: %llu\n",(*iter)->get_initial_seed());
      }
    }
  } else {
    print_initial_seeds();
  }
}

void scv_random::print_initial_seeds(std::ostream& os)
{
  std::list<scv_random*>::iterator iter;
  for (iter = s_list_of_generators().begin();
       !(iter == s_list_of_generators().end());
       ++iter) {
    std::string s = (*iter)->get_name();
    if (s!="") {
      os << "\"" << s.c_str() << "\" :: " <<
        (*iter)->get_initial_seed() << std::endl;
    } else {
      os << "\"<anonymous>\" :: " << (*iter)->get_initial_seed() << std::endl;
    }
  }
}

void scv_random::print_current_seeds(const char* fileName)
{
  if (fileName != NULL) {
    FILE * filePtr = fopen(fileName,"wb");

    std::list<scv_random*>::iterator iter;
    for (iter = s_list_of_generators().begin();
	 !(iter == s_list_of_generators().end());
	 ++iter) {

      std::string s = (*iter)->get_name();
      if (s!="") {
	fprintf(filePtr,"\"%s\" :: %llu\n",s.c_str(),
          (*iter)->get_current_seed());
      } else {
	fprintf(filePtr,"\"<anonymous>\" :: %llu\n",
          (*iter)->get_current_seed());
      }
    }
  } else {
    print_current_seeds();
  }
}

void scv_random::print_current_seeds(std::ostream& os)
{
  std::list<scv_random*>::iterator iter;
  for (iter = s_list_of_generators().begin();
       !(iter == s_list_of_generators().end());
       ++iter) {
    std::string s = (*iter)->get_name();
    if (s!="") {
      os << "\"" << s.c_str() << "\" :: " <<
        (*iter)->get_current_seed() << std::endl;
    } else {
      os << "\"<anonymous>\" :: " << (*iter)->get_current_seed() << std::endl;
    }
  }
}

static bool s_store = false;
static bool s_exclusive_seed_file = false;
static FILE *s_seed_file_ptr = NULL;
static int s_numOutstanding_seeds = 0;
static bool s_warnedOutOfOrder = false;
static bool s_warned_same_name = false;
static bool s_warned_anonymous = false;
static bool s_has_anonymous_generator = false;

static std::string& s_seed_file_name();
static _scv_associative_array<std::string,std::list<unsigned long long> >& s_outstanding_seeds();
static _scv_associative_array<std::string,int>& s_names();
static bool readname_and_seed(std::string& nextName, unsigned long long& next_seed);
static _scv_associative_array<std::string, int> unique_name_hash("unique_name_hash", 0);

void scv_random::seed_monitor_on(bool retrieve, const char* fileName)
{
  if (s_store || s_retrieve) {
    scv_random_error::seed_monitor_not_off(s_seed_file_name());
    seed_monitor_off();
  }
  if (fileName != NULL) {
    s_exclusive_seed_file = true;
    if (retrieve) {
      s_store = false;
      s_retrieve = true;
      s_seed_file_name() = fileName;
      s_seed_file_ptr = fopen(fileName,"r");
#ifdef OLD_SEED_GENERATION_SEMANTICS
      if (s_seed_generator) {
        delete s_seed_generator;
        s_seed_generator = NULL;
      }
#endif
    } else {
      s_store = true;
      s_retrieve = false;
      s_seed_file_name() = fileName;
      s_seed_file_ptr = fopen(fileName,"wb");
    }
    if (!s_seed_file_ptr) {
      scv_random_error::cannot_open_seed_file(fileName);
      s_store = false;
      s_retrieve = false;
    }
  }
}

void scv_random::seed_monitor_on(bool retrieve, const char* sectionName,
  FILE * file)
{
  if (s_store || s_retrieve) {
    scv_random_error::seed_monitor_not_off(s_seed_file_name());
    seed_monitor_off();
  }
  if (file) {
    s_exclusive_seed_file = false;
    s_seed_file_name() = sectionName?sectionName:"";
    s_seed_file_ptr = file;
    if (retrieve) {
      s_store = false;
      s_retrieve = true;
    } else {
      s_store = true;
      s_retrieve = false;
    }
#ifdef OLD_SEED_GENERATION_SEMANTICS
    if (s_seed_generator) {
      delete s_seed_generator;
      s_seed_generator = NULL;
    }
#endif
  }
}

void scv_random::seed_monitor_off()
{
  if (s_store || s_retrieve) {
    s_names().clear();
    s_warned_same_name = false;
    s_warned_anonymous = false;
    s_has_anonymous_generator = false;
  }
  if (s_store) {
    s_store = false;
    if (s_exclusive_seed_file) fclose(s_seed_file_ptr);
    s_seed_file_name() = "";
  }
  if (s_retrieve) {
    std::string dummyName;
    unsigned long long dummy_seed;
    s_retrieve = false;
    if (s_seed_file_ptr && readname_and_seed(dummyName,dummy_seed)) {
      scv_random_error::seed_not_exhausted(s_seed_file_name());
    }
    if (s_numOutstanding_seeds>0) {
      scv_random_error::seed_not_exhausted(s_seed_file_name());
    }
    if (s_exclusive_seed_file && s_seed_file_ptr) fclose(s_seed_file_ptr);
    s_seed_file_name() = "";
    s_seed_file_ptr = NULL;
    s_numOutstanding_seeds = 0;
    s_outstanding_seeds().clear();
    s_warnedOutOfOrder = false;
  }
}

int scv_random::get_debug() {
  return _scv_random_impl::get_debug();
}

const char *scv_random::kind() const {
  static const char *name = "scv_random";
  return name;
}

void scv_random::print(std::ostream& o, int details, int indent) const {
  char algorithm[100];

  o << "scv_random Name: " <<  get_name() << std::endl;
  switch(_coreP->_alg_type) {
    case scv_random::RAND48 :
      std::strcpy(algorithm, "jrand48");
      break;
    case scv_random::RAND:
      std::strcpy(algorithm, "rand");
      break;
    case scv_random::RAND32:
      std::strcpy(algorithm, "rand32");
      break;
    case scv_random::CUSTOM:
      std::strcpy(algorithm, "custom");
      break;
    default:
      break;
  }
  o << "\talgorithm: " << algorithm << std::endl;
  o << "\tseed: " << _coreP->_seed << std::endl;
  o << "\tnext: " << get_current_seed() << std::endl;
  o << "\tnext value: " << _coreP->testNext() << std::endl;
}

void scv_random::set_debug(int debug) {
  _scv_random_impl::set_debug(debug);
}

void scv_random::show(int details, int indent) const {
  print(scv_out, details, indent);
}

/////////////////////////////////////////////////////////////////
// Static functions used for scv_random implementation
//   * _scv_set_algorithm
//   * _scv_generate_seed
//   * s_seed_file_name
//   * s_outstanding_seeds
//   * s_names
//   * s_list_of_generators
//   * readname_and_seed
//   * retrieve_seed
//   * addSelf
//   * _scv_default_global_init_seed
//   * _scv_jrand48
/////////////////////////////////////////////////////////////////

extern const char *scv_get_process_name(sc_core::sc_process_handle);

static void _scv_update_current_thread_info(const std::string& name)
{
  sc_core::sc_process_handle handle = sc_core::sc_get_current_process_handle();
  if (handle.valid() ) {
	  s_current_thread_name = handle.name();
  } else {
    s_current_thread_name = "scv_main_thread";
  }
  std::string thread_based_name = s_current_thread_name + name;
  s_inst_num = unique_name_hash.getValue(thread_based_name);
  if (s_inst_num == 0) {
    std::sprintf(s_current_inst_num, "<noappend>");
  } else {
    std::sprintf(s_current_inst_num, "%d", s_inst_num);
  }
  unique_name_hash.insert(thread_based_name, (s_inst_num+1));
}

static unsigned long long _scv_generate_seed(const std::string& name)
{
#ifdef OLD_SEED_GENERATION_SEMANTICS
  return s_seed_generator->next();
#else
  std::string thread_based_name = s_current_thread_name + name;
  return _scv_get_seed_from_name(thread_based_name.c_str(), s_inst_num);
#endif
}

static void
_scv_set_algorithm(scv_random::value_generation_algorithm alg,
	     scv_random::alg_func custom_alg,
	     scv_random::alg_func * scv_be_set,
	     const std::string & algorithm_name) {
  switch(alg) {
  case scv_random::RAND48:
  case scv_random::RAND:
  case scv_random::RAND32:
    *scv_be_set = NULL; break;
  case scv_random::CUSTOM:
    if (custom_alg==NULL) {
      scv_random_error::missing_algorithm(algorithm_name);
      *scv_be_set = NULL; break;
    } else {
      *scv_be_set = custom_alg; break;
    }
  }
}

static std::string& s_seed_file_name() {
  static std::string seed_file_name;
  return seed_file_name;
}

static _scv_associative_array<std::string,std::list<unsigned long long> >&
s_outstanding_seeds() {
  static _scv_associative_array<std::string,std::list<unsigned long long> >
    outstanding_seeds("outstanding_seeds",std::list<unsigned long long>());
  return outstanding_seeds;
}

static _scv_associative_array<std::string,int>&
s_names() {
  static _scv_associative_array<std::string,int>
    names("_generator Name Data Base",0);
  return names;
}

static std::list<scv_random *> *_list_of_generators = NULL;

static std::list<scv_random *>& s_list_of_generators() {
  if (!_list_of_generators) {
    _list_of_generators = new std::list<scv_random*>;
  }
  return *(_list_of_generators);
}

static bool readname_and_seed(std::string& nextName, unsigned long long & next_seed) {
  char nextChar;
  std::string name;
  name.reserve(60);

  if (s_exclusive_seed_file) {
    do {
      nextChar = getc(s_seed_file_ptr);
      if (nextChar == EOF) return false;
    } while (nextChar != '\"');
  } else {
    bool done = false;

    while (!done) {
      do {
	nextChar = getc(s_seed_file_ptr);
	if (nextChar == EOF) return false;
      } while (nextChar != '<');
      do {
	nextChar = getc(s_seed_file_ptr);
	if (nextChar == EOF) return false;
	if (nextChar != '>') name = name.append(1,nextChar);
      } while (nextChar != '>');
      if (name == s_seed_file_name())
	done = true;
      else
	name = "";
    }
    do {
      nextChar = getc(s_seed_file_ptr);
      if (nextChar == EOF) return false;
    } while (nextChar != '\"');
  }

  name = "";


  do {
    nextChar = getc(s_seed_file_ptr);
    if (nextChar == EOF) return false;
    if (nextChar != '\"') name = name.append(1,nextChar);
  } while (nextChar != '\"');
  nextName = name.c_str();

  int result = fscanf(s_seed_file_ptr," :: %llu", &next_seed);

  return result != EOF;
}

static void retrieve_seed(const std::string & name, unsigned long long * seed) {
  std::string s = name;

  if (s == std::string("")) s = std::string("<anonymous>");
  if (s_numOutstanding_seeds>0 && s_outstanding_seeds()[s].size()>0) {
    *seed = s_outstanding_seeds()[s].front();
    s_outstanding_seeds()[s].pop_front();
    --s_numOutstanding_seeds;
    return;
  }

  if (s_seed_file_ptr) {
    std::string nextName;
    unsigned long long next_seed;
    bool done = false;

    while (!done && readname_and_seed(nextName,next_seed)) {
      if (nextName == s ||
	  (nextName == std::string("<anonymous>") && s == std::string("")) ) {
	*seed = next_seed;
	done = true;
      } else {
	++s_numOutstanding_seeds;
	s_outstanding_seeds()[nextName].push_back(next_seed);
	if (!s_warnedOutOfOrder) {
	  s_warnedOutOfOrder = true;
	  scv_random_error::out_of_order_seed(s, s_seed_file_name());
	}
      }
    }

    if (!done) {
      scv_random_error::cannot_match_seed(s, s_seed_file_name());
      fclose(s_seed_file_ptr);
      s_seed_file_ptr = NULL;
    }
  } else {
      scv_random_error::cannot_match_seed(s, s_seed_file_name());
  }
}

static void addSelf(scv_random * self) {
  s_list_of_generators().push_back(self);
  if (s_store || s_retrieve) {
    std::string s = self->get_name();

    if (s!="") {
      if (++s_names()[s]>1) {
	if (!s_warned_same_name) {
	  s_warned_same_name = true;
	  if (s_store) scv_random_error::storing_with_same_name(s, s_seed_file_name());
	  else scv_random_error::retrieving_with_same_name(s, s_seed_file_name());
	}
      }
    } else {
      if (s_has_anonymous_generator && !s_warned_anonymous) {
	s_warned_anonymous = true;
	if (s_store) scv_random_error::storing_with_same_name("", s_seed_file_name());
	else scv_random_error::retrieving_with_same_name("", s_seed_file_name());
      } else {
	s_has_anonymous_generator = true;
      }
    }

    if (s_store) {
      if (s_exclusive_seed_file) {
	if (s!="") {
	  fprintf(s_seed_file_ptr,"\"%s\" :: %llu\n",self->get_name(),self->get_initial_seed());
	} else {
	  fprintf(s_seed_file_ptr,"\"<anonymous>\" :: %llu\n",self->get_initial_seed());
	}
      } else {
	if (s!="") {
	  fprintf(s_seed_file_ptr,"<%s> \"%s\" :: %llu\n",s_seed_file_name().c_str(),self->get_name(),self->get_initial_seed());
	} else {
	  fprintf(s_seed_file_ptr,"<%s> \"<anonymous>\" :: %llu\n",s_seed_file_name().c_str(),self->get_initial_seed());
	}
      }
    }
  }
}

unsigned long long _scv_get_global_seed(void) {
  return scv_random::get_global_seed();
}

static std::string _scv_get_unique_name(const std::string & name) {
#ifndef DONT_UNIQUIFY_NAMES
  std::string hier_object_name;
  if (!std::strcmp(s_current_inst_num,"<noappend>")) {
    hier_object_name = s_current_thread_name + "." + name;
  } else {
    hier_object_name = s_current_thread_name + "." + name +
        "_" + s_current_inst_num;
  }
  return hier_object_name;
#else
  return name;
#endif
}

static std::string _scv_extract_name(const char * str) {
  if (!str || 0==std::strcmp("",str)) return std::string("<anonymous>");
  std::string s(str);
  return s;
}

//////////////////////////////////////////////////////////////////////
// Static methods for implementation of the jrand48 for LINUX
//   - jrand48 on RH 7.0 is incompatible with RH 7.1 or other platforms
//   - Following implementation is from RH7.1 jrand48 and is compatible
//     with other platforms.
//   - This is important to obtain reproducible random streams across
//     various different platforms
//////////////////////////////////////////////////////////////////////

#if defined(__linux__) || defined(_MSC_VER) || defined(_WIN32)

struct _scv_linux_drand48_data
{
  unsigned short int __x[3];
  unsigned short int __old_x[3];
  unsigned short int __c;
  unsigned short int __init;
  unsigned long long int __a;
};

static struct _scv_linux_drand48_data drand48_data_glbl;

static int _scv_linux_drand48_iterate(unsigned short int xsubi[3],
  struct _scv_linux_drand48_data *buffer)
{
  unsigned long long X;
  unsigned long long result;

  if (!buffer->__init)
    {
      buffer->__a = 0x5deece66dull;
      buffer->__c = 0xb;
      buffer->__init = 1;
    }

  X = ((unsigned long long) xsubi[2]) << 32 | ((unsigned long) xsubi[1]) << 16 | xsubi[0];

  result = X * buffer->__a + buffer->__c;

  xsubi[0] = (unsigned short) (result & 0xffff);
  xsubi[1] = (unsigned short) (result >> 16) & 0xffff;
  xsubi[2] = (unsigned short) (result >> 32) & 0xffff;

  return 0;
}

static int _scv_linux_jrand48_r(unsigned short int xsubi[3],
  struct _scv_linux_drand48_data *buffer,
  long int *result)
{
  if (_scv_linux_drand48_iterate (xsubi, buffer) < 0)
    return -1;

  *result = ((xsubi[2] << 16) | xsubi[1]) & 0xffffffffl;

  return 0;
}

static long int _scv_linux_jrand48(unsigned short xsubi[3])
{
  long int result;

  (void) _scv_linux_jrand48_r(xsubi, &drand48_data_glbl, &result);

  return result;
}

#endif

static inline unsigned int _scv_jrand48(unsigned short next[3]) {
#if defined(__linux__) || defined(_MSC_VER) || defined(_WIN32)
  return (unsigned int) _scv_linux_jrand48(next);
#else
  return (unsigned int) jrand48(next);
#endif
}

